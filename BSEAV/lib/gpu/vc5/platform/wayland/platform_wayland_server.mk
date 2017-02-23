# make the Nexus platform layer
#

ifneq ($(NXCLIENT_SUPPORT),y)
ifneq ($(NEXUS_CLIENT_SUPPORT),y)
$(error Wayland platform requires multi-process mode!)
endif
endif

LIBNAME = bcm_wl_server

WAYLAND_TOP ?= $(NEXUS_TOP)/../../wayland/install
WAYLAND_CFLAGS ?= -I$(WAYLAND_TOP)/include
WAYLAND_LIBS ?= -L$(WAYLAND_TOP)/lib -lwayland-server

# auto-generated wayland files
WAYLAND_SCANNER ?= $(WAYLAND_TOP)/wayland-scanner
include wayland_nexus_protocol.mk

include $(NEXUS_TOP)/nxclient/include/nxclient.inc

CFLAGS += \
	$(WAYLAND_CFLAGS) \
	-I$(WAYLAND_AUTOGEN_DIR) \
   -I$(NEXUS_TOP)/nxclient/server \
	-I../nexus

LDFLAGS += $(WAYLAND_LIBS) $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)

SOURCES :=	\
	../common/display_framework.c \
	../common/display_interface.c \
	../common/event.c \
	../common/fence.c \
	../common/fence_interface.c \
	../common/fence_interface_nexus.c \
	../common/fence_queue.c \
	../common/memory_nexus.c \
	../common/queue.c \
	../common/ring_buffer.c \
	../common/sched_nexus.c \
	../common/surface_interface.c \
	../common/surface_interface_nexus.c \
	../common/swapchain.c \
	../nexus/display_nexus_multi.c \
	../nexus/display_helpers.c \
	../nexus/display_nexus.c \
	../nexus/display_surface.c \
	$(WAYLAND_CODE) \
	server/bind_wl_display.c \
	server/default_wl_server.c \
	server/display_wl_server.c

$(SOURCES):  $(WAYLAND_SERVER_HEADER)

clean: clean-wayland-autogen

include ../nexus/platform_nexus_common.mk
