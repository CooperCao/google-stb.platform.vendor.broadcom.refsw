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

Q_=@
NEXUS_TOP=$(shell cd ../../../../nexus;pwd)

APPS=test stb_server stb_gallery_client
all: $(APPS) bwidgets

include ../include/bwidgets.mak
include $(NEXUS_TOP)/../nexus/platforms/$(PLATFORM)/build/platform_app.inc

CFLAGS += $(BWIDGETS_CFLAGS)
LDFLAGS += $(BWIDGETS_LDFLAGS)
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES) -lm

.PHONY: bwidgets clean-bwidgets

bwidgets:
	make -C ../../bwin/build
	make -C ../build

clean_bwidgets:
	make -C ../../bwin/build clean
	make -C ../build clean

$(OBJS): bwidgets

test: test.o b_app_server.o b_app_client.o bwidgets
	@echo Link $@
	${Q_}$(CC) -o $@ $(filter %.o,$^) $(LDFLAGS)

stb_server: stb_server.o b_app_server.o b_app_client.o bwidgets
	@echo Link $@
	${Q_}$(CC) -o $@ $(filter %.o,$^) $(LDFLAGS)

stb_gallery_client: stb_gallery_client.o b_app_client.o bwidgets
	@echo Link $@
	${Q_}$(CC) -o $@ $(filter %.o,$^) $(LDFLAGS)

%.o: %.c
	@echo Compile $@
	${Q_}$(CC) -c -o $@ $< $(CFLAGS)

clean_apps:
	$(RM) $(APPS) *.o

clean: clean_apps clean_bwidgets

ifneq ($(DESTDIR),)
install:
	$(CP) $(APPS) $(DESTDIR)
endif
