# Makefile handles Unix-hosted builds of the dhd utility for efi-edk2 using native cc.
#
#   <<Broadcom-WL-IPTag/Open:>>
#
# $Id: Makefile 621428 2016-02-25 23:42:47Z $

define usage
This Makefile handles Unix-hosted builds of the dhd utility for efi-edk2 using native cc.
Variants: "make uefidbg uefinodbg"
endef

ifneq (,$(filter help,$(MAKECMDGOALS)))
  $(info $(usage))
  .PHONY: help
  help:; @:
endif

# Note that most lists below (source files, -D flags, etc) are kept in
# alphabetical order, though -I paths are not since search order matters.

# Mark "all" as the default target.
.PHONY: all
all:

BLDDIR := X64
THIS_MAKEFILE := $(abspath $(lastword $(MAKEFILE_LIST)))
MWD := $(patsubst %/,%,$(dir $(THIS_MAKEFILE)))
RWD := $(patsubst $(MWD)/%,%,$(CURDIR))

EDK2 := /projects/hnd/tools/EFI/EDK2
EDK2_BCM := $(EDK2)/Vendor/Broadcom

# This makefile proceeds in two passes. The first (recursive) pass simply
# creates a subdirectory structure and bootstraps into it. The second
# (non-recursive) pass creates a full dependency graph and does the real build.
# So the actual build uses a single-graph, non-recursive method.

ifeq ($(CURDIR)/,$(dir $(THIS_MAKEFILE)))

# Pass 1 - set up infrastructure and bootstrap into it.

# In this first pass we suppress parallelism to allow "make -jN clean all" to work.
.NOTPARALLEL:

# Shorthand for "make clean all". This works with -j only because of .NOTPARALLEL.
.PHONY: fresh
fresh: clean all

# Allow "make relink" to rebuild just dhd.dll and downstream.
.PHONY: relink
relink: all
relink: MAKEFLAGS += --eval .PHONY:dhd.dll

all: uefidbg uefinodbg

# This macro encapsulates the logic of making a single variant.
define make_variant
$(eval
$$(BLDDIR)/$1:; mkdir -p $$@
.PHONY: $1
$1: | $$(BLDDIR)/$1
	+$$(strip $$(MAKE) -C $$(BLDDIR)/$1 --no-print-directory -f $$(THIS_MAKEFILE) \
	  VARIANT_DFLAGS="$2" STATIC_LIBRARY_BASE="$$(EDK2_BCM)/$3_GCC44/X64/MdePkg/Library" \
	  STDLIB_BASE="$$(EDK2)/Build/StdLib/$3_GCC44/X64" \
	  SHELLAPP_BASE="$$(EDK2)/Build/AppPkg/$3_GCC44/X64/ShellPkg/Library" \
	  APPDBG_BASE="$$(EDK2)/Build/AppPkg/$3_GCC44/X64/MdePkg/Library" \
	  SHELL_BASE="$$(EDK2)/Build/Shell/$3_GCC44/X64/MdePkg/Library" \
	  NEWLIB_BASE="$$(EDK2)/Build/NewLib/$3_GCC44/X64" \
	  NEWLIB_ENTRY_BASE="$$(EDK2)/Build/NewLibEntryPoint/$3_GCC44/X64" \
	  NEWLIB_TEST_BASE="$$(EDK2)/Build/NewLibTest/$3_GCC44/X64")
)
endef

# Generate rules for each variant.
DEBUG_DFLAGS := -DEFI_DEBUG -DBCMDBG -DBCMDBG_MEM
$(call make_variant,uefidbg,$(DEBUG_DFLAGS),DEBUG)
$(call make_variant,uefinodbg,,RELEASE)

.PHONY: clean
clean:
	$(RM) -r $(BLDDIR)

else  # Pass 2 - do the real build.

# Change the amount of compilation verbosity.
V := 1
show_ar = @$(info Archiving $(@F))
show_ld = @$(info Linking $(@F))
ifeq ($(V),0)
  show_cc = @$(info Compiling $(@F))
else ifeq ($(V),1)
  show_cc = @$(info Compiling $(@F) \
    $(if $(wildcard $@),$(if $(findstring _RECIPE_CHANGED,$?),(RECIPE CHANGED),OOD=$(wildcard $?))))
