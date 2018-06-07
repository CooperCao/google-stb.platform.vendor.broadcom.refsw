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
THIS_DIR := BSEAV/opensource/expat
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
include ${BSEAV_TOP}/opensource/expat/expat.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

SQUASH_OUTPUT = > /dev/null
FONTCONFIG_CONFIG_CFLAGS  := -O2 -fPIC
FETCH = $(realpath $(BSEAV_TOP))/tools/build/fetch.sh

SRC_URI = "http://sourceforge.net/projects/expat/files/expat/$(EXPAT_VER)/$(EXPAT_SOURCE_TARBALL)"

.PHONY: clean all target-clean

all: $(EXPAT_TAR_PATH)/$(EXPAT_SOURCE_TARBALL) $(EXPAT_TAR_PATH)/.libs/libexpat.so

$(EXPAT_TAR_PATH)/$(EXPAT_SOURCE_TARBALL):
	@mkdir -p ${EXPAT_SOURCE_PATH}
	@$(FETCH) expat-$(EXPAT_VER).info $(EXPAT_TAR_PATH)

$(EXPAT_TAR_PATH)/.libs/libexpat.so: $(EXPAT_TAR_PATH)/$(EXPAT_SOURCE_TARBALL)
	@tar -xzf ${EXPAT_TAR_PATH}/${EXPAT_SOURCE_TARBALL} -C ${EXPAT_TAR_PATH}

	@if [ ! -e "${EXPAT_TAR_PATH}/${EXPAT_NAME}/Makefile" ]; then \
		echo "================ Configure EXPAT"; \
		cd ${EXPAT_TAR_PATH}/${EXPAT_NAME}; \
		CC="${CC}" \
		CFLAGS="${EXPAT_CFLAGS}" \
		./configure \
		--prefix=${EXPAT_INSTALL_PREFIX} \
		--host=arm-linux \
		--disable-static \
		--enable-shared $(SQUASH_OUTPUT); \
	fi
	@echo "================ Compiling EXPAT"
	@${MAKE} ${MAKE_OPTIONS} -C  ${EXPAT_TAR_PATH}/${EXPAT_NAME} ${MAKE_SUFFIX} $(SQUASH_OUTPUT)
	@${MAKE} -C ${EXPAT_TAR_PATH}/${EXPAT_NAME} install ${MAKE_SUFFIX} $(SQUASH_OUTPUT)
	@echo "================ EXPAT Compiled!"

clean:
	@echo "================ MAKE EXPAT CLEAN"
	@rm -rf ${EXPAT_TAR_PATH}/${EXPAT_NAME}
	@rm -rf .$(EXPAT_TAR_PATH)/$(EXPAT_NAME)
	@echo "================ Done"
