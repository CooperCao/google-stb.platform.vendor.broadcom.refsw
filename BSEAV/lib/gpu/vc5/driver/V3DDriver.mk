# make the v3d - non proxy mode
#
BUILD_DYNAMIC ?= 1

NEXUS_TOP ?= $(shell cd ../../../../../nexus; pwd)
MAGNUM_TOP ?= $(shell cd $(NEXUS_TOP)/../magnum; pwd)
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

include common.mk

# default build mode - release

PROFILING?=0

ifeq ($(VERBOSE),)
Q := @
endif

$(info === Building V3D driver ===)

PYTHON2GTEQ_20703 := $(shell expr `python2 --version 2>&1 | awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$2, $$3, $$4) }'` \>= 20703)
ifneq ("$(PYTHON2GTEQ_20703)", "1")
$(error python2 >= 2.7.3 must be available on build machine)
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

GLSL_INTERMEDIATE = $(abspath $(OBJDIR)/libs/khrn/glsl)
DGLENUM_INTERMEDIATE = $(abspath $(OBJDIR)/libs/util/dglenum)

CFLAGS += \
	-fpic -DPIC \
	-std=c99 \
	-I$(MAGNUM_TOP)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) \
	-I. \
	-I./libs/core/vcos/include \
	-I./libs/platform/bcg_abstract \
	-I./libs/khrn/glsl \
	-I./libs/khrn/include \
	-I$(GLSL_INTERMEDIATE) \
	-I$(DGLENUM_INTERMEDIATE) \
	-Dkhronos_EXPORTS \
	-D_POSIX_C_SOURCE=200112 \
	-D_GNU_SOURCE \
	-DHAVE_ZLIB \
	-DGFX_DEFAULT_UIF_PAGE_SIZE=4096 \
	-DGFX_DEFAULT_UIF_NUM_BANKS=8 \
	-DGFX_DEFAULT_UIF_XOR_ADDR=16 \
	-DGFX_DEFAULT_DRAM_MAP_MODE=2 \
	-DEMBEDDED_SETTOP_BOX=1 \
	-DKHRN_GLES31_DRIVER=$(V3D_VER_AT_LEAST_3_3_0) \
	-DKHRN_GLES32_DRIVER=0 \
	-DGLSL_310_SUPPORT=1 \
	-DV3D_PLATFORM_SIM=0 \
	-DSECURE_SUPPORT=1 \
	-Wno-unused-function \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable

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

ifeq ($(BUILD_DYNAMIC),1)
all: $(LIBDIR)/libv3ddriver.so
else
all: $(LIBDIR)/libv3ddriver.a
endif

define generated_src_dir_exists
$(Q)mkdir -p $(GLSL_INTERMEDIATE)
$(Q)mkdir -p $(DGLENUM_INTERMEDIATE)
endef

glsl_primitive_types_deps := \
	libs/khrn/glsl/scripts/build_primitive_types.py \
	libs/khrn/glsl/scripts/scalar_types.table \
	libs/khrn/glsl/scripts/sampler_types.table \
	libs/khrn/glsl/scripts/image_types.table

define glsl_primitive_types_gen
$(generated_src_dir_exists)
$(Q)cd libs/khrn/glsl && \
	python2 \
		scripts/build_primitive_types.py \
		-I $(CURDIR)/libs/khrn/glsl \
		-O $(GLSL_INTERMEDIATE) \
		scripts/scalar_types.table \
		scripts/sampler_types.table \
		scripts/image_types.table;
endef

$(GLSL_INTERMEDIATE)/glsl_primitive_types.auto.table : \
	$(glsl_primitive_types_deps)
		$(glsl_primitive_types_gen)

$(GLSL_INTERMEDIATE)/glsl_primitive_type_index.auto.h \
$(GLSL_INTERMEDIATE)/glsl_primitive_types.auto.h \
$(GLSL_INTERMEDIATE)/glsl_primitive_types.auto.c : \
	$(GLSL_INTERMEDIATE)/glsl_primitive_types.auto.table
		@

$(GLSL_INTERMEDIATE)/glsl_intrinsic_lookup.auto.c : \
	libs/khrn/glsl/glsl_intrinsic_lookup.gperf \
	$(GLSL_INTERMEDIATE)/glsl_primitive_type_index.auto.h
		$(generated_src_dir_exists)
		$(Q)gperf libs/khrn/glsl/glsl_intrinsic_lookup.gperf > $(GLSL_INTERMEDIATE)/glsl_intrinsic_lookup.auto.c

