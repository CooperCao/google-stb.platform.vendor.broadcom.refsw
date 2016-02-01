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
MAGNUM_MODULES += BVBILIB

# Module Includes
BVBILIB_INCLUDES += \
	$(MAGNUM)/syslib/vbilib/src \
	$(MAGNUM)/syslib/vbilib/include

BVBILIB_SOURCES += \
	$(MAGNUM)/syslib/vbilib/src/bvbilib.c \
	$(MAGNUM)/syslib/vbilib/src/bvbilib_dccreorder.c \
	$(MAGNUM)/syslib/vbilib/src/bvbilib_list.c

