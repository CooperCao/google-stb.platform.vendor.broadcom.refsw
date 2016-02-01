##############################################################################
#    (c)2011-2013 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#############################################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BSAT

BSAT_INCLUDES += \
   $(MAGNUM)/commonutils/fec/include \
	$(MAGNUM)/portinginterface/sat/include/common \
	$(MAGNUM)/portinginterface/sat/src/common

BSAT_SOURCES += \
	$(MAGNUM)/portinginterface/sat/src/common/bsat.c


##############################
# Handle 7366
##############################
ifeq ($(findstring 7366,$(BSAT_CHIP)),7366)

# If chip version not specified, select a default
ifeq ($(BSAT_7366_VER),)
BSAT_7366_VER := A0
endif

BSAT_7366_VER_UPPER := $(strip $(BSAT_7366_VER))
BSAT_7366_VER_UPPER := $(subst a,A,$(BSAT_7366_VER_UPPER))
BSAT_7366_VER_UPPER := $(subst b,B,$(BSAT_7366_VER_UPPER))
BSAT_7366_VER_UPPER := $(subst c,C,$(BSAT_7366_VER_UPPER))

BSAT_7366_VER_LOWER := $(strip $(BSAT_7366_VER))
BSAT_7366_VER_LOWER := $(subst A,a,$(BSAT_7366_VER_LOWER))
BSAT_7366_VER_LOWER := $(subst B,b,$(BSAT_7366_VER_LOWER))
BSAT_7366_VER_LOWER := $(subst C,c,$(BSAT_7366_VER_LOWER))

# Source needs to know which revision
BSAT_DEFINES += BSAT_7366_VER=BCHP_VER_$(BSAT_7366_VER_UPPER)

ifeq ($(BYPASS_LEAP),1)

BSAT_DEFINES += BYPASS_LEAP
BSAT_INCLUDES += \
	$(MAGNUM)/portinginterface/sat/include/7366 \
	$(MAGNUM)/portinginterface/sat/include/g1 \
	$(MAGNUM)/portinginterface/sat/src/g1/common \
	$(MAGNUM)/portinginterface/sat/src/g1/7366

BSAT_SOURCES += \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_acq.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_afec.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_chan.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_cwc.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_dft.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_hp.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_misc.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_plc.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_qpsk.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_snr.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_tfec.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_timer.c \
	$(MAGNUM)/portinginterface/sat/src/g1/7366/bsat_7366_priv.c \
	$(MAGNUM)/portinginterface/sat/src/g1/7366/bsat_7366_priv_aci.c

else

BSAT_INCLUDES += \
	$(MAGNUM)/portinginterface/sat/include/7366 \
	$(MAGNUM)/portinginterface/sat/src/7366

BSAT_SOURCES += \
	$(MAGNUM)/portinginterface/sat/src/7366/bsat_7366.c \
	$(MAGNUM)/portinginterface/sat/src/7366/bsat_7366_priv.c
endif

endif


##############################
# Handle 45216
##############################
ifeq ($(findstring 45216,$(BSAT_CHIP)),45216)

# If chip version not specified, select a default
ifeq ($(BSAT_45216_VER),)
BSAT_45216_VER := A0
endif

BSAT_45216_VER_UPPER := $(strip $(BSAT_45216_VER))
BSAT_45216_VER_UPPER := $(subst a,A,$(BSAT_45216_VER_UPPER))
BSAT_45216_VER_UPPER := $(subst b,B,$(BSAT_45216_VER_UPPER))
BSAT_45216_VER_UPPER := $(subst c,C,$(BSAT_45216_VER_UPPER))

BSAT_45216_VER_LOWER := $(strip $(BSAT_45216_VER))
BSAT_45216_VER_LOWER := $(subst A,a,$(BSAT_45216_VER_LOWER))
BSAT_45216_VER_LOWER := $(subst B,b,$(BSAT_45216_VER_LOWER))
BSAT_45216_VER_LOWER := $(subst C,c,$(BSAT_45216_VER_LOWER))

# Source needs to know which revision
BSAT_DEFINES += BSAT_45216_VER=BCHP_VER_$(BSAT_45216_VER_UPPER)

