#############################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


# bwin Makefile
SHELL=/bin/bash

include ../../../api/build/tools.mak
BSEAV = $(shell cd "../../.." && ${PWD})

# By default build dependent libraries - override with B_BUILD_DEPS=n
B_BUILD_DEPS?=y

# Identify object directory
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

# Other builds
B_EXTRA_BUILDS :=
B_EXTRA_CLEANS :=

.PHONY: check_environment libs static clean-libs self clean-self
.PHONY: freetype libpng

all:
	$(MAKE) freetype
	$(MAKE) libpng
	$(MAKE) static
	$(MAKE) check_environment

include ../include/bwin.mak
include $(BSEAV)/build/refsw_inc.mak

ifeq ($(VERBOSE),)
MAKEFLAGS += -s
endif

LIB = bwin

ifeq ($(SYSTEM),vxworks)
CFLAGS += -I$(WIND_BASE)/target/h
endif
CFLAGS += -W -Wall
CFLAGS += -I$(BWIN_DIR)/include -I$(BWIN_DIR)/src
CFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS)

# Using floating point and math.h allows cpu-based circle
# drawing.
CFLAGS += -DFLOAT_SUPPORT

vpath %.c $(BWIN_DIR)/src

SRCS = \
	bwin.c \
	bwin_draw.c \
	bwin_default_drawops.c \
	bwin_font.c \
	bwin_rect.c \
	bwin_image.c \
	bwin_transform.c \
	bwin_image_bmp.c

CFLAGS += -I$(ZLIB_ODIR)/include
ifeq ($(PNG_SUPPORT),y)

SRCS += \
	bwin_image_png.c \
	bwin_image_png_priv.c

B_EXTRA_BUILDS += libpng
B_EXTRA_CLEANS += clean-libpng
include $(LIBPNG_DIR)/b_libpng_targets.inc
CFLAGS += -DPNG_SUPPORT -I$(LIBPNG_SOURCE_PATH)
libpng: b_libpng_target_build
clean-libpng: b_libpng_target_clean_all
endif

ifeq ($(JPEG_SUPPORT),y)
CFLAGS += -DJPEG_SUPPORT -I$(LIBJPEG_ODIR)
SRCS += \
	bwin_image_jpeg.c

B_EXTRA_BUILDS += libjpeg
B_EXTRA_CLEANS += clean-libjpeg

ifeq ($(SYSTEM),vxworks)
# Use pre-configured Makefile
libjpeg: $(LIBJPEG_DIR)/jconfig.h $(LIBJPEG_DIR)/libjpeg.a

$(LIBJPEG_DIR)/jconfig.h:
	$(CP) $(LIBJPEG_DIR)/$(B_REFSW_ARCH)/jconfig.h $(LIBJPEG_DIR)

$(LIBJPEG_DIR)/libjpeg.a: $(LIBJPEG_DIR)/jconfig.h
	${Q_}$(MAKE) -C $(LIBJPEG_DIR) -f $(B_REFSW_ARCH)/Makefile libjpeg.a

ifeq ($(vxWorksVersion), )
#DOS script to remove jconfig.h
clean-libjpeg:
	${Q_}$(MAKE) -C $(LIBJPEG_DIR) -f $(B_REFSW_ARCH)/Makefile clean
	if exist $(LIBJPEG_DIR)/jconfig.h $(RM) $(LIBJPEG_DIR)/jconfig.h
else
#This is vxworks 6 and vxworks for linux side
clean-libjpeg:
	${Q_}$(MAKE) -C $(LIBJPEG_DIR) -f $(B_REFSW_ARCH)/Makefile clean
	@if [ -e $(LIBJPEG_DIR)/jconfig.h ]; then \
		$(RM) -f $(LIBJPEG_DIR)/jconfig.h; \
	fi
endif
else

# Linux
include $(BSEAV)/opensource/jpeg/b_libjpeg_targets.inc
libjpeg: b_libjpeg_target_build
clean-libjpeg: b_libjpeg_target_clean_all
endif
endif

ifeq ($(FREETYPE_SUPPORT),y)
# We've checked in preconfigured source which uses our standard tools.mak
# for cross compiling. configure isn't going to work on DOS for vxworks.
B_EXTRA_BUILDS += freetype
B_EXTRA_CLEANS += clean-freetype
include $(FREETYPE_DIR)/b_freetype_targets.inc
CFLAGS += -DFREETYPE_SUPPORT -I$(FREETYPE_SOURCE_PATH)/include
freetype: b_freetype_target_build
clean-freetype: b_freetype_target_clean_all
endif

ODIR = $(BWIN_LIBDIR)

check_environment: $(BWIN_DIR)/lib $(ODIR)

$(BWIN_DIR)/lib:
	${Q_}$(MKDIR) "$@"

ifeq ($(B_BUILD_DEPS),y)
OTHER_MAKES += $(B_EXTRA_BUILDS)
OTHER_CLEANS += $(B_EXTRA_CLEANS)
endif

include ../../../api/build/rules.mak

# required for parallel make (make -jN)
ifeq ($(B_BUILD_DEPS),y)
$(ODIR)/bwin_image_jpeg.o : libjpeg
endif
