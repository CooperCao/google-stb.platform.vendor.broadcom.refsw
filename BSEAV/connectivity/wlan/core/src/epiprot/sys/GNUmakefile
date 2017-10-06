# GNUmakefile : build all brands and target types (debug or release)
# of the wireless 802.11 driver
#
# $Id: GNUmakefile,v 1.21 2010-10-30 00:58:55 $
#

## Vista WDK related changes needed
SHELL          := bash.exe
SRCBASE	       := ../..
RETRYCMD       ?= $(firstword \
                  $(wildcard \
		     C:/tools/build/bcmretrycmd.exe \
                     Z:/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                  ))
PWD_DOS        := $(shell cygpath -p -w "$(shell pwd)")
CWDIR          := $(shell cygpath -p -m "$(shell pwd)")
WDK_VER        ?= 7100
WDKBASEDIR     := $(if $(BASEDIR),$(BASEDIR),$(subst /,\,$(firstword $(wildcard C:/WinDDK/$(WDK_VER) C:/tools/msdev/$(WDK_VER)wdk D:/WinDDK/$(WDK_VER) D:/tools/msdev/$(WDK_VER)wdk Z:/projects/hnd/tools/win/msdev/$(WDK_VER)wdk))))
WDK_SETENV     := $(WDKBASEDIR)\bin\setenv $(WDKBASEDIR)
# Turn off post build code review tool launch
WDK_OACR       ?= no_oacr
ACTION         ?= build
# All newer WDK tools (after 6000 wdk) call 64bit config as x64 instead of amd64
ifneq ($(filter 6000% 5%,$(WDK_VER)),)
  BUILD_ARCHS  ?= x86 amd64
else
  BUILD_ARCHS  ?= x86 x64
endif
BUILD_TYPES    ?= checked free
BUILD_VARIANTS := $(foreach arch,$(BUILD_ARCHS),$(foreach type,$(BUILD_TYPES),$(ACTION)_$(arch)_$(type))) 
BCMDRVPFX_X86  ?= bcm42rly.sys 
BCMDRVPFX_X64  ?= bcm42rly.sys 
MS_CROSSCERT   ?=Z:/projects/hnd/tools/win/verisign/driver/MSCV-VSClass3.cer
DOSCMD         := $(subst \,/,$(COMSPEC))
export PATH    := /usr/bin:/bin:$(PATH):$(shell cygpath -u $(subst \,/,$(WDKBASEDIR))/bin/SelfSign)

all: $(BUILD_VARIANTS)

$(BUILD_VARIANTS):
	@if [ "$(WDK_SETENV)" == "" ]; then \
		echo "ERROR:"; \
		echo "ERROR: Could not find install dir for $(WDK_VER)"; \
		echo "ERROR:"; \
		exit 1; \
	fi
	@echo "Running $@ now"
	-@cwd=$(shell cygpath -m $(shell pwd)); \
	type=$(subst checked,chk,$(subst free,fre,$(shell echo $@ | cut -d_ -f3))); \
	arch=$(shell echo $@ | cut -d_ -f2); \
	scr="build_$${type}_$${arch}.bat"; \
	echo -e "\n===============================================\n"; \
	echo -e "Building Vista $${arch} $${type} Protocol driver in '$(CWDIR)'"; \
	rm -f $${scr}; \
	echo -e "@echo Build Vista $${arch} $${type} protocol driver\n" >> $${scr}; \
	echo "@set PATH=$(subst /,\,$(WDKBASEDIR)/bin/x86);%path%" >> $${scr}; \
	echo -e "@echo Using WDKVER = $(WDK_VER)\n" >> $${scr}; \
	if [ "$${arch}" == "amd64" -o "$${arch}" == "x64" ]; then \
	   echo "@call $(WDK_SETENV) $${type} $${arch} WLH $(WDK_OACR)" >> $${scr}; \
	   objdir=obj$${type}_wlh_amd64/amd64; driver="$(BCMDRVPFX_X64)"; \
	else \
	   echo "@call $(WDK_SETENV) $${type} WLH $(WDK_OACR)" >> $${scr}; \
	   objdir=obj$${type}_wlh_x86/i386; driver="$(BCMDRVPFX_X86)"; \
	fi; \
	echo "@cd /D $(PWD_DOS)" >> $${scr}; \
	echo "@set MAKEFLAGS=" >> $${scr}; \
	echo "@goto ec%ERRORLEVEL%" >> $${scr}; \
	echo -e "\n:ec0" >> $${scr}; \
	echo "@if /I NOT \"%cd%\"==\"$(PWD_DOS)\" goto ec1" >> $${scr}; \
	echo "@title $(if $(TAG),$(TAG)_,NIGHTLY_)$(if $(BRAND),$(BRAND)_)$@" >> $${scr}; \
	echo "@echo Current Dir : %cd%" >> $${scr}; \
	echo "@build -bceZ" >> $${scr}; \
	echo "@if EXIST build$${type}_wlh_$${arch}.log if DEFINED BRAND echo Build LOG from $@ : && type build$${type}_wlh_$${arch}.log"  >> $${scr}; \
	echo "@if EXIST build$${type}_wlh_$${arch}.wrn echo Build WARNINGS from $@ : && type build$${type}_wlh_$${arch}.wrn"  >> $${scr}; \
	echo "@if EXIST build$${type}_wlh_$${arch}.err echo Build ERRORS from $@ : && type build$${type}_wlh_$${arch}.err"  >> $${scr}; \
	echo "@title DONE_$(if $(TAG),$(TAG)_,NIGHTLY_)$(if $(BRAND),$(BRAND)_)@" >> $${scr}; \
	echo "@goto done" >> $${scr}; \
	echo -e "\n:ec1" >> $${scr}; \
	echo "@echo Could not change dir to $(PWD_DOS)" >> $${scr}; \
	echo ":done" >> $${scr}; \
	$(DOSCMD) /c "$${scr}"; \
	echo "Signing the built driver now"; \
	cd $${objdir}; $(RETRYCMD) signtool sign /ac $(MS_CROSSCERT) /s my /n "Broadcom Corporation" /t "http://timestamp.verisign.com/scripts/timstamp.dll" $${driver}; \
	echo "Verifying the signature on built driver now"; \
	$(RETRYCMD) signtool verify /kp $${driver}
clean : 
	rm -rf obj*_wlh_*/i386/*.obj
	rm -rf obj*_wlh_*/amd64/*.obj

clean_all clobber : 
	rm -rf obj*_wlh_*
	rm -f build*_x86.bat build*_*64.bat
	rm -f build*_x86.log build*_*64.log
	rm -f build*_x86.wrn build*_*64.wrn
