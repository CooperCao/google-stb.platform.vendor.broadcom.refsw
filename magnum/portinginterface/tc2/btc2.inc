############################################################
#     Copyright (c) 2005-2012, Broadcom Corporation
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
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#
# BTC2_3117_SUPPORT - If defined, support will be added for the 3117 TC2 


# Define this module
MAGNUM_MODULES += BTC2

# If the chip version is not specified, select a default
ifeq ($(BCHP_3461_VER),)
BCHP_3461_VER := A0
endif

# Source needs to know which revision
BTC2_DEFINES += BTC2_3461_VER=BCHP_VER_$(BCHP_3461_VER)

# Module Includes
BTC2_INCLUDES += $(MAGNUM)/portinginterface/tc2/include
BTC2_INCLUDES += $(MAGNUM)/portinginterface/tc2/src/common
BTC2_INCLUDES += $(MAGNUM)/commonutils/fec/include

# Module Sources
BTC2_SOURCES +=	$(MAGNUM)/portinginterface/tc2/src/common/btc2.c


ifneq ($(BTC2_3461_SUPPORT),)
BTC2_INCLUDES += $(MAGNUM)/basemodules/chp/3461
BTC2_INCLUDES += $(MAGNUM)/portinginterface/tc2/src/3461
BTC2_SOURCES += \
	$(MAGNUM)/portinginterface/tc2/src/3461/btc2_3461.c \
	$(MAGNUM)/portinginterface/tc2/src/3461/btc2_3461_priv.c
endif
 
