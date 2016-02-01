############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
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
# Revision History:
#
# $brcm_Log: $
# 
############################################################

# Define this module
MAGNUM_MODULES += BMXT

BMXT_INCLUDES += $(MAGNUM)/portinginterface/mxt/include
BMXT_INCLUDES += $(MAGNUM)/basemodules/hab
BMXT_INCLUDES += $(MAGNUM)/portinginterface/dcm/rpc/include

BMXT_SOURCES += $(MAGNUM)/portinginterface/mxt/src/bmxt.c
BMXT_SOURCES += $(MAGNUM)/portinginterface/mxt/src/bmxt_fe.c
BMXT_SOURCES += $(MAGNUM)/portinginterface/mxt/src/bmxt_wakeup.c
BMXT_SOURCES += $(MAGNUM)/portinginterface/mxt/src/bmxt_common.c

ifeq ($(filter BRPC BDCM,$(MAGNUM_MODULES)),)
BMXT_DEFINES += BMXT_NO_RPC
endif