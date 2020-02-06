#############################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#############################################################################

SHELL := /bin/bash

THIS_DIR := BSEAV/connectivity/wlan
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV_TOP}/../${B_REFSW_OBJ_DIR}

BSEAV_BIN_DIR=${B_REFSW_OBJ_ROOT}/BSEAV/bin
NEXUS_BIN_DIR=${B_REFSW_OBJ_ROOT}/nexus/bin

# Build the name of this makefile, relative to B_REFSW_TOP (used for recipe tracing)
# Keep this line before any includes!
B_THIS_MAKEFILE_NAME := $(subst $(abspath ${NEXUS_TOP}/..),,$(abspath $(lastword $(MAKEFILE_LIST))))
ifeq (${B_WLAN_MAKEFILE_ENABLE_DEBUG},y)
    B_TRACE_RECIPE_BEGIN = @echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>  ${B_THIS_MAKEFILE_NAME}: Begin recipe: $@"
    B_TRACE_RECIPE_END   = @echo "<<<<<<<<<<<<<<<<<<<<<<<<<<<<  ${B_THIS_MAKEFILE_NAME}:   End recipe: $@"
endif


#  Allow for building without nexus source tree.
NEXUS_DEFS_EXISTS = $(shell test -e ${NEXUS_TOP}/build/nexus_defs.inc && echo y)
ifeq (${NEXUS_DEFS_EXISTS},y)
    # include cross-compiler definitions
    include ${NEXUS_TOP}/build/nexus_defs.inc
else
    ifeq (${NEXUS_ENDIAN},)
        # Define BSTD_CPU_ENDIAN correctly
        ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc), ${B_REFSW_ARCH})
            NEXUS_ENDIAN = BSTD_ENDIAN_BIG
        else
            NEXUS_ENDIAN = BSTD_ENDIAN_LITTLE
        endif
    endif

    B_REFSW_VERBOSE ?= ${VERBOSE}
    B_REFSW_CROSS_COMPILE ?= ${B_REFSW_ARCH}-
    MKDIR ?= mkdir -p
    CP ?= cp -fp
    Q_ ?= @

endif

include $(BSEAV_TOP)/connectivity/wlan/wlan.inc

STB_BASE_SOC_FEAT := stbsoc
STB_FULL_SOC_FEAT := debug-cfg80211-${STB_BASE_SOC_FEAT}
STB_7271_FEAT := debug-cfg80211-stb7271
STB_DEF_FEAT  := debug-stb7271
$(warning "building WL flavor: ${B_WLAN_VER}")
STB_FEAT := $(STB_BASE_SOC_FEAT)

#  Set up some make output filtering to get a medium amount of neatly formatted output (unless VERBOSE is enabled).
B_WLAN_DRIVER_MAKE_REDIR:=
B_WLAN_WLEXE_MAKE_REDIR:=

ifeq ($(B_REFSW_VERBOSE),)
    B_WLAN_DRIVER_MAKE_REDIR :=   |                                                 \
        egrep --line-buffered  -e "^\# Build dependencies"                          \
                               -e "^Making .*\.depend$$"                            \
                               -e "^test -r .* .$$"                                 \
                               -e "^  CC \[M\] .*\.o$$"                             \
                               -e "^\# Strip off debugging symbols"                 \
                               -e "error:" -e "In function"         |               \
        sed -u                                                                      \
                -e  "s/^Making .\(.*\)\.depend$$/[Depends... \1 (WLAN)]/ "          \
                -e  "s/^test -r \(.*\) ||.*$$/[Symlinking... \1 (WLAN)]/ "          \
                -e  "s/^  CC \[M\] .*\/\(.*\)\.o$$/[Build... \1 (WLAN)]/ "          \
                -e  "s/^\# Build dependencies/Checking WLAN dependencies.../ "      \
                -e  "s/^\# Strip off debugging symbols/Stripping WLAN symbols.../ " \
        ; exit $${PIPESTATUS[0]}

    B_WLAN_WLEXE_MAKE_REDIR :=  |                                                               \
        egrep --line-buffered  -e "^${B_REFSW_CROSS_COMPILE}.*\.c"  |                           \
        sed -u                                                                                  \
                -e  "s|^${B_REFSW_CROSS_COMPILE}.*[/ ]\([^ ]*\.c\)$$|[Compile... \1 (WLAN)]|"  \
        ; exit $${PIPESTATUS[0]}

endif

# Set up the make/environment variables needed by the WLAN makefile.
# TARGETARCH: x86, mips, arm, or arm_le
#
# If TARGETARCH is mips
#   TARGETMACH: mipsel (little endian) or mipseb (big endian)
#
# If TARGETARCH is arm or arm_le
#   TARGETMACH: arm (little endian) or armeb (big endian)

WLAN_DEFAULT_BUILDCFG?=debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc

# Handle 32-bit ARM (B_REFSW_ARCH==arm*)
ifneq ($(filter arm%,${B_REFSW_ARCH}),)
    ifeq (${NEXUS_ENDIAN},BSTD_ENDIAN_LITTLE)
        # ARM LE
	#Added for stbapd
        export ARCHTOOL=arm-linux
        export WLARCH=arm
	# end of for stbapd
        export ARCH=arm
        export TARGETARCH?=arm
               TARGETARCH_FOR_WL=armv7l
        export TARGETMACH?=arm
        export TARGETENV?=linuxarm
		export BUILDCFG=${WLAN_DEFAULT_BUILDCFG}-armv7l
    else ifeq (${NEXUS_ENDIAN},BSTD_ENDIAN_BIG)
        #  ARM BE
        $(error ${B_REFSW_ARCH} not supported with big endian)
    else
        $(error Cannot determine endianness! NEXUS_ENDIAN: ${NEXUS_ENDIAN})
    endif
# Handle 64-bit ARM (B_REFSW_ARCH==aarch64-linux)
else ifeq (${B_REFSW_ARCH},aarch64-linux)
    ifeq (${NEXUS_ENDIAN},BSTD_ENDIAN_LITTLE)
        # ARM LE
	#Added for stbapd
        export ARCHTOOL=aarch64-linux
        export WLARCH=arm64
	# end of for stbapd
        export ARCH=arm64
        export TARGETARCH?=arm64
               TARGETARCH_FOR_WL=arm64
        export TARGETMACH?=arm
        export TARGETENV?=linuxarm
		export BUILDCFG=${WLAN_DEFAULT_BUILDCFG}-armv8
    else ifeq (${NEXUS_ENDIAN},BSTD_ENDIAN_BIG)
        #  ARM BE
        $(error ${B_REFSW_ARCH} not supported with big endian)
    else
        $(error Cannot determine endianness! NEXUS_ENDIAN: ${NEXUS_ENDIAN})
    endif
else
    $(error I do not understand B_REFSW_ARCH: ${B_REFSW_ARCH})
