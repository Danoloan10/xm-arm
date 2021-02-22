#SRCS := $(sort $(wildcard *.c))
#OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
#LOBJS := $(OBJS)

all: objs mods

include ../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/core/rules.mk
-include dep.mk

srcs-y := kdevice.c

ifdef CONFIG_ARM
srcs-y += zynqboard/init.c
srcs-y += zynqboard/zynq_cache.c
srcs-y += zynqboard/zynq_L2Cpl310.c
srcs-y += zynqboard/zynq_mmu.c
srcs-y += zynqboard/zynq_tzpc.c
srcs-y += zynqboard/zynq_errorControl.c
srcs-$(CONFIG_DEV_UART) += zynqboard/zynq_uart.c
srcs-$(CONFIG_DEV_UART) += zynqboard/cortexa9_globalTimer.c
srcs-$(CONFIG_DEV_UART) += zynqboard/cortexa9_privateTimer.c
endif

srcs-$(CONFIG_DEV_MEMBLOCK) += memblock.c

SRCS := $(srcs-y) $(srcs-m)
OBJS := $(patsubst %.c,%.o,$(srcs-y))
LOBJS := $(OBJS)

OBJS-M := $(patsubst %.c,%.xo,$(srcs-m))

objs: $(OBJS)
	$(TARGET_LD) $(TARGET_LDFLAGS) -r -o drivers.o $(LOBJS)

mods: $(OBJS-M)
