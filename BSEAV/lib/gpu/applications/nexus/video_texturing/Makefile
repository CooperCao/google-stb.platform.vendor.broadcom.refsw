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

ifeq ($(V3D_PREFIX),vc5)
CFLAGS += -DVC5
endif

# build paths
include ../common/common.inc

CFLAGS += -lm -lrt
CFLAGS += -I../common \
          -I$(V3D_DIR)/interface/khronos/include/ \
          -I$(V3D_PLATFORM_DIR)/nexus \
          -I$(NEXUS_TOP)/../BSEAV/lib/utils \
          -I$(NEXUS_TOP)/../BSEAV/lib/media \
          -I$(NEXUS_TOP)/nxclient/apps/utils \
          -I$(NEXUS_TOP)/utils

APPS = video_texturing

.PHONY: api nexus_api clean

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
LDFLAGS := -lpthread \
		-lv3ddriver \
		-lnxpl

api: nexus_api
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y

ifeq ($(NXCLIENT_SUPPORT),y)
# This builds the nexus api, v3d driver & platform glue
nexus_api:
	$(MAKE) -C $(NEXUS_TOP)/nxclient server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build
else
nexus_api:
	$(MAKE) -C $(NEXUS_TOP)/build
endif

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

.PHONY: clean_api clean_v3d clean_app clean_nxclient
# This cleans nexus and local apps
clean: clean_api clean_v3d clean_app clean_nxclient

clean_api:
	$(MAKE) -C $(NEXUS_TOP)/build clean

clean_v3d:
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET) clean
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y clean

clean_app:
	$(RM) $(TARGET_DIR)/$(APPS) $(TARGET_DIR)/*.d $(TARGET_DIR)/*.o $(TARGET_DIR)/*.out;

ifeq ($(NXCLIENT_SUPPORT),y)
clean_nxclient:
	$(MAKE) -C $(NEXUS_TOP)/nxclient clean_server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build clean
else
clean_nxclient:
	#
endif

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES)) -I$(NEXUS_TOP)/../BSEAV/lib/netaccel/include

ifeq ($(NXCLIENT_SUPPORT),y)
# add nxclient
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS += $(NXCLIENT_LDFLAGS)
else
ifeq ($(NEXUS_CLIENT_SUPPORT),y)
LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES)
else
CFLAGS += -DSINGLE_PROCESS
LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)
endif
endif

SOURCES := \
   video_texturing.cpp \
   video_decoder.cpp \
   media_data.cpp \
   texture_data.cpp \
   fence.cpp \
   geometry.cpp \
   shader_programs.cpp \
   camera.cpp \
   ../common/esutil.c \
   ../common/init.c \
   $(NEXUS_TOP)/nxclient/apps/utils/binput.c \
   $(NEXUS_TOP)/nxclient/apps/utils/media_probe.c \
   $(NEXUS_TOP)/nxclient/apps/utils/bfile_crypto.c \
   $(NEXUS_TOP)/utils/namevalue.c

OBJS0 := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SOURCES)))
OBJS0 += $(patsubst %.c,%.o,$(filter %.c,$(SOURCES)))
OBJS := $(addprefix $(TARGET_DIR)/, $(notdir $(OBJS0)))

define CCompileRule
$(2) : $(1) \
	$(AUTO_FILES)
		$(Q_)echo Compiling $(notdir $(1))
		$(Q_)$(3) -c -MMD -MP -MF"$(2:%.o=%.d)" -MT"$(2:%.o=%.d)" -o "$(2)" "$(1)"
endef

ifeq ("$(GCCGTEQ_40800)", "1")
CXXFLAGS := $(filter-out -std=c99,$(CFLAGS)) -std=c++11
else
CXXFLAGS := $(filter-out -std=c99,$(CFLAGS)) -std=c++0x -D_GLIBCXX_USE_NANOSLEEP -Dsteady_clock=monotonic_clock
endif

$(foreach src,$(filter %.c,$(SOURCES)),$(eval $(call CCompileRule,$(src),$(TARGET_DIR)/$(basename $(notdir $(src))).o,$(CC) $(CFLAGS))))
$(foreach src,$(filter %.cpp,$(SOURCES)),$(eval $(call CCompileRule,$(src),$(TARGET_DIR)/$(basename $(notdir $(src))).o,$(C++) $(CXXFLAGS))))

# Implicit rule for building local apps
$(APPS) : $(OBJS)
	$(Q_)echo [Link... $@]
	$(Q_)$(C++) $(LDFLAGS) -o $(TARGET_DIR)/$@ $(OBJS)
	$(Q_)cp -af $(TARGET_DIR)/$@ ${NEXUS_BIN_DIR}

install: all
