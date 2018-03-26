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
# BTNR_3250OB_SUPPORT - If defined, support will be added for the 3250 OOB tuner
# BTNR_3418_SUPPORT - If defined, support will be added for the 3418 tuner
# BTNR_3420_SUPPORT - If defined, support will be added for the 3420 IB tuner
# BTNR_DTT760X_SUPPORT - If defined, support will be added for the DTT760x tuner
# BTNR_DTT76800_SUPPORT - If defined, support will be added for the DTT76800 tuner
# BTNR_DTT75409_SUPPORT - If defined, support will be added for the DTT75409 tuner
# BTNR_3255IB_SUPPORT - If defined, support will be added for the 3255 IB tuner
# BTNR_3255OB_SUPPORT - If defined, support will be added for the 3255 OOB tuner
# BTNR_3117IB_SUPPORT - If defined, support will be added for the 3117 IB tuner
#

# Define this module
MAGNUM_MODULES += BTNR

# Module Includes
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/common
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/common
BTNR_INCLUDES += $(MAGNUM)/commonutils/fec/include

# Module Sources
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/common/btnr.c

# Conditional Sources
ifneq ($(BTNR_3250OB_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3250ob/

BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/docsis/3250ob/btnr_3250ob.c
endif

ifneq ($(BTNR_3418_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/standalone/3418/

BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/standalone/3418/btnr_3418.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/standalone/3418/btnr_3418_regio.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/standalone/3418/btnr_3418_smarttune.c
endif

ifneq ($(BTNR_3420_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/standalone/3420/

BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/standalone/3420/btnr_3420.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/standalone/3420/btnr_3420_priv.c
endif

ifneq ($(BTNR_DTT760X_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/standalone/dtt760x/

BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/standalone/dtt760x/btnr_dtt760x.c
endif

ifneq ($(BTNR_DTT76800_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/standalone/dtt76800/

BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/standalone/dtt76800/btnr_dtt76800.c
endif


ifneq ($(BTNR_DTT75409_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/standalone/dtt75409/

BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/standalone/dtt75409/btnr_dtt75409.c
endif

ifneq ($(BTNR_3255IB_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3255ib/

BTNR_SOURCES += \
    $(MAGNUM)/portinginterface/tnr/src/docsis/3255ib/btnr_3255ib.c
endif

ifneq ($(BTNR_3255OB_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3255ob/

BTNR_SOURCES += \
    $(MAGNUM)/portinginterface/tnr/src/docsis/3255ob/btnr_3255ob.c
endif

ifneq ($(BTNR_3117IB_SUPPORT),)
BTNR_31xx_CHIP := 3117ib
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3117ib/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/8051/3117ib/btnr_3117ib.c
endif

ifneq ($(BTNR_3112IB_SUPPORT),)
BTNR_31xx_CHIP := 3112ib
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3112ib/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/8051/3112ib/btnr_3112ib.c
endif

ifneq ($(BTNR_3109IB_SUPPORT),)
BTNR_31xx_CHIP := 3109ib
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3109ib/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/8051/3109ib/btnr_3109ib.c
endif

ifneq ($(BTNR_3114IB_SUPPORT),)
BTNR_31xx_CHIP := 3114ib
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3114ib/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/8051/3114ib/btnr_3114ib.c
endif

ifneq ($(BTNR_3128IB_SUPPORT),)
BTNR_312X_CHIP := 3128ib
BTNR_DEFINES += BTNR_3128_SUPPORT
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3128ib/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapwideband/3128ib/btnr_3128ib.c
endif

ifneq ($(BTNR_7584IB_SUPPORT),)
BTNR_7584_CHIP := 7584ib
BTNR_DEFINES += BTNR_7584_SUPPORT
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/7584ib/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapwideband/7584ib/btnr_7584ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapwideband/7584ib/btnr_7584ib_priv.c
endif

ifneq ($(BTNR_3461_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3461ib/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/3461ib/btnr_3461ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/3461ib/btnr_3461ib_priv.c
endif

ifneq ($(BTNR_3462_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3462ib/
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/3462ib/btnr_3462ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/btnr_34xxib_priv.c
endif

ifneq ($(BTNR_3466_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3466ib/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/3466ib/btnr_3466ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/3466ib/btnr_3466ib_priv.c
endif

ifneq ($(BTNR_3472_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3472ib/
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/3472ib/btnr_3472ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/btnr_34xxib_priv.c
endif

ifneq ($(BTNR_7255_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/7255ib/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/7255ib/btnr_7255ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/7255ib/btnr_7255ib_priv.c
endif

ifneq ($(BTNR_7563_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/7563ib/
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/7563ib/btnr_7563ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/7563ib/btnr_7563ib_priv.c
endif

ifneq ($(BTNR_75635_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/7563ib/
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/7563ib/btnr_7563ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/7563ib/btnr_7563ib_priv.c
endif

ifneq ($(BTNR_7364_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/7364ib/
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/7364ib/btnr_7364ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/btnr_leapib_priv.c
endif

ifneq ($(BTNR_75525_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/75525ib/
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/75525ib/btnr_75525ib.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapnarrowband/btnr_leapib_priv.c
endif

ifneq ($(BTNR_31xx_CHIP),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/8051/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/8051/btnr_31xxib_priv.c
endif

ifneq ($(BTNR_312X_CHIP),)
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapwideband/$(BTNR_312X_CHIP)/btnr_312xib_priv.c
endif

ifneq ($(BTNR_3117OB_SUPPORT),)
BTNR_31xx_CHIP_OB := 3117ob
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3117ob/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/8051/3117ob/btnr_3117ob.c
endif

ifneq ($(BTNR_3114OB_SUPPORT),)
BTNR_31xx_CHIP_OB := 3114ob
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3114ob/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/8051/3114ob/btnr_3114ob.c
endif

ifneq ($(BTNR_3128OB_SUPPORT),)
BTNR_312X_CHIP_OB := 3128ob
BTNR_DEFINES += BTNR_3128_SUPPORT
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3128ob/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapwideband/3128ob/btnr_3128ob.c
endif

ifneq ($(BTNR_7584OB_SUPPORT),)
BTNR_7584_CHIP_OB := 7584ob
BTNR_DEFINES += BTNR_7584_SUPPORT
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/7584ob/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapwideband/7584ob/btnr_7584ob.c
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapwideband/7584ob/btnr_7584ob_priv.c
endif

ifneq ($(BTNR_31xx_CHIP_OB),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/8051/
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/8051/btnr_31xxob_priv.c
endif

ifneq ($(BTNR_312X_CHIP_OB),)
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/leapwideband/$(BTNR_312X_CHIP_OB)/btnr_312xob_priv.c
endif

ifneq ($(BTNR_7550_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/7550ib
BTNR_INCLUDES += $(MAGNUM)/basemodules/chp/include/3412/rdb/a0
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/soc/7550ib
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/soc/7550ib/btnr_7550ib.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7550ib/btnr_75xxib_priv.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7550ib/btnr_7550ib_lna.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7550ib/btnr_7550ib_status.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7550ib/btnr_7550ib_tune.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7550ib/btnr_7550ib_utils.c
endif

ifeq ($(REF_BOARD_LNA), y)
BTNR_DEFINES += REF_7550_BOARD
endif

ifneq ($(BTNR_7125_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/7125
BTNR_INCLUDES += $(MAGNUM)/basemodules/chp/include/3412/rdb/a0
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/soc/7125ib/btnr_7125ib.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7125ib/btnr_7125ib_priv.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7125ib/btnr_7125_lna.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7125ib/btnr_7125_status.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7125ib/btnr_7125_tune.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/7125ib/btnr_7125_utils.c
ifneq ($(IFAGC_OD_MODE), n)
BTNR_DEFINES += IFAGC_OD_MODE
endif
else
ifeq ($(IFAGC_OD_MODE), y)
BTNR_DEFINES += IFAGC_OD_MODE
endif
endif


ifneq ($(BTNR_7552_SUPPORT),)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3x7xib
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/soc
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/soc/3x7xib
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/soc/3x7xib/btnr_3x7x.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/btnr_3x7xib_priv.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/3x7xib/btnr_init.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/3x7xib/btnr_tune.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/3x7xib/btnr_tune_ofdm.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/3x7xib/btnr_tune_ads.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/3x7xib/btnr_callback.c
ifeq ($(BTNR_7552_J83A_SUPPORT), y)
BTNR_DEFINES += BTNR_J83A_SUPPORT
endif

ifeq ($(BTNR_7552_LNAAGC_SAVE_POWER), y)
BTNR_DEFINES += BTNR_LNAAGC_SAVE_POWER
endif

ifeq ($(BTNR_7552_LNAAGC_FRZ), y)
BTNR_DEFINES += BTNR_LNAAGC_FRZ
endif

ifeq ($(BCHP_VER),B0)
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/include/3x7xob
BTNR_INCLUDES += $(MAGNUM)/portinginterface/tnr/src/soc/3x7xob
BTNR_SOURCES += $(MAGNUM)/portinginterface/tnr/src/soc/3x7xob/btnr_ob_3x7x.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/btnr_3x7xob_priv.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/3x7xob/btnr_ob_init.c \
				$(MAGNUM)/portinginterface/tnr/src/soc/3x7xob/btnr_ob_tune.c
endif
endif