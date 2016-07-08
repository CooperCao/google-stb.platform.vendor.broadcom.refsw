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
MAGNUM_MODULES += BMUXLIB_FILE_COMMON

# Module Includes
BMUXLIB_FILE_COMMON_INCLUDES += \
	$(MAGNUM)/syslib/muxlib/include


BMUXLIB_FILE_COMMON_SOURCES += \
        $(MAGNUM)/syslib/muxlib/src/common/bmuxlib_list.c \
        $(MAGNUM)/syslib/muxlib/src/common/bmuxlib_output.c \
        $(MAGNUM)/syslib/muxlib/src/common/bmuxlib_debug.c \
        $(MAGNUM)/syslib/muxlib/src/common/bmuxlib_input.c \
        $(MAGNUM)/syslib/muxlib/src/common/bmuxlib_pqueue.c

# DEBUG Build Flags
ifeq ($(BMUXLIB_INPUT_DUMP_DESC),y)
BMUXLIB_FILE_COMMON_DEFINES += BMUXLIB_INPUT_P_DUMP_DESC=1
endif
ifeq ($(BMUXLIB_INPUT_DUMP_FRAME_DESC),y)
BMUXLIB_FILE_COMMON_DEFINES += BMUXLIB_INPUT_P_DUMP_DESC=1
BMUXLIB_FILE_COMMON_DEFINES += BMUXLIB_INPUT_P_DUMP_FRAME_DESC=1
endif
ifeq ($(BMUXLIB_INPUT_DUMP_METADATA_DESC),y)
BMUXLIB_FILE_COMMON_DEFINES += BMUXLIB_INPUT_P_DUMP_DESC=1
BMUXLIB_FILE_COMMON_DEFINES += BMUXLIB_INPUT_P_DUMP_METADATA_DESC=1
endif
ifeq ($(BMUXLIB_TEST_MODE),y)
BMUXLIB_FILE_COMMON_DEFINES += BMUXLIB_INPUT_P_TEST_MODE=1
BMUXLIB_FILE_COMMON_DEFINES += BMUXLIB_INPUT_P_DUMP_DESC=1
BMUXLIB_FILE_COMMON_DEFINES += BMUXLIB_INPUT_P_DUMP_METADATA_DESC=1
endif

