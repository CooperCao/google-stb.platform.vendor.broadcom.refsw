############################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
############################################################

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
MAGNUM_DIR = ../../../../magnum
ROCKFORD_DIR = ../../../../rockford
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
