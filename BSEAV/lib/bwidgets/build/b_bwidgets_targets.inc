############################################################
#  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
ifndef B_BWIDGETS_TARGETS_DEFINED
B_BWIDGETS_TARGETS_DEFINED := y

ifndef BSEAV
$(error BSEAV must be defined)
endif

.PHONY: b_bwidgets_target_dummy b_bwidgets_target_build b_bwidgets_target_clean

include $(BSEAV)/lib/bwin/build/b_bwin_targets.inc
B_BWIDGETS_OTHER_BUILDS += b_bwin_target_build
B_BWIDGETS_OTHER_CLEANS += b_bwin_target_clean
B_BWIDGETS_OTHER_CLEAN_ALL += b_bwin_target_clean_all
B_BWIDGETS_BUILD_FILES += $(B_BWIN_BUILD_FILES)

b_bwidgets_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

b_bwidgets_target_build: $(B_BWIDGETS_OTHER_BUILDS)
	@echo [Build..... bwidgets]
	$(Q_)$(MAKE) -C $(BSEAV)/lib/bwidgets/build $(sort $(B_BWIDGETS_TARGET_MAKEFLAGS))

b_bwidgets_target_clean: $(B_BWIDGETS_OTHER_CLEANS)
	@echo [Clean..... bwidgets]
	$(Q_)$(MAKE) -C $(BSEAV)/lib/bwidgets/build clean $(sort $(B_BWIDGETS_TARGET_MAKEFLAGS))

b_bwidgets_target_clean_all: b_bwidgets_target_clean $(B_BWIDGETS_OTHER_CLEAN_ALL)

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
B_BWIDGETS_BUILD_FILES += $(B_REFSW_OBJ_ROOT)/BSEAV/lib/bwidgets/libbwidgets.a $(B_BWIN_BUILD_FILES)

endif #B_BWIDGETS_TARGETS_DEFINED
