#############################################################
#  Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
# BINT_ENABLE_IPI - If defined, IPI (inter-processor interrupts) are supported.  Not compatible with SMP Linux.
#

# Define this module
MAGNUM_MODULES += BINT

# Module Includes
BINT_INCLUDES += $(MAGNUM)/basemodules/int
BINT_INCLUDES += $(MAGNUM)/basemodules/int/$(BCHP_CHIP)

# Module Sources
BINT_SOURCES += $(MAGNUM)/basemodules/int/bint.c
BINT_SOURCES += $(MAGNUM)/basemodules/int/$(BCHP_CHIP)/bint_$(BCHP_CHIP).c

#
# Handle conditional options
#
ifeq ($(BAST_AFEC_CORE_DISABLED),y)
BINT_DEFINES += BAST_AFEC_CORE_DISABLED
endif

ifeq ($(BINT_AVD1_DISABLED),y)
BINT_DEFINES += BINT_AVD1_DISABLED
endif

ifeq ($(BINT_ENABLE_IPI),y)
BINT_DEFINES += BINT_ENABLE_IPI=1
endif

ifeq ($(BINT_OPEN_BYPASS_L2INIT), y)
BINT_DEFINES += BINT_OPEN_BYPASS_L2INIT=1
endif

ifeq ($(BINT_VICE2_DISABLED),y)
BINT_DEFINES += BINT_VICE2_DISABLED
endif

ifeq ($(BINT_DECODER_DISABLED),y)
BINT_DEFINES += BINT_DECODER_DISABLED
endif

ifeq ($(BINT_DISPLAY_DISABLED),y)
BINT_DEFINES += BINT_DISPLAY_DISABLED
endif

ifeq ($(BINT_RAAGA_DISABLED),y)
BINT_DEFINES += BINT_RAAGA_DISABLED
endif

ifeq ($(BCHP_DISABLE_IRQ0_SPI),y)
# For 3548/3556, the linux kernel will access IRQ0_SPI_IRQEN for SPI flash devices.
# If you are using SPI flash, you must export BCHP_DISABLE_IRQ0_SPI=y to prevent Magnum
# from disabling this interrupt. If you need magnum and kernel acess to SPI, you cannot enable
# this code. Instead use the linux kernel's spidev techinque. See SW3548-2904 for details.
BINT_DEFINES += BCHP_DISABLE_IRQ0_SPI=1
endif
