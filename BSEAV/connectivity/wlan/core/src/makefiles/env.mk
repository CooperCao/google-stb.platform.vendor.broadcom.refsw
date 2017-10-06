empty:=
space:= $(empty) $(empty)
SHELL=bash.exe
WLAN_WINPFX ?= Z:

ifdef REQUIRE_WDM 
    ifeq ($(origin WDMDDK), undefined)
        WDMDDK 	= $(firstword $(wildcard c:/tools/msdev/nt5ddk d:/tools/msdev/nt5ddk d:/NT5DDK $(WLAN_WINPFX)/tools/msdev/NT5DDK))
    endif

    WDMDDK := $(subst \,/,$(WDMDDK))
    ifeq ($(WDMDDK), )
        ERRORS += "WDMDDK is not defined!"
        ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
        ERRORS += ""
    endif
endif

ifdef REQUIRE_DDK 
    ifeq ($(origin DDK), undefined)
        DDK 	= $(firstword $(wildcard c:/tools/msdev/ddk d:/tools/msdev/ddk d:/NT4DDK $(WLAN_WINPFX)/tools/msdev/ddk))
    endif

    DDK := $(subst \,/,$(DDK))
    ifeq ($(DDK), )
        ERRORS += "DDK is not defined!"
        ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
        ERRORS += ""
    else
        DDK.UNC:=$(shell cygpath -u $(DDK))
	NEWPATH += $(DDK.UNC)/bin
    endif
endif

ifdef REQUIRE_W98DDK
    ifeq ($(origin W98DDK), undefined)
	W98DDK = $(firstword $(wildcard c:/tools/MSDEV/98ddk d:/tools/MSDEV/98ddk c:/98DDK d:/98ddk $(WLAN_WINPFX)/tools/msdev/98ddk ))
    endif
    ifeq ($(W98DDK), )
	ERRORS += "W98DDK is not defined!"
	ERRORS += "origin of W98DDK is $(origin W98DDK)"
	ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	ERRORS += ""
    else 	
	W98DDK := $(subst \,/,$(W98DDK))
        W98DDK.UNC:=$(shell cygpath -u $(W98DDK))
    endif
endif

ifdef REQUIRE_W95DDK
    ifeq ($(origin W95DDK), undefined)
	W95DDK = $(firstword $(wildcard c:/tools/msdev/95ddk d:/tools/msdev/95ddk d:/95ddk $(WLAN_WINPFX)/tools/msdev/95ddk))
    endif
    ifeq ($(W95DDK), )
	ERRORS += "W95DDK is not defined!"
	ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	ERRORS += ""
    else 	
	W95DDK := $(subst \,/,$(W95DDK))
        W95DDK.UNC:=$(shell cygpath -u $(W95DDK))
    endif
endif

ifdef REQUIRE_DOSEXE
    ifeq ($(origin DOS16), undefined)
	DOS16 = $(firstword $(wildcard d:/DOS16M $(WLAN_WINPFX)/tools/DOS16M ))
    endif
    ifeq ($(DOS16), )
	ERRORS += "DOS16 is not defined!"
	ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	ERRORS += ""
    else 	
        DOS16.UNC:=$(shell cygpath -u $(DOS16))
    endif
endif

ifdef REQUIRE_VC152
    ifeq ($(origin VC152), undefined)
	VC152 = $(firstword $(wildcard c:/tools/MSDEV/VC152 d:/tools/MSDEV/VC152 c:/msvc d:/VC152 $(WLAN_WINPFX)/tools/msdev/vc152))
    endif
    VC152 := $(subst \,/,$(VC152))
    ifeq ($(VC152), )
	ERRORS += "VC152 is not defined!"
	ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	ERRORS += ""
    else
        VC152.UNC:=$(shell cygpath -u $(VC152))
	endif
endif

