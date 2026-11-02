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

#include "../include/uplink.h"
#include "../include/ringbuf.h"
#include "../include/app_config.h"
#include "../include/mailbox.h"

#if defined(WITH_MQTT)
#include "../include/mqtt_client.h"
#endif

/*
 * 上报通道
 * --------
 * 在线：直接发，成功计数 +1
 * 离线：数据进环形队列并落盘；后台线程每 5s 探一次连接，
 *       连上后按 FIFO 逐条补传，补传成功才出队（相当于 QoS1 的「至少一次」）
 *
 * 默认编译的是模拟后端：把要发的 JSON 打到屏幕，
 * 并可用 --net-down 40:70 制造 40~70s 的断网窗口，用来亲眼验证缓存与补传。
 */

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t g_worker;
static int g_worker_started = 0;
static int g_stop = 0;

static int g_online = 0;
static int g_down_from = 0;
static int g_down_to = 0;
static time_t g_t0 = 0;

static int g_sent = 0;
static int g_queued = 0;
static int g_replayed = 0;
static int g_fail = 0;

static void sleep_ms(int ms)
{
    if (ms <= 0)
    {
        return;
    }
#if defined(_WIN32)
    Sleep((DWORD)ms);
#else
    usleep((useconds_t)ms * 1000);
#endif
}

static int elapsed_sec(void)
{
    if (0 == g_t0)
    {
        return 0;
    }
    return (int)(time(NULL) - g_t0);
}

static int in_blackout(void)
{
    int e;

    if (g_down_to <= g_down_from)
    {
        return 0;           /* 没配置断网窗口，一直在线 */
    }
    e = elapsed_sec();
    return (e >= g_down_from && e < g_down_to) ? 1 : 0;
}

static void build_json(const struct up_msg *m, char *out, int len)
{
    snprintf(out, len,
             "{\"id\":\"%lld\",\"version\":\"1.0\",\"params\":{"
             "\"temp\":{\"value\":%.2f},"
             "\"hum\":{\"value\":%.1f},"
             "\"accel\":{\"value\":%.2f},"
             "\"door\":{\"value\":%d},"
             "\"leak\":{\"value\":%d},"
             "\"alarm\":{\"value\":%u}}}",
             m->ts, m->temp, m->hum, m->accel, m->door, m->leak, m->alarm);
}

/* 真正发一条，返回 0 成功 */
static int do_publish(const struct up_msg *m, int is_replay)
{
    char json[512];

    build_json(m, json, sizeof(json));

#if defined(WITH_MQTT)
    if (0 != mqttc_publish_json(json))
    {
        return -1;
    }
    printf("[uplink] %s #%d\n", is_replay ? "replay" : "sent", m->seq);
#else
    printf("[uplink] %-6s #%-4d T=%5.2f H=%5.1f A=%4.2f D=%d L=%d ALM=0x%02X\n",
           is_replay ? "replay" : "sent", m->seq, m->temp, m->hum, m->accel,
           m->door, m->leak, m->alarm);
#endif
    return 0;
}

static int current_online(void)
{
#if defined(WITH_MQTT)
    return mqttc_is_connected();
#else
    return in_blackout() ? 0 : 1;
#endif
}

static void *uplink_worker(void *arg)
{
    (void)arg;

    while (!g_stop && system_run_flag)
    {
        int online;
        int pending;

#if defined(WITH_MQTT)
        if (!mqttc_is_connected())
        {
            if (0 == mqttc_connect())
            {
                printf("[uplink] mqtt connected\n");
            }
        }
#endif
        online = current_online();

        pthread_mutex_lock(&g_lock);
        if (online != g_online)
        {
            printf("[uplink] link %s (t=%ds)\n", online ? "UP" : "DOWN", elapsed_sec());
            g_online = online;
        }
        pending = ring_count();
        pthread_mutex_unlock(&g_lock);

        if (online && pending > 0)
        {
            printf("[uplink] replaying %d cached msg(s)\n", pending);
            while (!g_stop && system_run_flag)
            {
                struct up_msg m;
                int have;

                pthread_mutex_lock(&g_lock);
                have = ring_peek(&m);
                pthread_mutex_unlock(&g_lock);

                if (!have)
                {
                    break;
                }
                if (0 != do_publish(&m, 1))
                {
                    pthread_mutex_lock(&g_lock);
                    g_fail++;
                    g_online = 0;
                    pthread_mutex_unlock(&g_lock);
                    break;
                }

                pthread_mutex_lock(&g_lock);
                ring_drop_front();
                g_replayed++;
                pending = ring_count();
                pthread_mutex_unlock(&g_lock);
                printf("[uplink] replayed, pending=%d\n", pending);

                sleep_ms(CFG_UPLINK_FLUSH_MS);
            }
        }

        sleep_ms(online ? 200 : CFG_UPLINK_RETRY_MS);
    }
    return NULL;
}

int uplink_init(int net_down_from, int net_down_to)
{
    int loaded;

    g_down_from = net_down_from;
    g_down_to = net_down_to;
    g_t0 = time(NULL);

    if (0 != ring_init(CFG_QUEUE_SIZE, CFG_QUEUE_FILE))
    {
        printf("[uplink] ring init failed\n");
        return -1;
    }

    loaded = ring_load();
    if (loaded > 0)
    {
        printf("[uplink] recovered %d cached msg(s) from %s\n", loaded, CFG_QUEUE_FILE);
    }
    else if (loaded == 0)
    {
        printf("[uplink] queue file empty\n");
    }

    g_online = current_online();

    g_stop = 0;
    if (0 != pthread_create(&g_worker, NULL, uplink_worker, NULL))
    {
        printf("[uplink] worker create failed\n");
        return -1;
    }
    g_worker_started = 1;
    return 0;
}

int uplink_post(const struct msgdata *d)
{
    struct up_msg m;
    int online;
    time_t ts;

    if (NULL == d)
    {
        return -1;
    }

    memset(&m, 0, sizeof(m));
    ts = mktime((struct tm *)&d->tim);
    if ((time_t)-1 == ts || ts <= 0)
    {
        ts = time(NULL);
    }
    m.ts = (long long)ts;
    m.seq = d->cnt;
    m.temp = d->temp;
    m.hum = d->hum;
    m.accel = d->a;
    m.door = d->door;
    m.leak = d->leak;
    m.heater = d->heater;
    m.cooler = d->cooler;
    m.alarm = d->alarm;
    m.valid = 1;

    /* 先在锁外判断链路，避免把阻塞的上报放进临界区 */
    online = current_online();

    if (online && 0 == do_publish(&m, 0))
    {
        pthread_mutex_lock(&g_lock);
        g_sent++;
        g_online = 1;
        pthread_mutex_unlock(&g_lock);
        return 0;
    }

    pthread_mutex_lock(&g_lock);
    ring_push(&m);
    g_queued++;
    g_online = online;
    pthread_mutex_unlock(&g_lock);

    printf("[uplink] offline, cached #%d (pending=%d)\n", m.seq, ring_count());
    return 1;
}

void uplink_shutdown(void)
{
    g_stop = 1;
    if (g_worker_started)
    {
        pthread_join(g_worker, NULL);
        g_worker_started = 0;
    }
    ring_store();           /* 把剩余数据和计数落盘 */
    ring_free();
}

int uplink_online(void)
{
    return g_online;
}

int uplink_sent(void)
{
    return g_sent;
}

int uplink_queued(void)
{
    return g_queued;
}

int uplink_replayed(void)
{
    return g_replayed;
}

int uplink_pending(void)
{
    return ring_count();
}

int uplink_fail(void)
{
    return g_fail;
}
