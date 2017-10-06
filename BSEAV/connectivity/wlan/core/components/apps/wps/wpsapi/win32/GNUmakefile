##
## Makefile used to build projects using Microsoft Visual Studio 2005
##
##

SRCBASE            := ../../..
SOLUTION           := wps_enr.sln
PROJECT            := wps_api_test
VSVER              := VS2005
OEM                := bcm
OEM_LIST           := $(OEM)

build_all: showenv all_solution_brands

clean_all: clean_all_solution_brands

include $(SRCBASE)/makefiles/msvs_rules.mk

PROJECT_CONFIGS := "$(PROJCFG_DEBUG)|win32"
PROJECT_CONFIGS += "$(PROJCFG_RELEASE)|win32"
PROJECT_CONFIGS += "$(PROJCFG_DEBUG)|x64"
PROJECT_CONFIGS += "$(PROJCFG_RELEASE)|x64"
