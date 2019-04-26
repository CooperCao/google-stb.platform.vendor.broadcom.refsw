# Common make file for the Nexus platforms
#
# Usage:
# 1. Create platform make file with the following contents:
#      LIBNAME = XYZ
#      SOURCES = ...
#      include platform_nexus_common.mk
# 2. Run make
# This will build libXYZ.so (or libXYZ.a)

BUILD_DYNAMIC ?= 1

ifeq ($(SOURCES),)
$(error Please define SOURCES in the platform make file)
endif

ifeq ($(LIBNAME),)
$(error Please define LIBNAME in the platform make file)
endif

LIB_STATIC = $(LIBDIR)/lib$(LIBNAME).a
LIB_DYNAMIC = $(LIBDIR)/lib$(LIBNAME).so

ifeq ($(VERBOSE),)
Q := @
endif

NEXUS_TOP ?= $(shell cd ../../../../../../nexus; pwd)
MAGNUM_TOP ?= $(shell cd $(NEXUS_TOP)/../magnum; pwd)
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

V3D_DIR ?= $(shell cd ../../driver; pwd)

CFLAGS += \
	-fpic -DPIC \
	-std=c99 \
	-I. \
	-I../common \
	-I$(V3D_DIR)/libs/platform/bcg_abstract \
	-I$(V3D_DIR)/libs/platform \
	-I$(V3D_DIR) \
	-I$(V3D_DIR)/libs/core/vcos/include \
	-I$(V3D_DIR)/libs/core/vcos/pthreads \
	-I$(V3D_DIR)/libs/khrn/include \
	-I$(MAGNUM_TOP)/portinginterface/vc5/include \
	-I$(MAGNUM_TOP)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) \
	-fvisibility=hidden \
	-DBCHP_CHIP=$(BCHP_CHIP) \
	-D_POSIX_C_SOURCE=200112 \
	-D_GNU_SOURCE \
	-DHAVE_ZLIB \
	-DEMBEDDED_SETTOP_BOX=1 \
	-DGLSL_310_SUPPORT \
	-Wno-unused-function \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable

CFLAGS += -c $(foreach dir,$(NEXUS_APP_INCLUDE_PATHS),-I$(dir)) $(foreach def,$(NEXUS_APP_DEFINES),-D"$(def)")

include $(V3D_DIR)/../platform/platform_common.inc
CFLAGS += $(COMMON_PLATFORM_CFLAGS)

CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE

# 32-bit ARM
ifeq ($(findstring arm, ${B_REFSW_ARCH}), arm)
CFLAGS += \
	-march=armv7-a \
	-mfpu=neon
endif

LDFLAGS += -lpthread

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

ifeq ($(V3D_DEBUG),y)
CFLAGS += -O0 -g
LDFLAGS += -g
OBJDIR ?= obj_$(NEXUS_PLATFORM)_debug
LIBDIR ?= lib_$(NEXUS_PLATFORM)_debug
else
CFLAGS += -g -Os -DNDEBUG
LDFLAGS += -g -s
OBJDIR ?= obj_$(NEXUS_PLATFORM)_release
LIBDIR ?= lib_$(NEXUS_PLATFORM)_release
endif

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

CFLAGS += $(V3D_EXTRA_CFLAGS)
LDFLAGS += $(V3D_EXTRA_LDFLAGS)

.DEFAULT_GOAL := all
.PHONY : all

ifeq ($(NO_V3DDRIVER_BUILD),)
  ifeq ($(BUILD_DYNAMIC),1)
  all: V3DDriver $(LIB_DYNAMIC)
  else
  all: V3DDriver $(LIB_STATIC)
  endif
else
  ifeq ($(BUILD_DYNAMIC),1)
  all: $(LIB_DYNAMIC)
  else
  all: $(LIB_STATIC)
  endif
endif

.phony: OUTDIR
OUTDIR :
	$(Q)mkdir -p $(OBJDIR)

