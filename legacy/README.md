# legacy — v1 框架版

这里是本项目的第一版代码，**只把框架跑通，业务层是空的**。保留它是为了让演进有据可查：
v2（仓库根目录）在同样的「邮箱 + 多线程」骨架上，把采集、控温、存储、补传四块业务补完了。

## 里面有什么

```
legacy/
├── main.c                 带 UI 的整合版入口（三线程 + 邮箱 + LVGL 主循环）
├── Makefile.v1            framebuffer 版构建脚本（引用根目录的 lvgl / lv_drivers / UI）
└── project/
    ├── include/           link / linkqueue / mailbox / mqtt / mysqlite 头文件
    └── src/
        ├── main.c         纯逻辑版入口（不带 UI，四线程，便于单独验证业务）
        ├── mailbox.c      线程间邮箱，mutex + 信号量
        ├── link.c         链表（线程注册表）
        ├── linkqueue.c    链式队列
        ├── mqtt.c         MQTT 封装（paho）
        └── mysqlite.c     SQLite3 封装（v1 时为空实现）
```

## v1 的状态

| 模块 | 状态 |
|---|---|
| 多线程 + 邮箱解耦、链式队列 | 完整 |
| LVGL 显示 + 触摸 + 主线程刷新 | 完整 |
| MQTT 连接 / 发布 / 回调（OneNET 物模型） | 完整 |
| 传感器真实采集 | 未实现，用 `rand()` 模拟 |
| SQLite3 落盘 | 未实现，构建已链接 sqlite3 |
| 自动控温 / 声光报警 | 未实现 |
| 断线重连补传 | 未实现 |

## 怎么编译（仅作参考）

`Makefile.v1` 里的路径是相对**仓库根目录**写的，想跑请把它拷回根目录：

```bash
cp legacy/Makefile.v1 ./Makefile.v1
# 先按根目录 README 第十三节把 lvgl / lv_drivers 放到 ./lvgl、./lv_drivers
make -f Makefile.v1 CC=arm-linux-gnueabihf-gcc
```

v1 的 `main.c` 依赖根目录的 `UI/`、`lv_conf.h`、`lv_drv_conf.h`，这些在 v2 里继续使用，
所以没有一起搬进来。

## 关于密钥

`project/include/mqtt.h` 里的 OneNET 产品 ID 与 token 曾经是明文并随仓库公开，
现在已替换为 `YOUR_PRODUCT_ID` / `YOUR_DEVICE_TOKEN` 占位符。
**旧的设备密钥请到 OneNET 控制台重置**——它仍然留在本仓库的早期提交历史里，删不掉，只能作废。
v2 已改为从环境变量读取，见根目录 README 第九节。
