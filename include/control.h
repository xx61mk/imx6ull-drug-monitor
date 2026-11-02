#ifndef CONTROL_H
#define CONTROL_H

#include "linkqueue.h"

struct ctrl_output
{
    int heater;             /* 1 表示本周期应开启加热 */
    int cooler;             /* 1 表示本周期应开启制冷 */
    unsigned int alarm;     /* 当前报警位图 */
    unsigned int new_alarm; /* 本周期新出现的报警位（用于只打印一次） */
};

int  control_init(void);
int  control_step(const struct msgdata *in, struct ctrl_output *out);
int  control_switch_count(void);        /* 继电器动作次数，退出时打印 */

/* 控制决策的虚拟时钟步长（= 采样周期 × 仿真倍数）。
 * 真机传 CFG_SAMPLE_MS；仿真加速时传 interval*speed，
 * 这样加速跑出的控制行为与真机一致。 */
void control_set_step(int step_ms);

#endif
