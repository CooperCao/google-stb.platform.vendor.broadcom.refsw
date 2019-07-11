############################################################
#  Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#
############################################################
SHELL := /bin/bash

THIS_DIR := BSEAV/connectivity/wlan/4375
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus

ifneq ($(NEXUS_PLATFORM),)
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
endif
###############################################
#Adjust these parameters. DEBUG=y will add -debug to end ${4375_DEFAULT_BUILDCFG}
#####################################################
4375_DEBUG=y
4375_MFGTEST=n
4375_DEFAULT_BUILDCFG?=dhd-msgbuf-pciefd-cfg80211-stb
##########################################################

WIFI_4375_TAR_NAME := stb_4375_wifi.tar.gz
WIFI_4375_LOCAL_TAR_LOCATION=${BSEAV_TOP}/../${THIS_DIR}/${WIFI_4375_TAR_NAME}
ifneq ($(NEXUS_PLATFORM),)
B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV_TOP}/../${B_REFSW_OBJ_DIR}
BSEAV_BIN_DIR?=${B_REFSW_OBJ_ROOT}/BSEAV/bin
WIFI_4375_TAR_LOCATION=/project_it/stbdevrefsw/binaries/files/connectivity/wlan/4375/
4375_OOSRC?=${B_REFSW_OBJ_ROOT}/BSEAV/connectivity/wlan/4375

K_VERSION	:= $(shell grep "^VERSION" $(LINUX)/Makefile | cut -d " " -f3)
K_PATCHLEVEL	:= $(shell grep "^PATCHLEVEL" $(LINUX)/Makefile | cut -d " " -f3)
K_SUBLEVEL	:= $(shell grep "^SUBLEVEL" $(LINUX)/Makefile | cut -d " " -f3)
K_EXTRAVERSION	:= $(shell grep "^EXTRAVERSION" $(LINUX)/Makefile | cut -d- -f2-)
KERNEL_VERSION	:= $(K_VERSION).$(K_PATCHLEVEL).$(K_SUBLEVEL)-$(K_EXTRAVERSION)

# Handle 32-bit ARM (B_REFSW_ARCH==arm*)
ifneq ($(filter arm%,${B_REFSW_ARCH}),)
    ifeq (${NEXUS_ENDIAN},BSTD_ENDIAN_LITTLE)
        # ARM LE
        #Added for stbapd
        #export ARCHTOOL=arm-linux
        #export WLARCH=arm
        # end of for stbapd
        export ARCH=arm
        export TARGETARCH?=arm
        #       TARGETARCH_FOR_WL=armv7l
        export TARGETMACH?=arm
        export TARGETENV?=linuxarm
	ifeq (${4375_DEBUG},y)
		export BUILDCFG=${4375_DEFAULT_BUILDCFG}-armv7l-debug
	else
		export BUILDCFG=${4375_DEFAULT_BUILDCFG}-armv7l
	endif
	#location of wl and dhd apps
	APPS_DIR=${4375_OOSRC}/apps/armv7l
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
        #export ARCHTOOL=aarch64-linux
        #export WLARCH=arm64
        # end of for stbapd
        export ARCH=arm64
        export TARGETARCH?=arm64
        #       TARGETARCH_FOR_WL=arm64
        export TARGETMACH?=arm
        export TARGETENV?=linuxarm
        export BUILDCFG=${4375_DEFAULT_BUILDCFG}-armv8
	ifeq (${4375_DEBUG},y)
		export BUILDCFG=${4375_DEFAULT_BUILDCFG}-armv8-debug
	else
		export BUILDCFG=${4375_DEFAULT_BUILDCFG}-armv8
	endif
	#location of wl and dhd apps
	APPS_DIR=${4375_OOSRC}/apps/aarch64
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
export CROSS_COMPILE?=${B_REFSW_CROSS_COMPILE}

#location of clm, fw, and dhd.ko
ifeq (${4375_MFGTEST},y)
    # Note: clmblob built in to mfgtest fw. No need to do download clm_blob for mfgtest
    #CLM_DIR=${4375_OOSRC}/clm_blobs/dongle/4375b1-roml/config_pcie_stb_mfgtest
    CLM_DIR=${4375_OOSRC}/clm_blobs/dongle/4375b1-roml/dummy_nonexistant
    FW_BINARIES=${4375_OOSRC}/firmware/4375b1-roml/config_pcie_stb_mfgtest.*
else
    # Release
    CLM_DIR=${4375_OOSRC}/clm_blobs/dongle/4375b1-roml/config_pcie_stb_release
    FW_BINARIES=${4375_OOSRC}/firmware/4375b1-roml/config_pcie_stb_release.*
endif
DHD_KO=${4375_OOSRC}/dhd/src/dhd/linux/${BUILDCFG}-${LINUXVER}/dhd.ko
endif

