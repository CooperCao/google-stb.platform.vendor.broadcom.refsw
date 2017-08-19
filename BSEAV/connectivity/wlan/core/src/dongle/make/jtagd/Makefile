#
# Makefile for usb download code based on hndrte
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#


# driver source base
ifeq ($(SRCBASE),)
SRCBASE = $(shell /bin/pwd)/../../..
endif

WLAN_ComponentInUse := bcmwifi dongle rte
include $(SRCBASE)/makefiles/WLAN_Common.mk

SRCDIRS := $(SRCBASE)/dongle/hndrte/jtagd $(SRCBASE)/usbdev/dongle
SRCDIRS += $(WLAN_StdSrcDirsA) $(WLAN_ComponentSrcDirsA)

VPATH := $(SRCDIRS)

EXTRA_IFLAGS := -I$(SRCBASE)/usbdev/dongle
EXTRA_IFLAGS += $(WLAN_ComponentIncPathA) $(WLAN_IncPathA)

# The following are the possible choices for each component
STARTUP = boot run
ARCH = arm-thumb-cm3 arm-thumb mips
DISC = dis nodis
DEB = debug nodebug

ALL_TARGETS = \
	run-arm-thumb-cm3-dis-debug \
	run-arm-thumb-cm3-dis-nodebug \
	run-arm-thumb-cm3-nodis-debug \
	run-arm-thumb-cm3-nodis-nodebug \
	run-arm-thumb-dis-debug \
	run-arm-thumb-dis-nodebug \
	run-arm-thumb-nodis-debug \
	run-arm-thumb-nodis-nodebug \
	run-mips-dis-debug \
	run-mips-dis-nodebug \
	run-mips-nodis-debug \
	run-mips-nodis-nodebug

# This target list tries to get decent coverage for nightly build.
TARGETS = \
	run-arm-thumb-cm3-dis-nodebug \
	run-mips-nodis-nodebug

# Uncomment out if wanted
EXTRA_DFLAGS	+= -DBCMINTERNAL
EXTRA_LDFLAGS	+= -M
#CONFIG_XIP	:= 1

EXTRA_DFLAGS	+= -DRTE_CONS -DUNRELEASEDCHIP
EXTRA_DFLAGS	+= -DRTE_POLL

# Include both 32 & 64 bit dma.
EXTRA_DFLAGS += -DBCMDMA32

ifeq ($(findstring -debug,$(TARGET)),-debug)
	EXTRA_DFLAGS += -DBCMDBG -DBCMDBG_MEM
else
	EXTRA_DFLAGS += -DBCMDBG_ASSERT
endif

# BUS disconnect/reenumeration
ifeq ($(findstring -nodis-,$(TARGET)),-nodis-)
	NODIS := 1
	EXTRA_DFLAGS += -DBCMUSB_NODISCONNECT
	EXTRA_DFLAGS += -DBCM_DNGL_BL_PID=0x4a44
endif

ifeq ($(findstring arm,$(TARGET)),arm)
	TARGET_ARCH	= arm
	ifeq ($(findstring cm3,$(TARGET)),cm3)
		TARGET_CPU = cm3
	endif
	ifeq ($(findstring -thumb-,$(TARGET)),-thumb-)
		THUMB = 1
	endif
else
	TARGET_ARCH	= mips
endif

EXTRA_DFLAGS	+= -DBCMBUSTYPE=SI_BUS \
		   -DUSBBULK_RXBUFS=4 -DUSB_NTXD=8 -DUSBCTL_RXBUFS=4 \
		   -DUSB_NRXD=8 -DUSB_RXBND=-1 -DUSBCTLBUFSZ=128 \
		   -DUSBBULK_POSTBUFS=8 -DUSBCTL_POSTBUFS=4
EXTRA_CFLAGS	+= -fshort-wchar

ifeq ($(TARGET_ARCH),mips)
	BOOT_OBJECTS	:= boot.o trap.o sisdram.o sbsdram.o aisdram.o
	RUN_OBJECTS	:= head.o trap.o
	ARCH_OBJECTS	:= hndrte_mips.o hndmips.o
else
	BOOT_OBJECTS	:= bootarm.o
	RUN_OBJECTS	:= headarm.o
	ARCH_OBJECTS	:= hndrte_arm.o rte_arm.o hndarm.o
