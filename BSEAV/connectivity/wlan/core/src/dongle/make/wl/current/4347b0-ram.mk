# Makefile for hndrte based 4347b0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: $

include $(TOPDIR)/current/4347a0-ram.mk

ROMREV		:= b0
REV		:= b0
REVID		:= 3
PCIEREVID	:= 23
CCREV		:= 62
PMUREV		:= 33
GCIREV		:= 12

ifeq ($(call opt,epa),1)
	# ePA radio rev for both Main and Aux
	BCMRADIOREV     := 0x8
	BCMRADIOREV2     := 0x9
else ifeq ($(call opt,ipa),1)
	# iPA radio rev for both Main and Aux
	BCMRADIOREV     := 10
	BCMRADIOREV2    := 11
endif

WLTUNEFILE	:= wltunable_rte_4347b0.h
