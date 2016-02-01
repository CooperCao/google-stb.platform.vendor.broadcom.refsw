#############################################################################
#
#       Copyright (c) 2005-2013, Broadcom Corporation.
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
# File Description:
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################

# Requires the use of the 'plat' script which defines the following:
#   PLATFORM (97038, 97398, ...)
#   BCHP_VER (B0, B1, C0, ...)
#   BCHP_7411_VER (C0, D0, ...) plus versions for other secondary chipsets
#   DEBUG
#   LINUX
#   ARCH
#   TOOLCHAIN

# Target O/S (can be one of the following: linuxuser vxworks)
ifeq ($(SYSTEM),vxworks)
    TARGET_OS = vxworks
else
ifeq ($(SYSTEM), linuxemu)
    TARGET_OS = linuxemu
else
    TARGET_OS = linuxuser
endif
endif


# Directories relative to location of makefile
MAGNUM_DIR = ../../../magnum
ROCKFORD_DIR = ../../../rockford
FRAMEWORK_DIR = $(ROCKFORD_DIR)/appframework

# Name of application
ifeq ($(SYSTEM),vxworks)
APP = simple_vdc.out
else
APP = simple_vdc
endif

# Application object file list, include path, C flags, and link
# flags
APP_SOURCES= \
	./simple_vdc.c

# Initial application settings
APP_INCLUDES =
APP_CFLAGS   =
APP_LDFLAGS  =

# Must define ENDIAN to be either big or little
ENDIAN = little

# Want to see details (y/n)?
VERBOSE = n

# Debug (y/n)?
B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
APP_CFLAGS += -DBDBG_DEBUG_BUILD
endif

ifeq ($(TARGET_OS),vxworks)
CFLAGS += -DVXWORKS
else
CFLAGS += -DLINUX
	ifeq ($(TARGET_OS),linuxclient)
	CFLAGS += -DIKOS_EMULATION
	endif
endif

BVDC_BVN_DEBUG ?= y
ifeq ($(BVDC_BVN_DEBUG), y)
	CFLAGS += -DBVDC_SUPPORT_BVN_DEBUG=1
endif

# Can have Macrovision or DCS analog copy protection. But not both.
BVDC_MACROVISION ?= y
BVDC_DCS ?= n
ifeq ($(BVDC_MACROVISION), y)
        CFLAGS += -DBVDC_MACROVISION=1
        CFLAGS += -DMACROVISION_SUPPORT=1
else
        CFLAGS += -DBVDC_MACROVISION=0
endif

ifeq ($(BVDC_DCS), y)
        ifeq ($(BVDC_MACROVISION),y)
                error Macrovision plus DCS not allowed
        endif
        CFLAGS += -DDCS_SUPPORT=1
endif

# Include object file list, include path, and C flags for common utilities and base modules
# (sets MAGNUM_SOURCES, MAGNUM_INCLUDES, MAGNUM_CFLAGS, and MAGNUM_MODULE_DIR)
include $(FRAMEWORK_DIR)/build/magnum.inc

# Include object file list, include path, and C flags for other magnum modules required
# by this application (adds to MAGNUM_SOURCES, MAGNUM_INCLUDES, and MAGNUM_CFLAGS).
# Here is where you also define module specific makefile defines
include $(MAGNUM_DIR)/portinginterface/vdc/bvdc.inc

# At this point, user can add additional files, include paths, and C flags to
# MAGNUM_SOURCES, MAGNUM_INCLUDES, and MAGNUM_CFLAGS (optional).

# Include other common modules (adds to COMMON_SOURCES, COMMON_INCLUDES, and
# COMMON_CFLAGS)

# Specifies how magnum files and application files are built.
include $(FRAMEWORK_DIR)/build/$(TARGET_OS)/build.inc

###############################################################################
# This is an OPTIONAL test of completeness for BVDC preprocessor definitions. #
# To use this test, perform the following steps:                              #
#     make macro_test                                                         #
#     touch simple_vdc.c                                                      #
#     make BVDC_MACRO_TEST=y                                                  #
###############################################################################
macro_test: $(ODIR)/macro_test.h
ifeq ($(BVDC_MACRO_TEST), y)
APP_CFLAGS += -DBVDC_MACRO_TEST=y -I$(ODIR)
endif
$(ODIR)/macro_test.h: macro_list test_builder
	bash ./test_builder macro_list $@

.PHONY: macro_test

# End of file
