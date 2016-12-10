##############################################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
##############################################################################

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

include SignParams.inc

REFSW_TOP = ./../../../..

export NEXUS_TOP=$(REFSW_TOP)/nexus
export MAGNUM_TOP=$(REFSW_TOP)/magnum

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
include $(NEXUS_TOP)/platforms/common/build/platform_version.inc
include $(MAGNUM_TOP)/portinginterface/hsm/bhsm_defs.inc

BSEAV = $(REFSW_TOP)/BSEAV
include $(BSEAV)/build/refsw_inc.mak
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
OUT = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/fw_img_sign/$(B_REFSW_ARCH)

#defines for compiling signing tool
CC=gcc
SRC=src
SRCS= AutomatedSigning.c GenerateFiles.c
OBJS = $(SRCS:%.c=$(OUT)/%.o)

CFLAGS=-DBHSM_ZEUS_VER_MAJOR=$(BHSM_ZEUS_VER_MAJOR) -DBHSM_ZEUS_VER_MINOR=$(BHSM_ZEUS_VER_MINOR)
LDFLAGS=-lcrypto

#defines for perl script and generation of signing tool command file
export REFSW_VERSION=$(NEXUS_PLATFORM_VERSION_MAJOR).$(NEXUS_PLATFORM_VERSION_MINOR)
ifdef BCHP_CHIP
export BCHP_ID=$(BCHP_CHIP)
else
export BCHP_ID=$(patsubst 9%,%,${NEXUS_PLATFORM})
endif

export OUT_DIR = $(OUT)
export FW_IMAGE_DIR = $(OUT)

KEY_DIR=test_key
export PRV_KEY=$(KEY_DIR)/brcm_test_fwkey2.pem
export PUB_KEY=$(KEY_DIR)/brcm_test_fwkey2_pub.pem
export SS_PUB_KEY=$(OUT_DIR)/$(BCHP_ID)_ss_fw_key2.pem

export SID_BIN=$(FW_IMAGE_DIR)/fw_img_dump_$(BCHP_ID)_sid.bin
export RAAGA_BIN=$(FW_IMAGE_DIR)/fw_img_dump_$(BCHP_ID)_dsp.bin
export RAVE_BIN=$(FW_IMAGE_DIR)/fw_img_dump_$(BCHP_ID)_xpt_rave.bin
export VCE_PIC_BIN=$(FW_IMAGE_DIR)/fw_img_dump_$(BCHP_ID)_vce_picarc.bin
export VCE_MB_BIN=$(FW_IMAGE_DIR)/fw_img_dump_$(BCHP_ID)_vce_mbarc.bin
export AVD_OL_BIN=$(FW_IMAGE_DIR)/fw_img_dump_$(BCHP_ID)_xvd_outer.bin
export AVD_IL_BIN=$(FW_IMAGE_DIR)/fw_img_dump_$(BCHP_ID)_xvd_inner.bin

ifeq ($(target),ZD)
    #This Key variant will be used to sign firmware images. This is Key2.
    FW_KEY_VARIANT=_Prod_Key2
    #FW_KEY_VARIANT=_prod_key0

    #This Key Variant will be used to sign Key2.
    KEY2_KEY_VARIANT=ZS_dev_key0
#else ifeq ($(target),ZB)
else
    #This Key variant will be used to sign firmware images. This is Key2.
    FW_KEY_VARIANT=_Prod_Key2
    #FW_KEY_VARIANT=_prod_key0

    #This Key Variant will be used to sign Key2.
    KEY2_KEY_VARIANT=_prod_key0
endif

export BINS_TO_SIGN = $(OUT_DIR)/$(BCHP_ID)_bins_to_sign.txt
BOLT_BINS_TO_SIGN = $(OUT_DIR)/$(BCHP_ID)_bolt_bins_to_sign.txt
export AVS_BIN=$(OUT_DIR)/$(BCHP_ID)_avs_to_sign.bin
export MEMSYS_BIN=$(OUT_DIR)/$(BCHP_ID)_memsys_to_sign.bin

export KEY2_COMMAND_FILE = $(OUT_DIR)/$(BCHP_ID)_fw_key2.in
export KEY2_SIG_COMMAND_FILE = $(OUT_DIR)/$(BCHP_ID)_fw_key2_signature.in
export KEY2_BIN=$(BCHP_ID)_fw_key2.bin

export SIGNATURE_FILE_NAME = nexus_security_regver_signatures.h
export KEY2_FILE_NAME = nexus_security_regver_key.h

