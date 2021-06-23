# XAL_PATH: path to the XTRATUM directory
XAL_PATH=../..

CONTAINER = container.bin
RSW = resident_sw

all: $(CONTAINER) $(RSW)

include $(XAL_PATH)/common/rules.mk

# XMLCF: path to the XML configuration file
XMLCF=xm_cf.$(ARCH).xml

# PARTITIONS: partition files (xef format) composing the example
SRCS := $(sort $(wildcard *.cpp))

CRTI_OBJ:=$(shell $(TARGET_CXX) $(TARGET_CXXFLAGS) -print-file-name=crti.o)
CRTBEGIN_OBJ:=$(shell $(TARGET_CXX) $(TARGET_CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(TARGET_CXX) $(TARGET_CXXFLAGS) -print-file-name=crtend.o)
CRTN_OBJ:=$(shell $(TARGET_CXX) $(TARGET_CXXFLAGS) -print-file-name=crtn.o)

#OBJS := $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(patsubst %.cpp,%.o, $(SRCS)) $(patsubst %.S,%.o, $(ASRCS)) $(CRTEND_OBJ) $(CRTN_OBJ)
OBJS :=  $(patsubst %.cpp,%.o, $(SRCS)) 

PARTITIONS=partition0.xef #partition1.xef

partition0: $(OBJS) $(XMLCF)
	$(TARGET_LD) -o $@ $(OBJS) $(TARGET_LDFLAGS) -u init_array -Ttext=$(shell $(XPATHSTART) 0 $(XMLCF))
	
PACK_ARGS=-h $(XMCORE):xm_cf.xef.xmc \
	-p 0:partition0.xef# \
	-p 1:partition1.xef

$(CONTAINER): $(PARTITIONS) xm_cf.xef.xmc
	$(XMPACK) check xm_cf.xef.xmc $(PACK_ARGS)
	$(XMPACK) build $(PACK_ARGS) $@

