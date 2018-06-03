############################################################################
# Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

B_REFSW_TOP ?= $(realpath ../../..)
NEXUS_TOP   ?= $(B_REFSW_TOP)/nexus

B_REFSW_OBJ_DIR ?= obj.$(NEXUS_PLATFORM).$(BCHP_VER)

include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include flite.inc

TARGET_ROOT ?= $(realpath $(B_REFSW_OBJ_ROOT)/target/usr/local)

CONFIGURE_OPTIONS := \
  --host=$(B_REFSW_ARCH) \
  --prefix=$(TARGET_ROOT) \
  --disable-shared \
  --disable-sockets \
  CFLAGS=-fPIC \

.PHONY: all clean reallyclean
.PHONY: configure clone

all: install

install: ${FLITE_OUT_DIRECTORY}
	install --compare --verbose --target-directory=$(TARGET_ROOT)/lib/ $(wildcard ${FLITE_OUT_DIRECTORY}/*)

clone: $(FLITE_SOURCE_PATH)/configure

$(FLITE_SOURCE_PATH)/configure:
	echo "================ Cloning FLITE to ${FLITE_SOURCE_PATH}"; \
	mkdir -p ${FLITE_SOURCE_PATH}; \
	git clone ${FLITE_GIT_URI} ${FLITE_SOURCE_PATH}; \

configure: $(FLITE_SOURCE_PATH)/config.status

$(FLITE_SOURCE_PATH)/config.status: $(FLITE_SOURCE_PATH)/configure
	echo "================ Configuring with $(CONFIGURE_OPTIONS)"; \
	(cd $(FLITE_SOURCE_PATH); ./configure $(CONFIGURE_OPTIONS)); \

${FLITE_OUT_DIRECTORY}: $(FLITE_SOURCE_PATH)/config.status
	$(MAKE) -C $(FLITE_SOURCE_PATH)

clean:
	-$(MAKE) -C $(FLITE_SOURCE_PATH) clean

reallyclean: clean
	-rm -rf $(FLITE_SOURCE_PATH)
