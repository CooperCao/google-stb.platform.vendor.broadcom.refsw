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
# BCHP_QAM_CHIP - If defined to a chip number (i.e. 3250), qam support for the specified chip is included.
# BCHP_7411_VER - If defined (in lower-case), specifies the revision of the 7411 chip to be supported.
#

# Define this module
MAGNUM_MODULES += BCHP

#
# CHP and other modules require the definition of the variables BCHP_VER and BCHP_CHIP.
# Note, these must be UPPER-CASE.
#
# For compatiblity, these defines are still required at the application level.
# BCHP_DEFINES := BCHP_CHIP=$(BCHP_CHIP) BCHP_VER=BCHP_VER_$(BCHP_VER)
# #
# # 7411 requires a similar definition.
# #
# ifneq ($(BCHP_7411_VER),)
# BCHP_DEFINES += BCHP_7411_VER=$(BCHP_7411_VER)
# endif

# Make sure we have appropriate lower-case equivalents of BCHP_VER and BCHP_7411_VER
BCHP_VER_LOWER := $(subst A,a,$(BCHP_VER))
BCHP_VER_LOWER := $(subst B,b,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst C,c,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst D,d,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst E,e,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst F,f,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst G,g,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(strip $(subst H,h,$(BCHP_VER_LOWER)))

BCHP_7411_VER_LOWER := $(subst A,a,$(BCHP_7411_VER))
BCHP_7411_VER_LOWER := $(subst B,b,$(BCHP_7411_VER_LOWER))
BCHP_7411_VER_LOWER := $(subst C,c,$(BCHP_7411_VER_LOWER))
BCHP_7411_VER_LOWER := $(subst D,d,$(BCHP_7411_VER_LOWER))
BCHP_7411_VER_LOWER := $(subst E,e,$(BCHP_7411_VER_LOWER))
BCHP_7411_VER_LOWER := $(subst F,f,$(BCHP_7411_VER_LOWER))
BCHP_7411_VER_LOWER := $(subst G,g,$(BCHP_7411_VER_LOWER))
BCHP_7411_VER_LOWER := $(strip $(subst H,h,$(BCHP_7411_VER_LOWER)))

BENC_CHIP_VER_LOWER := $(subst A,a,$(BENC_CHIP_VER))
BENC_CHIP_VER_LOWER := $(subst B,b,$(BENC_CHIP_VER_LOWER))
BENC_CHIP_VER_LOWER := $(subst C,c,$(BENC_CHIP_VER_LOWER))

#
# Always include support for the chip specified by BCHP_CHIP and BCHP_VER
#
BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/include/common
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
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/common/bchp_avs_40nm.c
endif

# 28nm
ifeq ($(filter $(BCHP_CHIP), 7145 7445 7366 7439 74371 7364 7250 7586 11360 7271 7268), $(BCHP_CHIP))
# BCHP_AVS code.  (even) More extensive AVS solution is being included in the newer 28nm parts.
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/common/bchp_avs_28nm.c
# For 28nm and beyond, we have a unified BCHP implementation for the SoC
BCHP_DEFINES += BCHP_UNIFIED_IMPL
endif

# Use the unified approach for 7563[5]/7584[5]/7362[5] to use 5 digit chip IDs.
ifeq ($(filter $(BCHP_CHIP), 7563 75635 75845 7362 73625 74295 75525 73465), $(BCHP_CHIP))
BCHP_DEFINES += BCHP_UNIFIED_IMPL
endif

#
# Handle conditional options
#
ifneq ($(BCHP_QAM_CHIP),)
# 3250 support
BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/include/$(BCHP_QAM_CHIP)/common
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/$(BCHP_QAM_CHIP)/bchp_$(BCHP_QAM_CHIP).c
endif

ifneq ($(BCHP_7411_VER),)
# 7411 chp support
BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/include/7411/common
BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/include/7411/rdb/$(BCHP_7411_VER_LOWER)
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/7411/bchp_7411.c
endif

ifneq ($(BENC_CHIP),)
# 7043 support
BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/include/$(BENC_CHIP)/rdb/$(BENC_CHIP_VER_LOWER)
endif

# SAGE related
ifeq ($(SAGE_SUPPORT),y)
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/common/bchp_sage.c
endif
