
$(if $(XAL_PATH),, \
	$(warning "The configuration variable XAL_PATH is not set,") \
	$(error "check the \"common/mkconfig.dist\" file (see README)."))

ifneq ($(XAL_PATH)/common/config.mk, $(wildcard $(XAL_PATH)/common/config.mk))
XAL_PATH=../..
XTRATUM_PATH=../../../..
include $(XTRATUM_PATH)/core/.config
else
is_installed := 1
include $(XAL_PATH)/common/config.mk
include $(XTRATUM_PATH)/lib/dot.config
endif


# early detect of misconfiguration and missing variables
$(if $(XTRATUM_PATH),, \
	$(warning "The configuration variable XTRATUM_PATH is not set,") \
	$(error "check the \"common/mkconfig.dist\" file (see README)."))

#INCLUDES OF XTRATUM CONFIGURATION
include $(XTRATUM_PATH)/xmconfig
include $(XTRATUM_PATH)/version
ifdef is_installed
include $(XTRATUM_PATH)/lib/rules.mk
else
include $(XTRATUM_PATH)/user/rules.mk
endif

#PATHS
ifdef is_installed
XMBIN_PATH=$(XTRATUM_PATH)/bin
XMCORE_PATH=$(XTRATUM_PATH)/lib
else
XMBIN_PATH=$(XTRATUM_PATH)/user/bin
XMCORE_PATH=$(XTRATUM_PATH)/core
endif
XALLIB_PATH=$(XAL_PATH)/lib
XALBIN_PATH=$(XAL_PATH)/bin

# APPLICATIONS
XMCPARSER=$(XMBIN_PATH)/xmcparser
XMPACK=$(XMBIN_PATH)/xmpack
RSWBUILD=$(XMBIN_PATH)/rswbuild
XEF=$(XMBIN_PATH)/xmeformat build
XPATHSTART=$(XALBIN_PATH)/xpathstart

# XM CORE
XMCORE_ELF=$(XMCORE_PATH)/xm_core
XMCORE_BIN=$(XMCORE_PATH)/xm_core.bin
XMCORE=$(XMCORE_PATH)/xm_core.xef

#LIBRARIES
LIB_XM=-lxm
LIB_XAL=-lxal

#FLAGS
TARGET_CFLAGS += -I$(XAL_PATH)/include -fno-builtin
TARGET_CXXFLAGS += -I$(XAL_PATH)/include -fno-builtin

ifneq ($(EXTERNAL_LDFLAGS),y)
TARGET_LDFLAGS += -u start -u xmImageHdr -T$(XALLIB_PATH)/loader.lds\
	-L$(LIBXM_PATH) -L$(XALLIB_PATH)\
	--start-group $(LIBGCC) $(LIB_XM) $(LIB_XAL) --end-group
endif

export TARGET_CFLAGS TARGET_CFLAGS TARGET_LDFLAGS

# ADDRESS OF EACH PARTITION
# function usage: $(call xpathstart,partitionid,xmlfile)

xpathstart = $(shell $(XPATHSTART) $(1) $(2))
xpathmemarea = $(shell $(XAL_PATH)/bin/xpath -c -f $(2) '/xm:SystemDescription/xm:PartitionTable/xm:Partition['`echo '$(1)+1'|bc`']/xm:PhysicalMemoryAreas/xm:Area['$(3)']/@start')

%.xef:  %
	$(XEF) $< -o $@
ifdef CONFIG_ARM
	$(TARGET_OBJCOPY) -O binary $< $<.bin
endif

%.xef.xmc: %.bin.xmc
	$(XEF) -m $< -c -o $@

xm_cf.bin.xmc: xm_cf.$(ARCH).xml
	$(XMCPARSER) -o $@ $^
xm_cf.c.xmc: xm_cf.$(ARCH).xml
	$(XMCPARSER) -c -o $@ $^


resident_sw: container.bin
	$(RSWBUILD) $^ $@

%.dump:  %
	$(TARGET_OBJDUMP) -SC $< > $@

#XMLCF_CFLAGS = $(HOST_CFLAGS) --include xm_inc/config.h
#XMLCF_CFLAGS =  --include xm_inc/config.h

#xm_cf.$(ARCH).xml: xm_cf.$(ARCH).xml.base
#	@echo  "Eliminate old XM_CF"
#	@$(HOST_CC) $(XMLCF_CFLAGS) -x assembler-with-cpp -E -P $^ > $@
#	@sed -i "/^$$/d" $@


distclean: clean
	@$(RM) *~

clean:
	@$(RM) $(PARTITIONS) $(patsubst %.bin,%, $(PARTITIONS)) $(patsubst %.xef,%, $(PARTITIONS)) $(patsubst %.xef,%.bin, $(PARTITIONS))
	@$(RM) container.bin resident_sw resident_sw.srec xm_cf xm_cf.bin xm_cf.*.xmc
	@$(RM) kernel.out *.o *.*.xmc dep.mk
	@if test -e xm_cf.$(ARCH).xml.base; then $(RM) xm_cf.$(ARCH).xml; fi;
