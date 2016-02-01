#############################################################################
#
#       Copyright (c) 2003-2013, Broadcom Corporation.
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
#   PLATFORM (97038, 97395, ...)
#   BCHP_REV (B0, B1, C0, ...)
#   DEBUG
#   LINUX
#   ARCH
#   TOOLCHAIN

# Target O/S (can be one of the following: linuxuser vxworks)
ifeq ($(SYSTEM),vxworks)
    TARGET_OS = vxworks
	APP = regscope_server.out
else
ifeq ($(SYSTEM), linuxemu)
    TARGET_OS = linuxemu
	APP = stub_server
else
    TARGET_OS = linuxuser
	APP = regscope_server
endif
endif

# Directories relative to location of makefile
MAGNUM_DIR = ../../../magnum
ROCKFORD_DIR = ../../../rockford
FRAMEWORK_DIR = $(ROCKFORD_DIR)/appframework

TIPSIM ?= n
ifeq (${TIPSIM}, y)
	CFLAGS += -DTIPSIM
else
	CFLAGS += -DSETTOPBOX
endif

ifeq (${SYSTEM},vxworks)
	CFLAGS += -DVXWORKS
else
	ifeq ($(TARGET_OS),linuxemu)
		CFLAGS += -DEMULATION
		STUB_SERVER ?= y
		ifeq ($(STUB_SERVER), y)
			CFLAGS += -DSTUB_SERVER=1
		endif
	endif
	CFLAGS += -DLINUX
endif

# Application object file list, include path, and C flags
APP_SOURCES = ./regscope_server.c ./utils.c

APP_INCLUDES = ./
APP_CFLAGS =
ifneq ($(SYSTEM), linuxemu)
ifneq ($(B_REFSW_ARCH),arm-linux)
APP_LDFLAGS= -static
endif
endif

# Define other chips to use. If not defined, files will not be included.
#BCHP_QAM_CHIP = 3250
#BCHP_SDS_CHIP = 4500
#BCHP_VSB_CHIP = 3510

# Show build commands (y/n)?
VERBOSE?=n

# Debug (y/n)?
ifeq ($(origin DEBUG),undefined)
DEBUG = y
endif

# Must define ENDIAN to be either big or little
ifeq ($(SYSTEM),vxworks)
ENDIAN ?= big
else
ENDIAN ?= little
endif

# Include object file list, include path, and C flags for common utilities and base modules
# (sets MAGNUM_SOURCES, MAGNUM_INCLUDES, MAGNUM_CFLAGS, and MAGNUM_MODULE_DIR)
include $(FRAMEWORK_DIR)/build/magnum.inc

# Include object file list, include path, and C flags for other magnum modules required
# by this application (adds to MAGNUM_SOURCES, MAGNUM_INCLUDES, and MAGNUM_CFLAGS).
# Here is where you also define module specific makefile defines

# At this point, user can add additional files, include paths, and C flags to
# MAGNUM_OBJ, MAGNUM_INC, and MAGNUM_CFLAGS (optional).

# Include other common modules (adds to COMMON_SOURCES, COMMON_INCLUDES, and
# COMMON_CFLAGS)
#include $(FRAMEWORK_DIR)/build/modules/common/fpga.inc

# Specifies how magnum files and application files are built.
include $(FRAMEWORK_DIR)/build/$(TARGET_OS)/build.inc

# End of file