$(GLSL_INTERMEDIATE)/glsl_layout.auto.c : libs/khrn/glsl/glsl_layout.gperf
	$(generated_src_dir_exists)
	$(Q)gperf libs/khrn/glsl/glsl_layout.gperf > $(GLSL_INTERMEDIATE)/glsl_layout.auto.c

$(GLSL_INTERMEDIATE)/textures.auto.props : \
	libs/khrn/glsl/scripts/build_texture_functions.py
		$(generated_src_dir_exists)
		$(Q)cd libs/khrn/glsl && \
		python2 \
			scripts/build_texture_functions.py \
			-O $(GLSL_INTERMEDIATE);

$(GLSL_INTERMEDIATE)/textures.auto.glsl : \
	$(GLSL_INTERMEDIATE)/textures.auto.props
		@

# sources for stdlib from source tree.
STDLIB_SOURCES := \
	stdlib/atomics.glsl \
	stdlib/common.glsl \
	stdlib/derivatives.glsl \
	stdlib/exponential.glsl \
	stdlib/extensions.glsl \
	stdlib/geometry.glsl \
	stdlib/geom_shade.glsl \
	stdlib/hyperbolic.glsl \
	stdlib/image.glsl \
	stdlib/integer.glsl \
	stdlib/matrix.glsl \
	stdlib/packing.glsl \
	stdlib/synchronisation.glsl \
	stdlib/texture_gather.glsl \
	stdlib/trigonometry.glsl \
	stdlib/vector_relational.glsl \
	stdlib/common.inl \
	stdlib/derivatives.inl \
	stdlib/exponential.inl \
	stdlib/geometry.inl \
	stdlib/hyperbolic.inl \
	stdlib/integer.inl \
	stdlib/matrix.inl \
	stdlib/packing.inl \
	stdlib/trigonometry.inl \
	stdlib/vector_relational.inl \
	stdlib/texture.glsl \
	stdlib/globals.glsl \
	stdlib/stages.props \
	stdlib/v100_only.props \
	stdlib/v300_only.props \
	stdlib/v310_only.props \
	stdlib/v320_only.props \
	stdlib/extensions.props \
	stdlib/restrictions.props

STDLIB_AUTO_SOURCES := \
	$(GLSL_INTERMEDIATE)/glsl_primitive_types.auto.table \
	$(GLSL_INTERMEDIATE)/textures.auto.glsl \
	$(GLSL_INTERMEDIATE)/textures.auto.props

define glsl_stdlib_auto_gen
$(generated_src_dir_exists)
$(Q)cd libs/khrn/glsl && \
	python2 \
		$(CURDIR)/libs/khrn/glsl/scripts/build_stdlib.py \
		-I $(CURDIR)/libs/khrn/glsl \
		-O $(GLSL_INTERMEDIATE) \
		$(STDLIB_SOURCES) \
		$(STDLIB_AUTO_SOURCES)
endef

$(GLSL_INTERMEDIATE)/glsl_stdlib.auto.c : \
	libs/khrn/glsl/scripts/build_stdlib.py \
	$(addprefix libs/khrn/glsl/,$(STDLIB_SOURCES)) $(STDLIB_AUTO_SOURCES)
		$(glsl_stdlib_auto_gen)

$(GLSL_INTERMEDIATE)/glsl_stdlib.auto.h : \
$(GLSL_INTERMEDIATE)/glsl_stdlib.auto.c
	@


$(GLSL_INTERMEDIATE)/glsl_parser.output : \
	libs/khrn/glsl/glsl_parser.y
		$(generated_src_dir_exists)
		$(Q)bison -d -o $(GLSL_INTERMEDIATE)/glsl_parser.c libs/khrn/glsl/glsl_parser.y

$(GLSL_INTERMEDIATE)/glsl_parser.c \
$(GLSL_INTERMEDIATE)/glsl_parser.h : \
$(GLSL_INTERMEDIATE)/glsl_parser.output
	@

$(GLSL_INTERMEDIATE)/glsl_lexer.c : libs/khrn/glsl/glsl_lexer.l
	$(generated_src_dir_exists)
	$(Q)flex -L -o $(GLSL_INTERMEDIATE)/glsl_lexer.c --never-interactive libs/khrn/glsl/glsl_lexer.l

$(GLSL_INTERMEDIATE)/glsl_numbers.c : libs/khrn/glsl/glsl_numbers.l
	$(generated_src_dir_exists)
	$(Q)flex -L -o $(GLSL_INTERMEDIATE)/glsl_numbers.c --never-interactive libs/khrn/glsl/glsl_numbers.l

