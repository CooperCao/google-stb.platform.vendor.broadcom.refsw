############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#
############################################################
ifndef B_FREETYPE_TARGETS_DEFINED
B_FREETYPE_TARGETS_DEFINED := y

ifndef BSEAV
$(error BSEAV must be defined)
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
include $(BSEAV)/opensource/freetype/freetype.inc

ifeq ($(VERBOSE),)
B_FREETYPE_TARGET_MAKEFLAGS += -s
endif

.PHONY: b_freetype_target_dummy b_freetype_target_build b_freetype_target_clean

b_freetype_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

b_freetype_target_build: $(B_FREETYPE_OTHER_BUILDS)
ifeq ($(SYSTEM),linux)
	@if ! $(MAKE) -q -C $(FREETYPE_DIR) all ; then \
		echo [Build..... freetype]; \
		$(MAKE) -C $(FREETYPE_DIR) $(sort $(B_FREETYPE_TARGET_MAKEFLAGS)) all ; \
	fi
else
	echo [Build..... freetype];
	$(MAKE) -C $(FREETYPE_DIR) $(B_FREETYPE_TARGET_MAKEFLAGS)
endif

b_freetype_target_clean: $(B_FREETYPE_OTHER_CLEANS)

b_freetype_target_clean_all: b_freetype_target_clean $(B_FREETYPE_OTHER_CLEAN_ALL)
	@echo [Clean..... freetype];
	$(Q_)$(MAKE) -C $(FREETYPE_DIR) clean  $(sort $(B_FREETYPE_TARGET_MAKEFLAGS))

B_FREETYPE_BUILD_FILES += $(FREETYPE_LIB_FOLDER)/libfreetype.a

endif #B_FREETYPE_TARGETS_DEFINED
