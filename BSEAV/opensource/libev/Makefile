#############################################################################
# Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
THIS_DIR := BSEAV/opensource/libev
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(BSEAV_TOP)/opensource/openssl/openssl.inc
include libev.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_LIBEV_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)

LIBEV_CONFIG_CFLAGS  := -O2 -fPIC
LIBEV_CONFIG_CPPFLAGS := -DLIBEV_MAX_WRITE_SIZE=65536
LIBEV_CONFIG_LDFLAGS := -L${B_OPENSSL_SOURCE_PATH}

CA_CERTS_PATH ?= /etc/ssl/certs

.PHONY: source all clean

B_LIBEV_EXTRACT_REDIR :=

ifeq ($(B_REFSW_LIBEV_IS_EXTERNAL),y)
    $(warning B_REFSW_LIBEV_IS_EXTERNAL=${B_REFSW_LIBEV_IS_EXTERNAL} Skipping build of libev...)
endif   #  ifeq ($(B_REFSW_LIBEV_IS_EXTERNAL),y)

all : ${LIBEV_SOURCE_PATH}/.compiled

source:
ifneq ($(B_REFSW_LIBEV_IS_EXTERNAL),y)
	@if [ ! -d "${LIBEV_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "libev source directory: $(abspath ${LIBEV_SOURCE_PATH}) not found, extracting from tarball..."  ${B_LIBEV_EXTRACT_REDIR}; \
		echo ""; \
		echo "LIBEV_SOURCE_DIRECTORY is ${LIBEV_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${LIBEV_TAR_PATH}/${LIBEV_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading LIBEV tarball  to ${LIBEV_TAR_PATH}"; \
			mkdir -p ${LIBEV_TAR_PATH}; \
			echo "${LIBEV_BUILD_DIR}/${LIBEV_NAME}"; \
			echo "${LIBEV_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${LIBEV_BUILD_DIR}/${LIBEV_NAME}.info ${LIBEV_TAR_PATH}; \
		fi ;\
		if [ -e "${LIBEV_TAR_PATH}/${LIBEV_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting LIBEV tarball"; \
			echo "${LIBEV_SOURCE_PATH}"; \
			mkdir -p ${LIBEV_SOURCE_PATH}; \
			echo "LIBEV_TAR_PATH is ${LIBEV_TAR_PATH}"; \
			echo "LIBEV_SOURCE_TARBALL is ${LIBEV_SOURCE_TARBALL}"; \
			tar -xzf ${LIBEV_TAR_PATH}/${LIBEV_SOURCE_TARBALL} -C ${LIBEV_SOURCE_PATH}/..; \
		else \
			echo "Tarball ${LIBEV_TAR_PATH}/${LIBEV_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_LIBEV_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_LIBEV_IS_EXTERNAL),y)

${LIBEV_SOURCE_PATH}/.compiled : source
	@if [ ! -e "${LIBEV_TAR_PATH}/${LIBEV_NAME}/Makefile" ]; then \
		echo "================ Configuring LIBEV $(B_LIBEV_TOOLCHAIN_ARCH)"; \
		echo '(cd ${LIBEV_TAR_PATH}/${LIBEV_NAME} && CC=${CC} ./configure --host=${B_LIBEV_TOOLCHAIN_ARCH})'; \
		(cd ${LIBEV_TAR_PATH}/${LIBEV_NAME} && CC=${CC} ./configure --host=${B_LIBEV_TOOLCHAIN_ARCH} >/dev/null 2>&1 ); \
		echo "================ LIBEV Configured"; \
	fi

	@if [ ! -e "${LIBEV_TAR_PATH}/${LIBEV_NAME}/Makefile" ]; then \
		echo "${LIBEV_NAME} is not configured!"; exit 1;\
	fi

	@echo "================ Building LIBEV================================"
	@${MAKE} ${MAKE_OPTIONS} -C ${LIBEV_TAR_PATH}/${LIBEV_NAME} ${MAKE_SUFFIX}  > /dev/null 2>&1
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${LIBEV_LIB_FOLDER}/*.so > /dev/null 2>&1 ; then ${STRIP} ${LIBEV_LIB_FOLDER}/*.so ; fi \
	fi
	@echo "================ LIBEV Compiled! Run make install_to_nexus_bin to install."
	@touch $@

install_to_nexus_bin: all
	@echo "================ Installing LIBEV to ${NEXUS_BIN_DIR}"
	@mkdir -p ${NEXUS_BIN_DIR}
	@(cd $(LIBEV_LIB_FOLDER) && tar cBf - *.so* ) | (cd ${NEXUS_BIN_DIR} && tar xBf -)

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf libev_ver.inc
	@rm -rf ${LIBEV_TAR_PATH}
	@rm -rf ${LIBEV_SOURCE_PATH}/.compiled
	@rm -rf ${NEXUS_BIN_DIR}/libev.so*
