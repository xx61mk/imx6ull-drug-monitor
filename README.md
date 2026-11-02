# i.MX6ULL 药品恒温存储与安全监控系统

嵌入式芯片设计大赛（应用赛道）参赛项目。面向胰岛素、疫苗这类必须全程 2~8℃ 冷藏的药品，用 i.MX6ULL 搭建一个带触摸屏的监控终端：多传感器实时采集、温度越界自动控温、异常声光报警、本地结构化存储、云端远程监管。

## 功能

- **实时采集**：温度、湿度、倾斜/碰撞、门磁、液体泄漏多路采样
- **自动控温**：温度越界自动启停制冷/加热，维持在 2~8℃
- **异常报警**：开门超时、碰撞倾倒、泄漏、超温四类事件触发声光提示
- **本地存储**：SQLite3 结构化落盘，支持历史查询
- **云端监管**：MQTT 接入 OneNET，定时上报状态、即时推送报警
- **本地交互**：LVGL + Framebuffer + 触摸屏，实时曲线与参数设置

### 实现进度

框架层已跑通，业务层按下面的状态推进（细节见 [`docs/代码流程.md`](docs/代码流程.md) 第八节）：

| 模块 | 状态 |
|---|---|
| 多线程 + 邮箱解耦、链式队列 | ✅ 完整 |
| LVGL 显示 + 触摸 + 主线程刷新 | ✅ 完整 |
| MQTT 连接 / 发布 / 回调（OneNET 物模型） | ✅ 完整 |
| 传感器真实采集 | ⚠️ 当前用 `rand()` 模拟，接入点已预留 |
| SQLite3 落盘 | ⚠️ `mysqlite.c` 为空实现，构建已链接 sqlite3 |
| 自动控温 / 声光报警 | ❌ 待实现（新增 ctrl 线程注册进邮箱即可） |
| 断线重连补传 | ❌ 待实现 |

## 架构

采用「采集 → 邮箱 → 消费」的解耦结构，三个工作线程 + 一个 UI 主循环：

```
get_data_th   采样传感器，封装 struct msgdata 投递到邮箱
    ↓ (mailbox + 互斥锁)
show_data_th  取数据写共享缓存 → 主循环按序号变化调用 ui_update_data()
save_data_th  取数据 → SQLite3 落盘 / MQTT 上报
main           lv_timer_handler() 驱动 UI，UI 对象只在主线程操作
```

关键约束：**LVGL 8 非线程安全**，所以工作线程只写共享结构体并用 `g_data_seq` 递增做标记，UI 的刷新统一收敛到主线程，避免跨线程操作控件导致卡死。

📄 **代码执行流程详解**（启动链 → 初始化 → 邮箱机制 → 数据流 → 云端 → 各模块接入点）：[`docs/代码流程.md`](docs/代码流程.md)

## 目录

```
├── main.c                 带 UI 的整合版入口（三线程 + 邮箱 + LVGL 主循环）
├── lv_conf.h              LVGL 配置
├── lv_drv_conf.h          显示与输入设备配置（fbdev / evdev）
├── UI/                    SquareLine Studio 生成的界面代码
│   ├── screens/           界面与控件逻辑
│   ├── fonts/             中文字库（转 C 数组）
│   ├── images/            图标资源
│   └── ui_data.c          数据刷新接口 ui_update_data()
└── project/
    ├── include/           link / linkqueue / mailbox / mqtt / mysqlite 头文件
    └── src/
        ├── main.c         纯逻辑版入口（不带 UI，四线程，便于单独验证业务）
        ├── mailbox.c      线程间邮箱，mutex + 信号量（sem_wait/sem_post）
        ├── link.c         链表（线程注册表）
        ├── linkqueue.c    链式队列
        ├── mqtt.c         MQTT 封装（paho）
        └── mysqlite.c     SQLite3 封装（当前为空实现，见 docs/代码流程.md 第八节）
```

## 依赖

仓库只保留自研代码，第三方库需自行准备（体积大，未入库）：

| 依赖 | 版本 | 放置位置 |
|---|---|---|
| LVGL | 8.x | `./lvgl` |
| lv_drivers | 8.x | `./lv_drivers` |
| SQLite3 | 3.x | `./project/lib/sqlite3/{include,lib}` |
| paho.mqtt.c | 1.3.x | `./project/lib/build/output` |
| OpenSSL | 1.1.x | `./project/lib/openssl_install` |

```bash
git clone https://github.com/lvgl/lvgl.git --branch release/v8.3
git clone https://github.com/lvgl/lv_drivers.git --branch release/v8.3
```

## 编译

```bash
# 本机调试（x86 Linux，需要 /dev/fb0）
make

# 交叉编译到开发板
make CC=arm-linux-gnueabihf-gcc

# 启用 MQTT 上云
make WITH_MQTT=1 CC=arm-linux-gnueabihf-gcc
```

产物为 `./demo`，拷到板子上运行即可。纯逻辑版单独编译：

```bash
cd project/src && make
```

## 硬件

- 主控：NXP i.MX6ULL（ARM Cortex-A7），Linux 4.1.15
- 温度：DS18B20（1-Wire）、LM75（I2C）
- 姿态：ADXL345（SPI），用于碰撞倾倒检测
- 其他：门磁开关、液体泄漏检测、继电器控制的制冷/加热模块、蜂鸣器 + LED
- 显示：RGB LCD + 电阻触摸屏（Framebuffer + tslib/evdev）

## ⚠️ 安全提示

`project/include/mqtt.h` 里的 `PRODUCT_ID`、`PASSWD` 是 OneNET 平台的设备三元组与鉴权 token，目前已随仓库公开。**请到 OneNET 控制台重置该设备的密钥**，旧 token 随之失效。

后续建议改成本地不提交的配置头：

```c
/* project/include/onenet_secret.h  —— 加入 .gitignore */
#define ONENET_PRODUCT_ID "xxxxxxx"
#define ONENET_PASSWD     "version=2018-10-31&res=..."
```

```c
/* mqtt.h 里改为引用它，仓库只提交 onenet_secret.h.example */
#include "onenet_secret.h"
```
