all: objs

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/core/rules.mk
-include dep.mk

SRCS_C-y = irqs.c hypercalls.c kthread.c setup.c panic.c processor.c 
SRCS_C-y += vmmap.c physmm.c abort.c
SRCS_C-y += gic.c

SRCS_S-y = start.S commonEntry.S secureEntry.S nonsecureEntry.S monitorEntry.S

-mfpu=vfp

SRCS_C-$(CONFIG_SMP_SUPPORT) += smp.c

#SRCS_C-$(CONFIG_MMU) += vmmap.c

SRCS=$(SRCS_S-y) #$(SRCS_C-y)

OBJS := $(patsubst %.c,%.o,$(SRCS_C-y))
OBJS := $(patsubst %.S,%.o,$(SRCS_S-y)) $(OBJS)

objs: $(OBJS)
	$(TARGET_LD) $(TARGET_LDFLAGS) -r -o karch.o $(OBJS)