$(GLSL_INTERMEDIATE)/glsl_version.c : libs/khrn/glsl/glsl_version.l
	$(generated_src_dir_exists)
	$(Q)flex -L -o $(GLSL_INTERMEDIATE)/glsl_version.c --never-interactive libs/khrn/glsl/glsl_version.l

$(DGLENUM_INTERMEDIATE)/dglenum_gen.h : \
		libs/util/dglenum/dglenum_gen.py \
		libs/khrn/include/GLES3/gl3.h \
		libs/khrn/include/GLES3/gl3ext_brcm.h
	$(generated_src_dir_exists)
	$(Q)python2 libs/util/dglenum/dglenum_gen.py > $(DGLENUM_INTERMEDIATE)/dglenum_gen.h

AUTO_FILES = \
	$(GLSL_INTERMEDIATE)/glsl_primitive_types.auto.table \
	$(GLSL_INTERMEDIATE)/glsl_primitive_type_index.auto.h \
	$(GLSL_INTERMEDIATE)/glsl_primitive_types.auto.h \
	$(GLSL_INTERMEDIATE)/glsl_primitive_types.auto.c \
	$(GLSL_INTERMEDIATE)/glsl_intrinsic_lookup.auto.c \
	$(GLSL_INTERMEDIATE)/textures.auto.glsl \
	$(GLSL_INTERMEDIATE)/textures.auto.props \
	$(GLSL_INTERMEDIATE)/glsl_stdlib.auto.c \
	$(GLSL_INTERMEDIATE)/glsl_stdlib.auto.h \
	$(GLSL_INTERMEDIATE)/glsl_parser.c \
	$(GLSL_INTERMEDIATE)/glsl_parser.h \
	$(GLSL_INTERMEDIATE)/glsl_parser.output \
	$(GLSL_INTERMEDIATE)/glsl_lexer.c \
	$(GLSL_INTERMEDIATE)/glsl_numbers.c \
	$(GLSL_INTERMEDIATE)/glsl_version.c \
	$(GLSL_INTERMEDIATE)/glsl_layout.auto.c \
	$(DGLENUM_INTERMEDIATE)/dglenum_gen.h

C_SRC = \
	$(COMMON_SRC_FILES) \
	$(addprefix $(GLSL_INTERMEDIATE)/, $(COMMON_GENERATED_SRC_FILES))


OBJS0 := $(patsubst %.cpp,%.o,$(filter %.cpp,$(C_SRC)))
OBJS0 += $(patsubst %.c,%.o,$(filter %.c,$(C_SRC)))
OBJS := $(addprefix $(OBJDIR)/, $(notdir $(OBJS0)))

define CCompileRule
$(2) : $(1) \
	$(AUTO_FILES)
		$(Q)echo Compiling $(notdir $(1))
		$(Q)$(3) -c -MMD -MP -MF"$(2:%.o=%.d)" -MT"$(2:%.o=%.d)" -o "$(2)" "$(1)"
endef

CXXFLAGS := $(filter-out -std=c99,$(CFLAGS)) -std=c++11 -fno-rtti -fno-exceptions

$(foreach src,$(filter %.c,$(C_SRC)),$(eval $(call CCompileRule,$(src),$(OBJDIR)/$(basename $(notdir $(src))).o,$(CC) $(CFLAGS))))
$(foreach src,$(filter %.cpp,$(C_SRC)),$(eval $(call CCompileRule,$(src),$(OBJDIR)/$(basename $(notdir $(src))).o,$(C++) $(CXXFLAGS))))

$(LIBDIR)/libv3ddriver.so: $(OBJS)
	$(Q)echo Linking ... libv3ddriver.so
	$(Q)mkdir -p $(LIBDIR)
	$(Q)$(C++) $(LDFLAGS) -static-libstdc++ -shared -o $(LIBDIR)/libv3ddriver.so $(OBJS)

$(LIBDIR)/libv3ddriver.a: $(OBJS)
	$(Q)echo Archiving ... libv3ddriver.a
	$(Q)mkdir -p $(LIBDIR)
	$(Q)ar -rcs $(LIBDIR)/libv3ddriver.a $(OBJS)

# clean out the dross
.phony: clean
clean:
	$(Q)rm -f $(AUTO_FILES)
	$(Q)rm -f $(LIBDIR)/libv3ddriver.so *~ $(OBJS)
	$(Q)rm -f $(OBJDIR)/*.d
	$(Q)rm -f $(LIBDIR)/libv3ddriver.a
