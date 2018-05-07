###############################################################################
#   Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
###############################################################################
SHELL:=/bin/bash
PWD := ${shell pwd}
OPENSOURCE ?= ${PWD}/../
THIS_DIR := BSEAV/opensource/cjson
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include ${WORKING_DIR}/cjson.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_CJSON_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)
CJSON_CONFIG_CFLAGS  := -O2 -fPIC -DCJSON_MAX_WRITE_SIZE=65536

all: source

.PHONY: all clean

B_CJSON_EXTRACT_REDIR :=

ifeq ($(B_REFSW_CJSON_IS_EXTERNAL),y)
    $(warning B_REFSW_CJSON_IS_EXTERNAL=${B_REFSW_CJSON_IS_EXTERNAL} Skipping build of cjson ...)
endif   #  ifeq ($(B_REFSW_CJSON_IS_EXTERNAL),y)

source:
ifneq ($(B_REFSW_CJSON_IS_EXTERNAL),y)
	@if [ ! -d "${CJSON_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "cjson source directory: " $(abspath ${CJSON_SOURCE_PATH}) ; \
		echo "not found; extracting from tarball..."  ${B_CJSON_EXTRACT_REDIR}; \
		echo ""; \
		echo "CJSON_SOURCE_DIRECTORY is ${CJSON_SOURCE_DIRECTORY}"; \
		echo "looking for cjson tarball here: ${CJSON_TAR_PATH}/${CJSON_SOURCE_TARBALL}"; \
		if [ ! -e "${CJSON_TAR_PATH}/${CJSON_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading CJSON $(CJSON_VER) tarball  to ${CJSON_TAR_PATH}"; \
			mkdir -p ${CJSON_TAR_PATH}; \
			echo "${CJSON_BUILD_DIR}/${CJSON_NAME}"; \
			echo "${CJSON_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${CJSON_BUILD_DIR}/${CJSON_NAME}.info ${CJSON_TAR_PATH}; \
		fi ;\
		if [ -e "${CJSON_TAR_PATH}/${CJSON_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting ${CJSON_TAR_PATH}/${CJSON_SOURCE_TARBALL}  CJSON $(CJSON_VER) tarball"; \
			(cd ${CJSON_TAR_PATH} && unzip -o ${CJSON_TAR_PATH}/${CJSON_SOURCE_TARBALL} >/dev/null ) ; \
		else \
			echo "Tarball ${CJSON_TAR_PATH}/${CJSON_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_CJSON_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_CJSON_IS_EXTERNAL),y)

# Determine if we are compiling for 64-bit machine
COMPILER := ${B_REFSW_ARCH}-gcc
ARCHIVER := ${B_REFSW_ARCH}-ar
BIT64 = $(findstring aarch64,${COMPILER})

all:source
	@echo "================ TARGET all: CJSON $(CJSON_VER) ============================="
	@echo "================ Compiling CJSON $(CJSON_VER) ==============================="
	@CC=${COMPILER} AR=${ARCHIVER} ${MAKE} ${MAKE_OPTIONS} -C ${CJSON_TAR_PATH}/cJSON-$(CJSON_VER) ${MAKE_SUFFIX} >/dev/null
	@echo "================ Installing CJSON $(CJSON_VER) to nexus/bin ================="
	@mkdir -p ${NEXUS_BIN_DIR}
	@(cd $(CJSON_LIB_FOLDER) && tar cBf - libcjson*.so*) | (cd ${NEXUS_BIN_DIR} && tar xBf -)

clean:
	@echo "================ MAKE CLEAN =========================================="
	@rm -rf ${CJSON_TAR_PATH}
	@rm -rf ${NEXUS_BIN_DIR}/cjson* ${NEXUS_BIN_DIR}/libcjson*
