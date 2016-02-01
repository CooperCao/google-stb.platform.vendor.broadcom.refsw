#############################################################################
#
#       Copyright (c) 2005-2011, Broadcom Corporation.
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
APP = load_rts.out
else
APP = load_rts
endif

# Application object file list, include path, C flags, and link
# flags
APP_SOURCES= \
	./load_rts.c

# Initial application settings
APP_INCLUDES =
APP_CFLAGS   =
APP_LDFLAGS  =

# Must define ENDIAN to be either big or little
ENDIAN = little

# Want to see details (y/n)?
VERBOSE = n

# Debug (y/n)?
DEBUG ?= y
ifeq ($(DEBUG),y)
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

ifeq ($(BTST_USE_BSUR),1)
CFLAGS += -DBTST_USE_BSUR
APP_SOURCES += \
	bgfx_image.c
endif

# Include object file list, include path, and C flags for common utilities and base modules
# (sets MAGNUM_SOURCES, MAGNUM_INCLUDES, MAGNUM_CFLAGS, and MAGNUM_MODULE_DIR)
include $(FRAMEWORK_DIR)/build/magnum.inc

# Include object file list, include path, and C flags for other magnum modules required
# by this application (adds to MAGNUM_SOURCES, MAGNUM_INCLUDES, and MAGNUM_CFLAGS).
# Here is where you also define module specific makefile defines
BVDC_MACROVISION ?= y

ifeq ($(BTST_USE_BSUR),1)
include $(MAGNUM_DIR)/portinginterface/vdc/bvdc.inc
endif

# At this point, user can add additional files, include paths, and C flags to
# MAGNUM_SOURCES, MAGNUM_INCLUDES, and MAGNUM_CFLAGS (optional).

# Include other common modules (adds to COMMON_SOURCES, COMMON_INCLUDES, and
# COMMON_CFLAGS)

# Specifies how magnum files and application files are built.
include $(FRAMEWORK_DIR)/build/$(TARGET_OS)/build.inc

# End of file
