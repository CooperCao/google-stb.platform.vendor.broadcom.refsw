# default build mode - release

PROFILING?=0

ifeq ($(VERBOSE),)
Q := @
endif

include ${PLATFORM_APP_INC}

$(info === Building $(MAKECMDGOALS) ===)

DRV = ../../driver

CC = $(B_REFSW_CROSS_COMPILE)gcc
CPP = $(B_REFSW_CROSS_COMPILE)g++

CFLAGS += \
	-fpic -DPIC -DBCG_ABSTRACT_PLATFORM \
	-std=c99 \
	-mcpu=cortex-a15 \
	-mfpu=neon \
	-I${PLATFORM_DIR}/android \
	-I${PLATFORM_DIR}/common \
	-I. \
	-I${DRV} \
	-I${DRV}/vcos \
	-I${DRV}/vcos/include \
	-I${DRV}/vcos/pthreads \
	-I${DRV}/v3d_platform \
	-I${DRV}/v3d_platform/bcg_abstract \
	-I${DRV}/v3d_platform/bcg_abstract/egl \
	-I${DRV}/interface/khronos/include \
	-I${DRV}/middleware/khronos/glsl \
	-Dkhronos_EXPORTS \
	-D_POSIX_C_SOURCE=200112 \
	-D_GNU_SOURCE \
	-DHAVE_ZLIB

CFLAGS += -shared

NEXUS_CFLAGS:=$(filter-out -Wstrict-prototypes,$(NEXUS_CFLAGS))
NEXUS_CFLAGS:=$(filter-out -std=c89,$(NEXUS_CFLAGS))
NEXUS_CFLAGS:=$(filter-out -march=%,$(NEXUS_CFLAGS))

# Add any customer specific cflags from the command line
CFLAGS += $(V3D_EXTRA_CFLAGS) $(NEXUS_CFLAGS)

HOOK_LDFLAGS = -nostdlib -Wl,--gc-sections -Wl,-shared,-Bsymbolic \
	-fno-rtti -fno-exceptions -fno-use-cxa-atexit \
	-L$(ANDROID_LIBDIR_LINK) \
	-Wl,--no-whole-archive -lcutils \
	-lstdc++ \
	-lc \
	-lm \
	-llog \
	-lc++ \
	-lnexus \
	-lnexuseglclient \
	-ldl \
	-Wl,--no-undefined -Wl,--whole-archive -Wl,--fix-cortex-a8 \
	-z defs

HOOK_LDFLAGS += $(V3D_EXTRA_LDFLAGS)

LDFLAGS += -export-dynamic

ifeq ($(V3D_DEBUG),y)

ifneq ($(PROFILING),0)
CFLAGS += -Os -g -DNDEBUG
else
CFLAGS += -O0 -g -fvisibility=hidden
endif

LDFLAGS += -g
OBJDIR ?= obj_$(NEXUS_PLATFORM)_debug
LIBDIR ?= lib_$(NEXUS_PLATFORM)_debug

else

CFLAGS += -Os -DNDEBUG

ifeq ($(PROFILING),0)
CFLAGS += -fvisibility=hidden
# Strip
LDFLAGS += -s
else
CFLAGS += -g
LDFLAGS += -g -export-dynamic
endif

OBJDIR ?= obj_$(NEXUS_PLATFORM)_release
LIBDIR ?= lib_$(NEXUS_PLATFORM)_release
endif

CPPFLAGS:=$(filter-out -std=c99,$(CFLAGS))

## CAUTION: Using higher optimsation levels causes a SEGV when getting state
#CFLAGS += -O0 -fPIC -DPIC -fvisibility=hidden
CPPFLAGS += \
	-O0 \
	-DANDROID \
	-DHAVE_SYS_UIO_H \
	-fno-rtti \
	-fno-exceptions \
	-fno-use-cxa-atexit \
	-g -funwind-tables \
	-std=c++0x

CPPFLAGS += -I$(ROOT)/external/libcxx/include $(STL_PORT) \
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
				-I$(ROOT)/bionic/libm/include/arm

HOOK_SOURCES = \
	gpumon_hook.cpp \
	api.cpp \
	archive.cpp \
	circularbuffer.cpp \
	packet.cpp \
	platform.cpp \
	remote.cpp

all: $(LIBDIR)/libgpumon_hook.so

.phony: OUTDIR
OUTDIR :
	$(Q)mkdir -p $(OBJDIR)

# $(1) = src
# $(2) = obj
define CCompileRuleHook
HOOK_OBJS += $(2)
$(2) : $(1)
	$(Q)echo Compiling $(1)
	$(Q)$(CPP) -c $(CPPFLAGS) -o "$(2)" "$(1)"

endef

$(foreach src,$(HOOK_SOURCES),$(eval $(call CCompileRuleHook,$(src),$(OBJDIR)/$(basename $(notdir $(src))).o)))

# $(1) = src
# $(2) = d
# $(3) = obj
define DependRule_CPP
$(2) : $(1) | OUTDIR $(AUTO_FILES)
	$(Q)echo Making depends for $(1)
	$(Q)touch $(2).tmp
	$(Q)$(CC) -D__SSE__ -D__MMX__ -M -MQ $(3) -MF $(2).tmp -MM $(CPPFLAGS) $(1)
	$(Q)sed 's/D:/\/\/D/g' < $(2).tmp | sed 's/C:/\/\/C/g' > $(2)
	$(Q)rm -f $(2).tmp

PRE_BUILD_RULES += $(2)
# Don't know why, but a comment on this line is necessary

endef

ifneq ($(MAKECMDGOALS),clean_hook)

$(foreach src,$(filter %.cpp,$(HOOK_SOURCES)),$(eval $(call DependRule_CPP,$(src),$(OBJDIR)/$(basename $(notdir $(src))).d,\
			$(OBJDIR)/$(basename $(notdir $(src))).o)))

$(foreach src,$(filter %.cpp,$(HOOK_SOURCES)),$(eval -include $(OBJDIR)/$(basename $(notdir $(src))).d))

endif

$(LIBDIR)/libgpumon_hook.so: $(PRE_BUILD_RULES) $(HOOK_OBJS)
	$(Q)echo Linking ... libgpumon_hook.so
	$(Q)mkdir -p $(LIBDIR)
	$(Q)$(CC) $(HOOK_LDFLAGS) -shared -o $(LIBDIR)/libgpumon_hook.so $(HOOK_OBJS)

# clean out the dross
.PHONY: clean_hook
clean_hook:
	$(Q)rm -f $(LIBDIR)/libgpumon_hook.so *~ $(HOOK_OBJS)
	$(Q)rm -f $(OBJDIR)/*.d
