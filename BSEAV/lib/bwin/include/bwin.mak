#############################################################################
# (c) 2003-2014 Broadcom Corporation
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


# bwin.mak
# This file should be included by applications linking to bwin.
#
# Inputs: BSEAV, B_RESW_ARCH
# Outputs: BWIN_CFLAGS, BWIN_LDFLAGS

BWIN_DIR = $(BSEAV)/lib/bwin
ifneq ($(filter $(B_REFSW_DEBUG),n no_error_messages),)
    DEBUG_SUFFIX=release
else
    DEBUG_SUFFIX=debug
endif

BWIN_LIBDIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/bwin/lib/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

BWIN_CFLAGS = -I$(BWIN_DIR)/include
BWIN_LDFLAGS = -L$(BWIN_LIBDIR) -lbwin

# By default, use freetype offline and use
# prerendered fonts in the application.
FREETYPE_SUPPORT ?= n
FREETYPE_DIR = $(BSEAV)/lib/freetype-2.1.5

PNG_SUPPORT = y
JPEG_SUPPORT = y

LIB_DIR = $(BSEAV)/lib

ifeq ($(SYSTEM),vxworks)
EXIF_SUPPORT = n
else
EXIF_SUPPORT = y
endif

LIBEXIF_DIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/libexif-0.6.21
LIBPNG_DIR = $(BSEAV)/lib/libpng
LIBPNG_ODIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/libpng
ZLIB_DIR = $(BSEAV)/lib/zlib
ZLIB_ODIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/zlib
LIBJPEG_DIR = $(BSEAV)/lib/jpeg-6b
LIBJPEG_ODIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/jpeg-6b

ifeq ($(FREETYPE_SUPPORT),y)
BWIN_LDFLAGS += -L$(FREETYPE_DIR)/lib/$(B_REFSW_ARCH).$(DEBUG_SUFFIX) -lfreetype
endif

ifeq ($(PNG_SUPPORT),y)
BWIN_LDFLAGS += -L$(LIBPNG_ODIR) -lpng -L$(ZLIB_ODIR)/$(B_REFSW_ARCH) -lz
endif

ifeq ($(JPEG_SUPPORT),y)
BWIN_LDFLAGS += -L$(LIBJPEG_ODIR) -ljpeg
endif

ifeq ($(EXIF_SUPPORT),y)
BWIN_LDFLAGS += -L$(LIBEXIF_DIR)/libexif/.libs -lexif
endif
