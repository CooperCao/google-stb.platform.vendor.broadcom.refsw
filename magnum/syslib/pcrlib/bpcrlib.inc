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
# BCHP_7411_VER - If defined (in lower-case), specifies the revision of the 7411 chip to be supported.
#

# Define this module
MAGNUM_MODULES += BPCRLIB

# Module Includes
BPCRLIB_INCLUDES += $(MAGNUM)/syslib/pcrlib/include

# Module Sources
BPCRLIB_SOURCES += $(MAGNUM)/syslib/pcrlib/src/bpcrlib.c

# Chip Specifics
ifeq ($(filter $(BCHP_CHIP), 7038 3560 3563),$(BCHP_CHIP))
# 7038, 3560, 3563 have MVD
ifeq ($(BPCRLIB_HAS_MVD),)
BPCRLIB_HAS_MVD := y
endif
else
# Others have XVD
ifeq ($(BPCRLIB_HAS_XVD),)
BPCRLIB_HAS_XVD := y
endif
endif

ifeq ($(filter $(BCHP_CHIP),7038 3560),$(BCHP_CHIP))
# 7038 and 3560 have AUD
BPCRLIB_HAS_AUD := y
else
# Others have RAP or APE
# Set RAP for only 65nm chips with AUD/Raptor cores, otherwise default to APE
ifeq ($(BPCRLIB_HAS_RAP),)
ifeq ($(filter $(BCHP_CHIP), 7118 7125 7325 7335 7336 7340 7342 7400 7401 7403 7405 7411 7420 7438 7468 7550),$(BCHP_CHIP))
BPCRLIB_HAS_RAP := y
endif
endif

# Default to use APE of 40nm and newer SoC
ifeq ($(BPCRLIB_HAS_APE),)
BPCRLIB_HAS_APE := y
endif
endif

ifneq ($(BCHP_7411_VER),)
# 7411 has XVD and RAP - and a separate directory for them.
BPCRLIB_HAS_XVD := y
BPCRLIB_HAS_RAP := y
endif

ifeq ($(BPCRLIB_HAS_MVD),y)
BPCRLIB_SOURCES += $(MAGNUM)/syslib/pcrlib/src/bpcrlib_mvd.c
endif

ifeq ($(BPCRLIB_HAS_AUD),y)
BPCRLIB_SOURCES += $(MAGNUM)/syslib/pcrlib/src/bpcrlib_aud.c
endif

ifeq ($(BPCRLIB_HAS_XVD),y)
BPCRLIB_SOURCES += $(MAGNUM)/syslib/pcrlib/src/bpcrlib_xvd.c
endif

ifeq ($(BPCRLIB_HAS_RAP),y)
BPCRLIB_SOURCES += $(MAGNUM)/syslib/pcrlib/src/bpcrlib_rap.c
endif

ifeq ($(BPCRLIB_HAS_APE),y)
BPCRLIB_SOURCES += $(MAGNUM)/syslib/pcrlib/src/bpcrlib_ape.c
endif

