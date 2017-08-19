#
# Makefile for standalone programs based on hndrte
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

##################################################################
# Top-level build
##################################################################

THREADX ?= 1
HNDRTE ?= 0

# Note: only 'hello' is enabled by default (nightly build)
PROGRAMS += hello
PROGRAMS += armdiags
PROGRAMS += traps
ifeq ($(HNDRTE),1)
PROGRAMS += test
endif
PROGRAMS += memtest
PROGRAMS += nvram
PROGRAMS += srdump
PROGRAMS += nvotp
PROGRAMS += otpcid
PROGRAMS += otpdump
PROGRAMS += scanpci
PROGRAMS += testpci
PROGRAMS += tstjmp
PROGRAMS += justjmp
PROGRAMS += sleeptest
PROGRAMS += testeci
PROGRAMS += otprde
PROGRAMS += papdstub
PROGRAMS += patch
PROGRAMS += testtcam

BCMQT ?= 0
PRINT_NVRAM ?= 0

ifneq ($(INOBJDIR),true)

# The following are the possible choices for each component
#STARTUP = run boot
#ARCH = arm mips
#CPU = cm3 cr4 ca7 ca8
#ISA = thumb
#INTS = int poll
#SI = ai sb
#CACHED = uc c

# Specify which variants to build for each entry in $(PROGRAMS).
# This target list tries to get decent coverage for nightly build.
TARGET_DIRS = \
	run-arm-thumb-cm3-int-ai-uc \
	run-arm-thumb-cm3-int-ai-uc-tcam \
	run-arm-thumb-cm3-int-ai-uc-43602 \
	run-arm-thumb-cr4 \
	run-arm-thumb-cr4-flops \
	run-arm-thumb-cr4-43602 \
	run-arm-thumb-cr4-flops-4347-qt \
	run-arm-thumb-ca7 \
	run-arm-thumb-ca7-flops

ifeq ($(HNDRTE),1)
TARGET_DIRS += \
	run-mips-int-sb-c \
	boot-mips-poll-sb-uc
endif

TARGETS = $(strip $(TARGET_DIRS))

### build targets
all: $(TARGETS)

$(TARGETS): FORCE
	@$(MAKE) --no-print-directory TARGET=$@ objdir

.PHONY: showtgts
showtgts:
	@echo $(TARGETS) | xargs -n 1 echo

.PHONY: clean
clean:
	rm -rf boot-* run-*

FORCE:

##################################################################
# Second-level build of individual object directory
##################################################################

# build objdir
OBJDIR = $(TARGET)

.PHONY: objdir
objdir:
	@echo "TARGET=$(TARGET) PROGRAMS=$(PROGRAMS)"
	@install -d $(OBJDIR)
	@$(MAKE) -C $(OBJDIR) -f ../Makefile --no-print-directory INOBJDIR=true $(PROGRAMS)

endif # !INOBJDIR

##################################################################
# Third-level build of programs within object directory
##################################################################

# driver source base
ifeq ($(SRCBASE),)
export SRCBASE = $(shell /bin/pwd)/../../..
endif

ifeq ($(PROGRAM),)
.NOTPARALLEL:

$(PROGRAMS):
	@echo "Building $(TARGET)/$@"
	@$(MAKE) -f ../Makefile --no-print-directory \
		INOBJDIR=true TARGET=$(TARGET) PROGRAM=$@
endif # !PROGRAM

##################################################################
# Fourth-level build of inside of object directory
##################################################################

ifneq ($(PROGRAM),)

WLAN_ComponentsInUse :=
include $(SRCBASE)/makefiles/WLAN_Common.mk

SRCDIRS := $(SRCBASE)/dongle/rte/test $(SRCBASE)/shared/nvram $(SRCBASE)/dongle/threadx/test
SRCDIRS += $(WLAN_StdSrcDirsA) $(WLAN_ComponentSrcDirsA)

VPATH += $(SRCDIRS)

EXTRA_IFLAGS := $(WLAN_IncPathR) $(WLAN_ComponentIncPathR)

# Treat test programs part of RTE or whatever components they are testing
# so allow xxx_priv.h inclusions in test .c files
EXTRA_IFLAGS += -I$(SRCBASE)/rte/src

# Comment out if not wanted
EXTRA_DFLAGS += -DBCMDBG -DBCMINTERNAL -DSBPCI
EXTRA_DFLAGS += -DBCMNVRAMW
EXTRA_DFLAGS += -DRTE_CONS
EXTRA_DFLAGS += -DRTE_TEST -DBCM_DONGLEVARS

