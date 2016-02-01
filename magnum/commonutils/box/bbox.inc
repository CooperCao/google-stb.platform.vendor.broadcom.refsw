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
# Created:
#
# $brcm_Log: $
#
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Default
HAS_BOX_MODES = n

# SOCs with box mode support
ifeq ($(filter $(BCHP_CHIP),7445), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),D0 E0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7145), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7366), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0 C0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7435), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7364), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),A0 B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7439), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),74371), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),A0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7250), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),B0), $(BCHP_VER))
HAS_BOX_MODES = y
endif
endif


BCHP_VER_LOWER := $(shell echo $(BCHP_VER) | tr A-Z a-z)

# Define this module
MAGNUM_MODULES += BBOX

# Module Includes
BBOX_INCLUDES += \
	$(MAGNUM)/commonutils/box/include \
	$(MAGNUM)/commonutils/box/src

# Module Sources
BBOX_SOURCES += \
	$(MAGNUM)/commonutils/box/src/bbox.c

ifeq (${HAS_BOX_MODES},y)
include	$(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/bbox_$(BCHP_CHIP).inc
BBOX_DEFINES += BBOX_BOX_MODES_SUPPORTED=1
else
BBOX_SOURCES += \
	$(MAGNUM)/commonutils/box/src/common/bbox_priv.c \
	$(MAGNUM)/commonutils/box/src/common/bbox_priv_vce_nop.c
endif

BBOX_SOURCES += \
	$(MAGNUM)/commonutils/box/src/common/bbox_priv_vce_common.c \
	$(MAGNUM)/commonutils/box/src/common/bbox_priv_audio_common.c

#
# End of file
#
