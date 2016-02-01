############################################################
#     Copyright (c) 2003-2014, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#  $brcm_Workfile: $
#  $brcm_Revision: $
#  $brcm_Date: $
#
#  Revision History:s
#  $brcm_Log: $
#
############################################################

#  Check for required definitions.
$(if ${NEXUS_PLATFORM},,$(error NEXUS_PLATFORM is not defined))
$(if ${NEXUS_TOP},,$(error NEXUS_TOP is not defined))
$(if ${B_REFSW_ARCH},,$(error B_REFSW_ARCH is not defined))

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}

#  Define the blive_ext source directory.
BLIVE_EXT_DIR = $(abspath $(NEXUS_TOP)/../BSEAV/lib/livemedia/blive_ext)

ifneq ($(filter $(B_REFSW_DEBUG),n no_error_messages),)
    DEBUG_SUFFIX=release
else
    DEBUG_SUFFIX=debug
endif

#  Define the out-of-source tree location to hold any generated binaries.
BLIVE_EXT_LIBDIR := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/livemedia/blive_ext/lib/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

BLIVE_EXT_CFLAGS = -I$(BLIVE_EXT_DIR)/include
BLIVE_EXT_LDFLAGS = -L$(BLIVE_EXT_LIBDIR) -lblive_ext
