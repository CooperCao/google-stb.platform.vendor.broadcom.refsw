# Makefile for hndrte based 4335b0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4335a0-ram.mk 266614 2011-06-14 22:16:03Z $:
#

include $(TOPDIR)/current/4335a0-ram.mk

REVID		:= 1
WLTUNEFILE	:= wltunable_rte_4335b0.h

BCMRADIOREV	:= 0x17
BCMRADIOVER	:= 0x0
BCMRADIOID	:= 0x2069
EXTRA_DFLAGS	+= -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS	+= -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS	+= -DBCMRADIOID=$(BCMRADIOID)
