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
ifndef B_OPENSSL_TARGETS_DEFINED
B_OPENSSL_TARGETS_DEFINED := y

ifndef BSEAV
$(error BSEAV must be defined)
endif
ifndef NEXUS_TOP
$(error NEXUS_TOP must be defined)
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
include $(BSEAV)/lib/openssl/openssl.inc

B_OPENSSL_BUILD_ENABLED = y
ifeq ($(SSL_SUPPORT),n)
B_OPENSSL_BUILD_ENABLED = n
endif
ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
B_OPENSSL_BUILD_ENABLED = n
endif

NEXUS_BIN_DIR ?= $(B_REFSW_OBJ_ROOT)/nexus/bin
#$(error NEXUS_BIN_DIR=$(NEXUS_BIN_DIR))

.PHONY: b_openssl_target_dummy b_openssl_target_build b_openssl_target_clean

b_openssl_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

# There is a very subtle race condition in the openssl build with a parallel make that happens about once
# in every 200 runs while building concurrently with other things (like nexus).  This inhibits a parallel build
# until we can identify and resolve the issue.
.NOTPARALLEL: b_openssl_target_build
b_openssl_target_build: $(B_OPENSSL_OTHER_BUILDS)
        ifeq ($(B_OPENSSL_BUILD_ENABLED),y)
		$(Q_)if ! [ -f $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.so ] || ! [ -f $(B_OPENSSL_LIB_INSTALL_DIR)/libssl.so ]; then \
			echo "[Build... OpenSSL]" && \
			$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/openssl $(sort $(B_OPENSSL_TARGET_MAKEFLAGS)); \
		fi
		@echo "[Install... OpenSSL]"
		$(Q_)mkdir -p $(NEXUS_BIN_DIR)
		$(Q_)cp -fP $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.so*   $(NEXUS_BIN_DIR)
		$(Q_)cp -fP $(B_OPENSSL_LIB_INSTALL_DIR)/libssl.so*      $(NEXUS_BIN_DIR)
        endif

b_openssl_target_clean: $(B_OPENSSL_OTHER_CLEANS)

b_openssl_target_clean_all: b_openssl_target_clean $(B_OPENSSL_OTHER_CLEANS)
        ifeq ($(B_OPENSSL_BUILD_ENABLED),y)
		@echo [Clean..... openssl];
		$(Q_)$(MAKE) -C $(BSEAV)/lib/openssl clean $(sort $(B_OPENSSL_TARGET_MAKEFLAGS))
		$(Q_)$(MAKE) -C $(BSEAV)/lib/openssl target-clean $(sort $(B_OPENSSL_TARGET_MAKEFLAGS))
        endif

ifeq ($(B_OPENSSL_BUILD_ENABLED),y)
B_OPENSSL_BUILD_FILES += $(addprefix $(B_OPENSSL_LIB_INSTALL_DIR)/, libcrypto.so* libssl.so*)
endif

endif #B_OPENSSL_TARGETS_DEFINED