.phony: V3DDriver
V3DDriver:
	$(Q)$(MAKE) --no-print-directory -C $(V3D_DIR) -f V3DDriver.mk $(MAKECMDGOALS)

# $(1) = src
# $(2) = obj
# $(3) = cc + flags
define CCompileRule
$(2) : $(1) \
	$(AUTO_FILES)
		$(Q)echo Compiling $(notdir $(1))
		$(Q)$(3) -c -MMD -MP -MF"$(2:%.o=%.d)" -MT"$(2:%.o=%.d)" -o "$(2)" "$(1)"
endef

OBJS0 := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SOURCES)))
OBJS0 += $(patsubst %.c,%.o,$(filter %.c,$(SOURCES)))
OBJS := $(addprefix $(OBJDIR)/, $(notdir $(OBJS0)))

CXXFLAGS := $(filter-out -std=c99,$(CFLAGS)) -std=c++11 -fno-rtti -fno-exceptions

$(foreach src,$(filter %.c,$(SOURCES)),$(eval $(call CCompileRule,$(src),$(OBJDIR)/$(basename $(notdir $(src))).o,$(CC) $(CFLAGS))))
$(foreach src,$(filter %.cpp,$(SOURCES)),$(eval $(call CCompileRule,$(src),$(OBJDIR)/$(basename $(notdir $(src))).o,$(C++) $(CXXFLAGS))))

# $(1) = src
# $(2) = d
# $(3) = obj
# $(4) = cc + flags
define DependRule_C
$(2) : $(1) | OUTDIR
	$(Q)echo Making depends for $(1)
	$(Q)touch $(2).tmp
	$(Q)$(4) -D__SSE__ -D__MMX__ -M -MQ $(3) -MF $(2).tmp -MM $(1)
	$(Q)sed 's/D:/\/\/D/g' < $(2).tmp | sed 's/C:/\/\/C/g' > $(2)
	$(Q)rm -f $(2).tmp

PRE_BUILD_RULES += $(2)
# Don't know why, but a comment on this line is necessary
endef

ifneq ($(MAKECMDGOALS),clean)
$(foreach src,$(filter %.c,$(SOURCES)),$(eval $(call DependRule_C,$(src),$(OBJDIR)/$(basename $(notdir $(src))).d,\
              $(OBJDIR)/$(basename $(notdir $(src))).o,$(CC) $(CFLAGS))))
$(foreach src,$(filter %.cpp,$(SOURCES)),$(eval $(call DependRule_C,$(src),$(OBJDIR)/$(basename $(notdir $(src))).d,\
              $(OBJDIR)/$(basename $(notdir $(src))).o,$(C++) $(CXXFLAGS))))

$(foreach src,$(filter %.c,$(SOURCES)),$(eval -include $(OBJDIR)/$(basename $(notdir $(src))).d))
$(foreach src,$(filter %.cpp,$(SOURCES)),$(eval -include $(OBJDIR)/$(basename $(notdir $(src))).d))
endif

$(LIB_DYNAMIC): $(OBJS)
	$(Q)echo Linking ... $(notdir $@)
	$(Q)mkdir -p $(LIBDIR)
	$(Q)$(C++) $(LDFLAGS) -shared $(OBJS) -o $@

$(LIB_STATIC): $(OBJS)
	$(Q)echo Archiving ... $(notdir $@)
	$(Q)mkdir -p $(LIBDIR)
	$(Q)ar -rcs $@ $(OBJS)

.PHONY: clean
.PHONY: clean_self

# clean out the dross
clean: clean_self
	$(Q)$(MAKE) --no-print-directory -C $(V3D_DIR) -f V3DDriver.mk clean

# clean out the dross
clean_self:
	$(Q)rm -f $(LIB_DYNAMIC) *~ $(OBJS)
	$(Q)rm -f $(LIB_STATIC)
	$(Q)rm -f $(OBJDIR)/*.d
