############################################################################
#     Copyright (c) 2003-2011, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
# 
# Global definitions 
#
# Revision History:
#
# $brcm_Log: $
# 
###########################################################################


ifdef B_REFSW_DEBUG
# apply  ODIR conversion for debug/release build
ifdef  ODIR
ifeq    ($(B_REFSW_DEBUG),1)
ODIR	:= $(ODIR).debug
MAGNUM_CFLAGS += -DBDBG_DEBUG_BUILD=1 
else
ODIR 	:= $(ODIR).release
endif
endif
else
# if B_REFSW_DEBUG not defined use debug build
MAGNUM_CFLAGS += -DBDBG_DEBUG_BUILD=1 
endif

ifndef ODIR
ODIR=.
endif

_ODIR := $(ODIR)/.dir
APP := $(ODIR)/$(APP)


