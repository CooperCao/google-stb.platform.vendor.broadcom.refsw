# make the Nexus platform layer
#
BUILD_DYNAMIC ?= 1

ifeq ($(VERBOSE),)
Q := @
endif

NEXUS_TOP ?= $(shell cd ../../../../../../nexus; pwd)
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

ifndef B_REFSW_ARCH
B_REFSW_ARCH = mipsel-linux
endif

ifndef B_REFSW_CROSS_COMPILE
B_REFSW_CROSS_COMPILE = $(B_REFSW_ARCH)-
endif

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

GCCGTEQ_40800 := $(shell expr `$(CC) -dumpversion | awk 'BEGIN { FS = "." }; { printf("%d%02d%02d", $$1, $$2, $$3) }'` \>= 40800)

V3D_DIR ?= $(shell cd ../../driver; pwd)

CFLAGS += \
	-fpic -DPIC \
	-std=c99 \
	-I. \
	-I$(V3D_DIR)/interface/khronos/include \
	-I$(NEXUS_TOP)/../BSEAV/lib/zlib \
	-I$(NEXUS_TOP)/../BSEAV/lib/libpng \
	-DBCHP_CHIP=$(BCHP_CHIP) \
	-D_XOPEN_SOURCE=600 \
	-D_GNU_SOURCE \
	-Wunused-parameter \
	-Wsign-compare \
	-Wclobbered \
	-Wmissing-braces \
	-Wparentheses

ifeq ("$(GCCGTEQ_40800)", "1")
CFLAGS += \
	-Wmaybe-uninitialized
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

ifeq ($(USE_MMA),1)
CFLAGS += -DUSE_MMA
endif

ifeq ($(KHRN_AUTOCLIF),1)
	CFLAGS += -DKHRN_AUTOCLIF
endif

SOURCES = \
			default_nexus.c \
			bitmap.cpp \
			display_nexus.cpp \
			../common/memory_nexus.c \
			../common/packet_rgba.c \
			../common/packet_yv12.c \
			../common/perf_event.cpp \
			../common/hardware_nexus.cpp \
			../common/nexus_surface_memory.c \
			../common/autoclif.c

ifeq ($(NXCLIENT_SUPPORT),y)
	# Multi-process surface compositor client
	SOURCES += \
		./multi/worker.cpp \
		./multi/windowinfo.cpp

	CFLAGS += \
		-I./multi

	include $(NEXUS_TOP)/nxclient/include/nxclient.inc
	CFLAGS += $(NXCLIENT_CFLAGS)
else
	ifeq ($(NEXUS_CLIENT_SUPPORT),y)
		# Multi-process surface compositor client
		SOURCES += \
			./multi/worker.cpp \
			./multi/windowinfo.cpp

		CFLAGS += \
			-I./multi
	else
		SOURCES += \
			./single/worker.cpp \
			./single/windowstate.cpp \
			./single/windowinfo.cpp

		CFLAGS += \
			-I./single \
			-DSINGLE_PROCESS
	endif # NEXUS_CLIENT_SUPPORT
endif # NXCLIENT_SUPPORT

ifeq ("$(GCCGTEQ_40800)", "1")
CXXFLAGS := $(filter-out -std=c99,$(CFLAGS)) -std=c++11
else
CXXFLAGS := $(filter-out -std=c99,$(CFLAGS)) -std=c++0x
endif
CXXFLAGS += -fno-rtti -fno-exceptions

.PHONY : all

ifeq ($(BUILD_DYNAMIC),1)
all: $(LIBDIR)/libnxpl.so
else
all: $(LIBDIR)/libnxpl.a
endif

.phony: OUTDIR
OUTDIR :
	$(Q)mkdir -p $(OBJDIR)

.phony: V3DDriver
V3DDriver:
	$(Q)$(MAKE) --no-print-directory -C $(V3D_DIR) -f V3DDriver.mk $(MAKECMDGOALS)

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
# $(4) = compiler version
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

$(LIBDIR)/libnxpl.so: $(OBJS)
	$(Q)echo Linking ... libnxpl.so
	$(Q)mkdir -p $(LIBDIR)
	$(Q)$(C++) $(LDFLAGS) -shared -o $(LIBDIR)/libnxpl.so $(OBJS)

$(LIBDIR)/libnxpl.a: $(OBJS)
	$(Q)echo Archiving ... libnxpl.a
	$(Q)mkdir -p $(LIBDIR)
	$(Q)ar -rcs $(LIBDIR)/libnxpl.a $(OBJS)

.PHONY: clean
.PHONY: clean_self

# clean out the dross
clean:
	$(Q)rm -f $(LIBDIR)/libnxpl.so *~ $(OBJS)
	$(Q)rm -f $(LIBDIR)/libnxpl.a
	$(Q)rm -f $(OBJDIR)/*.d
	$(Q)$(MAKE) --no-print-directory -C $(V3D_DIR) -f V3DDriver.mk clean

# clean out the dross
clean_self:
	$(Q)rm -f $(LIBDIR)/libnxpl.so *~ $(OBJS)
	$(Q)rm -f $(LIBDIR)/libnxpl.a
	$(Q)rm -f $(OBJDIR)/*.d
