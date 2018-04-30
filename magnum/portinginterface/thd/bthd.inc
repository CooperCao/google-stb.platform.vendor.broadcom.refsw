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
############################################################
#
# Conditional Options for this Makefile
#
# BTHD_CHIP - Specifies a THD chip number to include in the build (e.g. 2940).  May specify multiple chips.
#

# Define this module
MAGNUM_MODULES += BTHD

# If the chip version is not specified, select a default
ifeq ($(BCHP_3461_VER),)
BCHP_3461_VER := A0
endif

# Source needs to know which revision
BTHD_DEFINES += BTHD_3461_VER=BCHP_VER_$(BCHP_3461_VER)

BCHP_VER_UPPER ?= $(shell awk 'BEGIN{print toupper("$(BCHP_VER)")}')
BTHD_3461_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_3461_VER)")}')

# Module Includes
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/include/common
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/src/common
BTHD_SOURCES +=	$(MAGNUM)/portinginterface/thd/src/common/bthd.c

#
# Handle 3461
#
ifeq ($(findstring 3461,$(BTHD_CHIP)),3461)
BTHD_INCLUDES += $(MAGNUM)/basemodules/chp/include/3461/hab/$(BTHD_3461_VER_LOWER)
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/include/3461
BTHD_INCLUDES += $(MAGNUM)/commonutils/fec/include

BTHD_SOURCES += \
	$(MAGNUM)/portinginterface/thd/src/3461/bthd_3461.c \
	$(MAGNUM)/portinginterface/thd/src/3461/bthd_3461_priv.c 
endif

#
# Handle 2940
#
ifeq ($(findstring 2940,$(BTHD_CHIP)),2940)
BTHD_INCLUDES += $(MAGNUM)/basemodules/chp/include/2940/common
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/2940

BTHD_SOURCES += \
	$(MAGNUM)/portinginterface/thd/2940/bthd_2940.c \
	$(MAGNUM)/portinginterface/thd/2940/bthd_2940_priv.c 
endif

#
# Handle 7552
#

ifeq ($(BCHP_CHIP),7552)
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/src/3x7x
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/include/3x7x

ifneq ($(BTHD_POWER_MANAGEMENT),n)
    BTHD_DEFINES += BTHD_POWER_MANAGEMENT
endif

BTHD_DEFINES += BTHD_ISDBT_SUPPORT
BTHD_SOURCES += $(MAGNUM)/portinginterface/thd/src/3x7x/bthd_acq_isdbt.c

BTHD_SOURCES += \
	$(MAGNUM)/portinginterface/thd/src/3x7x/bthd_coef.c \
	$(MAGNUM)/portinginterface/thd/src/3x7x/bthd_3x7x.c \
	$(MAGNUM)/portinginterface/thd/src/3x7x/bthd_acq_dvbt.c \
	$(MAGNUM)/portinginterface/thd/src/3x7x/bthd_acq.c \
	$(MAGNUM)/portinginterface/thd/src/3x7x/bthd_api.c
endif
