#ifndef STORAGE_H
#define STORAGE_H

#include "linkqueue.h"

/*
 * SQLite3 本地持久化：
 *   - WAL + synchronous=NORMAL，掉电不丢已提交事务
 *   - 按天分表 env_YYYYMMDD，写满保留天数后整表归档到 env_archive 再删表
 *   - 启动做 integrity_check，发现损坏自动改名留档并重建
 *   - 批量事务写入，降低每条 fsync 开销
 */

int  storage_open(const char *path, int retention_days);
int  storage_write(const struct msgdata *d);      /* 进内存缓批，满 BATCH 或 flush 时提交 */
int  storage_flush(void);
void storage_close(void);

long storage_rows_total(void);                    /* 所有 env_* 表行数和 */
long long storage_file_size(void);                /* 主库文件字节数 */
int  storage_dump_recent(int n);                  /* 打印最近 n 条，返回实际打印条数 */
int  storage_summary(char *buf, int buflen);      /* 一行汇总，给退出统计用 */

#endif
