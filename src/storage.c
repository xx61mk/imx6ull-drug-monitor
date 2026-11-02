#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "sqlite3.h"
#include "../include/storage.h"
#include "../include/app_config.h"

/*
 * SQLite 持久化
 * -------------
 * 为什么按天分表：单表跑几个月就是几百万行，查询和清理都变慢；
 * 按天建 env_YYYYMMDD，过期直接整表归档再 DROP，清理成本恒定。
 *
 * 掉电安全：WAL + synchronous=NORMAL，已提交事务不会因为断电丢；
 * 启动先跑 integrity_check，真损坏就把文件改名留证并重建，不让程序起不来。
 */

static sqlite3 *g_db = NULL;
static sqlite3_stmt *g_ins = NULL;      /* 当前表的插入语句，复用避免反复 prepare */
static char  g_path[256] = {0};
static char  g_table[32] = {0};
static int   g_retention = CFG_DB_RETENTION_DAYS;
static int   g_pending = 0;             /* 未提交行数 */
static int   g_in_tx = 0;
static long  g_total_written = 0;
static int   g_last_error = 0;

static const char *SCHEMA_FMT =
    "CREATE TABLE IF NOT EXISTS %s ("
    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " ts INTEGER NOT NULL,"
    " seq INTEGER,"
    " temp REAL,"
    " hum REAL,"
    " accel REAL,"
    " door INTEGER,"
    " leak INTEGER,"
    " heater INTEGER,"
    " cooler INTEGER,"
    " alarm INTEGER);";

/* 把时间转成本地日期整数 YYYYMMDD，用于表名和过期比较 */
static int ymd_of(time_t ts)
{
    struct tm tmv;
#if defined(_WIN32)
    struct tm *p = localtime(&ts);
    if (p != NULL)
    {
        tmv = *p;
    }
    else
    {
        memset(&tmv, 0, sizeof(tmv));
    }
#else
    localtime_r(&ts, &tmv);
#endif
    return (tmv.tm_year + 1900) * 10000 + (tmv.tm_mon + 1) * 100 + tmv.tm_mday;
}

static void day_table(time_t ts, char *out, int len)
{
    int ymd = ymd_of(ts);
    snprintf(out, len, "env_%08d", ymd);
}

static int exec_sql(const char *sql)
{
    char *errmsg = NULL;
    int rc;

    if (NULL == g_db)
    {
        return -1;
    }
    rc = sqlite3_exec(g_db, sql, NULL, NULL, &errmsg);
    if (SQLITE_OK != rc)
    {
        printf("[db] sql failed: %s | %s\n", sql, (errmsg != NULL) ? errmsg : "?");
        if (NULL != errmsg)
        {
            sqlite3_free(errmsg);
        }
        g_last_error = rc;
        return -1;
    }
    return 0;
}

static int table_count(const char *table)
{
    char sql[160];
    sqlite3_stmt *st = NULL;
    int n = 0;

    snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM \"%s\";", table);
    if (SQLITE_OK != sqlite3_prepare_v2(g_db, sql, -1, &st, NULL))
    {
        return 0;
    }
    if (SQLITE_ROW == sqlite3_step(st))
    {
        n = sqlite3_column_int(st, 0);
    }
    sqlite3_finalize(st);
    return n;
}

