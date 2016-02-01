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
# $brcm_Log: $
# 
############################################################

# Define this module
MAGNUM_MODULES += BMUXLIB_STREAM_TS

# Module Includes
BMUXLIB_STREAM_TS_INCLUDES += \
	$(MAGNUM)/syslib/muxlib/include \
	$(MAGNUM)/syslib/muxlib/src/stream/ts/


BMUXLIB_STREAM_TS_SOURCES += \
        $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts.c \
        $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_priv.c \
        $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_userdata.c \
        $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_mcpb.c

# DEBUG Build Flags
ifeq ($(BMUXLIB_DUMP_TRANSPORT_DESC),y)
BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_P_DUMP_TRANSPORT_DESC=1
endif
ifeq ($(BMUXLIB_DUMP_TRANSPORT_PES),y)
BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_P_DUMP_TRANSPORT_PES=1
endif
ifeq ($(BMUXLIB_DUMP_PCR),y)
BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_P_DUMP_PCR=1
endif
ifeq ($(BMUXLIB_TEST_MODE),y)
BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_P_TEST_MODE=1
endif
ifeq ($(BMUXLIB_MCPB_DUMP_TRANSPORT_DESC),y)
BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_MCPB_P_DUMP_TRANSPORT_DESC=1
endif
ifeq ($(BMUXLIB_MCPB_DUMP_TRANSPORT_OUTPUT),y)
BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_MCPB_P_DUMP_TRANSPORT_OUTPUT=1
endif
