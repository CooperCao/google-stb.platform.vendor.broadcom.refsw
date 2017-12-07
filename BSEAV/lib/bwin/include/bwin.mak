#############################################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#############################################################################


# bwin.mak
# This file should be included by applications linking to bwin.
#
# Inputs: BSEAV, B_REFSW_ARCH
# Outputs: BWIN_CFLAGS, BWIN_LDFLAGS

BWIN_DIR = $(BSEAV)/lib/bwin
B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
BWIN_LIBDIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/bwin

BWIN_CFLAGS = -I$(BWIN_DIR)/include
BWIN_LDFLAGS = -L$(BWIN_LIBDIR) -lbwin

# By default, use freetype offline and use
# prerendered fonts in the application.
FREETYPE_SUPPORT ?= n

ifeq ($(FREETYPE_SUPPORT),y)
PNG_SUPPORT := y
else
PNG_SUPPORT ?= y
endif

JPEG_SUPPORT = y

LIB_DIR = $(BSEAV)/lib

ifeq ($(FREETYPE_SUPPORT),y)
FREETYPE_DIR = $(BSEAV)/opensource/freetype
include $(FREETYPE_DIR)/freetype.inc
BWIN_LDFLAGS += $(FREETYPE_STATIC_LDFLAGS)
endif

ifeq ($(PNG_SUPPORT),y)
LIBPNG_DIR = $(BSEAV)/opensource/libpng
include $(LIBPNG_DIR)/libpng.inc
BWIN_LDFLAGS += $(LIBPNG_STATIC_LDFLAGS)
endif

ZLIB_DIR = $(BSEAV)/opensource/zlib
include $(ZLIB_DIR)/zlib.inc
BWIN_LDFLAGS += $(ZLIB_STATIC_LDFLAGS)

ifeq ($(JPEG_SUPPORT),y)
LIBJPEG_DIR = $(BSEAV)/opensource/jpeg
include $(LIBJPEG_DIR)/jpeg.inc
BWIN_LDFLAGS += $(JPEG_STATIC_LDFLAGS)
endif
