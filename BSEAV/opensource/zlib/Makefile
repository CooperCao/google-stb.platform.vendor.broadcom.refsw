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

PWD := ${shell pwd}
OPENSOURCE ?= ${PWD}/../
THIS_DIR := BSEAV/opensource/zlib
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(BSEAV_TOP)/opensource/zlib/zlib.inc
include zlib.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_ZLIB_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)
ZLIB_CONFIG_CFLAGS  := -O2 -fPIC -DZLIB_MAX_WRITE_SIZE=65536

.PHONY: source all clean

B_ZLIB_EXTRACT_REDIR :=

ifeq ($(B_REFSW_ZLIB_IS_EXTERNAL),y)
    $(warning B_REFSW_ZLIB_IS_EXTERNAL=${B_REFSW_ZLIB_IS_EXTERNAL} Skipping build of zlib...)
endif   #  ifeq ($(B_REFSW_ZLIB_IS_EXTERNAL),y)

all : source
	@if [ ! -d "${ZLIB_SOURCE_PATH}" ]; then \
		echo "zlib source directory: $(abspath ${ZLIB_SOURCE_PATH}) not found, extracting from tarball..."  ${B_ZLIB_EXTRACT_REDIR}; \
		if [ ! -e "${ZLIB_TAR_PATH}/${ZLIB_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading ZLIB tarball  to ${ZLIB_TAR_PATH}"; \
			mkdir -p ${ZLIB_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${ZLIB_BUILD_DIR}/${ZLIB_NAME}.info ${ZLIB_TAR_PATH}; \
		fi ;\
	fi

	@echo "================ Compiling ZLIB==============================="
	echo "NEXUS_TOP=$(abspath ${NEXUS_TOP})"
	${MAKE} ${MAKE_OPTIONS} -C ${ZLIB_TAR_PATH} BSEAV=$(abspath ${BSEAV_TOP}) ${MAKE_SUFFIX}
	if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${ZLIB_LIB_FOLDER}/*.so > /dev/null 2>&1 ; then ${STRIP} ${ZLIB_LIB_FOLDER}/*.so ; fi ; \
	fi
	@echo "================ ZLIB Compiled!"

source:
ifneq ($(B_REFSW_ZLIB_IS_EXTERNAL),y)
	@if [ ! -d "${ZLIB_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "zlib source directory: $(abspath ${ZLIB_SOURCE_PATH}) not found, extracting from tarball..."  ${B_ZLIB_EXTRACT_REDIR}; \
		echo ""; \
		echo "ZLIB_SOURCE_DIRECTORY is ${ZLIB_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${ZLIB_TAR_PATH}/${ZLIB_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading ZLIB tarball  to ${ZLIB_TAR_PATH}"; \
			mkdir -p ${ZLIB_TAR_PATH}; \
			echo "${ZLIB_BUILD_DIR}/${ZLIB_NAME}"; \
			echo "${ZLIB_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${ZLIB_BUILD_DIR}/${ZLIB_NAME}.info ${ZLIB_TAR_PATH}; \
		fi ;\
		if [ -e "${ZLIB_TAR_PATH}/${ZLIB_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting ZLIB tarball"; \
			echo "${ZLIB_SOURCE_PATH}"; \
			mkdir -p ${ZLIB_SOURCE_PATH}; \
			echo "ZLIB_TAR_PATH is ${ZLIB_TAR_PATH}"; \
			echo "ZLIB_SOURCE_TARBALL is ${ZLIB_SOURCE_TARBALL}"; \
			tar -xpf ${ZLIB_TAR_PATH}/${ZLIB_SOURCE_TARBALL} -C ${ZLIB_SOURCE_PATH}/..; \
			echo "Copy ${ZLIB_SOURCE_PATH} include files to ${ZLIB_SOURCE_PATH}/include" ; \
            mkdir -p ${ZLIB_SOURCE_PATH}/include ; \
			cp -r ${ZLIB_SOURCE_PATH}/*.h ${ZLIB_SOURCE_PATH}/include ; \
			echo "Copy ${ZLIB_BUILD_DIR}/${ZLIB_NAME} to ${ZLIB_SOURCE_PATH}/.." ; \
			cp -r ${ZLIB_BUILD_DIR}/${ZLIB_NAME} ${ZLIB_SOURCE_PATH}/.. ; \
		else \
			echo "Tarball ${ZLIB_TAR_PATH}/${ZLIB_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_ZLIB_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_ZLIB_IS_EXTERNAL),y)

install_to_nexus_bin: all
	@echo "================ Installing ZLIB to ${NEXUS_BIN_DIR}"
	@mkdir -p ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(ZLIB_LIB_FOLDER)/*.so* ${NEXUS_BIN_DIR}

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf zlib_ver.inc
	@rm -rf ${ZLIB_TAR_PATH}
	@echo "================ Done"
