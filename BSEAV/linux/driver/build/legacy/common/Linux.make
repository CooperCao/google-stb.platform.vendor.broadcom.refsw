############################################################
#     Copyright (c) 2003-2006, Broadcom Corporation
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
# BCM_CHIP - chip number (e.g. 7115)
#
# Output parameters:
# LINUX_INC - path to the linux includes

ifndef PLATFORM
$(error PLATFORM must be defined)
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
ARCH ?= mipsel-uclibc

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
SHARED_LINUX_INC := $(LINUX)/include.$(ARCH).$(PLATFORM)

# Check for SMP-aware kernel option
ifeq ($(SMP),y)
SHARED_LINUX_INC := $(SHARED_LINUX_INC)-smp
endif

ifeq ($(shell test -d $(SHARED_LINUX_INC) && echo "y"),y)
LINUX_INC = $(SHARED_LINUX_INC)
else
LINUX_INC := $(BUILDDIR)/linux_include

# Check for SMP-aware kernel option
ifeq ($(SMP),y)
LINUX_INC := $(LINUX_INC)-smp
endif

# Only clean up a local LINUX_INC, not a shared one.
OTHER_CLEANS += linuxinc_clean
endif

#
# Having this rule allows us to automate all-platform linux include builds
#
build_linux_include: ${LINUX_INC}

#
# If the defconfig is something other than the standard name,
# it must be defined in the platform-specific Makefile.
#
ifeq ($(ARCH),mipsel-linux)
# glibc
DEFCONFIG ?= arch/mips/defconfig-brcm-$(BCM_CHIP)
else
# uclibc, default
DEFCONFIG ?= arch/mips/defconfig-brcm-bb-$(BCM_CHIP)
endif

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
	echo "Configuring Linux:" $(LINUX_INC) "using" $(LINUX)/$(DEFCONFIG)
	if [ ! -w $(LINUX) ]; then \
		echo; \
		echo "ERROR: Configuring kernel source requires write access to $(LINUX)."; \
		echo; \
		test -w $(LINUX); \
	fi
	(\
	cd ${LINUX};${CP} -p ${DEFCONFIG} .config &&\
	make silentoldconfig ARCH=mips  && \
	make include/linux/version.h ARCH=mips &&\
	make include/asm ARCH=mips && make include/config/MARKER ARCH=mips)  && \
	echo "Copying linux include directory..." && \
	${RM} -rf ${LINUX_INC}.tmp.$$$$ && \
	${CP} -a ${LINUX}/include ${LINUX_INC}.tmp.$$$$ &&\
	mv ${LINUX_INC}.tmp.$$$$ ${LINUX_INC}

else
# 2.4 kernel include creation 
${LINUX_INC}: ${LINUX}
	@echo "Configuring Linux:" $(LINUX_INC) "using" $(LINUX)/$(DEFCONFIG)
	@if [ ! -w $(LINUX) ]; then \
		echo; \
		echo "ERROR: Configuring kernel source requires write access to $(LINUX)."; \
		echo; \
		test -w $(LINUX); \
	fi
	@(\
	cd ${LINUX};${CP} -p ${DEFCONFIG} .config &&\
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

