###############################################################################
#   Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to the terms and
#   conditions of a separate, written license agreement executed between you and Broadcom
#   (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#   no license (express or implied), right to use, or waiver of any kind with respect to the
#   Software, and Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#   HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#   NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#   secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#   and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#   AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#   WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#   THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#   OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#   LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#   OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#   USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#   LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#   EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#   USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#   THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#   ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#   LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#   ANY LIMITED REMEDY.
#
###############################################################################
THIS_DIR := BSEAV/opensource/sqlite
SHELL:=/bin/bash
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OPENSOURCE ?= ${ROOT_DIR}/../
B_REFSW_TOP ?= $(subst /${THIS_DIR},,$(ROOT_DIR))

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include ${NEXUS_TOP}/../${THIS_DIR}/sqlite.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_SQLITE_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)
SQLITE_CONFIG_CFLAGS  := -O2 -fPIC -DSQLITE_MAX_WRITE_SIZE=65536

all : source

.PHONY: all clean

B_SQLITE_EXTRACT_REDIR :=

ifeq ($(B_REFSW_SQLITE_IS_EXTERNAL),y)
    $(warning B_REFSW_SQLITE_IS_EXTERNAL=${B_REFSW_SQLITE_IS_EXTERNAL} Skipping build of sqlite...)
endif   #  ifeq ($(B_REFSW_SQLITE_IS_EXTERNAL),y)

source:
ifneq ($(B_REFSW_SQLITE_IS_EXTERNAL),y)
	@if [ ! -d "${SQLITE_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "sqlite source directory: " $(abspath ${SQLITE_SOURCE_PATH}) ; \
		echo "not found; extracting from tarball..."  ${B_SQLITE_EXTRACT_REDIR}; \
		echo ""; \
		echo "SQLITE_SOURCE_DIRECTORY is ${SQLITE_SOURCE_DIRECTORY}"; \
		if [ ! -e "${SQLITE_TAR_PATH}/${SQLITE_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading SQLITE $(SQLITE_VER) tarball  to ${SQLITE_TAR_PATH}"; \
			mkdir -p ${SQLITE_TAR_PATH}; \
			echo "${SQLITE_BUILD_DIR}/${SQLITE_NAME}"; \
			echo "${SQLITE_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${SQLITE_BUILD_DIR}/${SQLITE_NAME}.info ${SQLITE_TAR_PATH}; \
		fi ;\
		if [ -e "${SQLITE_TAR_PATH}/${SQLITE_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting SQLITE $(SQLITE_VER) tarball"; \
			echo "${SQLITE_SOURCE_PATH}"; \
			mkdir -p ${SQLITE_SOURCE_PATH}; \
			echo "SQLITE_TAR_PATH is ${SQLITE_TAR_PATH}"; \
			echo "SQLITE_SOURCE_TARBALL is ${SQLITE_SOURCE_TARBALL}"; \
			tar -xzf ${SQLITE_TAR_PATH}/${SQLITE_SOURCE_TARBALL} -C ${SQLITE_SOURCE_PATH}/..; \
		else \
			echo "Tarball ${SQLITE_TAR_PATH}/${SQLITE_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_SQLITE_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_SQLITE_IS_EXTERNAL),y)

# Determine if we are compiling for 64-bit machine
COMPILER := ${B_REFSW_ARCH}-gcc
BIT64 = $(findstring aarch64,${COMPILER})

all : source
	@echo "================ TARGET all: SQLITE $(SQLITE_VER) ============================="
	@echo "================ Configuring SQLITE $(SQLITE_VER) ============================="

	@if [ ! -e "${SQLITE_TAR_PATH}/${SQLITE_NAME}/Makefile" ]; then \
		echo "================ Configure SQLITE $(SQLITE_VER)"; \
		cd ${SQLITE_TAR_PATH}/${SQLITE_NAME}; \
		CFLAGS="${SQLITE_CONFIG_CFLAGS}" \
		LDFLAGS="${SQLITE_CONFIG_LDFLAGS}" \
		./configure --host=${B_SQLITE_TOOLCHAIN_ARCH} >/dev/null 2>&1 ; \
		echo "================ Configuring SQLITE $(SQLITE_VER) complete ===================="; \
	else \
		echo "================ SQLITE $(SQLITE_VER) already configured"; \
	fi

	@if [ ! -e "${SQLITE_TAR_PATH}/${SQLITE_NAME}/Makefile" ]; then \
		echo "${SQLITE_NAME} is not configured!"; exit 1;\
	fi

	@echo "================ Compiling SQLITE $(SQLITE_VER) ==============================="
	@${MAKE} ${MAKE_OPTIONS} -C ${SQLITE_TAR_PATH}/${SQLITE_NAME} -j10 ${MAKE_SUFFIX} >/dev/null 2>&1

	@echo "================ Installing SQLITE $(SQLITE_VER) to nexus/bin ================="
	@mkdir -p ${NEXUS_BIN_DIR}
	@(cd $(SQLITE_LIB_FOLDER) && tar cBf - libsqlite*.so*) | (cd ${NEXUS_BIN_DIR} && tar xBf -)
	@(cd $(SQLITE_SOURCE_PATH) && tar cBf - sqlite3) | (cd ${NEXUS_BIN_DIR} && tar xBf -)

clean :
	@echo "================ MAKE CLEAN =========================================="
	@rm -rf ${SQLITE_TAR_PATH}
	@rm -rf ${NEXUS_BIN_DIR}/sqlite* ${NEXUS_BIN_DIR}/libsqlite*
