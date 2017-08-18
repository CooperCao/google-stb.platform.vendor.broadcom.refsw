#	
# Makefile for 4324B0 ROM library build
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: Makefile 268359 2011-06-22 08:58:52Z $:
#

# top level make
#
SRCBASE = $(shell /bin/pwd)/../../../..
# include config 
include $(SRCBASE)/dongle/make/roml/Makefile.conf

ROMPFX		:=    H0E_L40RMHD65536X32R672VXVTSSY

# To handle different names for different banksizes
RENAMECODEFILE := mv H0E_L40RMHD32768X32R672VXVTSSY_002.codefile H0E_L40RMHD32768X32R632VXVTSSY_000.codefile

CHIP		:= 4335
CHIPREV		:= a0

PREPEND_BINARY2 := 1

# Location of boot loader binary file
PBDIR	:= $(CHIP_IMAGE_DIR)/usbrdl/$(CHIP)$(CHIPREV)
PBBIN	:= $(PBDIR)/usbrdl.bin
PBEXE	:= $(PBDIR)/usbrdl.exe
PBQT	:= $(PBDIR)/usbrdl.qt
PBLEN	:= $(shell stat -c %s $(PBBIN))

# Check to minimize chance of out-of-date boot ROM getting used
PBBLDDIR	:= $(SRCBASE)/dongle/make/usbrdl/$(CHIP)$(CHIPREV)/boot-rom
PBBLDBIN	:= $(PBBLDDIR)/rtecdc.bin
ifneq ($(shell if [ $(PBBIN) -ot $(PBBLDBIN) ]; then echo bad; fi),)
$(error Boot ROM binary in build area is newer than one in $(PBDIR))
endif

# ROM size limit
ROMSIZE 	?= 655360	# 0xA0000
ROMBANKSIZE	?= 262144   # 0x40000

# ROM address aliases
# The boot loader is linked to the HIGH alias; the ROM library is linked to the LOW alias.

#ROMADDR_DELTA	:= $(shell awk 'BEGIN { printf "0x%x\n", $(ROMADDR_HIGH) - $(ROMADDR_LOW) }')

# ROM library addresses
TEXT_START	= $(shell awk 'BEGIN { printf "0x%x\n", $(ROMADDR_LOW) + $(PBLEN) }')
DATA_START	= 0x00180100

# brand from which ROM library is based
# To add common features shared between wl/ and roml/:
#    - Update brand or conf file
#
# Romtable generation
ROMTBL_CFG	:= 4335a0
ROMTBL_VARIANT	?= sdio-usb


# Use TCAM to patch ROM functions
JMPTBL_TCAM := 1
GENROMTBL := 1
# include RTE makefile configuration
include $(SRCBASE)/dongle/make/roml/Makefile.cmn
