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
# NEXUS_FRONTEND_4500 - Enable support for the BCM4500 Satellite demodulator (cannot be used withother Sat frontends)
# NEXUS_FRONTEND_4501 - Enable support for the BCM4501 Satellite tuner/demodulator
# NEXUS_FRONTEND_4501_VER - Define 4501 chip revision (default=C0)
# NEXUS_FRONTEND_4506 - Enable support for the BCM4506 Satellite tuner/demodulator
# NEXUS_FRONTEND_4506_VER - Define 4506 chip revision (default=A0)
# NEXUS_FRONTEND_4517 - Enable support for the BCM4517 Satellite tuner/demodulator
# NEXUS_FRONTEND_4517_VER - Define 4517 chip revision (default=A0)
# NEXUS_FRONTEND_4538 - Enable support for the BCM4538 Satellite tuner/demodulator
# NEXUS_FRONTEND_4538_VER - Define 4538 chip revision (default=B0)
# NEXUS_FRONTEND_45216 - Enable support for the BCM45216 Satellite tuner/demodulator
# NEXUS_FRONTEND_45308 - Enable support for the BCM45308 Satellite tuner/demodulator
# NEXUS_FRONTEND_3255 - Enable support for the BCM3255 Cable frontend
# NEXUS_FRONTEND_3383 - Enable support for the BCM3383 Cable frontend
# NEXUS_FRONTEND_DOCSIS - Enable support for the DOCSIS cable modem frontend
# NEXUS_FRONTEND_3510 - Enable support for the BCM3510 QAM/VSB demodulator
# NEXUS_FRONTEND_3520 - Enable support for the BCM3520 QAM/VSB demodulator
# NEXUS_FRONTEND_3418 - Enable support for the BCM3418 tuner
# NEXUS_FRONTEND_3420 - Enable support for the BCM3420 tuner
# NEXUS_FRONTEND_3440 - Enable support for the BCM3420 satellite tuner
# NEXUS_FRONTEND_3431 - Enable support for the BCM3431 amplifier
# NEXUS_FRONTEND_DTT760X - Enable support for the Thomson DTT760x tuner
# NEXUS_FRONTEND_DTT7045 - Enable support for the Thomson DTT7045 tuner
# NEXUS_FRONTEND_DTT75409 - Enable support for the Thomson DTT75409 tuner
# NEXUS_FRONTEND_DTT76800 - Enable support for the Thomson DTT76800 tuner
# NEXUS_FRONTEND_3117 - Enable support for the BCM3117 ADS/AOB Demod and tuner and AUS modulator.

######################
# Define this module #
######################
NEXUS_MODULES += FRONTEND

############################
# Standard Module Includes #
############################
NEXUS_FRONTEND_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/frontend/common/include
NEXUS_FRONTEND_PRIVATE_INCLUDES := $(NEXUS_TOP)/modules/frontend/common/src


NEXUS_FRONTEND_PRECOMPILED_HEADER = $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_module.h

##################
# Module Sources #
##################
NEXUS_FRONTEND_SOURCES := $(NEXUS_TOP)/modules/frontend/common/src/nexus_amplifier.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_analog.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_init.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_ofdm.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_oob.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_qam.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_satellite.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_upstream.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_vsb.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_ifd.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_tuner.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_card.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_card_priv.c \
                          $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_channelbonding.c

#######################
# Module Dependencies #
#######################
NEXUS_FRONTEND_DEPENDENCIES := BASE CORE I2C GPIO SPI TRANSPORT

##################
# Magnum Modules #
##################
NEXUS_FRONTEND_MAGNUM_MODULES =

###############
# Thunk Layer #
###############
NEXUS_FRONTEND_PRIVATE_INCLUDES += $(NEXUS_SYNCTHUNK_DIR)
NEXUS_FRONTEND_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_frontend_thunks.c

###########
# Options #
###########

# IMG support
ifeq ($(NEXUS_CONFIG_IMAGE),y)
ifeq ($(NEXUS_MODE),proxy)
BHAB_IMG_ONLY := y
else
BHAB_IMG_EXTERNAL := y
endif
endif

#
# TNRs
#

ifdef NEXUS_FRONTEND_3418
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3418/src/nexus_tuner_3418.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3418/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3418
NEXUS_FRONTEND_TNR_CHIPS += 3418
BTNR_3418_SUPPORT := y
endif

