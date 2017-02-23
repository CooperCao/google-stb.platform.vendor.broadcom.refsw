############################################################
#  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# Module Description:
#   This file defines the OS-specific tools for building in linuxuser mode.
#
############################################################

############################################################
# Overview:
#
# This file should only be included by nexus_defs.inc
#
############################################################

#
# Default to little-endian
#
ifndef B_REFSW_ARCH
B_REFSW_ARCH = mipsel-linux
endif

#
# Set variables based on the toolchain
#
ifdef B_REFSW_KERNEL_CROSS_COMPILE
B_REFSW_CROSS_COMPILE := $(B_REFSW_KERNEL_CROSS_COMPILE)
endif

ifeq ($(B_REFSW_CROSS_COMPILE),)

    ifeq ($(filter ${B_REFSW_ARCH},  aarch64-linux arm-linux), ${B_REFSW_ARCH})
    B_REFSW_CROSS_COMPILE = ${B_REFSW_ARCH}-
    else
    # set default B_REFSW_CROSS_COMPILE based on B_REFSW_ARCH
    ifdef NEXUS_ANDROID_SUPPORT
    B_REFSW_CROSS_COMPILE=mipsel-linux-uclibc-
    else
    ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc), ${B_REFSW_ARCH})
    B_REFSW_CROSS_COMPILE = mips-linux-
    else
    B_REFSW_CROSS_COMPILE = mipsel-linux-
    endif
    endif
    endif
    
    # if you don't set B_REFSW_CROSS_COMPILE, you can set B_REFSW_TOOLCHAIN_DIR to give the default B_REFSW_CROSS_COMPILE an absolute path
    ifneq ($(B_REFSW_TOOLCHAIN_DIR),)
    B_REFSW_CROSS_COMPILE := $(B_REFSW_TOOLCHAIN_DIR)/$(B_REFSW_CROSS_COMPILE)
    endif
else
    # user has set B_REFSW_CROSS_COMPILE.
    # you can choose to set it with or without an absolute path, but you can't also set B_REFSW_TOOLCHAIN_DIR.
endif

# Define make variables
AS      = $(B_REFSW_CROSS_COMPILE)as
LD      = $(B_REFSW_CROSS_COMPILE)ld
CC      = $(strip ${B_REFSW_CCACHE} $(B_REFSW_CROSS_COMPILE)gcc)
CXX     = $(strip ${B_REFSW_CCACHE} $(B_REFSW_CROSS_COMPILE)c++)
AR      = $(B_REFSW_CROSS_COMPILE)ar
NM      = $(B_REFSW_CROSS_COMPILE)nm
STRIP   = $(B_REFSW_CROSS_COMPILE)strip
OBJCOPY = $(B_REFSW_CROSS_COMPILE)objcopy
OBJDUMP = $(B_REFSW_CROSS_COMPILE)objdump
RANLIB  = $(B_REFSW_CROSS_COMPILE)ranlib
READELF  = $(B_REFSW_CROSS_COMPILE)readelf
MKDIR   = mkdir -p
PWD     = pwd
MV      = mv
PERL    = perl

CPP     = $(CC) -E
CP      = cp -fp
RM      = rm -f
SORT    = sort
SED     = sed
TOUCH   = touch
AWK     = awk

#
# Select toolchain for glibc toolchains
#
B_REFSW_TOOLCHAIN_DIR ?= $(shell dirname $(shell dirname $(shell which $(filter-out ccache,$(CC)))))

# Define BSTD_CPU_ENDIAN correctly
ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc), ${B_REFSW_ARCH})
NEXUS_ENDIAN = BSTD_ENDIAN_BIG
else
NEXUS_ENDIAN = BSTD_ENDIAN_LITTLE
endif

