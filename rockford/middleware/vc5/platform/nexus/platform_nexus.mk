# make the Nexus platform layer
#
LIBNAME = nxpl

SOURCES =	default_nexus.c \
			display_nexus.c \
			display_helpers.c \
			display_surface.c \
			display_thread.c \
			../common/memory_nexus.c \
			../common/sched_nexus.c \
			../common/event.c \
			../common/fence.c \
			../common/message_queue.c

include platform_nexus_common.mk
