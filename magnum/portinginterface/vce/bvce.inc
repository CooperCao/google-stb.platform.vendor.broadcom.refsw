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
#
# [File Description:]
#
#############################################################################

############################################################
#
# Conditional Options for this Makefile
#
# BVCE_IMG_ONLY - If defined, only the firmware image related code will be compiled.
# BVCE_IMG_EXTERNAL - If defined, firmware related code (interface AND context) will NOT be compiled. Also defines -DBVCE_USE_CUSTOM_IMAGE.  The app is expected to specify their own firmware interface AND image context when calling BVCE_Open().
# BVCE_USE_CUSTOM_CONTEXT - If defined, the firmware image context will NOT be compiled. The app is expected to provide their own firmware image context when calling BVCE_Open().
# BVCE_IMG_FILE - If defined, firmware will be loaded from files: bvce_fw_image_[picarc/mbarc].bin OR the file(s) is specified in the environment variables: BVCE_IMG_PICARC and/or BVCE_IMG_MBARC

# Define this module
MAGNUM_MODULES += BVCE

#
# Select VCE CHIP family
#
BVCE_CHIP = $(BCHP_CHIP)

#### Module Includes ####
BVCE_INCLUDES += \
	$(MAGNUM)/portinginterface/vce/include \
	$(MAGNUM)/portinginterface/vce/src

#### Source File(s) ####
ifeq ($(BVCE_IMG_ONLY),)
#### Common File(s) ####
BVCE_SOURCES_LOCAL += \
             $(MAGNUM)/portinginterface/vce/src/bvce.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_platform.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_platform_$(BVCE_CHIP).c \
             $(MAGNUM)/portinginterface/vce/src/bvce_fw_api_common.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_telem.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_output.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_power.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_debug_priv.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_core.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_buffer.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_debug_common.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_image_common.c

#### Core File(s) ####

#### Platform File(s) ####

endif # BVCE_IMG_ONLY

#### Firmware image file(s) ####
ifeq ($(BVCE_IMG_FILE),)
ifeq ($(BVCE_IMG_EXTERNAL),)
# Common image file(s)
BVCE_SOURCES_LOCAL +=	\
             $(MAGNUM)/portinginterface/vce/src/bvce_image.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_image_common.c

ifeq ($(BVCE_USE_CUSTOM_CONTEXT),)
# Platform image file(s)
BVCE_SOURCES_LOCAL += \
             $(MAGNUM)/portinginterface/vce/src/bvce_core_image.c
endif # !BVCE_USE_CUSTOM_CONTEXT

else # !BVCE_USE_CUSTOM_IMAGE
BVCE_DEFINES += BVCE_USE_CUSTOM_IMAGE
endif #!BVCE_IMG_EXTERNAL
else
# Common image file(s)
BVCE_SOURCES_LOCAL +=	\
             $(MAGNUM)/portinginterface/vce/src/bvce_image_file.c
BVCE_DEFINES += BVCE_USE_FILE_IMAGE=1
endif

BVCE_DEBUG_INCLUDES += \
    $(MAGNUM)/portinginterface/vce/include \
    $(MAGNUM)/portinginterface/vce/src

BVCE_DEBUG_SOURCES += \
	$(MAGNUM)/portinginterface/vce/src/bvce_debug.c

# DEBUG Build Flags
ifeq ($(BVCE_DUMP_OUTPUT_CDB),y)
BVCE_DEFINES += BVCE_P_DUMP_OUTPUT_CDB=1
endif
ifeq ($(BVCE_DUMP_OUTPUT_ITB),y)
BVCE_DEFINES += BVCE_P_DUMP_OUTPUT_ITB=1
endif
ifeq ($(BVCE_DUMP_OUTPUT_ITB_DESC),y)
BVCE_DEFINES += BVCE_P_DUMP_OUTPUT_ITB_DESC=1
endif
ifeq ($(BVCE_DUMP_MAU_PERFORMANCE_DATA),y)
BVCE_DEFINES += BVCE_P_DUMP_MAU_PERFORMANCE_DATA=1
endif
ifeq ($(BVCE_ENABLE_UART),y)
BVCE_DEFINES += BVCE_P_ENABLE_UART=1
endif
ifeq ($(BVCE_ENABLE_UART),1)
BVCE_DEFINES += BVCE_P_ENABLE_UART=1
endif
ifeq ($(BVCE_ENABLE_UART),2)
BVCE_DEFINES += BVCE_P_ENABLE_UART=2
endif
ifeq ($(BVCE_DUMP_ARC_DEBUG_LOG),y)
BVCE_DEFINES += BVCE_P_DUMP_ARC_DEBUG_LOG=1
endif
ifeq ($(BVCE_DUMP_USERDATA_LOG),y)
BVCE_DEFINES += BVCE_P_DUMP_USERDATA_LOG=1
endif
ifeq ($(BVCE_TEST_MODE),y)
BVCE_DEFINES += BVCE_P_TEST_MODE=1
endif
ifeq ($(BVCE_PRERELEASE_TEST_MODE),y)
BVCE_DEFINES += BVCE_P_PRERELEASE_TEST_MODE=1
endif
ifeq ($(BVCE_ASSERT_ON_WATCHDOG),y)
BVCE_DEFINES += BVCE_P_ASSERT_ON_WATCHDOG=1
endif

# Remove duplicate sources to fix duplicate linker errors
BVCE_SOURCES += $(sort $(BVCE_SOURCES_LOCAL))
