###############################################################################
#   Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to the terms and
#   conditions of a separate, written license agreement executed between you and Broadcom
#   (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#   no license (express or implied), right to use, or waiver of any kind with respect to the
#   Software, and Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#   HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#   NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#   secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#   and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#   AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#   WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#   THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#   OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#   LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#   OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#   USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#   LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#   EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#   USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#   THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#   ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#   LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#   ANY LIMITED REMEDY.
#
###############################################################################
THIS_DIR := BSEAV/opensource/fontconfig
SHELL:=/bin/bash
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OPENSOURCE ?= ${ROOT_DIR}/../
B_REFSW_TOP ?= $(subst /${THIS_DIR},,$(ROOT_DIR))

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include ${BSEAV_TOP}/opensource/fontconfig/fontconfig.inc
include ${BSEAV_TOP}/opensource/freetype/freetype.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

SQUASH_OUTPUT = > /dev/null
FONTCONFIG_CONFIG_CFLAGS  := -O2 -fPIC
FETCH = $(realpath $(BSEAV_TOP))/tools/build/fetch.sh

SRC_URI = "http://www.freedesktop.org/software/fontconfig/release/$(FONTCONFIG_SOURCE_TARBALL)"

.PHONY: clean all

all: $(FONTCONFIG_TAR_PATH)/$(FONTCONFIG_NAME)/src/.libs/libfontconfig.so

$(FONTCONFIG_TAR_PATH)/$(FONTCONFIG_SOURCE_TARBALL):
	@mkdir -p ${FONTCONFIG_SOURCE_PATH}
	@$(FETCH) fontconfig-$(FONTCONFIG_VER).info $(FONTCONFIG_TAR_PATH)

$(FONTCONFIG_TAR_PATH)/$(FONTCONFIG_NAME)/src/.libs/libfontconfig.so: $(FONTCONFIG_TAR_PATH)/$(FONTCONFIG_SOURCE_TARBALL)
	@if [ ! -d "${FONTCONFIG_TAR_PATH}/${FONTCONFIG_NAME}" ]; then \
		if [ -e "${FONTCONFIG_TAR_PATH}/${FONTCONFIG_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting FONTCONFIG tarball"; \
			tar -xzf ${FONTCONFIG_TAR_PATH}/${FONTCONFIG_SOURCE_TARBALL} -C ${FONTCONFIG_TAR_PATH}; \
			cp config.sub ${FONTCONFIG_TAR_PATH}/${FONTCONFIG_NAME}; \
		else \
			echo "Tarball ${FONTCONFIG_TAR_PATH}/${FONTCONFIG_SOURCE_TARBALL} is missing"; exit 1;\
		fi \
	fi

	@if [ ! -e "${FONTCONFIG_TAR_PATH}/${FONTCONFIG_NAME}/Makefile" ]; then \
		echo "================ Configure FONTCONFIG"; \
		cd ${FONTCONFIG_TAR_PATH}/${FONTCONFIG_NAME}; \
		CC="${CC}" \
		CFLAGS="${FONTCONFIG_CONFIG_CFLAGS}" \
		./configure \
		--prefix=${FONTCONFIG_INSTALL_PREFIX} \
		--host=${FONTCONFIG_ARCH} \
		--with-expat=$(FONTCONFIG_INSTALL_TARGET) \
		--with-expat-includes=$(FONTCONFIG_INSTALL_TARGET)/usr/local/include \
		--with-expat-lib=$(FONTCONFIG_INSTALL_TARGET)/usr/local/lib \
		--disable-static \
		--disable-docs \
		--with-arch=${FONTCONFIG_ARCH} \
		--with-freetype-config=${FONTCONFIG_INSTALL_PREFIX}/bin/freetype-config \
		--enable-static \
		--disable-libxml2 \
		--enable-shared ${SQUASH_OUTPUT}; \
	fi

	@if [ ! -e "${FONTCONFIG_TAR_PATH}/${FONTCONFIG_NAME}/Makefile" ]; then \
		echo "${FONTCONFIG_NAME} is not configured!"; exit 1;\
	fi

	@echo "================ Compiling FONTCONFIG"
	@${MAKE} ${MAKE_OPTIONS} -C ${FONTCONFIG_TAR_PATH}/${FONTCONFIG_NAME} ${MAKE_SUFFIX} ${SQUASH_OUTPUT}
	@${MAKE} -C ${FONTCONFIG_TAR_PATH}/${FONTCONFIG_NAME} install ${MAKE_SUFFIX} ${SQUASH_OUTPUT}
	@sed -i 's/usr/usr\/local/g' ${FONTCONFIG_INSTALL_TARGET}/usr/local/etc/fonts/fonts.conf

clean:
	@echo "================ MAKE CLEAN"
	@rm -rf ${FONTCONFIG_TAR_PATH}/${FONTCONFIG_NAME}
	@rm -rf .$(FONTCONFIG_NAME)
	@echo "================ Done"
