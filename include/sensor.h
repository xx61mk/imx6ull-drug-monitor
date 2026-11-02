#ifndef SENSOR_H
#define SENSOR_H

#include "linkqueue.h"

/* 采集后端 */
enum
{
    SENSOR_AUTO = 0,    /* 先探测板端设备，探测不到自动转仿真 */
    SENSOR_BOARD,       /* 强制板端：只读 sysfs/hwmon/IIO，读不到置 ALARM_SENSOR */
    SENSOR_SIM          /* 强制仿真：热模型 + 故障注入，脱离硬件也能验证逻辑 */
};

/* 仿真场景 */
enum
{
    SCENARIO_NONE = 0,  /* 只跑稳态控温，观察滞回 */
    SCENARIO_DEMO       /* 依次注入 开门/倾倒/泄漏/制冷失效 四类故障 */
};

struct sensor_cfg
{
    int mode;           /* SENSOR_* */
    int scenario;       /* SCENARIO_* */
    int interval_ms;    /* 采样周期 */
    int speed;          /* 仿真加速倍数：每次采样推进 interval*speed 的物理时间，便于快速验证 */
};

int  sensor_init(const struct sensor_cfg *cfg);
int  sensor_read(struct msgdata *out);          /* 0 成功，-1 无有效数据 */
void sensor_actuator(int heater, int cooler);   /* 下发执行器状态：板端写 GPIO，仿真更新热模型 */
void sensor_deinit(void);

const char *sensor_backend_name(void);          /* "board" / "sim" */
const char *sensor_backend_detail(void);        /* 具体用了哪些设备节点 */

#endif
