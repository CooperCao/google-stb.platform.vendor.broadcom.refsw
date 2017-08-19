#
# NetBSD External WL build/release Makefile
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: netbsd-external-wl.mk 260128 2011-05-17 20:26:59Z $
#
DEFS :=
UNDEFS :=  BCMINTERNAL POCKET_PC WL_PCMCIA BCM_WL_EMULATOR MICROSOFT BCMNVRAMW WLDIAG
PARENT_MAKEFILE := netbsd-wl.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

include $(PARENT_MAKEFILE)
