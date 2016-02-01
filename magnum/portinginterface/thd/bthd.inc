############################################################
#     Copyright (c) 2003-2012, Broadcom Corporation
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

# Module Includes
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/include/common
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/src/common
BTHD_SOURCES +=	$(MAGNUM)/portinginterface/thd/src/common/bthd.c

#
# Handle 3461
#
ifeq ($(findstring 3461,$(BTHD_CHIP)),3461)
BTHD_INCLUDES += $(MAGNUM)/basemodules/chp/include/3461/common
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
# Handle 3556
#
ifeq ($(BCHP_CHIP),3556)
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/3556
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/

ifneq ($(BTHD_POWER_MANAGEMENT),n)
    BTHD_DEFINES += BTHD_POWER_MANAGEMENT
endif

BTHD_SOURCES += \
	$(MAGNUM)/portinginterface/thd/3556/bthd_3556.c \
	$(MAGNUM)/portinginterface/thd/3556/bthd_3556_priv.c \
	$(MAGNUM)/portinginterface/thd/3556/bthd_3556_ifdcoef.c \
	$(MAGNUM)/portinginterface/thd/3556/bthd_3556_ifd.c \
	$(MAGNUM)/portinginterface/thd/3556/microcode/$(BCHP_VER_UPPER)/bcm3556ap.c
endif

#
# Handle 7550
#

ifeq ($(BCHP_CHIP),7550)
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/src/7550
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/include/7550

ifneq ($(BTHD_POWER_MANAGEMENT),n)
    BTHD_DEFINES += BTHD_POWER_MANAGEMENT
endif

BTHD_SOURCES += \
	$(MAGNUM)/portinginterface/thd/src/7550/bthd_7550.c \
	$(MAGNUM)/portinginterface/thd/src/7550/bthd_7550_priv.c \
	$(MAGNUM)/portinginterface/thd/src/7550/bthd_7550_platform.c
endif



#
# Handle 35233
#

ifeq ($(BCHP_CHIP),35233)
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/35233
BTHD_INCLUDES += $(MAGNUM)/portinginterface/thd/

ifneq ($(BTHD_POWER_MANAGEMENT),n)
    BTHD_DEFINES += BTHD_POWER_MANAGEMENT
endif


BTHD_DEFINES += BTHD_ISDBT_SUPPORT
BTHD_SOURCES += $(MAGNUM)/portinginterface/thd/35233/bthd_acq_isdbt.c


BTHD_SOURCES += \
	$(MAGNUM)/portinginterface/thd/35233/bthd_coef.c \
	$(MAGNUM)/portinginterface/thd/35233/bthd_3x7x.c \
	$(MAGNUM)/portinginterface/thd/35233/bthd_acq_dvbt.c \
	$(MAGNUM)/portinginterface/thd/35233/bthd_acq.c \
	$(MAGNUM)/portinginterface/thd/35233/bthd_api.c
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


