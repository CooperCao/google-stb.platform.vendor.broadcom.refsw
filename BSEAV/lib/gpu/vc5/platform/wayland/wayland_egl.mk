# make the Nexus platform layer for Wayland client
#

LIBNAME = bcm_wayland_egl

include wayland_paths.mk

CFLAGS += \
	$(WAYLAND_CFLAGS)

LDFLAGS += $(WAYLAND_LIBS)

SOURCES :=	\
	wayland_egl/wayland_egl.c

include ../nexus/platform_nexus_common.mk
