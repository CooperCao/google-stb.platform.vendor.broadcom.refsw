NEXUS_TOP ?= $(shell cd ../../../../../nexus; pwd)

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

$(info *** Making GPU Monitor Hook for $(V3D_PREFIX) ***)

all: copy_to_bin

V3D_DIR ?= $(shell cd ../../$(V3D_PREFIX)/driver; pwd)
V3D_PLATFORM_DIR ?= $(shell cd ../../$(V3D_PREFIX)/platform; pwd)

GCCGTEQ_40800 := $(shell expr `$(CC) -dumpversion | awk 'BEGIN { FS = "." }; { printf("%d%02d%02d", $$1, $$2, $$3) }'` \>= 40800)
ifneq ("$(GCCGTEQ_40800)", "1")
$(error GCC >= 4.8 required to build hook)
endif

ifeq ($(V3D_PREFIX),vc5)
V3D_VER_INC_DIR = $(V3D_DIR)/libs/core/v3d
else #VC4
V3D_VER_INC_DIR = $(V3D_DIR)/helpers/v3d
endif

CFLAGS += -I. -I$(V3D_DIR)/interface/khronos/include -I$(V3D_VER_INC_DIR) -I$(V3D_DIR)
CFLAGS += -Os -fPIC -DPIC -DBCG_ABSTRACT_PLATFORM -std=c++0x -pthread
CFLAGS += -c $(foreach dir,$(NEXUS_APP_INCLUDE_PATHS),-I$(dir)) $(foreach def,$(NEXUS_APP_DEFINES),-D"$(def)")
CFLAGS += -I$(MAGNUM_TOP)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) -DEMBEDDED_SETTOP_BOX=1

LDFLAGS = -shared -Wl,--export-dynamic -pthread

.PHONY: copy_to_bin
copy_to_bin : lib/libgpumon_hook.so
	$(info Copying libgpumon_hook.so to $(NEXUS_BIN_DIR))
	@cp lib/libgpumon_hook.so $(NEXUS_BIN_DIR)
	@ln -sf libv3ddriver.so libkhronos.so
	@mv libkhronos.so $(NEXUS_BIN_DIR)
	@cp gpumon $(NEXUS_BIN_DIR)
	@cp capture $(NEXUS_BIN_DIR)
	@chmod 777 $(NEXUS_BIN_DIR)/gpumon

SOURCES := \
		gpumon_hook.cpp \
		api.cpp \
		remote.cpp \
		archive.cpp \
		datasinkasyncbuffer.cpp \
		datasinkbuffer.cpp \
		datasourcesinkfile.cpp \
		ringbuffer.cpp \
		packet.cpp \
		packetreader.cpp \
		platform.cpp \
		debuglog_linux.cpp

OBJECTS = $(SOURCES:%.cpp=obj/%.o)

lib/libgpumon_hook.so : $(OBJECTS)
	@mkdir -p lib
	$(info Linking $@)
	@$(B_REFSW_CROSS_COMPILE)g++ $(LDFLAGS) -o $@ $^

obj/%.o: %.cpp
	@mkdir -p obj
	$(info Compiling $<)
	@$(B_REFSW_CROSS_COMPILE)g++ -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -rf lib obj