/* 把超过保留期的表整表搬到 env_archive，再删表 */
static int cleanup_old_tables(void)
{
    sqlite3_stmt *st = NULL;
    char names[64][32];
    int  n = 0;
    int  i;
    int  cutoff;
    time_t now = time(NULL);
    char archive_sql[512];

    snprintf(archive_sql, sizeof(archive_sql), SCHEMA_FMT, CFG_DB_ARCHIVE);
    if (0 != exec_sql(archive_sql))
    {
        return -1;
    }

    cutoff = ymd_of(now - (time_t)g_retention * 86400);

    if (SQLITE_OK != sqlite3_prepare_v2(g_db,
            "SELECT name FROM sqlite_master WHERE type='table' AND name LIKE 'env\\_2%' ESCAPE '\\';",
            -1, &st, NULL))
    {
        return -1;
    }
    while (SQLITE_ROW == sqlite3_step(st) && n < 64)
    {
        const unsigned char *nm = sqlite3_column_text(st, 0);
        if (NULL != nm)
        {
            snprintf(names[n], 32, "%s", (const char *)nm);
            n++;
        }
    }
    sqlite3_finalize(st);

    for (i = 0; i < n; i++)
    {
        int ymd = 0;
        char sql[512];

        if (1 != sscanf(names[i], "env_%8d", &ymd))
        {
            continue;
        }
        if (ymd >= cutoff)
        {
            continue;       /* 还在保留期内 */
        }

        /* 先归档再删，保留可追溯性 */
        snprintf(sql, sizeof(sql),
                 "INSERT INTO %s (ts,seq,temp,hum,accel,door,leak,heater,cooler,alarm) "
                 "SELECT ts,seq,temp,hum,accel,door,leak,heater,cooler,alarm FROM \"%s\";",
                 CFG_DB_ARCHIVE, names[i]);
        if (0 == exec_sql(sql))
        {
            snprintf(sql, sizeof(sql), "DROP TABLE \"%s\";", names[i]);
            if (0 == exec_sql(sql))
            {
                printf("[db] archived & dropped %s\n", names[i]);
            }
        }
    }
    return 0;
}

static int integrity_check(void)
{
    sqlite3_stmt *st = NULL;
    int ok = 1;

    if (SQLITE_OK != sqlite3_prepare_v2(g_db, "PRAGMA integrity_check;", -1, &st, NULL))
    {
        return 0;
    }
    if (SQLITE_ROW == sqlite3_step(st))
    {
        const unsigned char *r = sqlite3_column_text(st, 0);
        if (NULL == r || 0 != strcmp((const char *)r, "ok"))
        {
            ok = 0;
            printf("[db] integrity_check -> %s\n", (r != NULL) ? (const char *)r : "?");
        }
    }
    sqlite3_finalize(st);
    return ok;
}