ifdef NEXUS_FRONTEND_3420
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3420/src/nexus_tuner_3420.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3420/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3420
NEXUS_FRONTEND_TNR_CHIPS += 3420
BTNR_3420_SUPPORT := y
endif

ifdef NEXUS_FRONTEND_3440
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3440/src/nexus_tuner_3440.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3440/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3440
endif

ifdef NEXUS_FRONTEND_DTT760X
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/dtt760x/src/nexus_tuner_dtt760x.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/dtt760x/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_DTT760X
NEXUS_FRONTEND_TNR_CHIPS += DTT760X
BTNR_DTT760X_SUPPORT := y
endif

ifdef NEXUS_FRONTEND_DTT7045
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/dtt7045/src/nexus_tuner_dtt7045.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/dtt7045/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_DTT7045
NEXUS_FRONTEND_TNR_CHIPS += DTT7045
BTNR_DTT7045_SUPPORT := y
endif

ifdef NEXUS_FRONTEND_DTT75409
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/dtt75409/src/nexus_tuner_dtt75409.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/dtt75409/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_DTT75409
NEXUS_FRONTEND_TNR_CHIPS += DTT75409
BTNR_DTT75409_SUPPORT := y
endif

ifdef NEXUS_FRONTEND_DTT76800
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/dtt76800/src/nexus_tuner_dtt76800.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/dtt76800/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_DTT76800
NEXUS_FRONTEND_TNR_CHIPS += DTT76800
BTNR_DTT76800_SUPPORT := y
endif

ifeq ($(NEXUS_FRONTEND_7125),y)
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7125/src/nexus_tuner_7125.c
NEXUS_FRONTEND_TNR_CHIPS += 7125
endif

#
# LNAs
#
ifdef NEXUS_FRONTEND_3431
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3431/src/nexus_amplifier_3431.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3431/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3431
NEXUS_FRONTEND_LNA_CHIPS += 3431
BLNA_3431_SUPPORT := y
endif

ifdef NEXUS_FRONTEND_3412
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3412/src/nexus_amplifier_3412.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3412/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3412
NEXUS_FRONTEND_LNA_CHIPS += 3412
BLNA_3412_SUPPORT := y
endif

#
# SDS
#
ifdef NEXUS_FRONTEND_4500
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/4500/src/nexus_frontend_4500.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/4500/src/nexus_frontend_sds.c
NEXUS_FRONTEND_SOURCES += $(MAGNUM)/portinginterface/sds/4500/bcm4500ap.c
NEXUS_FRONTEND_PRIVATE_INCLUDES += $(MAGNUM)/portinginterface/sds/4500

NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/4500/include/
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/sds/bsds.inc
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_4500
endif

#
# ASTs
#
ifeq ($(NEXUS_FRONTEND_4501),y)
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/4501/src/nexus_frontend_4501.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/4501/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_4501
BAST_CHIP += 4501
ifdef NEXUS_FRONTEND_4501_VER
BAST_4501_VER := $(NEXUS_FRONTEND_4501_VER)
else
BAST_4501_VER := C0
endif
endif

ifeq ($(NEXUS_FRONTEND_4506),y)
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/4506/src/nexus_frontend_4506.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/4506/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_4506
BAST_CHIP += 4506
ifdef NEXUS_FRONTEND_4506_VER
BAST_4506_VER := $(NEXUS_FRONTEND_4506_VER)
else
BAST_4506_VER := A0
endif
endif

ifdef NEXUS_FRONTEND_4528
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/4528/src/nexus_frontend_4528.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/4528/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_4528
BHAB_4528_SUPPORT:=y
NEXUS_CFLAGS += -DBAST_USE_HAB_PI=1
BAST_CHIP += 4528
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
ifdef NEXUS_FRONTEND_4528_VER
BAST_4528_VER := $(NEXUS_FRONTEND_4528_VER)
else
BAST_4528_VER := A0
endif
endif

ifeq ($(NEXUS_FRONTEND_4538),y)
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_SATFE_IMG
ifeq ($(filter $(NEXUS_PLATFORM), 97429 974295 97445), $(NEXUS_PLATFORM))
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_HAS_A8299_DISEQC
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_A8299_0_I2C_ADDR=0x8
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_A8299_1_I2C_ADDR=0xB
endif
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/4538/src/nexus_frontend_4538.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/4538/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_4538
BHAB_4538_SUPPORT:=y
NEXUS_CFLAGS += -DBAST_USE_HAB_PI=1
BAST_CHIP += 4538
BWFE_CHIP += 4538
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/wfe/bwfe.inc
ifdef NEXUS_FRONTEND_4538_VER
BAST_4538_VER := $(NEXUS_FRONTEND_4538_VER)
BCHP_4538_VER := $(shell awk 'BEGIN{print tolower("$(NEXUS_FRONTEND_4538_VER)")}')
else
BAST_4538_VER := B0
BCHP_4538_VER := B0
endif
endif

