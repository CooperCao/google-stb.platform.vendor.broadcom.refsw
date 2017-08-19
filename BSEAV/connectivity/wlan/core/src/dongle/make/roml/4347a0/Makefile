#
# Makefile for 4347A0 ROM library build
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: Makefile 251244 2011-04-05 23:26:07Z $:
#

# top level make
#
SRCBASE = $(shell /bin/pwd)/../../../..

ROMPFX		:= H3E_L40RMHDCA16384X64R532VXVTSSY

CHIP		:= 4347
CHIPREV		:= a0

# ROM size limit
ROMSIZE	 ?= 1245184	# 1216KB
ROMBANKSIZE ?= 131072	# 128KB

#ROM SIZE FOR QT
ROMADDR_QT = 3FFF

# ROM library addresses
TEXT_START	= 0x00000020
DATA_START	= 0x00170100

# brand from which ROM library is based
# To add common features shared between wl/ and roml/:
#    - Update brand or conf file
#
ROMTBL_CFG	:= 4347a0
ROMTBL_VARIANT	:= threadx-sdio-pcie

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
