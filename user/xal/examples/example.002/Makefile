# XAL_PATH: path to the XTRATUM directory
XAL_PATH=../..

CONTAINER = container.bin
RSW = resident_sw.elf

all: $(CONTAINER) $(RSW)
include $(XAL_PATH)/common/rules.mk

# XMLCF: path to the XML configuration file
XMLCF=xm_cf.$(ARCH).xml

# PARTITIONS: partition files (xef format) composing the example
PARTITIONS=partition0.xef partition1.xef partition2.xef

partition0: partition.o $(EXTRA_OBJS)
	$(TARGET_LD) -o $@ $^ $(TARGET_LDFLAGS) -Ttext=$(shell $(XPATHSTART) 0 $(XMLCF))

partition1: partition.o $(EXTRA_OBJS)
	$(TARGET_LD) -o $@ $^ $(TARGET_LDFLAGS) -Ttext=$(shell $(XPATHSTART) 1 $(XMLCF))

partition2: reader.o $(EXTRA_OBJS)
	$(TARGET_LD) -o $@ $^ $(TARGET_LDFLAGS) -Ttext=$(shell $(XPATHSTART) 2 $(XMLCF))

resident_sw.elf: resident_sw
	cp resident_sw resident_sw.elf

PACK_ARGS=-h $(XMCORE):xm_cf.xef.xmc \
	-p 0:partition0.xef \
	-p 1:partition1.xef \
	-p 2:partition2.xef

$(CONTAINER): $(PARTITIONS) xm_cf.xef.xmc
	$(XMPACK) check xm_cf.xef.xmc $(PACK_ARGS)
	$(XMPACK) build $(PACK_ARGS) $@
	@exec echo -en "> Done [container]\n"


