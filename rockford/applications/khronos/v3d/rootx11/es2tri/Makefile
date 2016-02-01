############################################################

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

# include applibs definitions
APPLIBS_TOP:=$(NEXUS_TOP)/../AppLibs
include $(APPLIBS_TOP)/common/common.inc

V3D_DIR ?= $(NEXUS_TOP)/../rockford/middleware/v3d
V3D_PLATFORM_DIR ?= $(NEXUS_TOP)/../rockford/middleware/platform

CFLAGS += -lm -lrt
CFLAGS += -I../common \
          -I$(APPLIBS_TARGET_INC_DIR) \
          -I$(APPLIBS_TARGET_INC_DIR)/libdrm \
          -I$(V3D_DIR)/interface/khronos/include \
          -I$(V3D_PLATFORM_DIR)/rootx11

APPS = es2tri

.PHONY: api clean

all: $(APPS)
$(APPS): api

ifeq ($(V3D_DEBUG),y)
V3D_LIB := lib_$(NEXUS_PLATFORM)_debug
else
V3D_LIB := lib_$(NEXUS_PLATFORM)_release
endif

NEXUS_MODE ?= $(MODE)
ifeq ($(NEXUS_MODE),)
CFLAGS += -DSINGLE_PROCESS
USE_NXCLIENT ?= 0
else
USE_NXCLIENT ?= 1
endif

ifeq ($(NEXUS_MODE),proxy)
ifeq ($(CLIENT),)
CFLAGS += -DSINGLE_PROCESS
USE_NXCLIENT ?= 0
else
USE_NXCLIENT ?= 1
endif
endif

ifeq ($(NEXUS_MODE),client)
CFLAGS += -DUSER_MODE_MULTIPROC_CLIENT
USE_NXCLIENT ?= 1
endif

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES)) -I$(NEXUS_TOP)/../BSEAV/lib/netaccel/include

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lnexus$(NEXUS_LIB_SUFFIX) -L${NEXUS_BIN_DIR} -lpthread \
		-L$(APPLIBS_TARGET_LIB_DIR) -lX11 -lxcb -lXau -lXdmcp -ldri2 -lXext -ldrm \
		-L$(V3D_DIR)/$(V3D_LIB) -lv3ddriver \
		-L$(V3D_PLATFORM_DIR)/rootx11/$(V3D_LIB) -lrxpl
endif

api:
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk
	$(MAKE) -C $(V3D_PLATFORM_DIR)/rootx11 -f platform_rootx11.mk USE_NXCLIENT=0

.PHONY: clean_api clean_v3d clean_app
# This cleans nexus and local apps
clean: clean_nxclient clean_api clean_v3d clean_app

clean_api:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	
clean_v3d:
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk clean
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk clean
	
clean_app:
	-$(RM) $(APPS) *.d *.o *.out;

ifeq ($(USE_NXCLIENT),1)
clean_nxclient:
	$(MAKE) -C $(NEXUS_TOP)/nxclient clean_server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build clean
else
clean_nxclient:
	#
endif

# Always build with debug
CFLAGS += -g

# Implicit rule for building local apps
%$(APPSUFFIX): %.c $(EXTRA_OBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	cp $@ ${NEXUS_BIN_DIR}

install: all

