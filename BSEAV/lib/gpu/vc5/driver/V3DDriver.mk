# make the v3d drivers - non proxy mode
#
BUILD_DYNAMIC ?= 1

NEXUS_TOP ?= $(shell cd ../../../../../nexus; pwd)
MAGNUM_TOP ?= $(shell cd $(NEXUS_TOP)/../magnum; pwd)
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

ifeq ($(V3D_VER_AT_LEAST_4_1_0), 1)
HAS_VULKAN ?= 1
else
HAS_VULKAN ?= 0
endif

# default build mode - release

PROFILING?=0

PYTHON_CMD := python2

ifeq ($(VERBOSE),)
hide := @
endif

ifeq ($(HAS_VULKAN), 1)
$(info === Building V3D OpenGL ES and Vulkan drivers ===)
else
$(info === Building V3D OpenGL ES driver ===)
endif

PYTHON2GTEQ_20703 := $(shell expr `$(PYTHON_CMD) --version 2>&1 | awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$2, $$3, $$4) }'` \>= 20703)
ifneq ("$(PYTHON2GTEQ_20703)", "1")
$(error $(PYTHON_CMD) >= 2.7.3 must be available on build machine)
endif

GPERFGTEQ_30004 := $(shell expr `gperf -v | head -1 | awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$3, $$4, $$5) }'` \>= 30004)
ifneq ("$(GPERFGTEQ_30004)", "1")
$(error gperf >= 3.0.4 must be available on build machine)
endif

FLEXGTEQ_20535 := $(shell expr `flex --version | awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$2, $$3, $$4) }'` \>= 20535)
ifneq ("$(FLEXGTEQ_20535)", "1")
$(error flex >= 2.5.35 must be available on build machine)
endif

BISONGTEQ_20700 := $(shell expr `bison --version | head -1 | awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$4, $$5, $$6) }'` \>= 20700)
ifneq ("$(BISONGTEQ_20700)", "1")
$(error bison >= 2.7.0 must be available on build machine)
endif

ifndef B_REFSW_CROSS_COMPILE
B_REFSW_CROSS_COMPILE = $(B_REFSW_ARCH)-
endif

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

# 32-bit ARM
ifeq ($(findstring arm, ${B_REFSW_ARCH}), arm)
CFLAGS += \
	-march=armv7-a \
	-mfpu=neon
endif

ifeq ($(V3D_DEBUG),y)

	# Show a BIG warning about debug mode
$(info ****************************************************)
$(info *****          D E B U G   B U I L D)
$(info *****)
$(info ***** You are building the V3D driver in debug mode.)
$(info ***** This will have a LARGE impact on performance.)
$(info ***** You must build in release mode for correct)
$(info ***** performance of the V3D driver.)
$(info ****************************************************)

	ifneq ($(PROFILING),0)
		CFLAGS += -O3 -g -DNDEBUG
		LDFLAGS += --export-dynamic
	else
		CFLAGS += -O0 -g -fvisibility=hidden
	endif

	LDFLAGS += -g
	OBJDIR ?= obj_$(NEXUS_PLATFORM)_debug
	LIBDIR ?= lib_$(NEXUS_PLATFORM)_debug

else

	CFLAGS += -O3 -DNDEBUG

	ifeq ($(PROFILING),0)
		CFLAGS += -fvisibility=hidden
		# Strip
		LDFLAGS += -s
	else
		CFLAGS += -g
		LDFLAGS += -g --export-dynamic
	endif

	OBJDIR ?= obj_$(NEXUS_PLATFORM)_release
	LIBDIR ?= lib_$(NEXUS_PLATFORM)_release
endif

V3D_OBJDIR := $(abspath $(OBJDIR)/)
GLSL_INTERMEDIATE_ABS_PATH := $(abspath $(V3D_OBJDIR)/libs/khrn/glsl)
DGLENUM_INTERMEDIATE_ABS_PATH := $(abspath $(V3D_OBJDIR)/libs/util/dglenum)
GLSL_INTERMEDIATE_REL_PATH := $(V3D_OBJDIR)/libs/khrn/glsl
DGLENUM_INTERMEDIATE_REL_PATH := $(V3D_OBJDIR)/libs/util/dglenum

include common.mk

