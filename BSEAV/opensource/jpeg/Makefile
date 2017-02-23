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
B_REFSW_OS ?= linuxuser

include ../../../nexus/build/os/$(B_REFSW_OS)/os_tools.inc

BSEAV = $(shell cd "../.." && ${PWD})

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

include jpeg.inc

all: jpeg

.PHONY: source all clean jpeg

source:
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
			echo "Patch $(JPEG_BUILD_DIR)/patch.txt to ${JPEG_TAR_PATH}" ; \
			patch -d ${JPEG_TAR_PATH} -p1 -i $(JPEG_BUILD_DIR)/patch.txt > /dev/null; \
		else \
			echo "Tarball ${JPEG_TAR_PATH}/${JPEG_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_JPEG_EXTRACT_REDIR};\
	fi

    .PHONY: config-libjpeg
config-libjpeg: source
#speed of machine vs. granularity of creation/modification creates the need for a little sleep here
	@if [ ! -e $(JPEG_TAR_PATH)/Makefile ]; then \
		echo [Config.... libjpeg]; \
		cd $(JPEG_TAR_PATH); RANLIB=$(RANLIB) AR="$(AR) rc" CC='$(CC)' sh ./configure 1> /dev/null; \
		sleep 1; \
	fi

jpeg: config-libjpeg
	@if ! $(MAKE) -q -C $(JPEG_TAR_PATH) libjpeg.a; then \
		echo [Build..... libjpeg]; \
		$(MAKE) -C $(JPEG_TAR_PATH) libjpeg.a 1>/dev/null; \
	fi

clean:
	@rm -fr $(JPEG_TAR_PATH)