ifdef NEXUS_FRONTEND_4548
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7366/src/nexus_frontend_7366.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/7366/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7366
# The line below is not necessary for bsat.inc but is needed below in this file
BSAT_CHIP += 7366
BHAB_4548_SUPPORT:=y
ifeq ($(BCHP_7366_VER),)
BCHP_7366_VER := A0
endif
BHAB_CHIP += 4548
BWFE_CHIP += 7366
BDSQ_CHIP += 7366
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/wfe/bwfe.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/dsq/bdsq.inc
ifeq ($(NEXUS_FRONTEND_FSK_SUPPORT),y)
BFSK_CHIP += 7366
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FSK=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/fsk/bfsk.inc
endif
endif

ifeq ($(NEXUS_FRONTEND_45216),y)
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_SATFE_IMG
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/45216/src/nexus_frontend_45216.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/45216/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_45216
# The line below is not necessary for bsat.inc but is needed below in this file
BSAT_CHIP += 45216
BHAB_45216_SUPPORT:=y
BHAB_CHIP += 45216
BWFE_CHIP += 45216
BDSQ_CHIP += 45216
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/wfe/bwfe.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/dsq/bdsq.inc
ifeq ($(NEXUS_FRONTEND_FSK_SUPPORT),y)
BFSK_CHIP += 45216
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FSK=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/fsk/bfsk.inc
endif
endif

ifeq ($(NEXUS_FRONTEND_45308),y)
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_SATFE_IMG
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/45308/src/nexus_frontend_45308.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/45308/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_45308
# The line below is not necessary for bsat.inc but is needed below in this file
BSAT_CHIP += 45308
BHAB_45308_SUPPORT:=y
BHAB_CHIP += 45308
BWFE_CHIP += 45308
BDSQ_CHIP += 45308
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/wfe/bwfe.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/dsq/bdsq.inc
ifeq ($(NEXUS_FRONTEND_FSK_SUPPORT),y)
BFSK_CHIP += 45308
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FSK=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/fsk/bfsk.inc
endif
ifdef NEXUS_FRONTEND_45308_VER
BSAT_45308_VER := $(NEXUS_FRONTEND_45308_VER)
BCHP_45308_VER := $(shell awk 'BEGIN{print tolower("$(NEXUS_FRONTEND_45308_VER)")}')
else
BSAT_45308_VER := B0
BCHP_45308_VER := B0
endif
endif

ifdef NEXUS_FRONTEND_4517
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/4517/src/nexus_frontend_4517.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/4517/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_4517
BHAB_4517_SUPPORT:=y
NEXUS_CFLAGS += -DBAST_USE_HAB_PI=1
BAST_CHIP += 4517
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
ifdef NEXUS_FRONTEND_4517_VER
BAST_4517_VER := $(NEXUS_FRONTEND_4517_VER)
else
BAST_4517_VER := A0
endif
endif

ifeq ($(findstring $(BCHP_CHIP), 7346 7344 7358 7360 7362 73625 7228 73465), $(BCHP_CHIP))
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7346/src/nexus_frontend_7346.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/7346/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7346
# The line below is not necessary for bast.inc but is needed below in this file
BAST_CHIP += $(BCHP_CHIP)
endif

ifeq ($(findstring $(BCHP_CHIP), 7366), $(BCHP_CHIP))
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_SATFE_IMG
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7366/src/nexus_frontend_7366.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/7366/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7366
# The line below is not necessary for bsat.inc but is needed below in this file
BSAT_CHIP += $(BCHP_CHIP)
BHAB_7366_SUPPORT:=y
ifeq ($(BCHP_7366_VER),)
ifeq ($(BCHP_VER),A0)
BCHP_7366_VER := A0
endif
endif
BWFE_CHIP += 7366
BDSQ_CHIP += 7366
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/wfe/bwfe.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/dsq/bdsq.inc
ifeq ($(NEXUS_FRONTEND_FSK_SUPPORT),y)
BFSK_CHIP += 7366
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FSK=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/fsk/bfsk.inc
endif
endif

