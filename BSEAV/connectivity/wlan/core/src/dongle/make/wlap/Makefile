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
# This Make expands input argument to FULL target, then create objdir and build each one
#
# Usage:
#
# make 
#      [debug|nodebug]-[xip|noxip]-[run|boot]

ifneq ($(INOBJDIR),true)

# The following are the possible choices for each component
DEBUG = nodebug debug
ARCH = mips arm-thumb arm-thumb-cm3
EXECUTE = noxip xip
STARTUP = run run-roml boot
WLUSER = nowlu wlu

ALL_TARGETS = \
	debug-arm-thumb-cm3-noxip-run-wlu \
	debug-arm-thumb-noxip-boot-nowlu \
	debug-arm-thumb-noxip-run-nowlu \
	debug-arm-thumb-noxip-run-roml-nowlu \
	debug-arm-thumb-xip-boot-nowlu \
	debug-arm-thumb-xip-boot-wlu \
	debug-mips-noxip-boot-nowlu \
	debug-mips-noxip-run-nowlu \
	debug-mips-xip-boot-nowlu \
	debug-mips-xip-boot-wlu \
	nodebug-arm-thumb-cm3-noxip-run-wlu \
	nodebug-arm-thumb-noxip-boot-nowlu \
	nodebug-arm-thumb-noxip-run-nowlu \
	nodebug-arm-thumb-noxip-run-roml-nowlu \
	nodebug-arm-thumb-noxip-run-roml-wlu \
	nodebug-arm-thumb-xip-boot-nowlu \
	nodebug-arm-thumb-xip-boot-wlu \
	nodebug-mips-noxip-boot-nowlu \
	nodebug-mips-noxip-run-nowlu \
	nodebug-mips-xip-boot-nowlu \
	nodebug-mips-xip-boot-wlu

# This target list tries to get decent coverage for nightly build.
TARGETS = \
	debug-arm-thumb-cm3-noxip-run-wlu \
	debug-arm-thumb-xip-run-roml-nowlu \
	nodebug-mips-xip-boot-wlu

### build targets
all: $(TARGETS)

.PHONY: $(TARGETS)
$(TARGETS):
	@echo "TARGET=$@"
	install -d $@
	$(MAKE) -C $@ -f ../Makefile INOBJDIR=true

.PHONY: clean
clean:
	rm -rf debug-* nodebug-*

endif # !INOBJDIR

##################################################################
# Second-level build of individual object directory
##################################################################

ifeq ($(INOBJDIR),true)

# driver source base
ifeq ($(SRCBASE),)
SRCBASE = $(shell /bin/pwd)/../../../..
endif

WLAN_ComponentsInUse := bcmwifi clm-api lwip ppr keymgmt iocv dump phy dongle rte msch chctx
include $(SRCBASE)/makefiles/WLAN_Common.mk

CHIP = 4328
CHIPREV = a0

FULLDNGLBLD	:= 0

ifeq ($(findstring -arm-,$(TARGET)),-arm-)
	TARGET_ARCH = arm
ifeq ($(findstring -cm3-,$(TARGET)),-cm3-)
	TARGET_CPU = cm3
	THUMB = 1
endif
ifeq ($(findstring -thumb-,$(TARGET)),-thumb-)
	THUMB = 1
endif
else
	TARGET_ARCH = mips
endif

ifeq ($(findstring -nodebug-,$(TARGET)),-nodebug-)
	DEBUG     = nodebug
endif
ifeq ($(findstring -debug-,$(TARGET)),-debug-)
	DEBUG   = debug
	EXTRA_DFLAGS  += -DBCMDBG -DBCMINTERNAL
endif

ifeq ($(findstring -noxip-,$(TARGET)),-noxip-)
	CONFIG_XIP := 0
	RECLAIM = 1
endif
ifeq ($(findstring -xip-,$(TARGET)),-xip-)
	CONFIG_XIP := 1
ifeq ($(TARGET_ARCH),mips)
	DATA_START	:= 0x80001000
else
	DATA_START	:= 0x00001000
endif
endif

BCMNVRAMW = 1

EXTRA_DFLAGS	+= -DBCMDBG_MEM

ifeq ($(findstring roml-,$(TARGET)),roml-)
	ROMOFFLOAD = 1
	ROMREV = $(CHIPREV)
	ROMOFFLOAD := 1
endif

ifeq ($(findstring -boot-,-$(TARGET)-),-boot-)
	STARTUP := boot
endif

CONSOLE = 1

ifeq ($(findstring -nowlu-,$(TARGET)),-nowlu-)
	WLUSER = nowlu
endif
ifeq ($(findstring -wlu-,$(TARGET)),-wlu-)
	WLUSER    = wlu
# wl command line functionallity: 70KB text, 5.5KB data; and 8KB bss added to exe
	EXTERN_OBJECTS  += wlu.o wlu_common.o wlu_cmd.o wlu_iov.o miniopt.o bcm_app_utils.o
	EXTRA_DFLAGS   += -DCONFIG_WLU
endif

PROGRAM	        = wlap

# for final objects
EXE		:= $(patsubst %, %.exe, $(PROGRAM))
BIN		:= $(patsubst %, %.bin, $(PROGRAM))
MAP		:= $(patsubst %, %.map, $(PROGRAM))
TRX		:= $(patsubst %, %.trx, $(PROGRAM))
OPT		:= $(patsubst %, %.opt, $(PROGRAM))
IMAGES          = $(OPT) $(EXE) $(BIN) $(MAP) $(TRX)

# add wl driver
WLCONFFILE ?= wlconfig_rte_dev2
WLTUNEFILE ?= wltunable_rte_dev.h

# include configuration
include $(SRCBASE)/dongle/make/Makeconf

# define WL OS flag to pick wl_rte.c from wl.mk
WLRTE=1
WLCFGDIR := $(SRCBASE)/wl/config
include $(WLCFGDIR)/$(WLCONFFILE)
include $(WLCFGDIR)/wl.mk
OBJECTS	+= $(WLFILES:.c=.o)

SRCDIRS := $(addprefix $(SRCBASE)/,dongle/hndrte/wlap \
		   shared/nvram usbdev/dongle wl/exe)
SRCDIRS += $(WLAN_StdSrcDirsA) $(WLAN_ComponentSrcDirsA)

VPATH += $(SRCDIRS)

EXTRA_IFLAGS	+= $(WLAN_ComponentIncPathR)
EXTRA_IFLAGS	+= $(WLAN_IncPathR)

EXTRA_DFLAGS	+= $(WLFLAGS)

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/Makerules

all: $(IMAGES)
	@$(SIZE) $(EXE)

endif	# INOBJDIR=true
