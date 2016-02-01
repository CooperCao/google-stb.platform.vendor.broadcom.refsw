#############################################################################
# (c) 2014 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################

#
# Conditional options for this makefile
#
# None at this time
#
BSEAV ?= ${NEXUS_TOP}/../BSEAV

#
# Identify this library
#
B_LIBS += B_XCODE_LIB

#
# Define source files in this library
#
B_XCODE_LIB_SOURCES += \
                    $(BSEAV)/lib/utils/bfifo.c \
                    $(B_LIB_TOP)/xcode/src/bxcode.c \
                    $(B_LIB_TOP)/os/src/bxcode_priv.c \
                    $(B_LIB_TOP)/os/src/bxcode_input.c \
                    $(B_LIB_TOP)/os/src/bxcode_output.c

#
# Define public include path(s) for this library
#
B_XCODE_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/xcode/include \
                            $(BSEAV)/lib/bcmplayer/include \
                            $(BSEAV)/lib/utils

#
# Define private include path(s) for this library
#
B_XCODE_LIB_PRIVATE_INCLUDES += $(B_LIB_TOP)/xcode/src

#
# Define other libs that this library is dependent upon.
# This will check for cyclic dependencies and setup include paths
#
B_XCODE_LIB_DEPENDENCIES := B_OS_LIB

#
# Define extra C definitions
#
#B_XCODE_LIB_DEFINES += ## N/A for this library

#
# Define possible options - this is only for debug purposes, it does _not_ actually define them.
#
#B_XCODE_LIB_OPTIONS += ## N/A for this library

#  Include definitions for libraries used by XCODE
#  The following will define the B_XCODE_LIB_LDFLAGS as
#  appropriate for each XCODE component.
B_XCODE_LIB_CFLAGS += $(addprefix -I,$(B_XCODE_LIB_PUBLIC_INCLUDES))
B_XCODE_LIB_LDFLAGS+= -L$(NEXUS_BIN_DIR) -lb_xcode

# Library: nexus
ifeq ($(NEXUS_CFLAGS),)
    include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif
B_XCODE_LIB_CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
B_XCODE_LIB_LDFLAGS+= -L$(NEXUS_BIN_DIR) -lnexus

# Library: b_os
# Note this must be after nexus platform_app.inc or nullify MAGNUM_MODULES
include ${NEXUS_TOP}/lib/os/b_os_lib.inc
B_XCODE_LIB_CFLAGS += $(addprefix -I,$(B_OS_LIB_PUBLIC_INCLUDES))
B_XCODE_LIB_LDFLAGS+= -L$(NEXUS_BIN_DIR) -lb_os -lpthread

# Library: tshdrbuilder
include ${NEXUS_TOP}/../BSEAV/lib/tshdrbuilder/tshdrbuilder.inc
B_XCODE_LIB_CFLAGS += $(addprefix -I,$(TSHDRBUILDER_INCLUDES))
B_XCODE_LIB_LDFLAGS+= -L$(NEXUS_BIN_DIR) -ltshdrbuilder

# Library: bcmplayer
include $(NEXUS_TOP)/../magnum/commonutils/vlc/bvlc.inc
include ${NEXUS_TOP}/../BSEAV/lib/bcmplayer/bcmplayer.inc
B_XCODE_LIB_CFLAGS += $(addprefix -I,$(BVLC_INCLUDES) $(B_NAV_PLAYER_INCLUDES))
B_XCODE_LIB_SOURCES += $(BVLC_SOURCES) $(B_NAV_PLAYER_SOURCES)
