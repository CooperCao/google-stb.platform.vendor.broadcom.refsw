############################################################
#     Copyright (c) 2003-2009, Broadcom Corporation
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
# Created: 07/07/2009 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#
# BELF_MIPS_SUPPORT - By default, this will be y.  Set to n to disable MIPS support.
#

# Define this module
MAGNUM_MODULES += BELF

# Module Includes
BELF_INCLUDES += $(MAGNUM)/commonutils/elf

# Module Sources
BELF_SOURCES += \
	$(MAGNUM)/commonutils/elf/belf.c \
	$(MAGNUM)/commonutils/elf/belf_priv.c

# Options
ifndef BELF_MIPS_SUPPORT
# Default this to on
BELF_MIPS_SUPPORT := y
endif

# MIPS support
ifeq ($(BELF_MIPS_SUPPORT),y)
BELF_SOURCES += $(MAGNUM)/commonutils/elf/belf_mips.c
BELF_DEFINES += BELF_MIPS_SUPPORT=1
endif

