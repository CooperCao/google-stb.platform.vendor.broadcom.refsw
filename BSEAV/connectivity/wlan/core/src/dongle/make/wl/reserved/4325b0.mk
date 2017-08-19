# Makefile for hndrte based 4325 standalone programs
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
	sdio-mfgtest-ag-cdc-seqcmds \
	sdio-mfgtest-g-cdc-seqcmds \
	sdio-mfgtest-g-cdc-ndis-seqcmds \
	sdio-internal-assert-g-cdc-reclaim-idsup \
	sdio-ag-cdc-reclaim-idsup \
	sdio-g-cdc-reclaim-idsup-wme \
	sdio-ag-cdc-reclaim-idsup-pno-acn-aoe-toe \
	sdio-g-cdc-reclaim-idsup-wme-pno-acn-aoe-toe \
	sdio-g-cdc-reclaim-ccx-wme-pno-acn-aoe-toe \
	sdio-ag-cdc-ndis-reclaim \
	sdio-g-cdc-dhdoid-reclaim-idsup-wme-rwl \
	sdio-g-cdc-dhdoid-reclaim-idsup \
	sdio-g-cdc-ndis-reclaim-idsup-wme \
        sdio-g-cdc-ndis-reclaim-idsup-wme-rwl \
        sdio-g-cdc-ndis-reclaim-idsup-wme-rwlwifi-af \
        sdio-g-cdc-ndis-reclaim-idsup-wme-uartreflector \
        sdio-g-cdc-ndis-reclaim-idsup-wme-wifireflector \
	sdio-g-cdc-ndis-reclaim-wme \
	sdio-g-cdc-wme-pno-aoe-toe-motolj \
	sdio-ag-cdc-reclaim-idsup-pno-acn-aoe-toe-rtdc \
	sdio-ag-cdc-reclaim-idsup-wme-keepalive \
	sdio-g-cdc-reclaim-idsup-wme-keepalive \
	sdio-ag-cdc-reclaim-idsup-wme-pktfilter \
	sdio-g-cdc-reclaim-idsup-wme-pktfilter \
	sdio-g-cdc-reclaim-idsup-wme-pktfilter-keepalive-aoe-toe \
	sdio-g-cdc-reclaim-msgtrace \
	sdio-g-cdc-reclaim-wme \
	sdio-g-cdc-reclaim-pool \
	sdio-g-cdc-reclaim-idsup-p2p-apsta-idauth \
	sdio-g-cdc-reclaim-idsup-idauth-p2p-apsta-af \
	sdio-g-cdc-reclaim-wme-apsta-idsup-idauth \
	sdio-g-cdc-reclaim-idsup-wme-rwl \
	sdio-g-cdc-reclaim-idsup-wme-rwlwifi-af \
	sdio-g-cdc-reclaim-idsup-wme-rwlwifi

# special targets
TARGETS += \
	sdio-g-cdc-rsock

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= 7s
THUMB		:= 1
BCMDMA32	:= 1

# SPROM hangs off PCMCIA bus
SPROMBUS	:= pcmcia

# wlconfig & wltunable
WLCONFFILE	:= wlconfig_rte_4325
WLTUNEFILE	:= wltunable_rte_4325b0_dev.h

# CLM info

#ERROR		:= 1
#INFORM		:= 0
#ASSERT		:= 1
NO_BCMINTERNAL  := 1

# SPROM hangs off PCMCIA bus
SPROMBUS	:= pcmcia

# maximum image size (.bin file)
#sdio-mfgtest-ag-cdc-seqcmds-maxsize   = 342874
#sdio-mfgtest-g-cdc-seqcmds-maxsize    = 343800
# warn limits to alert via emails/PRs
sdio-mfgtest-ag-cdc-seqcmds-warnlimit = 256
sdio-mfgtest-g-cdc-seqcmds-warnlimit  = 256

# other stuff
ECICOEX	:= 1
EXTRA_DFLAGS	+= -DBRCMAPIVTW=256
EXTRA_DFLAGS	+= -DBCMDBG_ARMRST
EXTRA_DFLAGS	+= -DWL20MHZ_ONLY

# Set MEMSIZE for sizing system.
MEMSIZE		:= 450000
