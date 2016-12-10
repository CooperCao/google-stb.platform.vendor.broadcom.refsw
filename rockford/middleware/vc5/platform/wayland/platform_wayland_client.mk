# make the Nexus platform layer for Wayland client
#

ifneq ($(NXCLIENT_SUPPORT),y)
ifneq ($(NEXUS_CLIENT_SUPPORT),y)
$(error Wayland platform requires multi-process mode!)
endif
endif

LIBNAME = bcm_wl_client

WAYLAND_TOP ?= $(NEXUS_TOP)/../../wayland/install
WAYLAND_CFLAGS ?= -I$(WAYLAND_TOP)/include
WAYLAND_LIBS ?= -L$(WAYLAND_TOP)/lib -lwayland-client

# auto-generated wayland files
WAYLAND_SCANNER ?= $(WAYLAND_TOP)/wayland-scanner
include wayland_nexus_protocol.mk

CFLAGS += \
	$(WAYLAND_CFLAGS) \
	-I$(WAYLAND_AUTOGEN_DIR) \
	-Iwayland_egl \
	-I../nexus

LDFLAGS += $(WAYLAND_LIBS) $(NEXUS_LDFLAGS) $(NXCLIENT_LDFLAGS)

SOURCES := \
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
	../common/swapchain.c \
	../nexus/display_helpers.c \
	../nexus/display_surface.c \
	$(WAYLAND_CODE) \
	client/default_wl_client.c \
	client/display_interface_wl_client.c \
	client/display_wl_client.c \
	client/surface_interface_wl_client.c

$(SOURCES):  $(WAYLAND_CLIENT_HEADER)

clean: clean-wayland-autogen

include ../nexus/platform_nexus_common.mk