ifeq ($(findstring $(BCHP_CHIP), 7364), $(BCHP_CHIP))
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_CTFE_IMG
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7364/src/nexus_frontend_7364.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7364/src/nexus_frontend_7364_cable.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7364/src/nexus_frontend_7364_terrestrial.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7364/src/nexus_frontend_7364_satellite.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/7364/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7364
ifeq ($(NEXUS_FRONTEND_LEAP_UART_ENABLE), y)
BHAB_LEAP_UART_ENABLE :=y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_LEAP_UART_ENABLE
endif
# The line below is not necessary for bsat.inc but is needed below in this file
BSAT_CHIP += $(BCHP_CHIP)
BHAB_7364_SUPPORT:=y
ifeq ($(BCHP_7364_VER),)
ifeq ($(BCHP_VER),A0)
BCHP_7364_VER := A0
endif
endif
BWFE_CHIP += 7364
BDSQ_CHIP += 7364
BTNR_7364_SUPPORT := y
BODS_7364_SUPPORT := y
BHAB_7364_SUPPORT := y
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ods/bods.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tfe/btfe.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/wfe/bwfe.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/dsq/bdsq.inc
ifeq ($(NEXUS_FRONTEND_FSK_SUPPORT),y)
BFSK_CHIP += 7364
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FSK=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/fsk/bfsk.inc
endif
endif

#
# VSBs
#
ifdef NEXUS_FRONTEND_3510
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3510/src/nexus_frontend_3510.c \
        $(MAGNUM)/portinginterface/vsb/src/3510/bcm3510ap.c
NEXUS_FRONTEND_PRIVATE_INCLUDES += $(MAGNUM)/portinginterface/vsb/src/3510 \
        $(MAGNUM)/portinginterface/vsb/src/common
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3510/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3510
BVSB_CHIP := 3510
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/vsb/bvsb.inc
ifdef NEXUS_FRONTEND_3520
$(error can not support 3510 and 3520 simultaneously)
endif
endif

ifdef NEXUS_FRONTEND_3520
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3520/src/nexus_frontend_3520.c \
        $(MAGNUM)/portinginterface/vsb/src/3520/bcm3520ap_a1.c \
        $(MAGNUM)/portinginterface/vsb/src/3520/bcm3520ap_b0.c
NEXUS_FRONTEND_PRIVATE_INCLUDES += $(MAGNUM)/portinginterface/vsb/src/3520 \
        $(MAGNUM)/portinginterface/vsb/src/common
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3520/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3520
BVSB_CHIP := 3520
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/vsb/bvsb.inc
ifdef NEXUS_FRONTEND_3510
$(error can not support 3510 and 3520 simultaneously)
endif
endif

ifeq ($(findstring $(BCHP_CHIP), 7552), $(BCHP_CHIP))
ifeq ($(NEXUS_USE_3128_FRONTEND_DAUGHTER_CARD),y)
NEXUS_FRONTEND_DEFINES += NEXUS_USE_3128_FRONTEND_DAUGHTER_CARD
endif
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/$(BCHP_CHIP)/src/nexus_frontend_scan.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/$(BCHP_CHIP)/src/nexus_frontend_7552.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/$(BCHP_CHIP)/include
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7552
NEXUS_FRONTEND_TNR_CHIPS += 7552
BTNR_7552_SUPPORT := y
BADS_7552_SUPPORT := y
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/thd/bthd.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
ifeq ($(BCHP_VER),A0)
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7552_A0
endif
ifeq ($(BCHP_VER),B0)
BAOB_CHIP := 7552
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aob/baob.inc
endif
endif

ifeq ($(filter $(BCHP_CHIP), 75525), $(BCHP_CHIP))
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/75525/src/nexus_frontend_75525.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/75525/src/nexus_frontend_75525_cable.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/75525/src/nexus_frontend_75525_terrestrial.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/75525/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_75525
ifeq ($(NEXUS_FRONTEND_LEAP_UART_ENABLE), y)
BHAB_LEAP_UART_ENABLE :=y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_LEAP_UART_ENABLE
endif
# The line below is not necessary for bsat.inc but is needed below in this file
BHAB_75525_SUPPORT:=y
ifeq ($(BCHP_75525_VER),)
BCHP_75525_VER := $(BCHP_VER)
endif
BTNR_75525_SUPPORT := y
BODS_75525_SUPPORT := y
BHAB_75525_SUPPORT := y
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ods/bods.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
endif

