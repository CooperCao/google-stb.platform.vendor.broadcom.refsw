#**************************************************************************
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
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
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#*************************************************************************/
include ../../../../api/build/tools.mak
BSEAV = $(shell cd "../../../.." && ${PWD})

include $(BSEAV)/build/refsw_inc.mak
include $(BSEAV)/opensource/livemedia/blive_ext/include/blive_ext.mak

.PHONY: clean settop_lib livemedia_lib

ifeq ($(VERBOSE),)
MAKEFLAGS += -s
endif

ifeq ($(SYSTEM),vxworks)
CFLAGS += -I$(WIND_BASE)/target/h
endif
CFLAGS = -W -Wall -g
CFLAGS += $(BLIVE_EXT_CFLAGS)
CFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS) \
	-I$(LIVEMEDIA_DIR)/live/liveMedia//include \
	-I$(LIVEMEDIA_DIR)/live/groupsock/include \
	-I$(LIVEMEDIA_DIR)/live/UsageEnvironment/include \
	-I$(LIVEMEDIA_DIR)/live/BasicUsageEnvironment/include

CXXFLAGS += $(CFLAGS)

LDFLAGS += -static
LDFLAGS += $(BLIVE_EXT_LDFLAGS)
LDFLAGS += $(BSETTOP_LDFLAGS)
LDFLAGS += -lpthread
LDFLAGS += -L$(BSEAV)/api/lib/linux97401B0_mipsel-uclibc.debug -lsettop
LDFLAGS += -L$(LIVEMEDIA_DIR)/live/liveMedia -lliveMedia
LDFLAGS += -L$(LIVEMEDIA_DIR)/live/groupsock -lgroupsock
LDFLAGS += -L$(LIVEMEDIA_DIR)/live/UsageEnvironment -lUsageEnvironment
LDFLAGS += -L$(LIVEMEDIA_DIR)/live/BasicUsageEnvironment -lBasicUsageEnvironment

vpath %.c ./
vpath %.cpp ./

SAP_OBJS = sapDemo.o
SCHEDULER_DEMO_OBJS = schedulerDemo.o
SCHEDULER_TEST_OBJS = schedulerTest.o
MCAST_TEST_OBJS = mcastTest.o

SAP_DEMO = sapDemo
SCHEDULER_DEMO = schedulerDemo
SCHEDULER_TEST = schedulerTest
MCAST_TEST = mcastTest

all: $(SAP_DEMO) $(SCHEDULER_DEMO) $(SCHEDULER_TEST) $(MCAST_TEST)

$(SAP_DEMO): settop_lib livemedia_lib blive_ext $(SAP_OBJS)
	@echo [Linking... $(notdir $@)]
	${Q_}$(CXX) $(CXXFLAGS) -o $@ $(SAP_OBJS) $(LDFLAGS)

$(SCHEDULER_DEMO): settop_lib livemedia_lib blive_ext $(SCHEDULER_DEMO_OBJS)
	@echo [Linking... $(notdir $@)]
	${Q_}$(CXX) $(CXXFLAGS) -o $@ $(SCHEDULER_DEMO_OBJS) $(LDFLAGS)

$(SCHEDULER_TEST): settop_lib livemedia_lib blive_ext $(SCHEDULER_TEST_OBJS)
	@echo [Linking... $(notdir $@)]
	${Q_}$(CXX) $(CXXFLAGS) -o $@ $(SCHEDULER_TEST_OBJS) $(LDFLAGS)

$(MCAST_TEST): settop_lib livemedia_lib blive_ext $(MCAST_TEST_OBJS)
	@echo [Linking... $(notdir $@)]
	${Q_}$(CXX) $(CXXFLAGS) -o $@ $(MCAST_TEST_OBJS) $(LDFLAGS)

%.o: %.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) $(CXXFLAGS) -c $< -o $@

settop_lib:
	@if ! $(MAKE) -q -C $(BSEAV)/api/build VERBOSE=y PLAYBACK_IP_SUPPORT=y B_HAS_PLAYPUMP_IP=y KERNELMODE_SETTOPAPI=y LIVEMEDIA_SUPPORT=y; then \
		echo Building Settop library; \
		${MAKE} -C $(BSEAV)/api/build VERBOSE=y PLAYBACK_IP_SUPPORT=y B_HAS_PLAYPUMP_IP=y KERNELMODE_SETTOPAPI=y LIVEMEDIA_SUPPORT=y; \
	fi

livemedia_lib:
	@if ! $(MAKE) -q -C $(LIVEMEDIA_DIR)/live/liveMedia; then \
		echo Building LiveMedia library; \
		${MAKE} -C $(LIVEMEDIA_DIR)/live; \
		${MAKE} -C $(LIVEMEDIA_DIR)/live/liveMedia; \
	fi

blive_ext:
	@if ! $(MAKE) -q -C $(BLIVE_EXT_DIR)/build; then \
		echo Building LIVE_EXT library; \
		${MAKE} -C $(BLIVE_EXT_DIR)/build; \
	fi

clean:
	$(RM) -rf $(SAP_OBJS) $(SAP_DEMO)
	$(RM) -rf $(SCHEDULER_OBJS) $(SCHEDULER_DEMO) $(SCHEDULER_Test)
	$(RM) -rf $(MCAST_TEST_OBJS) $(MCAST_TEST)
	${MAKE} -C $(BLIVE_EXT_DIR)/build clean
	${MAKE} -C $(BSEAV)/api/build clean
	${MAKE} -C $(LIVEMEDIA_DIR)/live clean

#include $(BSEAV)/api/build/rules.mak
