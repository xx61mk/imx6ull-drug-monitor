#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(__linux__)
#include <dirent.h>
#endif

#include "../include/sensor.h"
#include "../include/app_config.h"

/*
 * 采集层
 * ------
 * 板端只读 sysfs 节点，不把 I2C/SPI 时序耦合进业务代码：
 *   DS18B20  -> /sys/bus/w1/devices/28-xxxx/w1_slave   （内核 1-Wire 子系统保证时序）
 *   LM75     -> /sys/class/hwmon/hwmonX/temp1_input    （i2c 驱动已导出 hwmon）
 *   ADXL345  -> /sys/bus/iio/devices/iio:device0/in_accel_*  （IIO 子系统）
 *   门磁/泄漏 -> /sys/class/gpio/gpioN/value
 * 探测不到设备时自动切到仿真后端，所以同一份代码在没有板子的机器上也能跑通全流程。
 */

static struct sensor_cfg g_cfg;
static int  g_backend = SENSOR_SIM;
static int  g_act_heater = 0;
static int  g_act_cooler = 0;
static int  g_act_dt_ms = 0;         /* 每次采样推进的仿真时间 */
static float g_last_temp = 22.0f;
static char g_detail[256] = "sim";

/* ---------------- 板端文件读取小工具 ---------------- */

