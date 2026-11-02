#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "../include/mailbox.h"

/*
 * 邮箱系统：链表当线程注册表，每个线程节点挂一条收件队列。
 *
 *   send_mail_msg("get_data", data, "save_data")  按名字投递
 *   recv_mail_msg(&data, who)                     取自己的信件，函数内部靠 TLS 认出「自己」
 *
 * 相比原始版本修了三处：
 *   1. 先挂链表再 pthread_create，避免线程启动瞬间找不到自己（丢首条数据）
 *   2. 线程身份用线程局部存储记录，不再靠 tid 反查，彻底消除竞态
 *   3. 收件带超时并检查退出标志，Ctrl+C 后 join 不会永久阻塞
 */

volatile int system_run_flag = 1;

static Link_t *pmail = NULL;
static __thread Node_t *g_cur_node = NULL;   /* 线程自己所属的节点 */

struct start_arg
{
    Node_t *pnode;
};

static void *mailbox_trampoline(void *arg)
{
    struct start_arg *pa = (struct start_arg *)arg;
    Node_t *pnode = pa->pnode;
    void *ret;

    free(pa);
    g_cur_node = pnode;              /* 先认亲，再干活 */
    ret = pnode->data.pfun(NULL);
    return ret;
}

static Node_t *find_by_name(const char *name)
{
    Node_t *ptmp = (pmail == NULL) ? NULL : pmail->phead;
    while (ptmp != NULL)
    {
        if (0 == strcmp(ptmp->data.name, name))
        {
            return ptmp;
        }
        ptmp = ptmp->pnext;
    }
    return NULL;
}

static Node_t *find_self(void)
{
    if (g_cur_node != NULL)
    {
        return g_cur_node;
    }

    /* 容错：万一不是由 trampoline 拉起的线程，退回按 tid 查找 */
    {
        pthread_t mytid = pthread_self();
        Node_t *ptmp = (pmail == NULL) ? NULL : pmail->phead;
        while (ptmp != NULL)
        {
            if (pthread_equal(ptmp->data.tid, mytid))
            {
                return ptmp;
            }
            ptmp = ptmp->pnext;
        }
    }
    return NULL;
}

Link_t *create_mailbox_system(void)
{
    if (pmail != NULL)
    {
        return pmail;
    }
    pmail = create_link();
    if (NULL == pmail)
    {
        printf("[mailbox] create_link failed\n");
        return NULL;
    }
    return pmail;
}

int register_thread_mailbox(Link_t *plink, const char *name, void *(*pfun)(void *))
{
    Node_t *pnew;
    struct start_arg *pa;

    if (NULL == plink || NULL == name || NULL == pfun)
    {
        return -1;
    }

    pnew = (Node_t *)calloc(1, sizeof(Node_t));
    if (NULL == pnew)
    {
        printf("[mailbox] calloc node failed\n");
        return -1;
    }

    snprintf(pnew->data.name, sizeof(pnew->data.name), "%s", name);
    pnew->data.pfun = pfun;
    pnew->data.pque = create_link_queue();
    if (NULL == pnew->data.pque)
    {
        printf("[mailbox] create queue failed\n");
        free(pnew);
        return -1;
    }
    pthread_mutex_init(&pnew->data.mutex, NULL);
    sem_init(&pnew->data.sem, 0, 0);

    /* 关键顺序：先进注册表，再拉起线程 */
    pnew->pnext = plink->phead;
    plink->phead = pnew;
    plink->clen++;

    pa = (struct start_arg *)malloc(sizeof(struct start_arg));
    if (NULL == pa)
    {
        printf("[mailbox] malloc start_arg failed\n");
        return -1;
    }
    pa->pnode = pnew;

    if (0 != pthread_create(&pnew->data.tid, NULL, mailbox_trampoline, pa))
    {
        printf("[mailbox] pthread_create %s failed\n", name);
        free(pa);
        return -1;
    }
    return 0;
}

int send_mail_msg(const char *send_name, struct msgdata mydata, const char *recv_name)
{
    Node_t *pnode;
    Data_t qdata;

    if (0 == system_run_flag)
    {
        return -1;
    }
    if (NULL == send_name || NULL == recv_name)
    {
        return -1;
    }

    pnode = find_by_name(recv_name);
    if (NULL == pnode)
    {
        /* 接收者还没注册（启动瞬间）或写错了名字 */
        return -1;
    }

    memset(&qdata, 0, sizeof(qdata));
    qdata.data = mydata;
    snprintf(qdata.send_name, sizeof(qdata.send_name), "%s", send_name);
    snprintf(qdata.recv_name, sizeof(qdata.recv_name), "%s", recv_name);

    pthread_mutex_lock(&pnode->data.mutex);
    push_link_queue(pnode->data.pque, qdata);
    sem_post(&pnode->data.sem);          /* 先放数据再唤醒 */
    pthread_mutex_unlock(&pnode->data.mutex);

    return 0;
}

int recv_mail_msg_timeout(struct msgdata *mydata, char *send_name, int timeout_ms)
{
    Node_t *pnode = find_self();
    Data_t qdata;
    struct timespec ts;
    int rc;

    if (NULL == pnode || NULL == mydata)
    {
        return -1;
    }
    if (0 == system_run_flag)
    {
        return -1;
    }

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L)
    {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000L;
    }

    do
    {
        rc = sem_timedwait(&pnode->data.sem, &ts);
    } while (rc != 0 && errno == EINTR);

    if (rc != 0)
    {
        return -2;      /* 超时，调用方继续检查退出标志 */
    }

    pthread_mutex_lock(&pnode->data.mutex);
    rc = pop_link_queue(pnode->data.pque, &qdata);
    pthread_mutex_unlock(&pnode->data.mutex);

    if (0 != rc)
    {
        return -2;
    }

    *mydata = qdata.data;
    if (send_name != NULL)
    {
        snprintf(send_name, 32, "%s", qdata.send_name);
    }
    return 0;
}

int recv_mail_msg(struct msgdata *mydata, char *send_name)
{
    while (system_run_flag)
    {
        int rc = recv_mail_msg_timeout(mydata, send_name, 200);
        if (0 == rc)
        {
            return 0;
        }
        if (-1 == rc)
        {
            break;
        }
        /* -2 超时：转一圈继续等 */
    }
    return -1;
}

void mailbox_stop(void)
{
    Node_t *ptmp;

    system_run_flag = 0;
    if (NULL == pmail)
    {
        return;
    }
    /* 把所有还堵在 sem_wait 上的线程叫醒，让它们看到退出标志 */
    for (ptmp = pmail->phead; ptmp != NULL; ptmp = ptmp->pnext)
    {
        sem_post(&ptmp->data.sem);
    }
}

void destroy_mailbox(Link_t *plink)
{
    Node_t *ptmp;

    if (NULL == plink)
    {
        return;
    }

    mailbox_stop();

    ptmp = plink->phead;
    while (ptmp != NULL)
    {
        Node_t *pnext = ptmp->pnext;
        pthread_join(ptmp->data.tid, NULL);
        destroy_link_queue(ptmp->data.pque);
        pthread_mutex_destroy(&ptmp->data.mutex);
        sem_destroy(&ptmp->data.sem);
        free(ptmp);
        ptmp = pnext;
    }

    if (plink == pmail)
    {
        pmail = NULL;
    }
    free(plink);
}
