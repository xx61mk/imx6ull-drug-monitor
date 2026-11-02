#ifndef UPLINK_H
#define UPLINK_H

#include "linkqueue.h"

/*
 * 上报通道：在线直发，离线进环形队列，重连后按序补传。
 *
 * 两种后端：
 *   默认        模拟后端，把 JSON 打到屏幕，可用 --net-down a:b 制造断网窗口，
 *               用于验证「断线缓存 + 重连补传」这条逻辑。
 *   WITH_MQTT   真实 MQTT（Paho），连云平台。凭据只从环境变量读，不写死在源码里。
 */

int  uplink_init(int net_down_from, int net_down_to);
int  uplink_post(const struct msgdata *d);     /* 0 已发送 / 1 已入队等待补传 */
void uplink_shutdown(void);

int  uplink_online(void);
int  uplink_sent(void);          /* 直发成功条数 */
int  uplink_queued(void);        /* 入队条数 */
int  uplink_replayed(void);      /* 补传成功条数 */
int  uplink_pending(void);       /* 队列剩余 */
int  uplink_fail(void);          /* 发送失败次数 */

#endif
