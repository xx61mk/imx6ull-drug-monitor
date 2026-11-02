#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "../include/app.h"
#include "../include/mailbox.h"
#include "../include/sensor.h"
#include "../include/control.h"
#include "../include/storage.h"
#include "../include/uplink.h"
#include "../include/ringbuf.h"
#include "../include/app_config.h"

/* 线程编排
 * --------
 *   get_data ──► ctrl ──┬──► show_data  (写共享缓冲，主线程刷新 UI)
 *                       ├──► save_data  (SQLite)
 *                       └──► send_data  (云平台)
 *
 * 控制放在 ctrl 线程里做，做完把带执行器状态和报警位的样本再分发出去，
 * 这样存储和上报拿到的是「同一份、已决断」的数据，不会出现库里记的
 * 执行器状态比实际晚一拍的问题。
 */

static struct msgdata g_latest;
static pthread_mutex_t g_latest_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned int g_seq = 0;
static unsigned int g_consumed = 0;
static Link_t *g_pmail = NULL;

static int   g_samples = 0;
static int   g_interval_ms = CFG_SAMPLE_MS;
static int   g_alarm_count[6] = {0};

static void sleep_interval(int total_ms)
{
    int step = 100;
    int left = total_ms;

    while (left > 0 && system_run_flag)
    {
        int now = (left < step) ? left : step;
#if defined(_WIN32)
        Sleep((DWORD)now);
#else
        usleep((useconds_t)now * 1000);
#endif
        left -= now;
    }
}

static struct tm now_tm(void)
{
    time_t sec = time(NULL);
    struct tm tmv;
#if defined(_WIN32)
    struct tm *p = localtime(&sec);
    if (p != NULL)
    {
        tmv = *p;
    }
    else
    {
        memset(&tmv, 0, sizeof(tmv));
    }
#else
    localtime_r(&sec, &tmv);
#endif
    return tmv;
}

static const char *alarm_name(unsigned int bit)
{
    switch (bit)
    {
    case ALARM_TEMP_HIGH: return "temp_high";
    case ALARM_TEMP_LOW:  return "temp_low";
    case ALARM_DOOR:      return "door_open";
    case ALARM_TILT:      return "tilt";
    case ALARM_LEAK:      return "leak";
    case ALARM_SENSOR:    return "sensor_fault";
    default:              return "unknown";
    }
}

static void alarm_count(unsigned int bits)
{
    int i;
    for (i = 0; i < 6; i++)
    {
        if (bits & (1u << i))
        {
            g_alarm_count[i]++;
        }
    }
}

static void print_alarm_bits(const char *tag, unsigned int bits, const struct msgdata *d)
{
    int i;

    for (i = 0; i < 6; i++)
    {
        unsigned int bit = 1u << i;
        if (bits & bit)
        {
            printf("[ALARM] %-12s %s  T=%.2f door=%d leak=%d accel=%.2f\n",
                   alarm_name(bit), tag, d->temp, d->door, d->leak, d->a);
        }
    }
}

/* ---------------- 采集线程 ---------------- */
static void *get_data_th(void *arg)
{
    (void)arg;

    while (system_run_flag)
    {
        struct msgdata d;

        memset(&d, 0, sizeof(d));
        if (0 == sensor_read(&d))
        {
            d.id = 1;
            d.cnt = g_samples++;
            d.tim = now_tm();
            send_mail_msg("get_data", d, "ctrl");
        }

        sleep_interval(g_interval_ms);
    }
    return NULL;
}

/* ---------------- 控制 + 报警线程 ---------------- */
static void *ctrl_th(void *arg)
{
    struct msgdata d;
    char who[32];
    unsigned int last_alarm = 0;

    (void)arg;

    while (system_run_flag)
    {
        struct ctrl_output out;

        if (0 != recv_mail_msg(&d, who))
        {
            break;
        }

        memset(&out, 0, sizeof(out));
        if (0 != control_step(&d, &out))
        {
            continue;
        }

        /* 下发执行器：板端写 GPIO，仿真更新热模型 */
        sensor_actuator(out.heater, out.cooler);

        /* 把决断结果写回样本，再分发给下游 */
        d.heater = out.heater;
        d.cooler = out.cooler;
        d.alarm = out.alarm;

        if (out.new_alarm != 0)
        {
            print_alarm_bits("ON ", out.new_alarm, &d);
            alarm_count(out.new_alarm);
        }
        if (0 == out.alarm && last_alarm != 0)
        {
            printf("[ALARM] all clear\n");
        }
        last_alarm = out.alarm;

        printf("[mon] #%-4d T=%5.2f H=%5.1f A=%4.2f door=%d leak=%d HTR=%d CLR=%d ALM=0x%02X\n",
               d.cnt, d.temp, d.hum, d.a, d.door, d.leak,
               d.heater, d.cooler, d.alarm);

        send_mail_msg("ctrl", d, "show_data");
        send_mail_msg("ctrl", d, "save_data");
        send_mail_msg("ctrl", d, "send_data");
    }
    return NULL;
}

