#
# Makefile for 4347B0 ROM library build
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: $:
#

# top level make
#
SRCBASE = $(shell /bin/pwd)/../../../..

ROMPFX		:= H3E_L28RMCA16384X64R542VMVTSUS35YT

CHIP		:= 4369
CHIPREV		:= a0

# ROM size limit
ROMSIZE	 ?= 1245184	# 1216KB
ROMBANKSIZE ?= 131072	# 128KB

#ROM SIZE FOR QT
ROMADDR_QT = 3FFF
LAST_ROM_BK_ADDR_QT = 1FFF

# ROM library addresses
TEXT_START	= 0x00000020
DATA_START	= 0x00170100

# brand from which ROM library is based
# To add common features shared between wl/ and roml/:
#    - Update brand or conf file
#
ROMTBL_CFG	:= 4369a0

# removed sdio to accomadate other features; sdio is low priority
ROMTBL_VARIANT	:= threadx-pcie

MEM_WIDTH := 8

# Add flops support
FLOPS_SUPPORT := 1

# Use TCAM to patch ROM functions
JMPTBL_TCAM := 1

BCMSROMREV :=0x0b

# correct this with latest toolchain
TOOLSVER := 2013.11

# include common makefile
include $(SRCBASE)/dongle/make/roml/Makefile.cmn
