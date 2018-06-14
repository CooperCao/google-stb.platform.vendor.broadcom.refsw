###############################################################################
#   Copyright (C) 2018 Broadcom.
#   The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to
#   the terms and conditions of a separate, written license agreement executed
#   between you and Broadcom (an "Authorized License").  Except as set forth in
#   an Authorized License, Broadcom grants no license (express or implied),
#   right to use, or waiver of any kind with respect to the Software, and
#   Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#   THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#   IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization,
#   constitutes the valuable trade secrets of Broadcom, and you shall use all
#   reasonable efforts to protect the confidentiality thereof, and to use this
#   information only in connection with your use of Broadcom integrated circuit
#   products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#   OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#   RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#   IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#   A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#   THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#   OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#   INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#   RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#   EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#   WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#   FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################
SHELL:=/bin/bash
PWD := ${shell pwd}
OPENSOURCE ?= ${PWD}/../
THIS_DIR := BSEAV/opensource/websocketd
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include ${WORKING_DIR}/websocketd.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_WEBSOCKETD_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)

all: source

.PHONY: all clean

B_WEBSOCKETD_EXTRACT_REDIR :=

ifeq ($(B_REFSW_WEBSOCKETD_IS_EXTERNAL),y)
    $(warning B_REFSW_WEBSOCKETD_IS_EXTERNAL=${B_REFSW_WEBSOCKETD_IS_EXTERNAL} Skipping build of websocketd ...)
endif   #  ifeq ($(B_REFSW_WEBSOCKETD_IS_EXTERNAL),y)

source:
ifneq ($(B_REFSW_WEBSOCKETD_IS_EXTERNAL),y)
	@if [ ! -d "${WEBSOCKETD_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "websocketd source directory: " $(abspath ${WEBSOCKETD_SOURCE_PATH}) ; \
		echo "not found; extracting from zipfile..."  ${B_WEBSOCKETD_EXTRACT_REDIR}; \
		echo ""; \
		echo "WEBSOCKETD_SOURCE_DIRECTORY is ${WEBSOCKETD_SOURCE_DIRECTORY}"; \
		echo "looking for websocketd zipfile here: ${WEBSOCKETD_TAR_PATH}/${WEBSOCKETD_SOURCE_TARBALL}"; \
		if [ ! -e "${WEBSOCKETD_TAR_PATH}/${WEBSOCKETD_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading WEBSOCKETD $(WEBSOCKETD_VER) tarball to ${WEBSOCKETD_TAR_PATH}"; \
			mkdir -p ${WEBSOCKETD_TAR_PATH}; \
			echo "${WEBSOCKETD_BUILD_DIR}/${WEBSOCKETD_NAME}"; \
			echo "${WEBSOCKETD_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${WEBSOCKETD_BUILD_DIR}/${WEBSOCKETD_NAME}.info ${WEBSOCKETD_TAR_PATH}; \
		fi ;\
		if [ -e "${WEBSOCKETD_TAR_PATH}/${WEBSOCKETD_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting ${WEBSOCKETD_TAR_PATH}/${WEBSOCKETD_SOURCE_TARBALL}  WEBSOCKETD $(WEBSOCKETD_VER) tarball"; \
			(cd ${WEBSOCKETD_TAR_PATH} && unzip -o ${WEBSOCKETD_TAR_PATH}/${WEBSOCKETD_SOURCE_TARBALL} >/dev/null ) ; \
		else \
			echo "Zipfile ${WEBSOCKETD_TAR_PATH}/${WEBSOCKETD_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_WEBSOCKETD_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_WEBSOCKETD_IS_EXTERNAL),y)

# Determine if we are compiling for 64-bit machine
COMPILER := ${B_REFSW_ARCH}-gcc
ARCHIVER := ${B_REFSW_ARCH}-ar
BIT64 = $(findstring aarch64,${COMPILER})

all:source
	@echo "================ TARGET all: WEBSOCKETD $(WEBSOCKETD_VER) ============================="
	@echo "================ Installing WEBSOCKETD $(WEBSOCKETD_VER) to nexus/bin ================="
	@mkdir -p ${NEXUS_BIN_DIR}
	@cp $(WEBSOCKETD_LIB_FOLDER)/websocketd ${NEXUS_BIN_DIR}

clean:
	@echo "================ MAKE CLEAN =========================================="
	@rm -rf ${WEBSOCKETD_TAR_PATH}
	@rm -rf ${NEXUS_BIN_DIR}/websocketd* ${NEXUS_BIN_DIR}/websocketd*
