#ifndef APP_H
#define APP_H

#include "linkqueue.h"

/*
 * 线程编排层：把采集 / 控制 / 显示 / 存储 / 上报五个线程拉起来，
 * 线程之间只通过邮箱通信，不共享可变状态。
 *
 * 无 UI 的终端版和 LVGL 版共用这一层：
 *   终端版  while (running) { sleep; if (app_new_sample(&d)) print; }
 *   LVGL 版 while (running) { lv_timer_handler(); if (app_new_sample(&d)) ui_update_data(...); }
 */

int  app_start(void);
void app_stop(void);
int  app_running(void);

/* 采样周期（命令行 --interval 覆盖默认值），必须在 app_start 之前调用 */
void app_set_interval(int ms);
int  app_sample_count(void);

/* UI 桥：有未消费的新样本时返回 1 并填充 out，否则返回 0。
 * 内部用 mutex 保护，只在调用线程（主线程）里碰 UI。 */
int  app_new_sample(struct msgdata *out);

/* 退出时打印统计：样本数 / 报警计数 / 数据库 / 上报队列 */
void app_print_summary(void);

#endif
