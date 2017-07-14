#
# Broadcom Set Top Box(STB) AP Linux emf module Makefile
#
# Copyright (C) 2017, Broadcom Corporation. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

JOBS := 4

TARGET=emf

# Root name for object dir - could be just a prefix, could be a directory.
ObjPfx := obj-

WLAN_TreeBaseA := $(subst \,/,$(realpath $(dir $(lastword $(MAKEFILE_LIST)))../../../..))

# We've observed a bug, or at least a surprising behavior, in emake which
# causes the $(realpath) above to fail so this fallback is used.
ifndef WLAN_TreeBaseA
WLAN_TreeBaseA := $(shell cd $(dir $(lastword $(MAKEFILE_LIST)))../../../.. && pwd)
endif

# Export these values so they can be used by scripts or nmake/pmake makefiles.
export WLAN_TreeBaseA

# We may eventually remove this and require an
# explict absolute-vs-relative choice.
export WLAN_TreeBase := $(WLAN_TreeBaseA)

# For compatibility, due to the prevalence of $(SRCBASE)
WLAN_SrcBaseA := $(WLAN_TreeBaseA)/src
WLAN_SrcBaseR  = $(patsubst %/,%,%,%,$(dir $(WLAN_TreeBaseR)))
WLAN_ComponentBaseA := $(WLAN_TreeBaseA)/components

BCM_KVERSIONSTRING := $(word 1,$(subst -,$(space),$(LINUXVER)))

export KBUILD_EXTRA_SYMBOLS=$(WLAN_ComponentBaseA)/router/emf/emf/$(ObjPfx)emf-$(LINUXVER)/Module.symvers

# basic options, include current obj dir for wl driver config file
DFLAGS := -DLINUX

DFLAGS += -DSRCBASE=\"$(WLAN_SrcBaseA)\" -DBCMDRIVER

DFLAGS += -D__LINUX_ARM_ARCH__=7 -march=armv7-a

IFLAGS =
IFLAGS += -I$(LINUXDIR)/include

IFLAGS += -I$(LINUXDIR)/include/uapi
IFLAGS += -I$(LINUXDIR)/include/generated/uapi

IFLAGS += -include $(LINUXDIR)/include/linux/kconfig.h

LINUXDIR_INC_BASE = $(LINUXDIR)/arch/$(WLARCH)

IFLAGS += -I$(LINUXDIR_INC_BASE)/include
IFLAGS += -I$(LINUXDIR_INC_BASE)/include/generated

IFLAGS += -I$(LINUXDIR_INC_BASE)/include/uapi
IFLAGS += -I$(LINUXDIR_INC_BASE)/include/generated/uapi

IFLAGS += -I$(LINUXDIR_INC_BASE)/include/asm/mach-brcmstb
IFLAGS += -I$(LINUXDIR_INC_BASE)/include/asm/mach-generic

# Global include/source path
WLAN_StdSrcDirs = $(WLAN_SrcBaseA)/src/shared $(WLAN_SrcBaseA)/src/bcmcrypto
WLAN_StdIncDirs = components/shared

export WLAN_StdIncDirsA	 = $(addprefix $(WLAN_TreeBaseA)/,$(WLAN_StdIncDirs))
export WLAN_StdIncPathA	 = $(addprefix -I,$(WLAN_StdIncDirsA))

IFLAGS   += -I.
IFLAGS   += $(WLAN_ComponentIncPathA)
IFLAGS   += -I$(WLAN_SrcBaseA)/include
IFLAGS   += -I$(WLAN_SrcBaseA)/shared
IFLAGS   += -I$(WLAN_SrcBaseA)/shared/bcmwifi/include
IFLAGS   += -I$(WLAN_SrcBaseA)/include/emf/emf
IFLAGS   += -I$(WLAN_SrcBaseA)/include/emf/igs
IFLAGS   += -I$(WLAN_ComponentBaseA)/router/emf/emf
IFLAGS   += $(WLAN_StdIncPathA)

WFLAGS := -Wall -Wstrict-prototypes -Werror -Wno-unused-local-typedefs -Wno-unused-variable \
	  -Wno-unused-function -Wno-unused-but-set-variable -Wno-implicit-function-declaration \
	  -Wframe-larger-than=2048

LDFLAGS := -r
MODULES := emf.o

KMODULES:=emf.ko

###########################################################
# (TARGET) specified, determine OBJDIR and WLCONFS
###########################################################

