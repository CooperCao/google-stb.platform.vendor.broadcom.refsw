############################################################
#     Copyright (c) 2013-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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

ifdef COMSPEC
# Any DOS environment
B_LIB_TOP := $(shell cd ../../../../nexus/lib && cd)
B_CABLE_TOP := $(shell cd ../../../../BSEAV/cable && cd)
B_CABLE_TOP := $(shell cd ../../../../BSEAV && cd)
else
B_LIB_TOP := $(shell cd ../../../../nexus/lib; pwd)
B_CABLE_TOP := $(shell cd ../../../../BSEAV/cable; pwd)
B_BSEAV_TOP := $(shell cd ../../../../BSEAV && pwd)
CUR_DIR := $(shell pwd)
endif
NEXUS_TOP := $(B_LIB_TOP)/..

ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(B_LIB_TOP)/build/b_lib_defs.inc
THEAPPS = \
	mksfs \
	mkvsfs \
        tsb \
        tsbconvert \
        record \
        playback \
        vod \
        stream_permrecording \
        stream_tsb \
        programlist \
	tsb_from_playback \
	record_from_playback 

APPS = $(THEAPPS)
all: $(APPS)
$(APPS): api
.PHONY: api clean

# add flags for tshdrbuilder lib
include $(B_BSEAV_TOP)/lib/tshdrbuilder/tshdrbuilder.inc
CFLAGS += $(addprefix -I,$(TSHDRBUILDER_INCLUDES))
CFLAGS += -Wno-long-long
# include applib specifics
include $(B_LIB_TOP)/os/b_os_lib.inc
include $(B_CABLE_TOP)/lib/dvr/b_dvr_lib.inc

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lm -L$(NEXUS_BIN_DIR) -lpthread -lnexus -lb_os  -lb_dvr  -ltshdrbuilder
endif

# This builds nexus and needed applibs
api:
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(B_LIB_TOP)/os
	$(MAKE) -C $(B_CABLE_TOP)/lib/dvr
	$(MAKE) -C $(B_BSEAV_TOP)/lib/tshdrbuilder 
# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(B_LIB_TOP)/os clean
	$(MAKE) -C $(B_CABLE_TOP)/lib/dvr clean
	$(MAKE) -C $(B_BSEAV_TOP)/lib/tshdrbuilder clean
	-$(RM) $(APPS) *.d *.o *.out;

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Convert applib include paths into single variable.
APPLIB_INCLUDES := $(foreach lib,$(B_LIBS),$($(lib)_PUBLIC_INCLUDES))
CFLAGS += $(addprefix -I,$(APPLIB_INCLUDES))
APPLIB_DEFINES := $(foreach lib,$(B_LIBS),$($(lib)_DEFINES))
CFLAGS += $(addprefix -D,$(APPLIB_DEFINES))
#CFLAGS += $(B_DCC_LIB_CFLAGS_OPTIONS)


# Always build with debug
CFLAGS += -g

# building local apps
mksfs: %: %.c 
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	$(Q_)$(CP) $@ $(NEXUS_BIN_DIR)

mkvsfs: %: %.c 
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	$(Q_)$(CP) $@ $(NEXUS_BIN_DIR)

tsb_from_playback: %: %.c 
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	$(Q_)$(CP) $@ $(NEXUS_BIN_DIR)

record_from_playback: %: %.c 
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	$(Q_)$(CP) $@ $(NEXUS_BIN_DIR)

tsb tsbconvert record playback stream_tsb stream_permrecording vod programlist : %: %.c util.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	$(Q_)$(CP) $@ $(NEXUS_BIN_DIR)
#
# Installation: copying app to DESTDIR
#
ifeq ($(DESTDIR),)
DESTDIR = $(NEXUS_BIN_DIR)
endif

.PHONY: install install_only 
install: all install_only 

ifeq ($(APP),)
install_only:
	$(CP) $(APPS) $(DESTDIR)
else
install_only:
	$(CP) $(APP) $(DESTDIR)
endif