TGTSTR = -$(TARGET)-

ifeq ($(findstring -uc-,$(TGTSTR)),-uc-)
	EXTRA_DFLAGS += -DRTE_UNCACHED
endif

#default keep the backplane setup to be AXI
ifeq ($(findstring -sb-,$(TGTSTR)),-sb-)
	EXTRA_DFLAGS += -DBCMCHIPTYPE=SOCI_SB
else
	EXTRA_DFLAGS += -DBCMCHIPTYPE=SOCI_AI
endif

ifeq ($(findstring -poll-,$(TGTSTR)),-poll-)
	EXTRA_DFLAGS += -DRTE_POLL
endif

ifeq ($(findstring -flops-,$(TGTSTR)),-flops-)
	EXTRA_DFLAGS += -DFLOPS_SUPPORT
	HBUS_ENAB := pcie
	FLOPS_SUPPORT = 1
ifeq ($(findstring -ca7-,$(TGTSTR)),-ca7-)
	TEXT_START = 0x00200800
else
	TEXT_START = 0x00180800
endif
endif

ifeq ($(findstring -43602-,$(TGTSTR)),-43602-)
	HBUS_ENAB := pcie
ifeq ($(findstring -cm3-,$(TGTSTR)),-cm3-)
	TEXT_START = 0x00300000
	EXTRA_DFLAGS += -DVTABLE_RELLOC
else
	EXTRA_DFLAGS += -DFLOPS_SUPPORT=1
	FLOPS_SUPPORT = 1
	TEXT_START = 0x00180000
endif
endif

ifeq ($(findstring -qt-,$(TGTSTR)),-qt-)
	BCMQT = 1
endif

ifeq ($(findstring -4347-,$(TGTSTR)),-4347-)
	MEMSIZE		:= 1376256
	MEMBASE	:= 0x170000
	TEXT_START = 0x00170000
endif

#0 - 1 Consecutive locations are patched
#1 - 2 Consecutive locations are patched
#2 - 4 Consecutive locations are patched
#3 - 8 Consecutive locations are patched
#4 - 16 Consecutive locations are patched
ifeq ($(findstring -tcam-,$(TGTSTR)),-tcam-)
	EXTRA_DFLAGS += -DPATCHCOUNT=1
endif

ifeq ($(findstring -arm-,$(TGTSTR)),-arm-)
	TARGET_ARCH = arm
ifeq ($(findstring -cm3-,$(TGTSTR)),-cm3-)
	TARGET_CPU = cm3
endif
ifeq ($(findstring -cr4-,$(TGTSTR)),-cr4-)
	TARGET_CPU = cr4
endif
ifeq ($(findstring -ca7-,$(TGTSTR)),-ca7-)
	TARGET_CPU = ca7
endif
ifeq ($(findstring -ca8-,$(TGTSTR)),-ca8-)
	TARGET_CPU = ca8
endif
ifeq ($(findstring -thumb-,$(TGTSTR)),-thumb-)
	THUMB = 1
endif
	EXTRA_DFLAGS += -DSB_PCMCIA_SROM -DBCMUSBDEV -DBCMHOSTVARS -DBCMCCISSR3
else
	TARGET_ARCH = mips
endif

EXTRA_DFLAGS += -DUNRELEASEDCHIP -DBCMDMA32

ifdef MEMSIZE
	EXTRA_DFLAGS += -DMEMSIZE=$(MEMSIZE)
endif

ifeq ($(TARGET_ARCH),mips)
	BOOT_OBJECTS	:= boot.o trap.o sisdram.o sbsdram.o aisdram.o
	RUN_OBJECTS	:= head.o trap.o
	ARCH_OBJECTS	:= hndrte_mips.o hndmips.o
else
	BOOT_OBJECTS	:= bootarm.o
	RUN_OBJECTS	:= headarm.o
	ARCH_OBJECTS	:= rte_arm.o hndarm.o
endif

ifeq ($(findstring -run-,$(TGTSTR)),-run-)
	OBJECTS = $(RUN_OBJECTS)
	ifeq ($(TARGET_ARCH),arm)
		ifeq ($(TARGET_CPU),cm3)
			TRX_OPTIONS	+= -x 0x60000000 -x 0x00000081
		else
			TRX_OPTIONS	+= -x 0x80000000 -x 0x00000000
		endif
	else
		TRX_OPTIONS	+= -x 0x80001000 -x 0x80001000
	endif
