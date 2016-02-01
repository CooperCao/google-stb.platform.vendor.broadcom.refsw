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
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#
# BCHP_7411_VER - If defined, 7411 support will be added for the specified 7411 revision.
# BRAP_IMG_ONLY - If defined, only the firmware images will be included.  If not defined, non-firmware sources will be included as well.
# BRAP_IMG_EXTERNAL - If defined, firmware images will be excluded.  If not defined, firmware images will be included.
# BRAP_DDP_SUPPORT - If defined, Dolby Digital plus support is enabled
# BRAP_WMA_SUPPORT - If defined, Windows Media audio support is enabled
#

ifeq ($(BCHP_CHIP),7400)
include $(MAGNUM)/portinginterface/rap/build/brap_fmm130.inc
else
include $(MAGNUM)/portinginterface/rap/build/brap_fmm65.inc
endif

