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
# $brcm_Log: $
# 
############################################################

# Define this module
MAGNUM_MODULES += BSYSLIB

# Module Includes
BSYSLIB_INCLUDES += $(MAGNUM)/syslib/framework/include

# Module Sources
BSYSLIB_SOURCES = \
	$(MAGNUM)/syslib/framework/src/bsyslib_callback.c \
	$(MAGNUM)/syslib/framework/src/bsyslib_list.c \
	$(MAGNUM)/syslib/framework/src/bsyslib_resource_pool.c

