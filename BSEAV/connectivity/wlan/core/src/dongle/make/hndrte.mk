# hndrte specific configurations

RTOS_OBJECTS  += hndrte.o
ifeq ($(TARGET_ARCH),mips)
	RTOS_OBJECTS  += hndrte_mips.o
endif
ifeq ($(TARGET_ARCH),arm)
	RTOS_OBJECTS  += hndrte_arm.o
endif

EXTRA_DFLAGS += -D_HNDRTE_ -DGLOBAL_STACK

vpath %.c $(SRCBASE)/dongle/rte

EXTRA_IFLAGS += -I$(SRCBASE)/dongle/rte
