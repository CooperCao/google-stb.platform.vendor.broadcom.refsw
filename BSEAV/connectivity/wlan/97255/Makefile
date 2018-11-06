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

THIS_DIR := BSEAV/connectivity/wlan/97255
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus

include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc

WIFI_BCA_TAR_NAME := bca_wifi.tar.gz

B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV_TOP}/../${B_REFSW_OBJ_DIR}

WIFI_BCA_TAR_LOCATION=/project_it/stbdevrefsw/binaries/files/connectivity/wlan/7255/

K_VERSION 	:= $(shell grep "^VERSION" $(LINUX)/Makefile | cut -d " " -f3)
K_PATCHLEVEL 	:= $(shell grep "^PATCHLEVEL" $(LINUX)/Makefile | cut -d " " -f3)
K_SUBLEVEL 	:= $(shell grep "^SUBLEVEL" $(LINUX)/Makefile | cut -d " " -f3)
K_EXTRAVERSION	:= $(shell grep "^EXTRAVERSION" $(LINUX)/Makefile | cut -d- -f2-)
KERNEL_VERSION	:= $(K_VERSION).$(K_PATCHLEVEL).$(K_SUBLEVEL)-$(K_EXTRAVERSION)

ifeq ($(B_REFSW_ARCH),aarch64-linux)
	WIFI_DRV_OS := arm64-${KERNEL_VERSION}
	export drv_os=${WIFI_DRV_OS}
	WIFI_DRV_OS_SCRIPT :=  stbsta_build_dhd_full_src_64bit.sh
else ifeq ($(B_REFSW_ARCH),arm-linux)
	WIFI_DRV_OS := arml-${KERNEL_VERSION}
	export drv_os=${WIFI_DRV_OS}
	WIFI_DRV_OS_SCRIPT := stbsta_build_dhd_full_src_32bit.sh
else
	$(warning B_REFSW_ARCH not set)
endif

.PHONY: all clean

source:
	@echo "================ MAKE CLEAN remove tar files =========================================="
	@rm -rf ${B_REFSW_OBJ_ROOT}/BSEAV/connectivity/wlan/7255
ifeq (${NEXUS_PLATFORM},97255)
	@echo "================ MAKE ${NEXUS_PLATFORM}   ${B_REFSW_ARCH}         ## ${KERNEL_VERSION} ### =========================================="
	${Q_}mkdir -p ${B_REFSW_OBJ_ROOT}/BSEAV/connectivity/wlan/7255
	@cp -af ${WIFI_BCA_TAR_LOCATION}/${WIFI_BCA_TAR_NAME} ${B_REFSW_OBJ_ROOT}/BSEAV/connectivity/wlan/7255
	tar -C ${B_REFSW_OBJ_ROOT}/BSEAV/connectivity/wlan/7255 -xvzf ${B_REFSW_OBJ_ROOT}/BSEAV/connectivity/wlan/7255/${WIFI_BCA_TAR_NAME}
	cd ${B_REFSW_OBJ_ROOT}/BSEAV/connectivity/wlan/7255/scripts && export drv_os=${WIFI_DRV_OS} && export DEFAULT_CHIP=43570A2 && ./${WIFI_DRV_OS_SCRIPT}
endif	
all: source

clean:
	@echo "================ MAKE CLEAN remove tar files =========================================="
	@rm -rf ${B_REFSW_OBJ_ROOT}/BSEAV/connectivity/wlan/7255
