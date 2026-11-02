#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "../include/app.h"
#include "../include/app_config.h"
#include "../include/mailbox.h"
#include "../include/sensor.h"
#include "../include/storage.h"
#include "../include/uplink.h"
#include "../include/control.h"

/*
 * 终端版入口：把整条链路（采集 → 控温 → 存储 → 上报）跑起来，
 * 不依赖 LVGL / 帧缓冲，所以 PC 上直接就能验证逻辑。
 * 板端带界面的版本见 lvgl_app/main_lvgl.c，两者共用 app.c 这一层。
 */

static void on_signal(int sig)
{
    (void)sig;
    system_run_flag = 0;
}

static void msleep(int ms)
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

static void usage(const char *prog)
{
    printf("usage: %s [options]\n", prog);
    printf("\n");
    printf("  --sim                force software simulation (default: auto)\n");
    printf("  --board              force board mode, read sysfs even if nothing found\n");
    printf("  --auto               probe board devices first, fall back to sim\n");
    printf("  --scenario demo|none inject faults: door / tilt / leak / cooling failure\n");
    printf("  --interval MS        sample period, default %d ms\n", CFG_SAMPLE_MS);
    printf("  --speed N            simulation time multiplier, default 1\n");
    printf("  --seconds N          run for N seconds then exit (0 = until Ctrl+C)\n");
    printf("  --db PATH            sqlite file, default %s\n", CFG_DB_PATH);
    printf("  --retention DAYS     keep how many days, default %d\n", CFG_DB_RETENTION_DAYS);
    printf("  --net-down A:B       simulate link outage between second A and B\n");
    printf("  --dump N             print last N stored rows before exit\n");
    printf("  --help               this message\n");
    printf("\n");
    printf("examples:\n");
    printf("  %s --sim --scenario demo --seconds 300 --dump 10\n", prog);
    printf("  %s --sim --scenario none --seconds 120 --net-down 30:60\n", prog);
    printf("  %s --board --seconds 0            # run on i.MX6ULL\n", prog);
}

int main(int argc, char **argv)
{
    struct sensor_cfg scfg;
    const char *dbpath = CFG_DB_PATH;
    int mode = SENSOR_AUTO;
    int scenario = SCENARIO_DEMO;
    int interval = CFG_SAMPLE_MS;
    int speed = 1;
    int seconds = 0;
    int retention = CFG_DB_RETENTION_DAYS;
    int down_from = 0;
    int down_to = 0;
    int dump = 0;
    int i;
    time_t t0;

    for (i = 1; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "--help") || 0 == strcmp(argv[i], "-h"))
        {
            usage(argv[0]);
            return 0;
        }
        else if (0 == strcmp(argv[i], "--sim"))
        {
            mode = SENSOR_SIM;
        }
        else if (0 == strcmp(argv[i], "--board"))
        {
            mode = SENSOR_BOARD;
        }
        else if (0 == strcmp(argv[i], "--auto"))
        {
            mode = SENSOR_AUTO;
        }
        else if (0 == strcmp(argv[i], "--scenario") && i + 1 < argc)
        {
            i++;
            scenario = (0 == strcmp(argv[i], "demo")) ? SCENARIO_DEMO : SCENARIO_NONE;
        }
        else if (0 == strcmp(argv[i], "--interval") && i + 1 < argc)
        {
            i++;
            interval = atoi(argv[i]);
        }
        else if (0 == strcmp(argv[i], "--speed") && i + 1 < argc)
        {
            i++;
            speed = atoi(argv[i]);
        }
        else if (0 == strcmp(argv[i], "--seconds") && i + 1 < argc)
        {
            i++;
            seconds = atoi(argv[i]);
        }
        else if (0 == strcmp(argv[i], "--db") && i + 1 < argc)
        {
            i++;
            dbpath = argv[i];
        }
        else if (0 == strcmp(argv[i], "--retention") && i + 1 < argc)
        {
            i++;
            retention = atoi(argv[i]);
        }
        else if (0 == strcmp(argv[i], "--net-down") && i + 1 < argc)
        {
            i++;
            if (2 != sscanf(argv[i], "%d:%d", &down_from, &down_to))
            {
                printf("bad --net-down format, expect A:B\n");
                return 1;
            }
        }
        else if (0 == strcmp(argv[i], "--dump") && i + 1 < argc)
        {
            i++;
            dump = atoi(argv[i]);
        }
        else
        {
            printf("unknown option: %s (try --help)\n", argv[i]);
            return 1;
        }
    }

    if (interval < 100)
    {
        interval = 100;         /* 太快没意义，还会把控制回差压掉 */
    }
    if (speed < 1)
    {
        speed = 1;
    }
    if (speed > 30)
    {
        speed = 30;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
#if defined(SIGPIPE)
    signal(SIGPIPE, SIG_IGN);   /* 云端断开时写 socket 不要直接杀进程 */
#endif

    printf("========================================================\n");
    printf(" drug cold-chain monitor\n");
    printf("--------------------------------------------------------\n");

    memset(&scfg, 0, sizeof(scfg));
    scfg.mode = mode;
    scfg.scenario = scenario;
    scfg.interval_ms = interval;
    scfg.speed = speed;

    if (0 != sensor_init(&scfg))
    {
        printf("[main] sensor init failed\n");
        return 1;
    }

    printf(" backend   : %s (%s)\n", sensor_backend_name(), sensor_backend_detail());
    printf(" sample    : %d ms\n", interval);
    if (speed > 1)
    {
        printf(" speed     : x%d (1 real second = %d simulated seconds)\n", speed, speed);
    }
    printf(" control   : cool ON >= %.1fC / OFF <= %.1fC\n", CFG_TEMP_HIGH, CFG_TEMP_COOL_OFF);
    printf("             heat ON <= %.1fC / OFF >= %.1fC\n", CFG_TEMP_LOW, CFG_TEMP_HEAT_OFF);
    printf(" alarm     : temp %dms | door %dms | tilt %dms | leak instant\n",
           CFG_TEMP_ALARM_DELAY_MS, CFG_DOOR_ALARM_MS, CFG_TILT_ALARM_MS);
    printf(" storage   : %s (retention %d day)\n", dbpath, retention);
    if (down_to > down_from)
    {
        printf(" uplink    : sim, outage window %ds~%ds\n", down_from, down_to);
    }
    else
    {
        printf(" uplink    : sim (always online)\n");
    }
    printf("--------------------------------------------------------\n");

    if (0 != storage_open(dbpath, retention))
    {
        printf("[main] storage unavailable, continue without local db\n");
    }
    if (0 != uplink_init(down_from, down_to))
    {
        printf("[main] uplink init failed\n");
    }

    app_set_interval(interval);
    control_set_step(interval * speed);   /* 虚拟时钟与仿真推进速度对齐 */
    if (0 != app_start())
    {
        printf("[main] app start failed\n");
        storage_close();
        uplink_shutdown();
        return 1;
    }

    printf("[main] running, Ctrl+C to stop\n\n");

    t0 = time(NULL);
    while (system_run_flag)
    {
        struct msgdata d;

        if (seconds > 0 && (int)(time(NULL) - t0) >= seconds)
        {
            break;
        }
        msleep(200);

        /* 终端版不画界面，但把桥接路径走一遍，保证 UI 版复用的是同一段代码 */
        (void)app_new_sample(&d);
    }

    printf("\n[main] stopping...\n");
    app_stop();
    sensor_deinit();
    app_print_summary();

    if (dump > 0)
    {
        storage_dump_recent(dump);
    }

    storage_close();
    uplink_shutdown();

    printf("[main] bye\n");
    return 0;
}
