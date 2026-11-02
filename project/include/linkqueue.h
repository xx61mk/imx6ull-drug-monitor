#ifndef __LINKQUEUE_H__
#define __LINKQUEUE_H__

#include <time.h>

struct msgdata
{
      int id;        //设备id
      int cnt;       //数据编号 
      float temp;
      float hum;
      float  v;
      float a;
      //......
      struct tm tim;   //时间戳
};

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
}QNode_t;

typedef struct lqueue
{
    QNode_t *phead;
    QNode_t *ptail;
    int clen;
}LQue_t;

extern LQue_t * create_link_queue();
extern int push_link_queue(LQue_t *pque, Data_t data);
extern void show_link_queue(LQue_t *pque);
extern int pop_link_queue(LQue_t *pque, Data_t *pdata);
extern void destroy_link_queue(LQue_t *pque);
extern int get_link_queue_head(LQue_t *pque, Data_t *pdata);
#endif