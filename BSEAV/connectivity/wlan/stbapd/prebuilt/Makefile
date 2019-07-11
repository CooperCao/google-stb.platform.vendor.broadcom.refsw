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

THIS_DIR := BSEAV/connectivity/wlan/stbapd/prebuilt
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# Prebuilt binary tag.
VERSION_FILE := ${BSEAV_TOP}/connectivity/wlan/7271/src/include/epivers.sh
CUR_TAG := $(shell grep EXTERNAL_TAG= ${VERSION_FILE} | cut -d'"' -f2)
ifneq ($(NEXUS_PLATFORM),)
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc

#.DEFAULT_GOAL := all 

B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV_TOP}/../${B_REFSW_OBJ_DIR}
BSEAV_BIN_DIR ?=${B_REFSW_OBJ_ROOT}/BSEAV/bin


ifeq ($(B_REFSW_ARCH),aarch64-linux)
	STBAPD_PREBUILT_ARCH_TAR_NAME := prebuilt_aarch64_${CUR_TAG}.tgz
else ifeq ($(B_REFSW_ARCH),arm-linux)
	STBAPD_PREBUILT_ARCH_TAR_NAME := prebuilt_armv7l_${CUR_TAG}.tgz
else
	$(warning B_REFSW_ARCH not set)
endif
endif
STBAPD_PREBUILT_LOCATION=${BSEAV_TOP}/../${THIS_DIR}/${STBAPD_PREBUILT_ARCH_TAR_NAME}

STBAPD_FETCH_MAYBE := ${STBAPD_PREBUILT_LOCATION}
ifneq ("$(wildcard $(STBAPD_PREBUILT_LOCATION))","")
	STBAPD_FETCH_MAYBE :=
endif

.PHONY: all clean install
all: install

release:
	echo "fetching tarball: CUR_TAG=${CUR_TAG} VERSION_FILE=${VERSION_FILE} "
#	@bash ${B_REFSW_TOP}/BSEAV/tools/build/binaries/verify_binaries.sh stbapd_prebuilt.txt stbapd_prebuilt.tar.gz ${BSEAV_TOP}/../${THIS_DIR}
	wget http://git-irv-03.irv.broadcom.com:88/files/connectivity/wlan/stbapd_prebuilt/prebuilt_aarch64_${CUR_TAG}.tgz
	wget http://git-irv-03.irv.broadcom.com:88/files/connectivity/wlan/stbapd_prebuilt/prebuilt_armv7l_${CUR_TAG}.tgz

${STBAPD_FETCH_MAYBE}: 
	echo "fetching tarball: CUR_TAG=${CUR_TAG} VERSION_FILE=${VERSION_FILE} "
#	@bash ${B_REFSW_TOP}/BSEAV/tools/build/binaries/verify_binaries.sh stbapd_prebuilt.txt stbapd_prebuilt.tar.gz ${BSEAV_TOP}/../${THIS_DIR}
	wget http://git-irv-03.irv.broadcom.com:88/files/connectivity/wlan/stbapd_prebuilt/prebuilt_aarch64_${CUR_TAG}.tgz
	wget http://git-irv-03.irv.broadcom.com:88/files/connectivity/wlan/stbapd_prebuilt/prebuilt_armv7l_${CUR_TAG}.tgz

install: ${STBAPD_FETCH_MAYBE}
	@if [ "$(NEXUS_PLATFORM)" = "" ]; then \
		echo "Error:NEXUS_PLATFORM is not defined!"; \
		exit 1; \
	fi
	${Q_}mkdir -p ${BSEAV_BIN_DIR}/stbapd
	@echo "================ Untar ARCH based STBAPD prebuilt tarball =========================================="
	tar -C ${BSEAV_BIN_DIR}/stbapd -xvzf ${STBAPD_PREBUILT_LOCATION}

clean:
	@echo "================ MAKE CLEAN remove STBAPD tar files =========================================="
	@rm -rf ${BSEAV_BIN_DIR}/stbapd/*tgz
	@rm -rf ${BSEAV_TOP}/../${THIS_DIR}/*tgz
