################################################################################
#   Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
################################################################################

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

ifndef B_REFSW_ARCH
B_REFSW_ARCH = mipsel-linux
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

include ../../nexus/common/common.inc

APPLIBS_TOP:=$(NEXUS_TOP)/../AppLibs
include $(APPLIBS_TOP)/opensource/directfb/build/directfb_common.inc

THEAPPS = eglimage

CFLAGS += -I$(V3D_DIR)/interface/khronos/include/ \
		  -I$(DIRECTFB_INSTALL_INCLUDE_DIR)/directfb \
		  -I../../nexus/common \
		  -I$(V3D_PLATFORM_DIR)/directfb

CFLAGS += -DGL_GLEXT_PROTOTYPES -D$(V3D_PREFIX) -DEGL_EGLEXT_PROTOTYPES

APPS = $(THEAPPS)

.PHONY: api directfb_check clean

ifeq ($(APP),)
all: $(APPS)
$(APPS): api
else
all: $(APP)
$(APP): api
endif

ifeq ($(V3D_DEBUG),y)
V3D_LIB := lib_$(NEXUS_PLATFORM)_debug
else
V3D_LIB := lib_$(NEXUS_PLATFORM)_release
endif

V3DDRIVER_LIB_TARGET := $(APPLIBS_TARGET_LIB_DIR)
V3DDRIVER_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/$(shell sed 's/.*\($(BASEPATH)\)/\1/g' <<< $(V3D_DIR))/$(V3D_OBJ)
DBPL_LIB_TARGET := $(APPLIBS_TARGET_LIB_DIR)
DBPL_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/$(shell sed 's/.*\($(BASEPATH)\)/\1/g' <<< $(V3D_PLATFORM_DIR))/$(V3D_OBJ)

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lnexus$(NEXUS_LIB_SUFFIX) -L${NEXUS_BIN_DIR} -lpthread -lm -lrt \
		-L$(APPLIBS_TARGET_LIB_DIR) \
		-lv3ddriver \
		-ldbpl \
		-L$(DIRECTFB_INSTALL_LIB_DIR) \
		-Wl,-rpath-link,$(DIRECTFB_INSTALL_LIB_DIR) \
		-ldirectfb \
		-lstdc++
endif

# Check to make sure DirectFB is already built
directfb_check:
	@if [ ! -d $(DIRECTFB_INSTALL_DIRECTFB_INCLUDE_DIR) ]; then \
		echo "ERROR: Cannot find directfb installation - aborting!"; \
		/bin/false; \
	fi

# This builds the nexus api, v3d driver & platform glue
api: directfb_check
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)
	$(MAKE) -C $(V3D_PLATFORM_DIR)/directfb -f platform_directfb.mk OBJDIR=$(DBPL_OBJ_TARGET) LIBDIR=$(DBPL_LIB_TARGET)

# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET) clean
	$(MAKE) -C $(V3D_PLATFORM_DIR)/directfb -f platform_directfb.mk OBJDIR=$(DBPL_OBJ_TARGET) LIBDIR=$(DBPL_LIB_TARGET) clean
	-$(RM) $(APPS) *.d *.o *.out;

# DFB has lots of warnings if not gnu99
# Always build with gnu99 & debug
CFLAGS += -std=gnu99 -g

EXTRA_OBJS := ../../nexus/common/esutil.c

# Implicit rule for building local apps
%$(APPSUFFIX): %.c $(EXTRA_OBJS) $(VXOBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $(APPLIBS_TARGET_ROOTDIR)/$(DIRECTFB_TARGET_BIN_DFB_VER_DIR)/$@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	mkdir -p $(APPLIBS_TARGET_ROOTDIR)/$(DIRECTFB_TARGET_BIN_DFB_VER_DIR)/textures; \
	install -m 664 ../../nexus/eglimage/*.png $(APPLIBS_TARGET_ROOTDIR)/$(DIRECTFB_TARGET_BIN_DFB_VER_DIR)/textures; \
	cd $(APPLIBS_TARGET_ROOTDIR)/$(DIRECTFB_TARGET_BIN_DFB_VER_DIR); \
	ln -sf ../../../share/directfb-${DIRECTFB_VERSION}/textures textures

install: all