CFLAGS += \
	-fpic -DPIC \
	-std=c99 -fwrapv \
	-I$(MAGNUM_TOP)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) \
	-I. \
	-I./libs/core/vcos/include \
	-I./libs/platform \
	-I./libs/platform/bcg_abstract \
	-I./libs/khrn/glsl \
	-I./libs/khrn/include \
	-I$(GLSL_INTERMEDIATE_ABS_PATH) \
	-I$(DGLENUM_INTERMEDIATE_ABS_PATH) \
	-DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE \
	-DEMBEDDED_SETTOP_BOX=1 \
	-Dkhronos_EXPORTS \
	-D_POSIX_C_SOURCE=200112 \
	-D_GNU_SOURCE \
	-DHAVE_ZLIB \
	-DGFX_DEFAULT_UIF_PAGE_SIZE=4096 \
	-DGFX_DEFAULT_UIF_NUM_BANKS=8 \
	-DGFX_DEFAULT_UIF_XOR_ADDR=16 \
	-DGFX_DEFAULT_DRAM_MAP_MODE=2 \
	-DKHRN_GLES32_DRIVER=0 \
	-DV3D_PLATFORM_SIM=0 \
	-Wno-unused-function \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable

ifeq ($(HAS_VULKAN), 1)
	CFLAGS += \
		-I$(MAGNUM_TOP)/portinginterface/vc5/include \
		-I../platform/common \
		-I./libs/vulkan/include \
		-I./libs/vulkan/driver \
		-I./libs/vulkan/driver/spirv \
		-I./libs/vulkan/driver/platforms \
		-DBUILD_VULKAN_ICD=1 \
		-DVK_USE_PLATFORM_DISPLAY_KHR=1

	CFLAGS += -c $(foreach dir,$(NEXUS_APP_INCLUDE_PATHS),-I$(dir)) $(foreach def,$(NEXUS_APP_DEFINES),-D"$(def)")

	ifeq ($(NXCLIENT_SUPPORT),y)
		include $(NEXUS_TOP)/nxclient/include/nxclient.inc
		CFLAGS += $(NXCLIENT_CFLAGS)
		NXPL_PLATFORM_EXCLUSIVE := n
	else
		ifeq ($(NEXUS_CLIENT_SUPPORT),y)
			LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES)
			NXPL_PLATFORM_EXCLUSIVE := n
		else
			LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)
			CFLAGS += -DSINGLE_PROCESS
			CFLAGS += -DNXPL_PLATFORM_EXCLUSIVE
			NXPL_PLATFORM_EXCLUSIVE := y
		endif
	endif
endif

ifeq ($(BUILD_DYNAMIC),1)
CFLAGS += -shared
endif

CFLAGS += -Wall -Wpointer-arith

# Add any customer specific cflags from the command line
CFLAGS += $(V3D_EXTRA_CFLAGS)

LDFLAGS = -lpthread -ldl -lm

# Bind references to global functions to the definitions within the khronos
# library. This in particular means eglGetProcAddress will always return the
# khronos library function pointers, even if a GL wrapper library is in
# LD_PRELOAD.
LDFLAGS += -Wl,-Bsymbolic-functions

# Add any customer specific ldflags from the command line
LDFLAGS += $(V3D_EXTRA_LDFLAGS)

ifeq ($(HAS_VULKAN), 1)
VULKAN_TARGETS := $(LIBDIR)/libbcmvulkan_icd.so $(LIBDIR)/bcm.json
endif

ifeq ($(BUILD_DYNAMIC),1)
all: object_dir $(LIBDIR)/libv3ddriver.so $(VULKAN_TARGETS)
else
all: object_dir $(LIBDIR)/libv3ddriver.a $(VULKAN_TARGETS)
endif

.PHONY : object_dir
object_dir:
	$(hide)mkdir -p $(V3D_OBJDIR)

V3D_DRIVER_LIBS_REL_PATH = libs
V3D_DRIVER_LIBS_ABS_PATH = $(CURDIR)/libs
V3D_DRIVER_TOP_ABS_PATH = $(CURDIR)/..

include intermediates.mk

GLES_SRCS = \
	$(GLES_SRC_FILES)   \
	$(COMMON_SRC_FILES) \
	$(addprefix $(GLSL_INTERMEDIATE_ABS_PATH)/, $(COMMON_GENERATED_SRC_FILES)) \
	$(addprefix $(GLSL_INTERMEDIATE_ABS_PATH)/, $(GLES_GENERATED_SRC_FILES))

