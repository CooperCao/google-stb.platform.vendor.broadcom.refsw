# Makefile for threadx based full dongle standalone programs
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

# chip specification - all chips
CHIP	:= UNKNOWN

# include ThreadX
THREADX	:= 1
HNDRTE	:= 0

# turn off .flags and .depend generation to save build time
LOCALBUILD	:= 0

# Feel free to enable your features in this makefile to make sure code
# for your features are not affected by day-to-day development.  Please
# DO NOT add any new build targets unless absolutely necessary; just
# add your features as additional -option- to the existing
# TARGET_OPTIONS_config_pcie_all in config/full-dongle-config.mk
DBGOPTS := dump
TSTOPTS	:= mfgtest-seqcmds
FEATURES := idauth-idsup-ccx-p2p-pno-toe-aoe-pktfilter-keepalive-mchan-lpc-wl11u-txbf-pktctx-splitrx-rsdb


# These builds compile all chip-specific code and all d11 core-specific
# code as well as all PHY-specific code.  Of course, you can't load the
# images on any chip to run them because of the size.

# build targets
TARGETS := \
	cr4-pcie-msgbuf-$(DBGOPTS)-$(TSTOPTS)-$(FEATURES)

# common target attributes
TARGET_ARCH	:= arm
THUMB		:= 1
BAND		:= ag

# wlconfig & wltunable
WL		:= 1
WLCNT		:= 1
WL11N		:= 1
WL11AC		:= 1
WL11AX		:= 1
WLAMPDU		:= 1
WLAMPDU_TX	:= 1
WLAMSDU		:= 1
WLAMSDU_TX	:= 1
WLTUNEFILE	:= wltunable_sample.h

# features
DBG_ERROR	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 1
DBG_ERROR	:= 0
BCMPKTPOOL	:= 1
DMATXRC		:= 1
PROP_TXSTATUS	:= 1
AMPDU_HOSTREORDER := 1

# Set MEMSIZE to a very large value for precommit builds.
MEMSIZE		:= 6000000

# make control

# CLM info

# extra flags
BUS_POST	:= 6
BUS_NRXD	:= 32
BUS_NTXD	:= 32
BUS_RXBND	:= 6

BCMPKTPOOL	:= 1
POOL_LEN_MAX    := 200
POOL_LEN        := 6
WL_POST		:= 40

FRAG_POOL_LEN   := 200
RXFRAG_POOL_LEN   := 120

#dongle builds need BCMPKDITDMAP
HNDLBUFCOMPACT	:= 1
BCMPKTIDMAP	:= 1
PKT_MAXIMUM_ID	:= 640

BCMPKTIDMAP_ROM_COMPAT := 1
AMPDU_RX_BA_DEF_WSIZE := 48

EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=16
EXTRA_DFLAGS	+= -DPKTC_DONGLE

PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192
