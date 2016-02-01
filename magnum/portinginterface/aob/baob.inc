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
# N/A
#

# Define this module
MAGNUM_MODULES += BAOB

# If the chip version is not specified, select a default
ifeq ($(BCHP_312X_VER),)
BCHP_312X_VER := A0
endif

# Source needs to know which revision
BAOB_DEFINES += BAOB_312X_VER=BCHP_VER_$(BCHP_312X_VER)

# Module Includes
BAOB_INCLUDES += $(MAGNUM)/portinginterface/aob/
BAOB_INCLUDES += $(MAGNUM)/commonutils/fec/include

# Module Includes
BAOB_INCLUDES += $(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)
BAOB_INCLUDES += $(MAGNUM)/basemodules/chp/include/$(BAOB_CHIP)/common     

# Module Sources
BAOB_SOURCES += $(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)/baob.c
 

#
# Handle 7552
#

ifeq ($(BCHP_CHIP),7552)
BAOB_SOURCES += \
	$(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)/baob_acquire.c \
	$(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)/baob_status.c \
	$(MAGNUM)/portinginterface/aob/$(BAOB_CHIP)/baob_utils.c
endif
