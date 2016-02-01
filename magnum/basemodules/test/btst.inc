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
MAGNUM_MODULES += BTST

# Backward-compatibility
B_REFSW_OS ?= $(OS)

# Sanity Check OS provided
BTST_LINUX_OS := linuxuser linuxclient linuxemu linuxpal
BTST_SUPPORTED_OS := $(BTST_LINUX_OS) vxworks
ifeq ($(findstring $(B_REFSW_OS), $(BTST_SUPPORTED_OS)),)
$(error [tst] OS must be one of the following: $(BTST_SUPPORTED_OS))
endif

# Determine source directory
BTST_LINUX_OS := linuxuser linuxclient linuxemu linuxpal
ifeq ($(filter $(B_REFSW_OS), $(BTST_LINUX_OS)), $(B_REFSW_OS))
	BTST_OS = linux
else
	BTST_OS = $(B_REFSW_OS)
endif

# Module Includes
BTST_INCLUDES += $(MAGNUM)/basemodules/test/include/

BTST_SOURCES += $(MAGNUM)/basemodules/test/src/$(BTST_OS)/btst_kni.c