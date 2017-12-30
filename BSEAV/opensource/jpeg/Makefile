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
B_REFSW_OS ?= linuxuser

include ../../../nexus/build/os/$(B_REFSW_OS)/os_tools.inc

BSEAV = $(shell cd "../.." && ${PWD})

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

include jpeg.inc

.PHONY: all clean

all: ${JPEG_LIB_FOLDER}/libjpeg.a

$(JPEG_TAR_PATH)/Makefile:
	@if [ ! -d "${JPEG_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "jpeg source directory: $(abspath ${JPEG_SOURCE_PATH}) not found, extracting from tarball..."  ${B_JPEG_EXTRACT_REDIR}; \
		echo ""; \
		echo "JPEG_BUILD_DIR is ${JPEG_BUILD_DIR}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${JPEG_TAR_PATH}/${JPEG_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading jpeg tarball  to ${JPEG_TAR_PATH}"; \
			mkdir -p ${JPEG_TAR_PATH}; \
			echo "${JPEG_BUILD_DIR}/${JPEG_NAME}"; \
			echo "${JPEG_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(BSEAV)/tools/build/fetch.sh ${JPEG_BUILD_DIR}/${JPEG_NAME}.info ${JPEG_TAR_PATH}; \
		fi ;\
		if [ -e "${JPEG_TAR_PATH}/${JPEG_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting jpeg tarball"; \
			echo "${JPEG_SOURCE_PATH}"; \
			mkdir -p ${JPEG_SOURCE_PATH}; \
			echo "JPEG_TAR_PATH is ${JPEG_TAR_PATH}"; \
			echo "JPEG_SOURCE_TARBALL is ${JPEG_SOURCE_TARBALL}"; \
			tar -xpf ${JPEG_TAR_PATH}/${JPEG_SOURCE_TARBALL} -C ${JPEG_TAR_PATH}/..; \
			cp ${JPEG_BUILD_DIR}/config.sub ${JPEG_SOURCE_PATH} ; \
		else \
			echo "Tarball ${JPEG_TAR_PATH}/${JPEG_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_JPEG_EXTRACT_REDIR};\
	fi

$(JPEG_TAR_PATH)/.libs/libjpeg.a: $(JPEG_TAR_PATH)/Makefile
	@echo [Config.... libjpeg]
	@cd $(JPEG_TAR_PATH); \
	AS=$(AS) \
	LD=$(LD) \
	CC="$(CC)" \
	CXX=$(CXX) \
	AR=$(AR) \
	NM=$(NM) \
	STRIP=$(STRIP) \
	OBJCOPY=$(OBJCOPY) \
	OBJDUMP=$(OBJDUMP) \
	RANLIB=$(RANLIB) \
	CFLAGS="-fPIC -DPIC -O2" \
	CXXFLAGS="-fPIC -DPIC -O2" \
	CPPFLAGS="-fPIC -DPIC -O2" \
	./configure --build=`./config.guess` --host=${B_REFSW_ARCH} --prefix=$(JPEG_INSTALL_PREFIX) >/dev/null 2>&1;
	@echo [Build..... libjpeg]
	@$(MAKE) ${MAKE_OPTIONS} -C $(JPEG_TAR_PATH) > /dev/null;

${JPEG_LIB_FOLDER}/libjpeg.a: $(JPEG_TAR_PATH)/.libs/libjpeg.a
	@echo [Install... libjpeg]
	@$(MAKE) -C $(JPEG_TAR_PATH) install > /dev/null
	@cp $(JPEG_TAR_PATH)/jinclude.h $(JPEG_INC_FOLDER)
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${JPEG_LIB_FOLDER}/libjpeg.so.${JPEG_INSTALL_VER} > /dev/null 2>&1 ; then ${STRIP} ${JPEG_LIB_FOLDER}/libjpeg.so.${JPEG_INSTALL_VER}; fi \
	fi

clean:
	@rm -fr $(JPEG_TAR_PATH)