endif

export LINUXDIR?=${LINUX}
export LINUXVER?=$(shell cat ${LINUXDIR}/include/config/kernel.release)

# This is the directory (relative to ${B_WLAN_OOSRC_VER_DIR}/main/) where the driver binary will be placed.
export TARGETDIR?=bin

# USBSHIM=0 if bcmdbus is used.  This is used for STB LINUX
export USBSHIM?=0

export CROSS_COMPILE?=${B_REFSW_CROSS_COMPILE}

###################################################################
# STBLINUX=0; not use for STB
# STBLINUX=1; use for STB
###################################################################
export STBLINUX=1
###################################################################
# SHORTER_PATH=0; default target name in wl/linux/obj-{target}
# SHORTER_PATH=1; reduce path lengths. replace target with chipver
###################################################################
export SHORTER_PATH?=0

###################################################################
# B_WLAN_STBAPD=0; default: Do not Build
# B_WLAN_STBAPD=1; Build STBAP Apps
###################################################################
export B_WLAN_STBAPD?=0
ifneq (,$(findstring -stbapd-,$(WLAN_DEFAULT_BUILDCFG)))
B_WLAN_STBAPD=1
endif
###################################################################
#  Default rule:
###################################################################
.PHONY: all
all: install
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

#  Define the name of a marker file that will updated after the WLAN submakes have been run.
#  We'll only need to run the submakes again when source files (or directories) have been
#  modified after the marker file.
B_WLAN_LAST_BUILD_MARKER_FILE:=$(B_WLAN_OOSRC_VER_DIR)/b_wlan_last_build_marker.txt

# Try to find a WLAN file that's newer than the "last build" marker file.
# so we can make the driver build dependent on it and cause the driver to get rebuilt.
# We don't need all the newer files, just one will do.
B_WLAN_DRIVER_UPDATED_FILE_LIST:=
ifeq ($(wildcard ${B_WLAN_LAST_BUILD_MARKER_FILE}),)
    B_WLAN_DRIVER_UPDATED_FILE_LIST:="No marker file"
else
    #  See if anything in the Out-of-source directories have been updated.
    B_WLAN_DRIVER_UPDATED_FILE_LIST:=$(shell find ${B_WLAN_OOSRC_VER_DIR}   -newer ${B_WLAN_LAST_BUILD_MARKER_FILE} | head -n1 )

    #  If nothing, then check the in-source tree.
    ifeq (${B_WLAN_DRIVER_UPDATED_FILE_LIST},)
        B_WLAN_DRIVER_UPDATED_FILE_LIST:=$(shell find ${B_WLAN_INSRC_VER_DIR}  -newer ${B_WLAN_LAST_BUILD_MARKER_FILE} | head -n1 )
    endif
endif

#External BCA chips
ifneq (${B_WLAN_VER},7271)
B_WLAN_EXTRA_INSTALL += ${B_WLAN_VER}_install
B_WLAN_EXTRA_CLEAN += ${B_WLAN_VER}_clean
#used for 4375
a:=$(shell ${MKDIR} -p ${B_WLAN_OOSRC_VER_DIR} )
a:=$(shell $(CP) -a -u  ${B_WLAN_INSRC_DIR}/utils  ${B_WLAN_OOSRC_DIR} )
else
# Copy any updated files from the in-source tree to the out-of-source tree.
$(info  Syncing WLAN Out-of-source tree  to ${B_WLAN_OOSRC_VER_DIR}...)
a:=$(shell ${MKDIR} -p ${B_WLAN_OOSRC_VER_DIR} )
a:=$(shell $(CP) -a -u  ${B_WLAN_INSRC_VER_DIR}/components   ${B_WLAN_OOSRC_VER_DIR} )
a:=$(shell $(CP) -a -u  ${B_WLAN_INSRC_VER_DIR}/src   ${B_WLAN_OOSRC_VER_DIR} )
a:=$(shell $(CP) -a -u  ${B_WLAN_INSRC_DIR}/utils   ${B_WLAN_OOSRC_DIR} )
endif

#STBAPD variables
ifeq ($(B_WLAN_STBAPD),1)
#jsonc paths
include $(BSEAV_TOP)/opensource/jsonc/jsonc.inc
$(info  Syncing WLAN Stbapd Out-of-source tree  to ${B_WLAN_OOSRC_VER_DIR}...)
a:=$(shell $(CP) -a -u  ${B_WLAN_INSRC_DIR}/stbapd   ${B_WLAN_OOSRC_DIR} )
$(info  copying open sources from ${B_WLAN_INSRC_DIR}/../../opensource to ${B_WLAN_OOSRC_DIR}/../../ )
a:=$(shell $(CP) -a -u  ${B_WLAN_INSRC_DIR}/../../opensource  ${B_WLAN_OOSRC_DIR}/../../ )
B_WLAN_STBAPD_INSTALL = stbapd_install
B_WLAN_STBAPD_CLEAN = stbapd_clean_all
B_WLAN_STBAPD_OOSRC_DIR:=${B_WLAN_OOSRC_DIR}/stbapd/router

# alot of apps here should we list all the apps.
export INSTALLDIR=${B_WLAN_OOSRC_DIR}/stbapd/router/src/router/${ARCH}-glibc/target
B_WLAN_STBAPD_RESULT_OOSRC_DIR:=${B_WLAN_OOSRC_DIR}/stbapd/router/src/router/${ARCH}-glibc/target
ifeq ($(BUILD_BRCM_HOSTAPD),y)
B_WLAN_STBAPD_INSTALL_PATH:=${BSEAV_BIN_DIR}/stbapd/ext_auth
else
B_WLAN_STBAPD_INSTALL_PATH:=${BSEAV_BIN_DIR}/stbapd/int_auth
endif
#export for driver
export CONFIG_STBAP=y
export EMF_EXTRA_SYMBOLS=${B_WLAN_STBAPD_OOSRC_DIR}/src/router/emf/igs/obj-igs-${LINUXVER}/Module.symvers
export DPSTA_EXTRA_SYMBOLS=${B_WLAN_STBAPD_OOSRC_DIR}/src/router/dpsta/obj-dpsta-${LINUXVER}/Module.symvers
ifneq (${NEXUS_PLATFORM},97271)
export KBUILD_EXTRA_SYMBOLS=${B_WLAN_STBAPD_OOSRC_DIR}/Module.symvers
else
# Revisit what to do here  thought we didn't need wlplat or other stuff for this?
export KBUILD_EXTRA_SYMBOLS=${B_WLAN_OOSRC_VER_DIR}/src/wlplat/Module.symvers
endif
else
# B_WLAN_STBAPD=0
ifneq (,$(findstring -stbapd-,$(WLAN_DEFAULT_BUILDCFG)))
$(error B_WLAN_STABAPD == 0 but WLAN_DEFAULT_BUILDCFG($(WLAN_DEFAULT_BUILDCFG)) has -stbapd-)
endif
endif

