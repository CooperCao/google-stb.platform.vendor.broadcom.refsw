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
THIS_DIR := BSEAV/opensource/cwebsocket
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(BSEAV_TOP)/opensource/openssl/openssl.inc
include cwebsocket.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_CWEBSOCKET_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)

CWEBSOCKET_CONFIG_CFLAGS  := -O2 -fPIC
CWEBSOCKET_CONFIG_CPPFLAGS := -DCWEBSOCKET_MAX_WRITE_SIZE=65536
CWEBSOCKET_CONFIG_LDFLAGS := -L${B_OPENSSL_SOURCE_PATH}

CA_CERTS_PATH ?= /etc/ssl/certs

.PHONY: source all clean

B_CWEBSOCKET_EXTRACT_REDIR :=

ifeq ($(B_REFSW_CWEBSOCKET_IS_EXTERNAL),y)
    $(warning B_REFSW_CWEBSOCKET_IS_EXTERNAL=${B_REFSW_CWEBSOCKET_IS_EXTERNAL} Skipping build of cwebsocket...)
endif   #  ifeq ($(B_REFSW_CWEBSOCKET_IS_EXTERNAL),y)

OBJ_OPENSOURCE_DIR = $(subst nexus/bin,BSEAV/opensource,$(NEXUS_BIN_DIR))
OBJ_OPENSOURCE_DIR_WITH_SLASHES = $(subst /,\/,$(OBJ_OPENSOURCE_DIR))
NEXUS_BIN_DIR_WITH_SLASHES = $(subst /,\/,$(NEXUS_BIN_DIR))

all : libev libopenssl ${CWEBSOCKET_SOURCE_PATH}/.compiled

libopenssl:
	echo "Installing... $@";
	make -C $(WORKING_DIR)/../openssl

libev:
	echo "Installing... $@";
	make -C $(WORKING_DIR)/../libev install_to_nexus_bin

source:
ifneq ($(B_REFSW_CWEBSOCKET_IS_EXTERNAL),y)
	@if [ ! -d "${CWEBSOCKET_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "cwebsocket source directory: $(abspath ${CWEBSOCKET_SOURCE_PATH}) not found, extracting from tarball..."  ${B_CWEBSOCKET_EXTRACT_REDIR}; \
		echo ""; \
		echo "CWEBSOCKET_SOURCE_DIRECTORY is ${CWEBSOCKET_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading CWEBSOCKET tarball  to ${CWEBSOCKET_TAR_PATH}"; \
			mkdir -p ${CWEBSOCKET_TAR_PATH}; \
			echo "${CWEBSOCKET_BUILD_DIR}/${CWEBSOCKET_NAME}"; \
			echo "${CWEBSOCKET_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${CWEBSOCKET_BUILD_DIR}/${CWEBSOCKET_NAME}.info ${CWEBSOCKET_TAR_PATH}; \
		fi ;\
		if [ -e "${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_SOURCE_TARBALL}" ]; then \
		    echo "================ Extracting cwebsocket zipfile ================"; \
			echo "${CWEBSOCKET_SOURCE_PATH}"; \
			(cd ${CWEBSOCKET_TAR_PATH} && unzip ${CWEBSOCKET_SOURCE_TARBALL} >/dev/null 2>&1 && mv cwebsocket-master ${CWEBSOCKET_NAME} ) ; \
			echo "================ autogen cmwebsocket =========================="; \
			(cd ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME} && ./autogen.sh have_libcrypto=no have_libssl=no have_libpthread=no have_libev=no >/dev/null 2>&1 ) ; \
			echo "================ autogen cmwebsocket complete ================="; \
		else \
			echo "Zipfile ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_CWEBSOCKET_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_CWEBSOCKET_IS_EXTERNAL),y)

${CWEBSOCKET_SOURCE_PATH}/.compiled : source
	@if [ ! -e "${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME}/Makefile" ]; then \
		echo "================ Configuring cwebsocket Phase 1 ==============="; \
		(cd ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME} && CC=${CC} ./configure --host=${B_CWEBSOCKET_TOOLCHAIN_ARCH} --enable-ssl=no >/dev/null 2>&1 ); \
		echo "================ Patching cwebsocket =========================="; \
		(cd ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME} && patch -p1 < $(WORKING_DIR)/cwebsocket_configure.patch ); \
		echo "================ Configuring cwebsocket Phase 2 ==============="; \
		(cd ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME} && CC=${CC} ./configure --host=${B_CWEBSOCKET_TOOLCHAIN_ARCH} --enable-ssl=no >/dev/null 2>&1 ); \
		echo "================ Build Makefile Phase 1 ======================="; \
		(cd ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME} && make >/dev/null 2>&1 ); \
		echo "================ Adjust patch file for compiler ==============="; \
		cat ${WORKING_DIR}/cwebsocket_Makefile.patch | sed s/PUT_ARCH_HERE/$(B_REFSW_ARCH)/ | sed "s/PUT_OBJ_OPENSOURCE_PATH_HERE/$(OBJ_OPENSOURCE_DIR_WITH_SLASHES)/" | sed "s/PUT_NEXUS_BIN_PATH_HERE/$(NEXUS_BIN_DIR_WITH_SLASHES)/" | sed "s/PUT_OPENSSL_PATH_HERE/$(B_OPENSSL_NAME)/" > ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME}/Makefile.patch; \
		echo "================ Patching Makefile for compiler ==============="; \
		(cd ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME} && patch -p1 < ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME}/Makefile.patch ); \
		echo "================ Patching source =============================="; \
		(cd ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME} && patch -p1 < $(WORKING_DIR)/cwebsocket_source.patch   ); \
		echo "================ cwebsocket configured ========================"; \
	fi

	@if [ ! -e "${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME}/Makefile" ]; then \
		echo "${CWEBSOCKET_NAME} is not configured!"; exit 1;\
	fi

	@echo "================ Compiling cwebsocket =========================";
	@${MAKE} ${MAKE_OPTIONS} -C ${CWEBSOCKET_TAR_PATH}/${CWEBSOCKET_NAME} ${MAKE_SUFFIX} >/dev/null 2>&1
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${CWEBSOCKET_LIB_FOLDER}/*.so > /dev/null 2>&1 ; then ${STRIP} ${CWEBSOCKET_LIB_FOLDER}/*.so ; fi \
	fi
	@echo "================ cwebsocket Compiled ==========================";
	@touch $@

install_to_nexus_bin: all
	@echo "================ Installing CWEBSOCKET to ${NEXUS_BIN_DIR}"
	@mkdir -p ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(CWEBSOCKET_LIB_FOLDER)/*.so* ${NEXUS_BIN_DIR}

clean :
	@echo "================ Cleaning =====================================";
	@rm -rf cwebsocket_ver.inc
	@rm -rf ${CWEBSOCKET_TAR_PATH}
	@rm -rf ${CWEBSOCKET_SOURCE_PATH}/.compiled
	@echo "================ Cleaning complete ============================";