CUR_TARBALL_SHASUM:=$(shell sha1sum ${WIFI_4375_TAR_NAME})
RELEASE_TARBALL_SHASUM:=$(shell awk 'END{print}' wifi_4375.txt | awk -F ' ' '{print $$3}')

# Check if wifi tarball has been fetched
WIFI_4375_TAR_FETCH_MAYBE := ${WIFI_4375_LOCAL_TAR_LOCATION}
ifneq ("$(wildcard $(WIFI_4375_LOCAL_TAR_LOCATION))","")
ifeq ($(CUR_TARBALL_SHASUM),$(RELEASE_TARBALL_SHASUM))
	WIFI_4375_TAR_FETCH_MAYBE :=
endif
endif

.PHONY: all clean

all: install
release:
	@echo "================FETCHING 4375 wifi tarball  =========================================="
	@bash ${B_REFSW_TOP}/BSEAV/tools/build/binaries/verify_binaries.sh wifi_4375.txt stb_4375_wifi.tar.gz ${BSEAV_TOP}/../${THIS_DIR}


${WIFI_4375_TAR_FETCH_MAYBE}:
	@echo "================FETCHING 4375 wifi tarball  =========================================="
	@echo "================CUR_TARBALL_SHASUM=${CUR_TARBALL_SHASUM} != RELEASE_TARBALL_SHASUM=${RELEASE_TARBALL_SHASUM}"
	@bash ${B_REFSW_TOP}/BSEAV/tools/build/binaries/verify_binaries.sh wifi_4375.txt stb_4375_wifi.tar.gz ${BSEAV_TOP}/../${THIS_DIR}

build_dhd: ${WIFI_4375_TAR_FETCH_MAYBE}
	@if [ "$(NEXUS_PLATFORM)" = "" ]; then \
		echo "Error:NEXUS_PLATFORM is not defined!"; \
		exit 1; \
	fi
	@if [ ! -d "${4375_OOSRC}/dhd" ]; then \
		echo "================ Untar 4375 wifi tarball to ${4375_OOSRC}=========================================="; \
		mkdir -p ${4375_OOSRC}; \
		tar -xvzf ${WIFI_4375_LOCAL_TAR_LOCATION} -C ${4375_OOSRC}; \
	fi
	@echo "================ build dhd driver ${WIFI_4375_TAR_FETCH_MAYBE}=========================================="
	${Q_}${MAKE} -C ${4375_OOSRC}/dhd/src/include clean
	${Q_}${MAKE} -C ${4375_OOSRC}/dhd/src/include
#${Q_}${MAKE} -C ${B_WLAN_DRIVER_BUILD_OOSRC_DIR} JOBS= MAKEJOBS= ${BUILDCFG} FIRMWARE="${FIRMWARE}"  LINUXVER=${LINUXVER} ${BUILDARG}  ${B_WLAN_DRIVER_MAKE_REDIR}
	${Q_}${MAKE} -C ${4375_OOSRC}/dhd/src/dhd/linux ${BUILDCFG} WLTEST=${WLTEST} WLLXIW=${WLLXIW} LINUXVER=${LINUXVER} ${BUILDARG} CC=${CC} STRIP=${STRIP} 2>&1 | tee ${4375_OOSRC}/build-${BUILDCFG}.log

install: build_dhd
	${Q_}mkdir -p ${BSEAV_BIN_DIR}
	@echo "================ copy nvram=========================================="
	cp ${4375_OOSRC}/nvrams/* ${BSEAV_BIN_DIR}
	@echo "================ copy stb_release_clm=========================================="
	if [ -d ${CLM_DIR} ]; then \
		cp ${CLM_DIR}/* ${BSEAV_BIN_DIR}; \
		cp ${CLM_DIR}/* ${NEXUS_BIN_DIR}; \
	fi;
	@echo "================ copy fw=========================================="
	cp ${FW_BINARIES} ${BSEAV_BIN_DIR}
	@echo "================ copy driver=========================================="
	cp ${DHD_KO} ${BSEAV_BIN_DIR}
	cp ${DHD_KO} ${BSEAV_BIN_DIR}/${BUILDCFG}-${LINUXVER}.ko
	@echo "================ copy apps=========================================="
	cp ${APPS_DIR}/dhd ${BSEAV_BIN_DIR}
	cp ${APPS_DIR}/wl ${BSEAV_BIN_DIR}

clean:
	@echo "================ MAKE CLEAN remove tar files =========================================="
	@rm -rf ${4375_OOSRC}

dist-clean: clean
	@echo "================ MAKE DIST-CLEAN remove local tar files =========================================="
	@rm -rf ${WIFI_4375_LOCAL_TAR_LOCATION}