BSAT_INCLUDES += \
	$(MAGNUM)/portinginterface/sat/include/45216 \
	$(MAGNUM)/portinginterface/sat/src/45216

BSAT_SOURCES += \
	$(MAGNUM)/portinginterface/sat/src/45216/bsat_45216.c \
	$(MAGNUM)/portinginterface/sat/src/45216/bsat_45216_priv.c

endif


##############################
# Handle 45308
##############################
ifeq ($(findstring 45308,$(BSAT_CHIP)),45308)

# If chip version not specified, select a default
ifeq ($(BSAT_45308_VER),)
BSAT_45308_VER := A0
endif

BSAT_45308_VER_UPPER := $(strip $(BSAT_45308_VER))
BSAT_45308_VER_UPPER := $(subst a,A,$(BSAT_45308_VER_UPPER))
BSAT_45308_VER_UPPER := $(subst b,B,$(BSAT_45308_VER_UPPER))
BSAT_45308_VER_UPPER := $(subst c,C,$(BSAT_45308_VER_UPPER))

BSAT_45308_VER_LOWER := $(strip $(BSAT_45308_VER))
BSAT_45308_VER_LOWER := $(subst A,a,$(BSAT_45308_VER_LOWER))
BSAT_45308_VER_LOWER := $(subst B,b,$(BSAT_45308_VER_LOWER))
BSAT_45308_VER_LOWER := $(subst C,c,$(BSAT_45308_VER_LOWER))

# Source needs to know which revision
BSAT_DEFINES += BSAT_45308_VER=BCHP_VER_$(BSAT_45308_VER_UPPER)

BSAT_INCLUDES += \
	$(MAGNUM)/portinginterface/sat/include/45308 \
	$(MAGNUM)/portinginterface/sat/src/45308

BSAT_SOURCES += \
	$(MAGNUM)/portinginterface/sat/src/45308/bsat_45308.c \
	$(MAGNUM)/portinginterface/sat/src/45308/bsat_45308_priv.c

endif



##############################
# Handle 7364
##############################
ifeq ($(findstring 7364,$(BSAT_CHIP)),7364)

# If chip version not specified, select a default
ifeq ($(BSAT_7364_VER),)
BSAT_7364_VER := A0
endif

BSAT_7364_VER_UPPER := $(strip $(BSAT_7364_VER))
BSAT_7364_VER_UPPER := $(subst a,A,$(BSAT_7364_VER_UPPER))
BSAT_7364_VER_UPPER := $(subst b,B,$(BSAT_7364_VER_UPPER))
BSAT_7364_VER_UPPER := $(subst c,C,$(BSAT_7364_VER_UPPER))

BSAT_7364_VER_LOWER := $(strip $(BSAT_7364_VER))
BSAT_7364_VER_LOWER := $(subst A,a,$(BSAT_7364_VER_LOWER))
BSAT_7364_VER_LOWER := $(subst B,b,$(BSAT_7364_VER_LOWER))
BSAT_7364_VER_LOWER := $(subst C,c,$(BSAT_7364_VER_LOWER))

# Source needs to know which revision
BSAT_DEFINES += BSAT_7364_VER=BCHP_VER_$(BSAT_7364_VER_UPPER)

BSAT_DEFINES += BYPASS_LEAP
BSAT_INCLUDES += \
	$(MAGNUM)/portinginterface/sat/include/7364 \
	$(MAGNUM)/portinginterface/sat/include/g1 \
	$(MAGNUM)/portinginterface/sat/src/g1/common \
	$(MAGNUM)/portinginterface/sat/src/g1/7364

BSAT_SOURCES += \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_acq.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_afec.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_chan.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_cwc.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_dft.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_hp.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_misc.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_plc.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_qpsk.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_snr.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_tfec.c \
	$(MAGNUM)/portinginterface/sat/src/g1/common/bsat_g1_priv_timer.c \
	$(MAGNUM)/portinginterface/sat/src/g1/7364/bsat_7364_priv.c \
	$(MAGNUM)/portinginterface/sat/src/g1/7364/bsat_7364_priv_aci.c

endif
