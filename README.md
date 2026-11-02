# 药品恒温存储与安全监控系统

面向胰岛素、疫苗这类必须全程 **2~8℃** 冷藏的药品，用 **i.MX6ULL** 搭一个带触摸屏的监控终端：
多传感器实时采集 → 滞回控温 + 四类报警 → SQLite 本地落盘 → MQTT 上云（断网缓存、重连补传）。

`C / Linux` · `多线程 + 邮箱` · `SQLite3` · `MQTT` · `LVGL + Framebuffer` · `sysfs / w1 / hwmon / IIO`

> 这不是代码片段，是能直接 `make` 出来跑的完整工程。没有开发板也能测：
> 采集层内置热模型仿真，可复现降温、开门、碰撞、泄漏、制冷失效五种工况。

---

## 一、功能

| 功能 | 实现方式 |
|---|---|
| 实时采集 | DS18B20（1-Wire）、LM75（hwmon）、ADXL345（IIO）、门磁 / 泄漏（GPIO）；无设备时切热模型仿真 |
| 自动控温 | 制冷 8.0℃ 启 / 6.5℃ 停，加热 2.0℃ 启 / 3.5℃ 停，带回差、互锁、最小切换间隔 |
| 四类报警 | 超温 / 低温、开门超时、碰撞倾倒、液体泄漏，带延时确认与上电抑制 |
| 本地存储 | SQLite3：WAL、按天分表、批量事务、7 天归档、启动完整性检查 |
| 云端上报 | MQTT / OneNET，在线直发；掉线入环形队列并落盘，重连按序补传（QoS1），断电可续传 |
| 本地交互 | LVGL 8 + Framebuffer + evdev 触摸，实时曲线与参数显示，控件只在主线程操作 |

## 二、目录结构

```
imx6ull-drug-monitor/
├── Makefile                    本地 / 交叉 / MQTT 三种构建，make check 只做语法检查
├── include/
│   ├── app_config.h            全部阈值、周期、设备路径（改参数只动这里）
│   ├── link.h / linkqueue.h / mailbox.h    邮箱框架：线程注册表 + 每线程队列
│   ├── sensor.h                采集抽象：板端 sysfs / 仿真热模型
│   ├── control.h               控温 + 报警判定
│   ├── storage.h               SQLite：WAL、按天分表、归档
│   ├── ringbuf.h               离线环形队列（带落盘）
│   ├── uplink.h                上报：在线直发 / 离线入队 / 重连补传
│   ├── mqtt_client.h           Paho 封装（可选，-DWITH_MQTT）
│   └── app.h                   线程编排
├── src/
│   ├── link.c / linkqueue.c / mailbox.c    框架层
│   ├── sensor.c                采集 + 热模型 + 故障注入
│   ├── control.c               滞回控温 + 报警判定
│   ├── storage.c               SQLite 持久化
│   ├── ringbuf.c               环形队列
│   ├── uplink.c                上报与补传
│   ├── mqtt_client.c           仅 -DWITH_MQTT 参与编译
│   ├── app.c                   五个线程的编排
│   └── main.c                  终端版入口
├── lvgl_app/main_lvgl.c        板端带界面入口（LVGL + framebuffer + evdev）
├── UI/                         SquareLine 生成的界面、中文字库、图标
├── lv_conf.h / lv_drv_conf.h   LVGL 与显示输入配置
├── tools/
│   ├── verify.sh               自检脚本，四条链路自动断言
│   └── syntax_check.py         无编译器时的纯语法体检（pycparser）
├── third_party/sqlite3/        内置 amalgamation，免得板端缺 libsqlite3-dev
├── docs/代码流程.md             启动链 → 初始化 → 邮箱 → 数据流 的逐段说明（v1 视角）
└── legacy/                     v1 框架版（业务为空），保留作演进对照
```

## 三、架构

数据只走邮箱，线程间不共享可变状态：

```
get_data ──► ctrl ──┬──► show_data  ──► 主线程刷新 UI（LVGL 只在这里碰）
                    ├──► save_data  ──► SQLite（批量事务）
                    └──► send_data  ──► 在线直发 / 离线入环形队列
```

