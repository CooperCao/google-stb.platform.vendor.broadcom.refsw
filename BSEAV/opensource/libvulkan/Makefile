#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
THIS_DIR := BSEAV/opensource/libvulkan
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(BSEAV_TOP)/opensource/libvulkan/libvulkan.inc
include libvulkan.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_LIBVULKAN_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)

.PHONY: source all clean

B_LIBVULKAN_EXTRACT_REDIR :=

ifeq ($(B_REFSW_LIBVULKAN_IS_EXTERNAL),y)
    $(warning B_REFSW_LIBVULKAN_IS_EXTERNAL=${B_REFSW_LIBVULKAN_IS_EXTERNAL} Skipping build of libvulkan...)
endif   #  ifeq ($(B_REFSW_LIBVULKAN_IS_EXTERNAL),y)

all : source
	@if [ ! -d "${LIBVULKAN_SOURCE_PATH}" ]; then \
		echo "Vulkan libvulkan source directory: $(abspath ${LIBVULKAN_SOURCE_PATH}) not found, extracting from tarball..."  ${B_LIBVULKAN_EXTRACT_REDIR}; \
		if [ ! -e "${LIBVULKAN_TAR_PATH}/${LIBVULKAN_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading libvulkan (loader) tarball  to ${LIBVULKAN_TAR_PATH}"; \
			mkdir -p ${LIBVULKAN_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${LIBVULKAN_BUILD_DIR}/${LIBVULKAN_NAME}.info ${LIBVULKAN_TAR_PATH}; \
		fi ;\
	fi

	@echo "================ Compiling LIBVULKAN==============================="
	${MAKE} ${MAKE_OPTIONS} -C ${LIBVULKAN_TAR_PATH} NEXUS_TOP=${NEXUS_TOP} BSEAV=$(abspath ${BSEAV_TOP}) \
								LIBVULKAN_SOURCE_EXTRACTED=${LIBVULKAN_SOURCE_EXTRACTED} ${MAKE_SUFFIX}
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${LIBVULKAN_LIB_FOLDER}/*.so > /dev/null 2>&1 ; then ${STRIP} ${LIBVULKAN_LIB_FOLDER}/*.so ; fi \
	fi
	@echo "================ LIBVULKAN Compiled!"

source:
ifneq ($(B_REFSW_LIBVULKAN_IS_EXTERNAL),y)
	@if [ ! -d "${LIBVULKAN_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "libvulkan source directory: $(abspath ${LIBVULKAN_SOURCE_PATH}) not found, extracting from tarball..."  ${B_LIBVULKAN_EXTRACT_REDIR}; \
		echo ""; \
		echo "LIBVULKAN_SOURCE_DIRECTORY is ${LIBVULKAN_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${LIBVULKAN_TAR_PATH}/${LIBVULKAN_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading libvulkan (loader) tarball to ${LIBVULKAN_TAR_PATH}"; \
			mkdir -p ${LIBVULKAN_TAR_PATH}; \
			echo "${LIBVULKAN_BUILD_DIR}/${LIBVULKAN_NAME}"; \
			echo "${LIBVULKAN_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${LIBVULKAN_BUILD_DIR}/${LIBVULKAN_NAME}.info ${LIBVULKAN_TAR_PATH}; \
		fi ;\
		if [ -e "${LIBVULKAN_TAR_PATH}/${LIBVULKAN_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting libvulkan (loader) tarball"; \
			echo "${LIBVULKAN_SOURCE_PATH}"; \
			mkdir -p ${LIBVULKAN_SOURCE_PATH}; \
			echo "LIBVULKAN_TAR_PATH is ${LIBVULKAN_TAR_PATH}"; \
			echo "LIBVULKAN_SOURCE_TARBALL is ${LIBVULKAN_SOURCE_TARBALL}"; \
			echo "tar -xpf ${LIBVULKAN_TAR_PATH}/${LIBVULKAN_SOURCE_TARBALL} -C ${LIBVULKAN_SOURCE_PATH}/.."; \
			tar -xpf ${LIBVULKAN_TAR_PATH}/${LIBVULKAN_SOURCE_TARBALL} -C ${LIBVULKAN_SOURCE_PATH}/..; \
			echo "Copy ${LIBVULKAN_BUILD_DIR}/${LIBVULKAN_NAME} to ${LIBVULKAN_SOURCE_PATH}/.." ; \
			cp -r ${LIBVULKAN_BUILD_DIR}/${LIBVULKAN_NAME} ${LIBVULKAN_SOURCE_PATH}/.. ; \
		else \
			echo "Tarball ${LIBVULKAN_TAR_PATH}/${LIBVULKAN_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_LIBVULKAN_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_LIBVULKAN_IS_EXTERNAL),y)

install_to_nexus_bin: all
	@echo "================ Installing libvulkan (loader) to ${NEXUS_BIN_DIR}"
	@mkdir -p ${NEXUS_BIN_DIR}

	@install -c -m 0755 $(LIBVULKAN_LIB_FOLDER)/libvulkan.so.${LIBVULKAN_VER} ${NEXUS_BIN_DIR}
	@cd ${NEXUS_BIN_DIR} && ln -s -f libvulkan.so.${LIBVULKAN_VER} libvulkan.so.${LIBVULKAN_VER_MAJ}
	@cd ${NEXUS_BIN_DIR} && ln -s -f libvulkan.so.${LIBVULKAN_VER_MAJ} libvulkan.so

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf ${LIBVULKAN_TAR_PATH}
	@echo "================ Done"
