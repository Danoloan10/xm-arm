XM_CORE_PATH=$(XTRATUM_PATH)/core
LIBXM_PATH=$(XTRATUM_PATH)/user/libxm

check_gcc = $(shell if $(TARGET_CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi)

HOST_CFLAGS = -Wall -O2 -D$(ARCH) -I$(LIBXM_PATH)/include -DHOST
HOST_LDFLAGS =

TARGET_CFLAGS  = -Wall -O2 -nostdlib -nostdinc -ffreestanding -D$(ARCH) -fno-strict-aliasing -fomit-frame-pointer
TARGET_CFLAGS += -I$(LIBXM_PATH)/include --include xm_inc/config.h --include xm_inc/arch/arch_types.h

TARGET_CXXFLAGS  = -Wall -O2 -nostdlib -nostdinc -ffreestanding -D$(ARCH) -fno-strict-aliasing -fomit-frame-pointer -fno-rtti -fno-exceptions
TARGET_CXXFLAGS += -I$(LIBXM_PATH)/include --include xm_inc/config.h --include xm_inc/arch/arch_types.h

TARGET_CFLAGS_ARCH := $(shell echo $(TARGET_CFLAGS_ARCH))
TARGET_CXXFLAGS_ARCH := $(shell echo $(TARGET_CXXFLAGS_ARCH))
TARGET_LDFLAGS_ARCH := $(shell echo $(TARGET_LDFLAGS_ARCH))
TARGET_ASFLAGS_ARCH := $(shell echo $(TARGET_ASFLAGS_ARCH))

# disable pointer signedness warnings in gcc 4.0
TARGET_CFLAGS += $(call check_gcc,-Wno-pointer-sign,)
# disable stack protector in gcc 4.1
TARGET_CFLAGS += $(call check_gcc,-fno-stack-protector,)
TARGET_CXXFLAGS += $(call check_gcc,-fno-stack-protector,)

TARGET_CFLAGS += $(TARGET_CFLAGS_ARCH)
TARGET_CXXFLAGS += $(TARGET_CXXFLAGS_ARCH)
 
#TARGET_CFLAGS += $(TARGET_CFLAGS_ARCH)

#TARGET_CFLAGS += -Wno-comment -Wno-unused-value -Wno-return-type

TARGET_ASFLAGS = -Wall -O2 -D__ASSEMBLY__ -fno-builtin -D$(ARCH)
TARGET_ASFLAGS += -I$(LIBXM_PATH)/include -nostdlib -nostdinc --include xm_inc/config.h
TARGET_ASFLAGS += $(TARGET_ASFLAGS_ARCH) $(TARGET_CFLAGS_ARCH)
LIBGCC=`$(TARGET_CC) -print-libgcc-file-name $(TARGET_CFLAGS_ARCH)`

TARGET_LDFLAGS = $(TARGET_LDFLAGS_ARCH)

ifdef CONFIG_DEBUG
TARGET_CFLAGS+=-g -D_DEBUG_
TARGET_CXXFLAGS+=-g -D_DEBUG_
HOST_CFLAGS+=-g -D_DEBUG_
else
TARGET_CFLAGS+=-fomit-frame-pointer
TARGET_CXXFLAGS+=-fomit-frame-pointer
endif

export TARGET_CFLAGS_ARCH TARGET_LDFLAGS_ARCH TARGET_ASFLAGS_ARCH
export TARGET_CFLAGS TARGET_ASFLAGS TARGET_LDFLAGS
export HOST_CFLAGS

%.host.o: %.c
	$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

%.host.o: %.S
	$(HOST_CC) $(HOST_ASFLAGS) -o $@ -c $<

%.o: %.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

%.o: %.cpp
	$(TARGET_CXX) $(TARGET_CXXFLAGS) -c $< -o $@

%.o: %.S
	$(TARGET_CC) $(TARGET_ASFLAGS) -o $@ -c $<

.PHONY: $(clean-targets) $(config-targets)
dep.mk: $(SRCS)
# don't generate deps  when cleaning
ifeq ($(findstring $(MAKECMDGOALS), $(clean-targets) $(config-targets) ),)
	@for file in $(SRCS) ; do \
		$(TARGET_CC) $(TARGET_CFLAGS) -M $$file ; \
	done > dep.mk

#	@for file in $(SRCS) ; do \
		$(TARGET_CC) $(TARGET_CFLAGS) -M $$file | sed -e "s/.*:/`dirname $$file`\/&/" ; \
	done > dep.mk
endif

dephost.mk: $(HOST_SRCS)
# don't generate deps  when cleaning
ifeq ($(findstring $(MAKECMDGOALS), $(clean-targets) $(config-targets) ),)
	@for file in $(HOST_SRCS) ; do \
		$(HOST_CC) $(HOST_CFLAGS) -M $$file | sed -e "s/\(.*\).o:/`dirname $$file`\/\1.host.o:/" ; \
	done > dephost.mk
endif

%.dump : %
	$(TARGET_OBJDUMP) -SC $^ > $@