ifeq ($(HAS_VULKAN), 1)
VULKAN_SRCS = \
	$(VULKAN_SRC_FILES) \
	$(COMMON_SRC_FILES) \
	$(addprefix $(GLSL_INTERMEDIATE_ABS_PATH)/, $(COMMON_GENERATED_SRC_FILES))
endif

ALL_SRCS = \
	$(GLES_SRC_FILES)   \
	$(VULKAN_SRC_FILES) \
	$(COMMON_SRC_FILES) \
	$(addprefix $(GLSL_INTERMEDIATE_ABS_PATH)/, $(COMMON_GENERATED_SRC_FILES)) \
	$(addprefix $(GLSL_INTERMEDIATE_ABS_PATH)/, $(GLES_GENERATED_SRC_FILES))

GLES_OBJS0 := $(patsubst %.cpp,%.o,$(filter %.cpp,$(GLES_SRCS)))
GLES_OBJS0 += $(patsubst %.c,%.o,$(filter %.c,$(GLES_SRCS)))
GLES_OBJS := $(addprefix $(V3D_OBJDIR)/, $(notdir $(GLES_OBJS0)))

ifeq ($(HAS_VULKAN), 1)
VULKAN_OBJS0 := $(patsubst %.cpp,%.o,$(filter %.cpp,$(VULKAN_SRCS)))
VULKAN_OBJS0 += $(patsubst %.c,%.o,$(filter %.c,$(VULKAN_SRCS)))
VULKAN_OBJS := $(addprefix $(V3D_OBJDIR)/, $(notdir $(VULKAN_OBJS0)))
endif

define CCompileRule
$(2) : $(1) \
	$(AUTO_FILES)
		$(hide)echo Compiling $(notdir $(1))
		$(hide)$(3) -c -MMD -MP -MF"$(2:%.o=%.d)" -MT"$(2:%.o=%.d)" -o "$(2)" "$(1)"
endef

CXXFLAGS := $(filter-out -std=c99,$(CFLAGS)) -std=c++11

# Note: The GL driver used to have -fno-rtti & -fno-exceptions for its C++ code
# Supporting this separately would increase the Makefile complexity further, so I chose to leave these out.

$(foreach src,$(filter %.c,$(ALL_SRCS)),$(eval $(call CCompileRule,$(src),$(V3D_OBJDIR)/$(basename $(notdir $(src))).o,$(CC) $(CFLAGS))))
$(foreach src,$(filter %.cpp,$(ALL_SRCS)),$(eval $(call CCompileRule,$(src),$(V3D_OBJDIR)/$(basename $(notdir $(src))).o,$(C++) $(CXXFLAGS))))

ifeq ($(HAS_VULKAN), 1)
$(LIBDIR)/bcm.json: ./libs/vulkan/driver/bcm.json
	$(hide)mkdir -p $(LIBDIR)
	$(hide)sed 's,$${ICD_FILEPATH},$(abspath $(LIBDIR)/libbcmvulkan_icd.so),g' ./libs/vulkan/driver/bcm.json > $(LIBDIR)/bcm.json

$(LIBDIR)/libbcmvulkan_icd.so: $(VULKAN_OBJS)
	$(hide)echo Linking ... libbcmvulkan_icd.so
	$(hide)mkdir -p $(LIBDIR)
	$(hide)$(C++) $(LDFLAGS) -shared -o $(LIBDIR)/libbcmvulkan_icd.so $(VULKAN_OBJS)
endif

$(LIBDIR)/libv3ddriver.so: $(GLES_OBJS)
	$(hide)echo Linking ... libv3ddriver.so
	$(hide)mkdir -p $(LIBDIR)
	$(hide)$(C++) $(LDFLAGS) -static-libstdc++ -shared -o $(LIBDIR)/libv3ddriver.so $(GLES_OBJS)

$(LIBDIR)/libv3ddriver.a: $(GLES_OBJS)
	$(hide)echo Archiving ... libv3ddriver.a
	$(hide)mkdir -p $(LIBDIR)
	$(hide)ar -rcs $(LIBDIR)/libv3ddriver.a $(GLES_OBJS)

# clean out the dross
.phony: clean
clean:
	$(hide)rm -f $(AUTO_FILES)
	$(hide)rm -f $(LIBDIR)/libv3ddriver.so *~ $(GLES_OBJS)
	$(hide)rm -f $(V3D_OBJDIR)/*.d
	$(hide)rm -f $(LIBDIR)/libv3ddriver.a
	$(hide)rm -f $(VULKAN_TARGETS) $(VULKAN_OBJS)