endif

ifeq ($(findstring boot-,$(TARGET)),boot-)
	STARTUP = boot
	OBJECTS = $(BOOT_OBJECTS)
	TRX_OPTIONS	+= -x 0x80001000 -x 0x80001000
	TRX_OFFSET1	:= 0x80001000
	TRX_FLAGS	:= 0x0020
else
	STARTUP = run
	OBJECTS = $(RUN_OBJECTS)
	# download & jumpto locations for image downloaded to bootloader
	ifeq ($(TARGET_ARCH),arm)
		ifeq ($(TARGET_CPU),cm3)
			TRX_OPTIONS	+= -x 0x60000000 -x 0x00000080
			TRX_OFFSET1	:= 0x00000081
			TRX_FLAGS	:= 0x0020
		else
			TRX_OPTIONS	+= -x 0x80000000 -x 0x00000000
			TRX_OFFSET1	:= 0x00000000
			TRX_FLAGS	:= 0x0020
		endif
	else
		TRX_OPTIONS	+= -x 0x80001000 -x 0x80001000
		TRX_OFFSET1	:= 0x80001000
		TRX_FLAGS	:= 0x0020
	endif
endif

# Common code
OBJECTS		+= $(PROGRAM).o
OBJECTS		+= siutils.o sbutils.o aiutils.o bcmotp.o bcmutils.o hnd_pktq.o hnd_pktpool.o bcmstdlib.o \
		hndchipc.o hndpmu.o rte_osl.o rte.o rte_cons.o hndrte.o rte_dev.o rte_mem.o rte_pktpool.o \
		rte_heap.o rte_fetch.o rte_isr.o rte_chipc.o rte_uart.o rte_pmu.o
OBJECTS		+= $(ARCH_OBJECTS)

# JTAGD code
OBJECTS		+= sromstubs.o nvramstubs.o hnd_pkt.o hnd_lbuf.o hnddma.o hnddma_rx.o hnddma_tx.o
OBJECTS 	+= usbdev_sb.o usbdev_jtagd.o

PROGRAM		= jtagd
EXE		:= $(patsubst %, %.exe, $(PROGRAM))
BIN		:= $(patsubst %, %.bin, $(PROGRAM))
TRX		:= $(patsubst %, %.trx, $(PROGRAM))
UTRX		:= $(patsubst %, %.bin.trx, $(PROGRAM))
DIS		:= $(patsubst %, %.dis, $(PROGRAM))
MAP		:= $(patsubst %, %.map, $(PROGRAM))
IMAGES		= $(EXE) $(BIN) $(TRX) $(UTRX) $(DIS) $(MAP)

# include common rule after all configration is setup
include $(SRCBASE)/dongle/make/Makeconf
include $(SRCBASE)/dongle/make/Makerules

# build objdir
OBJDIR_BASE = $(TARGET)
OBJDIR  := $(strip $(subst --,,$(addprefix -,$(addsuffix -,$(OBJDIR_BASE)))))

### build targets
all: $(TARGETS)
	@echo "TARGETS=$(TARGETS) IMAGES=$(IMAGES)"

ifneq ($(INOBJDIR),true)
.DEFAULT:
	@echo "default-submake: MAKELEVEL=$(MAKELEVEL) MAKECMDGOALS=$(MAKECMDGOALS)"
	$(MAKE) TARGET=-$@- all
endif

$(TARGETS): FORCE
	$(MAKE) TARGET=-$@- objdir

objdir:
	@echo "objdir: MAKELEVEL=$(MAKELEVEL) TARGET=$(TARGET)"
	install -d $(OBJDIR)
	@echo "OBJECTS=$(OBJECTS)"
	$(MAKE) -C $(OBJDIR) -f ../Makefile SRCBASE=$(SRCBASE) INOBJDIR=true images

images: $(IMAGES)
	@echo "done: MAKELEVEL=$(MAKELEVEL) images=$(IMAGES)"

# Dependencies
$(EXE): $(OBJECTS)
$(EXE): %.exe: %.lds
$(EXE): %.exe: %.o

clean:
	rm -rf $(TARGETS)

FORCE:

PHONY: all objdir clean
