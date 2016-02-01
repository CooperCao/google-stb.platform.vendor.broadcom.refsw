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
MAGNUM_MODULES += BKNI

# Backward-compatibility
B_REFSW_OS ?= $(OS)

# Options
ifeq ($(BKNI_TRACK_MALLOCS),y)
BKNI_DEFINES := BKNI_TRACK_MALLOCS=1
endif

# Module Includes
ifeq ($(findstring $(B_REFSW_OS), linuxclient linuxemu linuxpal bare win32 wince),${B_REFSW_OS})
BKNI_INCLUDES += $(MAGNUM)/basemodules/kni/linuxuser
ifeq ($(findstring $(B_REFSW_OS), bare ),${B_REFSW_OS})
BKNI_INCLUDES += $(MAGNUM)/basemodules/kni/$(B_REFSW_OS) 
endif
else
BKNI_INCLUDES += $(MAGNUM)/basemodules/kni/$(B_REFSW_OS) 
endif

# Sanity check the OS value
BKNI_SUPPORTED_OS := linuxuser linuxkernel linuxclient linuxemu linuxpal nucleus ucos vxworks win32 bkernel ucos_ii ucos_iii bare no_os
ifeq ($(findstring $(B_REFSW_OS), $(BKNI_SUPPORTED_OS)),)
$(error [kni] OS must be one of the following: $(BKNI_SUPPORTED_OS))
endif

# For linux kernel, the bkni.c file is compiled separately.
ifneq ($(SYSTEM),linuxkernel)
# Include the basic KNI for this OS
ifeq ($(findstring $(B_REFSW_OS), win32),$(B_REFSW_OS))
# win32 has bkni_$(B_REFSW_OS).c as the filename
BKNI_SOURCES := $(MAGNUM)/basemodules/kni/$(B_REFSW_OS)/bkni_$(B_REFSW_OS).c
else
# others are just bkni.c
ifeq ($(B_REFSW_OS),ucos)
BKNI_SOURCES := $(MAGNUM)/basemodules/kni/$(B_REFSW_UCOS_SUBDIR_NAME)/bkni.c
else
BKNI_SOURCES := $(MAGNUM)/basemodules/kni/$(B_REFSW_OS)/bkni.c
endif
endif
endif

# Include ISO source for OS environments that use it
BKNI_ISO_OS :=
ifeq ($(findstring $(B_REFSW_OS), ${BKNI_ISO_OS}), $(B_REFSW_OS))
BKNI_SOURCES += $(MAGNUM)/basemodules/kni/$(B_REFSW_OS)/bkni_iso_c.c
endif


