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


# bwin Makefile
SHELL=/bin/bash

include ../../../api/build/tools.mak
BSEAV = $(shell cd "../../.." && ${PWD})

# Identify object directory
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

.PHONY: check_environment static libpng libjpeg libexif freetype
all: check_environment libpng libjpeg libexif freetype static

OTHER_CLEANS += clean-libpng clean-libjpeg clean-libexif clean-freetype
.PHONY: clean-libpng clean-libjpeg clean-libexif clean-freetype
clean: clean-libpng clean-libjpeg clean-libexif clean-freetype

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
ifeq ($(findstring $(PLATFORM),97400 97401 97403 97118 97118RNG 97455 97456 97458),)
CFLAGS += -DFLOAT_SUPPORT
else
CFLAGS += -DEMULATED_FLOAT_SUPPORT
endif

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

ifeq ($(PNG_SUPPORT),y)
CFLAGS += -DPNG_SUPPORT -I$(LIBPNG_DIR) -I$(ZLIB_DIR)/include
SRCS += \
	bwin_image_png.c \
	bwin_image_png_priv.c

libz:
	echo [Build..... zlib]
	$(MAKE) -C $(BSEAV)/lib/zlib

libpng: libz
	echo [Build..... libpng]
	$(MAKE) -C $(BSEAV)/lib/libpng

clean-libpng:
	${Q_}$(MAKE) -C $(BSEAV)/lib/libpng clean
	${Q_}$(MAKE) -C $(BSEAV)/lib/zlib clean
else
# do nothing
libpng:
clean-libpng:
endif

ifeq ($(JPEG_SUPPORT),y)
CFLAGS += -DJPEG_SUPPORT -I$(LIBJPEG_ODIR)
SRCS += \
	bwin_image_jpeg.c


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
.PHONY: libjpeg
libjpeg:
	@$(MAKE) -C $(LIBJPEG_DIR)

clean-libjpeg:
	@$(MAKE) -C $(LIBJPEG_DIR) clean

endif
else
# do nothing
libjpeg:
clean-libjpeg:
endif

ifeq ($(EXIF_SUPPORT),y)
CFLAGS += -DEXIF_SUPPORT -I$(LIBEXIF_DIR) -I$(LIBEXIF_DIR)/libexif

ifeq ($(VERBOSE),)
REDIR_LIBEXIF := >& /dev/null
else
REDIR_LIBEXIF :=
endif

# libexif doesn't support mips-uclibc, so convert
ifeq ($(B_REFSW_ARCH),)
LIBEXIF_ARCH=mipsel-linux
else
LIBEXIF_ARCH=$(B_REFSW_ARCH)
endif

LIBEXIF_PREFIX := ($(addsuffix $(LIBEXIF_ARCH),$(dirname $(dirname`which $(CC)`))))

# configure libexif if Makefile doesn't exist
.PHONY: config-libexif
config-libexif:
	#speed of machine vs. granularity of creation/modification creates the need for a little sleep after configure
	@if [ ! -e $(LIBEXIF_DIR)/Makefile ]; then \
		mkdir -p $(LIBEXIF_DIR); \
		cd $(LIBEXIF_DIR)/..; \
		echo [Unpack.... libexif]; \
		tar xjvf $(BSEAV)/lib/libexif/libexif-0.6.21.tar.bz2 $(REDIR_LIBEXIF); \
		echo [Config.... libexif]; \
		cd $(LIBEXIF_DIR); \
		sh ./configure --prefix=$(EXIF_PREFIX) --exec-prefix=$(EXIF_PREFIX) \
			--host=$(LIBEXIF_ARCH) \
			--enable-static=yes --enable-shared=no $(REDIR_LIBEXIF); \
		sleep 1; \
	fi

libexif: $(LIBEXIF_DIR)/.libs/libexif.a

$(LIBEXIF_DIR)/.libs/libexif.a: config-libexif
	@if ! $(MAKE) -q -C $(LIBEXIF_DIR) >/dev/null 2>&1; then \
		echo [Build..... libexif]; \
		$(MAKE) -C $(LIBEXIF_DIR) $(REDIR_LIBEXIF); \
	fi

clean-libexif:
	-@if [ -e $(LIBEXIF_DIR)/Makefile ]; then \
		$(MAKE) -C $(LIBEXIF_DIR) distclean >/dev/null 2>&1; \
	fi
else
# do nothing
libexif:
clean-libexif:
endif

ifeq ($(VERBOSE),)
REDIR_LIBBTE := > /dev/null
else
REDIR_LIBBTE :=
endif

ifeq ($(FREETYPE_SUPPORT),y)
CFLAGS += -DFREETYPE_SUPPORT -I$(FREETYPE_DIR)/include
# We've checked in preconfigured source which uses our standard tools.mak
# for cross compiling. configure isn't going to work on DOS for vxworks.
freetype:
ifeq ($(SYSTEM),linux)
	@if ! $(MAKE) -q -C $(FREETYPE_DIR); then \
		echo [Build..... freetype]; \
		$(MAKE) -C $(FREETYPE_DIR); \
	fi
else
	echo [Build..... freetype];
	$(MAKE) -C $(FREETYPE_DIR)
endif
clean-freetype:
	${Q_}$(MAKE) -C $(FREETYPE_DIR) clean
else
# do nothing
freetype:
clean-freetype:
endif

ODIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/bwin/lib/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

check_environment: $(BWIN_DIR)/lib $(ODIR)

$(BWIN_DIR)/lib:
	${Q_}$(MKDIR) "$@"

include ../../../api/build/rules.mak

# required for parallel make (make -jN)
$(ODIR)/bwin_image_jpeg.o : libjpeg
$(ODIR)/bwin_image.o : config-libexif
