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
# BTD_OMIT_DEFAULT_CONFIG - If defined, the path to basemodules/std/config/bstd_cfg.h will be removed from the build.  This allows the application to define a custom bstd_cfg.h file in their own directory.  If not defined (default), the standard path will be included.
#

# Define this module
MAGNUM_MODULES += BSTD

# Backward-compatibility
B_REFSW_OS ?= $(OS)

# Module Includes
BSTD_INCLUDES += $(MAGNUM)/basemodules/std
ifeq ($(BSTD_OMIT_DEFAULT_CONFIG),)
BSTD_INCLUDES += $(MAGNUM)/basemodules/std/config
endif

# Module Sources
ifeq ($(findstring $(B_REFSW_OS), linuxclient linuxemu linuxpal win32),${B_REFSW_OS})
BSTD_INCLUDES += $(MAGNUM)/basemodules/std/types/linuxuser
# Warning if types are not found
ifeq ($(wildcard $(MAGNUM)/basemodules/std/types/linuxuser),)
$(warning warning: Missing basemodules/std/types definition for OS=$(B_REFSW_OS))
endif
else
ifeq ($(B_REFSW_OS),ucos)
BSTD_INCLUDES += $(MAGNUM)/basemodules/std/types/$(B_REFSW_UCOS_SUBDIR_NAME)
# Warning if types are not found
ifeq ($(wildcard $(MAGNUM)/basemodules/std/types/$(B_REFSW_UCOS_SUBDIR_NAME)),)
$(warning warning: Missing basemodules/std/types definition for $(B_REFSW_UCOS_SUBDIR_NAME))
endif
else
BSTD_INCLUDES += $(MAGNUM)/basemodules/std/types/$(B_REFSW_OS)
# Warning if types are not found
ifeq ($(wildcard $(MAGNUM)/basemodules/std/types/$(B_REFSW_OS)),)
$(warning warning: Missing basemodules/std/types definition for OS=$(B_REFSW_OS))
endif
endif
endif

ifdef B_REFSW_SYSTEM_MODE
BSTD_DEFINES := B_REFSW_SYSTEM_MODE_${B_REFSW_SYSTEM_MODE}=1
endif
ifeq ($(BSTD_FILE),n)
BDBG_DEFINES += BSTD_FILE_DISABLED=1
endif
