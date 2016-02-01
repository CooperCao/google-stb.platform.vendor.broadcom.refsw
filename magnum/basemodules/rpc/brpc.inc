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
MAGNUM_MODULES += BRPC

# Backward-compatibility
B_REFSW_OS ?= $(OS)

# Module Includes
BRPC_INCLUDES += $(MAGNUM)/basemodules/rpc/include

# Module Sources
BRPC_SOURCES += $(MAGNUM)/basemodules/rpc/src/brpc.c
BRPC_SOURCES += $(MAGNUM)/basemodules/rpc/src/$(B_REFSW_OS)/brpc_socket.c

# Sanity check B_REFSW_OS value
BRPC_SUPPORTED_OS := linuxuser linuxkernel no_os ucos_ii vxworks
ifeq ($(findstring $(B_REFSW_OS),$(BRPC_SUPPORTED_OS)),)
$(error [rpc] Illegal OS [$(B_REFSW_OS)].  Supported OS values are [$(BRPC_SUPPORTED_OS)])
endif