else
  show_cc =
  show_ar =
  show_ld =
endif

include $(MWD)/../../../makefiles/WLAN_Common.mk

TOOLCHAIN := /usr/bin/
CC := $(TOOLCHAIN)cc
AR := $(TOOLCHAIN)ar
LD := $(TOOLCHAIN)ld
OBJCOPY := $(TOOLCHAIN)objcopy
CP := /bin/cp

export PATH := $(EDK2)/BaseTools/Bin/$(shell uname)-x86_64:$(EDK2)/BaseTools/BinWrappers/PosixLike:${PATH}

# This turns gcc "smart quotes" back to simple dumb ones.
export LC_ALL := C

DFLAGS := \
    -DEFIAPI="__attribute__((ms_abi))" \
    -DBCMWLUNDI \
	-DBCMDONGLEHOST \
    -DEFI \
    -DEFIX64 \
	-D_POSIX_C_SOURCE \
    -DSROM12 \
    -DOLYMPIC_API \
    -DBWL_NO_INTERNAL_STDLIB_SUPPORT


#DFLAGS += -DEFI_SPECIFICATION_VERSION=0x00020000
DFLAGS += -DTIANO_RELEASE_VERSION=0x00080006
DFLAGS += -DEDK_RELEASE_VERSION=0x00020000

EDK_IPATHS := \
    $(EDK2)/Vendor/Broadcom \
    $(EDK2)/MdePkg \
    $(EDK2)/MdePkg/Include \
    $(EDK2)/MdePkg/Include/Guid \
    $(EDK2)/MdePkg/Include/X64 \
    $(EDK2)/ShellPkg/Include \
    $(EDK2)/EmbeddedPkg/Include \
    $(EDK2)/NewLibPkg/Library/NewLib/libc/include

IPATHS := $(addprefix $(WLAN_TreeBaseR)/, \
	components/shared/ \
    components/shared/devctrl_if \
	src/dhd/exe \
    src/include \
    src/shared/bcmwifi/include \
    src/wl/exe \
	src/wl/exe/efi/AutoGen \
    src/wl/ppr/include)

IFLAGS := -I. $(addprefix -I,$(wildcard $(IPATHS) $(EDK_IPATHS)))

SRCS := \
    src/dhd/exe/dhdu.c \
    src/dhd/exe/dhdu_efi.c \
    src/shared/bcmutils.c \
    src/shared/bcmxtlv.c \
    src/shared/bcmwifi/src/bcmwifi_channels.c \
    src/shared/miniopt.c \
    src/shared/bcm_app_utils.c \
    src/dhd/exe/ucode_download.c \
    src/wl/exe/wlu_common.c \
    src/dhd/exe/efi/AutoGen/AutoGen.c

OBJS := $(sort $(patsubst %.c,%.obj,$(notdir $(SRCS))))

CFLAGS := \
    $(sort $(DFLAGS)) \
    $(IFLAGS) \
    -include AutoGen.h \
    -Wall \
    -Werror \
    -Wno-unused-function \
    -Wno-address \
    -Wno-array-bounds \
    -Wno-conversion \
    -Wno-int-to-pointer-cast \
    -fdata-sections \
    -ffunction-sections \
    -fno-asynchronous-unwind-tables \
    -fno-common \
    -fno-stack-protector \
    -fno-strict-aliasing \
    -fshort-wchar \
    -m64 \
    -maccumulate-outgoing-args \
    -mno-red-zone \
    -mcmodel=large \
    -nostdinc \
    -Ulinux

#########################################################################################################
# See this URL for explanation of auto-dependency generation and rebuild-on-recipe-change:
# http://svn.sj.broadcom.com/svn/wlansvn/groups/software/build/make/examples/recipe_changed/autodeps.mk
_space :=
_space +=

.PHONY: _RECIPE_CHANGED
_RECIPE_CHANGED:
.SECONDEXPANSION:
%.obj: _recipe_c2obj = $(strip $(CC) -c -o $@ -MD -MF $@.d -MP $(CFLAGS))
%.obj: %.c $$(if $$(filter $$(RECIPE_FOR_$$@),$$(subst $$(_space),^,$$(_recipe_c2obj))),,_RECIPE_CHANGED)
	$(show_cc)$(_recipe_c2obj) $<
	@echo >> $@.d && echo 'RECIPE_FOR_$@ := $(subst $(_space),^,$(subst $$,$$$$,$(_recipe_c2obj)))' >> $@.d