# Directory name includes targets
OBJDIR=$(ObjPfx)$(TARGET)-$(LINUXVER)

EMFFILES_SRC = emfc.c emf_linux.c
EMFFILES_SRC += linux_osl.c linux_pkt.c bcmutils.c nvram_ro.c siutils.c nicpci.c pcie_core.c aiutils.c sbutils.c hndpmu.c hndlhl.c bcmotp.c bcmsrom.c
EMFFILES := $(sort $(notdir $(EMFFILES_SRC)))

CFILES := $(sort $(EMFFILES))

CROSS_COMPILE ?= arm-linux-

# CROSS_COMPILE was decided by arch
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
NM := $(CROSS_COMPILE)nm
OBJCOPY := $(CROSS_COMPILE)objcopy
STRIP   := $(CROSS_COMPILE)strip

OFILES := $(CFILES:.c=.o)

# Add our custom CFLAGS
CFLAGS += $(DFLAGS) $(IFLAGS) $(WFLAGS)

CFLAGS += -D__KERNEL__

WLCFLAGS = $(CFLAGS) -I$(shell pwd)
export WLCFLAGS
WLOFILES := $(OFILES)
export WLOFILES

########################################################
# TARGET is one of completed $(TARGETS)
########################################################

TARGETS:
	+$(MAKE) objdir

objdir:
	install -d $(OBJDIR)
	# kernel-mode driver target requested
	# Build dependencies
	+$(MAKE) -r $(if $(JOBS),-j $(JOBS)) -C $(OBJDIR) -f $(WLAN_ComponentBaseA)/router/emf/emf/Makefile \
		INOBJDIR=true dep
	# Build modules
	+$(MAKE) -r $(if $(JOBS),-j $(JOBS)) -C $(OBJDIR) -f $(WLAN_ComponentBaseA)/router/emf/emf/Makefile \
		INOBJDIR=true modules

# Show compiler version, for the current target build
showenv:
	@echo "CC = $(CC) (ver=`$(CC) -dumpversion`; host=`hostname`; processor=`uname -m`)"

modules: $(OFILES)
	test -r ./Makefile || ln -s $(WLAN_ComponentBaseA)/router/emf/emf/makefile.stb ./Makefile
# Electric emake will not be able to tee to a log file,
# so do not update make to be $(MAKE) for kernelbuilds.
	+{ set -o pipefail 2> /dev/null || true; }; \
	$(strip make -r -C $(LINUXDIR) M=$(shell pwd) $(if $(VERBOSE),V=1) \
		$(WLAN_ShellDebugSHELL) \
		KBUILD_EXTRA_SYMBOLS=$(KBUILD_EXTRA_SYMBOLS) \
		modules 2>&1 | tee ,BUILD_LOG)
        # For most module builds (except for usermode and sdio) exit on
        # undefined symbols as driver will not be useable anyway

# Dependencies
dep: showenv
$(foreach file,$(CFILES),.$(file).depend):
ifneq ($(findstring x86_64,$(LINUXVER)),x86_64)
dep: $(foreach file,$(CFILES),.$(file).depend)
endif

.%.c.depend: %.c
	@echo "Making $@"
	@$(CC) $(CFLAGS) -M $< > $@

.%.c.preprocessor: %.c
	$(CC) -E -C $(CFLAGS) $< > $@
#	$(CC) -S $(CFLAGS) $<

vpath %.c $(addprefix $(WLAN_ComponentBaseA)/,emf/emf router/emf/emf)
vpath %.c $(addprefix $(WLAN_SrcBaseA)/,shared)

%.o: %.c
	$(strip \
	$(if $(filter-out $(addprefix %/,$(GENERATED_SOURCES)),$(<F)),test -r $(<F) || ln -s $< .) \
	$(if $(findstring wluser,$(TARGET)), && $(CC) $(CFLAGS) -g -c -o $@ $(<F)))

%.o: %.o_shipped
	test -r ./$(<F) || ln -s $< .

clean:
ifeq (,$(filter %/,$(ObjPfx)))
	$(RM) -r $(ObjPfx)*
else
	$(RM) -r $(ObjPfx)
endif

force:

-include $(wildcard .*.depend)
-include $(wildcard .*.flags)

.PHONY: native all objdir dep modules ext_modules clean force $(TARGETS:%=%-$(LINUXVER))
.PHONY: default $(DEBUGS) $(ARCHES)
