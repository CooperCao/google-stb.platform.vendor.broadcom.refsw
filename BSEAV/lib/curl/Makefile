#############################################################################
#    (c)2010-2016 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#
#
#
#############################################################################

PWD := ${shell pwd}
OPENSOURCE ?= ${PWD}/../
THIS_DIR := BSEAV/lib/curl
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(BSEAV_TOP)/lib/openssl/openssl.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
    DEBUG_SUFFIX=debug
else
    DEBUG_SUFFIX=release
endif

B_CURL_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)
CURL_VER?=7.41.0
CURL_NAME=curl-$(CURL_VER)

CURL_SOURCE_TARBALL:=$(CURL_NAME).tar.gz
CURL_SOURCE_DIRECTORY:=$(abspath $(B_REFSW_OBJ_ROOT)/BSEAV/lib/curl/$(B_REFSW_ARCH).$(DEBUG_SUFFIX))
CURL_SOURCE_PATH:=${CURL_SOURCE_DIRECTORY}/${CURL_NAME}
CURL_TAR_PATH:=${CURL_SOURCE_DIRECTORY}
CURL_BUILD_DIR:=$(abspath $(NEXUS_TOP)/../BSEAV/lib/curl)

CURL_CFLAGS?= -O2 -fPIC
CURL_LDFLAGS?=
CURL_DIR:=$(CURL_NAME)
CURL_LIB_FOLDER:=${CURL_TAR_PATH}/${CURL_DIR}/lib/.libs

CURL_INC_CFLAGS:= -D_REENTRANT -I${CURL_TAR_PATH}/${CURL_DIR}/include
CURL_INC_LDFLAGS:= -L$(CURL_LIB_FOLDER) -lcurl


.PHONY: source all clean

B_CURL_EXTRACT_REDIR :=

ifeq ($(B_REFSW_CURL_IS_EXTERNAL),y)
    $(warning B_REFSW_CURL_IS_EXTERNAL=${B_REFSW_CURL_IS_EXTERNAL} Skipping build of curl...)
endif   #  ifeq ($(B_REFSW_CURL_IS_EXTERNAL),y)

source:
ifneq ($(B_REFSW_CURL_IS_EXTERNAL),y)
	@if [ ! -d "${CURL_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "curl source directory: $(abspath ${CURL_SOURCE_PATH}) not found, extracting from tarball..."  ${B_CURL_EXTRACT_REDIR}; \
		echo ""; \
		echo "CURL_SOURCE_DIRECTORY is ${CURL_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${CURL_TAR_PATH}/${CURL_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading OPENSSL tarball  to ${CURL_TAR_PATH}"; \
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

#install: all
#	@echo "================ Installing CURL"
#	@echo "APPLIBS_TARGET_LIB_DIR is ${APPLIBS_TARGET_LIB_DIR}"
#	@echo "APPLIBS_TARGET_INC_DIR is ${APPLIBS_TARGET_INC_DIR}"
#	@mkdir -p ${APPLIBS_TARGET_LIB_DIR}
#	@mkdir -p ${APPLIBS_TARGET_INC_DIR}
#	@cp -af $(CURL_TAR_PATH)/$(CURL_NAME)/src/.libs/*.so* ${APPLIBS_TARGET_LIB_DIR}

all : source
	@if [ ! -d "${CURL_SOURCE_PATH}" ]; then \
		echo "Curl source directory: $(abspath ${CURL_SOURCE_PATH}) not found, extracting from tarball..."  ${B_CURL_EXTRACT_REDIR}; \
		if [ ! -e "${CURL_TAR_PATH}/${CURL_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading OPENSSL tarball  to ${CURL_TAR_PATH}"; \
			mkdir -p ${CURL_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${CURL_BUILD_DIR}/${CURL_NAME}.info ${CURL_TAR_PATH}; \
		fi ;\
	fi
	@echo "================ Configuring CURL B_CURL_TOOLCHAIN_ARCH is $(B_CURL_TOOLCHAIN_ARCH)"

	@if [ ! -e "${CURL_TAR_PATH}/${CURL_NAME}/Makefile" -o -z "${shell grep 'CC \= ' ${CURL_TAR_PATH}/${CURL_NAME}/Makefile |grep ${B_CURL_TOOLCHAIN_ARCH}}" ]; then \
		echo "================ Configure CURL"; \
		cd ${CURL_TAR_PATH}/${CURL_NAME}; \
		CFLAGS="${CURL_CFLAGS} -DCURL_MAX_WRITE_SIZE=65536" \
		LDFLAGS="-L${B_OPENSSL_SOURCE_PATH}" \
		LIBS="-lssl -lcrypto" \
		./configure \
		--host=${B_CURL_TOOLCHAIN_ARCH} \
		--prefix=${CURL_TAR_PATH}/${CURL_NAME}/${NEXUS_PLATFORM}${LOWER_BCHP_VER} \
		--enable-nonblocking \
		--enable-cookies \
		--enable-crypto-auth \
		--enable-optimize \
                --enable-ssl \
		--enable-ipv6 \
		--enable-threaded-resolver \
		--disable-verbose \
		--enable-hidden-symbols \
		--enable-shared \
		--enable-thread \
		--without-libidn \
		--with-pic \
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
	${MAKE} ${MAKE_OPTIONS} -C ${CURL_TAR_PATH}/${CURL_NAME} -j10 ${MAKE_SUFFIX}
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		${STRIP} ${CURL_LIB_FOLDER}/*.so; \
	fi
	@rm -rf curl_ver.inc
	@echo "CURL_VER:="$(CURL_VER) >> curl_ver.inc
	@echo "CURL_NAME:="$(CURL_NAME) >> curl_ver.inc
	@echo "CURL_CFLAGS:="$(CURL_INC_CFLAGS) >> curl_ver.inc
	@echo "CURL_LDFLAGS:="$(CURL_INC_LDFLAGS) >> curl_ver.inc
	@echo "CURL_SOURCE_PATH:="$(CURL_SOURCE_PATH) >> curl_ver.inc
	@echo "================ CURL Compiled!"
	@touch $@

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf curl_ver.inc
	@rm -rf ${CURL_TAR_PATH}
	@rm -rf all
	@echo "================ Done"
