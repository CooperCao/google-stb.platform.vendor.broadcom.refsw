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
THIS_DIR := BSEAV/opensource/curl
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(BSEAV_TOP)/opensource/openssl/openssl.inc
include curl.inc
include $(BSEAV_TOP)/opensource/zlib/b_zlib_targets.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_CURL_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)

ZLIB_ODIR=$(B_REFSW_OBJ_ROOT)/BSEAV/opensource/zlib-1.1.3

CURL_CONFIG_CFLAGS  := -O2 -fPIC
CURL_CONFIG_CPPFLAGS := -DCURL_MAX_WRITE_SIZE=65536 -I$(ZLIB_ODIR)/include
CURL_CONFIG_LDFLAGS := -L${B_OPENSSL_SOURCE_PATH} -L$(ZLIB_ODIR)

CA_CERTS_PATH ?= /etc/ssl/certs

.PHONY: source all clean

B_CURL_EXTRACT_REDIR :=

ifeq ($(B_REFSW_CURL_IS_EXTERNAL),y)
    $(warning B_REFSW_CURL_IS_EXTERNAL=${B_REFSW_CURL_IS_EXTERNAL} Skipping build of curl...)
endif   #  ifeq ($(B_REFSW_CURL_IS_EXTERNAL),y)

all : ${CURL_SOURCE_PATH}/.compiled

source:
ifneq ($(B_REFSW_CURL_IS_EXTERNAL),y)
	@if [ ! -d "${CURL_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "curl source directory: $(abspath ${CURL_SOURCE_PATH}) not found, extracting from tarball..."  ${B_CURL_EXTRACT_REDIR}; \
		echo ""; \
		echo "CURL_SOURCE_DIRECTORY is ${CURL_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${CURL_TAR_PATH}/${CURL_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading CURL tarball  to ${CURL_TAR_PATH}"; \
			mkdir -p ${CURL_TAR_PATH}; \
			echo "${CURL_BUILD_DIR}/${CURL_NAME}"; \
			echo "${CURL_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${CURL_BUILD_DIR}/${CURL_NAME}.info ${CURL_TAR_PATH}; \
		fi ;\
		if [ -e "${CURL_TAR_PATH}/${CURL_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting CURL tarball"; \
			echo "${CURL_SOURCE_PATH}"; \
			mkdir -p ${CURL_SOURCE_PATH}; \
			echo "CURL_TAR_PATH is ${CURL_TAR_PATH}"; \
			echo "CURL_SOURCE_TARBALL is ${CURL_SOURCE_TARBALL}"; \
			tar -xzf ${CURL_TAR_PATH}/${CURL_SOURCE_TARBALL} -C ${CURL_SOURCE_PATH}/..; \
		else \
			echo "Tarball ${CURL_TAR_PATH}/${CURL_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_CURL_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_CURL_IS_EXTERNAL),y)

${CURL_SOURCE_PATH}/.compiled : b_zlib_target_build | source
	@if [ ! -d "${CURL_SOURCE_PATH}" ]; then \
		echo "Curl source directory: $(abspath ${CURL_SOURCE_PATH}) not found, extracting from tarball..."  ${B_CURL_EXTRACT_REDIR}; \
		if [ ! -e "${CURL_TAR_PATH}/${CURL_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading CURL tarball  to ${CURL_TAR_PATH}"; \
			mkdir -p ${CURL_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${CURL_BUILD_DIR}/${CURL_NAME}.info ${CURL_TAR_PATH}; \
		fi ;\
	fi
	@echo "================ Configuring CURL B_CURL_TOOLCHAIN_ARCH is $(B_CURL_TOOLCHAIN_ARCH)"

	@if [ ! -e "${CURL_TAR_PATH}/${CURL_NAME}/Makefile" -o -z "${shell grep 'CC \= ' ${CURL_TAR_PATH}/${CURL_NAME}/Makefile |grep ${B_CURL_TOOLCHAIN_ARCH}}" ]; then \
		echo "================ Configure CURL B_CURL_TOOLCHAIN_ARCH is $(B_CURL_TOOLCHAIN_ARCH)"; \
		cd ${CURL_TAR_PATH}/${CURL_NAME}; \
		CC="${CC} -I${B_OPENSSL_SOURCE_PATH}/include" \
		MANIFEST_TOOL="/bin/true" \
		PKGCONFIG="/bin/true" \
		CFLAGS="${CURL_CONFIG_CFLAGS}" \
		CPPFLAGS="${CURL_CONFIG_CPPFLAGS}" \
		LDFLAGS="${CURL_CONFIG_LDFLAGS}" \
		LIBS="-lssl -lcrypto -lz" \
		./configure \
		--host=${B_CURL_TOOLCHAIN_ARCH} \
		--prefix=${CURL_TAR_PATH}/${CURL_NAME} \
		--enable-nonblocking \
		--enable-cookies \
		--enable-crypto-auth \
		--enable-optimize \
                --enable-ssl \
		--with-ca-path="${CA_CERTS_PATH}" \
		--enable-ipv6 \
		--enable-threaded-resolver \
		--disable-verbose \
		--enable-hidden-symbols \
		--enable-shared \
		--enable-thread \
		--without-libpsl \
		--without-libidn \
		--disable-ldap \
		--disable-ldaps \
		--with-pic \
		--with-zlib \
		--with-ssl=${B_OPENSSL_SOURCE_PATH} \
		--disable-manual \
		--with-random=/dev/urandom; \
		echo "================ CURL Configured"; \
	else \
		echo "YOU SHOULD NOT BE HERE!!!"; \
	fi

	@if [ ! -e "${CURL_TAR_PATH}/${CURL_NAME}/Makefile" ]; then \
		echo "${CURL_NAME} is not configured!"; exit 1;\
	fi

	@echo "================ Compiling CURL==============================="
	${MAKE} ${MAKE_OPTIONS} -C ${CURL_TAR_PATH}/${CURL_NAME} ${MAKE_SUFFIX}
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${CURL_LIB_FOLDER}/*.so > /dev/null 2>&1 ; then ${STRIP} ${CURL_LIB_FOLDER}/*.so ; fi \
	fi
	@echo "================ CURL Compiled!"
	@touch $@

install_to_nexus_bin: all
	@echo "================ Installing CURL to ${NEXUS_BIN_DIR}"
	@mkdir -p ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(CURL_LIB_FOLDER)/*.so* ${NEXUS_BIN_DIR}

clean : b_zlib_target_clean_all
	@echo "================ MAKE CLEAN"
	@rm -rf curl_ver.inc
	@rm -rf ${CURL_TAR_PATH}
	@rm -rf ${CURL_SOURCE_PATH}/.compiled
	@echo "================ Done"
