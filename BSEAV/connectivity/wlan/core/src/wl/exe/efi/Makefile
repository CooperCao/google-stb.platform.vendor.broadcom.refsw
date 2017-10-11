# <<Broadcom-WL-IPTag/Open:>>
# $Id: Makefile.ux 630189 2016-04-07 19:31:38Z $

define usage
This Makefile handles Unix-hosted builds of the EFI-DHD driver using native cc.
Variants: "make uefidbg uefinodbg"

EXAMPLE: make clean all
EXAMPLE: make -j24          (build all in parallel on a 12-core system)
EXAMPLE: make V=1           (show compile lines)
EXAMPLE: make -j32 uefidbg
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

# Allow "make relink" to rebuild just wl.dll and downstream.
.PHONY: relink
relink: all
relink: MAKEFLAGS += --eval .PHONY:wl.dll

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
    -DEFI \
    -DEFIX64 \
	-D_POSIX_C_SOURCE \
    -DSROM12 \
    -DOLYMPIC_API \
    -DBWL_NO_INTERNAL_STDLIB_SUPPORT \
    -D_HAVE_SYSTYPES



#ifdef WLCNT
DFLAGS += -DWLCNT
#endif

# Use newer 11ac ratespec for wl command line
DFLAGS += -DD11AC_IOTYPES

# Use ppr opaque stucture for wl command line
DFLAGS += -DPPR_API

##ifdef WL_NAN
#DFLAGS += -DWL_NAN
##endif


#ifdef WLEXTLOG
DFLAGS += -DWLEXTLOG
#endif


#ifdef WLNDOE
DFLAGS += -DWLNDOE
#endif

#ifdef WLP2PO
DFLAGS += -DWLP2PO
#endif

#ifdef WLANQPO
DFLAGS += -DWLANQPO
#endif

#ifdef WLPFN
DFLAGS += -DWLPFN
#endif

#ifdef WLP2P
DFLAGS += -DWLP2P
#endif

#ifdef WLTDLS
DFLAGS += -DWLTDLS
#endif

#ifdef TRAFFIC_MGMT
DFLAGS += -DTRAFFIC_MGMT
#endif

#ifdef WL_PROXDETECT
DFLAGS += -DWL_PROXDETECT
#endif

#DFLAGS += -DEFI_SPECIFICATION_VERSION=0x00020000
DFLAGS += -DTIANO_RELEASE_VERSION=0x00080006
DFLAGS += -DEDK_RELEASE_VERSION=0x00020000

EDK_IPATHS := \
    $(EDK2)/Vendor/Broadcom \
	$(EDK2)/Vendor/Broadcom/Include/Apple \
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
    src/common/include \
    src/common/include/devctrl_if \
    src/include \
    src/shared \
    src/shared/bcmwifi/include \
    src/wl/exe \
	src/wl/exe/efi/AutoGen \
    src/wl/phy \
    src/wl/ppr/include \
    src/wl/sys)

IFLAGS := -I. $(addprefix -I,$(wildcard $(IPATHS) $(EDK_IPATHS)))

SRCS := \
    src/wl/exe/wlu_efi.c \
    src/wl/exe/wlu.c \
    src/wl/exe/wluc_awdl.c \
    src/wl/exe/wluc_phy.c \
    src/wl/exe/wluc_wnm.c \
    src/wl/exe/wluc_cac.c \
    src/wl/exe/wluc_relmcast.c \
    src/wl/exe/wluc_rrm.c \
    src/wl/exe/wluc_wowl.c \
    src/wl/exe/wluc_pkt_filter.c \
    src/wl/exe/wluc_mfp.c \
    src/wl/exe/wluc_ota_test.c \
    src/wl/exe/wluc_bssload.c \
    src/wl/exe/wluc_stf.c \
    src/wl/exe/wluc_offloads.c \
    src/wl/exe/wluc_tpc.c \
    src/wl/exe/wluc_toe.c \
    src/wl/exe/wluc_arpoe.c \
    src/wl/exe/wluc_keep_alive.c \
    src/wl/exe/wluc_ap.c \
    src/wl/exe/wluc_ampdu.c \
    src/wl/exe/wluc_ampdu_cmn.c \
    src/wl/exe/wluc_bmac.c \
    src/wl/exe/wluc_he.c \
    src/wl/exe/wluc_ht.c \
    src/wl/exe/wluc_wds.c \
    src/wl/exe/wluc_keymgmt.c \
    src/wl/exe/wluc_scan.c \
    src/wl/exe/wluc_obss.c \
    src/wl/exe/wluc_prot_obss.c \
    src/wl/exe/wluc_lq.c \
    src/wl/exe/wluc_seq_cmds.c \
    src/wl/exe/wluc_btcx.c \
    src/wl/exe/wluc_led.c \
    src/wl/exe/wluc_interfere.c \
    src/wl/exe/wluc_ltecx.c \
    src/wl/exe/wluc_extlog.c \
    src/wl/exe/wluc_sdio.c \
    src/wl/exe/wluc_ndoe.c \
    src/wl/exe/wluc_p2po.c \
    src/wl/exe/wluc_anqpo.c \
    src/wl/exe/wluc_pfn.c \
    src/wl/exe/wluc_p2p.c \
    src/wl/exe/wluc_tdls.c \
    src/wl/exe/wluc_traffic_mgmt.c \
    src/wl/exe/wluc_proxd.c \
    src/wl/exe/wlu_rates_matrix.c \
    src/wl/ppr/src/wlc_ppr.c \
    src/wl/exe/wlu_common.c \
    src/shared/bcmwifi/src/bcmwifi_channels.c \
    src/shared/bcmutils.c \
    src/wl/exe/wlu_cmd.c \
    src/wl/exe/wlu_iov.c \
    src/shared/miniopt.c \
    src/shared/bcmxtlv.c \
    src/shared/bcm_app_utils.c \
    src/wl/exe/wluc_bdo.c \
    src/wl/exe/efi/AutoGen/AutoGen.c \
    src/wl/exe/wlu_subcounters.c

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

wl.lib: $(OBJS)
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

wl.dll: wl.lib
	$(show_ld)$(strip $(LD) -o $@ $(DLINK_FLAGS) -start-group $(STATIC_LIBRARIES) $^ --end-group $(DLINK2_FLAGS))
	$(strip $(OBJCOPY) $@)

wl.efi: wl.dll
	$(CP) $< $@.debug
	$(OBJCOPY) --strip-unneeded -R .eh_frame $<
	$(OBJCOPY) --add-gnu-debuglink=$@.debug $<
	GenFw -e UEFI_APPLICATION -o $@ $<
	$(RM) $(CURDIR)/wl.txt
	@echo "Produced $(RWD)/$@"

all: wl.efi

endif  # End of pass 2
