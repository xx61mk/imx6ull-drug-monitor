#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "UI/ui.h"
#include "project/include/mailbox.h"
#ifdef WITH_MQTT
#include "project/include/mqtt.h"   /* OneNET 云平台上云（make WITH_MQTT=1 启用） */
#endif

/* 邮箱 -> UI 的数据桥接
 * show_data 线程(邮箱消费者)收到 struct msgdata 后写入 g_latest 并递增 g_data_seq，
 * LVGL 主循环检测到序号变化后调用 ui_update_data()（ui_data.c），
 * 避免 LVGL 工作线程之外直接操作 UI 对象(LVGL 8 非线程安全)。 */
static pthread_mutex_t g_data_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct msgdata g_latest;
static volatile uint32_t g_data_seq = 0;   /* 每收到一份新数据 +1 */
int system_run_flag = 1;

/* 数据生成线程：模拟设备产生数据并通过邮箱投递 */
void *get_data_th(void *arg)
{
    struct msgdata mydata;
    int cnt = 0;
    time_t sec;
    struct tm *ptm = NULL;

    while (system_run_flag)
    {
        memset(&mydata, 0, sizeof(mydata));
        mydata.id  = 1;
        mydata.cnt = cnt++;
        mydata.temp = 25.0f + ((float)(rand() % 100) - 50) / 10.0f;  /* ~20.0 ~ 30.0 */
        mydata.hum  = 60.0f + ((float)(rand() % 160) - 80) / 10.0f;  /* ~52.0 ~ 68.0 */
        time(&sec);
        ptm = localtime(&sec);
        mydata.tim = *ptm;

        send_mail_msg("get_data", mydata, "show_data");  /* -> UI 刷新 */
        send_mail_msg("get_data", mydata, "save_data");  /* -> 存储 */

        sleep(3);
    }

    return NULL;
}

/* UI 刷新线程：从邮箱取出数据写入共享缓冲（不直接碰 LVGL 对象） */
void *show_data_th(void *arg)
{
    struct msgdata mydata;
    char send_name[32] = {0};

    while (system_run_flag)
    {
        if (recv_mail_msg(&mydata, send_name) != 0)
        {
            continue;
        }
        pthread_mutex_lock(&g_data_mutex);
        g_latest = mydata;
        g_data_seq++;
        pthread_mutex_unlock(&g_data_mutex);
    }

    return NULL;
}

/* 存储线程：打印数据，并在启用 WITH_MQTT 时把温湿度上传 OneNET 云平台 */
void *save_data_th(void *arg)
{
    struct msgdata mydata;
    char send_name[32] = {0};

    while (system_run_flag)
    {
        if (recv_mail_msg(&mydata, send_name) != 0)
        {
            continue;
        }
        printf("%s->save: id=%d cnt=%d temp=%.1f hum=%.1f\n",
               send_name, mydata.id, mydata.cnt, mydata.temp, mydata.hum);

#ifdef WITH_MQTT
        /* 上传 OneNET 物模型属性（key 需与云平台物模型标识符一致） */
        mqtt_send("temp", mydata.temp);
        mqtt_send("hum",  mydata.hum);
#endif
    }

    return NULL;
}

int main(void)
{
    /* LittlevGL init */
    lv_init();

    /* Linux frame buffer device init（/dev/fb0，不依赖 SDL） */
    fbdev_init();

    /* 双缓冲，整屏刷新 */
    static lv_color_t buf1[800 * 480];
    static lv_color_t buf2[800 * 480];

    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, 800 * 480);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf  = &disp_buf;
    disp_drv.flush_cb  = fbdev_flush;
    disp_drv.hor_res   = 800;
    disp_drv.ver_res   = 480;
    disp_drv.full_refresh = 1;
    lv_disp_drv_register(&disp_drv);

    /* 触摸屏 / 鼠标（evdev，/dev/input/eventX，见 lv_drv_conf.h） */
    evdev_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv);

    /* 鼠标光标图标 */
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t *cursor_obj = lv_img_create(lv_scr_act());
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);
    lv_indev_set_cursor(mouse_indev, cursor_obj);

    /* ---- 创建邮箱系统并注册线程：数据经邮箱驱动 UI ---- */
    Link_t *pmail = create_mailbox_system();
    if (NULL == pmail)
    {
        return -1;
    }
#ifdef WITH_MQTT
    /* 先连云再启动数据线程；失败仅提示，程序继续以离线模式运行 */
    mqtt_init();
#endif
    register_thread_mailbox(pmail, "get_data",  get_data_th);
    register_thread_mailbox(pmail, "show_data", show_data_th);
    register_thread_mailbox(pmail, "save_data", save_data_th);

    /* Create the UI */
    ui_init();

    uint32_t last_seq = 0;
    while (1) {
        lv_timer_handler();

        /* 有新数据到达时刷新 UI（标签 + 折线图） */
        if (g_data_seq != last_seq) {
            last_seq = g_data_seq;
            struct msgdata d;
            pthread_mutex_lock(&g_data_mutex);
            d = g_latest;
            pthread_mutex_unlock(&g_data_mutex);
            ui_update_data(d.temp, d.hum, d.id);
        }

        usleep(5000);
    }

    destroy_mailbox(pmail);

    return 0;
}

/* Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR` */
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
