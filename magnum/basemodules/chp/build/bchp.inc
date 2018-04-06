#############################################################
#  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
# BCHP_QAM_CHIP - If defined to a chip number (i.e. 3250), qam support for the specified chip is included.
#

# Define this module
MAGNUM_MODULES += BCHP

#
# CHP and other modules require the definition of the variables BCHP_VER and BCHP_CHIP.
# Note, these must be UPPER-CASE.
#
# For compatiblity, these defines are still required at the application level.
# BCHP_DEFINES := BCHP_CHIP=$(BCHP_CHIP) BCHP_VER=BCHP_VER_$(BCHP_VER)

#
# Always include support for the chip specified by BCHP_CHIP and BCHP_VER
#
BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/include/common
BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/include/remap
BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/common

include $(MAGNUM)/basemodules/chp/build/bchp_rdb.inc
BCHP_INCLUDES += ${BCHP_RDB_PATH}

BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/src/common

BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/common/bchp.c
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/$(BCHP_CHIP)/bchp_$(BCHP_CHIP).c

# BCHP_PWR basemodule.
include $(MAGNUM)/basemodules/chp/build/bchp_pwr.inc

# BCHP_AVS code.  More extensive AVS solution is being included in the newer 40nm parts.
ifeq ($(filter $(BCHP_CHIP), 7425 7435 7344 7346 73465 7358 7231 7552 75525 7360 7429 74295 7563 75635 7584 75845 7543 7362 73625 7228), $(BCHP_CHIP))
BCHP_DEFINES += BCHP_40NM
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/common/bchp_avs_40nm.c

# Use the unified approach for 7563[5]/7584[5]/7362[5] to use 5 digit chip IDs.
ifeq ($(filter $(BCHP_CHIP), 7563 75635 75845 7362 73625 74295 75525 73465), $(BCHP_CHIP))
BCHP_DEFINES += BCHP_UNIFIED_IMPL
endif
else
# 28nm
# BCHP_AVS code.  (even) More extensive AVS solution is being included in the newer 28nm parts.
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/common/bchp_avs_28nm.c

BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/common/bchp_reset_common.c
BCHP_DEFINES += BCHP_RESET_COMMON

# For 28nm and beyond, we have a unified BCHP implementation for the SoC
BCHP_DEFINES += BCHP_UNIFIED_IMPL
endif

#
# Handle conditional options
#
# SAGE related
ifeq ($(SAGE_SUPPORT),y)
BCHP_INCLUDES += $(MAGNUM)/syslib/sagelib/include
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/common/bchp_sage.c
BCHP_DEFINES += BCHP_SAGE_SUPPORT
endif
