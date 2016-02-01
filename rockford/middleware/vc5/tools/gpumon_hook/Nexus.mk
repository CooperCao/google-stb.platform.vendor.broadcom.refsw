
$(info *** Making GPU Monitor Hook ***)

NEXUS_TOP ?= ../../../../../nexus
ROCKFORD_TOP ?= $(NEXUS_TOP)/../rockford

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

all: copy_to_bin

V3D_DIR ?= $(ROCKFORD_TOP)/middleware/vc5/driver
V3D_PLATFORM_DIR ?= $(ROCKFORD_TOP)/middleware/vc5/platform

CFLAGS += -I. -I$(V3D_DIR)/interface/khronos/include -I$(V3D_DIR)
## CAUTION: Using higher optimsation levels causes a SEGV when getting state
#CFLAGS += -O0 -fPIC -DPIC -fvisibility=hidden
CFLAGS += -O0 -fPIC -DPIC -DBCG_ABSTRACT_PLATFORM -std=c++0x
CFLAGS += -c $(foreach dir,$(NEXUS_APP_INCLUDE_PATHS),-I$(dir)) $(foreach def,$(NEXUS_APP_DEFINES),-D"$(def)")

LDFLAGS = -shared -Wl,--export-dynamic

.PHONY: copy_to_bin
copy_to_bin : lib/libgpumon_hook.so
	$(info Copying libgpumon_hook.so to $(NEXUS_BIN_DIR))
	@cp lib/libgpumon_hook.so $(NEXUS_BIN_DIR)
	@ln -sf libv3ddriver.so libkhronos.so
	@mv libkhronos.so $(NEXUS_BIN_DIR)
	@cp gpumon $(NEXUS_BIN_DIR)
	@cp capture $(NEXUS_BIN_DIR)
	@chmod 777 $(NEXUS_BIN_DIR)/gpumon

lib/libgpumon_hook.so : obj/gpumon_hook.o obj/api.o obj/remote.o obj/archive.o obj/packet.o obj/platform.o obj/circularbuffer.o
	@mkdir -p lib
	$(info Linking $@)
	@$(B_REFSW_CROSS_COMPILE)g++ $(LDFLAGS) -o $@ $^

obj/%.o: %.cpp
	@mkdir -p obj
	$(info Compiling $<)
	@$(B_REFSW_CROSS_COMPILE)g++ -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -rf lib obj
