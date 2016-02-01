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
# N/A
#
# BADS_3117_SUPPORT - If defined, support will be added for the 3117 ADS 
# BADS_3109_SUPPORT - If defined, support will be added for the 3109 ADS

# Define this module
MAGNUM_MODULES += BADS

# If the chip version is not specified, select a default
ifeq ($(BCHP_312X_VER),)
BCHP_312X_VER := C0
endif

ifeq ($(BCHP_3461_VER),)
BCHP_3461_VER := B0
endif

ifeq ($(BCHP_3462_VER),)
BCHP_3462_VER := A0
endif

ifeq ($(BCHP_7584_VER),)
BCHP_7584_VER := A0
endif

BADS_312x_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_312X_VER)")}')
BADS_3461_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_3461_VER)")}')
BADS_3462_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_3462_VER)")}')

# Source needs to know which revision
BADS_DEFINES += BADS_312X_VER=BCHP_VER_$(BCHP_312X_VER)
BADS_DEFINES += BADS_3461_VER=BCHP_VER_$(BCHP_3461_VER)
BADS_DEFINES += BADS_3462_VER=BCHP_VER_$(BCHP_3462_VER)
BADS_DEFINES += BADS_7584_VER=BCHP_VER_$(BCHP_7584_VER)

# Module Includes
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/include
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/common
BADS_INCLUDES += $(MAGNUM)/commonutils/fec/include

# Module Sources
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/common/bads.c

ifneq ($(BADS_3461_SUPPORT),)
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_leap_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/3461/bads_3461.c
endif

ifneq ($(BADS_3462_SUPPORT),)
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_leap_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/3462/bads_3462.c
endif

ifneq ($(BADS_3128_SUPPORT),)
BADS_DEFINES += BADS_3128_SUPPORT
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_leap_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/3128/bads_3128.c
endif

ifneq ($(BADS_7584_SUPPORT),)
BADS_DEFINES += BADS_7584_SUPPORT
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/bads_leap_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/leap/7584/bads_7584.c
endif

ifneq ($(BADS_3109_SUPPORT),)
BADS_31XX_CHIP := 3109
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/3109/bads_3109.c
endif

ifneq ($(BADS_3112_SUPPORT),)
BADS_31XX_CHIP := 3112
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/3112/bads_3112.c
endif
 
ifneq ($(BADS_3114_SUPPORT),)
BADS_31XX_CHIP := 3114
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/3114/bads_3114.c
endif
 
ifneq ($(BADS_3117_SUPPORT),)
BADS_31XX_CHIP := 3117
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/3117/bads_3117.c
endif

ifneq ($(BADS_31XX_CHIP),)
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/include/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/8051/bads_31xx_priv.c
endif

ifneq ($(BADS_7550_SUPPORT),)
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/7550/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550_api.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550_acquire.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550_status.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7550/bads_7550_utils.c
endif



ifneq ($(BADS_35233_SUPPORT),)
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/35233/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/7552/bads_coef.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_3x7x.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_acquire.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_status.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_utils.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/35233/bads_api.c
endif

ifneq ($(BADS_7552_SUPPORT),)
ifneq ($(BADS_7552_ANNEXA_BURST_SUPPORT),)
BADS_DEFINES += BADS_ANNEXA_BURST_SUPPORT
endif
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/7552/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/bads_3x7x_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_coef.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_7552.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_acquire.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_status.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7552/bads_utils.c
endif

ifneq ($(BADS_7125_SUPPORT),)
BADS_INCLUDES += $(MAGNUM)/portinginterface/ads/src/soc/7125/
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125_priv.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125_acquire.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125_status.c
BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/soc/7125/bads_7125_utils.c
endif

ifeq ($(BADS_CHIP),3255)
    BADS_SOURCES += $(MAGNUM)/portinginterface/ads/src/docsis/bads_docsis_priv.c
endif
 
