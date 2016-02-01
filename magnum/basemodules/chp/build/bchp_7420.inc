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
#
# $brcm_Log: $
# 
############################################################

ifeq (${BCHP_VER_LOWER},a1)
BCHP_RDB_PATH := $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/a0
else
BCHP_RDB_PATH := $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)
endif
