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

#VPATH = ..
ifeq ($(B_REFSW_ARCH),)
	CC=gcc
else
	CC=$(B_REFSW_ARCH)-gcc
endif

CFLAGS=-I../ -Wall -Wformat -g -std=c99

all: asp_utils_test asp_nw_sw_test asp_xpt_test asp_proxy_server_test

asp_utils_test: asp_utils_test.o ../asp_utils.o
	$(CC) -o $@ asp_utils_test.o ../asp_utils.o

asp_nw_sw_test: asp_nw_sw_test.o ../asp_utils.o ../asp_nw_sw.o
	$(CC) -o $@ asp_nw_sw_test.o ../asp_utils.o ../asp_nw_sw.o

asp_xpt_test: asp_xpt_test.o ../asp_utils.o ../asp_xpt.o
	$(CC) -o $@ asp_xpt_test.o ../asp_utils.o ../asp_xpt.o

asp_proxy_server_test: asp_proxy_server_test.o ../asp_utils.o ../asp_proxy_server.o ../asp_proxy_server_messages.o
	$(CC) -o $@ asp_proxy_server_test.o ../asp_utils.o ../asp_proxy_server.o ../asp_proxy_server_messages.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

DEPS := $(patsubst %.o,%.d,$(OBJS))

clean:
	rm -rf *.o; rm -f *.d; rm -rf ../*.o; rm -f asp_utils_test; rm -f asp_nw_sw_test; rm -f asp_xpt_test; rm -f asp_proxy_server_test
install:
	cp -f asp_proxy_server_test ../../../../../obj.$(NEXUS_PLATFORM)/nexus/bin

.PHONY: all clean
