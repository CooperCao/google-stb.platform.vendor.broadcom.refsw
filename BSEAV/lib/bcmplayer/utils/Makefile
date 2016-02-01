############################################################
#	  (c)2012-2014 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
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
# Linux makefile
#
BSEAV=$(shell cd ../../..; pwd)
NEXUS_TOP=$(shell cd $(BSEAV)/../nexus; pwd)
MAGNUM=$(shell cd $(BSEAV)/../magnum; pwd)
ODIR=.
Q_=@

NEXUS_PLATFORM ?= $(PLATFORM)
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

APPS = createindex printindex filterpes filterts byteswap printsc rmscprevent \
	printpes mkbtpfile printavc printvc1 muxts btp_generator  b_playback_capture

ifeq (${B_REFSW_ARCH},i386-linux)
NEXUS_LD_LIBRARIES:=
else
APPS += b_playback_decode
endif

all: $(APPS)

LFLAGS += -lpthread

ifeq (${B_REFSW_ARCH},i386-linux)
LFLAGS += -lrt
endif
DESTDIR ?= $(NEXUS_BIN_DIR)

CFLAGS += -DCMDLINE_TEST

CFLAGS += -I../include -I../src -Wall -I. -std=c99
#CFLAGS += -DSTORE_SEQUENCE_END_CODES

include $(BSEAV)/../magnum/commonutils/vlc/bvlc.inc
include ${BSEAV}/lib/bcmplayer/bcmplayer.inc

CFLAGS += $(addprefix -I,$(BVLC_INCLUDES) ${BSEAV}/lib/utils)
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

BCMPLAYER_SRC += $(sort ${B_NAV_INDEXER_SOURCES} ${B_NAV_PLAYER_SOURCES} )
BCMPLAYER_SRC += ${BSEAV}/lib/utils/bioatom.c ${BSEAV}/lib/utils/bpool.c ${BSEAV}/lib/utils/barena.c

MAGNUM_SRC = \
	$(BSTD_SOURCES) \
	$(BKNI_SOURCES) \
	$(BDBG_SOURCES) \
	$(BVLC_SOURCES)

createindex: createindex.c $(BCMPLAYER_SRC) ../src/tsindexer.c $(MAGNUM_SRC)
	@echo $(CC) -o $@
	$(Q_)$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

printindex: printindex.c $(BCMPLAYER_SRC) $(MAGNUM_SRC)
	@echo $(CC) -o $@
	$(Q_)$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

mkbtpfile: mkbtpfile.c $(BCMPLAYER_SRC) $(MAGNUM_SRC)
	@echo $(CC) -o $@
	$(Q_)$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

printavc: printavc.c ts_utils.c $(MAGNUM_SRC)
	@echo $(CC) -o $@
	$(Q_)$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

b_playback_capture: b_playback.c b_playback_capture.c $(MAGNUM_SRC) $(BCMPLAYER_SRC)
	@echo $(CC) -o $@
	$(Q_)$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) $(NEXUS_LD_LIBRARIES)

.PHONY: b_playback_decode
b_playback_decode:
	$(MAKE) -f Makefile.nexus $@

# default is for single-file apps w/ ts_utils
%: %.c ts_utils.c b_playback.c $(MAGNUM_SRC) $(BCMPLAYER_SRC)
	@echo $(CC) -o $@
	$(Q_)$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) $(NEXUS_LD_LIBRARIES)

clean:
	$(RM) $(APPS) *.o

ifeq ($(DESTDIR),)
install:
	$(error You must define DESTDIR first)
else
install:
	cp -f $(APPS) $(DESTDIR)
endif
