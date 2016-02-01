#############################################################################
#
#       Copyright (c) 2003-2011, Broadcom Corporation.
#       All rights reserved.
#       Confidential Property of Broadcom Corporation.
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# File Description: Builds video shmoo for the Bcm97422 linux platform
#
# Revision History:
#
# $brcm_Log: $
# 
#############################################################################

COMMON_INCLUDES += $(ROCKFORD_DIR)/applications/video_shmoo_test/
COMMON_SOURCES  += \
	$(ROCKFORD_DIR)/applications/video_shmoo_test/btst_video_shmoo.c