static int open_db(void)
{
    int rc = sqlite3_open_v2(g_path, &g_db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (SQLITE_OK != rc)
    {
        printf("[db] open %s failed: %s\n", g_path,
               (g_db != NULL) ? sqlite3_errmsg(g_db) : "?");
        if (g_db != NULL)
        {
            sqlite3_close(g_db);
            g_db = NULL;
        }
        return -1;
    }
    sqlite3_busy_timeout(g_db, 2000);
    return 0;
}

int storage_open(const char *path, int retention_days)
{
    int attempt;

    if (NULL == path || '\0' == path[0])
    {
        path = CFG_DB_PATH;
    }
    snprintf(g_path, sizeof(g_path), "%s", path);
    g_retention = (retention_days > 0) ? retention_days : CFG_DB_RETENTION_DAYS;

    for (attempt = 0; attempt < 2; attempt++)
    {
        if (0 != open_db())
        {
            return -1;
        }

        /* WAL：读写并发不互锁；NORMAL：每次事务落盘但不每条 fsync，掉电最多丢最后一批 */
        exec_sql("PRAGMA journal_mode=WAL;");
        exec_sql("PRAGMA synchronous=NORMAL;");
        exec_sql("PRAGMA temp_store=MEMORY;");

        if (integrity_check())
        {
            printf("[db] %s ready (WAL, retention=%d days)\n", g_path, g_retention);
            break;
        }

        /* 损坏：改名留档，重建一个空库，保证程序能继续跑 */
        sqlite3_close(g_db);
        g_db = NULL;
        {
            char bad[300];
            char side[320];
            time_t now = time(NULL);

            snprintf(bad, sizeof(bad), "%s.corrupt.%ld", g_path, (long)now);
            remove(bad);
            if (0 == rename(g_path, bad))
            {
                printf("[db] corrupted file kept as %s\n", bad);
            }

            /* 主库改名后必须把 -wal / -shm 一起清掉，
             * 否则旧 WAL 会挂到新库上，把新库也带坏 */
            snprintf(side, sizeof(side), "%s-wal", g_path);
            remove(side);
            snprintf(side, sizeof(side), "%s-shm", g_path);
            remove(side);
        }
    }

    if (NULL == g_db)
    {
        return -1;
    }

    cleanup_old_tables();
    return 0;
}

static int prepare_insert(const char *table)
{
    char sql[512];

    if (NULL != g_ins)
    {
        sqlite3_finalize(g_ins);
        g_ins = NULL;
    }

    snprintf(sql, sizeof(sql), SCHEMA_FMT, table);
    if (0 != exec_sql(sql))
    {
        return -1;
    }
    snprintf(sql, sizeof(sql), "CREATE INDEX IF NOT EXISTS idx_%s_ts ON %s(ts);", table, table);
    exec_sql(sql);

    snprintf(sql, sizeof(sql),
             "INSERT INTO \"%s\" (ts,seq,temp,hum,accel,door,leak,heater,cooler,alarm) "
             "VALUES (?,?,?,?,?,?,?,?,?,?);", table);
    if (SQLITE_OK != sqlite3_prepare_v2(g_db, sql, -1, &g_ins, NULL))
    {
        printf("[db] prepare insert failed: %s\n", sqlite3_errmsg(g_db));
        g_ins = NULL;
        return -1;
    }
    snprintf(g_table, sizeof(g_table), "%s", table);
    return 0;
}

int storage_write(const struct msgdata *d)
{
    time_t ts;
    char table[32];
    int rc;

    if (NULL == g_db || NULL == d)
    {
        return -1;
    }

    ts = mktime((struct tm *)&d->tim);
    if ((time_t)-1 == ts || ts <= 0)
    {
        ts = time(NULL);
    }

    day_table(ts, table, sizeof(table));
    if (0 != strcmp(table, g_table))
    {
        /* 跨天：先把昨天的尾巴提交，再切到新表 */
        storage_flush();
        if (0 != prepare_insert(table))
        {
            return -1;
        }
        printf("[db] switched to table %s\n", table);
    }

    if (!g_in_tx)
    {
        if (0 != exec_sql("BEGIN;"))
        {
            return -1;
        }
        g_in_tx = 1;
    }

    sqlite3_bind_int64(g_ins, 1, (sqlite3_int64)ts);
    sqlite3_bind_int(g_ins, 2, d->cnt);
    sqlite3_bind_double(g_ins, 3, (double)d->temp);
    sqlite3_bind_double(g_ins, 4, (double)d->hum);
    sqlite3_bind_double(g_ins, 5, (double)d->a);
    sqlite3_bind_int(g_ins, 6, d->door);
    sqlite3_bind_int(g_ins, 7, d->leak);
    sqlite3_bind_int(g_ins, 8, d->heater);
    sqlite3_bind_int(g_ins, 9, d->cooler);
    sqlite3_bind_int(g_ins, 10, (int)d->alarm);

    rc = sqlite3_step(g_ins);
    sqlite3_reset(g_ins);
    sqlite3_clear_bindings(g_ins);

    if (SQLITE_DONE != rc && SQLITE_CONSTRAINT != rc)
    {
        printf("[db] insert failed rc=%d %s\n", rc, sqlite3_errmsg(g_db));
        g_last_error = rc;
        return -1;
    }

    g_pending++;
    g_total_written++;

    if (g_pending >= CFG_DB_BATCH_ROWS)
    {
        return storage_flush();
    }
    return 0;
}

int storage_flush(void)
{
    if (NULL == g_db || !g_in_tx)
    {
        g_pending = 0;
        return 0;
    }
    if (0 != exec_sql("COMMIT;"))
    {
        exec_sql("ROLLBACK;");
        g_in_tx = 0;
        g_pending = 0;
        return -1;
    }
    g_in_tx = 0;
    g_pending = 0;
    return 0;
}

long storage_rows_total(void)
{
    sqlite3_stmt *st = NULL;
    char names[64][32];
    int n = 0;
    int i;
    long sum = 0;

    if (NULL == g_db)
    {
        return 0;
    }
    if (SQLITE_OK != sqlite3_prepare_v2(g_db,
            "SELECT name FROM sqlite_master WHERE type='table' AND name LIKE 'env\\_2%' ESCAPE '\\';",
            -1, &st, NULL))
    {
        return 0;
    }
    while (SQLITE_ROW == sqlite3_step(st) && n < 64)
    {
        const unsigned char *nm = sqlite3_column_text(st, 0);
        if (NULL != nm)
        {
            snprintf(names[n], 32, "%s", (const char *)nm);
            n++;
        }
    }
    sqlite3_finalize(st);

    for (i = 0; i < n; i++)
    {
        sum += table_count(names[i]);
    }
    return sum;
}

long long storage_file_size(void)
{
    struct stat st;
    if (0 == stat(g_path, &st))
    {
        return (long long)st.st_size;
    }
    return 0;
}

int storage_dump_recent(int n)
{
    sqlite3_stmt *st = NULL;
    char sql[256];
    int printed = 0;

    if (NULL == g_db || n <= 0)
    {
        return 0;
    }
    if ('\0' == g_table[0])
    {
        return 0;
    }

    snprintf(sql, sizeof(sql),
             "SELECT ts,temp,hum,accel,door,leak,heater,cooler,alarm FROM \"%s\" "
             "ORDER BY id DESC LIMIT ?;", g_table);
    if (SQLITE_OK != sqlite3_prepare_v2(g_db, sql, -1, &st, NULL))
    {
        return 0;
    }
    sqlite3_bind_int(st, 1, n);

    printf("---- last %d rows in %s ----\n", n, g_table);
    printf("  %-19s %7s %7s %6s %4s %4s %4s %4s %6s\n",
           "time", "temp", "hum", "accel", "door", "leak", "htr", "clr", "alarm");
    while (SQLITE_ROW == sqlite3_step(st))
    {
        long long ts = sqlite3_column_int64(st, 0);
        time_t t = (time_t)ts;
        struct tm tmv;
        char tbuf[32];

#if defined(_WIN32)
        {
            struct tm *p = localtime(&t);
            memset(&tmv, 0, sizeof(tmv));
            if (p != NULL)
            {
                tmv = *p;
            }
        }
#else
        localtime_r(&t, &tmv);
#endif
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", &tmv);
        printf("  %-19s %7.2f %7.1f %6.2f %4d %4d %4d %4d %6u\n",
               tbuf,
               sqlite3_column_double(st, 1),
               sqlite3_column_double(st, 2),
               sqlite3_column_double(st, 3),
               sqlite3_column_int(st, 4),
               sqlite3_column_int(st, 5),
               sqlite3_column_int(st, 6),
               sqlite3_column_int(st, 7),
               (unsigned int)sqlite3_column_int(st, 8));
        printed++;
    }
    sqlite3_finalize(st);
    return printed;
}

int storage_summary(char *buf, int buflen)
{
    if (NULL == buf || buflen <= 0)
    {
        return -1;
    }
    snprintf(buf, buflen, "db=%s rows=%ld written=%ld size=%lldKB err=%d",
             g_path, storage_rows_total(), g_total_written,
             storage_file_size() / 1024, g_last_error);
    return 0;
}

void storage_close(void)
{
    storage_flush();
    if (NULL != g_ins)
    {
        sqlite3_finalize(g_ins);
        g_ins = NULL;
    }
    if (NULL != g_db)
    {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}
