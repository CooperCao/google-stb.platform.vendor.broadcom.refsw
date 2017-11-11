#############################################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
THIS_DIR := BSEAV/opensource/libpng
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(BSEAV_TOP)/opensource/libpng/libpng.inc
include libpng.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_LIBPNG_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)
LIBPNG_CONFIG_CFLAGS  := -O2 -fPIC -DLIBPNG_MAX_WRITE_SIZE=65536

.PHONY: all clean

B_LIBPNG_EXTRACT_REDIR :=

ifeq ($(B_REFSW_LIBPNG_IS_EXTERNAL),y)
    $(warning B_REFSW_LIBPNG_IS_EXTERNAL=${B_REFSW_LIBPNG_IS_EXTERNAL} Skipping build of libpng...)
endif   #  ifeq ($(B_REFSW_LIBPNG_IS_EXTERNAL),y)

all: ${LIBPNG_LIB_FOLDER}/libpng.a

${LIBPNG_LIB_FOLDER}/libpng.a: ${LIBPNG_TAR_PATH}/.libs/libpng16.a
	@${MAKE} ${MAKE_OPTIONS} -C ${LIBPNG_TAR_PATH} BSEAV=$(abspath ${BSEAV_TOP}) install ${MAKE_SUFFIX} > /dev/null 2>&1
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${LIBPNG_LIB_FOLDER}/libpng${LIBPNG_INSTALL_VER}.so.${LIBPNG_INSTALL_VER_FULL}.0 > /dev/null 2>&1 ; then \
		    ${STRIP} ${LIBPNG_LIB_FOLDER}/libpng${LIBPNG_INSTALL_VER}.so.${LIBPNG_INSTALL_VER_FULL}.0 ; fi \
	fi


${LIBPNG_TAR_PATH}/.libs/libpng16.a: ${LIBPNG_TAR_PATH}/Makefile
	@echo "================ Configuring LIBPNG==============================="
	@cd ${LIBPNG_TAR_PATH}; \
	CC="${CC}" \
	CFLAGS="-I$(LIBPNG_INSTALL_PREFIX)/include" \
	CPPFLAGS="-I$(LIBPNG_INSTALL_PREFIX)/include" \
	LDFLAGS="-L$(LIBPNG_INSTALL_PREFIX)/lib" \
	./configure --host=${B_REFSW_ARCH} --prefix=$(LIBPNG_INSTALL_PREFIX) \
	--with-zlib-prefix=$(LIBPNG_INSTALL_PREFIX) \
	--with-pkgconfigdir=$(LIBPNG_INSTALL_PREFIX)/lib/pkgconfig \
	--enable-static --enable-shared 1>/dev/null 2>&1;
	@echo "================ LIBPNG Configured!"

	@echo "================ Compiling LIBPNG==============================="
	@${MAKE} ${MAKE_OPTIONS} -C ${LIBPNG_TAR_PATH} BSEAV=$(abspath ${BSEAV_TOP}) ${MAKE_SUFFIX} > /dev/null
	@echo "================ LIBPNG Compiled!"

${LIBPNG_TAR_PATH}/Makefile:
ifneq ($(B_REFSW_LIBPNG_IS_EXTERNAL),y)
	@if [ ! -d "${LIBPNG_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "libpng source directory: $(abspath ${LIBPNG_SOURCE_PATH}) not found, extracting from tarball..."  ${B_LIBPNG_EXTRACT_REDIR}; \
		echo ""; \
		echo "LIBPNG_SOURCE_DIRECTORY is ${LIBPNG_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${LIBPNG_TAR_PATH}/${LIBPNG_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading LIBPNG tarball  to ${LIBPNG_TAR_PATH}"; \
			mkdir -p ${LIBPNG_TAR_PATH}; \
			echo "${LIBPNG_BUILD_DIR}/${LIBPNG_NAME}"; \
			echo "${LIBPNG_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${LIBPNG_BUILD_DIR}/${LIBPNG_NAME}.info ${LIBPNG_TAR_PATH}; \
		fi ;\
		if [ -e "${LIBPNG_TAR_PATH}/${LIBPNG_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting LIBPNG tarball"; \
			echo "${LIBPNG_SOURCE_PATH}"; \
			mkdir -p ${LIBPNG_SOURCE_PATH}; \
			echo "LIBPNG_TAR_PATH is ${LIBPNG_TAR_PATH}"; \
			echo "LIBPNG_SOURCE_TARBALL is ${LIBPNG_SOURCE_TARBALL}"; \
			tar -xpf ${LIBPNG_TAR_PATH}/${LIBPNG_SOURCE_TARBALL} -C ${LIBPNG_SOURCE_PATH}/..; \
#			echo "Copy ${LIBPNG_BUILD_DIR}/${LIBPNG_NAME} to ${LIBPNG_SOURCE_PATH}/.." ; \
#			cp -r ${LIBPNG_BUILD_DIR}/${LIBPNG_NAME} ${LIBPNG_SOURCE_PATH}/.. ; \
		else \
			echo "Tarball ${LIBPNG_TAR_PATH}/${LIBPNG_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_LIBPNG_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_LIBPNG_IS_EXTERNAL),y)

install_to_nexus_bin: all
	@echo "================ Installing LIBPNG to ${NEXUS_BIN_DIR}"
	@mkdir -p ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(LIBPNG_LIB_FOLDER)/*.so* ${NEXUS_BIN_DIR}

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf libpng_ver.inc
	@rm -rf ${LIBPNG_TAR_PATH}
	@echo "================ Done"
