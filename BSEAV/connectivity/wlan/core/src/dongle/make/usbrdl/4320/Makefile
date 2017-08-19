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

SRCBASE = $(shell /bin/pwd)/../../../..

TARGETS	:= boot-rom

TARGET_ARCH := mips

DATA_BASE := 0x80001000
DATA_START := 0x8005d000

BCM_ID_PRODUCT := 0xbd11

EXTRA_DFLAGS += -DRTE_UNCACHED -DSB_PCMCIA_SROM
EXTRA_DFLAGS += -DBCMCHIPTYPE=SOCI_SB -DBCMBUSTYPE=SI_BUS
EXTRA_DFLAGS += -DHND_STACK_SIZE=6144
EXTRA_DFLAGS += -DDL_NVRAM=2044
EXTRA_DFLAGS += -DUSBBULK_RXBUFS=4 -DUSB_NTXD=8 -DUSBCTL_RXBUFS=4 \
		-DUSB_NRXD=8 -DUSB_RXBND=-1 -DUSBCTLBUFSZ=128

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/usbrdl/Makefile.cmn
