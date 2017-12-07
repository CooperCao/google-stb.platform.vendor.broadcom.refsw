# make the Nexus platform layer
#
LIBNAME = nxpl

SOURCES =	default_nexus.c \
			display_nexus.c \
			display_helpers.c \
			display_surface.c \
			../common/debug_helper.cpp \
			../common/memory_nexus.c \
			../common/memory_drm.c \
			../common/sched_nexus.c \
			../common/display_framework.c \
			../common/display_interface.c \
			../common/event.c \
			../common/fence_interface.c \
			../common/fence_queue.c \
			../common/queue.c \
			../common/ring_buffer.c \
			../common/surface_interface.c \
			../common/surface_interface_nexus.c \
			../common/swapchain.c

ifneq ($(NXCLIENT_SUPPORT),y)
ifneq ($(NEXUS_CLIENT_SUPPORT),y)
	NXPL_PLATFORM_EXCLUSIVE := y
endif
endif

ifeq ($(NXPL_PLATFORM_EXCLUSIVE),y)
SOURCES += display_nexus_exclusive.c
else
SOURCES += display_nexus_multi.c
endif

include platform_nexus_common.mk