ifdef REQUIRE_MSDEV
    ifeq ($(REQUIRE_SDK60), 1)
	MSDEV = $(firstword $(wildcard c:/tools/msdev/5472sdk60 d:/tools/msdev/5472sdk60 $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/5472sdk60))
        MSDEV := $(subst \,/,$(MSDEV))

	MSSDK = $(firstword $(wildcard c:/tools/msdev/5472sdk60 d:/tools/msdev/5472sdk60 $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/5472sdk60))
        MSSDK := $(subst \,/,$(MSSDK))
        ifeq ($(MSSDK), )
		ERRORS += "MSSDK is not defined!"
		ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
		ERRORS += ""
        else
		MSSDK.UNC:=$(shell cygpath -u $(MSSDK))
        endif

	MSDEV.UNC:=$(shell cygpath -u $(MSDEV))
	LIBVPATH += $(MSSDK.UNC)/Lib $(MSDEV.UNC)/VC/LIB
	LIBVPATH.W = $(foreach p,$(LIBVPATH),$(shell cygpath -w $p))
	MSDEV.LIBPATH = $(MSSDK)/Lib;$(MSDEV)/VC/LIB
	MSDEV.INCLUDE = $(MSSDK)/Include;$(MSDEV)/VC/INCLUDE

	NEWPATH += $(MSSDK.UNC)/Bin:$(MSDEV.UNC)/VC/Bin

	CLOK := $(strip $(shell PATH="$(PATH):$(NEWPATH)" type -P cl 2>&1))
	ifeq ($(CLOK),)
		ERRORS += "Fatal makefile error (env.mk)"
		ERRORS += "CC=$(CC) cannot be found in NEWPATH=$(NEWPATH)"
		ERRORS += "Current PATH=$(PATH)"
	endif

    else # REQUIRE_SDK60
        ifeq ($(origin MSDEV), undefined)
		MSDEV := $(firstword $(wildcard c:/tools/msdev/Studio d:/tools/msdev/Studio d:/Studio $(WLAN_WINPFX)/tools/msdev/studio))
        endif

        ifeq ($(origin MSSDK), undefined)
		MSSDK = $(firstword $(wildcard c:/tools/msdev/PlatformSDK d:/tools/msdev/PlatformSDK $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/PlatformSDK))
        endif
        MSSDK := $(subst \,/,$(MSSDK))
        ifeq ($(MSSDK), )
		ERRORS += "MSSDK is not defined!"
		ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
		ERRORS += ""
        else
		MSSDK.UNC:=$(shell cygpath -u $(MSSDK))
        endif

        ifeq ($(MSDEV), )
		ERRORS += "MSDEV is not defined!"
		ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
		ERRORS += ""
        else
                MSDEV.UNC:=$(shell cygpath -u $(MSDEV))
		# export LIB = "$(MSDEV)/vc98/mfc/lib;$(MSDEV)/vc98/lib"
		LIBVPATH += $(MSSDK.UNC)/lib $(MSDEV.UNC)/vc98/lib $(MSDEV.UNC)/vc98/mfc/lib
		LIBVPATH.W = $(foreach p,$(LIBVPATH),$(shell cygpath -w $p))
		MSDEV.LIBPATH = $(MSSDK)/lib;$(MSDEV)/vc98/mfc/lib;$(MSDEV)/vc98/lib
		MSDEV.INCLUDE = $(MSSDK)/include;$(MSDEV)/VC98/atl/include;$(MSDEV)/VC98/mfc/include;$(MSDEV)/VC98/include

		NEWPATH += $(MSSDK.UNC)/nt/bin:$(MSDEV.UNC)/Common/Tools/WINNT:$(MSDEV.UNC)/Common/MSDev98/Bin:$(MSDEV.UNC)/Common/Tools:$(MSDEV.UNC)/VC98/bin

		CLOK := $(strip $(shell PATH="$(PATH):$(NEWPATH)" type -P cl 2>&1))
		ifeq ($(CLOK),)
	   		ERRORS += "Fatal makefile error (env.mk)"
	   		ERRORS += "CC=$(CC) cannot be found in NEWPATH=$(NEWPATH)"
	   		ERRORS += "Current PATH=$(PATH)"
		endif
        endif # MSDEV
    endif # REQUIRE_SDK60
