# make the Nexus platform layer for Wayland client
#

LIBNAME = bcm_wayland_egl

WAYLAND_TOP ?= $(NEXUS_TOP)/../../wayland/install
WAYLAND_CFLAGS ?= -I$(WAYLAND_TOP)/include
WAYLAND_LIBS ?= -L$(WAYLAND_TOP)/lib -lwayland-client

CFLAGS += \
	$(WAYLAND_CFLAGS)

LDFLAGS += $(WAYLAND_LIBS)

SOURCES :=	\
	wayland_egl/wayland_egl.c

include ../nexus/platform_nexus_common.mk
