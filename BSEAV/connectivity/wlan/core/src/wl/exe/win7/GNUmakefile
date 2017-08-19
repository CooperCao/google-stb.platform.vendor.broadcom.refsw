##
## Vista/Win7 WLU DLL makefile
##
## $Id: GNUmakefile,v 1.19 2008-03-10 23:01:56 $

SRCBASE            := ../../../
COMPONENTS_SRCBASE := ../../../../components
SOLUTION           := $(SRCBASE)/wl/exe/wlu_dll.sln
VSVER              := VS2008
OEM                ?= bcm

## Build all projects with all OEM branding turned on
## To build currently set OEM, run 'gmake solution'
build_all: showenv all_solution_brands
	@echo "Scanning build_all_solution_brands.log for errors...."
	@$(MAKE) LOGFILE=build_all_solution_brands.log EXIT_ON_ERROR=1 scan_log

include $(COMPONENTS_SRCBASE)/apps/windows/makefiles/msvs_rules.mk

PROJECT_CONFIGS :=
PROJECT_CONFIGS += "$(PROJCFG_DEBUG)|win32"
PROJECT_CONFIGS += "$(PROJCFG_RELEASE)|win32"
PROJECT_CONFIGS += "$(PROJCFG_DEBUG)|x64"
PROJECT_CONFIGS += "$(PROJCFG_RELEASE)|x64"

## Clean all projects with all OEM branding turned on
## To clean currently set OEM, run 'gmake clean_solution'
clean_all: clean_all_solution_brands