#
# QAM
#

ifeq ($(NEXUS_FRONTEND_DOCSIS),y)
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/docsis/src/nexus_docsis.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/docsis/src/nexus_docsis_data.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/docsis/src/nexus_docsis_priv.c
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/docsis/src/nexus_docsis_interface.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/docsis/include/
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/docsis/common/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_DOCSIS
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/dcm/bdcm.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
endif # NEXUS_FRONTEND_DOCSIS

ifeq ($(NEXUS_FRONTEND_3383),y)
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/docsis_legacy/3383/src/nexus_frontend_3255.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/docsis_legacy/3383/include/
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/docsis/common/include/
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3383

BTNR_3255IB_SUPPORT := y
BTNR_3255OB_SUPPORT := y
BADS_CHIP := 3255
BAOB_CHIP := 3255
BAUS_CHIP := 3255

NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aob/baob.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aus/baus.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/rpc/brpc.inc

NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/rpc/brpc.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
endif # NEXUS_FRONTEND_3383

ifdef NEXUS_FRONTEND_3255
ifeq ($(findstring $(BCHP_CHIP), 7125), $(BCHP_CHIP))
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/docsis_legacy/7125/src/nexus_frontend_3255.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/docsis_legacy/7125/include/
else
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/docsis_legacy/3255/src/nexus_frontend_3255.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/docsis_legacy/3255/include/
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/docsis/common/include/
endif
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3255
ifeq ($(NEXUS_FRONTEND_ACCUMULATE_STATISTICS),n)
#use NEXUS_FRONTEND_ACCUMULATE_STATISTICS=n to keep existing behavior for 3255
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_ACCUMULATE_STATISTICS=0
else
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_ACCUMULATE_STATISTICS=1
endif
BTNR_3255IB_SUPPORT := y
BTNR_3255OB_SUPPORT := y
BADS_CHIP := 3255
BAOB_CHIP := 3255
BAUS_CHIP := 3255
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aob/baob.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aus/baus.inc

NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/rpc/brpc.inc

endif # NEXUS_FRONTEND_3255

ifeq ($(NEXUS_FRONTEND_7125),y)
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7125/src/nexus_frontend_7125_qam.c
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/7125/include
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7125
BADS_7125_SUPPORT := y
BTNR_7125_SUPPORT ?= y
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
endif

#
# These lines should be last in the file
#
# Based on earlier settings, include additional magnum modules.
ifneq ($(NEXUS_FRONTEND_LNA_CHIPS),)
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/lna/blna.inc
NEXUS_FRONTEND_DEFINES += NEXUS_AMPLIFIER_SUPPORT
endif

ifneq ($(NEXUS_FRONTEND_TNR_CHIPS),)
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_DEFINES += NEXUS_TUNER_SUPPORT
endif

ifneq ($(NEXUS_FRONTEND_IFD_CHIPS),)
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ifd/bifd.inc
NEXUS_FRONTEND_DEFINES += NEXUS_IFD_SUPPORT
endif

ifneq ($(BAST_CHIP),)
# There are multiple AST chips.  Include the magnum support here.
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ast/bast.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_ast.c
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_AST
endif

ifneq ($(BSAT_CHIP),)
ifeq ($(NEXUS_FRONTEND_BYPASS_LEAP),y)
BYPASS_LEAP:=1
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_BYPASS_LEAP
endif
# There are multiple SAT chips.  Include the magnum support here.
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/sat/bsat.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/common/src/nexus_frontend_sat.c
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_SAT
endif

# Allow NEXUS_POWER_MANAGEMENT to be a master switch
ifeq ($(NEXUS_POWER_MANAGEMENT),n)
BVSB_POWER_MANAGEMENT := n
endif

#
# Combo frontend chips ADS, AOb, AUS, THD, T2
#