控温放在 `ctrl` 线程做完，再把「带执行器状态和报警位的样本」分发出去，
所以库里记的继电器状态、上报的状态与实际动作是同一拍，不会错位。

**LVGL 8 非线程安全**，所以工作线程只写共享结构体，UI 刷新统一收敛到主线程。

## 四、编译与运行

需要 Linux（用到 pthread、sysfs；Windows 只能读代码）。

```bash
make                 # 产出 ./drug-monitor，SQLite 源码一起编，无需额外依赖
./drug-monitor --help

make check           # 只做语法/告警检查，不产出可执行文件
make arm             # 交叉编译到 i.MX6ULL（arm-linux-gnueabihf-gcc）
make WITH_MQTT=1     # 接真实 MQTT（需 libpaho-mqtt3c）
```

环境里没有编译器时，先做一次纯语法体检（需要 pycparser）：

```bash
python3 tools/syntax_check.py
```

第一次跑，建议按这个顺序看效果：

```bash
# 1) 只看控温：25℃ 降到 6.5℃ 停，回升到 8℃ 再启，形成锯齿
./drug-monitor --sim --scenario none --seconds 120

# 2) 看四类报警：开门超时 / 碰撞 / 泄漏 / 制冷失效导致超温
./drug-monitor --sim --scenario demo --interval 1000 --seconds 300

# 3) 看断网补传：第 10~30 秒链路断开，数据入队，恢复后补传
./drug-monitor --sim --scenario none --seconds 60 --net-down 10:30

# 4) 一条命令把四条链路全测一遍（约 2 分钟）
sh tools/verify.sh
```

调试时用 `--speed 10 --interval 100`：仿真时间按 10 倍推进，40 秒跑完 5 分钟工况。
控制用的是虚拟时钟，加速不改变控温算法的实际行为。

| 参数 | 说明 |
|---|---|
| `--sim` / `--board` / `--auto` | 采集后端。`auto` 先探测板端设备，探测不到自动转仿真 |
| `--scenario demo\|none` | 是否注入故障（默认 demo） |
| `--interval MS` | 采样周期，默认 1000ms |
| `--speed N` | 仿真时间倍数，默认 1 |
| `--seconds N` | 跑多少秒后自动退出，0 表示一直跑到 Ctrl+C |
| `--db PATH` | SQLite 文件路径，默认 `env.db` |
| `--retention DAYS` | 数据保留天数，默认 7 |
| `--net-down A:B` | 模拟第 A~B 秒断网（仅仿真后端） |
| `--dump N` | 退出前打印最近 N 条记录 |

## 五、输出怎么读

```
[mon] #42   T= 6.51 H= 48.2 A=1.00 door=0 leak=0 HTR=0 CLR=1 ALM=0x00
[ALARM] temp_high    ON   T= 8.63 door=0 leak=0 accel=1.00
[uplink] sent   #42   T= 6.51 H= 48.2 A=1.00 D=0 L=0 ALM=0x00
[uplink] offline, cached #43 (pending=1)
```

| 字段 | 含义 |
|---|---|
| `T` | 药品区温度 ℃（DS18B20 优先，缺失则用 LM75 的 hwmon） |
| `H` | 湿度 %RH，板端未接则为 -1 |
| `A` | 加速度模值 g，静止约 1.0，倾倒/碰撞会跳变 |
| `HTR` / `CLR` | 加热 / 制冷继电器当前状态 |
| `ALM` | 报警位图：`0x01` 超温 `0x02` 低温 `0x04` 开门超时 `0x08` 碰撞 `0x10` 泄漏 `0x20` 传感器故障 |

退出时打印汇总，全部 `key=value`，方便脚本断言：

```
samples=120
db=env.db rows=120 written=120 size=32KB err=0
relay_switches=14
alarm_high=0 alarm_low=0 alarm_door=1 alarm_tilt=1 alarm_leak=1 alarm_sensor=0
uplink sent=90 queued=30 replayed=30 fail=0 pending=0
queue_dropped=0
```

## 六、控制策略