endif # REQUIRE_MSDEV

ifdef REQUIRE_VS2003
    ifeq ($(origin VCINSTALLDIR), undefined)
        VCINSTALLDIR := $(firstword $(wildcard c:/tools/msdev/VS2003 d:/tools/msdev/VS2003 $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/VisualStudioNET_2003/ ))
    endif
    ifeq ($(origin 3790SDK1830), undefined)
		3790SDK1830 = $(firstword $(wildcard c:/tools/msdev/3790sdk1830 d:/tools/msdev/3790sdk1830 $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/3790sdk1830))
    endif
    3790SDK1830 := $(subst \,/,$(3790SDK1830))
    ifeq ($(3790SDK1830), )
	 ERRORS += "3790SDK1830 is not defined!"
	 ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	 ERRORS += ""
    else
	 3790SDK1830.UNC:=$(shell cygpath -u $(3790SDK1830))
    endif

    ifeq ($(VCINSTALLDIR), )
	 ERRORS += "VCINSTALLDIR is not defined!"
	 ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	 ERRORS += ""
    else
        VCINSTALLDIR.UNC:=$(shell cygpath -u $(VCINSTALLDIR))
		LIBVPATH += $(3790SDK1830.UNC)/lib $(VCINSTALLDIR.UNC)/Vc7/atlmfc/lib $(VCINSTALLDIR.UNC)/Vc7/lib $(VCINSTALLDIR)/VC7/PlatformSDK/lib/prerelease $(VCINSTALLDIR)/VC7/PlatformSDK/lib $(VCINSTALLDIR)/SDK/v1.1/lib
		LIBVPATH.W = $(foreach p,$(LIBVPATH),$(shell cygpath -w $p))
		VCINSTALLDIR.LIBPATH = $(3790SDK1830)/lib; \
				$(VCINSTALLDIR)/Vc7/atlmfc/lib; \
				$(VCINSTALLDIR)/Vc7/lib; \
				$(VCINSTALLDIR)/VC7/PlatformSDK/lib/prerelease; \
				$(VCINSTALLDIR)/VC7/PlatformSDK/lib; \
				$(VCINSTALLDIR)/SDK/v1.1/lib
		VS2003.INCLUDE = $(3790SDK1830)/Include; \
				$(3790SDK1830)/Include/crt; \
				$(3790SDK1830)/Include/crt/sys; \
				$(VCINSTALLDIR)/Vc7/atlmfc/include; \
				$(VCINSTALLDIR)/Vc7/include; \
				$(VCINSTALLDIR)/VC7/PlatformSDK/include/prerelease; \
				$(VCINSTALLDIR)/VC7/PlatformSDK/include; \
				$(VCINSTALLDIR)/SDK/v1.1/include

		NEWPATH += $(VCINSTALLDIR.UNC)/Common7/IDE:$(VCINSTALLDIR.UNC)/Common7/Tools:$(VCINSTALLDIR.UNC)/Vc7/bin

		CLOK := $(strip $(shell PATH="$(PATH):$(NEWPATH)" type -P cl 2>&1))
		ifeq ($(CLOK),)
	   		ERRORS += "Fatal makefile error (env.mk)"
	   		ERRORS += "CC=$(CC) cannot be found in NEWPATH=$(NEWPATH)"
	   		ERRORS += "Current PATH=$(PATH)"
		endif
    endif
endif

