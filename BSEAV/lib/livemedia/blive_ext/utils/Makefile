############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#  $brcm_Workfile: $
#  $brcm_Revision: $
#  $brcm_Date: $
# 
#  Revision History:
#  $brcm_Log: $
# 
############################################################

include ../../../../api/build/tools.mak
BSEAV = $(shell cd "../../../.." && ${PWD})

include $(BSEAV)/build/refsw_inc.mak
include $(BSEAV)/lib/livemedia/blive_ext/include/blive_ext.mak

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

