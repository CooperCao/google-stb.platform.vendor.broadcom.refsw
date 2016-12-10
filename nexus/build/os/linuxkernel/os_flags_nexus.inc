############################################################
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
############################################################


ifneq ($(filter ${B_REFSW_ARCH}, aarch64-linux arm-linux), ${B_REFSW_ARCH})
# Set these flags for MIPS only. ARM uses Kbuild.
B_REFSW_OPTIMIZER_LEVEL ?= s
ifeq ($(B_REFSW_OPTIMIZER_LEVEL),0)
$(error B_REFSW_OPTIMIZER_LEVEL=0 not supported for linuxkernel)
endif
NEXUS_CFLAGS += -O$(B_REFSW_OPTIMIZER_LEVEL)
ifeq ($(B_REFSW_DEBUG),y)
NEXUS_CFLAGS += -g
endif
endif

NEXUS_OBJ_SUFFIX := o
MAGNUM_LIB := $(NEXUS_OBJ_DIR)/libmagnum.a
ifeq ($(B_REFSW_SHAREABLE),n)
NEXUS_STATIC_LIB := $(NEXUS_OBJ_DIR)/libnexus_driver.a
ifeq ($(NEXUS_MODE),driver)
NEXUS_DRIVER := $(NEXUS_OBJ_DIR)/nexus.ko
endif
else
NEXUS_STATIC_LIB := $(NEXUS_OBJ_DIR)/libnexus.a
NEXUS_DRIVER := $(NEXUS_OBJ_DIR)/nexus.ko
endif
NEXUS_CFLAGS += -DKBUILD_MODNAME=\"nexus\"

NEXUS_MAGNUM_PRECOMPILED_HEADER_H := $(NEXUS_OBJ_DIR)/MAGNUM/$(notdir $(NEXUS_MAGNUM_PRECOMPILED_HEADER))
NEXUS_PRECOMPILED_HEADER_H := $(NEXUS_OBJ_DIR)/MAGNUM/$(notdir $(NEXUS_PRECOMPILED_HEADER))

# remove unused code to reduce size
# TODO: this is stripping the "config" insmod param. also, this will not work for drivers without the proxy. so we need to use some build param.
#NEXUS_CFLAGS += -fdata-sections -ffunction-sections
#NEXUS_LDFLAGS += --gc-sections --entry=init_module --undefined=cleanup_module
