# Makefile for hndrte based 4322 standalone programs
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

# chip specification
CHIP	:= 4322

# common target attributes
TARGET_HBUS	:= usb
TARGET_ARCH	:= arm
TARGET_CPU	:= cm3
THUMB		:= 1
HBUS_PROTO 	:= cdc
BCM_DNGL_BL_PID := 0xbd13
BCMDMA32	:= 1
SUFFIX_ENAB := 1

# ROM rev
ROMREV		:= a0

# other stuff, 4322usb otp hack

# config/target
#CONFFILE = wlconfig
#TUNEFILE = wltunable
#TARGET_XP = roml-ndis-xp
#TARGET_VISTA = roml-ndis-vista
#TARGET_LINUX = roml
#$(TARGET_XP)-$(CONFFILE) = wlconfig_rte_4322_dngl
#$(TARGET_XP)-$(TUNEFILE) = wltunable_rte_4322_dngl.h
#$(roml-ndis-vista)-$(CONFFILE) = wlconfig_rte_4322_dngl
#$(roml-ndis-vista)-$(TUNEFILE) = wltunable_rte_4322_dngl.h

WLCONFFILE	:= wlconfig_rte_4322_dngl
WLTUNEFILE	:= wltunable_rte_4322_dngl.h

# CLM info

# default targets
TARGETS := \
	roml-ndis-vista-noreclaim-noccx-g \
	roml-ndis-xp-noreclaim-noccx-mfgtest-g \
	roml-noreclaim-noccx-mfgtest-g \
	roml-reclaim-assert-g-err-poll \

# maximum image size (.bin file) full dongle can't fit any more, skip the sizechecking
#roml-ndis-vista-noreclaim-noccx-g-maxsize = 304000
#roml-ndis-xp-noreclaim-noccx-mfgtest-g-maxsize = 304000
# warn limits to alert via emails/PRs
#roml-ndis-vista-noreclaim-noccx-g-warnlimit = 500
#roml-ndis-xp-noreclaim-noccx-mfgtest-g-warnlimit = 500

BINDL = $(BIN_TRX)

# Set MEMSIZE for sizing system.
MEMSIZE = 450000
