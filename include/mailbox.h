#ifndef MAILBOX_H
#define MAILBOX_H

#include "link.h"

/* 全局运行标志：置 0 后所有邮箱线程退出（定义在 mailbox.c） */
extern volatile int system_run_flag;

Link_t *create_mailbox_system(void);
int  register_thread_mailbox(Link_t *pmail, const char *name, void *(*pfun)(void *));
int  send_mail_msg(const char *send_name, struct msgdata mydata, const char *recv_name);

/* 阻塞收件：内部每 200ms 醒一次检查 system_run_flag，线程可安全退出 */
int  recv_mail_msg(struct msgdata *mydata, char *send_name);

/* 带超时收件：超时返回 -2，退出标志置位返回 -1 */
int  recv_mail_msg_timeout(struct msgdata *mydata, char *send_name, int timeout_ms);

/* 置退出标志并唤醒全部等待线程，之后 destroy_mailbox 才能 join 成功 */
void mailbox_stop(void);

void destroy_mailbox(Link_t *pmail);

#endif
