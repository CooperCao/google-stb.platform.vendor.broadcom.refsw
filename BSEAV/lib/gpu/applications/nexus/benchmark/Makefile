###############################################################################
#    Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

APPS = benchmark

.PHONY: api nexus_api libpng clean

TARGET_DIR := $(B_REFSW_OBJ_ROOT)/$(shell pwd | sed 's/.*\($(BASEPATH)\)/\1/g')

all: $(TARGET_DIR) $(APPS)
$(APPS): api

LIBPNG_DIR = $(NEXUS_TOP)/../BSEAV/opensource/libpng
include $(LIBPNG_DIR)/b_libpng_targets.inc
include $(LIBPNG_DIR)/libpng.inc # WORKAROUND for b_libpng_targets.inc missing this include
libpng: b_libpng_target_build
clean_libpng: b_libpng_target_clean_all

LDFLAGS = -lm -lrt -lstdc++

CFLAGS = -std=c99 \
          -I../common \
          -I$(V3D_DIR)/interface/khronos/include/ \
          -I$(V3D_PLATFORM_DIR)/nexus
CFLAGS += $(LIBPNG_CFLAGS)

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

LDFLAGS += -lpthread \
		-lv3ddriver \
		-lnxpl
LDFLAGS += $(LIBPNG_LDFLAGS)
LDFLAGS += -Wl,-rpath-link=$(LIBPNG_LIB_FOLDER)

api: nexus_api
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y

ifeq ($(NXCLIENT_SUPPORT),y)
nexus_api:
	$(MAKE) -C $(NEXUS_TOP)/nxclient server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build
else
nexus_api:
	$(MAKE) -C $(NEXUS_TOP)/build
endif

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

.PHONY: clean_api clean_v3d clean_app clean_nxclient clean_libpng
# This cleans nexus and local apps
clean: clean_api clean_v3d clean_app clean_nxclient clean_libpng

clean_api:
	$(MAKE) -C $(NEXUS_TOP)/build clean

clean_v3d:
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET) clean
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y clean

clean_app:
	$(RM) $(TARGET_DIR)/$(APPS) $(TARGET_DIR)/*.d $(TARGET_DIR)/*.o $(TARGET_DIR)/*.out;

ifeq ($(USE_NXCLIENT),1)
clean_nxclient:
	$(MAKE) -C $(NEXUS_TOP)/nxclient clean_server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build clean
else
clean_nxclient:
	#
endif

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES)) -I$(NEXUS_TOP)/../BSEAV/lib/netaccel/include

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

SOURCES := \
	../common/esutil.c \
	../common/init.c \
	load_texture.c \
	benchmark.cpp

# $(1) = src
# $(2) = obj
# $(3) = CC & FLAGS
define CCompileRule
OBJECTS += $(2)
$(2) : $(1) \
	libpng
		$(Q_)echo [Compiling... $(1)]
		$(Q_)$(3) -c -o "$(2)" "$(1)"
endef

ifeq ("$(GCCGTEQ_40800)", "1")
CXXFLAGS = $(filter-out -std=c99,$(CFLAGS)) -std=c++11
else
CXXFLAGS = $(filter-out -std=c99,$(CFLAGS)) -std=c++0x
endif

$(foreach src,$(filter %.c,$(SOURCES)),$(eval $(call CCompileRule,$(src),$(addprefix $(TARGET_DIR)/,$(basename $(notdir $(src))).o),$(CC) $(CFLAGS))))
$(foreach src,$(filter %.cpp,$(SOURCES)),$(eval $(call CCompileRule,$(src),$(addprefix $(TARGET_DIR)/,$(basename $(notdir $(src))).o),$(C++) $(CXXFLAGS))))

# Implicit rule for building local apps
$(APPS) : $(OBJECTS)
	$(Q_)echo [Link... $@]
	$(Q_)$(C++) $(filter %.cpp %.c %.s %.o, $^) $(LDFLAGS) -o $(TARGET_DIR)/$@
	$(Q_)cp -af $(TARGET_DIR)/$@ $(NEXUS_BIN_DIR)
	$(Q_)cp -af *.txt $(NEXUS_BIN_DIR)
	$(Q_)cp -af earth_cloud0.pkm $(NEXUS_BIN_DIR)
	$(Q_)echo -e "\n\n*********************************************************************************"
	$(Q_)echo "You can run the benchmark with './nexus benchmark batch=depth.txt +s d=1280x720'"
	$(Q_)echo "or './nexus benchmark batch=nodepth.txt +s d=1280x720 -depth -stencil'"
	$(Q_)echo "*********************************************************************************"
