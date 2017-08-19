##
## Makefile used to build projects using Microsoft Visual Studio 2005
##
##

SRCBASE            := ../../../..
SOLUTION           := wps_test_gui.sln
VSVER              := VS2005
OEM                := bcm
OEM_LIST           := $(OEM)
REQUIRE_WM6PSDK	   := 1
VSLIB_PFX          := $(VSINSTALLDIR)/SmartDevices/SDK/PocketPC2003/Lib/armv4
VSINC_PFX          := $(VSINSTALLDIR)/SmartDevices/SDK/PocketPC2003/Include

build_all: showenv all_solution_brands

clean_all: clean_all_solution_brands

include $(SRCBASE)/makefiles/msvs_rules.mk

PROJECT_CONFIGS := "ARMV4I_Release|Pocket PC 2003 (ARMV4)"
