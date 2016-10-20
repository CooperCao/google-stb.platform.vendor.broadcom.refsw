##############################################################################
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
#############################################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BDSQ

# Add dependency on BFEC
BDSQ_INCLUDES += \
   $(MAGNUM)/commonutils/fec/include \
	$(MAGNUM)/portinginterface/dsq/include/common \
	$(MAGNUM)/portinginterface/dsq/src/common

BDSQ_SOURCES += \
	$(MAGNUM)/portinginterface/dsq/src/common/bdsq.c


##############################
# Handle 7366
##############################
ifeq ($(findstring 7366,$(BDSQ_CHIP)),7366)

# If chip version not specified, select a default
ifeq ($(BDSQ_7366_VER),)
BDSQ_7366_VER := A0
endif

BDSQ_7366_VER_UPPER := $(strip $(BDSQ_7366_VER))
BDSQ_7366_VER_UPPER := $(subst a,A,$(BDSQ_7366_VER_UPPER))
BDSQ_7366_VER_UPPER := $(subst b,B,$(BDSQ_7366_VER_UPPER))
BDSQ_7366_VER_UPPER := $(subst c,C,$(BDSQ_7366_VER_UPPER))

BDSQ_7366_VER_LOWER := $(strip $(BDSQ_7366_VER))
BDSQ_7366_VER_LOWER := $(subst A,a,$(BDSQ_7366_VER_LOWER))
BDSQ_7366_VER_LOWER := $(subst B,b,$(BDSQ_7366_VER_LOWER))
BDSQ_7366_VER_LOWER := $(subst C,c,$(BDSQ_7366_VER_LOWER))

# Source needs to know which revision
BDSQ_DEFINES += BDSQ_7366_VER=BCHP_VER_$(BDSQ_7366_VER_UPPER)

ifeq ($(BYPASS_LEAP),1)

# Module Includes
BDSQ_INCLUDES += \
	$(MAGNUM)/portinginterface/dsq/include/g1 \
	$(MAGNUM)/portinginterface/dsq/src/g1/common \
	$(MAGNUM)/portinginterface/dsq/src/g1/7366

# Module Sources
BDSQ_SOURCES += \
	$(MAGNUM)/portinginterface/dsq/src/common/bdsq_priv.c \
	$(MAGNUM)/portinginterface/dsq/src/g1/common/bdsq_g1.c \
	$(MAGNUM)/portinginterface/dsq/src/g1/common/bdsq_g1_priv.c \
	$(MAGNUM)/portinginterface/dsq/src/g1/common/bdsq_g1_priv_txn.c \
	$(MAGNUM)/portinginterface/dsq/src/g1/7366/bdsq_7366_priv.c

else

# Module Includes
BDSQ_INCLUDES += \
	$(MAGNUM)/portinginterface/dsq/include/7366 \
	$(MAGNUM)/portinginterface/dsq/src/7366

# Module Sources
BDSQ_SOURCES += \
	$(MAGNUM)/portinginterface/dsq/src/7366/bdsq_7366.c \
	$(MAGNUM)/portinginterface/dsq/src/7366/bdsq_7366_priv.c

endif

endif


##############################
# Handle 45216
##############################
ifeq ($(findstring 45216,$(BDSQ_CHIP)),45216)

# If chip version not specified, select a default
ifeq ($(BDSQ_45216_VER),)
BDSQ_45216_VER := A0
endif

BDSQ_45216_VER_UPPER := $(strip $(BDSQ_45216_VER))
BDSQ_45216_VER_UPPER := $(subst a,A,$(BDSQ_45216_VER_UPPER))
BDSQ_45216_VER_UPPER := $(subst b,B,$(BDSQ_45216_VER_UPPER))
BDSQ_45216_VER_UPPER := $(subst c,C,$(BDSQ_45216_VER_UPPER))

BDSQ_45216_VER_LOWER := $(strip $(BDSQ_45216_VER))
BDSQ_45216_VER_LOWER := $(subst A,a,$(BDSQ_45216_VER_LOWER))
BDSQ_45216_VER_LOWER := $(subst B,b,$(BDSQ_45216_VER_LOWER))
BDSQ_45216_VER_LOWER := $(subst C,c,$(BDSQ_45216_VER_LOWER))

# Source needs to know which revision
BDSQ_DEFINES += BDSQ_45216_VER=BCHP_VER_$(BDSQ_45216_VER_UPPER)