| 项 | 规则 | 为什么这么定 |
|---|---|---|
| 制冷 | ≥8.0℃ 启动，≤6.5℃ 停止 | 1.5℃ 回差，避免在阈值上反复启停 |
| 加热 | ≤2.0℃ 启动，≥3.5℃ 停止 | 同上，且与制冷互锁，绝不同时开 |
| 继电器保护 | 两次动作间隔 ≥5s | 压缩机频繁启停会缩短寿命 |
| 越界优先 | 偏差 >1.5℃ 时无视间隔立刻动作 | 保护药品优先于保护继电器 |
| 温度报警 | 越限持续 3s 才报 | 开门、搬运造成的瞬时波动不误报 |
| 开门报警 | 持续 60s | 正常取药不会超过这个时间 |
| 碰撞报警 | 加速度偏离 1g 超过 0.45g 且持续 2s | 分得清「放歪了」和「被撞了」 |
| 泄漏报警 | 立即 | 液体进箱没有可容忍的延迟 |
| 上电抑制 | 上电 60s 内不报温度越限 | 刚装载时箱内是室温，预冷到 2~8℃ 之前不该算故障 |

时序基准是虚拟时钟（每来一个样本前进一个采样周期），真机 1s 采样等于实际时间，
`--speed 10` 时按 10 倍推进，所以加速测试出的控制行为与真机一致。

## 七、数据与断网设计

**本地存储**（`src/storage.c`）

- WAL 模式：读写不互斥，采集线程写、查询读互不阻塞
- 按天分表 `env_YYYYMMDD`：单表只装一天，删历史是 `DROP TABLE` 而不是 `DELETE`，不留碎片
- 批量事务：每 10 条提交一次（`CFG_DB_BATCH_ROWS`），SD 卡上写放大降一个量级
- 保留 7 天：到期把整表搬进 `env_archive` 再删表
- 启动 `PRAGMA integrity_check`：真损坏就把文件改名留证并重建，不让程序起不来

**断网补传**（`src/ringbuf.c` + `src/uplink.c`）

- 在线直接 publish；失败或已离线就进 512 条环形队列（`CFG_QUEUE_SIZE`）
- 队列每条同时写 `uplink_queue.bin`，断电重启继续传，不丢已确认要上报的数据
- 重连后按序号补传，间隔 50ms 一条（`CFG_UPLINK_FLUSH_MS`），避免弱网被一次性打爆
- 队满丢弃最旧并计入 `queue_dropped`，日志可见，不静默丢数据

## 八、跑在板子上

```bash
make arm                    # 用 arm-linux-gnueabihf-gcc
scp drug-monitor-arm root@<board>:/root/
```

上板前准备好设备节点（路径都在 `include/app_config.h`，按实际改）：

```bash
# 1-Wire（DS18B20）：设备树里加 w1-gpio，内核会导出 /sys/bus/w1/devices/28-xxxx/w1_slave
ls /sys/bus/w1/devices/            # 有 28- 开头的目录就通了

# LM75：i2c 驱动加载后走 hwmon
cat /sys/class/hwmon/hwmon0/temp1_input     # 单位 0.001℃

# ADXL345：走 IIO
cat /sys/bus/iio/devices/iio:device0/in_accel_x_raw

# 门磁 / 泄漏 / 继电器：sysfs GPIO（编号按板子改）
for n in 45 46 47 48; do echo $n > /sys/class/gpio/export; done
echo out > /sys/class/gpio/gpio47/direction     # 继电器输出
echo in  > /sys/class/gpio/gpio45/direction     # 门磁输入
```

然后 `./drug-monitor-arm --board --seconds 0`。若探测不到任何温度设备，
程序会退回仿真并在启动横幅里说明，不会直接崩。

**带界面的版本**：把 `lvgl_app/main_lvgl.c` 当作 LVGL framebuffer 工程的入口，
与 `src/` 下除 `main.c` 外的文件一起编译，头文件加 `-I.` 和 `-Iinclude`（`-I.` 是给 `UI/ui.h` 用的）。
界面侧若实现了 `void ui_update_alarm(unsigned int alarm)`，主循环会自动调用；没实现就用
`main_lvgl.c` 里的 weak 空实现兜底，链接不会失败。`legacy/Makefile.v1` 里有把 LVGL、UI、
业务代码串起来的完整编译规则，可作参照。

