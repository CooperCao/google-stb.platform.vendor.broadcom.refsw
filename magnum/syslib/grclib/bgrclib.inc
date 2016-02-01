############################################################
#     Copyright (c) 2003-2012, Broadcom Corporation
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
# Created:
#
# $brcm_Log: $
# 
############################################################

# Define this module
MAGNUM_MODULES += BGRCLIB

# Module Includes
BGRCLIB_INCLUDES += $(MAGNUM)/syslib/grclib/include

# Module Sources
ifeq ($(GRCPACKET_SUPPORT),y)
BGRCLIB_DEFINES += GRCPACKET_SUPPORT=1
BGRCLIB_SOURCES += $(MAGNUM)/syslib/grclib/src/bgrclib_packet.c
else
BGRCLIB_SOURCES += $(MAGNUM)/syslib/grclib/src/bgrclib.c
endif

