############################################################
#     Copyright (c) 2012 Broadcom Corporation
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

# Define this module
MAGNUM_MODULES += BMMA
BMMA_USE_STUB?=n

# Module Includes
BMMA_INCLUDES += $(MAGNUM)/basemodules/mma
BMMA_POOL_SOURCES := $(MAGNUM)/basemodules/mma/bmma_pool.c \

# Module Sources
BMMA_SOURCES := \
    ${BMMA_POOL_SOURCES} \
    $(MAGNUM)/basemodules/mma/bmma_range.c

ifeq (${BMMA_USE_STUB}, y)
BMMA_SOURCES += $(MAGNUM)/basemodules/mma/bmma_stub.c
BMMA_DEFINES := BMMA_USE_STUB=1
else
BMMA_INCLUDES += $(MAGNUM)/basemodules/mem
BMMA_SOURCES += $(MAGNUM)/basemodules/mma/bmma_alloc.c
BMMA_SOURCES += $(MAGNUM)/basemodules/mma/bmma_bmem.c
endif