ifdef NEXUS_FRONTEND_3472
$(error 3472 Frontend support has been removed)
BTNR_3472_SUPPORT := y
BODS_3472_SUPPORT := y
BHAB_3472_SUPPORT := y
ifeq ($(NEXUS_FRONTEND_LEAP_UART_ENABLE), y)
BHAB_LEAP_UART_ENABLE :=y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_LEAP_UART_ENABLE
endif
ifeq ($(NEXUS_FRONTEND_3472_VER),)
BCHP_3472_VER := B0
else
BCHP_3472_VER := $(NEXUS_FRONTEND_3472_VER)
endif
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3472
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3461/include/
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ods/bods.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3472/src/nexus_frontend_3472.c
ifeq ($(NEXUS_USE_FRONTEND_DAUGHTER_CARD),y)
NEXUS_FRONTEND_DEFINES += NEXUS_USE_FRONTEND_DAUGHTER_CARD
endif
endif

ifdef NEXUS_FRONTEND_3462
BTNR_3462_SUPPORT := y
BADS_3462_SUPPORT := y
BODS_3462_SUPPORT := y
BHAB_3462_SUPPORT := y
ifeq ($(NEXUS_FRONTEND_3462_VER),)
BCHP_3462_VER := A0
else
BCHP_3462_VER := $(NEXUS_FRONTEND_3462_VER)
endif
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3461/include/
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ods/bods.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3462/src/nexus_frontend_3462.c
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3462
endif

ifdef NEXUS_FRONTEND_3461
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_CTFE_IMG
BTNR_3461_SUPPORT := y
BADS_3461_SUPPORT := y
BTC2_3461_SUPPORT := y
BHAB_3461_SUPPORT := y
ifeq ($(NEXUS_FRONTEND_LEAP_UART_ENABLE), y)
BHAB_LEAP_UART_ENABLE :=y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_LEAP_UART_ENABLE
endif
BTHD_CHIP := 3461
ifeq ($(NEXUS_FRONTEND_3461_VER),)
BCHP_3461_VER := B0
else
BCHP_3461_VER := $(NEXUS_FRONTEND_3461_VER)
endif
ifeq ($(NEXUS_FRONTEND_3461_VER), A0)
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3461_A0
else
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3461_BX
endif
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3461/include/
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tc2/btc2.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/thd/bthd.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3461/src/nexus_frontend_3461.c
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3461
endif

ifdef NEXUS_FRONTEND_7563
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_CTFE_IMG
BTNR_7563_SUPPORT := y
BADS_7563_SUPPORT := y
BODS_7563_SUPPORT := y
BHAB_7563_SUPPORT := y
ifeq ($(NEXUS_FRONTEND_LEAP_UART_ENABLE), y)
BHAB_LEAP_UART_ENABLE :=y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_LEAP_UART_ENABLE
endif
ifeq ($(NEXUS_FRONTEND_7563_VER),)
BCHP_7563_VER := A0
else
BCHP_7563_VER := $(NEXUS_FRONTEND_7563_VER)
endif

NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7563

NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3461/include/
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ods/bods.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7563/src/nexus_frontend_7563.c
endif

ifdef NEXUS_FRONTEND_7584
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_CTFE_IMG
BTNR_7584IB_SUPPORT := y
BADS_7584_SUPPORT := y
BHAB_7584_SUPPORT := y

ifeq ($(NEXUS_FRONTEND_LEAP_UART_ENABLE), y)
BHAB_LEAP_UART_ENABLE :=y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_LEAP_UART_ENABLE
endif
ifeq ($(NEXUS_FRONTEND_7584_OOB), y)
BAOB_CHIP := 7584
BTNR_7584OB_SUPPORT := y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7584_OOB
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aob/baob.inc
endif
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_7584


NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3128/include/
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/7584/src/nexus_frontend_7584.c
endif

ifdef NEXUS_FRONTEND_3128
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_CTFE_IMG
BTNR_3128IB_SUPPORT := y
BADS_3128_SUPPORT := y
BHAB_3128_SUPPORT := y
ifeq ($(NEXUS_FRONTEND_LEAP_UART_ENABLE), y)
BHAB_LEAP_UART_ENABLE :=y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_LEAP_UART_ENABLE
endif
ifeq ($(NEXUS_FRONTEND_312x_OOB), y)
BAOB_CHIP := 3128
BTNR_3128OB_SUPPORT := y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_312x_OOB
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aob/baob.inc
endif
ifeq ($(NEXUS_FRONTEND_312x_VER),)
BCHP_312X_VER := B0
else
BCHP_312X_VER := $(NEXUS_FRONTEND_312x_VER)
endif
ifeq ($(NEXUS_FRONTEND_312x_VER), A0)
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_312x_A0
endif
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3128
endif

