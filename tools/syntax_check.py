#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""用 pycparser 对工程里的 .c 做纯语法解析（本机没有 C 编译器时的替代验证）。

做法：
  1. 递归展开工程内 include/ 下的双引号头文件，系统头文件跳过；
  2. 去掉所有预处理行（#include/#define/#if/#ifdef...），两分支代码都保留；
  3. 去掉 GCC 扩展关键字；
  4. 前面贴一段类型桩（FILE/pthread/sqlite3 等），只关心语法，不做语义检查。
"""
import os
import re
import sys

from pycparser import c_parser

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

STUB = r"""
typedef unsigned long size_t;
typedef long ssize_t;
typedef long time_t;
typedef long clock_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned int useconds_t;
typedef unsigned long pthread_t;
typedef unsigned long pthread_key_t;
typedef union { char __d[48]; } pthread_mutex_t;
typedef union { char __d[48]; } pthread_cond_t;
typedef union { char __d[32]; } sem_t;
typedef struct _IO_FILE FILE;
typedef struct __dirstream DIR;
typedef unsigned long DWORD;
struct dirent { char d_name[256]; };
struct tm {
    int tm_sec; int tm_min; int tm_hour; int tm_mday; int tm_mon;
    int tm_year; int tm_wday; int tm_yday; int tm_isdst;
};
struct timeval { long tv_sec; long tv_usec; };
struct timespec { long tv_sec; long tv_nsec; };
struct stat { long st_size; long st_mode; };
typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
typedef long long sqlite3_int64;
"""

MACRO_INIT = {
    "PTHREAD_MUTEX_INITIALIZER": "0",
    "SIG_IGN": "0",
    "NULL": "0",
    "CLOCK_REALTIME": "0",
}


def expand(path, seen):
    """展开工程内头文件，返回去掉预处理行后的源码文本。"""
    if path in seen:
        return ""
    seen.add(path)

    with open(path, "r", encoding="utf-8") as f:
        lines = f.readlines()

    out = []
    for line in lines:
        s = line.strip()

        m = re.match(r'#\s*include\s+"([^"]+)"', s)
        if m:
            inc = m.group(1)
            cand = os.path.normpath(os.path.join(os.path.dirname(path), inc))
            # 只展开工程自己的头文件，系统头 / sqlite3.h 跳过
            if os.path.exists(cand) and (os.sep + "include" + os.sep) in cand + os.sep:
                out.append(expand(cand, seen))
            continue

        if s.startswith("#"):
            continue

        out.append(line)

    return "".join(out)


def strip_comments(text):
    """把注释替换成等长空白，保持行号不变。"""
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            if j == -1:
                j = n
            out.append(" " * (j - i))
            i = j
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j == -1 else j + 2
            out.append("".join("\n" if ch == "\n" else " " for ch in text[i:j]))
            i = j
        elif c == '"' or c == "'":
            q = c
            out.append(c)
            i += 1
            while i < n:
                out.append(text[i])
                if text[i] == "\\" and i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                    continue
                if text[i] == q:
                    i += 1
                    break
                i += 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def clean(text):
    text = strip_comments(text)
    text = re.sub(r"\b__thread\b", "", text)
    text = re.sub(r"__attribute__\s*\(\([^)]*\)\)", "", text)
    text = re.sub(r"\b__inline\b", "inline", text)
    for k, v in MACRO_INIT.items():
        text = re.sub(r"\b%s\b" % k, v, text)
    return text


def check(relpath):
    path = os.path.join(ROOT, relpath)
    src = clean(expand(path, set()))
    full = STUB + "\n#line 1\n" + src
    parser = c_parser.CParser()
    try:
        parser.parse(full, filename=relpath)
    except Exception as exc:
        msg = str(exc)
        print("FAIL %s" % relpath)
        for line in msg.splitlines()[:6]:
            print("   " + line)
        return False
    print("OK   %s" % relpath)
    return True


def main():
    targets = [
        "src/link.c",
        "src/linkqueue.c",
        "src/mailbox.c",
        "src/sensor.c",
        "src/control.c",
        "src/storage.c",
        "src/ringbuf.c",
        "src/uplink.c",
        "src/app.c",
        "src/main.c",
    ]
    bad = 0
    for t in targets:
        if not check(t):
            bad += 1
    print("\n%d file(s) parsed, %d failed" % (len(targets), bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
