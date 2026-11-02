#ifndef RINGBUF_H
#define RINGBUF_H

/* 离线环形队列：断网时缓存待上报数据，重连后按序补传。
 * 满时丢最旧一条并累加 dropped，保证最新数据一定在。
 * 支持落盘（uplink_queue.bin），断电重启后仍能补传。 */

struct up_msg
{
    long long ts;           /* 采集时间戳（epoch 秒） */
    int  seq;               /* 采样序号 */
    float temp;
    float hum;
    float accel;
    int  door;
    int  leak;
    int  heater;
    int  cooler;
    unsigned int alarm;
    int  valid;             /* 1 表示该槽有效（落盘时用） */
};

int  ring_init(int capacity, const char *path);
void ring_free(void);
int  ring_push(const struct up_msg *m);
int  ring_pop(struct up_msg *m);            /* 1 取到，0 空 */
int  ring_peek(struct up_msg *m);           /* 只看队首不取出，1 有数据 */
void ring_drop_front(void);                 /* 确认发送成功后丢弃队首 */
int  ring_count(void);
int  ring_dropped(void);
int  ring_saved(void);                      /* 落盘成功次数 */
int  ring_load(void);                       /* 启动时从文件恢复 */
int  ring_store(void);                      /* 手动落盘 */

#endif
