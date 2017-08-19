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

THIS_DIR := BSEAV/opensource/dtc
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${BSEAV_TOP}/opensource/dtc/dtc.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

.PHONY: all clean

${DTC_SOURCE_DIRECTORY}/extracted:
	@mkdir -p ${DTC_SOURCE_DIRECTORY}
	@B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${DTC_BUILD_DIR}/${DTC_NAME}.info ${DTC_SOURCE_DIRECTORY}
ifneq ($(BDTC_OBJ_DIR),)
	@mkdir -p ${BDTC_OBJ_DIR}/${DTC_NAME}
	@tar -xzf ${DTC_SOURCE_DIRECTORY}/${DTC_SOURCE_TARBALL} -C ${BDTC_OBJ_DIR} ${DTC_NAME}/libfdt
	@cp -f $(DTC_BUILD_DIR)/dtc.inc $(BDTC_OBJ_DIR)
	@cd $(BDTC_OBJ_DIR) && patch -p0 < $(DTC_BUILD_DIR)/$(DTC_NAME).patch
endif
	@touch $@

all : ${DTC_SOURCE_DIRECTORY}/extracted

clean :
	@rm -rf ${DTC_SOURCE_DIRECTORY}
ifneq ($(BDTC_OBJ_DIR),)
	@rm -rf ${BDTC_OBJ_DIR}/dtc.inc ${BDTC_OBJ_DIR}/${DTC_NAME}
endif
