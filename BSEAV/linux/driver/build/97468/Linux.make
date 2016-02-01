############################################################
#     Copyright (c) 2003-2010, Broadcom Corporation
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
# Created: 02/09/2001 by Marcus Kellerman
#
# $brcm_Log: $
# 
############################################################

# This file creates local copy of the linux include files
# using config file based on a chip number
#
# Input parameters:
# BUILDDIR - destination dir (optional, current directory used by default)
# LINUX - path to the LINUX source tree (optional, /opt/brcm/linux used by default)
# PLATFORM
#
# Output parameters:
# LINUX_INC - path to the linux includes
# OTHER_CLEANS - if linux source is configured, this allows it to be cleaned

include $(BSEAV)/build/platforms.mak

# backward compatibility
ifdef DEFCONFIG
LINUX_DEFCONFIG ?= $(DEFCONFIG)
endif
ifdef PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)
endif
ifdef ARCH
B_REFSW_ARCH ?= $(ARCH)
endif

ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM must be defined)
endif

ifndef BUILDDIR
BUILDDIR=.
endif

#
# If LINUX_INC is defined by the user, don't redefine it.
#
ifdef LINUX_INC

# We still want a rule. If it doesn't exist, then Common.make's checkdirs will catch it.
$(LINUX_INC):

else

#
# This is the default location of the linux kernel source.
# This can be a symlink to your latest version of kernel source.
#
LINUX ?= /opt/brcm/linux
B_REFSW_ARCH ?= mipsel-uclibc

#
# If you have a shared build environment, you need to set up pre-built
# LINUX_INC directories. This is because the 'make menuconfig' process is not
# reentrant and you'll stomp on each other. The SHARED_LINUX_INC
# variable is tested first. If it exists, it's used as LINUX_INC. Otherwise a local
# LINUX_INC is created.
#
# The easiest way to create SHARED_LINUX_INC directories for each platform is to
# make it locally, then copy it into the LINUX directory.
#
# Check for SMP-aware kernel option
BCHP_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_VER)")}')

ifeq ($(SMP),y)
#
# SMP
#
ifeq ($(findstring $(NEXUS_PLATFORM), $(LINUX_REV_SPECIFIC_SMP_PLATFORMS)),)
# Same kernel across all revisions
SHARED_LINUX_INC := $(LINUX)/include.$(B_REFSW_ARCH).$(NEXUS_PLATFORM)-smp
DEFAULT_LINUX_INC := $(BUILDDIR)/linux_include_smp
else
# Rev-specific kernels
SHARED_LINUX_INC := $(LINUX)/include.$(B_REFSW_ARCH).$(NEXUS_PLATFORM)${BCHP_VER_LOWER}-smp
DEFAULT_LINUX_INC := $(BUILDDIR)/linux_include_${BCHP_VER_LOWER}_smp
endif
else
#
# Non-SMP
#
ifeq ($(findstring $(NEXUS_PLATFORM), $(LINUX_REV_SPECIFIC_PLATFORMS)),)
# Same kernel across all revisions
SHARED_LINUX_INC := $(LINUX)/include.$(B_REFSW_ARCH).$(NEXUS_PLATFORM)
DEFAULT_LINUX_INC := $(BUILDDIR)/linux_include
else
# Rev-specific kernels
SHARED_LINUX_INC := $(LINUX)/include.$(B_REFSW_ARCH).$(NEXUS_PLATFORM)${BCHP_VER_LOWER}
DEFAULT_LINUX_INC := $(BUILDDIR)/linux_include_${BCHP_VER_LOWER}
endif
endif

ifeq ($(shell test -d $(SHARED_LINUX_INC) && echo "y"),y)
LINUX_INC = $(SHARED_LINUX_INC)
else
LINUX_INC := $(DEFAULT_LINUX_INC)

# Only clean up a local LINUX_INC, not a shared one.
OTHER_CLEANS += linuxinc_clean
endif

#
# Having this rule allows us to automate all-platform linux include builds
#
build_linux_include: ${LINUX_INC}

###########################################################

#
# Determine .config file for linux kernel
#
PLATFORM_PATH := ${BCHP_VER_LOWER}
# Use c0 for all cx platforms
ifeq ($(findstring $(PLATFORM_PATH), c1 c2 c3),$(PLATFORM_PATH))
PLATFORM_PATH := c0
endif

ifeq ($(NEXUS_PLATFORM),97403)
ifeq ($(PLATFORM_PATH),a1)
PLATFORM_PATH := a0
endif
endif

ifeq ($(NEXUS_PLATFORM),97458)
ifeq ($(PLATFORM_PATH),a1)
PLATFORM_PATH := a0
endif
endif

ifeq ($(shell (grep 'SUBLEVEL = 12' ${LINUX}/Makefile >/dev/null && echo 'y')),y)
ifeq ($(findstring $(NEXUS_PLATFORM),97400 97456),$(NEXUS_PLATFORM))
ifeq ($(findstring $(PLATFORM_PATH),c0 d0 e0),$(PLATFORM_PATH))
PLATFORM_PATH := b0
endif
endif
else
ifeq ($(findstring $(NEXUS_PLATFORM),97400 97456),$(NEXUS_PLATFORM))
ifeq ($(findstring $(PLATFORM_PATH),b0 c0 e0),$(PLATFORM_PATH))
PLATFORM_PATH := d0
endif
endif
endif


