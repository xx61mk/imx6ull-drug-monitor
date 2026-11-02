#
# 药品恒温监控 —— 构建脚本
#
#   make                      本地编译（PC / Ubuntu），产出 ./drug-monitor
#   make run                  跑一遍演示场景（5 分钟，带四类故障注入）
#   make check                只做语法与告警检查，不产出可执行文件
#   make arm                  交叉编译到 i.MX6ULL
#   make WITH_MQTT=1          接真实 MQTT 上云（需要 Paho MQTT C 库）
#   make USE_SYSTEM_SQLITE=1  用系统 libsqlite3，不编译内置源码
#   make clean
#
# 默认把 SQLite 源码一起编进来：板端往往没有 libsqlite3-dev，
# 内置一份 amalgamation 后只需要交叉工具链，少一个绊脚石。
#

CC       ?= gcc
TARGET   ?= drug-monitor
BUILD    ?= build

CFLAGS   ?= -O2 -g -std=gnu99 -Wall -Wextra
CPPFLAGS += -Iinclude
LDLIBS    = -lpthread -lrt -lm

SRC_CORE  = src/link.c \
            src/linkqueue.c \
            src/mailbox.c \
            src/sensor.c \
            src/control.c \
            src/storage.c \
            src/ringbuf.c \
            src/uplink.c \
            src/app.c

SRC_MAIN  = src/main.c
HEADERS   = $(wildcard include/*.h)

ifeq ($(WITH_MQTT),1)
  CPPFLAGS  += -DWITH_MQTT
  SRC_CORE  += src/mqtt_client.c
  LDLIBS    += -lpaho-mqtt3c
endif

CORE_OBJS = $(patsubst %.c,$(BUILD)/%.o,$(SRC_CORE))
MAIN_OBJ  = $(patsubst %.c,$(BUILD)/%.o,$(SRC_MAIN))

ifeq ($(USE_SYSTEM_SQLITE),1)
  SQLITE_OBJ =
  SQLITE_CPPFLAGS =
  LDLIBS    += -lsqlite3
else
  SQLITE_OBJ = $(BUILD)/third_party/sqlite3/sqlite3.o
  SQLITE_CPPFLAGS = -Ithird_party/sqlite3 -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION
endif

OBJS = $(CORE_OBJS) $(MAIN_OBJ) $(SQLITE_OBJ)

.PHONY: all run check arm clean help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
	@echo "built: $@"

$(BUILD)/src/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SQLITE_CPPFLAGS) -c $< -o $@

# SQLite 源码不跟业务代码一起开 -Wall，否则满屏都是第三方告警
$(BUILD)/third_party/sqlite3/%.o: third_party/sqlite3/%.c
	@mkdir -p $(dir $@)
	$(CC) -O2 $(SQLITE_CPPFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) --sim --scenario demo --seconds 300 --dump 10

check:
	@for f in $(SRC_CORE) $(SRC_MAIN); do \
		echo "  syntax $$f"; \
		$(CC) $(CFLAGS) $(CPPFLAGS) $(SQLITE_CPPFLAGS) -fsyntax-only $$f || exit 1; \
	done
	@echo "syntax check passed"

# 交叉编译单独用 build-arm 目录，避免和本机 x86 的 .o 混在一起
arm:
	$(MAKE) CC=arm-linux-gnueabihf-gcc TARGET=drug-monitor-arm BUILD=build-arm

clean:
	rm -rf build build-arm
	rm -f $(TARGET) drug-monitor-arm
	rm -f env.db env.db-wal env.db-shm
	rm -f uplink_queue.bin uplink_queue.bin.tmp

help:
	@sed -n '2,20p' Makefile
