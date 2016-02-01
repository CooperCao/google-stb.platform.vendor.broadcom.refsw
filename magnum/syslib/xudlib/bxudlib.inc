#############################################################################
#
#       Copyright (c) 2003-2012, Broadcom Corporation.
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
# [File Description:]
#
# Revision History:
#
# $brcm_Log: $
# 
#############################################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BXUDLIB

# Module Includes
BXUDLIB_INCLUDES += $(MAGNUM)/syslib/xudlib

BXUDLIB_SOURCES += \
	$(MAGNUM)/syslib/xudlib/bxudlib.c

ifeq ($(BXUDLIB_TEST_MODE),y)
BXUDLIB_DEFINES += BXUDLIB_P_TEST_MODE=1
endif

ifeq ($(BXUDLIB_DUMP_INPUT_CC),y)
BXUDLIB_DEFINES += BXUDLIB_P_DUMP_INPUT_CC=1
endif

ifeq ($(BXUDLIB_DUMP_OUTPUT_CC),y)
BXUDLIB_DEFINES += BXUDLIB_P_DUMP_OUTPUT_CC=1
endif