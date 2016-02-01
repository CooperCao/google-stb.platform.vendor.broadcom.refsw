############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#
# BCHP_7411_VER - If defined (in lower-case), specifies the revision of the 7411 chip to be supported.
# BCHP_QAM_CHIP - If defined, specifies the revision of the QAM chip to be supported.
# BINT_ENABLE_IPI - If defined, IPI (inter-processor interrupts) are supported.  Not compatible with SMP Linux.
#

# Define this module
MAGNUM_MODULES += BINT

# Module Includes
BINT_INCLUDES += $(MAGNUM)/basemodules/int
BINT_INCLUDES += $(MAGNUM)/basemodules/int/$(BCHP_CHIP)

# Module Sources
BINT_SOURCES += $(MAGNUM)/basemodules/int/bint.c
BINT_SOURCES += $(MAGNUM)/basemodules/int/bint_dump.c
BINT_SOURCES += $(MAGNUM)/basemodules/int/$(BCHP_CHIP)/bint_$(BCHP_CHIP).c

#
# Handle conditional options
#
ifneq ($(BCHP_7411_VER),)
# 7411 support
BINT_INCLUDES += $(MAGNUM)/basemodules/int/7411
BINT_SOURCES += $(MAGNUM)/basemodules/int/7411/bint_7411.c
endif

ifeq ($(BAST_AFEC_CORE_DISABLED),y)
BINT_DEFINES += BAST_AFEC_CORE_DISABLED
endif

ifeq ($(BINT_AVD1_DISABLED),y)
BINT_DEFINES += BINT_AVD1_DISABLED
endif

ifneq ($(BCHP_QAM_CHIP),)
BINT_INCLUDES += $(MAGNUM)/basemodules/int
BINT_INCLUDES += $(MAGNUM)/basemodules/int/$(BCHP_QAM_CHIP)

BINT_SOURCES += $(MAGNUM)/basemodules/int/$(BCHP_QAM_CHIP)/bint_$(BCHP_QAM_CHIP).c
endif

ifneq ($(BENC_CHIP),)
# 7043 support
BINT_INCLUDES += $(MAGNUM)/basemodules/int/$(BENC_CHIP)
BINT_SOURCES += $(MAGNUM)/basemodules/int/$(BENC_CHIP)/bint_$(BENC_CHIP).c
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
