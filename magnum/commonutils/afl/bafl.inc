#############################################################################
#
#       Copyright (c) 2003-2010, Broadcom Corporation.
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
MAGNUM_MODULES += BAFL

# Module Includes
BAFL_INCLUDES += $(MAGNUM)/commonutils/afl/

# Module Sources
BAFL_SOURCES+= $(MAGNUM)/commonutils/afl/bafl.c


