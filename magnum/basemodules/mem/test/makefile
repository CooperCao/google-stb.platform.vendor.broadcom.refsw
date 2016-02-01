# Yes, this is linuxuser specific.
# Needs to be changed, or moved.

TARGET_OS = linux
CHIP_NAME = 7038

DEFINES = -DLINUX \
	  -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE \
	  -DBCHP_CHIP=$(CHIP_NAME) \
	  -DBMEM_SAFETY_CONFIG=BMEM_CONFIG_SAFEST \
	  -DBMEM_BOOKKEEPING_CONFIG=BMEM_BOOKKEEPING_LOCAL \
	  -DBMEM_REENTRANT_CONFIG=BMEM_REENTRANT \
	  -DBCHP_REV_B0 \
	  -DBCHP_FAMILY=BCHP_FAMILY_HYDRA \
	  -DBDBG_DEBUG_BUILD=1 \
	  -DBCHP_FAMILY_REV_B0 

MAGNUM=$(shell cd ../../..; pwd)
include $(MAGNUM)/basemodules/kni/linuxuser/testapp/makefile.common

TARGET=bmem_test

#CROSS_COMPILE = /opt/toolchains/mipsel/bin/mipsel-linux-
#CC	= $(CROSS_COMPILE)gcc

all: $(TARGET)

MEMMGR=$(MAGNUM)/basemodules/mem
MAGNUM_OBJS+=\
	bmem_dotest.o \
	bmem_test.o \
	$(MEMMGR)/bmem.o \
	$(MEMMGR)/bmem_debug.o \
	$(MEMMGR)/bmem_local.o \
	$(MEMMGR)/bmem_system.o
MAGNUM_DEPENDS=$(MAGNUM_OBJS:.o=.d)

ifeq ($(debug), 1)
	DEFINES += -DBMEM_TEST_DEBUG
endif

ifeq "$(safety_cfg)" "fastest"
	DEFINES += -DBMEM_SAFETY_CONFIG=BMEM_CONFIG_FASTEST
endif

ifeq "$(safety_cfg)" "fast"
	DEFINES += -DBMEM_SAFETY_CONFIG=BMEM_CONFIG_FAST
endif

ifeq "$(safety_cfg)" "normal"
	DEFINES += -DBMEM_SAFETY_CONFIG=BMEM_CONFIG_NORMAL
endif

ifeq "$(safety_cfg)" "safe"
	DEFINES += -DBMEM_SAFETY_CONFIG=BMEM_CONFIG_SAFE
endif

CFLAGS += \
	-I$(MEMMGR) -DBDBG_DEBUG_BUILD=1 -MMD -DDEBUG $(DEFINES)


$(TARGET): $(LIB) $(MAGNUM_OBJS)
	$(CC) $(LIB) $(CFLAGS) -lpthread -o $@ $^ $(LDFLAGS)

clean:
	rm $(MAGNUM_OBJS) $(TARGET) *.o *.d core*

-include $(MAGNUM_DEPENDS)
