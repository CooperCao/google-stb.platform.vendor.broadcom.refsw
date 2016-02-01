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
MAGNUM_MODULES += BMUXLIB_FILE_MP4

# Module Includes
BMUXLIB_FILE_MP4_INCLUDES += \
	$(MAGNUM)/syslib/muxlib/include \
	$(MAGNUM)/syslib/muxlib/src/file/mp4


BMUXLIB_FILE_MP4_SOURCES += \
        $(MAGNUM)/syslib/muxlib/src/file/mp4/bmuxlib_file_mp4.c \
        $(MAGNUM)/syslib/muxlib/src/file/mp4/bmuxlib_file_mp4_priv.c \
        $(MAGNUM)/syslib/muxlib/src/file/mp4/bmuxlib_file_mp4_boxes.c \
        $(MAGNUM)/syslib/muxlib/src/file/mp4/bmuxlib_file_mp4_metadata.c

ifeq ($(BMUXLIB_TEST_MODE),y)
BMUXLIB_FILE_MP4_DEFINES += BMUXLIB_MP4_P_TEST_MODE=1
endif