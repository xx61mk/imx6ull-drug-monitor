#ifndef LINKQUEUE_H
#define LINKQUEUE_H

#include <time.h>

/* 报警位图 */
#define ALARM_TEMP_HIGH   0x0001u   /* 高于 8℃ */
#define ALARM_TEMP_LOW    0x0002u   /* 低于 2℃ */
#define ALARM_DOOR        0x0004u   /* 开门超时 */
#define ALARM_TILT        0x0008u   /* 碰撞 / 倾倒 */
#define ALARM_LEAK        0x0010u   /* 液体泄漏 */
#define ALARM_SENSOR      0x0020u   /* 传感器读取异常 */

/* 一次采集的完整快照，也是邮箱里传的消息体 */
struct msgdata
{
    int id;                 /* 设备号 */
    int cnt;                /* 采样序号，从 0 递增 */
    float temp;             /* 药品区温度 ℃ */
    float hum;              /* 箱内湿度 %RH */
    float v;                /* 供电电压 V */
    float a;                /* 加速度模值 g，静止时约 1.0 */
    int door;               /* 0 关闭 / 1 打开 */
    int leak;               /* 0 正常 / 1 泄漏 */
    int heater;             /* 0 停 / 1 加热中 */
    int cooler;             /* 0 停 / 1 制冷中 */
    unsigned int alarm;     /* 当前报警位图 */
    struct tm tim;          /* 采集时刻 */
};

/* 邮箱中传递的整条消息：谁发的、发给谁、内容 */
struct msg
{
    char send_name[32];
    struct msgdata data;
    char recv_name[32];
};

typedef struct msg Data_t;

typedef struct qnode
{
    Data_t data;
    struct qnode *pnext;
} QNode_t;

typedef struct lqueue
{
    QNode_t *phead;
    QNode_t *ptail;
    int clen;
} LQue_t;

extern LQue_t *create_link_queue(void);
extern int push_link_queue(LQue_t *pque, Data_t data);
extern int pop_link_queue(LQue_t *pque, Data_t *pdata);
extern int get_link_queue_head(LQue_t *pque, Data_t *pdata);
extern void show_link_queue(LQue_t *pque);
extern void destroy_link_queue(LQue_t *pque);

#endif
