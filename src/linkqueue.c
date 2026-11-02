#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/linkqueue.h"

LQue_t *create_link_queue(void)
{
    LQue_t *pque = (LQue_t *)malloc(sizeof(LQue_t));
    if (NULL == pque)
    {
        return NULL;
    }
    pque->phead = NULL;
    pque->ptail = NULL;
    pque->clen = 0;
    return pque;
}

int push_link_queue(LQue_t *pque, Data_t data)
{
    if (NULL == pque)
    {
        return -1;
    }
    QNode_t *pnew = (QNode_t *)malloc(sizeof(QNode_t));
    if (NULL == pnew)
    {
        return -1;
    }
    pnew->data = data;
    pnew->pnext = NULL;

    if (NULL == pque->ptail)
    {
        pque->phead = pnew;
        pque->ptail = pnew;
    }
    else
    {
        pque->ptail->pnext = pnew;
        pque->ptail = pnew;
    }
    pque->clen++;
    return 0;
}

int pop_link_queue(LQue_t *pque, Data_t *pdata)
{
    if (NULL == pque || NULL == pque->phead)
    {
        return -1;
    }
    QNode_t *ptmp = pque->phead;
    if (pdata != NULL)
    {
        *pdata = ptmp->data;
    }
    pque->phead = ptmp->pnext;
    if (NULL == pque->phead)
    {
        pque->ptail = NULL;
    }
    free(ptmp);
    pque->clen--;
    return 0;
}

int get_link_queue_head(LQue_t *pque, Data_t *pdata)
{
    if (NULL == pque || NULL == pque->phead || NULL == pdata)
    {
        return -1;
    }
    *pdata = pque->phead->data;
    return 0;
}

void show_link_queue(LQue_t *pque)
{
    if (NULL == pque)
    {
        return;
    }
    printf("[queue] %d msg(s)\n", pque->clen);
}

void destroy_link_queue(LQue_t *pque)
{
    if (NULL == pque)
    {
        return;
    }
    QNode_t *ptmp = pque->phead;
    while (ptmp != NULL)
    {
        QNode_t *pnext = ptmp->pnext;
        free(ptmp);
        ptmp = pnext;
    }
    free(pque);
}
