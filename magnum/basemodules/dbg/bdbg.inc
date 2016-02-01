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
MAGNUM_MODULES += BDBG

# Backward-compatibility
B_REFSW_OS ?= $(OS)

# Module Includes
BDBG_INCLUDES := $(MAGNUM)/basemodules/dbg

# Sanity Check OS provided
BDBG_SUPPORTED_OS := linuxuser linuxkernel linuxclient linuxemu linuxpal vxworks ucos nucleus win32 bkernel ucos_ii ucos_iii bare no_os
ifeq ($(findstring $(B_REFSW_OS), $(BDBG_SUPPORTED_OS)),)
$(error [dbg] OS must be one of the following: $(BDBG_SUPPORTED_OS))
endif

BDBG_SOURCES := $(MAGNUM)/basemodules/dbg/bdbg.c \
                $(MAGNUM)/basemodules/dbg/bdbg_fifo.c 

ifeq ($(findstring $(B_REFSW_OS), linuxclient linuxemu linuxpal),${B_REFSW_OS})
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/linuxuser/bdbg_os_priv.c
else
ifeq ($(B_REFSW_OS),ucos)
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/$(B_REFSW_UCOS_SUBDIR_NAME)/bdbg_os_priv.c
else
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/$(B_REFSW_OS)/bdbg_os_priv.c
endif
endif

ifeq ($(B_REFSW_OS),vxworks)
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/$(B_REFSW_OS)/bdbg_output.c
else
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/bdbg_output.c
endif

#
# Compile time options:
# B_REFSW_DEBUG is used by Nexus (see nexus/build/nexus_defs.inc) and other SW above Magnum.
# There is no Magnum build variable to enable DBG; instead, build with the macro -DBDBG_DEBUG_BUILD.
# In addition to that macro, you can also build with the following build variables which
# give intermediate levels of debug functionality:
#
# B_REFSW_DEBUG_LEVEL - compile out some but not all levels
# B_REFSW_DEBUG_COMPACT_ERR - reduce size by using stock messages for errors
# B_REFSW_DEBUG_ASSERT_FAIL - don't call BKNI_Fail if BDBG_ASSERT fails
#

ifeq ($(B_REFSW_DEBUG_LEVEL),none)
BDBG_DEFINES += BDBG_NO_ERR=1 BDBG_NO_LOG=1 BDBG_NO_WRN=1 BDBG_NO_MSG=1
endif
ifeq ($(B_REFSW_DEBUG_LEVEL),err)
BDBG_DEFINES += BDBG_NO_LOG=1 BDBG_NO_WRN=1 BDBG_NO_MSG=1
endif
ifeq ($(B_REFSW_DEBUG_LEVEL),log)
BDBG_DEFINES += BDBG_NO_WRN=1 BDBG_NO_MSG=1
endif
ifeq ($(B_REFSW_DEBUG_LEVEL),wrn)
BDBG_DEFINES += BDBG_NO_MSG=1
endif

# reduce size by reusing a single format string for all errors
ifeq ($(B_REFSW_DEBUG_COMPACT_ERR),y)
BDBG_DEFINES += B_REFSW_DEBUG_COMPACT_ERR=1
endif

ifeq ($(B_REFSW_DEBUG_ASSERT_FAIL),n)
# BDBG_ASSERT will print ERR, but not fail
# the env variable is positive logic, but the macro is negative logic to not introduce a new default
BDBG_DEFINES += B_REFSW_DEBUG_ASSERT_NO_FAIL=1
endif

ifeq ($(B_REFSW_STRICT_PRINTF_FORMAT),y)
BDBG_DEFINES += B_REFSW_STRICT_PRINTF_FORMAT=1
endif

ifeq ($(B_REFSW_ANDROID), y)
ifneq ($(B_REFSW_OS), linuxkernel)
BDBG_DEFINES += BDBG_ANDROID_LOG=1
else
BDBG_DEFINES += BDBG_ASSERT_TO_KNI=1
endif
endif

# used for proc, otherwise save code size
ifeq ($(B_REFSW_OS), linuxkernel)
BDBG_DEFINES += BDBG_USE_VA_ARGS
endif
