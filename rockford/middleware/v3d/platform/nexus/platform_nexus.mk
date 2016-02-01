# make the Nexus platform layer
#
BUILD_DYNAMIC ?= 1

ifeq ($(VERBOSE),)
Q := @
endif

NEXUS_TOP ?= ../../../../../nexus
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

V3D_DIR ?= $(NEXUS_TOP)/../rockford/middleware/v3d/driver

CFLAGS += \
	-fpic -DPIC \
	-I. \
	-I$(V3D_DIR)/interface/khronos/include \
	-I$(NEXUS_TOP)/../BSEAV/lib/zlib \
	-I$(NEXUS_TOP)/../BSEAV/lib/libpng \
	-DBCHP_CHIP=$(BCHP_CHIP)

CFLAGS += -c $(foreach dir,$(NEXUS_APP_INCLUDE_PATHS),-I$(dir)) $(foreach def,$(NEXUS_APP_DEFINES),-D"$(def)")

include ../platform_common.inc
CFLAGS += $(COMMON_PLATFORM_CFLAGS)

ifndef B_REFSW_ARCH
B_REFSW_ARCH = mipsel-linux
endif

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

SOURCES =	default_nexus.c \
			../common/memory_nexus.c \
			../common/packet_rgba.c \
			../common/packet_yv12.c \
			../common/hardware_nexus.c

ifeq ($(NXCLIENT_SUPPORT),y)

# Multi-process surface compositor client
SOURCES += surface_comp_display_nexus.c

include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)

else

ifeq ($(NEXUS_CLIENT_SUPPORT),y)

# Multi-process surface compositor client
SOURCES += surface_comp_display_nexus.c

else

CFLAGS += -DSINGLE_PROCESS

ifeq ($(NULL_DISPLAY),y)

# Null display
SOURCES += null_display_nexus.c

CFLAGS += -DNULL_DISPLAY

else

# Single process exclusive/blit display
SOURCES += exclusive_display_nexus.c

endif # NULL_DISPLAY

endif # NEXUS_CLIENT_SUPPORT

endif # NXCLIENT_SUPPORT

.PHONY : all

ifeq ($(NO_V3DDRIVER_BUILD),)
  ifeq ($(BUILD_DYNAMIC),1)
  all: V3DDriver $(LIBDIR)/libnxpl.so
  else
  all: V3DDriver $(LIBDIR)/libnxpl.a
  endif
else
  ifeq ($(BUILD_DYNAMIC),1)
  all: $(LIBDIR)/libnxpl.so
  else
  all: $(LIBDIR)/libnxpl.a
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
define CCompileRule
OBJS += $(2)
$(2) : $(1)
	$(Q)echo Compiling $(1)
	$(Q)$(CC) -c $(CFLAGS) -o "$(2)" "$(1)"

endef

$(foreach src,$(SOURCES),$(eval $(call CCompileRule,$(src),$(OBJDIR)/$(basename $(notdir $(src))).o)))

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

ifneq ($(MAKECMDGOALS),clean)
$(foreach src,$(filter %.c,$(SOURCES)),$(eval $(call DependRule_C,$(src),$(OBJDIR)/$(basename $(notdir $(src))).d,\
              $(OBJDIR)/$(basename $(notdir $(src))).o)))

$(foreach src,$(filter %.c,$(SOURCES)),$(eval -include $(OBJDIR)/$(basename $(notdir $(src))).d))
endif

$(LIBDIR)/libnxpl.so: $(OBJS)
	$(Q)echo Linking ... libnxpl.so
	$(Q)mkdir -p $(LIBDIR)
	$(Q)$(CC) $(LDFLAGS) -shared -o $(LIBDIR)/libnxpl.so $(OBJS)

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
