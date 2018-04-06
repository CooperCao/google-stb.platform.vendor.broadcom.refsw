###############################################################################$
#    Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.$
###############################################################################$

ifdef COMSPEC
# Any DOS environment
NEXUS_TOP ?= $(shell cd ../../../../../../nexus && cd)
else
NEXUS_TOP ?= $(shell cd ../../../../../../nexus; pwd)
endif

BSEAV_DIR     := $(NEXUS_TOP)/../BSEAV
LIBVULKAN_DIR := $(BSEAV_DIR)/opensource/libvulkan

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error Neither NEXUS_PLATFORM or PLATFORM is defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

# build paths
include ../common/common.inc

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

CFLAGS += -lm -lrt
CFLAGS += -I$(V3D_DIR)/libs/vulkan/include/

APP = hello_vulkan
SRC = hello_vulkan.cpp

TARGET_DIR := $(B_REFSW_OBJ_ROOT)/$(shell pwd | sed 's/.*\($(BASEPATH)\)/\1/g')

all: $(TARGET_DIR) $(APP)

include $(LIBVULKAN_DIR)/b_libvulkan_targets.inc
include $(LIBVULKAN_DIR)/libvulkan.inc

CFLAGS += -I$(LIBVULKAN_SOURCE_PATH)
libvulkan: b_libvulkan_target_build
clean_libvulkan: b_libvulkan_target_clean_all

.PHONY: api nexus_api clean libvulkan

vulkan:	nexus_api
	$(MAKE) libvulkan
	@install -c -m 0755 $(LIBVULKAN_LIB_FOLDER)/libvulkan.so.${LIBVULKAN_VER} ${NEXUS_BIN_DIR}
	@cd ${NEXUS_BIN_DIR} && ln -s -f libvulkan.so.${LIBVULKAN_VER} libvulkan.so.${LIBVULKAN_VER_MAJ}
	@cd ${NEXUS_BIN_DIR} && ln -s -f libvulkan.so.${LIBVULKAN_VER_MAJ} libvulkan.so

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
LDFLAGS := -lpthread

ifneq ($(wildcard $(V3D_DIR)),)
LDFLAGS += -L$(LIBVULKAN_LIB_FOLDER) -lvulkan
endif

ifneq ($(wildcard $(V3D_DIR)),)
api: nexus_api vulkan
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y
else
api: nexus_api
	@echo "This platform does not have V3D hardware"
endif

ifeq ($(B_NEXUS_API_BUILD_COMPLETED),y)
nexus_api:
else
ifeq ($(NXCLIENT_SUPPORT),y)
nexus_api:
	$(MAKE) -C $(NEXUS_TOP)/nxclient server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build
else
nexus_api:
	$(MAKE) -C $(NEXUS_TOP)/build
endif
endif

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

$(APP): api
	@echo [Compile... $<]
	$(C++) -o $(TARGET_DIR)/$@ $(SRC) $(CXXFLAGS) $(LDFLAGS)
	$(CP) $(TARGET_DIR)/$@ ${NEXUS_BIN_DIR}
	$(CP) -r resources_hello_vulkan ${NEXUS_BIN_DIR}

.PHONY: clean_api clean_v3d clean_app clean_nxclient clean_libvulkan
# This cleans nexus and local apps
clean: clean_api clean_v3d clean_app clean_nxclient clean_libvulkan

clean_api:
	$(MAKE) -C $(NEXUS_TOP)/build clean

ifneq ($(wildcard $(V3D_DIR)),)
clean_v3d:
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET) clean
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y clean
else
clean_v3d:
	#
endif

clean_app:
	$(RM) $(TARGET_DIR)/$(APP) $(TARGET_DIR)/*.d $(TARGET_DIR)/*.o $(TARGET_DIR)/*.out;

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

CXXFLAGS := $(filter-out -std=c99,$(CFLAGS)) -std=c++11

# Always build with debug
CFLAGS += -g

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

ifeq ($(wildcard $(V3D_DIR)),)
CFLAGS += -DNO_V3D
endif

install: all
