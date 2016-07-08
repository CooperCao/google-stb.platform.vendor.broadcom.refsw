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
MAGNUM_MODULES += BAOB

# If the chip version is not specified, select a default
ifeq ($(BCHP_312X_VER),)
BCHP_312X_VER := A0
endif

# Source needs to know which revision
BAOB_DEFINES += BAOB_312X_VER=BCHP_VER_$(BCHP_312X_VER)

# Module Includes
BAOB_INCLUDES += $(MAGNUM)/portinginterface/aob/
BAOB_INCLUDES += $(MAGNUM)/commonutils/fec/include

ifeq ($(BAOB_CHIP),3158)
# Module Includes
BAOB_INCLUDES += $(MAGNUM)/portinginterface/aob/include
BAOB_INCLUDES += $(MAGNUM)/basemodules/chp/include/$(BAOB_CHIP)/common
BAOB_DEFINES += BAOB_CHIP=3158

# Module Sources
BAOB_SOURCES += $(MAGNUM)/portinginterface/aob/src/baob.c
else
# Module Includes
BAOB_INCLUDES += $(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)
BAOB_INCLUDES += $(MAGNUM)/basemodules/chp/include/$(BAOB_CHIP)/common

# Module Sources
BAOB_SOURCES += $(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)/baob.c
endif

#
# Handle 7552
#

ifeq ($(BCHP_CHIP),7552)
BAOB_SOURCES += \
	$(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)/baob_acquire.c \
	$(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)/baob_status.c \
	$(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)/baob_utils.c
endif
