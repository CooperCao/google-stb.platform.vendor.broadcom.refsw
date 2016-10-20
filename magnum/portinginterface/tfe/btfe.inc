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
#
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BTFE

# Determine which chip we are using for tfe

#
# 7364
#
ifeq ($(findstring $(BCHP_CHIP), 7364), $(BCHP_CHIP))
ifeq ($(findstring $(BCHP_VER), A0 B0), $(BCHP_VER))
BTFE_INCLUDES += \
    $(MAGNUM)/portinginterface/tfe/include/ \
    $(MAGNUM)/basemodules/hab/include/common \
    $(MAGNUM)/commonutils/fec/include/

BTFE_DEFINES += USE_DFE_ADDR_32BIT
BTFE_SOURCES += \
    $(MAGNUM)/portinginterface/tfe/src/btfe.c
endif
endif

#
# 7543
#
ifeq ($(findstring $(BCHP_CHIP), 7543), $(BCHP_CHIP))
BTFE_INCLUDES += \
    $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/a0 \
    $(MAGNUM)/portinginterface/tfe/include/ \
    $(MAGNUM)/commonutils/fec/include/

BTFE_INCLUDES += $(MAGNUM)/portinginterface/tfe/src/$(BCHP_CHIP)/firmware/$(BCHP_VER)

BTFE_DEFINES += USE_DFE_ADDR_32BIT
BTFE_SOURCES += \
    $(MAGNUM)/portinginterface/tfe/src/btfe.c \
    $(MAGNUM)/portinginterface/tfe/src/btfe_scd_priv.c \
    $(MAGNUM)/portinginterface/tfe/src/btfe_scd_hal_mips_priv.c \
    $(MAGNUM)/portinginterface/tfe/src/btfe_scd_common_func_priv.c \
    $(MAGNUM)/portinginterface/tfe/src/btfe_scd_chip_priv.c
endif
