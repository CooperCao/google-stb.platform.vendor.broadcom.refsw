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
# BODS_3462_SUPPORT - If defined, support will be added for the 3462 ODS

# Define this module
MAGNUM_MODULES += BODS

# If the chip version is not specified, select a default
ifeq ($(BCHP_3462_VER),)
BCHP_3462_VER := A0
endif

# Source needs to know which revision
BODS_DEFINES += BODS_3462_VER=BCHP_VER_$(BCHP_3462_VER)

# Module Includes
BODS_INCLUDES += $(MAGNUM)/portinginterface/ods/include
BODS_INCLUDES += $(MAGNUM)/portinginterface/ods/src/common
BODS_INCLUDES += $(MAGNUM)/portinginterface/ods/src/leap
BODS_INCLUDES += $(MAGNUM)/commonutils/fec/include

# Module Sources
BODS_SOURCES +=	$(MAGNUM)/portinginterface/ods/src/common/bods.c

ifneq ($(BODS_7364_SUPPORT),)
BODS_DEFINES += BODS_CHIP=7364
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
    $(MAGNUM)/portinginterface/ods/src/leap/7364/bods_7364.c
endif

ifneq ($(BODS_7563_SUPPORT),)
BODS_DEFINES += BODS_CHIP=7563
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
    $(MAGNUM)/portinginterface/ods/src/leap/7563/bods_7563.c
endif

ifneq ($(BODS_75635_SUPPORT),)
BODS_DEFINES += BODS_CHIP=7563
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
    $(MAGNUM)/portinginterface/ods/src/leap/7563/bods_7563.c
endif

ifneq ($(BODS_3472_SUPPORT),)
BODS_INCLUDES += $(MAGNUM)/basemodules/chp/3472
BODS_DEFINES += BODS_CHIP=3472
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
    $(MAGNUM)/portinginterface/ods/src/leap/3472/bods_3472.c
endif

ifneq ($(BODS_3462_SUPPORT),)
BODS_INCLUDES += $(MAGNUM)/basemodules/chp/3462
BODS_DEFINES += BODS_CHIP=3462
BODS_SOURCES += \
    $(MAGNUM)/portinginterface/ods/src/leap/bods_leap_priv.c \
	$(MAGNUM)/portinginterface/ods/src/leap/3462/bods_3462.c
endif
