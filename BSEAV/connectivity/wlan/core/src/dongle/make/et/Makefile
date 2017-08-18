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
# Usage:
#
# gmake [V=1] [{usb|sdio}-{mips|arm}-{debug|nodebug}-
#              {boot|run}-{retail|noretail|mfgtest}-{cdc}]
#
# Use V=1 to display verbose output (all compile lines)
#
# Targets consist of build specs in the above form.  If target(s) are
# specified on the command line, they're validated and built.
# Otherwise, the list of DEFAULT_TARGETS are built.
#
# Example 1: all default targets
#	gmake
#
# Example 2: single target
#	gmake debug-boot-mfgtest-cdc
#
# Example 3: multiple targets
#	gmake debug-{boot,run}-mfgtest-cdc
#
# The first section of this Makefile runs in the parent directory.
# For each build spec, it creates an object directory of the same name,
# descends into it, and re-runs itself.  The second section of this
# Makefile handles the build inside the object directory.
# 

##############################################################################
# PARENT DIRECTORY SECTION
##############################################################################

ifneq ($(INOBJDIR),true)

### flags
BUS = usb
ARCH = mips arm
DEBUG   = debug nodebug
#EXECUTE = xip noxip
STARTUP = boot run
RETAIL = retail noretail mfgtest
PROTO = cdc

TARGET_BASE = $(filter-out \
	debug-run-retail-cdc \
	debug-run-noretail-cdc \
	debug-run-mfgtest-cdc \
	debug-boot-mfgtest-cdc \
	debug-run-mfgtest-cdc \
	debug-boot-mfgtest-rndis \
	debug-run-mfgtest-rndis \
	debug-boot-mfgtest-rndis \
	debug-run-mfgtest-rndis \
	debug-boot-mfgtest-rndis \
	debug-run-noretail-rndis \
	nodebug-boot-mfgtest-cdc \
	nodebug-boot-mfgtest-rndis \
	nodebug-run-mfgtest-cdc, \
$(foreach debug, $(DEBUG),   \
$(foreach star, $(STARTUP), \
$(foreach retl, $(RETAIL), \
$(foreach cdctype, $(PROTO), \
$(debug)-$(star)-$(retl)-$(cdctype) \
)))))

VALID_TARGETS = \
$(foreach bus, $(BUS),   \
$(foreach arch, $(ARCH),   \
$(foreach base, $(TARGET_BASE), \
$(bus)-$(arch)-$(base) \
)))

.PHONY: all
all: $(VALID_TARGETS)

$(VALID_TARGETS): FORCE
	@echo; echo -------- Building $@ --------
	@install -d $@
	@$(MAKE) --no-print-directory -C $@ -f ../Makefile INOBJDIR=true TARGET=$@

helpall helpdef targlist:
	@echo $(VALID_TARGETS) | xargs -n1 echo

.PHONY:
clean:
	rm -rf usb* sdio*

FORCE:

endif	# !INOBJDIR

##############################################################################
# OBJECT DIRECTORY SECTION
##############################################################################

ifeq ($(INOBJDIR),true)

# driver source base
SRCBASE = $(shell /bin/pwd)/../../../..

WLAN_ComponentsInUse := bcmwifi dongle rte
include $(SRCBASE)/makefiles/WLAN_Common.mk

# See below for more options.
EXTRA_DFLAGS += -DBCMDBG_MEM

VPATH += $(WLAN_StdSrcDirsA) $(WLAN_ComponentSrcDirs)
VPATH += $(SRCBASE)/dongle/hndrte/et
VPATH += $(SRCBASE)/et/sys

EXTRA_IFLAGS	+= $(WLAN_ComponentIncPathR) $(WLAN_IncPathR)
EXTRA_IFLAGS	+= -I$(SRCBASE)/et/sys -I$(SRCBASE)/et/include

### basic options
ifeq ($(findstring -arm-,-$(TARGET)-),-arm-)
	TARGET_ARCH = arm
	TARGET_CPU = cm3
else
	TARGET_ARCH = mips
endif

ifeq ($(findstring -boot-,-$(TARGET)-),-boot-)
	STARTUP = boot
	UPGRADE := 1
	BCMNVRAMW := 1
endif
ifeq ($(findstring -run-,-$(TARGET)-),-run-)
	STARTUP = run
endif

ifeq ($(findstring -usb-,-$(TARGET)-),-usb-)
	TARGET_HBUS := usb
