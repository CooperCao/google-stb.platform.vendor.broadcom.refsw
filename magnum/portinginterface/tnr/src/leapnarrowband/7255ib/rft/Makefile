#############################################################################
# Copyright (C) 2018 Broadcom.
# The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to
# the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied),
# right to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#############################################################################

.PHONY: all clean

ifeq ($(BRFT_OBJ_DIR),)
$(error BRFT_OBJ_DIR must be set by calling makefile)
endif

ifeq ($(BRFT_OBJ_SUFFIX),)
BRFT_OBJ_SUFFIX ?= o
endif

MAGNUM ?= ../../../../../../../magnum
ROCKFORD ?= rockford
BRFT_ROOT ?= $(ROCKFORD)/third_party/rft
$(info MAGNUM=$(MAGNUM))
$(info ROCKFORD=$(ROCKFORD))

BRFT_ROOT_ORIGIN := $(BRFT_ROOT)
BRFT_ROOT_COPIED := $(BRFT_OBJ_DIR)/rft/copied
BRFT_ROOT_PATCHED := $(BRFT_OBJ_DIR)/rft/patched
BRFT_REF_PATCH := $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/7255ib/rft/brafael.patch

BRFT_LIB_DEP := $(BRFT_ROOT_PATCHED)/done

BRFT_LIB_INCLUDES := $(BRFT_ROOT_PATCHED)/rft $(BRFT_ROOT_PATCHED)/rft/Tuner
BRFT_LIB_SOURCES := $(BRFT_ROOT_PATCHED)/rft/I2C_Sys.c $(BRFT_ROOT_PATCHED)/rft/Tuner/R642.c
BRFT_LIB_OBJS := $(addprefix $(BRFT_OBJ_DIR)/,$(notdir $(patsubst %.c,%.$(BRFT_OBJ_SUFFIX),$(BRFT_LIB_SOURCES))))

all: $(BRFT_OBJ_DIR)/brft.inc

$(BRFT_OBJ_DIR)/brft.inc: $(BRFT_LIB_DEP)
	@printf 'BRFT_LIB_INCLUDES := $(BRFT_LIB_INCLUDES)\n' > $@
	@printf 'BRFT_LIB_SOURCES := $(BRFT_LIB_SOURCES)\n' >> $@
	@printf 'BRFT_LIB_OBJS := $(BRFT_LIB_OBJS)\n' >> $@

$(BRFT_ROOT_COPIED)/exists:
	@mkdir -p $(BRFT_ROOT_COPIED)/exists

$(BRFT_ROOT_PATCHED)/exists:
	@mkdir -p $(BRFT_ROOT_PATCHED)/exists

$(BRFT_ROOT_COPIED)/done: $(BRFT_ROOT_COPIED)/exists $(BRFT_ROOT_ORIGIN)
	@echo "[Copy Rafael Tuner files ...]"
	@cp $(BRFT_ROOT_ORIGIN)/R642_API_*_Multi.rar $(BRFT_ROOT_COPIED)
	@cd $(BRFT_ROOT_COPIED) && 7z x -aoa R642_API_*_Multi.rar && mv R642_API_*_Multi rft
	@cp -af $(BRFT_REF_PATCH) $(BRFT_ROOT_COPIED)
	@mkdir -p $(BRFT_ROOT_COPIED)/done

$(BRFT_ROOT_PATCHED)/done: $(BRFT_ROOT_COPIED)/done $(BRFT_ROOT_PATCHED)/exists
	@echo "[Patch Rafael Tuner reference files ...]"
	@cd $(BRFT_ROOT_COPIED)/rft && patch -p1 -i ../brafael.patch > ../patch.log 2>&1
	@cp -af $(BRFT_ROOT_COPIED)/* $(BRFT_ROOT_PATCHED)
	@echo "[Replacing C++ comments with C Style comments ...]"
	@cd $(BRFT_ROOT_PATCHED)/rft && find . -name "*.c" -exec sed -i 's#//\(.*\)#/*\1 */#' {} \; && find . -name "*.h" -exec sed -i 's#//\(.*\)#/*\1 */#' {} \;
	@mkdir -p $(BRFT_ROOT_PATCHED)/done

clean:
	-rm -rf $(BRFT_ROOT_PATCHED) $(BRFT_ROOT_COPIED) /* TODO: also remove the inc file */
