/*
 * 板端带界面的入口（i.MX6ULL + Framebuffer + 触摸屏）
 *
 * 编译方式见 README「板端构建」：把本文件和 src/ 下除 main.c 以外的文件一起编译，
 * 并加上 -Iinclude。业务逻辑全部在 app.c 里，这里只负责 LVGL 初始化与主循环。
 *
 * 关键约束：LVGL 8 不是线程安全的，所有控件操作只能在主线程做。
 * 工作线程把数据写进 app.c 的共享缓冲，主循环用 app_new_sample() 取，
 * 取到了才调 ui_update_data()，绝不跨线程碰控件。
 */

#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "UI/ui.h"
#include "../include/app.h"
#include "../include/app_config.h"
#include "../include/mailbox.h"
#include "../include/sensor.h"
#include "../include/storage.h"
#include "../include/uplink.h"

/* 如果 UI 里没实现报警显示，用这个空实现兜底，链接不会报错 */
__attribute__((weak)) void ui_update_alarm(unsigned int alarm)
{
    (void)alarm;
}

static void on_signal(int sig)
{
    (void)sig;
    system_run_flag = 0;
}

int main(void)
{
    struct msgdata d;

    lv_init();
    fbdev_init();

    {
        static lv_color_t buf1[800 * 480];
        static lv_color_t buf2[800 * 480];
        static lv_disp_draw_buf_t disp_buf;
        static lv_disp_drv_t disp_drv;

        lv_disp_draw_buf_init(&disp_buf, buf1, buf2, 800 * 480);
        lv_disp_drv_init(&disp_drv);
        disp_drv.draw_buf = &disp_buf;
        disp_drv.flush_cb = fbdev_flush;
        disp_drv.hor_res = 800;
        disp_drv.ver_res = 480;
        disp_drv.full_refresh = 1;
        lv_disp_drv_register(&disp_drv);
    }

    evdev_init();
    {
        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = evdev_read;
        lv_indev_drv_register(&indev_drv);
    }

    ui_init();

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    /* 业务层：采集 + 控温 + 存储 + 上报，全部线程在 app.c 里拉起 */
    {
        struct sensor_cfg scfg;

        memset(&scfg, 0, sizeof(scfg));
        scfg.mode = SENSOR_AUTO;         /* 有板子读 sysfs，没板子自动转仿真 */
        scfg.scenario = SCENARIO_NONE;   /* 真机不要注入假故障 */
        scfg.interval_ms = CFG_SAMPLE_MS;

        sensor_init(&scfg);
        storage_open(CFG_DB_PATH, CFG_DB_RETENTION_DAYS);
        uplink_init(0, 0);
        app_set_interval(CFG_SAMPLE_MS);

        if (0 != app_start())
        {
            printf("[lvgl] app start failed\n");
            return -1;
        }
    }

    while (app_running())
    {
        lv_timer_handler();

        if (app_new_sample(&d))
        {
            ui_update_data(d.temp, d.hum, d.id);
            ui_update_alarm(d.alarm);
        }

        usleep(5000);       /* 5ms，留出 CPU 给采集线程 */
    }

    app_stop();
    sensor_deinit();
    app_print_summary();
    storage_close();
    uplink_shutdown();

    return 0;
}

/* lv_conf.h 里 LV_TICK_CUSTOM_SYS_TIME_EXPR 调用它 */
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    if (0 == start_ms)
    {
        start_ms = (uint64_t)tv.tv_sec * 1000u + (uint64_t)tv.tv_usec / 1000u;
    }
    return (uint32_t)(((uint64_t)tv.tv_sec * 1000u + (uint64_t)tv.tv_usec / 1000u) - start_ms);
}
