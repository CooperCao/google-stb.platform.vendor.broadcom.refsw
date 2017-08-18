# Makefile for hndrte based 4320 standalone programs
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

# chip specification
CHIP := 4320

# default targets
TARGETS :=
TARGETS += usb-retail-g-rndis-reclaim-idsup-wme-led

# common target attributes
TARGET_ARCH	:= mips

# 32-bit DMA engine
BCMDMA32	:= 1

# wlconfig & wltunable
WLCONFFILE	:= wlconfig_rte_dev
g-wltunable	:= wltunable_rte_dev.h
ag-wltunable	:= wltunable_rte_ag_dev.h

# other stuff
FLASH		:= 1
