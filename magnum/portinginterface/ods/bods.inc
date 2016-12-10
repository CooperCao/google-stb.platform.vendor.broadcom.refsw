############################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
# N/A
#
# BODS_3462_SUPPORT - If defined, support will be added for the 3462 ODS

# Define this module
MAGNUM_MODULES += BODS

# If the chip version is not specified, select a default
ifeq ($(BCHP_3462_VER),)
BCHP_3462_VER := A0
endif

# Source needs to know which revision
BODS_DEFINES += BODS_3462_VER=BCHP_VER_$(BCHP_3462_VER)

# Module Includes
BODS_INCLUDES += $(MAGNUM)/portinginterface/ods/include
BODS_INCLUDES += $(MAGNUM)/portinginterface/ods/src/common
BODS_INCLUDES += $(MAGNUM)/portinginterface/ods/src/leap
BODS_INCLUDES += $(MAGNUM)/commonutils/fec/include

# Module Sources
BODS_SOURCES +=	$(MAGNUM)/portinginterface/ods/src/common/bods.c

ifneq ($(BODS_7364_SUPPORT),)
BODS_DEFINES += BODS_CHIP=7364
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
    $(MAGNUM)/portinginterface/ods/src/leap/7364/bods_7364.c
endif

ifneq ($(BODS_75525_SUPPORT),)
BODS_DEFINES += BODS_CHIP=75525
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
    $(MAGNUM)/portinginterface/ods/src/leap/75525/bods_75525.c
endif

ifneq ($(BODS_7563_SUPPORT),)
BODS_DEFINES += BODS_CHIP=7563
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
    $(MAGNUM)/portinginterface/ods/src/leap/7563/bods_7563.c
endif

ifneq ($(BODS_75635_SUPPORT),)
BODS_DEFINES += BODS_CHIP=7563
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
    $(MAGNUM)/portinginterface/ods/src/leap/7563/bods_7563.c
endif

ifneq ($(BODS_3472_SUPPORT),)
BODS_INCLUDES += $(MAGNUM)/basemodules/chp/3472
BODS_DEFINES += BODS_CHIP=3472
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
    $(MAGNUM)/portinginterface/ods/src/leap/3472/bods_3472.c
endif

ifneq ($(BODS_3462_SUPPORT),)
BODS_INCLUDES += $(MAGNUM)/basemodules/chp/3462
BODS_DEFINES += BODS_CHIP=3462
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
	$(MAGNUM)/portinginterface/ods/src/leap/3462/bods_3462.c
endif

ifneq ($(BODS_3466_SUPPORT),)
BODS_INCLUDES += $(MAGNUM)/basemodules/chp/3466
BODS_DEFINES += BODS_CHIP=3466
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
	$(MAGNUM)/portinginterface/ods/src/leap/3466/bods_3466.c
endif