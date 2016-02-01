############################################################################
#     Copyright (c) 2003-2011, Broadcom Corporation
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
#  i386-linux = Intel Linux systems
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
ifeq ($(ARCH),i386-linux)
CROSS_COMPILE ?=
TOOLCHAIN_DIR=/usr/bin
else
CROSS_COMPILE ?= $(B_REFSW_ARCH)-
ifeq ($(B_REFSW_ARCH),mipsel-linux)
#
# Discover the uclibc toolchain directory assuming the compiler exists in bin subdir
# Use which and dirname bash shell commands.
#
TOOLCHAIN_DIR=$(shell dirname $(shell dirname $(shell which mipsel-linux-gcc)))
else
TOOLCHAIN_DIR=$(shell dirname $(shell dirname $(shell which mips-linux-gcc)))
endif
endif

# Define make variables
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)c++
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB  = $(CROSS_COMPILE)ranlib
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

# Define options for quiet builds
export VERBOSE
ifneq ($(VERBOSE),)
Q_:=
else
# This was Q_?=@, but that caused vxWorks problems.  VERBOSE=y must be used now to get verbose msgs.
Q_:=@
MAKEFLAGS += --no-print-directory
endif