endif
ifeq ($(findstring -sdio-,-$(TARGET)-),-sdio-)
	TARGET_HBUS := sdio
endif

ifeq ($(findstring -rndis-,-$(TARGET)-),-rndis-)
	HBUS_PROTO := rndis
	ETCONFFILE := etconfig_rte_rdev
else
	HBUS_PROTO := cdc
endif

ifeq ($(findstring -nodebug-,-$(TARGET)-),-nodebug-)
	DEBUG     = nodebug
endif
ifeq ($(findstring -debug-,-$(TARGET)-),-debug-)
	DEBUG   = debug
	EXTRA_DFLAGS  += -DBCMDBG -DBCMINTERNAL
endif

CONSOLE	:= 1

EXECUTE = noxip
CONFIG_XIP := 0

# EXECUTE = xip
# CONFIG_XIP := 1
# DATA_START	:= 0x80001000

ifeq ($(findstring -retail-,-$(TARGET)-),-retail-)
	RETAIL := 1
	ETLED := 1
	BCMNVRAMW := 1
endif
ifeq ($(findstring -mfgtest-,-$(TARGET)-),-mfgtest-)
	MFGTEST := 1
	ETCONFFILE := etconfig_rte_mfgtest
	BCMNVRAMW := 1
endif

# Uncomment for debug compile
# EXTRA_DFLAGS  += -DBCMDBG -DBCMINTERNAL

# add usb cdc
USBBULK_RXBUFS = 6
USBCTL_RXBUFS = 8
USB_RXBND = 4

include $(SRCBASE)/dongle/make/Makeconf

# add et driver
ETCONFFILE ?= etconfig_rte_dev
ETTUNEFILE ?= ettunable_rte_dev.h
ETCFGDIR   := $(SRCBASE)/et/config
UPDATESH   := $(ETCFGDIR)/diffupdate.sh
# define ET OS flag to pick et_rte.c from et.mk
ETRTE=1
ET=1
include $(ETCFGDIR)/$(ETCONFFILE)
include $(ETCFGDIR)/et.mk
ifeq ($(ETFILES),)
$(error ETFILES is undefined in $(ETCONFFILE))
endif
OBJECTS		+= $(ETFILES:.c=.o)

EXTRA_DFLAGS    += -DBCMET -DBCMETRTE -DBCM47XX_CHOPS -DDMA
EXTRA_DFLAGS    += -I$(SRCBASE)/et/sys $(ETFLAGS)

EXTRA_CFLAGS	+= -fshort-wchar

BCM_DNGL_BL_PID ?= 0xbdc

EXTRA_DFLAGS += -DBCM_DNGL_BL_PID=$(BCM_DNGL_BL_PID)

# for final objects
EXE		:= $(patsubst %, %.exe, $(PROJECT))
BIN		:= $(patsubst %, %.bin, $(PROJECT))
MAP		:= $(patsubst %, %.map, $(PROJECT))
TRX		:= $(patsubst %, %.trx, $(PROJECT))
UPG		:= $(patsubst %, %.upg, $(PROJECT))
NOE		:= $(patsubst %, %.noe, $(PROJECT))
HDR		:= $(patsubst %, %.h, $(PROJECT))
OPT		:= $(patsubst %, %.opt, $(PROJECT))
IMAGES          = $(OPT) $(EXE) $(BIN) $(MAP) $(TRX) $(HDR)

# create upgrade images for retail & flash upgrade builds
ifneq ($(UPGRADE),)
EXTRA_DFLAGS += -DFLASH_UPGRADE
IMAGES += $(UPG) $(NOE)
endif

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/Makerules

all: $(IMAGES)
	@$(SIZE) $(EXE)

ifdef DEPENDS
$(ETFILES:%.c=.%.depend): etconf.h
endif
$(ETFILES:%.c=%.o): etconf.h

etconf.h: $(ETCFGDIR)/$(ETTUNEFILE) FORCE
#	@echo "check and update config file"
	@[ ! -f $@ ] || chmod +w $@
	@cp $< ettemp
	@$(UPDATESH) ettemp $@

$(HDR): $(TRX)
	bin2c $< $@ dlarray

%.upg:	%.bin
	nvserial -i $< -o upgrade.bin -s 57005 $(SRCBASE)/shared/nvram/bcm94320r.txt
	trx -f noheader -o $@ upgrade.bin
	@rm upgrade.bin

FORCE:

endif	# INOBJDIR
