############################################################
#     (c)2003-2013 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
############################################################

#
# Conditional Options for this Makefile
#
# BCHP_CHIP - Chip number (i.e. 7401)
# BCHP_VER - Chip Revision in upper-case (i.e. C2)

######################
# Define this module #
######################
NEXUS_MODULES += CORE

##################
# Module Options #
##################
NEXUS_CORE_OPTIONS := BCHP_CHIP BCHP_VER

###################
# Module Includes #
###################
NEXUS_CORE_PUBLIC_INCLUDES := \
    $(NEXUS_TOP)/modules/core/include \
    $(NEXUS_TOP)/base/include/public
NEXUS_CORE_PRIVATE_INCLUDES = $(NEXUS_TOP)/modules/core/src $(NEXUS_SYNCTHUNK_DIR)

##################
# Module Sources #
##################
NEXUS_CORE_SOURCES := \
	$(NEXUS_TOP)/modules/core/src/nexus_core.c \
	$(NEXUS_TOP)/modules/core/src/nexus_core_utils.c \
	$(NEXUS_TOP)/modules/core/src/nexus_core_convert.c \
	$(NEXUS_TOP)/modules/core/src/nexus_memory.c \
	$(NEXUS_TOP)/modules/core/src/nexus_core_profile.c \
	$(NEXUS_TOP)/modules/core/src/nexus_syslib_framework.c \
	$(NEXUS_TOP)/modules/core/src/nexus_core_img.c \
	$(NEXUS_TOP)/modules/core/src/nexus_watchdog.c

NEXUS_CORE_LOCAL_SOURCES := \
	$(NEXUS_TOP)/modules/core/src/nexus_memory_local.c \
	$(NEXUS_TOP)/modules/core/src/nexus_core_convert_local.c 

NEXUS_CORE_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_core_thunks.c

#
# Power management
#
NEXUS_CORE_SOURCES += $(NEXUS_TOP)/modules/core/src/nexus_power_management_2.c
ifeq ($(NEXUS_POWER_MANAGEMENT),y)
NEXUS_CFLAGS += -DNEXUS_POWER_MANAGEMENT
ifeq ($(BCHP_PWR_SUPPORT),)
BCHP_PWR_SUPPORT=y
endif
else
ifeq ($(BCHP_PWR_SUPPORT),)
BCHP_PWR_SUPPORT=n
endif
endif

ifeq ($(NEXUS_TOUCHPAD_SUPPORT),y)
BCHP_PWR_CTK_SUPPORT=y
else
BCHP_PWR_CTK_SUPPORT=n
endif

##################
# Module Defines #
##################
NEXUS_CORE_DEFINES := BCHP_CHIP=$(BCHP_CHIP) BCHP_VER=BCHP_VER_$(BCHP_VER) BMEM_REENTRANT_CONFIG=BMEM_REENTRANT BINT_REENTRANT_CONFIG=BINT_REENTRANT $(MAGNUM_SYSTEM_DEFINES)


#######################
# Module Dependencies #
#######################
# Base has no module dependencies
NEXUS_CORE_DEPENDENCIES := BASE

##################
# Magnum Modules #
##################
NEXUS_CORE_MAGNUM_MODULES :=  \
    $(MAGNUM)/basemodules/int/bint.inc \
    $(MAGNUM)/basemodules/mma/bmma.inc \
    $(MAGNUM)/basemodules/tmr/btmr.inc \
    $(MAGNUM)/commonutils/avc/bavc.inc \
    $(MAGNUM)/commonutils/fmt/bfmt.inc \
    $(MAGNUM)/commonutils/mrc/bmrc.inc \
    $(MAGNUM)/commonutils/mth/bmth.inc \
    $(MAGNUM)/commonutils/pxl/bpxl.inc \
    $(MAGNUM)/commonutils/sur/bsur.inc \
    $(MAGNUM)/commonutils/img/bimg.inc \
    $(MAGNUM)/commonutils/vlc/bvlc.inc \
    $(MAGNUM)/commonutils/box/bbox.inc \
    $(MAGNUM)/commonutils/tee/btee.inc \
    $(MAGNUM)/syslib/framework/bsyslib_framework.inc

BMMA_USE_STUB?=n
ifeq (${BMMA_USE_STUB}, y)
NEXUS_CORE_DEFINES += BMMA_STUB_BMEM_ALIAS=0
NEXUS_CORE_MAGNUM_MODULES +=  \
    $(MAGNUM)/basemodules/mem/bmem.inc
endif

# Optional profiler support
ifneq ($(NEXUS_MODE),client)
ifeq ($(BPROFILE_SUPPORT),y)
NEXUS_CORE_MAGNUM_MODULES += \
    $(BSEAV)/lib/bprofile/bprofile.inc
# BSEAV makefiles don't include their module names into MAGNUM_MODULES
NEXUS_EXTRA_MODULES += BPROFILE
NEXUS_CORE_DEFINES +=B_HAS_SYMTABLE
ifneq ($(BPROFILE_COUNTERS),)
NEXUS_CORE_DEFINES += BPROFILE_CFG_PERF_COUNTER=$(BPROFILE_COUNTERS)
endif
endif
endif

# Optional BTRC (manual profiler) support
ifeq ($(BTRC_SUPPORT),y)
NEXUS_CORE_MAGNUM_MODULES += \
    $(BSEAV)/lib/bprofile/bprofile.inc
# BSEAV makefiles don't include their module names into MAGNUM_MODULES
NEXUS_EXTRA_MODULES += BTRC
NEXUS_CORE_DEFINES +=B_HAS_TRC=1
endif
NEXUS_CORE_DEFINES += NEXUS_PROFILE_OS_$(B_REFSW_OS)
NEXUS_SYMBOLS_SRC := bsymbols.c

# Tell magnum to compile out anything that isn't required for Nexus
B_REFSW_MINIMAL := y
NEXUS_CORE_DEFINES += B_REFSW_MINIMAL

#####################
# Module Extensions #
#####################
ifneq ($(NEXUS_AVS_EXTENSION_INC),)
include $(NEXUS_AVS_EXTENSION_INC)
NEXUS_CORE_DEFINES += NEXUS_AVS_EXTENSION
endif
