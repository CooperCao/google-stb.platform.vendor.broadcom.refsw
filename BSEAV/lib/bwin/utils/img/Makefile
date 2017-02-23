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
.PHONY: png zlib
APPS=pngreader jpegreader
all: $(APPS)

ifeq ($(STATIC_SETTOPAPI),y)
SHAREDLIB=no
LFLAGS += -static
endif

include $(BSEAV)/api/build/tools.mak
BSEAV=$(shell cd ../../../.. && ${PWD})

CFLAGS += -MD -W -Wall

PNGDIR=$(BSEAV)/opensource/libpng
PNG_ODIR=$(BSEAV)/../obj.${NEXUS_PLATFORM}/BSEAV/opensource/libpng-1.2.8
ZLIBDIR=$(BSEAV)/opensource/zlib
ZLIB_ODIR=$(BSEAV)/../obj.${NEXUS_PLATFORM}/BSEAV/opensource/zlib-1.1.3
PNG_CFLAGS = -I$(PNG_ODIR) -I$(ZLIB_ODIR)/include
PNG_LFLAGS = -L$(PNGDIR) -lpng -L$(ZLIB_ODIR) -lz -lm

JPEG_ODIR=$(BSEAV)/../obj.${NEXUS_PLATFORM}/BSEAV/opensource/jpeg-6b
JPEG_CFLAGS = -I$(JPEG_ODIR)
JPEG_LFLAGS = -L$(JPEG_ODIR) -ljpeg

png: zlib
	make -C $(PNGDIR)

zlib:
	make -C $(ZLIBDIR)

jpeg:
	cd $(JPEG_ODIR); RANLIB=$(RANLIB) AR=$(AR) CC=$(CC) ./configure && make libjpeg.a

pngreader: pngreader.c png
	$(CC) -o $@ $(CFLAGS) $(PNG_CFLAGS) pngreader.c $(PNG_LFLAGS)

jpegreader: jpegreader.c jpeg
	$(CC) -o $@ $(CFLAGS) $(JPEG_CFLAGS) jpegreader.c $(JPEG_LFLAGS)

clean:
	-$(RM) $(APPS) *.o *.d

-include *.d
