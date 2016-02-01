#
# make -f Android.make B_REFSW_ARCH=arm-linux-androideabi B_REFSW_CROSS_COMPILE=arm-linux-androideabi- B_REFSW_TOOLCHAIN_ARCH=arm-linux-androideabi ROOT=/scratch/hauxwell/android-l NEXUS_PLATFORM=97252 BCHP_VER=D0
#

ifeq ($(VERBOSE),)
Q := @
endif

$(info *** Making GPU Monitor Hook ***)

$(info $(NEXUS_DEFINES))

all: lib/libgpumonitor.so

NEXUS_TOP:=$(ROOT)/vendor/broadcom/stb/refsw/nexus

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

NEXUS_CFLAGS:=$(filter-out -Wstrict-prototypes,$(NEXUS_CFLAGS))
NEXUS_CFLAGS:=$(filter-out -std=c89,$(NEXUS_CFLAGS))

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

CFLAGS += -c $(foreach dir,$(NEXUS_APP_INCLUDE_PATHS),-I$(dir)) $(foreach def,$(NEXUS_APP_DEFINES),-D"$(def)")

CFLAGS += -I$(ROOT)/external/libcxx/include $(STL_PORT) \
				-isystem $(ROOT)/system/core/include \
				-isystem $(ROOT)/bionic \
				-isystem $(ROOT)/bionic/libc/arch-arm/include \
				-isystem $(ROOT)/bionic/libc/include \
				-isystem $(ROOT)/bionic/libstdc++/include \
				-isystem $(ROOT)/bionic/libc/kernel/uapi \
				-isystem $(ROOT)/bionic/libc/kernel/uapi/asm-arm \
				-isystem $(ROOT)/bionic/libm/include \
				-isystem $(ROOT)/bionic/libm/include/arm \
				-I$(ROOT)/system/core/include \
				-I$(ROOT)/bionic \
				-I$(ROOT)/bionic/libc/arch-arm/include \
				-I$(ROOT)/bionic/libc/include \
				-I$(ROOT)/bionic/libstdc++/include \
				-I$(ROOT)/bionic/libc/kernel/uapi \
				-I$(ROOT)/bionic/libc/kernel/uapi/asm-arm \
				-I$(ROOT)/bionic/libm/include \
				-I$(ROOT)/bionic/libm/include/arm \
				-I. \
				-I$(NEXUS_TOP)/../rockford/middleware/v3d/driver/interface/khronos/include \
				-I$(NEXUS_TOP)/../rockford/middleware/v3d/driver

## CAUTION: Using higher optimsation levels causes a SEGV when getting state
#CFLAGS += -O0 -fPIC -DPIC -fvisibility=hidden
CFLAGS += \
	-O0 \
	-fPIC \
	-DPIC \
	-DANDROID \
	-DHAVE_SYS_UIO_H \
	-fno-rtti \
	-fno-exceptions \
	-fno-use-cxa-atexit \
	-std=c++0x

CFLAGS += \
	-DLOGD=ALOGD \
	-DLOGE=ALOGE

LDFLAGS = -nostdlib -Wl,--gc-sections -Wl,-shared,-Bsymbolic \
	-fno-rtti -fno-exceptions -fno-use-cxa-atexit \
	-L$(ROOT)/out/target/product/bcm_platform/system/lib/ \
	-Wl,--no-whole-archive -lcutils \
	-lstdc++ \
	-lc \
	-lm \
	-lgcc \
	-lcutils \
	-llog \
	-lc++ \
	-lnexus \
	-Wl,--no-undefined -Wl,--whole-archive -Wl,--fix-cortex-a8 \
	-z defs

lib/libgpumonitor.so : obj/spyhook.o obj/remote.o obj/archive.o obj/packet.o obj/platform.o
	$(Q)mkdir -p lib
	$(info Linking $@)
	$(Q)$(C++) $(NEXUS_LDFLAGS) $(LDFLAGS) -o $@ $^

obj/%.o: %.cpp
	$(Q)mkdir -p obj
	$(info Compiling $<)
	$(Q)$(C++) -c $(NEXUS_CFLAGS) $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -rf lib obj
