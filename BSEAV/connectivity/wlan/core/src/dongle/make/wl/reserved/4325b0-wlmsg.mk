# Makefile for hndrte based 4325 standalone programs to
# make sure debug capability is not broken
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
CHIP := 4325

# default targets
TARGETS := \
	g-cdc-prhdrs \
	g-cdc-prpkt \
	g-cdc-inform \
	g-cdc-assoc \
	g-cdc-ps \
	g-cdc-mpc \
	g-cdc-wsec \
	g-cdc-scan \
	g-cdc-mem \
	g-cdc-err \
	g-cdc-assert \
	g-cdc-dump \
	g-cdc-ndis-oid \
	g-cdc-internal \
	g-cdc-debug

# common target attributes
TARGET_HBUS	:= sdio
TARGET_ARCH	:= arm
TARGET_CPU	:= 7s
THUMB		:= 1
BCMDMA32	:= 1
NO_BCMINTERNAL	:= 1

# wlconfig & wltunable
WLCONFFILE	:= wlconfig_rte_dev
WLTUNEFILE	:= wltunable_rte_4325b0_dev.h

# CLM info

# all images are reclaim
RECLAIM		:= 1

# other stuff
EXTRA_DFLAGS	+= -DBCMDBG_ARMRST

# Set MEMSIZE for sizing system.
MEMSIZE = 450000
