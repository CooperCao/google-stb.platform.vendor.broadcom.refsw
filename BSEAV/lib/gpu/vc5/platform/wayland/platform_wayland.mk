# make the Wayland platform layer

ifneq ($(NXCLIENT_SUPPORT),y)
ifneq ($(NEXUS_CLIENT_SUPPORT),y)
$(error Wayland platform requires multi-process mode!)
endif
endif

LIBNAME = wlpl

include wayland_paths.mk

# auto-generated Wayland protocol files
include wayland_nexus_protocol.mk

include $(NEXUS_TOP)/nxclient/include/nxclient.inc

CFLAGS += \
	$(WAYLAND_CFLAGS) \
	-I$(WAYLAND_AUTOGEN_DIR) \
	-Iwayland_egl \
	-I$(NEXUS_TOP)/nxclient/server \
	-I../nexus \
	-DWL_HIDE_DEPRECATED

LDFLAGS += $(WAYLAND_LIBS) $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)

SOURCES :=	\
	../common/perf_event.cpp \
	../common/display_framework.c \
	../common/display_helpers.c \
	../common/display_interface.c \
	../common/event.c \
	../common/fence_interface.c \
	../common/fence_queue.c \
	../common/memory_convert.c \
	../common/memory_drm.c \
	../common/memory_nexus.c \
	../common/queue.c \
	../common/ring_buffer.c \
	../common/sched_nexus.c \
	../common/surface_interface.c \
	../common/surface_interface_nexus.c \
	../common/swapchain.c \
	../nexus/display_nexus_multi.c \
	../nexus/display_nexus.c \
	../nexus/display_surface.c \
	$(WAYLAND_CODE) \
	default_wayland.c \
	wl_client.c \
	wl_server.c \
	display_nx/display_nx.c \
	display_wl/display_interface_wl.c \
	display_wl/surface_interface_wl.c \
	display_wl/display_wl.c

$(SOURCES):  $(WAYLAND_SERVER_HEADER)

clean: clean-wayland-autogen

include ../nexus/platform_nexus_common.mk
