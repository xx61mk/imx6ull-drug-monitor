#ifndef LINK_H
#define LINK_H

#include <pthread.h>
#include <semaphore.h>
#include "linkqueue.h"

/* 一个线程节点：名字、tid、入口函数，以及它专属的收件队列 */
typedef struct thread_task
{
    char name[32];              /* 线程名，也是邮箱地址 */
    pthread_t tid;              /* 线程 id */
    void *(*pfun)(void *);      /* 线程入口 */
    LQue_t *pque;               /* 收件队列 */
    pthread_mutex_t mutex;      /* 保护 pque */
    sem_t sem;                  /* 有新消息时唤醒 */
} LData_t;

typedef struct node
{
    LData_t data;
    struct node *pnext;
} Node_t;

typedef struct link
{
    Node_t *phead;
    int clen;
} Link_t;

extern Link_t *create_link(void);
extern int insert_link_head(Link_t *plink, LData_t data);
extern int insert_link_tail(Link_t *plink, LData_t data);
extern int delete_link_head(Link_t *plink);
extern int delete_link_tail(Link_t *plink);
extern void show_link(Link_t *plink);
extern void destroy_link(Link_t *plink);

#endif
