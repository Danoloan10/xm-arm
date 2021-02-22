XM_CORE_PATH=$(XTRATUM_PATH)/core

check_gcc = $(shell if $(TARGET_CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi)

TARGET_CFLAGS = -Wall -D_XM_KERNEL_ -fno-builtin -nostdlib -nostdinc -D$(ARCH) -fno-strict-aliasing 
TARGET_CFLAGS += -I$(XM_CORE_PATH)/include --include config.h --include $(ARCH)/arch_types.h 
#-fno-stack-protector

TARGET_CFLAGS_ARCH := $(shell echo $(TARGET_CFLAGS_ARCH))
TARGET_LDFLAGS_ARCH := $(shell echo $(TARGET_LDFLAGS_ARCH))
TARGET_ASFLAGS_ARCH := $(shell echo $(TARGET_ASFLAGS_ARCH))

TARGET_CFLAGS += $(TARGET_CFLAGS_ARCH)

# disable pointer signedness warnings in gcc 4.0
TARGET_CFLAGS += $(call check_gcc,-Wno-pointer-sign,)
# disable stack protector in gcc 4.1
TARGET_CFLAGS += $(call check_gcc,-fno-stack-protector,)

ifndef CONFIG_NO_GCC_OPT
TARGET_CFLAGS += -O2
endif

TARGET_ASFLAGS = -Wall -D__ASSEMBLY__ -D_XM_KERNEL_ -fno-builtin -D$(ARCH) -nostdlib -nostdinc
TARGET_ASFLAGS += -I$(XM_CORE_PATH)/include --include config.h

TARGET_ASFLAGS += $(TARGET_ASFLAGS_ARCH) $(TARGET_CFLAGS_ARCH)

LIBGCC = `$(TARGET_CC) -print-libgcc-file-name $(TARGET_CFLAGS)`

TARGET_LDFLAGS =

TARGET_LDFLAGS += $(TARGET_LDFLAGS_ARCH)

ifdef CONFIG_DEBUG
	TARGET_CFLAGS+=-g
else
	TARGET_CFLAGS+=-fomit-frame-pointer
endif

export TARGET_CFLAGS_ARCH TARGET_LDFLAGS_ARCH TARGET_ASFLAGS_ARCH
export TARGET_CFLAGS TARGET_ASFLAGS TARGET_LDFLAGS

%.o: %.c
	$(TARGET_CC) $(TARGET_CFLAGS)  -c $< -o $@

%.o: %.S
	$(TARGET_CC) $(TARGET_ASFLAGS)  -c $< -o $@

%.xo: %.c
	@$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@.1
	@$(TARGET_OBJCOPY) -O binary -j .esymb $@.1 $@.symb
	@$(TARGET_OBJCOPY) -X --keep-global-symbols=$@.symb $@.1 $@
	@$(RM) -f $@.1 $@.symb

%.xo: %.o
	@$(TARGET_OBJCOPY) -O binary -j .esymb $< $@.symb
	@$(TARGET_OBJCOPY) -X --keep-global-symbols=$@.symb $< $@

dep.mk: $(SRCS)
# don't generate deps  when cleaning
ifeq ($(findstring $(MAKECMDGOALS), clean distclean),)
	@for file in $(SRCS) ; do \
		$(TARGET_CC) $(TARGET_CFLAGS) -M $$file ; \
	done > dep.mk
endif

