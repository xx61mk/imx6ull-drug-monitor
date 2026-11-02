#
# Makefile  (framebuffer 版：不依赖 SDL，使用 /dev/fb0 + evdev 触摸屏)
#
# 本机编译（x86_64 Linux，需有 /dev/fb0 帧缓冲，如文本控制台 / 虚拟机）：
#   make
#   ./demo            # 全屏显示 UI，刷新邮箱推送的温度/湿度/设备号
#
# 交叉编译到 ARM 开发板（带 framebuffer + 触摸屏）：
#   make CC=arm-linux-gnueabihf-gcc
#
# 需要上云（MQTT/SQLite）时：
#   make WITH_MQTT=1 CC=arm-linux-gnueabihf-gcc
#
CC = arm-linux-gnueabihf-gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
CFLAGS ?= -O3 -g0 -I$(LVGL_DIR)/ -std=gnu99 -Wall #-Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare
LDFLAGS ?= -lm -lpthread -lrt -ldl
BIN = demo

# Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

CSRCS += $(LVGL_DIR)/mouse_cursor_icon.c

# UI (SquareLine Studio 生成)
CSRCS += $(wildcard $(LVGL_DIR)/UI/*.c)
CSRCS += $(wildcard $(LVGL_DIR)/UI/components/*.c)
CSRCS += $(wildcard $(LVGL_DIR)/UI/screens/*.c)
CSRCS += $(wildcard $(LVGL_DIR)/UI/fonts/*.c)
CSRCS += $(wildcard $(LVGL_DIR)/UI/images/*.c)

# 邮箱(mailbox)核心模块：链表 + 队列 + 邮箱
CSRCS += $(LVGL_DIR)/project/src/link.c
CSRCS += $(LVGL_DIR)/project/src/linkqueue.c
CSRCS += $(LVGL_DIR)/project/src/mailbox.c

# 邮箱头文件路径
CFLAGS += -I$(LVGL_DIR)/project/include

# 可选：MQTT 上云模块（默认不编）
#   make WITH_MQTT=1            # 本机编译
#   make WITH_MQTT=1 CC=arm-linux-gnueabihf-gcc   # 交叉编译
# 说明：直接把 Paho MQTT C 源码编进工程（未定义 OPENSSL 宏，无需 openssl 库），
#       数据由 save_data_th 线程经 MQTT 上传 OneNET 平台（见 project/include/mqtt.h）。
ifeq ($(WITH_MQTT),1)
CFLAGS += -DWITH_MQTT
CFLAGS += -I$(LVGL_DIR)/project/include/src
CSRCS += $(filter-out %MQTTVersion.c,$(wildcard $(LVGL_DIR)/project/include/src/*.c))
CSRCS += $(LVGL_DIR)/project/src/mqtt.c
CSRCS += $(LVGL_DIR)/project/src/mysqlite.c
LDFLAGS += -L$(LVGL_DIR)/project/lib/sqlite3/lib -lsqlite3
endif

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean:
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)