# Module Includes
BDSQ_INCLUDES += \
	$(MAGNUM)/portinginterface/dsq/include/45216 \
	$(MAGNUM)/portinginterface/dsq/src/45216

# Module Sources
BDSQ_SOURCES += \
	$(MAGNUM)/portinginterface/dsq/src/45216/bdsq_45216.c \
	$(MAGNUM)/portinginterface/dsq/src/45216/bdsq_45216_priv.c

endif


##############################
# Handle 45308
##############################
ifeq ($(findstring 45308,$(BDSQ_CHIP)),45308)

# If chip version not specified, select a default
ifeq ($(BDSQ_45308_VER),)
BDSQ_45308_VER := A0
endif

BDSQ_45308_VER_UPPER := $(strip $(BDSQ_45308_VER))
BDSQ_45308_VER_UPPER := $(subst a,A,$(BDSQ_45308_VER_UPPER))
BDSQ_45308_VER_UPPER := $(subst b,B,$(BDSQ_45308_VER_UPPER))
BDSQ_45308_VER_UPPER := $(subst c,C,$(BDSQ_45308_VER_UPPER))

BDSQ_45308_VER_LOWER := $(strip $(BDSQ_45308_VER))
BDSQ_45308_VER_LOWER := $(subst A,a,$(BDSQ_45308_VER_LOWER))
BDSQ_45308_VER_LOWER := $(subst B,b,$(BDSQ_45308_VER_LOWER))
BDSQ_45308_VER_LOWER := $(subst C,c,$(BDSQ_45308_VER_LOWER))

# Source needs to know which revision
BDSQ_DEFINES += BDSQ_45308_VER=BCHP_VER_$(BDSQ_45308_VER_UPPER)

# Module Includes
BDSQ_INCLUDES += \
	$(MAGNUM)/portinginterface/dsq/include/45308 \
	$(MAGNUM)/portinginterface/dsq/src/45308

# Module Sources
BDSQ_SOURCES += \
	$(MAGNUM)/portinginterface/dsq/src/45308/bdsq_45308.c \
	$(MAGNUM)/portinginterface/dsq/src/45308/bdsq_45308_priv.c

endif



##############################
# Handle 7364
##############################
ifeq ($(findstring 7364,$(BDSQ_CHIP)),7364)

# If chip version not specified, select a default
ifeq ($(BDSQ_7364_VER),)
BDSQ_7364_VER := C0
endif

BDSQ_7364_VER_UPPER := $(strip $(BDSQ_7364_VER))
BDSQ_7364_VER_UPPER := $(subst a,A,$(BDSQ_7364_VER_UPPER))
BDSQ_7364_VER_UPPER := $(subst b,B,$(BDSQ_7364_VER_UPPER))
BDSQ_7364_VER_UPPER := $(subst c,C,$(BDSQ_7364_VER_UPPER))

BDSQ_7364_VER_LOWER := $(strip $(BDSQ_7364_VER))
BDSQ_7364_VER_LOWER := $(subst A,a,$(BDSQ_7364_VER_LOWER))
BDSQ_7364_VER_LOWER := $(subst B,b,$(BDSQ_7364_VER_LOWER))
BDSQ_7364_VER_LOWER := $(subst C,c,$(BDSQ_7364_VER_LOWER))

# Source needs to know which revision
BDSQ_DEFINES += BDSQ_7364_VER=BCHP_VER_$(BDSQ_7364_VER_UPPER)

# Module Includes
BDSQ_INCLUDES += \
	$(MAGNUM)/portinginterface/dsq/include/g1 \
	$(MAGNUM)/portinginterface/dsq/src/g1/common \
	$(MAGNUM)/portinginterface/dsq/src/g1/7364

# Module Sources
BDSQ_SOURCES += \
	$(MAGNUM)/portinginterface/dsq/src/common/bdsq_priv.c \
	$(MAGNUM)/portinginterface/dsq/src/g1/common/bdsq_g1.c \
	$(MAGNUM)/portinginterface/dsq/src/g1/common/bdsq_g1_priv.c \
	$(MAGNUM)/portinginterface/dsq/src/g1/common/bdsq_g1_priv_txn.c \
	$(MAGNUM)/portinginterface/dsq/src/g1/7364/bdsq_7364_priv.c

endif
