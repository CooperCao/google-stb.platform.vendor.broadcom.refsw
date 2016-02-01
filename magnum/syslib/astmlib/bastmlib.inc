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

# Define this module
MAGNUM_MODULES += BASTMLIB

# Module Includes
BASTMLIB_INCLUDES += $(MAGNUM)/syslib/astmlib/include

BASTMLIB_SOURCES += \
    $(MAGNUM)/syslib/astmlib/src/bastmlib.c \
    $(MAGNUM)/syslib/astmlib/src/bastmlib_priv.c \
    $(MAGNUM)/syslib/astmlib/src/bastmlib_clock_coupling_task.c \
    $(MAGNUM)/syslib/astmlib/src/bastmlib_clock_reference.c \
    $(MAGNUM)/syslib/astmlib/src/bastmlib_presentation_task.c \
    $(MAGNUM)/syslib/astmlib/src/bastmlib_presenter.c

