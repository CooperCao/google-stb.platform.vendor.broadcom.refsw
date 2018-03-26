################################################################################
# Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
###############################################################################

# Build Options
#
#  BMUXLIB_TS_USE_MUX_V2=y
#

# Define this module
MAGNUM_MODULES += BMUXLIB_STREAM_TS

# Module Includes
BMUXLIB_STREAM_TS_INCLUDES += \
    $(MAGNUM)/syslib/muxlib/include \
    $(MAGNUM)/syslib/muxlib/src/stream/ts/

BMUXLIB_STREAM_TS_SOURCES += \
   $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_proxy.c

# TS MUXlib (Legacy)
# Module Includes
BMUXLIB_STREAM_TS_INCLUDES += \
  $(MAGNUM)/syslib/muxlib/src/stream/ts

BMUXLIB_STREAM_TS_SOURCES += \
        $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts.c \
        $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_priv.c \
        $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_userdata.c \
        $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_mcpb.c

# DEBUG Build Flags
ifeq ($(BMUXLIB_DUMP_TRANSPORT_DESC),y)
BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_P_DUMP_TRANSPORT_DESC=1
endif
ifeq ($(BMUXLIB_DUMP_TRANSPORT_DESC_PER_CHANNEL), y)
BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_P_DUMP_TRANSPORT_DESC_PER_CHANNEL=1
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
ifeq ($(BMUXLIB_ENABLE_STATS),y)
BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_P_ENABLE_STATS=1
endif

# TS MUXlib (ASP)
ifeq ($(BMUXLIB_TS_USE_MUX_V2),y)
BMUXLIB_STREAM_TS_INCLUDES += \
  $(MAGNUM)/syslib/muxlib/src/asp/hal/sw \
  $(MAGNUM)/syslib/muxlib/src/asp/include \
  $(MAGNUM)/syslib/muxlib/src/asp/include/priv \
  $(MAGNUM)/syslib/muxlib/src/asp/src/ts

BMUXLIB_STREAM_TS_DEFINES += BMUXLIB_TS_P_USE_MUX_V2=1

BMUXLIB_STREAM_TS_SOURCES += \
   $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_asp.c \
   $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_asp_userdata.c \
   $(MAGNUM)/syslib/muxlib/src/stream/ts/bmuxlib_ts_asp_sw.c \
   $(MAGNUM)/syslib/muxlib/src/asp/src/common/bmuxlib_asp_output_priv.c \
   $(MAGNUM)/syslib/muxlib/src/asp/src/common/bmuxlib_asp_source_helper_priv.c \
   $(MAGNUM)/syslib/muxlib/src/asp/src/common/bmuxlib_asp_source_list_priv.c \
   $(MAGNUM)/syslib/muxlib/src/asp/src/ts/bmuxlib_asp_ts_channel_priv.c \
   $(MAGNUM)/syslib/muxlib/src/asp/src/ts/bmuxlib_asp_ts_memory_priv.c \
   $(MAGNUM)/syslib/muxlib/src/asp/src/ts/bmuxlib_asp_ts_source_av_priv.c \
   $(MAGNUM)/syslib/muxlib/src/asp/src/ts/bmuxlib_asp_ts_source_sys_priv.c \
   $(MAGNUM)/syslib/muxlib/src/asp/src/ts/bmuxlib_asp_ts_source_user_priv.c
endif
