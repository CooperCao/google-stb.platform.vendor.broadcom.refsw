############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# Module Description:
#   This file defines the sources, include paths, and defintions required
#   to use this library
#
############################################################

#
# Conditional options for this makefile
#
# None at this time
#

#
# Identify this library
#
B_LIBS += B_DCC_LIB



ifeq ($(SOFT_GFX_SUPPORT),y)
B_DCC_LIB_PRIVATE_INCLUDES += $(B_LIB_TOP)/softgfx/include
B_DCC_LIB_DEFINES += B_HAS_SOFT_GFX=1
endif #SOFT_GFX_SUPPORT

ifeq ($(FREETYPE_SUPPORT),y)
FREETYPE_DIR = $(NEXUS_TOP)/../BSEAV/opensource/freetype
include $(FREETYPE_DIR)/freetype.inc
B_DCC_LIB_PRIVATE_INCLUDES += $(FREETYPE_SOURCE_PATH)/include
B_DCC_LIB_DEFINES += FREETYPE_SUPPORT
else

ifeq ($(FONT_CFG),SCALE_480I_FONTS)
B_DCC_LIB_DEFINES += SCALE_480I_FONTS
endif

ifeq ($(FONT_CFG),SCALE_720P_FONTS)
B_DCC_LIB_DEFINES += SCALE_720P_FONTS
endif

endif


#
# Define source files in this library
#
B_DCC_LIB_SOURCES += \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_bits.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_cbuf.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_packet.c\
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_service.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_coding.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_coding_c1_handlers.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_608transcoder.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_int.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_intgfx.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_gfx.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/bdcc_engine.c \
                    $(B_BSEAV_TOP)/lib/dcc/src/win_library/bdcc_winlib.c \
                    $(B_BSEAV_TOP)/../nexus/nxclient/apps/utils/bfont.c

#
# Define public include path(s) for this library
#
B_DCC_LIB_PUBLIC_INCLUDES += $(B_BSEAV_TOP)/lib/dcc/include \
                             $(B_BSEAV_TOP)/../nexus/nxclient/apps/utils

#
# Define private include path(s) for this library
#
B_DCC_LIB_PRIVATE_INCLUDES += $(B_BSEAV_TOP)/lib/dcc/src \
                              $(B_BSEAV_TOP)/lib/dcc/include \
                              $(B_LIB_TOP)/os/include \
                              $(B_LIB_TOP)/os/include/$(B_REFSW_OS)
                              
#
# Define extra C definitions
#
#B_DCC_LIB_DEFINES += ## N/A for this library

ifeq ($(TEST_STRING),y)
B_DCC_LIB_DEFINES += TEST_STRING_SUPPORT
endif

ifeq ($(ARGB2222_FRAMEBUFFER),y)

B_DCC_LIB_DEFINES += ARGB2222_CANVAS
B_DCC_LIB_DEFINES += USE_ARGB2222

else

ifeq ($(ARGB2222_SURFACE),y)
B_DCC_LIB_DEFINES += USE_ARGB2222
endif

endif


#
# Define possible options - this is only for debug purposes, it does _not_ actually define them.
#
#B_DCC_LIB_OPTIONS += ## N/A for this library

