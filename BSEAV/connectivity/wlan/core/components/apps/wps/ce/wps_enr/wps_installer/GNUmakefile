##
## MS cabwiz tool does not work inside directories with longer paths.
## Hence create a temporary directory to create the .cab files
##

TEMP     := C:/temp
SRCBASE  := ../../../..
VARIANTS := sta
TTYPES   := free checked
SHELL    := bash.exe
CWD      := $(shell pwd)
CWD      := $(shell cygpath -m $(CWD))
TIMESTAMP:= $(shell date '+%Y%m%d%H%M%S')
#TEMPDIR  := $(TEMP)/WCE/$(TIMESTAMP)
#OLD TARGETS  := $(foreach variant,$(VARIANTS),$(foreach type,$(TYPES),$(variant)/$(WINCEVER)/$(PROCESSOR)/$(type)))
DEVPREFIX:= bcmwps
RETRYCMD       ?= $(firstword \
                  $(wildcard \
                     Z:/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                  ))
WINCEVER ?= 600
PROCESSOR?= ARM
TEMPDIR  := $(TEMP)/WCE/$(if $(TAG),$(TAG),NIGHTLY)/wps_api/CE$(WINCEVER)_$(PROCESSOR)_$(TIMESTAMP)
TARGETS  := ce$(WINCEVER)_$(PROCESSOR)
BTARGETS ?= $(foreach ttype,$(TTYPES),$(WINCEVER)/$(PROCESSOR)/$(ttype))
TARGETEXE:= wps_ce$(WINCEVER)_$(PROCESSOR)_setup.exe

WPS_API_OBJ_DIR  := $(SRCBASE)/wps/ce/wps_enr/wps_api/obj
WPS_GUI_OBJ_DIR  := $(SRCBASE)/wps/ce/wps_enr/wps_test_gui/WizTest/Pocket PC 2003 (ARMV4)/ARMV4I_Release'
BCMWLNPF_OBJ_DIR  := $(SRCBASE)/wl/ce/xsupplicant/winpcap/driver/obj
BCMWLPKT_OBJ_DIR  := $(SRCBASE)/wl/ce/xsupplicant/winpcap/dll/obj

## Tools needed to package
EZSETUP        := Z:/projects/hnd/tools/wince420/bin/ipaq/ezsetup.exe
WINCE_TOOL_DIR := Z:/projects/hnd/tools/win/msdev/VS2005/SmartDevices/SDK/SDKTools
#EZSETUP        := Z:/projects/hnd_tools/tools/wince420/bin/ipaq/ezsetup.exe
#WINCE_TOOL_DIR := Z:/projects/hnd_tools/tools/win/msdev/VS2005/SmartDevices/SDK/SDKTools

all: clean $(BTARGETS)

$(BTARGETS):
	@echo -e "\n-----------------------------------------------"
	@buildnode=$(shell hostname); \
	echo -e "$(if $(BRAND),$(BRAND): )Building variant $@ on $$buildnode\n"
	mkdir -p $(VARIANTS)/$@ $(TEMPDIR)
	@echo -e "TAG   = $(if $(TAG),$(TAG),NIGHTLY)"     >  $(TEMPDIR)/bldinfo.txt
	@echo -e "BRAND = $(if $(BRAND),$(BRAND),ce_$(DEVTYPE))" >> $(TEMPDIR)/bldinfo.txt
	# 
	# Copy over tools
	# 
	install -p -v $(WINCE_TOOL_DIR)/cabwiz.* $(TEMPDIR)
	install -p -v $(WINCE_TOOL_DIR)/Makecab.exe $(TEMPDIR)
	install -p -v $(EZSETUP) $(TEMPDIR)
	# 
	# Copy over pre-reqs for cabwiz and ezsetup
	# 
	install -p -v $(CWD)/$(DEVPREFIX).in* $(TEMPDIR)
	install -p -v $(CWD)/readme.txt $(TEMPDIR)
	install -p -v $(CWD)/license.txt $(TEMPDIR)
	install -p -v $(WPS_API_OBJ_DIR)/$(VARIANTS)/$@/wps_api.dll $(TEMPDIR)
	install -p -v "../../../../wps/ce/wps_enr/wps_test_gui/WizTest/Pocket PC 2003 (ARMV4)/ARMV4I_Release/wps_test_gui.exe" $(TEMPDIR)
	install -p -v $(BCMWLNPF_OBJ_DIR)/$@/bcmwlnpf.dll $(TEMPDIR)
	install -p -v $(BCMWLPKT_OBJ_DIR)/$@/bcmwlpkt.dll $(TEMPDIR)

	cd $(TEMPDIR) && ./Cabwiz.exe $(DEVPREFIX).inf /err cabwiz.error /compress
	@cat $(TEMPDIR)/cabwiz.error | col -b
	chmod +x ./signtool.exe
	$(RETRYCMD) ./signtool sign /v /f SDKSamplePrivDeveloper.pfx $(TEMPDIR)/$(DEVPREFIX).CAB
	cd $(TEMPDIR) && ./ezsetup.exe -l english -i $(DEVPREFIX).ini -r \
	readme.txt -e license.txt -o $(TARGETEXE)
	cp $(TEMPDIR)/$(TARGETEXE) $(CWD)/$(VARIANTS)/$@/$(TARGETEXE)
	$(RETRYCMD) ./signtool sign /f SDKSamplePrivDeveloper.pfx $(CWD)/$(VARIANTS)/$@/$(TARGETEXE)
	@if [ -f "$(CWD)/$(VARIANTS)/$@/$(TARGETEXE)" ]; then \
	   if [ -d "$(TEMPDIR)" ]; then rm -vrf $(TEMPDIR); fi; \
	else \
	   echo -e "\nError: $(CWD)/$(VARIANTS)/$@/$(TARGETEXE) not built\n"; \
	   echo -e "Error: Verify errors in $(TEMPDIR)\n"; \
	fi

clean:
	rm -rf $(VARIANTS)/$(foreach btgt,$(BTARGETS),$(btgt))

