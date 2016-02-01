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
MAGNUM_MODULES += BMUXLIB_FILE_PES

# Module Includes
BMUXLIB_FILE_PES_INCLUDES += \
	$(MAGNUM)/syslib/muxlib/include \
	$(MAGNUM)/syslib/muxlib/src/file/pes


BMUXLIB_FILE_PES_SOURCES += \
        $(MAGNUM)/syslib/muxlib/src/file/pes/bmuxlib_file_pes.c \
        $(MAGNUM)/syslib/muxlib/src/file/pes/bmuxlib_file_pes_priv.c
