############################################################
#  Copyright (C) 2019 Broadcom.
#  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to
#  the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied),
#  right to use, or waiver of any kind with respect to the Software, and
#  Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
# Module Description:
#
############################################################
#SHELL := /bin/bash

CFLAGS	+= -I. -I../include -Wall
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -fPIC -lgcc_s
CFLAGS  += -O2
#CFLAGS	+= -DBCMDBG
LDFLAGS += -L.

OBJ = nvram_shared.o
OBJ += bcmutils.o

vpath %.c

all: libnvram.so nvram nvramd envrams envram

install:
	@echo "Not supported"

clean:
	rm -f *.o *.a *.so *.d nvram nvramd envram envrams

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

sources = envrams.c envramc.c main.c nvramd.c bcmutils.c
-include $(sources:.c=.d)

envrams: envrams.o
	$(CC) $^ -o $@

envram: envramc.o
	$(CC) $^ -o $@

libnvram.so: ${OBJ}
	$(LD) $(LDFLAGS) -shared -o $@ $^

nvramd: nvramd.o libnvram.so
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(EXTRA_LDFLAGS) -lnvram -lrt -pthread

nvram: main.o libnvram.so
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lnvram
