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

#############################################################################

#VPATH = ..

# Figure out where we are and define NEXUS_TOP.
B_THIS_DIR:=rockford/third_party/ixia_dut_asp

ifdef B_REFSW_REAL_NEXUS_TOP
    NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

ifeq ($(B_REFSW_ARCH),)
	CC=gcc
else
#	CC=$(B_REFSW_ARCH)-g++
	CC=$(B_REFSW_ARCH)-gcc
endif

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

ifeq ($(NXCLIENT_SUPPORT),y)
        # Nexus NxClient Mode: NXCLIENT_SUPPORT=y, NEXUS_CLIENT_SUPPORT=<don't care>
        include $(NEXUS_TOP)/nxclient/include/nxclient.inc
        CFLAGS += ${NXCLIENT_CFLAGS}
        CXXFLAGS += ${NXCLIENT_CFLAGS}
        LDFLAGS += ${NXCLIENT_LDFLAGS}
else
	ifeq ($(NEXUS_CLIENT_SUPPORT),y)
        # Nexus Multiprocess non-NxClient Mode: NXCLIENT_SUPPORT=, NEXUS_CLIENT_SUPPORT=y
        LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES) -lm
    else
        # Nexus Single-process Mode: NXCLIENT_SUPPORT=, NEXUS_CLIENT_SUPPORT=
        LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES) -lm
    endif
endif



include $(NEXUS_TOP)/lib/bip/bip_lib.inc

CFLAGS  += $(BIP_ALL_LIB_CFLAGS)
LDFLAGS += $(BIP_ALL_LIB_LDFLAGS)


$(info CFLAGS = $(CFLAGS))

# Always use NEXUS_CFLAGS (with or without NxClient).
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS+=-I../ -Wall -Wformat -g -std=gnu99

all: ixia_dut_asp install

.PHONY: nexus_libs
nexus_libs:
    ifeq ($(NXCLIENT_SUPPORT),y)
		@echo "[Build... nxclient (server)]"
		$(Q_)B_REFSW_REAL_MAKE= $(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/nxclient/server
    else
		@echo "[Build... nexus]"
		$(Q_)$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build
    endif

bip_libs:
	@echo "[Building ... bip]"
	$(MAKE) -C $(NEXUS_TOP)/lib/bip/build

ixia_dut_asp: mntcpapp.o mntcplnx.o ../../asp/emulation/dut/asp_manager.o ../../asp/emulation/dut/asp_connx_migration.o ../../asp/emulation/dut/asp_proxy_server_messages.o bip_libs nexus_libs
	$(CC) -o $@ mntcpapp.o mntcplnx.o ../../asp/emulation/dut/asp_manager.o ../../asp/emulation/dut/asp_connx_migration.o ../../asp/emulation/dut/asp_proxy_server_messages.o -lbip_base -L../../../obj.$(NEXUS_PLATFORM)/nexus/bin -lb_os -lpthread -lnexus -lb_psip -lb_playback_ip -L../../../obj.97445/BSEAV/lib/livemedia/blive_ext/lib/arm-linux.debug/  -lstdc++
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
-include ../../../obj.$(NEXUS_PLATFORM)/nexus/bin/*.d

DEPS := $(patsubst %.o,%.d,$(OBJS))

clean:
	rm -rf *.o; rm -f *.d; rm -rf ../*.o; rm -f ixia_dut_asp
	@echo "[Cleaning... bip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/bip/build   clean

install:
	cp -f ixia_dut_asp ../../../obj.$(NEXUS_PLATFORM)/nexus/bin
PHONY: all clean install