# Build a list of resulting filenames (without directories) that we want from the WLAN build.
# Start with the wl.ko driver:
B_WLAN_DRIVER_RESULT_FILE_NAME:=wl.ko
B_WLAN_DRIVER_RESULT_WLPLAT_FILE_NAME:=wlplat.ko
B_WLAN_DRIVER_RESULT_EMF_FILE_NAME:=emf.ko
B_WLAN_DRIVER_RESULT_IGS_FILE_NAME:=igs.ko

#Define path to epiver.h
B_WLAN_EPIVER_OOSRC_FILE:=$(WLAN_DRIVER_PATH)/src/include/epiver.h
B_WLAN_EPIVER_OOSRC_DIR:=$(WLAN_DRIVER_PATH)/src/include

# Define the out-of-source dir where the wlan driver Makefile lives.
B_WLAN_DRIVER_BUILD_OOSRC_DIR:=$(B_WLAN_OOSRC_VER_DIR)/src/wl/linux
ifeq ($(SHORTER_PATH),1)
B_WLAN_DRIVER_RESULT_OOSRC_DIR:=$(B_WLAN_DRIVER_BUILD_OOSRC_DIR)/obj-${BCHP_VER}-${LINUXVER}
else
B_WLAN_DRIVER_RESULT_OOSRC_DIR:=$(B_WLAN_DRIVER_BUILD_OOSRC_DIR)/obj-${BUILDCFG}-${LINUXVER}
endif
B_WLAN_DRIVER_RESULT_OOSRC_PATH?=${B_WLAN_DRIVER_RESULT_OOSRC_DIR}/${B_WLAN_DRIVER_RESULT_FILE_NAME}
B_WLAN_DRIVER_RESULT_WLPLAT_OOSRC_PATH?=${B_WLAN_DRIVER_RESULT_OOSRC_DIR}/${B_WLAN_DRIVER_RESULT_WLPLAT_FILE_NAME}
B_WLAN_DRIVER_RESULT_EMF_OOSRC_PATH?=${B_WLAN_DRIVER_RESULT_OOSRC_DIR}/${B_WLAN_DRIVER_RESULT_EMF_FILE_NAME}
B_WLAN_DRIVER_RESULT_IGS_OOSRC_PATH?=${B_WLAN_DRIVER_RESULT_OOSRC_DIR}/${B_WLAN_DRIVER_RESULT_IGS_FILE_NAME}

# Then the "wl" utility executable:
B_WLAN_WLEXE_INSTALL_FILE_NAME:=wl
B_WLAN_WLEXE_RESULT_FILE_NAME:=${B_WLAN_WLEXE_INSTALL_FILE_NAME}${TARGETARCH_FOR_WL}
# Define the out-of-source dir where the "wl" Makefile lives.
B_WLAN_WLEXE_RESULT_OOSRC_DIR:=${B_WLAN_OOSRC_VER_DIR}/src/wl/exe
B_WLAN_WLEXE_RESULT_OOSRC_PATH?=${B_WLAN_WLEXE_RESULT_OOSRC_DIR}/${B_WLAN_WLEXE_RESULT_FILE_NAME}

# Then the "mfgtest" utility executable:
B_WLAN_MFGTEST_RESULT_FILE_NAME_LIST:=nvram libnvram.so nvramd envram envrams
# Define the out-of-source dir where the mfgtest Makefile lives.
B_WLAN_MFGTEST_OOSRC_DIR:=${B_WLAN_OOSRC_DIR}/utils/mfgtest
B_WLAN_MFGTEST_BUILD_OOSRC_DIR:=${B_WLAN_MFGTEST_OOSRC_DIR}/nvram