-include *.d

.DELETE_ON_ERROR:
#########################################################################################################

# Set up vpaths for .c files with relative paths, then absolute paths.
vpath %.c $(addprefix $(WLAN_TreeBaseR)/,$(sort $(patsubst %/,%,$(dir $(filter-out /%,$(SRCS)))))) \
          $(sort $(patsubst %/,%,$(dir $(filter /%,$(SRCS)))))

dhd.lib: $(OBJS)
	$(show_ar)$(strip $(AR) -cr $@ $^)

DLINK_FLAGS = -nostdlib -n -q --gc-sections -z common-page-size=0x20 \
    --entry=_ModuleEntryPoint -u _ModuleEntryPoint \
    -Map $(basename $@).map -melf_x86_64 --oformat=elf64-x86-64

STATIC_LIBRARIES := $(addprefix $(STATIC_LIBRARY_BASE)/, \
	UefiLib/UefiLib/OUTPUT/UefiLib.lib \
    BaseLib/BaseLib/OUTPUT/BaseLib.lib \
	BasePrintLib/BasePrintLib/OUTPUT/BasePrintLib.lib \
	BasePcdLibNull/BasePcdLibNull/OUTPUT/BasePcdLibNull.lib \
    BaseMemoryLib/BaseMemoryLib/OUTPUT/BaseMemoryLib.lib \
    BaseDebugLibNull/BaseDebugLibNull/OUTPUT/BaseDebugLibNull.lib \
	UefiMemoryAllocationLib/UefiMemoryAllocationLib/OUTPUT/UefiMemoryAllocationLib.lib \
	UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib/OUTPUT/UefiRuntimeServicesTableLib.lib \
	UefiBootServicesTableLib/UefiBootServicesTableLib/OUTPUT/UefiBootServicesTableLib.lib)

STATIC_LIBRARIES += $(addprefix $(APPDBG_BASE)/, \
	BaseDebugLibNull/BaseDebugLibNull/OUTPUT/BaseDebugLibNull.lib \
	UefiFileHandleLib/UefiFileHandleLib/OUTPUT/UefiFileHandleLib.lib)

STATIC_LIBRARIES += $(addprefix $(NEWLIB_BASE)/, \
	NewLibPkg/Library/NewLib/NewLib/OUTPUT/NewLib.lib)

STATIC_LIBRARIES += $(addprefix $(NEWLIB_TEST_BASE)/, \
	EmbeddedPkg/Library/EblNetworkLib/EblNetworkLib/OUTPUT/EblNetworkLib.lib \
	EmbeddedPkg/Library/EfiFileLib/EfiFileLib/OUTPUT/EfiFileLib.lib)

STATIC_LIBRARIES += $(addprefix $(NEWLIB_ENTRY_BASE)/, \
	NewLibPkg/Library/NewLib/NewLibEntryPoint/OUTPUT/NewLibEntryPoint.lib)

STATIC_LIBRARIES += $(addprefix $(SHELL_BASE)/, \
	UefiDevicePathLib/UefiDevicePathLibOptionalDevicePathProtocol/OUTPUT/UefiDevicePathLibOptionalDevicePathProtocol.lib)

DLINK2_FLAGS = --defsym=PECOFF_HEADER_SIZE=0x228 \
    --script=$(EDK2)/BaseTools/Scripts/GccBase.lds

dhd.dll: dhd.lib
	$(show_ld)$(strip $(LD) -o $@ $(DLINK_FLAGS) -start-group $(STATIC_LIBRARIES) $^ --end-group $(DLINK2_FLAGS))
	$(strip $(OBJCOPY) $@)

dhd.efi: dhd.dll
	$(CP) $< $@.debug
	$(OBJCOPY) --strip-unneeded -R .eh_frame $<
	$(OBJCOPY) --add-gnu-debuglink=$@.debug $<
	GenFw -e UEFI_APPLICATION -o $@ $<
	$(RM) $(CURDIR)/dhd.txt
	@echo "Produced $(RWD)/$@"

all: dhd.efi

endif  # End of pass 2