ifdef REQUIRE_VS2003AMD64
    ifeq ($(origin VS2003AMD64), undefined)
        VS2003AMD64 := $(firstword $(wildcard c:/tools/msdev/VS2003AMD64 d:/tools/msdev/VS2003AMD64 $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/VS2003AMD64/ ))
    endif
    ifeq ($(origin 3790SDK1830), undefined)
		3790SDK1830 = $(firstword $(wildcard c:/tools/msdev/3790sdk1830 d:/tools/msdev/3790sdk1830 $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/3790sdk1830))
    endif
    3790SDK1830 := $(subst \,/,$(3790SDK1830))
    ifeq ($(3790SDK1830), )
	 ERRORS += "3790SDK1830 is not defined!"
	 ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	 ERRORS += ""
    else
	 3790SDK1830.UNC:=$(shell cygpath -u $(3790SDK1830))
    endif

    ifeq ($(VS2003AMD64), )
	 ERRORS += "VS2003AMD64 is not defined!"
	 ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	 ERRORS += ""
    else
        VS2003AMD64.UNC:=$(shell cygpath -u $(VS2003AMD64))
		LIBVPATH += $(VS2003AMD64.UNC)/atlmfc/lib/amd64 $(VS2003AMD64.UNC)/lib/amd64 $(3790SDK1830.UNC)/lib/amd64
		LIBVPATH.W = $(foreach p,$(LIBVPATH),$(shell cygpath -w $p))
		VS2003AMD64.LIBPATH = $(VS2003AMD64)/atlmfc/lib/amd64; \
				$(VS2003AMD64)/lib/amd64; \
				$(3790SDK1830)/lib/amd64
		VS2003.INCLUDE = $(VS2003AMD64)/atlmfc/include; \
				$(VS2003AMD64)/include; \
				$(VS2003AMD64)/include/sys; \
				$(3790SDK1830)/Include

		NEWPATH += $(VS2003AMD64.UNC)/bin/x86_amd64

		CLOK := $(strip $(shell PATH="$(PATH):$(NEWPATH)" type -P cl 2>&1))
		ifeq ($(CLOK),)
	   		ERRORS += "Fatal makefile error (env.mk)"
	   		ERRORS += "CC=$(CC) cannot be found in NEWPATH=$(NEWPATH)"
	   		ERRORS += "Current PATH=$(PATH)"
		endif
    endif
endif

ifdef REQUIRE_LOCAL
    ifeq ($(origin LOCAL), undefined)
	LOCAL = $(firstword $(wildcard c:/tools/local $(WLAN_WINPFX)/tools/local))
    endif

    LOCAL := $(subst \,/,$(LOCAL))
    ifeq ($(LOCAL), )
	ERRORS += "LOCAL is not defined!"
	ERRORS += "This variable must be defined to build the cert tool."
	ERRORS += ""
    endif
endif

ifdef REQUIRE_WDM7600
    ifeq ($(origin WDM7600WDK), undefined)
        WDM7600WDK = $(firstword $(wildcard c:/tools/msdev/7600wdk d:/tools/msdev/7600wdk d:/7600wdk $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/7600wdk))
    endif

    WDM7600WDK := $(subst \,/,$(WDM7600WDK))
    ifeq ($(WDM7600WDK), )
        ERRORS += "WDM7600WDK is not defined!"
        ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
        ERRORS += ""
    endif
    WDM7600WDK.UNC:=$(shell cygpath -u $(WDM7600WDK))
endif

ifdef REQUIRE_WDM2600
    ifeq ($(origin WDM2600DDK), undefined)
        WDM2600DDK = $(firstword $(wildcard c:/tools/msdev/2600ddk d:/tools/msdev/2600ddk d:/2600ddk $(WLAN_WINPFX)/tools/msdev/2600ddk))
    endif

    WDM2600DDK := $(subst \,/,$(WDM2600DDK))
    ifeq ($(WDM2600DDK), )
        ERRORS += "WDM2600DDK is not defined!"
        ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
        ERRORS += ""
    endif
    WDM2600DDK.UNC:=$(shell cygpath -u $(WDM2600DDK))
endif

