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

SHELL:=/bin/bash
PWD := ${shell pwd}
OPENSOURCE ?= ${PWD}/../
THIS_DIR := BSEAV/lib/iperf
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include iperf.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_IPERF_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)
IPERF_CONFIG_CFLAGS  := -O2 -fPIC -DIPERF_MAX_WRITE_SIZE=65536

.PHONY: source all clean

B_IPERF_EXTRACT_REDIR :=

ifeq ($(B_REFSW_IPERF_IS_EXTERNAL),y)
    $(warning B_REFSW_IPERF_IS_EXTERNAL=${B_REFSW_IPERF_IS_EXTERNAL} Skipping build of iperf...)
endif   #  ifeq ($(B_REFSW_IPERF_IS_EXTERNAL),y)

source:
ifneq ($(B_REFSW_IPERF_IS_EXTERNAL),y)
	@if [ ! -d "${IPERF_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "iperf source directory: " $(abspath ${IPERF_SOURCE_PATH}) ; \
		echo "not found; extracting from tarball..."  ${B_IPERF_EXTRACT_REDIR}; \
		echo ""; \
		echo "IPERF_SOURCE_DIRECTORY is ${IPERF_SOURCE_DIRECTORY}"; \
		if [ ! -e "${IPERF_TAR_PATH}/${IPERF_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading IPERF $(IPERF_VER) tarball  to ${IPERF_TAR_PATH}"; \
			mkdir -p ${IPERF_TAR_PATH}; \
			echo "${IPERF_BUILD_DIR}/${IPERF_NAME}"; \
			echo "${IPERF_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${IPERF_BUILD_DIR}/${IPERF_NAME}.info ${IPERF_TAR_PATH}; \
		fi ;\
		if [ -e "${IPERF_TAR_PATH}/${IPERF_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting IPERF $(IPERF_VER) tarball"; \
			echo "${IPERF_SOURCE_PATH}"; \
			mkdir -p ${IPERF_SOURCE_PATH}; \
			echo "IPERF_TAR_PATH is ${IPERF_TAR_PATH}"; \
			echo "IPERF_SOURCE_TARBALL is ${IPERF_SOURCE_TARBALL}"; \
			tar -xzf ${IPERF_TAR_PATH}/${IPERF_SOURCE_TARBALL} -C ${IPERF_SOURCE_PATH}/..; \
		else \
			echo "Tarball ${IPERF_TAR_PATH}/${IPERF_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_IPERF_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_IPERF_IS_EXTERNAL),y)

# Determine if we are compiling for 64-bit machine
COMPILER := ${B_REFSW_ARCH}-gcc
BIT64 = $(findstring aarch64,${COMPILER})

all : source
	@if [ ! -d "${IPERF_SOURCE_PATH}" ]; then \
		echo "iperf source directory: $(abspath ${IPERF_SOURCE_PATH}) not found, extracting from tarball..."  ${B_IPERF_EXTRACT_REDIR}; \
		if [ ! -e "${IPERF_TAR_PATH}/${IPERF_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading IPERF $(IPERF_VER) tarball  to ${IPERF_TAR_PATH}"; \
			mkdir -p ${IPERF_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${IPERF_BUILD_DIR}/${IPERF_NAME}.info ${IPERF_TAR_PATH}; \
		fi ;\
	fi
	@echo "================ Configuring IPERF $(IPERF_VER) ============================="

	@if [ ! -e "$(IPERF_EXE_FOLDER)/../patch_complete" ]; then \
		echo [Patching IPERF $(IPERF_VER) configuration files]; \
		(cd $(IPERF_EXE_FOLDER)/.. && patch -p1 < $(WORKING_DIR)/iperf-$(IPERF_VER).patch && date > patch_complete ); \
	else \
		echo "================ Patch $(IPERF_VER) already completed"; \
	fi

	@if [ ! -e "${IPERF_TAR_PATH}/${IPERF_NAME}/Makefile" ]; then \
		echo "================ Configure IPERF $(IPERF_VER)"; \
		cd ${IPERF_TAR_PATH}/${IPERF_NAME}; \
		CFLAGS="${IPERF_CONFIG_CFLAGS}" \
		LDFLAGS="${IPERF_CONFIG_LDFLAGS}" \
		./configure --host=${B_IPERF_TOOLCHAIN_ARCH} >/dev/null 2>&1 ; \
		echo "================ Configuring IPERF $(IPERF_VER) complete ===================="; \
	else \
		echo "================ IPERF $(IPERF_VER) already configured"; \
	fi

	@if [ ! -e "${IPERF_TAR_PATH}/${IPERF_NAME}/Makefile" ]; then \
		echo "${IPERF_NAME} is not configured!"; exit 1;\
	fi

	@echo "================ Compiling IPERF $(IPERF_VER) ==============================="
	@${MAKE} ${MAKE_OPTIONS} -C ${IPERF_TAR_PATH}/${IPERF_NAME} -j10 ${MAKE_SUFFIX} >/dev/null 2>&1

install_to_nexus_bin: all
	@echo "================ Installing IPERF $(IPERF_VER) to nexus/bin ================="
	@mkdir -p ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(IPERF_EXE_FOLDER)/iperf ${NEXUS_BIN_DIR}

clean :
	@echo "================ MAKE CLEAN =========================================="
	@rm -rf ${IPERF_TAR_PATH}
	@rm -rf ${NEXUS_BIN_DIR}/iperf