ifdef NEXUS_FRONTEND_3158
ifdef NEXUS_FRONTEND_3158_VER
BCHP_3158_VER := $(shell awk 'BEGIN{print tolower("$(NEXUS_FRONTEND_3158_VER)")}')
else
BCHP_3158_VER := a0
endif

NEXUS_FRONTEND_DEFINES += NEXUS_HAS_FRONTEND_CTFE_IMG
BADS_3158_SUPPORT := y
BHAB_3158_SUPPORT := y
ifeq ($(NEXUS_FRONTEND_LEAP_UART_ENABLE), y)
BHAB_LEAP_UART_ENABLE :=y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_LEAP_UART_ENABLE
endif
ifeq ($(NEXUS_FRONTEND_315x_OOB), y)
BAOB_CHIP := 3158
BTNR_3158OB_SUPPORT := y
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_315x_OOB
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aob/baob.inc
endif
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3158

NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3158/include/
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3158/src/nexus_frontend_3158.c
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
endif

ifneq ($(BCHP_312X_VER),)

# 7344 and 7346 use 3128, but in legacy mode. MXT not required
ifneq ($(findstring $(BCHP_CHIP), 7344 7346 7584 73465), $(BCHP_CHIP))
NEXUS_FRONTEND_DEFINES += NEXUS_HAS_MXT=1
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/mxt/bmxt.inc
endif

ifeq ($(NEXUS_USE_FRONTEND_DAUGHTER_CARD),y)
NEXUS_FRONTEND_DEFINES += NEXUS_USE_FRONTEND_DAUGHTER_CARD
endif
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3128/include/
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3128/src/nexus_frontend_3128.c
endif

ifdef NEXUS_FRONTEND_3117
BTNR_3117IB_SUPPORT := y
BTNR_3117OB_SUPPORT := y
BADS_3117_SUPPORT := y
BAUS_CHIP := 3117
BHAB_3117_SUPPORT := y
31XX_CHIP := 3117
ifeq ($(BAOB_CHIP),)
BAOB_CHIP := 3117
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_31xx_OOB
endif
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3117
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aob/baob.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aus/baus.inc
endif

ifdef NEXUS_FRONTEND_3114
BTNR_3114IB_SUPPORT := y
BTNR_3114OB_SUPPORT := y
BADS_3114_SUPPORT := y
BHAB_3114_SUPPORT := y
31XX_CHIP := 3117
ifeq ($(BAOB_CHIP),)
BAOB_CHIP := 3114
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_31xx_OOB
endif
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3114
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/aob/baob.inc
endif

ifdef NEXUS_FRONTEND_3112
BTNR_3112IB_SUPPORT := y
BADS_3112_SUPPORT := y
BHAB_3112_SUPPORT := y
31XX_CHIP := 3112
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3112
endif

ifdef NEXUS_FRONTEND_3109
BTNR_3109IB_SUPPORT := y
BADS_3109_SUPPORT := y
BHAB_3109_SUPPORT := y
31XX_CHIP := 3109
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_3109
endif

ifneq ($(31XX_CHIP),)
ifeq ($(NEXUS_FRONTEND_31xx_VER),)
    BCHP_31XX_VER := B0
else
    BCHP_31XX_VER := $(NEXUS_FRONTEND_31xx_VER)
endif
ifeq ($(NEXUS_FRONTEND_31xx_VER), B0)
    NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_31xx_B0
endif
NEXUS_FRONTEND_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/frontend/3117/include/
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/ads/bads.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/portinginterface/tnr/btnr.inc
NEXUS_FRONTEND_MAGNUM_MODULES += $(MAGNUM)/basemodules/hab/bhab.inc
NEXUS_FRONTEND_SOURCES += $(NEXUS_TOP)/modules/frontend/3117/src/nexus_frontend_31xx.c
endif

ifeq ($(NEXUS_PLATFORM_VER),HR44)
NEXUS_CFLAGS += -DBAST_USE_SNRE
endif

#####################
# Module Extensions #
#####################
ifneq ($(NEXUS_FRONTEND_EXTENSION_INC),)
include $(NEXUS_FRONTEND_EXTENSION_INC)
NEXUS_FRONTEND_DEFINES += NEXUS_FRONTEND_EXTENSION
endif

ifneq ($(NEXUS_FRONTEND_CUSTOMER_EXTENSION_INC),)
include $(NEXUS_FRONTEND_CUSTOMER_EXTENSION_INC)
endif
