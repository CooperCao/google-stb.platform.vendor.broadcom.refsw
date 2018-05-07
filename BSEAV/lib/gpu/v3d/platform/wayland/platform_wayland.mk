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
	-I./multi \
	$(WAYLAND_CFLAGS) \
	-I$(WAYLAND_AUTOGEN_DIR) \
	-DWL_HIDE_DEPRECATED

LDFLAGS += $(WAYLAND_LIBS) $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)

SOURCES := \
			default_wayland.c \
			wl_client.c \
			wl_server.c \
			display_nx/display_nx.cpp \
			display_nx/nxbitmap.cpp \
			display_nx/nxwindowinfo.cpp \
			display_nx/nxworker.cpp \
			display_wl/display_wl.cpp \
			display_wl/wlwindow.cpp \
			../common/memory_nexus.c \
			../common/packet_rgba.c \
			../common/packet_yv12.c \
			../common/hardware_nexus.cpp \
			../common/nexus_surface_memory.c \
			../common/perf_event.cpp \
			../common/autoclif.c \
			$(WAYLAND_CODE)

$(SOURCES):  $(WAYLAND_SERVER_HEADER)

clean: clean-wayland-autogen

include platform_wayland_common.mk
