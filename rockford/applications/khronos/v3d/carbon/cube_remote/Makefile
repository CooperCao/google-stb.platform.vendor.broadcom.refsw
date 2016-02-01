############################################################

ifdef COMSPEC
# Any DOS environment
NEXUS_TOP := $(shell cd ../../../../../../nexus && cd)
else
NEXUS_TOP := $(shell cd ../../../../../../nexus; pwd)
endif

DEBUG ?= n

ifndef PLATFORM
$(error PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(PLATFORM)/build/platform_app.inc

THEAPPS = cube

CFLAGS += -lm -lrt
CFLAGS += -I$(MAGNUM)/../rockford/middleware/v3d/interface/khronos/include/ \
		  -I$(MAGNUM)/../rockford/middleware/platform/carbon_remote

APPS = $(THEAPPS)

.PHONY: api clean

ifeq ($(APP),)
all: $(APPS)
$(APPS): api
else
all: $(APP)
$(APP): api
endif

ifeq ($(DEBUG),y)
V3D_LIB := lib_$(PLATFORM)_debug
else
V3D_LIB := lib_$(PLATFORM)_release
endif

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lnexus -L${NEXUS_BIN_DIR} -lpthread \
		-L$(NEXUS_TOP)/../rockford/middleware/v3d/$(V3D_LIB) -lv3ddriver \
		-L$(NEXUS_TOP)/../rockford/middleware/platform/carbon_remote/$(V3D_LIB) -lcapl
endif

# This builds the nexus api, v3d driver & platform glue
api:
	$(MAKE) -C $(NEXUS_TOP)/build DEBUG=$(DEBUG)
	$(MAKE) -C $(NEXUS_TOP)/../rockford/middleware/v3d -f V3DDriver.mk DEBUG=$(DEBUG)
	$(MAKE) -C $(NEXUS_TOP)/../rockford/middleware/platform/carbon_remote -f platform_carbon.mk DEBUG=$(DEBUG)

# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build DEBUG=$(DEBUG) clean
	$(MAKE) -C $(NEXUS_TOP)/../rockford/middleware/v3d -f V3DDriver.mk DEBUG=$(DEBUG) clean
	$(MAKE) -C $(NEXUS_TOP)/../rockford/middleware/platform/carbon_remote -f platform_carbon.mk DEBUG=$(DEBUG) clean
	-$(RM) $(APPS) *.d *.o *.out;

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES)) -I$(NEXUS_TOP)/../BSEAV/lib/netaccel/include

# Always build with debug
CFLAGS += -g

EXTRA_OBJS := esutil.c ../common/init.c

# Implicit rule for building local apps
%$(APPSUFFIX): %.c $(EXTRA_OBJS) $(VXOBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	cp $@ ${NEXUS_BIN_DIR}

install: all