## 九、接真实云平台

源码不保存任何密钥（v1 曾把 OneNET 三元组写死在 `mqtt.h` 里并随仓库公开，
**那个设备密钥必须去控制台重置**）。v2 只从环境变量读：

```bash
export MQTT_BROKER="tcp://183.230.40.96:1883"
export MQTT_PRODUCT="你的产品ID"
export MQTT_DEVICE="你的设备名"
export MQTT_TOKEN="你重新生成的设备token"

make WITH_MQTT=1
./drug-monitor --sim --scenario none --seconds 60
```

环境变量没配齐时只打印一句提示并以离线模式运行，不影响本地监控。

## 十、版本演进

| 位置 | v1 的问题 | v2 的处理 |
|---|---|---|
| `get_data_th` | `rand()` 造温湿度 | 板端读 sysfs（w1 / hwmon / IIO / gpio），无设备时走热模型仿真 |
| `mysqlite.c` | 只有一行 `#include` | 完整实现：WAL、按天分表、批量事务、过期归档、启动完整性检查 |
| 控温 / 报警 | 没有 | 滞回控温 + 最小切换间隔；四类报警带延时确认，只在新出现时输出 |
| 上报 | 掉线只打印 | 离线环形队列 + 落盘 + 重连按序补传（QoS1），断电重启可续传 |
| `mailbox.c` | 先 `pthread_create` 再挂链表，线程启动瞬间找不到自己；`strcpy` 无长度检查；`sem_wait` 无超时导致 join 卡死 | 先挂链表再起线程、线程身份用 TLS、收件带超时并检查退出标志 |
| `destroy_mailbox` | 主循环 `while(1)` 永不退出，`destroy` 是死代码 | 信号触发退出标志，`app_stop()` 能真正 join 所有线程 |

v1 的入口与框架代码保留在 `legacy/`（`main.c`、`project/`、`Makefile.v1`），仅作演进对照，
业务层是空的，不建议直接使用。

## 十一、已知边界

面试被问到时说清楚，比被问穿好：

1. **湿度没接**：板端 `H=-1`。DHT11 是单总线时序敏感器件，放用户态读会被调度打断，
   要做得稳需要内核驱动或专用 IC（如 SHT30 走 I2C）。
2. **继电器是 sysfs 直写**：没有做掉电保持和硬件互锁，真机建议加继电器互锁电路，并在驱动层做状态机。
3. **MQTT 用同步 API**：发送最多阻塞 5s，已放在独立线程，不影响采集与 UI；更高吞吐可换异步 API + 本地批量。
4. **加速度只判模值**：能检出碰撞和倾倒，但分不清「倾斜放置」和「瞬时冲击」，真机可再结合重力方向做姿态角估计。
5. **单机存储**：SQLite 适合单终端；多终端汇总要靠云端，本地不做分布式。

## 十二、硬件

- 主控：NXP i.MX6ULL（Cortex-A7），Linux 4.1.15
- 温度：DS18B20（1-Wire）、LM75（I2C）；姿态：ADXL345（SPI/IIO）
- 其他：门磁开关、液体泄漏检测、继电器驱动的制冷/加热模块、蜂鸣器 + LED
- 显示：RGB LCD + 电阻触摸屏（Framebuffer + evdev）

## 十三、第三方依赖

| 依赖 | 版本 | 说明 |
|---|---|---|
| SQLite3 | 3.x | 已内置 amalgamation（`third_party/sqlite3/`），也可 `make USE_SYSTEM_SQLITE=1` |
| paho.mqtt.c | 1.3.x | 仅 `WITH_MQTT=1` 时需要 |
| LVGL | 8.x | 仅 UI 版需要，放 `./lvgl`；`lv_drivers` 放 `./lv_drivers` |

```bash
git clone https://github.com/lvgl/lvgl.git --branch release/v8.3
git clone https://github.com/lvgl/lv_drivers.git --branch release/v8.3
```

## 许可证

MIT，见 [LICENSE](LICENSE)。
