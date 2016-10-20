############################################################
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
# Module Description:
#
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BWFE

# Add dependency on BFEC
BWFE_INCLUDES += \
   $(MAGNUM)/commonutils/fec/include \
	$(MAGNUM)/portinginterface/wfe/include/common \
	$(MAGNUM)/portinginterface/wfe/src/common

BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/common/bwfe.c


##############################
# Handle 4538
##############################
ifeq ($(findstring 4538,$(BWFE_CHIP)),4538)

# If chip version not specified, select a default
ifeq ($(BWFE_4538_VER),)
BWFE_4538_VER := A0
endif

BWFE_4538_VER_UPPER := $(strip $(BWFE_4538_VER))
BWFE_4538_VER_UPPER := $(subst a,A,$(BWFE_4538_VER_UPPER))
BWFE_4538_VER_UPPER := $(subst b,B,$(BWFE_4538_VER_UPPER))
BWFE_4538_VER_UPPER := $(subst c,C,$(BWFE_4538_VER_UPPER))

BWFE_4538_VER_LOWER := $(strip $(BWFE_4538_VER))
BWFE_4538_VER_LOWER := $(subst A,a,$(BWFE_4538_VER_LOWER))
BWFE_4538_VER_LOWER := $(subst B,b,$(BWFE_4538_VER_LOWER))
BWFE_4538_VER_LOWER := $(subst C,c,$(BWFE_4538_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_4538_VER=BCHP_VER_$(BWFE_4538_VER_UPPER)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/4538 \
	$(MAGNUM)/portinginterface/wfe/src/4538

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/4538/bwfe_4538.c \
	$(MAGNUM)/portinginterface/wfe/src/4538/bwfe_4538_priv.c

endif


##############################
# Handle 7366
##############################
ifeq ($(findstring 7366,$(BWFE_CHIP)),7366)

# If chip version not specified, select a default
ifeq ($(BWFE_7366_VER),)
BWFE_7366_VER := A0
endif

BWFE_7366_VER_UPPER := $(strip $(BWFE_7366_VER))
BWFE_7366_VER_UPPER := $(subst a,A,$(BWFE_7366_VER_UPPER))
BWFE_7366_VER_UPPER := $(subst b,B,$(BWFE_7366_VER_UPPER))
BWFE_7366_VER_UPPER := $(subst c,C,$(BWFE_7366_VER_UPPER))

BWFE_7366_VER_LOWER := $(strip $(BWFE_7366_VER))
BWFE_7366_VER_LOWER := $(subst A,a,$(BWFE_7366_VER_LOWER))
BWFE_7366_VER_LOWER := $(subst B,b,$(BWFE_7366_VER_LOWER))
BWFE_7366_VER_LOWER := $(subst C,c,$(BWFE_7366_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_7366_VER=BCHP_VER_$(BWFE_7366_VER_UPPER)

ifeq ($(BYPASS_LEAP),1)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/g3 \
	$(MAGNUM)/portinginterface/wfe/src/g3/common \
	$(MAGNUM)/portinginterface/wfe/src/g3/7366

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/common/bwfe_priv.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_adc.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_corr.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_lic.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_rffe.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/7366/bwfe_7366_priv.c

else

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/7366 \
	$(MAGNUM)/portinginterface/wfe/src/7366

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/7366/bwfe_7366.c \
	$(MAGNUM)/portinginterface/wfe/src/7366/bwfe_7366_priv.c

endif

endif


##############################
# Handle 45216
##############################
ifeq ($(findstring 45216,$(BWFE_CHIP)),45216)

# If chip version not specified, select a default
ifeq ($(BWFE_45216_VER),)
BWFE_45216_VER := A0
endif

BWFE_45216_VER_UPPER := $(strip $(BWFE_45216_VER))
BWFE_45216_VER_UPPER := $(subst a,A,$(BWFE_45216_VER_UPPER))
BWFE_45216_VER_UPPER := $(subst b,B,$(BWFE_45216_VER_UPPER))
BWFE_45216_VER_UPPER := $(subst c,C,$(BWFE_45216_VER_UPPER))

BWFE_45216_VER_LOWER := $(strip $(BWFE_45216_VER))
BWFE_45216_VER_LOWER := $(subst A,a,$(BWFE_45216_VER_LOWER))
BWFE_45216_VER_LOWER := $(subst B,b,$(BWFE_45216_VER_LOWER))
BWFE_45216_VER_LOWER := $(subst C,c,$(BWFE_45216_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_45216_VER=BCHP_VER_$(BWFE_45216_VER_UPPER)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/45216 \
	$(MAGNUM)/portinginterface/wfe/src/45216

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/45216/bwfe_45216.c \
	$(MAGNUM)/portinginterface/wfe/src/45216/bwfe_45216_priv.c

endif


##############################
# Handle 45308
##############################
ifeq ($(findstring 45308,$(BWFE_CHIP)),45308)

# If chip version not specified, select a default
ifeq ($(BWFE_45308_VER),)
BWFE_45308_VER := A0
endif

BWFE_45308_VER_UPPER := $(strip $(BWFE_45308_VER))
BWFE_45308_VER_UPPER := $(subst a,A,$(BWFE_45308_VER_UPPER))
BWFE_45308_VER_UPPER := $(subst b,B,$(BWFE_45308_VER_UPPER))
BWFE_45308_VER_UPPER := $(subst c,C,$(BWFE_45308_VER_UPPER))

BWFE_45308_VER_LOWER := $(strip $(BWFE_45308_VER))
BWFE_45308_VER_LOWER := $(subst A,a,$(BWFE_45308_VER_LOWER))
BWFE_45308_VER_LOWER := $(subst B,b,$(BWFE_45308_VER_LOWER))
BWFE_45308_VER_LOWER := $(subst C,c,$(BWFE_45308_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_45308_VER=BCHP_VER_$(BWFE_45308_VER_UPPER)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/45308 \
	$(MAGNUM)/portinginterface/wfe/src/common \
	$(MAGNUM)/portinginterface/wfe/src/45308

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/45308/bwfe_45308.c \
	$(MAGNUM)/portinginterface/wfe/src/45308/bwfe_45308_priv.c

endif



##############################
# Handle 7364
##############################
ifeq ($(findstring 7364,$(BWFE_CHIP)),7364)

# If chip version not specified, select a default
ifeq ($(BWFE_7364_VER),)
BWFE_7364_VER := C0
endif

BWFE_7364_VER_UPPER := $(strip $(BWFE_7364_VER))
BWFE_7364_VER_UPPER := $(subst a,A,$(BWFE_7364_VER_UPPER))
BWFE_7364_VER_UPPER := $(subst b,B,$(BWFE_7364_VER_UPPER))
BWFE_7364_VER_UPPER := $(subst c,C,$(BWFE_7364_VER_UPPER))

BWFE_7364_VER_LOWER := $(strip $(BWFE_7364_VER))
BWFE_7364_VER_LOWER := $(subst A,a,$(BWFE_7364_VER_LOWER))
BWFE_7364_VER_LOWER := $(subst B,b,$(BWFE_7364_VER_LOWER))
BWFE_7364_VER_LOWER := $(subst C,c,$(BWFE_7364_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_7364_VER=BCHP_VER_$(BWFE_7364_VER_UPPER)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/g3 \
	$(MAGNUM)/portinginterface/wfe/src/g3/common \
	$(MAGNUM)/portinginterface/wfe/src/g3/7364 \

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/common/bwfe_priv.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_adc.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_corr.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_lic.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_rffe.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/7364/bwfe_7364_priv.c

endif