ifdef REQUIRE_WDM3790
    ifeq ($(origin WDM3790DDK), undefined)
        WDM3790DDK = $(firstword $(wildcard c:/tools/msdev/3790ddk d:/tools/msdev/3790ddk d:/3790ddk $(WLAN_WINPFX)/tools/msdev/3790ddk))
    endif

    WDM3790DDK := $(subst \,/,$(WDM3790DDK))
    ifeq ($(WDM3790DDK), )
        ERRORS += "WDM3790DDK is not defined!"
        ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
        ERRORS += ""
    endif
    WDM3790DDK.UNC:=$(shell cygpath -u $(WDM3790DDK))
endif

ifdef REQUIRE_WDM3790DDK1830
    ifeq ($(origin WDM3790DDK1830), undefined)
        WDM3790DDK1830 = $(firstword $(wildcard c:/tools/msdev/3790ddk1830 d:/3790ddk1830 $(WLAN_WINPFX)/tools/msdev/3790ddk1830))
    endif

    WDM3790DDK1830 := $(subst \,/,$(WDM3790DDK1830))
    ifeq ($(WDM3790DDK1830), )
        ERRORS += "WDM3790DDK1830 is not defined!"
        ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
        ERRORS += ""
    endif
    WDM3790DDK1830.UNC:=$(shell cygpath -u $(WDM3790DDK1830))
endif

ifdef REQUIRE_ACTIVE_PERL
    ifeq ($(origin ACTIVE_PERL_ROOT), undefined)
        ACTIVE_PERL_ROOT = $(firstword $(wildcard c:/ActivePerl/Perl d:/ActivePerl/Perl c:/tools/ActivePerl/Perl d:/tools/ActivePerl/Perl $(WLAN_WINPFX)/projects/hnd/tools/win/ActivePerl/Perl))
    endif


    ifeq ($(ACTIVE_PERL_ROOT), )
        ERRORS += "ACTIVE_PERL_ROOT is not defined!"
        ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
        ERRORS += ""
    else
		ACTIVE_PERL_ROOT.UNC := $(shell cygpath -u $(ACTIVE_PERL_ROOT)/bin)
		NEWPATH += $(ACTIVE_PERL_ROOT.UNC)
	endif

endif

ifdef REQUIRE_HTMLHELP
    ifeq ($(origin HTMLHELP), undefined)
	empty:=
	space:= $(empty) $(empty)
	SHELL=bash.exe
	HTMLHELP := $(firstword $(wildcard c:/tools/msdev/HTMLHelp d:/tools/msdev/HTMLHelp $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/HTMLHelp f:/PROGRA~1/HTMLHE~1/ e:/PROGRA~1/HTMLHE~1/ ))
    endif

    ifneq ($(HTMLHELP), )
        HTMLHELP.UNC:=$(shell cygpath -u $(HTMLHELP))
	LIBVPATH += $(HTMLHELP.UNC)/lib
	LIBVPATH.W = $(foreach p,$(LIBVPATH),$(shell cygpath -w $p))
	HTMLHELP.LIBPATH = $(HTMLHELP)/lib
	HTMLHELP.INCLUDE = $(HTMLHELP)/include

	NEWPATH += $(HTMLHELP.UNC)
    endif # ($(HTMLHELP), )
endif # REQUIRE_HTMLHELP

