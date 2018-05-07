# make the DirectFB platform layer
#

ifeq ($(VERBOSE),)
Q := @
endif

ifndef B_REFSW_ARCH
B_REFSW_ARCH = mipsel-linux
endif

NEXUS_TOP ?= $(shell cd ../../../../../../nexus; pwd)
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

V3D_DIR ?= $(shell cd ../../driver; pwd)

APPLIBS_TOP:=$(NEXUS_TOP)/../AppLibs
include $(APPLIBS_TOP)/opensource/directfb/build/directfb_common.inc

NXPL_PLATFORM_EXCLUSIVE := n
ifeq ($(NEXUS_MODE),)
NXPL_PLATFORM_EXCLUSIVE := y
endif

ifeq ($(NEXUS_MODE),proxy)
ifneq ($(CLIENT),y)
NXPL_PLATFORM_EXCLUSIVE := y
endif
endif

CFLAGS += \
	-fpic -DPIC \
	-std=c99 \
	-I. \
	-I../common \
	-I$(V3D_DIR)/interface/khronos/include \
	-I$(V3D_DIR)/libs/platform \
	-I$(V3D_DIR)/libs/platform/bcg_abstract \
	-I$(V3D_DIR)/libs/khrn/egl/platform/bcg_abstract \
	-I$(V3D_DIR) \
	-I$(V3D_DIR)/libs/core/vcos \
	-I$(V3D_DIR)/libs/core/vcos/include \
	-I$(V3D_DIR)/libs/core/vcos/pthreads \
	-I$(V3D_NEXUS_TOP)/include \
	-I$(V3D_PLATFORM_DIR)/nexus \
	-I${DIRECTFB_INSTALL_DIRECTFB_INCLUDE_DIR} \
	-I../nexus \
	-I../magnum/portinginterface/vc5/include \
	-I../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) \
	-DBCHP_CHIP=$(BCHP_CHIP) \
	-D_POSIX_C_SOURCE=200112 \
	-D_GNU_SOURCE \
	-DHAVE_ZLIB \
	-DEMBEDDED_SETTOP_BOX=1 \
	-DGLSL_310_SUPPORT \
	-DDIRECTFB

ifeq ($(NXPL_PLATFORM_EXCLUSIVE),y)
CFLAGS += -DNXPL_PLATFORM_EXCLUSIVE
endif

CFLAGS += -c $(foreach dir,$(NEXUS_APP_INCLUDE_PATHS),-I$(dir)) $(foreach def,$(NEXUS_APP_DEFINES),-D"$(def)")

include ../platform_common.inc
CFLAGS += $(COMMON_PLATFORM_CFLAGS)

LDFLAGS = -lpthread

ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc), ${B_REFSW_ARCH})
CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_BIG
else
CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE
endif

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

ifeq ($(V3D_DEBUG),y)
CFLAGS += -O0 -g
LDFLAGS += -g
OBJDIR = obj_$(NEXUS_PLATFORM)_debug
LIBDIR = lib_$(NEXUS_PLATFORM)_debug
else
CFLAGS += -Os -DNDEBUG
LDFLAGS += -s
OBJDIR = obj_$(NEXUS_PLATFORM)_release
LIBDIR = lib_$(NEXUS_PLATFORM)_release
endif

LDFLAGS += -L$(LIBDIR) -lv3ddriver

SOURCES =   default_directfb.c \
            ../common/memory_nexus.c \
            ../nexus/display_surface.c \
            ../common/sched_nexus.c \
            ../common/fence_interface.c \
            ../common/memory_convert.c \
            ../common/display_helpers.c \
            ../common/perf_event.cpp \
            directfb_helpers.c \
            display_directfb.cpp

CXXFLAGS := $(filter-out -std=c99,$(CFLAGS)) -std=c++0x

.PHONY : all

all: $(LIBDIR)/libdbpl.so

.phony: OUTDIR
OUTDIR :
	$(Q)mkdir -p $(OBJDIR)

# Check to make sure DirectFB is already built
.phony: directfb_check
directfb_check:
	@if [ ! -d $(DIRECTFB_INSTALL_DIRECTFB_INCLUDE_DIR) ]; then \
		echo "ERROR: Cannot find directfb installation - aborting!"; \
		/bin/false; \
	fi

OBJS0 := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SOURCES)))
OBJS0 += $(patsubst %.c,%.o,$(filter %.c,$(SOURCES)))
OBJS := $(addprefix $(OBJDIR)/, $(notdir $(OBJS0)))

# $(1) = src
# $(2) = obj
define CCompileRule
$(2) : $(1)
	$(Q)echo Compiling $(1)
	$(Q)$(3) -c -MMD -MP -MF"$(2:%.o=%.d)" -MT"$(2:%.o=%.d)" -o "$(2)" "$(1)"
endef

$(foreach src,$(filter %.c,$(SOURCES)),$(eval $(call CCompileRule,$(src),$(OBJDIR)/$(basename $(notdir $(src))).o,$(CC) $(CFLAGS))))
$(foreach src,$(filter %.cpp,$(SOURCES)),$(eval $(call CCompileRule,$(src),$(OBJDIR)/$(basename $(notdir $(src))).o,$(C++) $(CXXFLAGS))))

# $(1) = src
# $(2) = d
# $(3) = obj
define DependRule_C
$(2) : $(1) | OUTDIR
	$(Q)echo Making depends for $(1)
	$(Q)touch $(2).tmp
	$(Q)$(CC) -D__SSE__ -D__MMX__ -M -MQ $(3) -MF $(2).tmp -MM $(CFLAGS) $(1)
	$(Q)sed 's/D:/\/\/D/g' < $(2).tmp | sed 's/C:/\/\/C/g' > $(2)
	$(Q)rm -f $(2).tmp

PRE_BUILD_RULES += $(2)
# Don't know why, but a comment on this line is necessary
endef

PRE_BUILD_RULES += directfb_check

ifneq ($(MAKECMDGOALS),clean)
$(foreach src,$(filter %.c,$(SOURCES)),$(eval $(call DependRule_C,$(src),$(OBJDIR)/$(basename $(notdir $(src))).d,\
              $(OBJDIR)/$(basename $(notdir $(src))).o)))

$(foreach src,$(filter %.c,$(SOURCES)),$(eval -include $(OBJDIR)/$(basename $(notdir $(src))).d))
endif

$(LIBDIR)/libdbpl.so: $(PRE_BUILD_RULES) $(OBJS)
	$(Q)echo Linking ... libdbpl.so
	$(Q)mkdir -p $(LIBDIR)
	$(Q)$(CC) $(LDFLAGS) -shared -o $(LIBDIR)/libdbpl.so $(OBJS)

# clean out the dross
clean:
	$(Q)rm -f $(LIBDIR)/libdbpl.so *~ $(OBJS)
	$(Q)rm -f $(OBJDIR)/*.d
	$(Q)$(MAKE) --no-print-directory -C $(V3D_DIR) -f V3DDriver.mk clean
