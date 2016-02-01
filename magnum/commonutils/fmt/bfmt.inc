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
# N/A
#

# Define this module
MAGNUM_MODULES += BFMT

# To build shrinked bfmt and bvdc for BOOTUPDATER, refer to
# /rockford/applications/vdc_for_bootupdater/Makefile
ifeq ($(B_PI_FOR_BOOTUPDATER),y)
#CFLAGS += -DBFMT_PICK_e480p=1
#CFLAGS += -DBFMT_PICK_ePAL_M=1
#CFLAGS += -DBFMT_PICK_eNTSC
#CFLAGS += -DBFMT_PICK_e720p=1
#CFLAGS += -DBFMT_PICK_e1080i=1
#CFLAGS += -DBFMT_PICK_e1080p=1
#CFLAGS += -DBFMT_PICK_e1080p_24Hz=1
#CFLAGS += -DBFMT_PICK_e1080p_30Hz=1
CFLAGS += -DBFMT_DO_PICK=1
CFLAGS += -Os -fdata-sections -ffunction-sections -Wl,--gc-sections
endif ### ifeq ($(B_PI_FOR_BOOTUPDATER),y)

# Module Includes
BFMT_INCLUDES += $(MAGNUM)/commonutils/fmt/include

# Module Sources
BFMT_SOURCES += $(MAGNUM)/commonutils/fmt/src/bfmt.c

#
# End of file
#