B_WLAN_UTILS_RESULT_OOSRC_DIR?=${B_WLAN_OOSRC_DIR}/utils
B_WLAN_UTILS_RESULT_OOSRC_PATH_LIST:=$(patsubst %.c,%,$(wildcard ${B_WLAN_UTILS_RESULT_OOSRC_DIR}/*.c))
B_WLAN_UTILS_RESULT_FILE_NAME_LIST:=$(foreach f,${B_WLAN_UTILS_RESULT_OOSRC_PATH_LIST},$(notdir $f))

# And if we can find any NVRAM files, lets take care of that, too.
# They should get copied to the same directory as the WLAN driver.
# Here is the in-source directory that holds the NVRAM files.
B_WLAN_NVRAM_INSRC_DIR?=$(B_WLAN_INSRC_VER_DIR)/components/nvram

# Make a list of the various NVRAM files.
B_WLAN_NVRAM_INSRC_PATH_LIST:=$(wildcard $(B_WLAN_NVRAM_INSRC_DIR)/*)
ifeq ($(B_WLAN_NVRAM_INSRC_PATH_LIST),)
    $(info *** Message, cannot find NVRAM files in: "${B_WLAN_NVRAM_INSRC_DIR}".  Continuing anyway...)
endif
B_WLAN_NVRAM_RESULT_FILE_NAME_LIST:=$(foreach f,${B_WLAN_NVRAM_INSRC_PATH_LIST},$(notdir $f))

# Build a list of directories where we need to install those resulting files.
B_WLAN_INSTALL_DIR_LIST:=$(abspath $(BSEAV_BIN_DIR))
ifeq ($(shell test $(NEXUS_BIN_DIR) && test -d $(NEXUS_BIN_DIR) && echo y),y)
    B_WLAN_INSTALL_DIR_LIST += $(abspath $(NEXUS_BIN_DIR))
endif
ifeq ($(shell test $(INSTALL_DIR) && test -d $(INSTALL_DIR) && echo y),y)
    B_WLAN_INSTALL_DIR_LIST += $(abspath $(INSTALL_DIR))
endif

# Now combine the list of files with the list of directories to
# get a list of targets.
B_WLAN_DRIVER_INSTALL_PATH_LIST=$(foreach dir,${B_WLAN_INSTALL_DIR_LIST},${dir}/${B_WLAN_DRIVER_RESULT_FILE_NAME})
B_WLAN_DRIVER_INSTALL_PATH_LIST+=$(foreach dir,${B_WLAN_INSTALL_DIR_LIST},${dir}/${BUILDCFG}-wl.ko)
B_WLAN_WLPLAT_INSTALL_PATH_LIST=$(foreach dir,${B_WLAN_INSTALL_DIR_LIST},${dir}/${B_WLAN_DRIVER_RESULT_WLPLAT_FILE_NAME})
B_WLAN_WLEXE_INSTALL_PATH_LIST=$(foreach dir,${B_WLAN_INSTALL_DIR_LIST},${dir}/${B_WLAN_WLEXE_INSTALL_FILE_NAME})
B_WLAN_MFGTEST_INSTALL_PATH_LIST=$(foreach dir,${B_WLAN_INSTALL_DIR_LIST},$(foreach file,${B_WLAN_MFGTEST_RESULT_FILE_NAME_LIST},${dir}/${file}))
B_WLAN_IGS_INSTALL_PATH_LIST=$(foreach dir,${B_WLAN_INSTALL_DIR_LIST},${dir}/${B_WLAN_DRIVER_RESULT_IGS_FILE_NAME})
B_WLAN_EMF_INSTALL_PATH_LIST=$(foreach dir,${B_WLAN_INSTALL_DIR_LIST},${dir}/${B_WLAN_DRIVER_RESULT_EMF_FILE_NAME})
B_WLAN_NVRAM_INSTALL_PATH_LIST=$(foreach dir,${B_WLAN_INSTALL_DIR_LIST},$(foreach file,${B_WLAN_NVRAM_RESULT_FILE_NAME_LIST},${dir}/${file}))
B_WLAN_UTILS_INSTALL_PATH_LIST=$(foreach dir,${B_WLAN_INSTALL_DIR_LIST},$(foreach file,${B_WLAN_UTILS_RESULT_FILE_NAME_LIST},${dir}/${file}))

B_WLAN_BUILD_TARGET_MAYBE:=
ifneq (${B_WLAN_DRIVER_UPDATED_FILE_LIST},)
    B_WLAN_BUILD_TARGET_MAYBE:=build_wlan_all
endif

###################################################################
#  Wrappers for WLAN submakes.
###################################################################
#  Submake the epiver.
.PHONY: build_epiver
# STBAPD_INSTALL aka stbapd_install needs to be done before wlan driver can build. Needed for multiap.
ifeq ($(B_WLAN_STBAPD),1)
build_epiver: ${B_WLAN_EPIVER_OOSRC_FILE} stbapd_drv_build
else
build_epiver: ${B_WLAN_EPIVER_OOSRC_FILE}
endif

${B_WLAN_EPIVER_OOSRC_FILE}:
	@echo "[Submake... WLAN Driver (WLAN) Generate epiver.h]"
	@${MAKE} -C ${B_WLAN_EPIVER_OOSRC_DIR}

#  Submake the driver.
.PHONY: build_wldriver
build_wldriver: build_epiver
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Submake... WLAN Driver (WLAN) Target ${BUILDCFG}]"
	${Q_}${MAKE} -C ${B_WLAN_DRIVER_BUILD_OOSRC_DIR} JOBS= MAKEJOBS= ${BUILDCFG} FIRMWARE="${FIRMWARE}"  LINUXVER=${LINUXVER} ${BUILDARG}  ${B_WLAN_DRIVER_MAKE_REDIR}
	$(B_TRACE_RECIPE_END)

#  Submake the wl executable.
.PHONY: build_wlexe
build_wlexe: build_epiver
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Submake... $(notdir ${B_WLAN_WLEXE_RESULT_OOSRC_PATH}) (WLAN)]"
	${Q_}${MAKE} -C ${B_WLAN_WLEXE_RESULT_OOSRC_DIR} TARGETARCH=${TARGETARCH_FOR_WL} CC=$(B_REFSW_CROSS_COMPILE)gcc STRIP=${B_REFSW_CROSS_COMPILE}strip WLTEST=1 ${B_WLAN_WLEXE_MAKE_REDIR}
	$(B_TRACE_RECIPE_END)

#  Submake the mfgtest executable.
.PHONY: build_mfgtest_utils
build_mfgtest_utils:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Submake... $(notdir ${B_WLAN_MFGTEST_OOSRC_DIR}) (WLAN)]"
	${Q_}${MAKE} -C ${B_WLAN_MFGTEST_OOSRC_DIR} TARGETARCH=${TARGETARCH_FOR_WL} CC=$(B_REFSW_CROSS_COMPILE)gcc LD=$(B_REFSW_CROSS_COMPILE)ld STRIP=${B_REFSW_CROSS_COMPILE}strip ${B_WLAN_DRIVER_MAKE_REDIR}
	$(B_TRACE_RECIPE_END)

#  Submake the hostapd, wpa_supplicant, iw and their dependencies
.PHONY: build_hostapd_wpasupp_iw build_hostapd_wpasupp_iw_clean
build_hostapd_wpasupp_iw:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Submake... hostapd & wpa_supplicant (WLAN)]"
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/libnl install
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/openssl install
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/hostapd install
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/wpa_supplicant install
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/iw install
	$(B_TRACE_RECIPE_END)

build_hostapd_wpasupp_iw_clean:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Submake... hostapd & wpa_supplicant (WLAN)]"
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/libnl clean
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/openssl clean
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/hostapd clean
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/wpa_supplicant clean
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/iw clean
	$(B_TRACE_RECIPE_END)

# Submake for stbapd
.PHONY: stbapd_apps_build stbapd_drv_build
ifeq ($(BUILD_BRCM_HOSTAPD),y)
stbapd_apps_build: build_hostapd_wpasupp_iw
else
stbapd_apps_build:
endif
	$(B_TRACE_RECIPE_BEGIN)
	@echo "hostapd = $(BUILD_BRCM_HOSTAPD)"
	@if [ $(B_WLAN_STBAPD) = "0" ]; then \
		echo "Error:B_WLAN_STBAPD=$(B_WLAN_STBAPD), must export to 1 if building for stbapd"; \
		exit 1; \
	fi
	@echo "[Submake... stbapd version (WLAN)]"
	${Q_}${MAKE} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/include
	@echo "[Submake... ebtables (WLAN)]"
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/ebtables
	@echo "[Fetching... jsonc (WLAN)]"
	${Q_}${MAKE} -C ${B_WLAN_OOSRC_DIR}/../../../../BSEAV/opensource/jsonc
	@echo "[Submake... stbpad oldconfig (WLAN)]"
	cp ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/config/defconfig-stbap-dhd ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/.config
	yes "" | ${MAKE}  CROSS_COMPILE=${ARCHTOOL}- EXTRA_LDFLAGS=-lgcc_s LINUX_VERSION=${LINUXVER} PLT=${ARCH} STBLINUX=1 B_REFSW_OBJ_ROOT=${B_REFSW_OBJ_ROOT} BUILD_BRCM_HOSTAPD=${BUILD_BRCM_HOSTAPD} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/router oldconfig
	@echo "[Submake... stbpad apps apps_install (WLAN)]"
	# Must have a -j1 so prevent parallel build on stbapd router Makefile.
	${Q_}${MAKE} -j1 CROSS_COMPILE=${ARCHTOOL}- EXTRA_LDFLAGS=-lgcc_s LINUX_VERSION=${LINUXVER} PLT=${ARCH} STBLINUX=1 B_REFSW_OBJ_ROOT=${B_REFSW_OBJ_ROOT} BUILD_BRCM_HOSTAPD=${BUILD_BRCM_HOSTAPD} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/router apps apps_install
	$(B_TRACE_RECIPE_END)

stbapd_drv_build:
	$(B_TRACE_RECIPE_BEGIN)
	@if [ $(B_WLAN_STBAPD) = "0" ]; then \
		echo "Error:B_WLAN_STBAPD=$(B_WLAN_STBAPD), must export to 1 if building for stbapd"; \
		exit 1; \
	fi
	@echo "[Submake... Stbapd Driver (WLAN)]"
	${Q_}${MAKE} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/emf/emf SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}-

	${Q_}${MAKE} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/emf/igs SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}-

	${Q_}${MAKE} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/dpsta SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}-

	cat ${EMF_EXTRA_SYMBOLS} ${DPSTA_EXTRA_SYMBOLS} > ${KBUILD_EXTRA_SYMBOLS}
	$(B_TRACE_RECIPE_END)

stbapd_build_all: stbapd_drv_build stbapd_apps_build
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

#Submake for  BCA driver
.PHONY: ${B_WLAN_VER}_build
${B_WLAN_VER}_build: ${B_WLAN_STBAPD_INSTALL}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Submake... Unpackaging for ${B_WLAN_VER} ]"
	${Q_}${MAKE} -C ${WORKING_DIR}/${B_WLAN_VER}
	$(B_TRACE_RECIPE_END)

#  After submaking the driver and wl executable (via prerequisites), create a marker file.
#  Then as long as the marker file is newer than all source files, we don't need to do
#  any submakes.
.PHONY: build_wlan_all
build_wlan_all: build_wldriver build_wlexe
	$(B_TRACE_RECIPE_BEGIN)
	@echo "Creating WLAN marker file..."
	${Q_}$(shell date >${B_WLAN_LAST_BUILD_MARKER_FILE})
	$(B_TRACE_RECIPE_END)

###################################################################
#  WLAN nvram files
###################################################################
#  Define a template to create a static pattern rule for a specified install directory.
nvram_copy_template=\
$(foreach file,${B_WLAN_NVRAM_RESULT_FILE_NAME_LIST},$(1)/${file}) : $(1)/% : ${B_WLAN_NVRAM_INSRC_DIR}/%

#  Define the a static pattern rule for each install directory.
$(foreach result_dir,${B_WLAN_INSTALL_DIR_LIST},$(eval $(call nvram_copy_template,$(result_dir))))

#  Define the recipe for installing an nvram file.
${B_WLAN_NVRAM_INSTALL_PATH_LIST}:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... $(notdir $@) to $(dir $@) (WLAN)]"
	${Q_}${MKDIR} -p $(dir $@)
	${Q_}$(CP) -dfp  ${B_WLAN_NVRAM_INSRC_DIR}/$(notdir $@) $(dir $@)
	$(B_TRACE_RECIPE_END)

###################################################################
#  WLAN "utilities" including Prashant's "install" utility.
###################################################################
#  Define a template to create a static pattern rule for a specified install directory.
utils_copy_template=\
  $(foreach file,${B_WLAN_UTILS_RESULT_FILE_NAME_LIST},$(1)/${file}): $(1)/%: ${B_WLAN_UTILS_RESULT_OOSRC_DIR}/%

#  Define the a static pattern rule for each install directory.
$(foreach result_dir,${B_WLAN_INSTALL_DIR_LIST},$(eval $(call utils_copy_template,${result_dir})))

#  Build each utility from it's corresponding C source file in the same directory.
${B_WLAN_UTILS_RESULT_OOSRC_PATH_LIST}: % : %.c
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Compile... $(notdir $<) (WLAN)]"
	${Q_}${MKDIR} -p $(dir $@)
	${Q_}$(B_REFSW_CROSS_COMPILE)gcc -o $@ $<
	$(B_TRACE_RECIPE_END)

#  For a specified install directory, copy all utilities from the post-build location in the out-of-source tree.
${B_WLAN_UTILS_INSTALL_PATH_LIST}:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... $(notdir $@) to $(dir $@) (WLAN)]"
	${Q_}${MKDIR} -p $(dir $@)
	${Q_}$(CP) -dfp  ${B_WLAN_UTILS_RESULT_OOSRC_DIR}/$(notdir $@) $@
	$(B_TRACE_RECIPE_END)

###################################################################
#  WLAN wl executable
###################################################################
#  Wait for the wl executable to get built if any source files have changed.
${B_WLAN_WLEXE_RESULT_OOSRC_PATH}:  ${B_WLAN_BUILD_TARGET_MAYBE}
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

#  Each installed wl executable is copied from its post-build location in the out-of-source tree.
${B_WLAN_WLEXE_INSTALL_PATH_LIST}:  ${B_WLAN_WLEXE_RESULT_OOSRC_PATH}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... $(notdir ${B_WLAN_WLEXE_RESULT_OOSRC_PATH}) to $(dir $@) (WLAN)]"
	${Q_}$(MKDIR) -p $(dir $@)
	${Q_}$(CP) -dfp  ${B_WLAN_WLEXE_RESULT_OOSRC_PATH} $@
	$(B_TRACE_RECIPE_END)

###################################################################
#  WLAN mfgtest executable
###################################################################
#  Each installed binary is copied from its post-build location in the out-of-source tree.
	# add additional mfgtest files here
${B_WLAN_MFGTEST_INSTALL_PATH_LIST}: build_mfgtest_utils
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... $(notdir ${@}) to $(dir $@) (WLAN)]"
	${Q_}$(MKDIR) -p $(dir $@)
	${Q_}-$(CP) -dfp  ${B_WLAN_MFGTEST_BUILD_OOSRC_DIR}/$(notdir $@) $@
	$(B_TRACE_RECIPE_END)

###################################################################
#  WLAN driver
###################################################################
#  Wait for the driver to get built if any source files have changed.
${B_WLAN_DRIVER_RESULT_OOSRC_PATH}:  ${B_WLAN_BUILD_TARGET_MAYBE}
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

#  Wait for the driver to get built if any source files have changed.
${B_WLAN_DRIVER_RESULT_WLPLAT_OOSRC_PATH}:  ${B_WLAN_BUILD_TARGET_MAYBE}
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

#  Wait for the driver to get built if any source files have changed.
${B_WLAN_DRIVER_RESULT_IGS_OOSRC_PATH}:  ${B_WLAN_BUILD_TARGET_MAYBE}
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

#  Wait for the driver to get built if any source files have changed.
${B_WLAN_DRIVER_RESULT_EMF_OOSRC_PATH}:  ${B_WLAN_BUILD_TARGET_MAYBE}
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

#  Each installed driver is copied from its post-build location in the out-of-source tree.
${B_WLAN_DRIVER_INSTALL_PATH_LIST}: ${B_WLAN_DRIVER_RESULT_OOSRC_PATH}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... ${B_WLAN_DRIVER_RESULT_FILE_NAME} to $(dir $@) (WLAN)]"
	${Q_}$(MKDIR) -p $(dir $@)
	${Q_}$(CP) -dfp  ${B_WLAN_DRIVER_RESULT_OOSRC_PATH} $@
	$(B_TRACE_RECIPE_END)

#  Each installed driver is copied from its post-build location in the out-of-source tree.
${B_WLAN_WLPLAT_INSTALL_PATH_LIST}: ${B_WLAN_DRIVER_RESULT_WLPLAT_OOSRC_PATH}
	$(B_TRACE_RECIPE_BEGIN)
	echo "[Install... ${B_WLAN_DRIVER_RESULT_WLPLAT_FILE_NAME} to $(dir $@) (WLAN)]";
	$(MKDIR) -p $(dir $@);
	$(CP) -dfp  ${B_WLAN_DRIVER_RESULT_WLPLAT_OOSRC_PATH} $@;
	$(B_TRACE_RECIPE_END)

#  Each installed driver is copied from its post-build location in the out-of-source tree.
${B_WLAN_IGS_INSTALL_PATH_LIST}: ${B_WLAN_DRIVER_RESULT_IGS_OOSRC_PATH}
	$(B_TRACE_RECIPE_BEGIN)
	if [ -f ${B_WLAN_DRIVER_RESULT_IGS_OOSRC_PATH} ]; then \
		echo "[Install... ${B_WLAN_DRIVER_RESULT_IGS_FILE_NAME} to $(dir $@) (WLAN)]";\
		$(MKDIR) -p $(dir $@);\
		$(CP) -dfp  ${B_WLAN_DRIVER_RESULT_IGS_OOSRC_PATH} $@;\
	fi
	$(B_TRACE_RECIPE_END)

#  Each installed driver is copied from its post-build location in the out-of-source tree.
${B_WLAN_EMF_INSTALL_PATH_LIST}: ${B_WLAN_DRIVER_RESULT_EMF_OOSRC_PATH}
	$(B_TRACE_RECIPE_BEGIN)
	if [ -f ${B_WLAN_DRIVER_RESULT_EMF_OOSRC_PATH} ]; then \
		echo "[Install... ${B_WLAN_DRIVER_RESULT_EMF_FILE_NAME} to $(dir $@) (WLAN)]";\
		$(MKDIR) -p $(dir $@);\
		$(CP) -dfp  ${B_WLAN_DRIVER_RESULT_EMF_OOSRC_PATH} $@;\
	fi
	$(B_TRACE_RECIPE_END)

# Install for stbapd
.PHONY: stbapd_install
stbapd_tarball:
	$(B_TRACE_RECIPE_BEGIN)
	echo "[Tarball... Stbapd (WLAN)]";
	if [ -d ${B_WLAN_STBAPD_RESULT_OOSRC_DIR} ]; then \
		if [ $(B_REFSW_ARCH) = "aarch64-linux" ]; then \
			cd ${B_WLAN_STBAPD_INSTALL_PATH} && tar -jcvf target_bin.tar.bz2 etc root sbin usr www bin lib lib64; \
		else \
			cd ${B_WLAN_STBAPD_INSTALL_PATH} && tar -jcvf target_bin.tar.bz2 etc root sbin usr www bin lib; \
		fi \
	else \
		if [ $(B_REFSW_ARCH) = "aarch64-linux" ]; then \
			cd ${B_WLAN_STBAPD_INSTALL_PATH} && tar -jcvf target_bin.tar.bz2 lib lib64; \
		else \
			cd ${B_WLAN_STBAPD_INSTALL_PATH} && tar -jcvf target_bin.tar.bz2 lib; \
		fi \
	fi
	$(B_TRACE_RECIPE_END)

stbapd_install:
	$(B_TRACE_RECIPE_BEGIN)
	@if [ $(B_WLAN_STBAPD) = "0" ]; then \
		echo "Error:B_WLAN_STBAPD=$(B_WLAN_STBAPD), must export to 1 if building for stbapd"; \
		exit 1; \
	fi
	# Driver install
	$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/
	if [ -f ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/emf/emf/obj-emf-${LINUXVER}/emf.ko ]; then \
		echo "[Install... emf.ko  (WLAN)]";\
		$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/emf/emf/obj-emf-${LINUXVER}/emf.ko  ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/; \
	fi
	if [ -f ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/emf/igs/obj-igs-${LINUXVER}/igs.ko ]; then \
		echo "[Install... igs.ko  (WLAN)]";\
		$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/emf/igs/obj-igs-${LINUXVER}/igs.ko  ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/; \
	fi
	if [ -f ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/dpsta/obj-dpsta-${LINUXVER}/dpsta.ko ]; then \
		echo "[Install... dpsta.ko  (WLAN)]";\
		$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/dpsta/obj-dpsta-${LINUXVER}/dpsta.ko  ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/; \
	fi
	if [ -f ${B_WLAN_DRIVER_RESULT_OOSRC_PATH} ]; then \
		echo "[Install... wl.ko  (WLAN)]";\
		$(CP) ${B_WLAN_DRIVER_RESULT_OOSRC_PATH} ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/; \
	fi
	if [ -f ${B_WLAN_DRIVER_RESULT_WLPLAT_OOSRC_PATH} ]; then \
		echo "[Install... wlplat.ko  (WLAN)]";\
		$(CP) ${B_WLAN_DRIVER_RESULT_WLPLAT_OOSRC_PATH} ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/; \
	fi
	# add additional nvram files here
	$(CP) ${B_WLAN_NVRAM_INSRC_DIR}/bcm97271wlan.txt ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/
	$(CP) ${B_WLAN_NVRAM_INSRC_DIR}/bcm97271wlan.txt ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/nvram.txt
	$(CP) ${B_WLAN_NVRAM_INSRC_DIR}/bcm97271sdb_dr.txt ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/


	#Apps install
	if [ -d ${B_WLAN_STBAPD_RESULT_OOSRC_DIR} ]; then \
		echo "[Install... Stbapd  (WLAN)]";\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/etc;\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/root;\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/etc/init.d;\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/sbin;\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/usr/sbin;\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/www;\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/bin;\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/lib/modules/;\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/lib/firmware/brcm;\
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/lib64;\
		$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/src/shared/nvram/stbap_nvram.txt ${B_WLAN_STBAPD_INSTALL_PATH}/etc/nvrams_ap_default.txt;\
		if [[ "${NEXUS_PLATFORM}" != *7271* ]]; then \
			$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/rc/stbap_init.sh ${B_WLAN_STBAPD_INSTALL_PATH}/sbin/stbap_init.sh;\
		else \
			$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/rc/stbap_init_7271.sh ${B_WLAN_STBAPD_INSTALL_PATH}/sbin/stbap_init.sh; \
		fi; \
		chmod 777 ${B_WLAN_STBAPD_INSTALL_PATH}/sbin/stbap_init.sh;\
		$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/rc/rc.user ${B_WLAN_STBAPD_INSTALL_PATH}/root/;\
		chmod 777 ${B_WLAN_STBAPD_INSTALL_PATH}/root/rc.user;\
		$(CP) ${B_WLAN_STBAPD_RESULT_OOSRC_DIR}/sbin/* ${B_WLAN_STBAPD_INSTALL_PATH}/sbin/;\
		$(CP) ${B_WLAN_STBAPD_RESULT_OOSRC_DIR}/bin/* ${B_WLAN_STBAPD_INSTALL_PATH}/bin/;\
		$(CP) ${B_WLAN_STBAPD_RESULT_OOSRC_DIR}/usr/sbin/* ${B_WLAN_STBAPD_INSTALL_PATH}/usr/sbin;\
		$(CP) ${B_WLAN_STBAPD_RESULT_OOSRC_DIR}/usr/lib/* ${B_WLAN_STBAPD_INSTALL_PATH}/lib;\
		if [[ "${NEXUS_PLATFORM}" != *7271* ]]; then \
			$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/apps/* ${B_WLAN_STBAPD_INSTALL_PATH}/usr/sbin;\
			$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/lib/* ${B_WLAN_STBAPD_INSTALL_PATH}/lib;\
		fi; \
		$(CP) -r ${B_WLAN_STBAPD_RESULT_OOSRC_DIR}/www ${B_WLAN_STBAPD_INSTALL_PATH}/;\
		if [[ "${BUILD_BRCM_HOSTAPD}" == y ]]; then \
			find ${B_REFSW_OBJ_ROOT}/target/usr/local/bin -perm /a+x -name hostapd\* -exec cp -v -t ${B_WLAN_STBAPD_INSTALL_PATH}/bin {} +;\
			find ${B_REFSW_OBJ_ROOT}/target/usr/local/bin -perm /a+x -name wpa\* -exec cp -v -t ${B_WLAN_STBAPD_INSTALL_PATH}/bin {} +;\
			$(CP) ${B_REFSW_OBJ_ROOT}/target/usr/local/bin/*.conf ${B_WLAN_STBAPD_INSTALL_PATH}/etc;\
			$(CP) ${B_REFSW_OBJ_ROOT}/target/usr/local/bin/iw ${B_WLAN_STBAPD_INSTALL_PATH}/bin;\
			cd ${B_REFSW_OBJ_ROOT}/target/usr/local/lib && find . -name \*.so\* -exec cp -v -f --parents \{\} ${B_WLAN_STBAPD_INSTALL_PATH}/lib \; && cd -;\
		fi; \
		$(CP) ${B_WLAN_WLEXE_RESULT_OOSRC_PATH} ${B_WLAN_STBAPD_INSTALL_PATH}/usr/sbin/wl; \
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/prebuilt;\
		if [ -d ${B_WLAN_STBAPD_OOSRC_DIR}/components/apps/ieee1905/prebuilt ]; then \
			$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/components/apps/ieee1905/prebuilt/* ${B_WLAN_STBAPD_INSTALL_PATH}/prebuilt; \
		fi; \
		if [ -d ${B_WLAN_STBAPD_OOSRC_DIR}/components/apps/wbd/prebuilt ]; then \
			$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/components/apps/wbd/prebuilt/* ${B_WLAN_STBAPD_INSTALL_PATH}/prebuilt; \
		fi; \
		if [ -d ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/bsd/prebuilt ]; then \
			$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/bsd/prebuilt/* ${B_WLAN_STBAPD_INSTALL_PATH}/prebuilt; \
		fi; \
		if [ -d ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/bshared/prebuilt ]; then \
			$(CP) ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/bshared/prebuilt/* ${B_WLAN_STBAPD_INSTALL_PATH}/prebuilt; \
		fi; \
	fi

	@if [ $(B_REFSW_ARCH) = "aarch64-linux" ]; then \
		$(MKDIR) -p ${B_WLAN_STBAPD_INSTALL_PATH}/lib64/; \
		$(CP) -r ${B_WLAN_STBAPD_INSTALL_PATH}/lib/* ${B_WLAN_STBAPD_INSTALL_PATH}/lib64; \
	fi

	${Q_}${MAKE} -f $(WORKING_DIR)/Makefile stbapd_tarball BUILD_BRCM_HOSTAPD=$(BUILD_BRCM_HOSTAPD)

	$(B_TRACE_RECIPE_END)

.PHONY: ${B_WLAN_VER}_install
${B_WLAN_VER}_install: ${B_WLAN_VER}_build
	$(B_TRACE_RECIPE_BEGIN)
	@echo "Done. Unpacking ${B_WLAN_VER}"
	$(B_TRACE_RECIPE_END)
###################################################################
#  install target
###################################################################
#  The "install" target just lists all of the installed files as its prerequisites.
.PHONY: install
ifeq (${B_WLAN_VER},)
install:
	$(info *** Message, NO wifi chip designated for "${NEXUS_PLATFORM}". Check BSEAV/connectivity/wlan/wlan.inc for NEXUS_PLATFORM->B_WLAN_VER. Exit.)
else ifneq (${B_WLAN_VER},7271)
install: ${B_WLAN_EXTRA_INSTALL} ${B_WLAN_UTILS_INSTALL_PATH_LIST} ${B_WLAN_MFGTEST_INSTALL_PATH_LIST}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "Done. Non 7271"
	$(B_TRACE_RECIPE_END)
else
install: ${B_WLAN_DRIVER_INSTALL_PATH_LIST} ${B_WLAN_WLPLAT_INSTALL_PATH_LIST} ${B_WLAN_WLEXE_INSTALL_PATH_LIST} ${B_WLAN_UTILS_INSTALL_PATH_LIST} ${B_WLAN_NVRAM_INSTALL_PATH_LIST} ${B_WLAN_MFGTEST_INSTALL_PATH_LIST} ${B_WLAN_IGS_INSTALL_PATH_LIST} ${B_WLAN_EMF_INSTALL_PATH_LIST}
	$(B_TRACE_RECIPE_BEGIN)
	@if [ $(B_WLAN_STBAPD) = "1" ]; then \
		${MAKE} -f $(WORKING_DIR)/Makefile stbapd_install BUILD_BRCM_HOSTAPD=$(BUILD_BRCM_HOSTAPD); \
        fi
	@echo "Done. 7271"
	$(B_TRACE_RECIPE_END)
endif
# The "driver" intended for Coverity just builds driver related files
driver: build_wldriver
	$(B_TRACE_RECIPE_BEGIN)
	@echo "Done."
	$(B_TRACE_RECIPE_END)

# The "utils" intended for Coverity just builds app/util related files
utils:  build_wlexe  ${B_WLAN_UTILS_INSTALL_PATH_LIST}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "Done."
	$(B_TRACE_RECIPE_END)
###################################################################
#  clean target
###################################################################
stbapd_apps_clean:
	$(B_TRACE_RECIPE_BEGIN)
	${Q_}${MAKE} -i CROSS_COMPILE=${ARCHTOOL}- EXTRA_LDFLAGS=-lgcc_s LINUX_VERSION=${LINUXVER} PLT=${ARCH} STBLINUX=1 B_REFSW_OBJ_ROOT=${B_REFSW_OBJ_ROOT} BUILD_BRCM_HOSTAPD=${BUILD_BRCM_HOSTAPD} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/router clean apps_clean
	#force clean all stbapd output files
	$(Q_)$(RM) -rf ${B_WLAN_STBAPD_INSTALL_PATH}
	$(B_TRACE_RECIPE_END)

stbapd_drv_clean:
	$(B_TRACE_RECIPE_BEGIN)
	${Q_}${MAKE} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/emf/emf SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- clean
	${Q_}${MAKE} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/emf/igs SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- clean
	${Q_}${MAKE} -C ${B_WLAN_STBAPD_OOSRC_DIR}/src/router/dpsta SHOWWLCONF=1 LINUXVER=${LINUXVER} LINUXDIR=${LINUX} CROSS_COMPILE=${ARCHTOOL}- clean
	#force clean all stbapd output files
	$(Q_)$(RM) -rf ${B_WLAN_STBAPD_INSTALL_PATH}
	$(B_TRACE_RECIPE_END)

ifeq ($(BUILD_BRCM_HOSTAPD),y)
stbapd_clean_all: build_hostapd_wpasupp_iw_clean stbapd_drv_clean stbapd_apps_clean
else
stbapd_clean_all: stbapd_drv_clean stbapd_apps_clean
endif
	$(B_TRACE_RECIPE_BEGIN)
	@echo "Cleaning stbapd..."
	@echo "hostapd = $(BUILD_BRCM_HOSTAPD)"
	@if [ $(B_WLAN_STBAPD) = "0" ]; then \
		echo "Error:B_WLAN_STBAPD=$(B_WLAN_STBAPD), must export to 1 if building for stbapd"; \
		exit 1; \
	fi
	$(Q_)$(MAKE) -C ${B_WLAN_OOSRC_DIR}/../../opensource/ebtables clean
	$(Q_)$(MAKE) -C ${B_JSONC_SOURCE_DEST} clean
	$(Q_)$(RM) -rf ${B_WLAN_OOSRC_DIR}/stbapd
	$(Q_)$(RM) -rf ${BSEAV_BIN_DIR}/stbapd/*
	$(B_TRACE_RECIPE_END)

${B_WLAN_VER}_clean:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "Cleaning ${B_WLAN_VER} package..."
	$(Q_)$(MAKE) -C ${WORKING_DIR}/${B_WLAN_VER} clean
	$(B_TRACE_RECIPE_END)

.PHONY: clean
ifeq (${B_WLAN_VER},)
clean:
	$(info *** Message, NO wifi chip designated for "${NEXUS_PLATFORM}". Check BSEAV/connectivity/wlan/wlan.inc for NEXUS_PLATFORM->B_WLAN_VER. Exit.)
else
clean: ${B_WLAN_EXTRA_CLEAN} ${B_WLAN_STBAPD_CLEAN}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "Cleaning..."
	$(Q_)$(RM) -rf ${B_WLAN_OOSRC_VER_DIR}
	$(Q_)$(RM) -f ${B_WLAN_DRIVER_INSTALL_PATH_LIST} ${B_WLAN_WLPLAT_INSTALL_PATH_LIST} ${B_WLAN_WLEXE_INSTALL_PATH_LIST} ${B_WLAN_MFGTEST_INSTALL_PATH_LIST} ${B_WLAN_NVRAM_INSTALL_PATH_LIST} ${B_WLAN_UTILS_INSTALL_PATH_LIST} ${B_WLAN_IGS_INSTALL_PATH_LIST} ${B_WLAN_EMF_INSTALL_PATH_LIST}
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/connectivity/wlan/wakeonwlan clean
	$(Q_)$(MAKE) -C $(B_WLAN_MFGTEST_OOSRC_DIR) clean
	$(B_TRACE_RECIPE_END)
endif

#  Print out a list of make variables for debugging.
ifeq (${B_WLAN_MAKEFILE_ENABLE_DEBUG},y)
    $(foreach i,                        \
    B_REFSW_ARCH                        \
    B_REFSW_CROSS_COMPILE               \
    B_REFSW_TOOLCHAIN_DIR               \
    NEXUS_ENDIAN                        \
    ARCH                                \
    LINUX                               \
    LINUXDIR                            \
    LINUXVER                            \
    TARGETDIR                           \
    USBSHIM                             \
    TARGETARCH                          \
    TARGETMACH                          \
    TARGETENV                           \
    CROSS_COMPILE                       \
    ARCH                                \
    STBLINUX                            \
    MAKEFLAGS                           \
    MAKEOVERRIDES                       \
    NEXUS_BIN_DIR                       \
    B_WLAN_INSTALL_DIR_LIST             \
    B_WLAN_DRIVER_RESULT_FILE_NAME      \
    B_WLAN_DRIVER_INSTALL_PATH_LIST     \
    B_WLAN_DRIVER_INSTALL_BUILDCFG_PATH_LIST     \
    B_WLAN_DRIVER_UPDATED_FILE_LIST     \
    B_WLAN_DRIVER_RESULT_OOSRC_PATH     \
    B_WLAN_WLEXE_RESULT_FILE_NAME       \
    B_WLAN_WLEXE_INSTALL_PATH_LIST      \
    B_WLAN_WLEXE_RESULT_OOSRC_PATH      \
    B_WLAN_MFGTEST_RESULT_FILE_NAME_LIST        \
    B_WLAN_MFGTEST_INSTALL_PATH_LIST	\
    B_WLAN_NVRAM_INSRC_PATH_LIST        \
    B_WLAN_NVRAM_RESULT_FILE_NAME_LIST  \
    B_WLAN_NVRAM_INSRC_DIR              \
    B_WLAN_NVRAM_INSTALL_PATH_LIST      \
    B_WLAN_BUILD_TARGET_MAYBE           \
    B_WLAN_LAST_BUILD_MARKER_FILE       \
    B_WLAN_UTILS_RESULT_FILE_NAME_LIST  \
    B_WLAN_UTILS_RESULT_OOSRC_DIR       \
    B_WLAN_UTILS_RESULT_OOSRC_PATH_LIST \
    B_WLAN_UTILS_INSTALL_PATH_LIST      \
    ,                                   \
        $(info )                        \
        $(info $i: **************)      \
        $(foreach j,$($(i)),            \
            $(info $(j))                \
         )                              \
    )$(info )
endif