ifeq ($(findstring $(NEXUS_PLATFORM),97405 97466 97205 97459),$(NEXUS_PLATFORM))
ifeq ($(findstring $(PLATFORM_PATH),b0 c0),$(PLATFORM_PATH))
PLATFORM_PATH := b0
endif
endif


ifeq ($(SMP),y)
PLATFORM_PATH := ${PLATFORM_PATH}-smp
endif

ifeq ($(CHIP),)
CHIP=$(patsubst 9%,%,$(NEXUS_PLATFORM))
endif

ifeq ($(B_REFSW_ARCH),mips-uclibc)
# big endian
LINUX_DEFCONFIG_ENDIAN=_be
export CONFIG_CPU_BIG_ENDIAN=y
export CONFIG_CPU_LITTLE_ENDIAN=
else
# little endian
LINUX_DEFCONFIG_ENDIAN=
export CONFIG_CPU_LITTLE_ENDIAN=y
export CONFIG_CPU_BIG_ENDIAN=
endif

# Linux kernel doesn't support separate defconfigs for 7466 and 7205
ifeq ($(findstring $(NEXUS_PLATFORM), 97466 97205),$(NEXUS_PLATFORM))
LINUX_DEFCONFIG := arch/mips/configs/bcm97405${PLATFORM_PATH}${LINUX_DEFCONFIG_ENDIAN}_defconfig
endif

# Linux kernel doesn't support separate defconfigs for 97208
ifeq ($(findstring $(CHIP), 7208),$(CHIP))
CHIP=7468
endif

# define LINUX_DEFCONFIG according to standard naming
ifeq ($(LINUX_DEFCONFIG),)
# this is the standard naming for 2.6.30 and later
LINUX_DEFCONFIG := arch/mips/configs/bcm${CHIP}${PLATFORM_PATH}_defconfig

LINUX_DEFCONFIG_EXISTS = $(shell test -e $(LINUX)/$(LINUX_DEFCONFIG) && echo "y")
ifneq ($(LINUX_DEFCONFIG_EXISTS),y)
# this is the standard naming for 2.6.18 and earlier
LINUX_DEFCONFIG := arch/mips/configs/bcm${NEXUS_PLATFORM}${PLATFORM_PATH}${LINUX_DEFCONFIG_ENDIAN}_defconfig
endif
endif


###########################################################
#
# Rule for configuring kernel source locally
#
LINUX_EXISTS = $(shell test -d $(LINUX) && echo "y")
ifneq ($(LINUX_EXISTS),y)
$(error The LINUX make variable points to a directory which does not exist. LINUX is currently defined as ${LINUX}. It should point to your kernel source.)
endif
LINUX_INC_EXISTS = $(shell test -d $(LINUX_INC) && echo "y")
LINUX_VER_GE_2_6 ?= $(shell (grep 'PATCHLEVEL = 6' ${LINUX}/Makefile >/dev/null && echo 'y'))
ifneq ($(LINUX_INC_EXISTS),y)
ifeq ($(LINUX_VER_GE_2_6),y)
# 2.6 kernel include creation
${LINUX_INC}: ${LINUX}
	echo Configuring Linux: $(LINUX_INC) using $(LINUX)/$(LINUX_DEFCONFIG)
	if [ ! -w $(LINUX) ]; then \
		echo; \
		echo ERROR: Configuring kernel source requires write access to $(LINUX).; \
		echo; \
		test -w $(LINUX); \
	fi
	(\
	cd ${LINUX};${CP} -p ${LINUX_DEFCONFIG} .config &&\
	make silentoldconfig ARCH=mips  && \
	make prepare ARCH=mips) && \
	echo "Copying linux include directory..." && \
	${RM} -rf ${LINUX_INC}.tmp.$$$$ && \
	${CP} -a ${LINUX}/include ${LINUX_INC}.tmp.$$$$ &&\
	${CP} -a ${LINUX}/arch/mips/include/asm/*  ${LINUX_INC}.tmp.$$$$/asm/ &&\
	mv ${LINUX_INC}.tmp.$$$$ ${LINUX_INC}

else
# 2.4 kernel include creation
${LINUX_INC}: ${LINUX}
	@echo "Configuring Linux:" $(LINUX_INC) "using" $(LINUX)/$(LINUX_DEFCONFIG)
	@if [ ! -w $(LINUX) ]; then \
		echo; \
		echo "ERROR: Configuring kernel source requires write access to $(LINUX)."; \
		echo; \
		test -w $(LINUX); \
	fi
	@(\
	cd ${LINUX};${CP} -p ${LINUX_DEFCONFIG} .config &&\
	echo 'x'|make menuconfig && make CROSS_COMPILE=$(CROSS_COMPILE) dep) && \
	echo "Copying linux include directory..." && \
	${RM} -rf ${LINUX_INC}.tmp.$$$$ && \
	${CP} -a ${LINUX}/include ${LINUX_INC}.tmp.$$$$ &&\
	mv ${LINUX_INC}.tmp.$$$$ ${LINUX_INC}

endif
endif

linuxinc_clean:
	${RM} -r ${LINUX_INC}

endif
