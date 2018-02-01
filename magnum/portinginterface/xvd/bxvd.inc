############################################################
# Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
# Module Description:
#
############################################################
#
# Conditional Options for this Makefile
#
# BXVD_IMG_ONLY - If defined, only the firmware image related code will be compiled.
# BXVD_IMG_EXTERNAL - If defined, firmware related code (interface AND context) will NOT be compiled. Also defines -DBXVD_USE_CUSTOM_IMAGE.  The app is expected to specify their own firmware interface AND image context when calling BXVD_Open().
# BXVD_USE_CUSTOM_CONTEXT - If defined, the firmware image context will NOT be compiled. The app is expected to provide their own firmware image context when calling BXVD_Open().
# BXVD_USE_DM_V1 - If defined, Display Manager (DM) version 1.x will be used.
# BXVD_POWER_MANAGEMENT - Defaults to 'y' (yes) on DTV platforms, to 'n' (no) used to enable dynamic power management feature
# BXVD_FW_DEBUG_DRAM_LOGGING - If 'y' (yes), starts AVD FW DRAM logging and reads and prints out the debug log during picture data ready_isr
# BXVD_FLATTEN_USERDATA - If 'y' (yes), the userdata code will coalesce userdata packets otherwise, they will be delivered singly.
# BXVD_BREAK_ON_TYPE_CHANGE - If 'y' (yes), the userdata packet whose UD type is different from other packets in a group will be delivered separately from coalesced packets of the same type. Must be used in conjunction with BXVD_FLATTEN_USERDATA=1
#

# Define this module
MAGNUM_MODULES += BXVD

#
# When true, select the BDBG_INSTANCE_* macros, otherwise use BDBG_*
#
ifeq ($(BXVD_USE_INSTANCE_MACROS),y)
BXVD_DEFINES += BXVD_P_USE_INSTANCE_MACROS
endif

#
# When true, coalesce userdata packets, otherwise they are delivered singly
#
ifeq ($(BXVD_FLATTEN_USERDATA),y)
BXVD_DEFINES += BXVD_FLATTEN_USERDATA
endif

#
# When true, along with BXVD_FLATTEN_USERDATA, a change in userdata type will cause packet coalescing to stop
#
ifeq ($(BXVD_BREAK_ON_TYPE_CHANGE),y)
BXVD_DEFINES += BXVD_BREAK_ON_TYPE_CHANGE
endif

#
# When true, this will cause a live dump of userdata packets. Use with caution as this will have
# a negative effect on packet timing.
#
ifeq ($(BXVD_USERDATA_EXTRA_DEBUG),y)
BXVD_DEFINES += BXVD_USERDATA_EXTRA_DEBUG
endif

ifeq ($(BXVD_P_USE_BINARY_IMAGE),y)
BXVD_DEFINES += BXVD_P_USE_BINARY_IMAGE
endif

ifeq ($(BXVD_ENABLE_DRAM_PERF_INFO),y)
BXVD_DEFINES += BXVD_P_ENABLE_DRAM_PREF_INFO
endif

#
# When true, pass the heap handle to XDM.  XDM will then allocate a BDBG_Fifo.
#
ifeq ($(BXVD_ENABLE_BDBG_FIFO),y)
BXVD_DEFINES += BXVD_ENABLE_BDBG_FIFO
endif

include $(MAGNUM)/portinginterface/xvd/bxvd_chip.inc

#
# Power management is enabled by default on DTV chips
#
ifeq ($(findstring $(BCHP_CHIP), 3548 3556 35233), $(BCHP_CHIP))
ifneq ($(BXVD_POWER_MANAGEMENT),n)
BXVD_DEFINES += BXVD_P_POWER_MANAGEMENT
endif
endif

#
# Power management is disabled by default on certain platforms
#

ifneq ($(findstring $(BCHP_CHIP), 35125 35230 7135 7325 7336 7340 7342 7401 7403 7440 7550 7601 7630 7635 7640), $(BCHP_CHIP))
ifeq ($(BXVD_POWER_MANAGEMENT),y)
BXVD_DEFINES += BXVD_P_POWER_MANAGEMENT
endif
endif

#
# Force the SVD picture buffers to be on MEMC0
#
ifeq ($(BXVD_SVD_USE_MEMC0),y)
BXVD_DEFINES += BXVD_P_SVD_USE_MEMC0
endif

#
# For MVC, set the pNext on the older DVD platforms.
#
ifeq ($(findstring $(BCHP_CHIP), 7630 7631 7632 7633 7634), $(BCHP_CHIP))
ifneq ($(BXVD_LEGACY_MVC_SUPPORT),n)
BXVD_DEFINES += BXVD_LEGACY_MVC_SUPPORT
endif
endif

#
# Enable FW DRAM Log output to be read and printed in PDR_isr()
#
ifeq ($(BXVD_FW_DEBUG_DRAM_LOGGING),y)
BXVD_DEFINES += BXVD_P_FW_DEBUG_DRAM_LOGGING
endif

#
# Enable PPB grab mode
#
ifeq ($(BXVD_DM_ENABLE_PPB_GRAB_MODE),y)
BXVD_DEFINES += BXVD_DM_ENABLE_PPB_GRAB_MODE
endif

#### Use DMv2 by default for all platforms
BXVD_DM_VERSION = 2

ifeq ($(BXVD_USE_DM_V1),)
else
BXVD_DM_VERSION = 1
endif

#### Module Includes ####
BXVD_INCLUDES += $(MAGNUM)/portinginterface/xvd/include

#### Source file(s) ####
ifeq ($(BXVD_IMG_ONLY),)
#### Common file(s) ####
BXVD_SOURCES += \
	$(MAGNUM)/portinginterface/xvd/src/bxvd.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_devcmds_priv.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_relf.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_priv.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_reg.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_userdata.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_pvr.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_intr.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_memory_priv.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_status.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_dbg.c

#### Core file(s) ####
BXVD_SOURCES += \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_core_avd_reve0.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_core_avd_revk0.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_core_avd_revn0.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_core_avd_revt0.c

#### Platform specific file(s) ####
ifeq ($(BXVD_DM_VERSION),2)
BXVD_SOURCES += \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_dip.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_decoder.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_decoder_timer.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_decoder_dbg.c
endif

BXVD_SOURCES += \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_platform_$(BXVD_CHIP).c

endif # BXVD_IMG_ONLY

#### Firmware image file(s) ####
ifeq ($(BXVD_IMG_EXTERNAL),)
# Common image file(s)
BXVD_SOURCES +=	\
        $(MAGNUM)/portinginterface/xvd/src/bxvd_image.c

ifeq ($(BXVD_USE_CUSTOM_CONTEXT),)
# Platform image file(s)
BXVD_SOURCES += \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_platform_$(BXVD_CHIP)_image.c
endif # !BXVD_USE_CUSTOM_CONTEXT

else # !BXVD_USE_CUSTOM_IMAGE
BXVD_DEFINES += BXVD_USE_CUSTOM_IMAGE
endif
