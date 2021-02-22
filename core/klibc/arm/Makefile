all: objs

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/core/rules.mk
-include dep.mk

SRCS := $(sort $(wildcard *.c))
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
OBJS := $(patsubst %.S,%.o,$(wildcard *.S)) $(OBJS)
LOBJS := $(OBJS)

objs: $(OBJS)
	@$(TARGET_LD) $(TARGET_LDFLAGS) -r -o arch_libc.o $(LOBJS)
