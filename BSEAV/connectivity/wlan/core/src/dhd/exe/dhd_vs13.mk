#
# Makefile to build dhd.exe with MSbuild
#
VS_VER         ?= 2013

VSDIR          := $(firstword $(wildcard C:/tools/msdev/VS$(VS_VER) D:/tools/msdev/VS$(VS_VER) C:/tools/mdev/VS$(VS_VER)))

ACTION         := build
BUILD_ARCHS    ?= x86 x64
BUILD_CONFS    ?= Debug Release
BUILD_VARIANTS ?= $(foreach arch,$(BUILD_ARCHS),$(foreach conf,$(BUILD_CONFS),$(ACTION)_$(arch)_$(conf)))

empty          :=
space          := $(empty) $(empty)
NULL           := /dev/null
DOSCMD         := $(subst \,/,$(COMSPEC))

all: $(BUILD_VARIANTS)

$(BUILD_VARIANTS): ARCH=$(word 2,$(subst _,$(space),$@))
$(BUILD_VARIANTS): CONF=$(word 3,$(subst _,$(space),$@))
$(BUILD_VARIANTS):
	@echo -e "@echo off\n\n\
	@REM Automatically generated on $(shell date). Do not edit\n\n\
	echo Running at %date% %time% on %computername%\n\
	echo Current Dir : %cd%\n\
	if '$(ARCH)' == 'x86' set BLDOUTARCH=win32&& set VCVARSARCH=x86\n\
	if '$(ARCH)' == 'x64' set BLDOUTARCH=x64&& set VCVARSARCH=x86_amd64\n\
	if '$(ARCH)' == 'arm' set BLDOUTARCH=ARM&& set VCVARSARCH=x86_arm\n\
	set VCDIR=%VSDIR%\\\\VC\n\
	echo $(ACTION) $(CONF) $(ARCH) with VS$(VS_VER)\n\
	if NOT EXIST %VCDIR%\\\\vcvarsall.bat goto vcenverror\n\
	call %VCDIR%\\\\vcvarsall.bat %VCVARSARCH%\n\
	goto ec%ERRORLEVEL%\n\n\
	:ec0\n\
	which msbuild.exe\n\
	echo msbuild.exe /p:Configuration='$(CONF)' /p:Platform=%BLDOUTARCH% dhd.vcxproj /fileLoggerParameters:LogFile=$(ACTION)_$(ARCH)_$(CONF).log\n\
	msbuild.exe /p:Configuration=\"$(CONF)\" /p:Platform=%BLDOUTARCH% dhd.vcxproj /fileLoggerParameters:LogFile=$(ACTION)_$(ARCH)_$(CONF).log\n\
	set buildec=%ERRORLEVEL%\n\
	if /I NOT \"%buildec%\"==\"0\" goto buildec1\n\
	goto done\n\n\
	:vcenverror\n\
	echo ERROR: VS directory '%VCDIR%' is not found\n\
	exit /B 1\n\n\
	:buildec1\n\
	echo ERROR: %ACTION% failed with error code 1\n\
	exit /B 1\n\n\
	:buildec0\n\
	:done\n\
	echo Done with $(ACTION) $(CONF) $(ARCH)\n" \
	| sed \
	  -e 's/^[[:space:]]//g' \
	  -e "s!%WDKDIR%!$(subst /,~,$(WDKDIR))!g" \
	  -e "s!%VSDIR%!$(subst /,~,$(VSDIR))!g" \
	| sed -e "s/~/\\\\/g" > $@.bat
	$(DOSCMD) /c "$@.bat"
	rm -f $@.bat

clean_all:
	rm -f $(ACTION)_*.bat
	rm -rf x64
	rm -rf Debug
	rm -rf Release

PHONY: all clean_all $(BUILD_VARIANTS)
