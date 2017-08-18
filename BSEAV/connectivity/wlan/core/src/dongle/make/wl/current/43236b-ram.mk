# Makefile for hndrte based 43236b1 standalone programs
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id:

CHIP_BUILD := enabled

BINDL = $(BIN_TRX)

# chip specification
CHIP	:= 43236
ROMREV	:= b1
REVID	:= 3

# default targets, pool/ipa are on by default
TARGETS := \
	g \
	g-nodis

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cm3
TARGET_HBUS	:= usb
THUMB		:= 1
HBUS_PROTO	:= cdc

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_43236b_dev
WLTUNEFILE	:= wltunable_rte_43236b.h
WLTUNEPARMS	+= WLC_DATAHIWAT

# ROM image info
# ROMOFFLOAD	:= 1
# ROMLDIR		:= $(TOPDIR)/../chipimages/43236b1
# ROMESC		:= $(ROMLDIR)/romesc.txt
# ROMLLIB         := roml.exe
# ROMLMAP         := roml.map
# ROMLBASEADDR	:= $(shell awk '/ text_start$$/ {print "0x" $$1}' $(ROMLDIR)/$(ROMLMAP))
JMPTBL          := 1
JMPTBL_FULL	:= 1
ROMDATBL        := 1
GLOBALIZE	:= 1
# WLPATCHFILE	:= wlc_patch_43236b1.c
# WLCHAN_PATCHFILE:= wlc_patch_channel.c

# features (sync with 43236b.mk, 43236b-roml.mk)
SMALL		:= 0
MEMBASE		:= 0
MEMSIZE		:= 458752	# Hardcoding it saves ~112 bytes from startarm.S
FLASH		:= 0
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ERROR	:= 0
BCMPKTPOOL	:= 1
BCMPKTPOOL_ENABLED := 1
DMATXRC		:= 1
DMATXRC_ENABLED := 1
PROP_TXSTATUS	:= 1
PROP_TXSTATUS_ENABLED := 1
VDEV		:= 1		# p2p dongle code support
NO_BCMINTERNAL  := 1
POOL_LEN	:= 40
USB_IMAGE_UNZ	:= 1
MFGTEST_POOL_LEN := 34
HND_STACK_SIZE := 4096

# USB tunables for full dongle operation
USBBULK_RXBUFS		:= 8
USBCTL_RXBUFS	    := 2
USB_NRXD		    := 32
USB_NTXD		    := 32
USB_RXBND		    := 16
USB_TXQ_DATAHIWAT	:= 2
USB_TXQ_DATALOWAT	:= 1


ifeq ($(call opt,p2p),1)
    POOL_LEN := 24
endif

USBCTL_RXBUFS		:= 2
USB_NRXD		:= 32
USB_NTXD		:= 32
USB_TXQ_DATAHIWAT	:= 2
USB_TXQ_DATALOWAT	:= 1


#radio specific defines
BCMRADIOREV	:= 0x7
BCMRADIOVER	:= 0x2
BCMRADIOID	:= 0x2057

BIN_TRX_OPTIONS_SUFFIX := -x -x0
# extra flags
EXTRA_DFLAGS	+= -I$(SRC)/dongle/rte/roml/43236b1

# 43236b1 radio rev, ver, id
EXTRA_DFLAGS	+= -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS	+= -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS	+= -DBCMRADIOID=$(BCMRADIOID)

#EXTRA_DFLAGS	+= -DBCMTRAPTEST	# include 'tr' command to test trap handler
#EXTRA_DFLAGS	+= -DBCMDBG_POOL

EXTRA_DFLAGS    += -DUSB43236 -DBCMUSBDEV_BULKIN_2EP
EXTRA_DFLAGS	+= -DBCMUSBDEV_ENABLED -DBCM_VARS_APPEND
#EXTRA_DFLAGS    += -DWLPHY_IPA_ONLY
EXTRA_DFLAGS    += -DHND_STACK_SIZE=$(HND_STACK_SIZE)


ifeq ($(call opt,epa),1)
	EXTRA_DFLAGS    += -DWLPHY_EPA_ONLY
else
	EXTRA_DFLAGS    += -DWLPHY_IPA_ONLY
endif

ifeq ($(call opt,nodis),1)
EXTRA_DFLAGS += -DSEPARATE_EP_FOR_RPC
endif

#window size of 64
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_MAX_WSIZE_64

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT

# enable GPIO interrupt handling
EXTRA_DFLAGS	+= -DWLGPIOHLR

# WLMEDIA is enabled ==> WLMEDIA_RATESTATS is also enabled, this has a structure change for later chips
EXTRA_DFLAGS += -DNIN_LEGACY_ROM

ifeq ($(call opt,dlystat),1)
	EXTRA_DFLAGS += -DWLPKTDLYSTAT -DWLMEDIA_EXT
endif

#use ucode  only aggregation
WLAMPDU_UCODE_ONLY := 1

#wowl gpio pin3 polarity at logic low is 1
WOWL_GPIOPIN = 0x3
WOWL_GPIO_POLARITY = 0x1
EXTRA_DFLAGS    += -DWOWL_GPIO=$(WOWL_GPIOPIN)  -DWOWL_GPIO_POLARITY=$(WOWL_GPIO_POLARITY)
