############################################################
#     Copyright (c) 2003-2010, Broadcom Corporation
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
#

# Define this module
MAGNUM_MODULES += BREG

# Module Includes
BREG_INCLUDES += $(MAGNUM)/basemodules/reg

BREG_SOURCES += \
	$(MAGNUM)/basemodules/reg/breg_i2c.c \
	$(MAGNUM)/basemodules/reg/breg_spi.c

ifeq ($(BREG_CAPTURE),y)
BREG_DEFINES += BREG_CAPTURE
endif

ifeq ($(SYSTEM),linuxemu)
BREG_SOURCES += $(MAGNUM)/basemodules/reg/breg_client.c
else
ifneq ($(B_REFSW_SYSTEM_MODE),CLIENT)
BREG_SOURCES += $(MAGNUM)/basemodules/reg/breg_mem.c
endif
endif

#
# Handle conditional options
#
ifneq ($(BCHP_7411_VER),)
# 7411 support
BREG_INCLUDES += $(MAGNUM)/basemodules/reg/7411
endif
