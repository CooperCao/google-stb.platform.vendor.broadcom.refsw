###############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################
# Makefile for bip_play util

B_THIS_DIR:=nexus/lib/bip/utils

ifdef B_REFSW_REAL_NEXUS_TOP
    NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

NXUTILS_LIB := nxutils_lib.a
LDFLAGS += $(NXUTILS_LIB)
EXTRA_OBJS = cmd_parsing.o gui.o $(NXUTILS_LIB)

BIP_CLIENT_APP := true
THEAPPS += bip_play

include ${NEXUS_TOP}/lib/bip/utils/Makefile.inc

# nexus/nxclient/apps/utils
include $(NEXUS_TOP)/nxclient/apps/utils/nxutils.inc
#NXUTILS_LIB_SRC := $(foreach src,$(NXUTILS_SOURCES),$(if $(findstring nexus/nxclient/apps/utils,$(src)),$(notdir $(src))))
NXUTILS_LIB_SRC:= bfont.c bgui.c binput.c picdecoder.c
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/nxclient/apps.$(NEXUS_MODE)
CFLAGS += $(addprefix -I,$(NXUTILS_INCLUDES)) -I$(BINDIR) -g

$(BINDIR)/exists:
	@make -p $@

$(NXUTILS_LIB): $(patsubst %.c,$(BINDIR)/%.o,$(NXUTILS_LIB_SRC))
	@echo [Archive... $@]
	@$(AR) rcs $@ $(filter %.o, $^)

$(BINDIR)/%.o: $(NEXUS_TOP)/nxclient/apps/utils/%.c
	@echo [Compile... $(notdir $<)]
	@$(CC) -c -o $@ $< $(CFLAGS)

# Install images and fonts
bip_play: $(NEXUS_BIN_DIR)/images $(NEXUS_BIN_DIR)/fonts

$(NEXUS_BIN_DIR)/images:
	@mkdir -p $@
	-@cp -rf $(NEXUS_TOP)/../$(B_THIS_DIR)/images/* $@/

$(NEXUS_BIN_DIR)/fonts:
	@mkdir -p $@
	-@cp -rf $(NEXUS_TOP)/../$(B_THIS_DIR)/fonts/* $@/
