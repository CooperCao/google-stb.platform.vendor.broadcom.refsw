#
# Makefile for 4365c0 ROM library build
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

ROMPFX		:=    H49_L40RMHD40960X32R642VXVTSSYT
CHIP		:= 4365
CHIPREV		:= c0

# ROM size 640KB+160KB = 800KB
ROMSIZE		?= 819200
ROMBANKSIZE	?= 655360

# ROM library addresses
TEXT_START	= 0x00000040
DATA_START	= 0x200100

# brand from which ROM library is based
# To add common features shared between wl/ and roml/:
#    - Update brand or conf file
#
# Romtable generation
ROMTBL_CFG	:= 4365c0
ROMTBL_VARIANT	:= pcie

# Use TCAM to patch ROM functions
JMPTBL_TCAM := 1

#These values are chip specific/chip specific ROM params

MEM_WIDTH       := 4

SPLIT_WIDTH     := 1
ROMADDR_QT      := 9fff

# For BIG_LITTLE ROM banks (640KB+160KB) in C0
ROMPFX_LITTLE       := H49_L40RMHD10240X32R602VXVTSSYT
SPLIT_WIDTH_BL_BANK := 1
ROMADDR_QT_LITTLE   := 27ff

# ROM is 128 bits wide
QT_MEM_WIDTH    := 128

# include RTE makefile configuration
include $(SRCBASE)/dongle/make/roml/Makefile.cmn
