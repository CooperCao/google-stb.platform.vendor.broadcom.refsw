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
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#
# BVSB_CHIP - Defines support for the external VSB chip to be used.  Should be either 3510 or 3520.  If not defined, assumed to be internal.
#

# Define this module
MAGNUM_MODULES += BVSB

# Determine which chip we are using for vsb
ifeq ($(BVSB_CHIP),3510)
 #
 # 3510 Support uses a different filename scheme.
 #
 BVSB_INCLUDES += $(MAGNUM)/basemodules/chp/include/3510/common
 BVSB_INCLUDES += \
	$(MAGNUM)/portinginterface/vsb/include/3510 \
	$(MAGNUM)/portinginterface/vsb/src/3510 

 BVSB_SOURCES += \
	$(MAGNUM)/portinginterface/vsb/src/3510/bvsb.c \
	$(MAGNUM)/portinginterface/vsb/src/3510/bvsb_priv.c
else 
   #
   # Handle 3520, 3560, 3563, 3548
   #
   ifneq ($(BVSB_CHIP),3520)
     BVSB_CHIP := ${BCHP_CHIP}
   endif
   # Module Includes
   BVSB_INCLUDES += \
     $(MAGNUM)/portinginterface/vsb/include/common \
     $(MAGNUM)/portinginterface/vsb/src/common \
     $(MAGNUM)/portinginterface/vsb/include/$(BVSB_CHIP) \
     $(MAGNUM)/basemodules/chp/include/$(BVSB_CHIP)/common
 
   # Module Sources
   BVSB_SOURCES += \
     $(MAGNUM)/portinginterface/vsb/src/common/bvsb.c \
     $(MAGNUM)/portinginterface/vsb/src/$(BVSB_CHIP)/bvsb_$(BVSB_CHIP).c \
     $(MAGNUM)/portinginterface/vsb/src/$(BVSB_CHIP)/bvsb_$(BVSB_CHIP)_priv.c 

     #starting with 3548, the firmware is compiled at magnum level
     ifeq ($(BVSB_CHIP),3548)
        ifneq ($(BVSB_POWER_MANAGEMENT),n)
           BVSB_DEFINES += BVSB_POWER_MANAGEMENT
        endif

        BVSB_SOURCES += \
         $(MAGNUM)/portinginterface/vsb/src/3548/bvsb_3548_ifd.c \
         $(MAGNUM)/portinginterface/vsb/src/3548/bvsb_3548_ifdcoef.c \
         $(MAGNUM)/portinginterface/vsb/src/3548/bcm3548ap_bx.c
     endif
endif