ifdef REQUIRE_WINCE

	ifeq ($(origin WINCEVER), undefined)
		ERRORS += "WINCEVER in not defined"
	endif

    ifeq ($(origin _WINCEROOT), undefined)
		_WINCEROOT = $(firstword $(wildcard c:/tools/msdev/wince$(WINCEVER) d:/tools/msdev/wince$(WINCEVER) $(WLAN_WINPFX)/projects/hnd/tools/wince$(WINCEVER)))
    else # _WINCEROOT (if _WINCEROOT wrongly defined, correct it)
		ifeq ($(filter %$(WINCEVER),$(_WINCEROOT)),)
			WARNING += "_WINCEROOT is WRONGLY defined for WINCEVER=$(WINCEVER). Fixing!"
			override _WINCEROOT:=$(firstword $(wildcard c:/tools/msdev/wince$(WINCEVER) d:/tools/msdev/wince$(WINCEVER) $(WLAN_WINPFX)/projects/hnd/tools/wince$(WINCEVER)))
		endif
    endif # _WINCEROOT

    _WINCEROOT := $(subst \,/,$(_WINCEROOT))
    ifeq ($(_WINCEROOT), )
	 ERRORS += "_WINCEROOT is not defined!"
	 ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	 ERRORS += ""
    else
	 _WINCEROOT.UNC:=$(shell cygpath -u $(_WINCEROOT))
    endif

    ifeq ($(origin _SDKROOT), undefined)
	 _SDKROOT = $(firstword $(wildcard $(_WINCEROOT)/SDK))
    endif
    _SDKROOT := $(subst \,/,$(_SDKROOT))
    ifeq ($(_SDKROOT), )
	 ERRORS += "_SDKROOT is not defined!"
	 ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	 ERRORS += ""
    else
	 _SDKROOT.UNC:=$(shell cygpath -u $(_SDKROOT))
    endif

    ifeq ($(origin _PUBLICROOT), undefined)
	 _PUBLICROOT = $(firstword $(wildcard $(_WINCEROOT)/PUBLIC))
    endif
    _PUBLICROOT := $(subst \,/,$(_PUBLICROOT))
    ifeq ($(_PUBLICROOT), )
	 ERRORS += "_PUBLICROOT is not defined!"
	 ERRORS += "This variable must be defined for TARGETTYPE=$(TARGETTYPE)."
	 ERRORS += ""
    else
	 _PUBLICROOT.UNC:=$(shell cygpath -u $(_PUBLICROOT))
    endif

ifeq ($(WINCEVER), 420)
    ifeq ($(PROCESSOR), MIPS)
	    CC=clmips
	endif
	ifeq ($(PROCESSOR), ARM)
	    CC=clarm
    endif
	ifeq ($(PROCESSOR), X86)
		CC=cl386
	endif
    export NEWPATH += $(_SDKROOT.UNC)/BIN/I386:$(_PRIVATEROOT.UNC)/BIN/I386/prefast/scripts:$(_PUBLICROOT.UNC)/common/oak/Bin/i386:$(_PRIVATEROOT.UNC)/bin/i386:$(_PRIVATEROOT.UNC)/bat:$(_PUBLICROOT.UNC)/common/oak/misc
else
    ifeq ($(PROCESSOR), MIPS)
	CC=$(_WINCEROOT)/SDK/bin/I386/$(PROCESSOR)/cl
    endif
    ifeq ($(PROCESSOR), ARM)
	CC=$(_WINCEROOT)/SDK/bin/I386/$(PROCESSOR)/cl
    endif
    ifeq ($(PROCESSOR), X86)
	CC=$(_WINCEROOT)/SDK/bin/I386/$(PROCESSOR)/cl
    endif
    export NEWPATH += $(_SDKROOT.UNC)/BIN/I386:$(_SDKROOT.UNC)/BIN/I386/$(PROCESSOR):$(_PRIVATEROOT.UNC)/BIN/I386/prefast/scripts:$(_PUBLICROOT.UNC)/common/oak/Bin/i386:$(_PRIVATEROOT.UNC)/bin/i386:$(_PRIVATEROOT.UNC)/bat:$(_PUBLICROOT.UNC)/common/oak/misc
endif

    CCOK = $(shell PATH="$(PATH):$(NEWPATH)" type -P $(CC) 2>&1)
    ifeq ($(CCOK),)
	   ERRORS += "Fatal makefile error (env.mk)"
	   ERRORS += "CC=$(CC) cannot be found in path $(PATH):$(NEWPATH)"
    endif

endif # REQUIRE_WINCE