IMAGE_DUMP_DIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/fw_img_dump/$(DEBUG_SUFFIX)
IMAGE_DUMP_BIN = $(IMAGE_DUMP_DIR)/fw_img_dump
IMAGE_DUMP_CMD = cd $(IMAGE_DUMP_DIR) && ./fw_img_dump # fw_img_dump outputs in current working directory
IMAGE_MOVE = mv $(IMAGE_DUMP_DIR)/*.bin $(FW_IMAGE_DIR)/

COMMAND_FILE = $(OUT_DIR)/$(BCHP_ID).in
SIG_COMMAND_FILE = $(OUT_DIR)/$(BCHP_ID)_signature.in

KEY2_FILE=$(NEXUS_BUILD_DIR)/nexus_security_regver_key.inc
KEY2_COPY = cp $(OUT_DIR)/$(KEY2_FILE_NAME) $(KEY2_FILE)
STOCK_KEY2 = key2/$(BCHP_ID)_$(KEY2_FILE_NAME)
STOCK_KEY2_COPY ?= cp $(STOCK_KEY2) $(KEY2_FILE)

SIGNATURE_FILE = $(NEXUS_BUILD_DIR)/nexus_security_regver_signatures.inc
SIGNATURE_COPY = cp $(OUT_DIR)/$(SIGNATURE_FILE_NAME) $(SIGNATURE_FILE)

$(OUT)/AutomatedSigning: $(OBJS) prep_dir
	gcc -o $@ $(OBJS) $(LDFLAGS)

$(OUT)/%.o: $(SRC)/%.c $(DEPS) prep_dir
	$(CC) -c  -o $@ $< $(CFLAGS)

${IMAGE_DUMP_BIN}: $(BSEAV)/lib/security/fw_img_dump/fw_img_dump.c
	$(MAKE) -C $(BSEAV)/lib/security/fw_img_dump --no-print-directory

dump_run: prep_dir ${IMAGE_DUMP_BIN}
	${IMAGE_DUMP_CMD}
	${IMAGE_MOVE}

prep_dir:
	mkdir -p $(OUT_DIR)
	mkdir -p $(NEXUS_BUILD_DIR)

sign: dump_run $(OUT)/AutomatedSigning
	perl scripts/sign_command.pl fw
	$(OUT)/AutomatedSigning $(COMMAND_FILE)
	${SIGNATURE_COPY}
	${STOCK_KEY2_COPY}

prod_sign : dump_run $(OUT)/AutomatedSigning
	perl scripts/sign_command.pl pre_process
	$(OUT)/AutomatedSigning $(COMMAND_FILE) $(BINS_TO_SIGN)
	perl scripts/ss_sign_req.pl $(BINS_TO_SIGN) $(FW_KEY_VARIANT)
	perl scripts/sign_command.pl post_process
	$(OUT)/AutomatedSigning $(SIG_COMMAND_FILE)
	${SIGNATURE_COPY}
	perl scripts/sign_command.pl ss_key
	$(OUT)/AutomatedSigning $(KEY2_COMMAND_FILE) $(BINS_TO_SIGN)
	perl scripts/ss_sign_req.pl $(BINS_TO_SIGN) $(KEY2_KEY_VARIANT)
	$(OUT)/AutomatedSigning $(KEY2_SIG_COMMAND_FILE)
	${KEY2_COPY}

prod_fw_sign : dump_run $(OUT)/AutomatedSigning
	perl scripts/sign_command.pl pre_process
	$(OUT)/AutomatedSigning $(COMMAND_FILE) $(BINS_TO_SIGN)
	perl scripts/ss_sign_req.pl $(BINS_TO_SIGN) $(FW_KEY_VARIANT)
	perl scripts/sign_command.pl post_process
	$(OUT)/AutomatedSigning $(SIG_COMMAND_FILE)
	${SIGNATURE_COPY}

key2_sign: prep_dir $(OUT)/AutomatedSigning
	perl scripts/sign_command.pl key
	$(OUT)/AutomatedSigning $(KEY2_COMMAND_FILE) $(BINS_TO_SIGN)
	perl scripts/ss_sign_req.pl $(BINS_TO_SIGN) $(KEY2_KEY_VARIANT)
	$(OUT)/AutomatedSigning $(KEY2_SIG_COMMAND_FILE)
	${KEY2_COPY}

DIR:
	mkdir -p $(OUT)

clean:
	rm -rf $(OUT_DIR)/*
	rm -f $(FW_IMAGE_DIR)/fw_img_dump*
	$(MAKE) -C $(BSEAV)/lib/security/fw_img_dump clean --no-print-directory > /dev/null
