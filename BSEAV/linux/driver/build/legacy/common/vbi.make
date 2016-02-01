############################################################
#     Copyright (c) 2003-2004, Broadcom Corporation
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
# Created: 02/09/2001 by Marcus Kellerman
#
# $brcm_Log: $
# 
############################################################

# VBI code
# common for 71xx and 73xx

PI_CCE			= $(PORTINTF)/ccecontrol
PI_CCE_CHIP 	= $(PORTINTF)/ccecontrol/$(BCM_CHIP)
SL_CCE			= $(SYSLIB)/ccelib
PI_CCD			= $(PORTINTF)/ccdcontrol
PI_CCD_CHIP		= $(PORTINTF)/ccdcontrol/$(BCM_CHIP)
SL_CCD			= $(SYSLIB)/ccdlib

PI_TT			= $(PORTINTF)/ttcontrol
PI_TT_CHIP		= $(PORTINTF)/ttcontrol/$(BCM_CHIP)
SL_TT			= $(SYSLIB)/ttlib
SL_VBI			= $(SYSLIB)/vbilib/$(BCM_CHIP)

PI_WSSD			= $(PORTINTF)/wssdcontrol
PI_WSSD_CHIP	= $(PORTINTF)/wssdcontrol/$(BCM_CHIP)
PI_WSSE			= $(PORTINTF)/wssecontrol
PI_WSSE_CHIP	= $(PORTINTF)/wssecontrol/$(BCM_CHIP)

SL_CGMS			= $(SYSLIB)/cgmslib

BACKEND_PATH += \
	$(PI_CCE) 			\
	$(PI_CCE_CHIP) 		\
	$(SL_CCE) 			\
	$(PI_TT)			\
	$(SL_TT)			\
	$(SL_VBI)

BACKEND_SRC += \
	cce.c				\
	bcmcce.c			\
	vbilib.c			\
	bcmvbidrv.c

ifeq ($(CCD_SUPPORT),y)
# This is only needed for analog platforms
BACKEND_PATH += \
	$(PI_CCD) 			\
	$(PI_CCD_CHIP) 		\
	$(SL_CCD)
BACKEND_SRC += \
	ccd.c				\
	bcmccd.c
endif

ifeq ($(TT_SUPPORT),y)
BACKEND_PATH += \
	$(PI_TT_CHIP)

BACKEND_SRC += \
	bcmtt.c				\
	bcmtte.c			\
	bcmttd.c			\
	bteletext.c
endif
