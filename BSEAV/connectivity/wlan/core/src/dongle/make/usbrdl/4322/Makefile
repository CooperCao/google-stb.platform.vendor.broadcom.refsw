#
# Makefile for USB bootloader based on hndrte
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

#
# top level make
#

SRCBASE = $(shell /bin/pwd)/../../../..

# override above to produce more useful targets
TARGETS = boot-rom run-debug

CHIP := 4322

TCAM := 1

ROMSIZE := 81920

# normally, this will be applied to wl/config/wl.mk, since bootloader doesn't use that,
# need to set in DFLAGS below explicitly, keey this variable here for reference
BCMDMA32 := 1

BCM_ID_PRODUCT := 0xbd13

DATA_BASE := 0x60000000
ifeq ($(findstring boot,$(TARGET)),boot)
DATA_START	:= 0x6004a000
else
TEXT_START	:= 0x00000000
endif

# This is SB chip, select to eliminate AI code
EXTRA_DFLAGS	+= -DBCMCHIPTYPE=SOCI_SB -DBCMBUSTYPE=SI_BUS
EXTRA_DFLAGS	+= -DHND_STACK_SIZE=6144
EXTRA_DFLAGS	+= -DDL_NVRAM=2044
EXTRA_DFLAGS	+= -DUSBBULK_RXBUFS=4 -DUSB_NTXD=8 -DUSBCTL_RXBUFS=4 \
		   -DUSB_NRXD=8 -DUSB_RXBND=-1 -DUSBCTLBUFSZ=128

ifeq ($(findstring boot-rom,$(TARGET)),boot-rom)
EXTERN_OBJECTS	+= aes.o aeskeywrap.o bcmccx.o bcmwpa.o ccx.o des.o hmac.o \
		md4.o md5.o passhash.o prf.o rc4.o rijndael-alg-fst.o sha1.o \
		tkhash.o tkmic.o wep.o
EXTRA_DFLAGS	+= -DBCMSUP_PSK -DBCMCCX -DWLCAC -DBCMWPA2
EXTERN_OBJECTS	+= romtable.o
EXTRA_DFLAGS	+= -DROMTABLE=_romtable
vpath %.S	$(SRCBASE)/dongle/make/usbrdl/4322
endif

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/usbrdl/Makefile.cmn
