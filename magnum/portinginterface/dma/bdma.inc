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

# Define this module
MAGNUM_MODULES += BDMA

# Module Include Path
BDMA_INCLUDES += $(MAGNUM)/portinginterface/dma/include/common

# Module Sources
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/common/bdma.c
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/common/bdma_mem.c
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/common/bdma_mem_priv.c

BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/pci/bdma_pci.c
ifeq ($(filter $(BCHP_CHIP), 3548 3556 7630 7640 7468 7408 7422 7425 7358 7552 7231 7344 7346 7550 73465),)
BDMA_INCLUDES += $(MAGNUM)/portinginterface/dma/include/pci
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/pci/bdma_pci_priv.c
endif

BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/common/bdma_priv.c
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/common/bdma_queue.c

ifeq (${BCHP_CHIP}, 7440)
ifneq (${BCHP_VER}, A0)
BDMA_INCLUDES += $(MAGNUM)/portinginterface/dma/include/sharf
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/sharf/bdma_mem_sharf.c
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/sharf/bdma_mem_sharf_priv.c
endif
endif

ifeq (${BCHP_CHIP}, 7550)
ifeq (${BCHP_VER}, B0)
BDMA_INCLUDES += $(MAGNUM)/portinginterface/dma/include/sharf
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/sharf/bdma_mem_sharf.c
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/sharf/bdma_mem_sharf_priv.c
endif
endif

ifeq (${BCHP_CHIP}, 35230)
ifeq (${BCHP_VER}, C0)
BDMA_INCLUDES += $(MAGNUM)/portinginterface/dma/include/sharf
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/sharf/bdma_mem_sharf.c
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/sharf/bdma_mem_sharf_priv.c
endif
endif


ifneq ($(filter ${BCHP_CHIP}, 7635 7640 7601 7630 35233),)
BDMA_INCLUDES += $(MAGNUM)/portinginterface/dma/include/sharf
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/sharf/bdma_mem_sharf.c
BDMA_SOURCES += $(MAGNUM)/portinginterface/dma/src/sharf/bdma_mem_sharf_priv.c
endif



