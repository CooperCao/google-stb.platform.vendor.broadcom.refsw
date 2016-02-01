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
# BLNA_3405_SUPPORT - If defined, support will be included for the 3405
# BLNA_3431_SUPPORT - If defined, support will be included for the 3431
# BLNA_3412_SUPPORT - If defined, support will be included for the 3412
#

# Define this module
MAGNUM_MODULES += BLNA

# Module Includes
BLNA_INCLUDES += $(MAGNUM)/portinginterface/lna/

# Module sources
BLNA_SOURCES += $(MAGNUM)/portinginterface/lna/blna.c

#
# Conditional options
#
ifneq ($(BLNA_3405_SUPPORT),)
BLNA_INCLUDES += $(MAGNUM)/portinginterface/lna/3405/

BLNA_SOURCES += $(MAGNUM)/portinginterface/lna/3405/blna_3405.c
endif

ifneq ($(BLNA_3431_SUPPORT),)
BLNA_INCLUDES += $(MAGNUM)/portinginterface/lna/3431/

BLNA_SOURCES += $(MAGNUM)/portinginterface/lna/3431/blna_3431.c
endif

ifneq ($(BLNA_3412_SUPPORT),)
BLNA_INCLUDES += $(MAGNUM)/portinginterface/lna/3412/
BLNA_INCLUDES += $(MAGNUM)/basemodules/chp/include/3412/rdb/a0

BLNA_SOURCES += $(MAGNUM)/portinginterface/lna/3412/blna_3412.c
endif
