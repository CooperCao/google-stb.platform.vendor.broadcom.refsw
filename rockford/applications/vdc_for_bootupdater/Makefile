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

# To build with shrinked vdc for BOOTUPDATER
# make -j 8 B_PI_FOR_BOOTUPDATER=y B_REFSW_DEBUG=n
# must update this Makefile to pick the right display format you need
# must update the memc usage for RDC, VDC default, and gfx surface in test.c for your board
# could pass NULL for pDefSettings when BVDC_Open is called, could also
#       get default and modify BVDC_Settings before BVDC_Open as illustrated in test.c
# must take off "CFLAGS += -g" every where
# debug notes: To make debug version: make -j 8 B_PI_FOR_BOOTUPDATER=y B_REFSW_DEBUG=y
#              only add "CFLAGS += -g" during debug
#              if gfx win does not show and have ENABLE_ERROR, likely wrong m2mc is used
#              you could read linker generated output.map to calculate VDC size
ifeq ($(B_PI_FOR_BOOTUPDATER),y)
CFLAGS += -DBVDC_FOR_BOOTUPDATER=1
CFLAGS += -DBFMT_DO_PICK=1
CFLAGS += -DBFMT_PICK_e480p=1
CFLAGS += -DBFMT_PICK_ePAL_M=1
CFLAGS += -DDISP_FORMAT0=BFMT_VideoFmt_e480p
CFLAGS += -DDISP_FORMAT1=BFMT_VideoFmt_ePAL_M
#CFLAGS += -DBFMT_PICK_eNTSC
#CFLAGS += -DBFMT_PICK_e720p=1
#CFLAGS += -DBFMT_PICK_e1080i=1
#CFLAGS += -DBFMT_PICK_e1080p=1
#CFLAGS += -DBFMT_PICK_e1080p_24Hz=1
#CFLAGS += -DBFMT_PICK_e1080p_30Hz=1
BVDC_MACROVISION = n
BVDC_DCS = n
CFLAGS += -Os -fdata-sections -ffunction-sections -Wl,--gc-sections
CFLAGS += -Wl,-Map=output.map
else
CFLAGS += -DDISP_FORMAT0=BFMT_VideoFmt_e480p
CFLAGS += -DDISP_FORMAT1=BFMT_VideoFmt_ePAL_M
endif ### ifeq ($(B_PI_FOR_BOOTUPDATER),y)

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
APP = test_vdc_for_bootupdater.out
else
APP = test_vdc_for_bootupdater
endif

# Application object file list, include path, C flags, and link
# flags
APP_SOURCES= \
	./test.c

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
endif

BVDC_BVN_DEBUG ?= y
ifeq ($(BVDC_BVN_DEBUG), y)
	CFLAGS += -DBVDC_SUPPORT_BVN_DEBUG=1
endif

CFLAGS += -DBVDC_MACROVISION=0

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
#     touch test.c                                                            #
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