static int read_line(const char *path, char *buf, int len)
{
    FILE *f = fopen(path, "r");
    if (NULL == f)
    {
        return -1;
    }
    if (NULL == fgets(buf, len, f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

static int write_value(const char *path, int v)
{
    FILE *f = fopen(path, "w");
    if (NULL == f)
    {
        return -1;
    }
    fputc(v ? '1' : '0', f);
    fclose(f);
    return 0;
}

static int gpio_read(const char *path, int *v)
{
    char buf[16];
    if (0 != read_line(path, buf, sizeof(buf)))
    {
        return -1;
    }
    *v = (buf[0] == '1') ? 1 : 0;
    return 0;
}

/* ---------------- DS18B20 ---------------- */

static char g_w1_slave[256] = {0};

static int probe_w1(void)
{
#if defined(__linux__)
    DIR *d;
    struct dirent *e;

    d = opendir(CFG_W1_DEVICE_DIR);
    if (NULL == d)
    {
        return -1;
    }
    while ((e = readdir(d)) != NULL)
    {
        if (0 == strncmp(e->d_name, "28-", 3))
        {
            snprintf(g_w1_slave, sizeof(g_w1_slave), "%s/%s/%s",
                     CFG_W1_DEVICE_DIR, e->d_name, CFG_W1_SLAVE_FILE);
            closedir(d);
            return 0;
        }
    }
    closedir(d);
#endif
    return -1;
}

static int read_ds18b20(float *out)
{
    char buf[256];
    char *p;

    if ('\0' == g_w1_slave[0])
    {
        return -1;
    }

    {
        FILE *f = fopen(g_w1_slave, "r");
        if (NULL == f)
        {
            return -1;
        }
        /* 第一行末尾必须是 YES，否则 CRC 校验失败，数据不可信 */
        if (NULL == fgets(buf, sizeof(buf), f) || NULL == strstr(buf, "YES"))
        {
            fclose(f);
            return -1;
        }
        if (NULL == fgets(buf, sizeof(buf), f))
        {
            fclose(f);
            return -1;
        }
        fclose(f);
    }

    p = strstr(buf, "t=");
    if (NULL == p)
    {
        return -1;
    }
    *out = (float)atoi(p + 2) / 1000.0f;

    /* 85℃ 是上电默认值，说明上一次转换还没完成，丢掉重读下一轮 */
    if (*out > 84.9f && *out < 85.1f)
    {
        return -1;
    }
    return 0;
}

/* ---------------- LM75 (hwmon) ---------------- */

static int read_hwmon(float *out)
{
    char buf[64];
    if (0 != read_line(CFG_HWMON_TEMP, buf, sizeof(buf)))
    {
        return -1;
    }
    *out = (float)atoi(buf) / 1000.0f;
    return 0;
}

/* ---------------- ADXL345 (IIO) ---------------- */

static int read_iio_accel(float *g)
{
    char path[256];
    char buf[64];
    float scale = 0.0039f;      /* ±16g 量程默认 3.9mg/LSB，驱动若提供 in_accel_scale 则以它为准 */
    long rx = 0;
    long ry = 0;
    long rz = 0;

    snprintf(path, sizeof(path), "%s/in_accel_scale", CFG_IIO_DIR);
    if (0 == read_line(path, buf, sizeof(buf)))
    {
        scale = (float)atof(buf);
    }

    snprintf(path, sizeof(path), "%s/in_accel_x_raw", CFG_IIO_DIR);
    if (0 != read_line(path, buf, sizeof(buf)))
    {
        return -1;
    }
    rx = strtol(buf, NULL, 10);

    snprintf(path, sizeof(path), "%s/in_accel_y_raw", CFG_IIO_DIR);
    if (0 != read_line(path, buf, sizeof(buf)))
    {
        return -1;
    }
    ry = strtol(buf, NULL, 10);

    snprintf(path, sizeof(path), "%s/in_accel_z_raw", CFG_IIO_DIR);
    if (0 != read_line(path, buf, sizeof(buf)))
    {
        return -1;
    }
    rz = strtol(buf, NULL, 10);

    *g = (float)sqrt((double)(rx * rx + ry * ry + rz * rz)) * scale;
    return 0;
}

/* ---------------- 仿真后端的物理模型 ---------------- */

struct sim_state
{
    float temp;         /* 箱内温度 ℃ */
    float amb;          /* 环境温度 ℃ */
    float hum;
    int   heater;
    int   cooler;
    int   door;
    int   leak;
    int   tilt;
    int   cool_fault;   /* 制冷失效，用来演示超温报警 */
    int   elapsed_ms;
};

static struct sim_state g_sim;

static void sim_reset(void)
{
    memset(&g_sim, 0, sizeof(g_sim));
    g_sim.temp = 25.0f;     /* 上电时箱内=环境，需要先制冷降到 2~8℃ */
    g_sim.amb = 25.0f;
    g_sim.hum = 45.0f;
}

static void sim_scenario(int t_sec)
{
    if (SCENARIO_DEMO != g_cfg.scenario)
    {
        return;
    }

    /* 0~40s   正常降温到设定区
     * 40~125s 门被打开（触发开门超时，同时温度被环境拉高）
     * 120~140s 搬运碰撞（加速度模值跳变）
     * 160s~   液体泄漏
     * 200~280s 制冷失效（温度冲高，触发超温报警） */
    g_sim.door = (t_sec >= 40 && t_sec < 125) ? 1 : 0;
    g_sim.tilt = (t_sec >= 120 && t_sec < 140) ? 1 : 0;
    g_sim.leak = (t_sec >= 160) ? 1 : 0;
    g_sim.cool_fault = (t_sec >= 200 && t_sec < 280) ? 1 : 0;
}

static void sim_advance(int dt_ms)
{
    float dt = (float)dt_ms / 1000.0f;
    float k_amb = 0.012f;               /* 箱体自然回温系数 1/s */
    float k_door = 0.045f;              /* 开门时与环境耦合更强 */
    float rate_heat = 0.35f;            /* 加热能力 ℃/s */
    float rate_cool = 0.50f;            /* 制冷能力 ℃/s */
    float dT;
    float noise;

    g_sim.elapsed_ms += dt_ms;

    /* 环境温度缓慢波动，让曲线不是一条直线 */
    g_sim.amb = 25.0f + 1.5f * (float)sin((double)g_sim.elapsed_ms / 37000.0);
    sim_scenario(g_sim.elapsed_ms / 1000);

    dT = (g_sim.door ? k_door : k_amb) * (g_sim.amb - g_sim.temp) * dt;
    if (g_sim.heater)
    {
        dT += rate_heat * dt;
    }
    if (g_sim.cooler && 0 == g_sim.cool_fault)
    {
        dT -= rate_cool * dt;
    }

    /* 小噪声：±0.05℃，模拟真实读数抖动，用来验证滤波和回差是否够 */
    noise = ((float)(rand() % 100) - 50.0f) / 1000.0f;
    g_sim.temp += dT + noise;

    if (g_sim.temp < -10.0f)
    {
        g_sim.temp = -10.0f;
    }
    if (g_sim.temp > 60.0f)
    {
        g_sim.temp = 60.0f;
    }

    /* 湿度：开门时向环境湿度靠，关门后缓慢回落 */
    if (g_sim.door)
    {
        g_sim.hum += (60.0f - g_sim.hum) * 0.05f * dt;
    }
    else
    {
        g_sim.hum += (45.0f - g_sim.hum) * 0.01f * dt;
    }
}

/* ---------------- 对外接口 ---------------- */

int sensor_init(const struct sensor_cfg *cfg)
{
    memset(&g_cfg, 0, sizeof(g_cfg));
    if (cfg != NULL)
    {
        g_cfg = *cfg;
    }
    if (g_cfg.interval_ms <= 0)
    {
        g_cfg.interval_ms = CFG_SAMPLE_MS;
    }
    if (g_cfg.speed <= 0)
    {
        g_cfg.speed = 1;
    }
    g_act_dt_ms = g_cfg.interval_ms * g_cfg.speed;

    if (SENSOR_SIM == g_cfg.mode)
    {
        g_backend = SENSOR_SIM;
        sim_reset();
        snprintf(g_detail, sizeof(g_detail), "thermal model, scenario=%s",
                 (SCENARIO_DEMO == g_cfg.scenario) ? "demo" : "none");
        return 0;
    }

    /* 探测板端设备 */
    {
        int have_w1 = (0 == probe_w1());
        int have_hwmon = (0 == read_hwmon(&g_last_temp));
        int have_accel = 0;
        float gtest = 0.0f;

        if (0 == read_iio_accel(&gtest))
        {
            have_accel = 1;
        }

        if (have_w1 || have_hwmon)
        {
            g_backend = SENSOR_BOARD;
            snprintf(g_detail, sizeof(g_detail), "w1=%s hwmon=%s accel=%s",
                     have_w1 ? "yes" : "no",
                     have_hwmon ? "yes" : "no",
                     have_accel ? "yes" : "no");
            return 0;
        }

        if (SENSOR_BOARD == g_cfg.mode)
        {
            /* 强制板端但什么都没找到：保留 board 后端，读失败会上报 ALARM_SENSOR */
            g_backend = SENSOR_BOARD;
            snprintf(g_detail, sizeof(g_detail), "board mode but no device found");
            return 0;
        }
    }

    g_backend = SENSOR_SIM;
    sim_reset();
    snprintf(g_detail, sizeof(g_detail), "auto->sim (no board device), scenario=%s",
             (SCENARIO_DEMO == g_cfg.scenario) ? "demo" : "none");
    return 0;
}

int sensor_read(struct msgdata *out)
{
    float temp = g_last_temp;
    float accel = 1.0f;
    int   door = 0;
    int   leak = 0;
    int   temp_ok = 0;
    unsigned int alarm = 0;

    if (NULL == out)
    {
        return -1;
    }

    if (SENSOR_SIM == g_backend)
    {
        sim_advance(g_act_dt_ms);
        temp = g_sim.temp;
        temp_ok = 1;

        /* 倾倒时加速度模值不再等于 1g */
        accel = g_sim.tilt ? 1.62f : 1.0f;
        door = g_sim.door;
        leak = g_sim.leak;

        out->hum = g_sim.hum;
        out->v = 5.02f + ((float)(rand() % 40) - 20.0f) / 1000.0f;
    }
    else
    {
        if (0 == read_ds18b20(&temp))
        {
            temp_ok = 1;
        }
        else if (0 == read_hwmon(&temp))
        {
            temp_ok = 1;
        }

        if (0 == read_iio_accel(&accel))
        {
            /* 读到就用 */
        }
        else
        {
            accel = 1.0f;       /* 没有加速度计就当作静止 */
        }

        if (0 != gpio_read(CFG_GPIO_DOOR, &door))
        {
            door = 0;
        }
        if (0 != gpio_read(CFG_GPIO_LEAK, &leak))
        {
            leak = 0;
        }

        out->hum = -1.0f;       /* 本机未接湿度传感器 */
        out->v = -1.0f;
    }

    if (temp_ok)
    {
        g_last_temp = temp;
    }
    else
    {
        /* 读不到温度就用上一次的值，同时置传感器故障位，业务层会看到并报警 */
        temp = g_last_temp;
        alarm |= ALARM_SENSOR;
    }

    out->temp = temp;
    out->a = accel;
    out->door = door;
    out->leak = leak;
    out->heater = g_act_heater;
    out->cooler = g_act_cooler;
    out->alarm = alarm;
    return 0;
}

void sensor_actuator(int heater, int cooler)
{
    g_act_heater = heater ? 1 : 0;
    g_act_cooler = cooler ? 1 : 0;

    if (SENSOR_SIM == g_backend)
    {
        g_sim.heater = g_act_heater;
        g_sim.cooler = g_act_cooler;
        return;
    }

    write_value(CFG_GPIO_HEATER, g_act_heater);
    write_value(CFG_GPIO_COOLER, g_act_cooler);
}

const char *sensor_backend_name(void)
{
    return (SENSOR_BOARD == g_backend) ? "board" : "sim";
}

const char *sensor_backend_detail(void)
{
    return g_detail;
}

void sensor_deinit(void)
{
    sensor_actuator(0, 0);
}
