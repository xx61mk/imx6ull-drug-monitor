#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/*
 * 全部可调参数集中在这里。改阈值、改采样周期只动这一个文件。
 * 运行期可用命令行覆盖的项在 main.c 里给出（见 --help）。
 */

/* ---------------- 采样与刷新 ---------------- */
#define CFG_SAMPLE_MS            1000     /* 采集周期 ms */
#define CFG_UI_REFRESH_MS         200     /* UI 刷新周期 ms（LVGL 主线程） */

/* ---------------- 药品恒温阈值 ℃ ---------------- */
#define CFG_TEMP_HIGH             8.0f    /* 高于此值启动制冷 */
#define CFG_TEMP_COOL_OFF         6.5f    /* 低于此值停止制冷（回差 1.5℃） */
#define CFG_TEMP_LOW              2.0f    /* 低于此值启动加热 */
#define CFG_TEMP_HEAT_OFF         3.5f    /* 高于此值停止加热（回差 1.5℃） */
#define CFG_ACTUATOR_MIN_GAP_MS   5000    /* 制冷/加热最小切换间隔，保护继电器 */

/* ---------------- 报警条件 ---------------- */
#define CFG_TEMP_ALARM_DELAY_MS   3000    /* 温度越限持续多久才报警，滤掉瞬时抖动 */
#define CFG_DOOR_ALARM_MS         60000   /* 开门超过 60s 报警 */
#define CFG_TILT_THRESHOLD_G      0.45f   /* 加速度模值偏离 1g 超过此值判定碰撞 */
#define CFG_TILT_ALARM_MS         2000    /* 碰撞持续 2s 才报警 */
#define CFG_ALARM_WARMUP_MS       60000   /* 上电/复电抑制期：装载时箱内是室温，预冷阶段不报温度超限 */

/* ---------------- 本地存储 ---------------- */
#define CFG_DB_PATH               "env.db"
#define CFG_DB_RETENTION_DAYS     7       /* 保留天数，超过归档后删表 */
#define CFG_DB_BATCH_ROWS         10      /* 每 10 条提交一次事务 */
#define CFG_DB_ARCHIVE            "env_archive"

/* ---------------- 云端上报 ---------------- */
#define CFG_QUEUE_SIZE            512     /* 离线环形队列容量 */
#define CFG_QUEUE_FILE            "uplink_queue.bin"
#define CFG_UPLINK_RETRY_MS       2000    /* 断线重连间隔 */
#define CFG_UPLINK_FLUSH_MS       50     /* 补传时每条之间的间隔，避免打爆弱网 */

/* ---------------- 板端设备路径（sysfs / hwmon / IIO） ---------------- */
#define CFG_W1_DEVICE_DIR         "/sys/bus/w1/devices"
#define CFG_W1_SLAVE_FILE         "w1_slave"
#define CFG_HWMON_TEMP            "/sys/class/hwmon/hwmon0/temp1_input"
#define CFG_IIO_DIR               "/sys/bus/iio/devices/iio:device0"
#define CFG_GPIO_DOOR             "/sys/class/gpio/gpio45/value"
#define CFG_GPIO_LEAK             "/sys/class/gpio/gpio46/value"
#define CFG_GPIO_HEATER           "/sys/class/gpio/gpio47/value"
#define CFG_GPIO_COOLER           "/sys/class/gpio/gpio48/value"

#endif