/* ---------------- UI 桥接线程 ---------------- */
static void *show_data_th(void *arg)
{
    struct msgdata d;
    char who[32];

    (void)arg;

    while (system_run_flag)
    {
        if (0 != recv_mail_msg(&d, who))
        {
            break;
        }
        pthread_mutex_lock(&g_latest_lock);
        g_latest = d;
        g_seq++;
        pthread_mutex_unlock(&g_latest_lock);
    }
    return NULL;
}

/* ---------------- 存储线程 ---------------- */
static void *save_data_th(void *arg)
{
    struct msgdata d;
    char who[32];

    (void)arg;

    while (system_run_flag)
    {
        if (0 != recv_mail_msg(&d, who))
        {
            break;
        }
        storage_write(&d);
    }

    storage_flush();        /* 退出前把最后一批未满 BATCH 的数据提交 */
    return NULL;
}

/* ---------------- 上报线程 ---------------- */
static void *send_data_th(void *arg)
{
    struct msgdata d;
    char who[32];

    (void)arg;

    while (system_run_flag)
    {
        if (0 != recv_mail_msg(&d, who))
        {
            break;
        }
        uplink_post(&d);
    }
    return NULL;
}

int app_start(void)
{
    Link_t *pmail;

    control_init();

    pmail = create_mailbox_system();
    if (NULL == pmail)
    {
        return -1;
    }

    if (0 != register_thread_mailbox(pmail, "get_data", get_data_th) ||
        0 != register_thread_mailbox(pmail, "ctrl", ctrl_th) ||
        0 != register_thread_mailbox(pmail, "show_data", show_data_th) ||
        0 != register_thread_mailbox(pmail, "save_data", save_data_th) ||
        0 != register_thread_mailbox(pmail, "send_data", send_data_th))
    {
        printf("[app] register thread failed\n");
        mailbox_stop();
        return -1;
    }

    g_pmail = pmail;
    printf("[app] %d thread(s) running, sample=%dms\n", pmail->clen, g_interval_ms);
    return 0;
}

void app_stop(void)
{
    mailbox_stop();
    if (NULL != g_pmail)
    {
        destroy_mailbox(g_pmail);   /* 内部 join 所有线程 */
        g_pmail = NULL;
    }
}

int app_running(void)
{
    return system_run_flag;
}

int app_new_sample(struct msgdata *out)
{
    int ret = 0;

    if (NULL == out)
    {
        return 0;
    }
    pthread_mutex_lock(&g_latest_lock);
    if (g_seq != g_consumed)
    {
        *out = g_latest;
        g_consumed = g_seq;
        ret = 1;
    }
    pthread_mutex_unlock(&g_latest_lock);
    return ret;
}

void app_set_interval(int ms)
{
    if (ms > 0)
    {
        g_interval_ms = ms;
    }
}

int app_sample_count(void)
{
    return g_samples;
}

void app_print_summary(void)
{
    char db[256];

    storage_summary(db, sizeof(db));

    /* 一行一个 key=value，既方便人看，也方便 tools/verify.sh 直接 grep 断言 */
    printf("\n================ summary ================\n");
    printf("samples=%d\n", g_samples);
    printf("%s\n", db);
    printf("relay_switches=%d\n", control_switch_count());
    printf("alarm_high=%d alarm_low=%d alarm_door=%d alarm_tilt=%d alarm_leak=%d alarm_sensor=%d\n",
           g_alarm_count[0], g_alarm_count[1], g_alarm_count[2],
           g_alarm_count[3], g_alarm_count[4], g_alarm_count[5]);
    printf("uplink sent=%d queued=%d replayed=%d fail=%d pending=%d\n",
           uplink_sent(), uplink_queued(), uplink_replayed(),
           uplink_fail(), uplink_pending());
    printf("queue_dropped=%d\n", ring_dropped());
    printf("=========================================\n");
}
