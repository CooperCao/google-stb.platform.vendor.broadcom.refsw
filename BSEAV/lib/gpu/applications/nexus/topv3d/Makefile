###############################################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
###############################################################################

ifdef COMSPEC
# Any DOS environment
NEXUS_TOP ?= $(shell cd ../../../../../../nexus && cd)
else
NEXUS_TOP ?= $(shell cd ../../../../../../nexus; pwd)
endif

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error Neither NEXUS_PLATFORM or PLATFORM is defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

GCCGTEQ_40800 := $(shell expr `$(CC) -dumpversion | awk 'BEGIN { FS = "." }; { printf("%d%02d%02d", $$1, $$2, $$3) }'` \>= 40800)

# build paths
include ../common/common.inc

APPS = topv3d

.PHONY: api clean

TARGET_DIR := $(B_REFSW_OBJ_ROOT)/$(shell pwd | sed 's/.*\($(BASEPATH)\)/\1/g')

all: $(TARGET_DIR) $(APPS)
$(APPS): api

ifeq ($(V3D_DEBUG),y)
V3D_LIB := lib_$(NEXUS_PLATFORM)_debug
V3D_OBJ := obj_$(NEXUS_PLATFORM)_debug
else
V3D_LIB := lib_$(NEXUS_PLATFORM)_release
V3D_OBJ := obj_$(NEXUS_PLATFORM)_release
endif

V3DDRIVER_LIB_TARGET := ${NEXUS_BIN_DIR}
V3DDRIVER_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/$(shell sed 's/.*\($(BASEPATH)\)/\1/g' <<< $(V3D_DIR))/$(V3D_OBJ)
NXPL_LIB_TARGET := ${NEXUS_BIN_DIR}
NXPL_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/$(shell sed 's/.*\($(BASEPATH)\)/\1/g' <<< $(V3D_PLATFORM_DIR))/$(V3D_OBJ)

# For linux builds, link to the correct libraries

ifeq ($(NXCLIENT_SUPPORT),y)
# This builds the nexus api, v3d driver & platform glue
api:
	$(MAKE) -C $(NEXUS_TOP)/nxclient server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build
else
api:
	$(error only supported on NXCLIENT_SUPPORT=y)
endif

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

.PHONY: clean_api clean_app clean_nxclient
# This cleans nexus and local apps
clean: clean_nxclient clean_api clean_app

clean_api:
	$(MAKE) -C $(NEXUS_TOP)/build clean

clean_app:
	$(RM) $(TARGET_DIR)/$(APPS) $(TARGET_DIR)/*.d $(TARGET_DIR)/*.o $(TARGET_DIR)/*.out;

clean_nxclient:
	$(MAKE) -C $(NEXUS_TOP)/nxclient clean_server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build clean

# This is the minimum needed to compile and link with Nexus
CXXFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES)) -I$(NEXUS_TOP)/../BSEAV/lib/netaccel/include

# Always build with debug
CXXFLAGS += -g
ifeq ("$(GCCGTEQ_40800)", "1")
CXXFLAGS += -std=c++11
else
CXXFLAGS += -std=c++0x -D_GLIBCXX_USE_NANOSLEEP -Dsteady_clock=monotonic_clock
endif

# add nxclient
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CXXFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS += $(NXCLIENT_LDFLAGS)

# Implicit rule for building local apps
%$(APPSUFFIX): %.cpp $(EXTRA_OBJS)
	@echo [Compile... $<]
	$(Q_)$(C++) -o $(TARGET_DIR)/$@ $(filter %.cpp %.s %.o, $^) $(CXXFLAGS) $(LDFLAGS)
	cp $(TARGET_DIR)/$@ ${NEXUS_BIN_DIR}

install: all
