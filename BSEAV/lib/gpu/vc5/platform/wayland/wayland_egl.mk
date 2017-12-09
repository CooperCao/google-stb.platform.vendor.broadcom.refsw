# make the wayland-egl client library

LIBNAME = wayland-egl

include wayland_paths.mk

CFLAGS += \
	$(WAYLAND_CFLAGS) \
	-DWL_HIDE_DEPRECATED

LDFLAGS += $(WAYLAND_LIBS)

SOURCES :=	\
	wayland_egl/wayland_egl.c

include ../nexus/platform_nexus_common.mk
