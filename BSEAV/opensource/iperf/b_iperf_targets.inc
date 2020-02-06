###############################################################################
#   Copyright (C) 2019 Broadcom.
#   The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to
#   the terms and conditions of a separate, written license agreement executed
#   between you and Broadcom (an "Authorized License").  Except as set forth in
#   an Authorized License, Broadcom grants no license (express or implied),
#   right to use, or waiver of any kind with respect to the Software, and
#   Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#   THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#   IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization,
#   constitutes the vaiperfble trade secrets of Broadcom, and you shall use all
#   reasonable efforts to protect the confidentiality thereof, and to use this
#   information only in connection with your use of Broadcom integrated circuit
#   products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#   OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#   RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#   IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#   A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#   THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#   OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#   INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#   RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#   EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#   WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#   FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################
ifndef B_IPERF_TARGETS_DEFINED
B_IPERF_TARGETS_DEFINED := y

ifndef BSEAV
$(error BSEAV must be defined)
endif

.PHONY: b_iperf_target_dummy b_iperf_target_build b_iperf_target_clean

b_iperf_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

b_iperf_target_build: $(B_IPERF_OTHER_BUILDS)
	$(Q_)$(MAKE) -C $(BSEAV)/opensource/iperf $(sort $(B_IPERF_TARGET_MAKEFLAGS))

b_iperf_target_clean: $(B_IPERF_OTHER_CLEANS)

b_iperf_target_clean_all: b_iperf_target_clean $(B_IPERF_OTHER_CLEAN_ALL)
	$(Q_)$(MAKE) -C $(BSEAV)/opensource/iperf clean $(sort $(B_IPERF_TARGET_MAKEFLAGS))

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
### B_IPERF_BUILD_FILES += $(B_REFSW_OBJ_ROOT)/BSEAV/opensource/iperf/iperf-5.2.4/install/libiperf.a

endif #B_IPERF_TARGETS_DEFINED
