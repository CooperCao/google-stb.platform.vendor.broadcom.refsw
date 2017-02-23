############################################################
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
#
############################################################


#
# Conditional Options for this Makefile
#
# N/A
#

# Default
HAS_BOX_MODES = n

# SOCs with box mode support
ifeq ($(filter $(BCHP_CHIP),7445), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),D0 E0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7145), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7366), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0 C0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7435), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7364), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),A0 B0 C0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7439), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),74371), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),A0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7250), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7271), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),A0 B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7268), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),A0 B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7260), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),A0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7278), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),A0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif


BCHP_VER_LOWER := $(shell echo $(BCHP_VER) | tr A-Z a-z)

# Define this module
MAGNUM_MODULES += BBOX

# Module Includes
BBOX_INCLUDES += \
        $(MAGNUM)/commonutils/box/include \
        $(MAGNUM)/commonutils/box/src

# Module Sources
BBOX_SOURCES += \
        $(MAGNUM)/commonutils/box/src/bbox.c \
        $(MAGNUM)/commonutils/box/src/common/bbox_priv_vdc_common.c

ifeq (${HAS_BOX_MODES},y)
include $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/bbox_$(BCHP_CHIP).inc
BBOX_DEFINES += BBOX_BOX_MODES_SUPPORTED=1
else
BBOX_INCLUDES += \
        $(MAGNUM)/commonutils/box/src/legacy
BBOX_SOURCES += \
        $(MAGNUM)/commonutils/box/src/legacy/bbox_priv_vdc.c \
        $(MAGNUM)/commonutils/box/src/common/bbox_priv_vce_nop.c \
        $(MAGNUM)/commonutils/box/src/common/bbox_priv_xvd_nobox.c
endif

BBOX_SOURCES += \
        $(MAGNUM)/commonutils/box/src/common/bbox_priv_vce_common.c \
        $(MAGNUM)/commonutils/box/src/common/bbox_priv_audio_common.c \
        $(MAGNUM)/commonutils/box/src/common/bbox_priv_xvd_common.c

#
# End of file
#
