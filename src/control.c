#include <stdio.h>
#include <string.h>
#include "../include/control.h"
#include "../include/app_config.h"

/*
 * 控制与报警
 * ----------
 * 控温：带回差的开关控制，不用 PID。药品箱热惯性大，回差足够稳，
 *       而且继电器最怕频繁动作，所以再加一个最小切换间隔。
 *     制冷：>=8.0℃ 启动，<=6.5℃ 停止
 *     加热：<=2.0℃ 启动，>=3.5℃ 停止
 *
 * 报警：温度越限要持续 3s 才报（滤掉开门瞬间的抖动），
 *       开门 60s、碰撞 2s 才报，泄漏立即报。
 *       每条报警只在「刚出现」的那一拍返回 new_alarm，避免刷屏。
 *
 * 时间基准用「虚拟时钟」：每来一个样本前进一个采样周期。
 * 真机 1s 采样时虚拟时钟就等于实际时间；仿真加速时按倍数前进，
 * 因此加速跑出来的控制行为与真机一致，而不是把延时按比例压缩那种近似。
 */

static struct
{
    int heater;
    int cooler;
    long long last_switch_ms;
    long long temp_since;
    long long door_since;
    long long tilt_since;
    unsigned int alarm;
    int switches;
} g_ctrl;

static long long g_vt_ms = 0;       /* 虚拟时钟 */
static long long g_vt_start = 0;    /* 上电时刻，用于抑制期判断 */
static int g_step_ms = CFG_SAMPLE_MS;

int control_init(void)
{
    memset(&g_ctrl, 0, sizeof(g_ctrl));
    g_vt_ms = 1000000;              /* 从正的初值起步，第一次决策不受最小间隔限制 */
    g_vt_start = g_vt_ms;
    return 0;
}

int control_switch_count(void)
{
    return g_ctrl.switches;
}

void control_set_step(int step_ms)
{
    if (step_ms > 0)
    {
        g_step_ms = step_ms;
    }
}

int control_step(const struct msgdata *in, struct ctrl_output *out)
{
    long long t;
    unsigned int al = 0;
    int want_heater;
    int want_cooler;

    if (NULL == in || NULL == out)
    {
        return -1;
    }

    t = g_vt_ms;
    g_vt_ms += g_step_ms;

    /* ---------- 1. 报警判定 ---------- */

    if (0 != (in->alarm & ALARM_SENSOR))
    {
        al |= ALARM_SENSOR;
    }
    if (in->leak)
    {
        al |= ALARM_LEAK;
    }

    /* 温度越限：持续 CFG_TEMP_ALARM_DELAY_MS 才确认 */
    if (in->temp >= CFG_TEMP_HIGH || in->temp <= CFG_TEMP_LOW)
    {
        if (0 == g_ctrl.temp_since)
        {
            g_ctrl.temp_since = t;
        }
        else if (t - g_ctrl.temp_since >= CFG_TEMP_ALARM_DELAY_MS)
        {
            al |= (in->temp >= CFG_TEMP_HIGH) ? ALARM_TEMP_HIGH : ALARM_TEMP_LOW;
        }
    }
    else
    {
        g_ctrl.temp_since = 0;
    }

    /* 开门超时 */
    if (in->door)
    {
        if (0 == g_ctrl.door_since)
        {
            g_ctrl.door_since = t;
        }
        else if (t - g_ctrl.door_since >= CFG_DOOR_ALARM_MS)
        {
            al |= ALARM_DOOR;
        }
    }
    else
    {
        g_ctrl.door_since = 0;
    }

    /* 碰撞 / 倾倒：加速度模值偏离 1g */
    {
        float dev = in->a - 1.0f;
        if (dev < 0.0f)
        {
            dev = -dev;
        }
        if (dev > CFG_TILT_THRESHOLD_G)
        {
            if (0 == g_ctrl.tilt_since)
            {
                g_ctrl.tilt_since = t;
            }
            else if (t - g_ctrl.tilt_since >= CFG_TILT_ALARM_MS)
            {
                al |= ALARM_TILT;
            }
        }
        else
        {
            g_ctrl.tilt_since = 0;
        }
    }

    /* 上电抑制期：刚上电箱内还是室温，预冷到设定值之前的超温不算故障。
     * 计时不清零，抑制期一结束如果仍然越限，下一拍就会报出来。 */
    if ((t - g_vt_start) < CFG_ALARM_WARMUP_MS)
    {
        al &= ~(ALARM_TEMP_HIGH | ALARM_TEMP_LOW);
    }

    out->new_alarm = al & ~g_ctrl.alarm;
    g_ctrl.alarm = al;

    /* ---------- 2. 执行器决策 ---------- */

    want_cooler = g_ctrl.cooler;
    want_heater = g_ctrl.heater;

    if (in->temp >= CFG_TEMP_HIGH)
    {
        want_cooler = 1;
    }
    else if (in->temp <= CFG_TEMP_COOL_OFF)
    {
        want_cooler = 0;
    }

    if (in->temp <= CFG_TEMP_LOW)
    {
        want_heater = 1;
    }
    else if (in->temp >= CFG_TEMP_HEAT_OFF)
    {
        want_heater = 0;
    }

    /* 互斥：不允许同时加热和制冷，按当前温度偏向哪边决定 */
    if (want_heater && want_cooler)
    {
        if (in->temp >= (CFG_TEMP_HIGH + CFG_TEMP_LOW) / 2.0f)
        {
            want_heater = 0;
        }
        else
        {
            want_cooler = 0;
        }
    }

    /* 最小切换间隔：保护继电器。但温度严重越界时优先保药品，直接动作。 */
    {
        int changing = (want_heater != g_ctrl.heater) || (want_cooler != g_ctrl.cooler);
        int critical = (in->temp > CFG_TEMP_HIGH + 1.5f) || (in->temp < CFG_TEMP_LOW - 1.5f);

        if (changing && !critical && (t - g_ctrl.last_switch_ms) < CFG_ACTUATOR_MIN_GAP_MS)
        {
            want_heater = g_ctrl.heater;
            want_cooler = g_ctrl.cooler;
        }

        if (want_heater != g_ctrl.heater || want_cooler != g_ctrl.cooler)
        {
            g_ctrl.heater = want_heater;
            g_ctrl.cooler = want_cooler;
            g_ctrl.last_switch_ms = t;
            g_ctrl.switches++;
        }
    }

    out->heater = g_ctrl.heater;
    out->cooler = g_ctrl.cooler;
    out->alarm = al;
    return 0;
}
