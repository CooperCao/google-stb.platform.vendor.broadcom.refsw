# Set paths to Wayland includes, libs and wayland-scanner tool

ifneq ($(WAYLAND_TOP),)
   WAYLAND_CFLAGS ?= -I$(WAYLAND_TOP)/include
   WAYLAND_LIBS ?= -L$(WAYLAND_TOP)/lib -lwayland-client
   WAYLAND_SCANNER ?= $(WAYLAND_TOP)/wayland-scanner
else
   ifeq ($(WAYLAND_CFLAGS),)
      $(error WAYLAND_TOP or WAYLAND_CFLAGS must be set.)
   endif
   ifeq ($(WAYLAND_LIBS),)
      $(error WAYLAND_TOP or WAYLAND_LIBS must be set.)
   endif
   ifeq ($(WAYLAND_SCANNER),)
      $(error WAYLAND_TOP or WAYLAND_SCANNER must be set.)
   endif
endif
