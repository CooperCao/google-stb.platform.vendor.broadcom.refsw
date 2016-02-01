############################################################
#     Copyright (c) 2009-2014, Broadcom Corporation
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
MAGNUM_MODULES += BV3D

# Module Includes
BV3D_INCLUDES += $(MAGNUM)/portinginterface/v3d/include

# Module Sources
BV3D_SOURCES += $(MAGNUM)/portinginterface/v3d/src/bv3d.c \
				$(MAGNUM)/portinginterface/v3d/src/bv3d_job.c \
				$(MAGNUM)/portinginterface/v3d/src/bv3d_iq.c \
				$(MAGNUM)/portinginterface/v3d/src/bv3d_notifyq.c \
				$(MAGNUM)/portinginterface/v3d/src/bv3d_qmap.c \
				$(MAGNUM)/portinginterface/v3d/src/bv3d_waitq.c \
				$(MAGNUM)/portinginterface/v3d/src/bv3d_callbackmap.c \
				$(MAGNUM)/portinginterface/v3d/src/bv3d_worker.c \
				$(MAGNUM)/portinginterface/v3d/src/bv3d_binmem.c \
				$(MAGNUM)/portinginterface/v3d/src/bv3d_fence.c \
				$(V3D_MAGNUM_TOP)/src/$(B_REFSW_OS)/bv3d_os.c

# Sanity Check OS provided
BV3D_SUPPORTED_OS := linuxuser linuxkernel linuxclient linuxemu linuxpal
ifeq ($(findstring $(B_REFSW_OS), $(BV3D_SUPPORTED_OS)),)
$(error [v3d] OS must be one of the following: $(BV3D_SUPPORTED_OS))
endif

ifeq ($(findstring $(B_REFSW_OS), linuxuser linuxclient linuxemu linuxpal),${B_REFSW_OS})
BV3D_DEFINES += BV3D_USERMODE
else
BV3D_DEFINES += BV3D_KERNELMODE
endif