else
	OBJECTS = $(BOOT_OBJECTS)
endif

# jumpto locations and flag to indicate uncompressed image to bootloader
ifeq ($(findstring -run-,$(TGTSTR)),-run-)
	ifeq ($(TARGET_ARCH),arm)
		ifeq ($(TARGET_CPU),cm3)
			TRX_OFFSET1	:= 0x00000081
			TRX_FLAGS	:= 0x0020
		else
			TRX_OFFSET1	:= 0x00000000
			TRX_FLAGS	:= 0x0020
		endif
	endif
	ifeq ($(TARGET_ARCH),mips)
		TRX_OFFSET1	:= 0x80001000
		TRX_FLAGS	:= 0x0020
	endif
endif

TRX_OFFSET1	?= 0x0
TRX_FLAGS	?= 0x0

# special support for QT
ifneq ($(BCMQT),0)
	EXTRA_DFLAGS += -DBCMQT
endif

ifneq ($(PRINT_NVRAM),0)
	EXTRA_DFLAGS += -DPRINT_NVRAM -DBCM_DONGLEVARS
endif

HNDLBUFCOMPACT := 0
RECLAIM_PROTECT_ENAB := 0

EXTRA_DFLAGS += -DBCMSTANDALONE_TEST

EXE = $(PROGRAM).exe
BIN = $(PROGRAM).bin
DIS = $(PROGRAM).dis
MAP = $(PROGRAM).map
TRX = $(PROGRAM).trx
OPT = $(PROGRAM).opt
BIN_TRX = $(PROGRAM).bin.trx
QT = $(PROGRAM).qt
TRX_NVM = $(PROGRAM).nvm.trx
IMAGES = $(OPT) $(EXE) $(BIN) $(DIS) $(MAP) $(TRX) $(BIN_TRX) $(QT)
# create your own nvm.txt file to test nvram download
NVM_FILE := $(wildcard $(SRCBASE)/dongle/rte/test/nvm.txt)
ifneq ($(findstring nvm.txt,$(NVM_FILE)),)
	IMAGES 	+= $(TRX_NVM)
endif

# Override ARM toolchain for testing of new compiler
#arm-none-eabi-TOOLSVER=2010.09

include $(SRCBASE)/dongle/make/Makeconf

OBJECTS		+= $(PROGRAM).o
OBJECTS		+= c_main.o
OBJECTS		+= $(ARCH_OBJECTS) siutils.o sbutils.o aiutils.o
OBJECTS		+= hndpmu.o hndlhl.o hndchipc.o hndpci.o nicpci.o pcie_core.o
OBJECTS		+= bcmutils.o hnd_pkt.o hnd_lbuf.o bcmstdlib.o bcmxtlv.o
OBJECTS		+= rte_osl.o rte.o rte_uart.o rte_cons.o bcmsrom.o
OBJECTS		+= nvram_ro.o sflash.o bcmotp.o
OBJECTS		+= hndtcam.o hnd_gci.o
#OBJECTS	+= patchtable.o
OBJECTS		+= pciedma.o
OBJECTS		+= bme.o hnd_pktq.o hnd_pktpool.o rte_dev.o rte_fetch.o rte_pktfetch.o
OBJECTS		+= rte_heap.o rte_mem.o hnd_pt.o rte_isr.o rte_chipc.o rte_pmu.o rte_pktpool.o
OBJECTS		+= cc_uart.o seci_uart.o
OBJECTS		+= $(RTOS_OBJECTS)
OBJECTS		:= $(sort $(OBJECTS))

include $(SRCBASE)/dongle/make/Makerules

all: $(IMAGES)
	@$(SIZE) $(EXE)

# build uncompressed trx with nvram
%.nvm.trx: $(PROGRAM).bin $(NVM_FILE)
	( \
	BIN_SIZE="`stat -c %s $<`"; \
	TMPFILE="tmp_$@"; \
	rm -rf $$TMPFILE && \
	nvserial -a -s 57005 -o $(PROGRAM).nvm $(NVM_FILE) && \
	cat $(PROGRAM).bin $(PROGRAM).nvm >> $$TMPFILE && \
	NVMSIZE="`stat -c %s $(PROGRAM).nvm`" && \
	trx -f $(TRX_FLAGS) -x $$BIN_SIZE -x $(TRX_OFFSET1) -x $$NVMSIZE -o $@ $$TMPFILE && \
	rm $$TMPFILE \
	)

endif	# PROGRAM
