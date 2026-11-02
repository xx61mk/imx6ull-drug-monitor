#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ringbuf.h"

/*
 * 离线环形队列
 * ------------
 * 断网时数据不能丢：先进这个定长队列，重连后按 FIFO 顺序补传。
 * 满了丢最旧的一条（并计数），因为对监管来说最新状态比历史更重要。
 * 每次入队/出队后落盘成二进制快照，断电重启还能接着补。
 */

#define RING_MAGIC 0x52494E47u   /* 'RING' */

struct ring_head
{
    unsigned int magic;
    int capacity;
    int count;
    int head;
    int tail;
    int dropped;
};

static struct up_msg *g_buf = NULL;
static int  g_cap = 0;
static int  g_head = 0;
static int  g_tail = 0;
static int  g_cnt = 0;
static int  g_dropped = 0;
static int  g_saved = 0;
static char g_path[256] = {0};
static int  g_use_file = 0;

int ring_init(int capacity, const char *path)
{
    if (capacity <= 0)
    {
        capacity = 32;
    }
    g_buf = (struct up_msg *)calloc((size_t)capacity, sizeof(struct up_msg));
    if (NULL == g_buf)
    {
        return -1;
    }
    g_cap = capacity;
    g_head = 0;
    g_tail = 0;
    g_cnt = 0;
    g_dropped = 0;
    g_saved = 0;

    if (NULL != path && '\0' != path[0])
    {
        snprintf(g_path, sizeof(g_path), "%s", path);
        g_use_file = 1;
    }
    return 0;
}

void ring_free(void)
{
    free(g_buf);
    g_buf = NULL;
    g_cap = 0;
    g_cnt = 0;
}

int ring_push(const struct up_msg *m)
{
    if (NULL == g_buf || NULL == m || g_cap <= 0)
    {
        return -1;
    }

    if (g_cnt == g_cap)
    {
        /* 满了：覆盖最旧的一条 */
        g_buf[g_tail] = *m;
        g_tail = (g_tail + 1) % g_cap;
        g_head = g_tail;
        g_dropped++;
    }
    else
    {
        g_buf[g_tail] = *m;
        g_tail = (g_tail + 1) % g_cap;
        g_cnt++;
    }

    ring_store();
    return 0;
}

int ring_pop(struct up_msg *m)
{
    if (NULL == g_buf || NULL == m || g_cnt <= 0)
    {
        return 0;
    }
    *m = g_buf[g_head];
    g_head = (g_head + 1) % g_cap;
    g_cnt--;
    return 1;
}

int ring_peek(struct up_msg *m)
{
    if (NULL == g_buf || NULL == m || g_cnt <= 0)
    {
        return 0;
    }
    *m = g_buf[g_head];
    return 1;
}

void ring_drop_front(void)
{
    if (NULL == g_buf || g_cnt <= 0)
    {
        return;
    }
    g_head = (g_head + 1) % g_cap;
    g_cnt--;
    ring_store();
}

int ring_count(void)
{
    return g_cnt;
}

int ring_dropped(void)
{
    return g_dropped;
}

int ring_saved(void)
{
    return g_saved;
}

int ring_store(void)
{
    struct ring_head hd;
    FILE *f;
    char tmp[300];

    if (!g_use_file || NULL == g_buf)
    {
        return -1;
    }

    hd.magic = RING_MAGIC;
    hd.capacity = g_cap;
    hd.count = g_cnt;
    hd.head = g_head;
    hd.tail = g_tail;
    hd.dropped = g_dropped;

    snprintf(tmp, sizeof(tmp), "%s.tmp", g_path);
    f = fopen(tmp, "wb");
    if (NULL == f)
    {
        return -1;
    }
    if (1 != fwrite(&hd, sizeof(hd), 1, f) ||
        (g_cap > 0 && (size_t)g_cap != fwrite(g_buf, sizeof(struct up_msg), (size_t)g_cap, f)))
    {
        fclose(f);
        remove(tmp);
        return -1;
    }
    fclose(f);

    /* 先写临时文件再改名，避免写到一半掉电留下半个文件 */
    remove(g_path);
    if (0 != rename(tmp, g_path))
    {
        return -1;
    }
    g_saved++;
    return 0;
}

int ring_load(void)
{
    struct ring_head hd;
    FILE *f;

    if (!g_use_file || NULL == g_buf)
    {
        return -1;
    }
    f = fopen(g_path, "rb");
    if (NULL == f)
    {
        return -1;      /* 首次运行没有文件，正常 */
    }
    if (1 != fread(&hd, sizeof(hd), 1, f))
    {
        fclose(f);
        return -1;
    }
    if (RING_MAGIC != hd.magic || hd.capacity != g_cap ||
        hd.count < 0 || hd.count > g_cap)
    {
        fclose(f);
        return -1;      /* 格式不匹配就忽略，不冒险读脏数据 */
    }
    if ((size_t)g_cap != fread(g_buf, sizeof(struct up_msg), (size_t)g_cap, f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    g_cnt = hd.count;
    g_head = hd.head;
    g_tail = hd.tail;
    g_dropped = hd.dropped;
    return g_cnt;
}
