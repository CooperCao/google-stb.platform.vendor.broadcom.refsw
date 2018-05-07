###############################################################################
#  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
###############################################################################
THIS_DIR := BSEAV/opensource/pixman
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
include ${BSEAV_TOP}/opensource/pixman/pixman.inc

FETCH = $(realpath $(BSEAV_TOP))/tools/build/fetch.sh
SQUASH_OUTPUT = > /dev/null

SRC_URI = "http://cairographics.org/releases/$(PIXMAN_SOURCE_TARBALL)"

CONFIGURE_ENV = \
	CC=$(CC) \
	PKG_CONFIG_PATH="${PIXMAN_INSTALL_TARGET}/usr/local/lib/pkgconfig"

CONFIGURE_ARGS = \
	--prefix=${PIXMAN_INSTALL_PREFIX} \
	--disable-gtk \
	--disable-arm-neon \
	--disable-arm-simd \
	--disable-arm-iwmmxt \
	--disable-arm-iwmmxt2 \
	--build=$(shell arch) \
	--host=${PIXMAN_ARCH}

BUILD=$(BUILD_FOLDER)/.build

.PHONY: clean

all: $(PIXMAN_TAR_PATH)/$(PIXMAN_NAME)/pixman/.libs/libpixman-1.so

$(PIXMAN_TAR_PATH)/$(PIXMAN_SOURCE_TARBALL):
	@mkdir -p ${PIXMAN_SOURCE_PATH}
	@$(FETCH) pixman-$(PIXMAN_VER).info $(PIXMAN_TAR_PATH)

$(PIXMAN_TAR_PATH)/$(PIXMAN_NAME)/pixman/.libs/libpixman-1.so: $(PIXMAN_TAR_PATH)/$(PIXMAN_SOURCE_TARBALL)
	@echo "================ Compiling Pixman "
	@tar -xzf ${PIXMAN_TAR_PATH}/${PIXMAN_SOURCE_TARBALL} -C ${PIXMAN_TAR_PATH}
	@cd ${PIXMAN_TAR_PATH}/${PIXMAN_NAME}; \
	$(CONFIGURE_ENV) ./configure $(CONFIGURE_ARGS) > /dev/null
	@${MAKE} ${MAKE_OPTIONS} -C  ${PIXMAN_TAR_PATH}/${PIXMAN_NAME} ${MAKE_SUFFIX} $(SQUASH_OUTPUT)
	@${MAKE} ${MAKE_OPTIONS} -C  ${PIXMAN_TAR_PATH}/${PIXMAN_NAME} install ${MAKE_SUFFIX} $(SQUASH_OUTPUT)
	@echo "================ Pixman Compiled "

clean:
	rm -rf ${PIXMAN_TAR_PATH}/${PIXMAN_NAME}
	@echo "================ Done"
