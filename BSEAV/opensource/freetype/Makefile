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
THIS_DIR := BSEAV/opensource/freetype
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(BSEAV_TOP)/opensource/freetype/freetype.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_FREETYPE_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)
FREETYPE_CONFIG_CFLAGS  := -O2 -fPIC -DFREETYPE_MAX_WRITE_SIZE=65536

.PHONY: source all clean
.DEFAULT: all
B_FREETYPE_EXTRACT_REDIR :=

ifeq ($(B_REFSW_FREETYPE_IS_EXTERNAL),y)
    $(warning B_REFSW_FREETYPE_IS_EXTERNAL=${B_REFSW_FREETYPE_IS_EXTERNAL} Skipping build of freetype...)
endif   #  ifeq ($(B_REFSW_FREETYPE_IS_EXTERNAL),y)

all : source
	@if [ ! -d "${FREETYPE_SOURCE_PATH}" ]; then \
		echo "Freetype source directory: $(abspath ${FREETYPE_SOURCE_PATH}) not found, extracting from tarball..."  ${B_FREETYPE_EXTRACT_REDIR}; \
		if [ ! -e "${FREETYPE_TAR_PATH}/${FREETYPE_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading OPENSSL tarball  to ${FREETYPE_TAR_PATH}"; \
			mkdir -p ${FREETYPE_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${FREETYPE_BUILD_DIR}/${FREETYPE_NAME}.info ${FREETYPE_TAR_PATH}; \
		fi ;\
	fi

	@echo "================ Configuring FREETYPE 2.3.12 ============================="
	@if [ ! -e "${FREETYPE_TAR_PATH}/config.mk" ]; then \
		echo "================ Configure FREETYPE"; \
		cd ${FREETYPE_TAR_PATH}; \
		./configure --host=${B_REFSW_ARCH} >/dev/null 2>&1 ; \
		echo "================ Configuring FREETYPE 2.3.12 complete ===================="; \
	else \
		echo "================ FREETYPE 2.3.12 already configured"; \
	fi

	@echo "================ Compiling FREETYPE 2.3.12 ==============================="
	echo "NEXUS_TOP=$(abspath ${NEXUS_TOP})"
	${MAKE} ${MAKE_OPTIONS} -C ${FREETYPE_TAR_PATH} NEXUS_TOP=$(abspath ${NEXUS_TOP}) ${MAKE_SUFFIX}
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${FREETYPE_LIB_FOLDER}/*.so > /dev/null 2>&1 ; then ${STRIP} ${FREETYPE_LIB_FOLDER}/*.so ; fi \
	fi
	@echo "================ FREETYPE 2.3.12 Compiled!"

source:
ifneq ($(B_REFSW_FREETYPE_IS_EXTERNAL),y)
	@if [ ! -d "${FREETYPE_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "freetype source directory: $(abspath ${FREETYPE_SOURCE_PATH}) not found, extracting from tarball..."  ${B_FREETYPE_EXTRACT_REDIR}; \
		echo ""; \
		echo "FREETYPE_SOURCE_DIRECTORY is ${FREETYPE_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${FREETYPE_TAR_PATH}/${FREETYPE_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading FREETYPE 2.3.12 tarball  to ${FREETYPE_TAR_PATH}"; \
			mkdir -p ${FREETYPE_TAR_PATH}; \
			echo "${FREETYPE_BUILD_DIR}/${FREETYPE_NAME}"; \
			echo "${FREETYPE_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${FREETYPE_BUILD_DIR}/${FREETYPE_NAME}.info ${FREETYPE_TAR_PATH}; \
		fi ;\
		if [ -e "${FREETYPE_TAR_PATH}/${FREETYPE_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting FREETYPE 2.3.12 tarball"; \
			echo "${FREETYPE_SOURCE_PATH}"; \
			mkdir -p ${FREETYPE_SOURCE_PATH}; \
			echo "FREETYPE_TAR_PATH is ${FREETYPE_TAR_PATH}"; \
			echo "FREETYPE_SOURCE_TARBALL is ${FREETYPE_SOURCE_TARBALL}"; \
			tar -xpf ${FREETYPE_TAR_PATH}/${FREETYPE_SOURCE_TARBALL} -C ${FREETYPE_SOURCE_PATH}/..; \
			echo "Copy ${FREETYPE_BUILD_DIR}/${FREETYPE_NAME} to ${FREETYPE_SOURCE_PATH}/.." ; \
			cp -r ${FREETYPE_BUILD_DIR}/${FREETYPE_NAME} ${FREETYPE_SOURCE_PATH}/.. ; \
		else \
			echo "Tarball ${FREETYPE_TAR_PATH}/${FREETYPE_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_FREETYPE_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_FREETYPE_IS_EXTERNAL),y)

install_to_nexus_bin: all
	@echo "================ Installing FREETYPE 2.3.12 to ${NEXUS_BIN_DIR}"
	@mkdir -p ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(FREETYPE_LIB_FOLDER)/*.so* ${NEXUS_BIN_DIR}

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf freetype_ver.inc
	@rm -rf ${FREETYPE_TAR_PATH}
	@echo "================ Done"
