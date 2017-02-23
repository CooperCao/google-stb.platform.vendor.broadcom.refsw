# make the rootx11 platform layer
#
BUILD_DYNAMIC ?= 1

ifeq ($(VERBOSE),)
Q := @
endif

NEXUS_TOP ?= $(shell cd ../../../../../nexus; pwd)
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc

V3D_DIR ?= $(shell cd ../../driver; pwd)

XF86_INC_DIR := =/usr/include/xorg \
		=/usr/include/X11 \
		=/usr/include/pixman-1 \
		=/usr/include/libdrm \
		=/usr/include/xcb

ifneq ($(SYSROOT),)
CFLAGS += --sysroot=${SYSROOT}
endif

CFLAGS += \
	-fpic -DPIC \
	-I. \
	-I$(V3D_DIR)/interface/khronos/include \
	-DBCHP_CHIP=$(BCHP_CHIP)

CFLAGS += -c $(foreach dir,$(NEXUS_APP_INCLUDE_PATHS),-I$(dir)) $(foreach def,$(NEXUS_APP_DEFINES),-D"$(def)")
CFLAGS += $(foreach dir,${XF86_INC_DIR},-I$(dir))

include ../platform_common.inc
CFLAGS += $(COMMON_PLATFORM_CFLAGS)

ifndef B_REFSW_ARCH
B_REFSW_ARCH = mipsel-linux
endif

LDFLAGS += -lpthread -ldri2 -lv3ddriver -lnexus_client

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

ifneq ($(SYSROOT),)
CFLAGS += --sysroot=${SYSROOT}
endif

SOURCES =	default_rootx11.c \
		display_rootx11.c \
		../common/memory_nexus.c \
		../common/packet_rgba.c \
		../common/packet_yv12.c \
		../common/hardware_nexus.c

.PHONY : all

ifeq ($(BUILD_DYNAMIC),1)
all: $(LIBDIR)/librxpl.so
else
all: $(LIBDIR)/librxpl.a
endif

.phony: OUTDIR
OUTDIR :
	$(Q)mkdir -p $(OBJDIR)

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

$(LIBDIR)/librxpl.so: $(OBJS)
	$(Q)$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk
	$(Q)echo Linking ... librxpl.so
	$(Q)mkdir -p $(LIBDIR)
	$(Q)$(CC) $(LDFLAGS) -shared -o $(LIBDIR)/librxpl.so $(OBJS)
	$(Q)cp $(LIBDIR)/librxpl.so $(NEXUS_BIN_DIR)

$(LIBDIR)/librxpl.a: $(OBJS)
	$(Q)echo Archiving ... librxpl.a
	$(Q)mkdir -p $(LIBDIR)
	$(Q)ar -rcs $(LIBDIR)/librxpl.a $(OBJS)

.PHONY: clean
.PHONY: clean_self

# clean out the dross
clean:
	$(Q)rm -f $(LIBDIR)/librxpl.so *~ $(OBJS)
	$(Q)rm -f $(LIBDIR)/librxpl.a
	$(Q)rm -f $(OBJDIR)/*.d
	$(Q)$(MAKE) --no-print-directory -C $(V3D_DIR) -f V3DDriver.mk clean

# clean out the dross
clean_self:
	$(Q)rm -f $(LIBDIR)/librxpl.so *~ $(OBJS)
	$(Q)rm -f $(LIBDIR)/librxpl.a
	$(Q)rm -f $(OBJDIR)/*.d
