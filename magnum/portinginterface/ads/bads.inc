############################################################
#  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#
# Conditional Options for this Makefile
#
# N/A
#
# BADS_3117_SUPPORT - If defined, support will be added for the 3117 ADS
# BADS_3109_SUPPORT - If defined, support will be added for the 3109 ADS

# Define this module
MAGNUM_MODULES += BADS

# If the chip version is not specified, select a default
ifeq ($(BCHP_312X_VER),)
BCHP_312X_VER := C0
endif

ifeq ($(BCHP_3461_VER),)
BCHP_3461_VER := B0
endif

ifeq ($(BCHP_3462_VER),)
BCHP_3462_VER := A0
endif

ifeq ($(BCHP_7584_VER),)
BCHP_7584_VER := A0
endif

BADS_312x_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_312X_VER)")}')
BADS_3461_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_3461_VER)")}')
BADS_3462_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_3462_VER)")}')

# Source needs to know which revision
BADS_DEFINES += BADS_312X_VER=BCHP_VER_$(BCHP_312X_VER)
BADS_DEFINES += BADS_3461_VER=BCHP_VER_$(BCHP_3461_VER)
BADS_DEFINES += BADS_3462_VER=BCHP_VER_$(BCHP_3462_VER)
BADS_DEFINES += BADS_7584_VER=BCHP_VER_$(BCHP_7584_VER)

# Module Includes
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/include
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/common
BADS_INCLUDES += $(MAGNUM)/commonutils/fec/include

# Module Sources
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/common/bads.c

ifneq ($(BADS_3461_SUPPORT),)
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_leap_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/3461/bads_3461.c
endif

ifneq ($(BADS_3462_SUPPORT),)
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_leap_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/3462/bads_3462.c
endif

ifneq ($(BADS_3128_SUPPORT),)
BADS_DEFINES += BADS_3128_SUPPORT
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_leap_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/3128/bads_3128.c
endif

ifneq ($(BADS_3158_SUPPORT),)
BADS_DEFINES += BADS_3158_SUPPORT
BADS_DEFINES += BADS_CHIP=3158
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_leap_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/3158/bads_3158.c
endif

ifneq ($(BADS_3466_SUPPORT),)
BADS_DEFINES += BADS_3466_SUPPORT
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_3466_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/3466/bads_3466.c
endif

ifneq ($(BADS_7584_SUPPORT),)
BADS_DEFINES += BADS_7584_SUPPORT
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_leap_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/7584/bads_7584.c
endif

ifneq ($(BADS_3109_SUPPORT),)
BADS_31XX_CHIP := 3109
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/3109/bads_3109.c
endif

ifneq ($(BADS_3112_SUPPORT),)
BADS_31XX_CHIP := 3112
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/3112/bads_3112.c
endif

ifneq ($(BADS_3114_SUPPORT),)
BADS_31XX_CHIP := 3114
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/3114/bads_3114.c
endif

ifneq ($(BADS_3117_SUPPORT),)
BADS_31XX_CHIP := 3117
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/3117/bads_3117.c
endif

ifneq ($(BADS_31XX_CHIP),)
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/include/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/bads_31xx_priv.c
endif

ifneq ($(BADS_7550_SUPPORT),)
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/7550/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550_api.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550_acquire.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550_status.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550_utils.c
endif



ifneq ($(BADS_35233_SUPPORT),)
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/35233/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/7552/bads_coef.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_3x7x.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_acquire.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_status.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_utils.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_api.c
endif

ifneq ($(BADS_7552_SUPPORT),)
ifneq ($(BADS_7552_ANNEXA_BURST_SUPPORT),)
BADS_DEFINES += BADS_ANNEXA_BURST_SUPPORT
endif
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/7552/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/bads_3x7x_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_coef.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_7552.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_acquire.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_status.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_utils.c
endif

ifneq ($(BADS_7125_SUPPORT),)
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/7125/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125_acquire.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125_status.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125_utils.c
endif

ifeq ($(BADS_CHIP),3255)
    BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/docsis/bads_docsis_priv.c
endif
