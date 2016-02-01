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
# Created:
#
# $brcm_Log: $
#
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BTFE

# Determine which chip we are using for tfe

#
# 7364
#
ifeq ($(findstring $(BCHP_CHIP), 7364), $(BCHP_CHIP))
BTFE_INCLUDES += \
    $(MAGNUM)/portinginterface/tfe/include/ \
    $(MAGNUM)/basemodules/hab/include/common \
    $(MAGNUM)/commonutils/fec/include/

BTFE_DEFINES += USE_DFE_ADDR_32BIT
BTFE_SOURCES += \
    $(MAGNUM)/portinginterface/tfe/src/btfe.c
endif

#
# 7543
#
ifeq ($(findstring $(BCHP_CHIP), 7543), $(BCHP_CHIP))
BTFE_INCLUDES += \
    $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/a0 \
    $(MAGNUM)/portinginterface/tfe/include/ \
    $(MAGNUM)/commonutils/fec/include/

BTFE_INCLUDES += $(MAGNUM)/portinginterface/tfe/src/$(BCHP_CHIP)/firmware/$(BCHP_VER)

BTFE_DEFINES += USE_DFE_ADDR_32BIT
BTFE_SOURCES += \
    $(MAGNUM)/portinginterface/tfe/src/btfe.c \
    $(MAGNUM)/portinginterface/tfe/src/btfe_scd_priv.c \
    $(MAGNUM)/portinginterface/tfe/src/btfe_scd_hal_mips_priv.c \
    $(MAGNUM)/portinginterface/tfe/src/btfe_scd_common_func_priv.c \
    $(MAGNUM)/portinginterface/tfe/src/btfe_scd_chip_priv.c
endif

