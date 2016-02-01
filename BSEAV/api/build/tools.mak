############################################################################
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
# $brcm_Log: $
# 
###########################################################################

# This make include defines macros for cross-development tools.
# It is used in a wide range of environments, so don't add anything
# that doesn't belong here.

# ARCH must indentify the CPU and operating system being compiled for.
# If you chose a value of ARCH which doesn't uniquely identify this, you
# will have an unstable or hacked-up build system elsewhere.

# Valid values include:
#  i386-linux = Intel Linux systems(e.g. unix host compiler)
#  i386-mingw = host compiler in Windows Mingw environment
#  mipsel-linux = uclibc, little endian Linux on MIPS
#  mips-linux = uclibc, big endian Linux on MIPS
#  mipsel-uclibc = uclibc (busybox), little endian Linux on MIPS
#  mips-uclibc = uclibc (busybox), big endian Linux on MIPS
#  mips-vxworks = vxworks, big endian on MIPS

# Handle vxworks from either the SYSTEM or ARCH variable.
ifeq ($(B_REFSW_ARCH),mips-vxworks)
SYSTEM=vxworks
endif

ifeq ($(SYSTEM),vxworks)

B_REFSW_ARCH = mips-vxworks
CROSS_COMPILE = mips
CC      = cc${CROSS_COMPILE}
CXX     = c++${CROSS_COMPILE}
LD      = ld${CROSS_COMPILE}
AR      = ar${CROSS_COMPILE}
NM      = nm${CROSS_COMPILE}
STRIP   = strip${CROSS_COMPILE}
OBJCOPY = objcopy${CROSS_COMPILE}
OBJDUMP = objdump${CROSS_COMPILE}
RANLIB  = ranlib${CROSS_COMPILE}

# MKDIR must make recursive dirs, and not fail if already existing
ifeq ($(OSTYPE),linux)
MKDIR   = mkdir -p
PWD     = pwd
MV      = mv
else
ifeq ($(vxWorksVersion),6)
MKDIR   = mkdir -p
PWD     = pwd
MV      = mv
else
# These are really DOS options:
MKDIR   = -mkdir
PWD     = chdir
MV      = move
endif
endif

else

#
# Default toolchain
#
B_REFSW_ARCH ?= mipsel-linux
SYSTEM ?= linux

#
# Set variables based on the toolchain
#
ifeq ($(filter ${B_REFSW_ARCH}, i386-linux i386-mingw), ${B_REFSW_ARCH})
# native compiler
B_REFSW_CROSS_COMPILE ?=
TOOLCHAIN_DIR=/usr/bin
else
# cross compiler
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux arm-linux mips-linux mips-uclibc mips-linux-uclibc), ${B_REFSW_ARCH})
# Use RefSW Arch value for cross compile
B_REFSW_CROSS_COMPILE ?= $(B_REFSW_ARCH)-
else
# Default Little Endian MIPS
B_REFSW_CROSS_COMPILE ?= mipsel-linux-
endif
TOOLCHAIN_DIR:=$(shell dirname $(shell dirname $(shell which $(B_REFSW_CROSS_COMPILE)gcc 2>/dev/null) 2>/dev/null))
endif

# Define make variables
AS      = $(B_REFSW_CROSS_COMPILE)as
LD      = $(B_REFSW_CROSS_COMPILE)ld
CC      = $(B_REFSW_CROSS_COMPILE)gcc
CXX     = $(B_REFSW_CROSS_COMPILE)c++
AR      = $(B_REFSW_CROSS_COMPILE)ar
NM      = $(B_REFSW_CROSS_COMPILE)nm
STRIP   = $(B_REFSW_CROSS_COMPILE)strip
OBJCOPY = $(B_REFSW_CROSS_COMPILE)objcopy
OBJDUMP = $(B_REFSW_CROSS_COMPILE)objdump
RANLIB  = $(B_REFSW_CROSS_COMPILE)ranlib
MKDIR   = mkdir -p
PWD     = pwd
MV      = mv

endif

# These are operations common to all environments.
CPP     = $(CC) -E
CP      = cp -f
RM      = rm -f
SORT    = sort
SED     = sed
TOUCH   = touch
AWK 	= awk

# Define options for quiet builds
export VERBOSE
ifneq ($(VERBOSE),)
Q_:=
else
# This was Q_?=@, but that caused vxWorks problems.  VERBOSE=y must be used now to get verbose msgs.
Q_:=@
MAKEFLAGS += --no-print-directory
endif

