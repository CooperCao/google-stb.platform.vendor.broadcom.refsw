## ######################################################################
##
## Common rules for Visual Studio solutions and projects
##
## $Id$
##
## ######################################################################

# Include old VS03 makefine definitions for backword compatibility

include $(SRCBASE)/makefiles/msvs_defs_vs03.mk

## PROJECT and SOLUTION definitions come from caller makefile
## some basic definitions are left in this file instead of 
## msvs_defs_vs03.mk, for ready reference to understand the rules

ACTION            := build
TARGET_TYPE_LIST  := debug free 
TARGET_DIR_free    = Release
TARGET_DIR_debug   = Debug
TARGET_DIR_freemh  = ReleaseMH
TARGET_DIR_debugmh = DebugMH
TARGET_DIR_freev   = Releasev
TARGET_DIR_debugv  = Debugv
TARGET_DIR_free64  = ReleaseAMD64
TARGET_DIR_debug64 = DebugAMD64
MAKE              := $(MAKE) --no-print-directory
WINDIR            := $(subst \,/,$(WINDIR))
LOGFILE           ?= $(ACTION)_$(basename $(PROJECT)).log
ifeq ($(findstring emake,$(MAKE)),)
      LOGOPT       = /out $(ACTION)_$(@F)_$(OEM).log
endif
UNICONV_WIN = $(subst /,\\,$(UNICONV))

## PROJCFGDIR is passed from VS pre/post-build events
ifneq ($(PROJCFGDIR),)
  override PROJCFGDIR      := $(subst \,/,$(PROJCFGDIR))
endif

# Inherit the brand/oem list from calling makefile
OEM               ?= bcm
ifeq ($(VSVER),VS2005)
OEM_LIST          ?= bcm dell
else
OEM_LIST          ?= bcm dell hp
endif

## List of configurations to build by default
## Individual modules can override this locally

ifndef PROJECT_CONFIGS

  ifeq ($(VSVER),VS2003)
     PROJECT_CONFIGS          := $(TARGET_DIR_debug)
     PROJECT_CONFIGS          += $(TARGET_DIR_free)

     ifndef PROJECT_CONFIGS_AMD64
        PROJECT_CONFIGS_AMD64    := $(TARGET_DIR_debug64)
        PROJECT_CONFIGS_AMD64    += $(TARGET_DIR_free64)
     endif

     ifndef PROJECT_CONFIGS_MH
        PROJECT_CONFIGS_MH       := $(TARGET_DIR_debugmh)
        PROJECT_CONFIGS_MH       += $(TARGET_DIR_freemh)
     endif

  else # VSVER

     PROJECT_CONFIGS          := "$(TARGET_DIR_debugv)|win32"
     PROJECT_CONFIGS          += "$(TARGET_DIR_freev)|win32"
     PROJECT_CONFIGS          += "$(TARGET_DIR_debugv)|x64"
     PROJECT_CONFIGS          += "$(TARGET_DIR_freev)|x64"
     PROJECT_CONFIGS_AMD64    :=
     PROJECT_CONFIGS_MH       :=

  endif # VSVER

endif # PROJECT_CONFIGS

## ######################################################################
## Start of rules. Do not put any definitions below this line
## ######################################################################

##
## Build project(s). For branded projects, default action is to build all brands
##
ifdef BUILD_BRANDS
   all: all_project_brands
else
   all: $(PROJECT)
endif

##
## Phony targets
##
.PHONY: all all_project_brands all_solution_brands $(PROJECT) project solution scan_log showerrors $(OEM)_free_rc $(OEM)_debug_rc $(OEM)_free64_rc $(OEM)_debug64_rc $(OEM)_freev_rc $(OEM)_debugv_rc $(OEM)_freemh_rc $(OEM)_debugmh_rc clean clean_cfg_dirs clean_res_dirs clean_$(PROJECT) rebuild_$(PROJECT) $(PROJECT_CONFIGS)

## build or rebuild or clean a single project for a given OEM setting
##
$(PROJECT) project: log=$(ACTION)_$(@F)_$(OEM).log
$(PROJECT) project:
	@date +"Start: $@, %D %T [OEM=$(OEM)] $(if $(BUILD_AMD64),[AMD64])"
	@if [ ! -n $(BUILD_BRANDS) ]; then rm -f $(log); fi
ifndef BUILD_AMD64
	@for cfg in $(PROJECT_CONFIGS); do \
	    echo "$(DEVENV) $(SOLUTION) /$(ACTION) $$cfg /project $(@F) \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS)"; \
	    $(DEVENV) $(SOLUTION) /$(ACTION) $$cfg /project $(@F) \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS); \
	done
else # BUILD_AMD64
	@for cfg in $(PROJECT_CONFIGS_AMD64); do \
	    echo "$(DEVENV) $(SOLUTION) /$(ACTION) $$cfg /project $(@F) \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS)"; \
	    $(DEVENV) $(SOLUTION) /$(ACTION) $$cfg /project $(@F) \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS); \
	done
endif # BUILD_AMD64
	-$(MAKE) -f $(MAKEFILE_VSXX) LOGFILE=$(log) ACTION=$(ACTION) scan_log
	@date +"End:   $@, %D %T [OEM=$(OEM)] $(if $(BUILD_AMD64),[AMD64])"

##
## Build or rebuild or clean a solution for a given OEM setting
##
$(SOLUTION) solution: log=$(ACTION)_$(@F)_$(OEM).log
$(SOLUTION) solution:
	@date +"Start: $@, %D %T [SOLUTION=$(SOLUTION) OEM=$(OEM)] $(if $(BUILD_AMD64),[AMD64])"
ifndef BUILD_AMD64
	@for cfg in $(PROJECT_CONFIGS); do \
	    if echo "$${cfg}" | grep x64 > /dev/null 2>&1; then \
	       if [ "$(VSVER)" == "VS2005" ]; then \
	          echo "Setting PATH, LIB, LIBPATH for x64 configuration"; \
	          PATH="$(PATH_X64)"; LIB="$(LIB_X64)"; LIBPATH="$(LIBPATH_X64)"; \
	       fi; \
	    fi; \
	    echo "[OEM=$(OEM)] $(DEVENV) $(SOLUTION) /$(ACTION) $$cfg \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS)"; \
	    $(DEVENV) $(SOLUTION) /$(ACTION) $$cfg \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS); \
	done
ifneq ($(PROJECT_CONFIGS_MH),)
	@for cfg in $(PROJECT_CONFIGS_MH); do \
	    echo "[OEM=$(OEM)] $(DEVENV) $(SOLUTION) /$(ACTION) $$cfg /project TrayApp \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS)"; \
	    $(DEVENV) $(SOLUTION) /$(ACTION) $$cfg /project TrayApp \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS); \
	done
endif # PROJECT_CONFIGS_MH
else # BUILD_AMD64
	@for cfg in $(PROJECT_CONFIGS_AMD64); do \
	    echo "[OEM=$(OEM)] $(DEVENV) $(SOLUTION) /$(ACTION) $$cfg \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS)"; \
	    $(DEVENV) $(SOLUTION) /$(ACTION) $$cfg \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS); \
	done
endif # BUILD_AMD64
	@date +"End:   $@, %D %T [SOLUTION=$(SOLUTION) OEM=$(OEM)] $(if $(BUILD_AMD64),[AMD64])"

##
## Build or rebuild or clean a project for all OEMs in OEM_LIST
## PROJECT variable is set by the calling makefile
##
all_project_brands: log=$(ACTION)_$(@).log
all_project_brands:
	@date +"Start: $@, %D %T [OEM_LIST=$(OEM_LIST)]"
	@rm -f $(log)
	@for oem in $(OEM_LIST); do \
	    rm -f $(ACTION)_project_$(OEM).log; \
	    $(MAKE) -f $(MAKEFILE_VSXX) OEM=$$oem ACTION=$(ACTION) $(PROJECT); \
	done
	-@for oem in $(OEM_LIST); do \
	    echo "Scanning for errors in $(ACTION)_project_$${oem}.log"; \
	    $(MAKE) -f $(MAKEFILE_VSXX) LOGFILE=$(ACTION)_project_$${oem}.log scan_log; \
	done
	@date +"End:   $@, %D %T [OEM_LIST=$(OEM_LIST)]"

##
## Build or rebuild or clean solution for all OEMs in OEM_LIST
##
all_solution_brands: log=$(ACTION)_$(@).log
all_solution_brands:
	@date +"Start: $@, %D %T [OEM_LIST=$(OEM_LIST)]"
	@rm -f $(log)
	@for oem in $(OEM_LIST); do \
	    rm -f $(ACTION)_solution_$(OEM).log; \
	    $(MAKE) -f $(MAKEFILE_VSXX) OEM=$$oem ACTION=$(ACTION) solution; \
	done
ifeq ($(VSVER),VS2003)
  ifneq ($(PROJECT_CONFIGS_AMD64),)
	@for oem in $(OEM_LIST); do \
	    $(MAKE) -f $(MAKEFILE_VSXX) OEM=$$oem ACTION=$(ACTION) BUILD_AMD64=1 solution; \
	done
  endif
endif
	-@for oem in $(OEM_LIST); do \
	    echo "Scanning for errors in $(ACTION)_solution_$${oem}.log"; \
	    $(MAKE) -f $(MAKEFILE_VSXX) LOGFILE=$(ACTION)_solution_$${oem}.log scan_log; \
	done
	@date +"End:   $@, %D %T [OEM_LIST=$(OEM_LIST)]"

##
## Rebuild target
##
ifeq ($(BUILD_BRANDS),)
  rebuild rebuild_%: ACTION := rebuild
  rebuild rebuild_project:  $(PROJECT)
else
  rebuild: ACTION := rebuild
  rebuild: all_project_brands
endif

##
## Clean target
##
ifeq ($(BUILD_BRANDS),)
  clean clean_%: ACTION := clean
  clean clean_project:  $(PROJECT)
else
  clean: ACTION := clean
  clean: all_project_brands
endif

##
## Clean solution with current OEM setting
##
clean_solution: ACTION := clean
clean_solution: $(SOLUTION)

##
## [Manual] Clean Projects. Recursively call project cleans
##
clean_all_solution_brands: ACTION := clean
clean_all_solution_brands: all_solution_brands

##
## Build individual project configurations. This does not work
## at the solution folder level.
##
Debug Release DebugMH ReleaseMH:
	$(MAKE) -f $(MAKEFILE_VSXX) PROJECT_CONFIGS=$@ all

ifdef BUILD_AMD64
DebugAMD64 ReleaseAMD64:
	$(MAKE) -f $(MAKEFILE_VSXX) PROJECT_CONFIGS=$@ BUILD_AMD64=1 all
endif

##
## Scan the build log from the solution build
##

showerrors scan_log scanlog:
	@echo "============================================================="
	@echo "BUILD RESULTS:"
	@if [ -f $(LOGFILE) ]; then \
	    grep -a "error(s)" $(LOGFILE) | grep -a -v "0 error(s)" | sort -u; \
	    ec=$(shell grep -a "error(s)" $(LOGFILE) | grep -a -v "0 error(s)" | sort -u | wc -l | sed -e "s/[[:space:]]//g"); \
	    if [ "$${ec}" != "0" -a "$(EXIT_ON_ERROR)" != "" ]; then \
	       echo -e "\nExiting! Complete $(ACTION) output at $(LOGFILE)\n"; \
	       exit 123; \
	    fi; \
	    echo -e "\nComplete $(ACTION) output at $(LOGFILE)"; \
	else \
	    echo -e "\nFile $(LOGFILE) doesn't exist, scan_log skipped"; \
	fi
	@echo "============================================================="

## ###########################################################################
## Rules to generate .rc2 and locale pre-requisites for branding purposes
##

PROJECTSTRINGS ?= dummystrings.txt
PROJECTRC2     ?= dummy.rc2
BRANDINGRC     ?= dummy_b.rc
BRANDINGRC2    ?= dummy_b.rc2

define GENERATE_RES2_FILE
	@echo ">>> Generating resource file: $@"
	@echo "From: $(subst $(SRCBASE)/wl/locale/, ,$^)"
	@mkdir -p $(@D); cat $^ > $(@D)/temp_$(OEM)_genres2;
	$(UNICONV) -s utf8 -d utf16 -i $(@D)/temp_$(OEM)_genres2 -o $@
	rm -f $(@D)/temp_$(OEM)_genres2
endef

##
$(foreach oem, $(OEM_LIST), $(oem)_free_rc) : %_free_rc : $(UNICONV) $(TARGET_DIR_free)/$(PROJECTRC2) $(TARGET_DIR_free)/%/$(BRANDINGRC2) $(TARGET_DIR_free)/%/OEMDefs.h $(TARGET_DIR_free)/autores.h

$(foreach oem, $(OEM_LIST), $(oem)_freev_rc) : %_freev_rc : $(UNICONV) $(TARGET_DIR_freev)/$(PROJECTRC2) $(TARGET_DIR_freev)/%/$(BRANDINGRC2) $(TARGET_DIR_freev)/%/OEMDefs.h $(TARGET_DIR_freev)/%/autores.h

$(foreach oem, $(OEM_LIST), $(oem)_free64_rc) : %_free64_rc : $(UNICONV) $(TARGET_DIR_free64)/$(PROJECTRC2) $(TARGET_DIR_free64)/%/$(BRANDINGRC2) $(TARGET_DIR_free64)/%/OEMDefs.h $(TARGET_DIR_free64)/autores.h

## For MH
$(foreach oem, $(OEM_LIST), $(oem)_freemh_rc) : %_freemh_rc : $(UNICONV) $(TARGET_DIR_freemh)/$(PROJECTRC2) $(TARGET_DIR_freemh)/%/$(BRANDINGRC2) $(TARGET_DIR_freemh)/%/OEMDefs.h $(TARGET_DIR_freemh)/autores.h

##
$(foreach oem, $(OEM_LIST), $(oem)_debug_rc) : %_debug_rc : $(UNICONV) $(TARGET_DIR_debug)/$(PROJECTRC2) $(TARGET_DIR_debug)/%/$(BRANDINGRC2) $(TARGET_DIR_debug)/%/OEMDefs.h $(TARGET_DIR_debug)/autores.h

$(foreach oem, $(OEM_LIST), $(oem)_debugv_rc) : %_debugv_rc : $(UNICONV) $(TARGET_DIR_debugv)/$(PROJECTRC2) $(TARGET_DIR_debugv)/%/$(BRANDINGRC2) $(TARGET_DIR_debugv)/%/OEMDefs.h $(TARGET_DIR_debugv)/%/autores.h

$(foreach oem, $(OEM_LIST), $(oem)_debug64_rc) : %_debug64_rc : $(UNICONV) $(TARGET_DIR_debug64)/$(PROJECTRC2) $(TARGET_DIR_debug64)/%/$(BRANDINGRC2) $(TARGET_DIR_debug64)/%/OEMDefs.h $(TARGET_DIR_debug64)/autores.h

## For MH
$(foreach oem, $(OEM_LIST), $(oem)_debugmh_rc) : %_debugmh_rc : $(UNICONV) $(TARGET_DIR_debugmh)/$(PROJECTRC2) $(TARGET_DIR_debugmh)/%/$(BRANDINGRC2) $(TARGET_DIR_debugmh)/%/OEMDefs.h $(TARGET_DIR_debugmh)/autores.h

## For MH also
$(TARGET_DIR_debug)/$(PROJECTRC2) $(TARGET_DIR_debug64)/$(PROJECTRC2) $(TARGET_DIR_free)/$(PROJECTRC2) $(TARGET_DIR_free64)/$(PROJECTRC2) $(TARGET_DIR_debugv)/$(PROJECTRC2) $(TARGET_DIR_freev)/$(PROJECTRC2) $(TARGET_DIR_debugmh)/$(PROJECTRC2) $(TARGET_DIR_freemh)/$(PROJECTRC2): $(foreach locale,$(LOCALES),$(PROJECTSTRINGS:%=$(SRCBASE)/wl/locale/$(locale)/%))
	cmd /c " $(foreach locale,$(LOCALES), echo $(locale) && $(UNICONV_WIN) -s utf8 -d mb -p $(CODEPAGE_$(locale)) -i $(PROJECTSTRINGS:%=$(SRCBASE)/wl/locale/$(locale)/%) -o .temp_$(OEM)_locale_test &&) true "
	rm -f .temp_$(OEM)_locale_test
	$(GENERATE_RES2_FILE)

##
$(foreach oem, $(OEM_LIST),$(TARGET_DIR_free)/$(oem)/$(BRANDINGRC2)) : $(TARGET_DIR_free)/%/$(BRANDINGRC2): $(foreach locale,$(LOCALES),$(SRCBASE)/wl/locale/$(locale)/%.txt)
	$(GENERATE_RES2_FILE)

$(foreach oem, $(OEM_LIST),$(TARGET_DIR_free64)/$(oem)/$(BRANDINGRC2)) : $(TARGET_DIR_free64)/%/$(BRANDINGRC2): $(foreach locale,$(LOCALES),$(SRCBASE)/wl/locale/$(locale)/%.txt)
	$(GENERATE_RES2_FILE)

$(foreach oem, $(OEM_LIST),$(TARGET_DIR_freev)/$(oem)/$(BRANDINGRC2)) : $(TARGET_DIR_freev)/%/$(BRANDINGRC2): $(foreach locale,$(LOCALES),$(SRCBASE)/wl/locale/$(locale)/%.txt)
	$(GENERATE_RES2_FILE)

## For MH
$(foreach oem, $(OEM_LIST),$(TARGET_DIR_freemh)/$(oem)/$(BRANDINGRC2)) : $(TARGET_DIR_freemh)/%/$(BRANDINGRC2): $(foreach locale,$(LOCALES),$(SRCBASE)/wl/locale/$(locale)/%.txt)
	$(GENERATE_RES2_FILE)

##
$(foreach oem, $(OEM_LIST),$(TARGET_DIR_debug)/$(oem)/$(BRANDINGRC2)) : $(TARGET_DIR_debug)/%/$(BRANDINGRC2): $(foreach locale,$(LOCALES),$(SRCBASE)/wl/locale/$(locale)/%.txt)
	@echo $(TARGET_DIR_debug)/$*/$(BRANDINGRC)
	cmd /c " $(foreach oem,$(OEM_LIST), $(foreach locale,$(LOCALES), echo $(oem) $(locale) && $(UNICONV_WIN) -s utf8 -d mb -p $(CODEPAGE_$(locale)) -i $(SRCBASE)/wl/locale/$(locale)/$(oem).txt -o .temp_$(OEM)_locale_test &&)) true "
	rm -f .temp_$(OEM)_locale_test
	$(GENERATE_RES2_FILE)

$(foreach oem, $(OEM_LIST),$(TARGET_DIR_debug64)/$(oem)/$(BRANDINGRC2)) : $(TARGET_DIR_debug64)/%/$(BRANDINGRC2): $(foreach locale,$(LOCALES),$(SRCBASE)/wl/locale/$(locale)/%.txt)
	@echo $(TARGET_DIR_debug64)/$*/$(BRANDINGRC)
	cmd /c " $(foreach oem,$(OEM_LIST), $(foreach locale,$(LOCALES), echo $(oem) $(locale) && $(UNICONV_WIN) -s utf8 -d mb -p $(CODEPAGE_$(locale)) -i $(SRCBASE)/wl/locale/$(locale)/$(oem).txt -o .temp_$(OEM)_locale_test &&)) true "
	rm -f .temp_$(OEM)_locale_test
	$(GENERATE_RES2_FILE)

$(foreach oem, $(OEM_LIST),$(TARGET_DIR_debugv)/$(oem)/$(BRANDINGRC2)) : $(TARGET_DIR_debugv)/%/$(BRANDINGRC2): $(foreach locale,$(LOCALES),$(SRCBASE)/wl/locale/$(locale)/%.txt)
	@echo $(TARGET_DIR_debugv)/$*/$(BRANDINGRC)
	cmd /c " $(foreach oem,$(OEM_LIST), $(foreach locale,$(LOCALES), echo $(oem) $(locale) && $(UNICONV_WIN) -s utf8 -d mb -p $(CODEPAGE_$(locale)) -i $(SRCBASE)/wl/locale/$(locale)/$(oem).txt -o .temp_$(OEM)_locale_test &&)) true "
	rm -f .temp_$(OEM)_locale_test
	$(GENERATE_RES2_FILE)

## For MH
$(foreach oem, $(OEM_LIST),$(TARGET_DIR_debugmh)/$(oem)/$(BRANDINGRC2)) : $(TARGET_DIR_debugmh)/%/$(BRANDINGRC2): $(foreach locale,$(LOCALES),$(SRCBASE)/wl/locale/$(locale)/%.txt)
	@echo $(TARGET_DIR_debugmh)/$*/$(BRANDINGRC)
	cmd /c " $(foreach oem,$(OEM_LIST), $(foreach locale,$(LOCALES), echo $(oem) $(locale) && $(UNICONV_WIN) -s utf8 -d mb -p $(CODEPAGE_$(locale)) -i $(SRCBASE)/wl/locale/$(locale)/$(oem).txt -o .temp_$(OEM)_locale_test &&)) true "
	rm -f .temp_$(OEM)_locale_test
	$(GENERATE_RES2_FILE)

##
$(foreach oem, $(OEM_LIST),$(TARGET_DIR_free)/$(oem)/OEMDefs.h) :  $(TARGET_DIR_free)/%/OEMDefs.h : $(SRCBASE)/wl/locale/english/%.txt
	mkdir -p $(@D)
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' >$@

$(foreach oem, $(OEM_LIST),$(TARGET_DIR_free64)/$(oem)/OEMDefs.h) :  $(TARGET_DIR_free64)/%/OEMDefs.h : $(SRCBASE)/wl/locale/english/%.txt
	mkdir -p $(@D)
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' >$@

$(foreach oem, $(OEM_LIST),$(TARGET_DIR_freev)/$(oem)/OEMDefs.h) :  $(TARGET_DIR_freev)/%/OEMDefs.h : $(SRCBASE)/wl/locale/english/%.txt
	mkdir -p $(@D)
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' >$@

## For MH
$(foreach oem, $(OEM_LIST),$(TARGET_DIR_freemh)/$(oem)/OEMDefs.h) :  $(TARGET_DIR_freemh)/%/OEMDefs.h : $(SRCBASE)/wl/locale/english/%.txt
	mkdir -p $(@D)
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' >$@

##
$(foreach oem, $(OEM_LIST),$(TARGET_DIR_debug)/$(oem)/OEMDefs.h) :  $(TARGET_DIR_debug)/%/OEMDefs.h : $(SRCBASE)/wl/locale/english/%.txt
	mkdir -p $(@D)
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' >$@

$(foreach oem, $(OEM_LIST),$(TARGET_DIR_debug64)/$(oem)/OEMDefs.h) :  $(TARGET_DIR_debug64)/%/OEMDefs.h : $(SRCBASE)/wl/locale/english/%.txt
	mkdir -p $(@D)
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' >$@

$(foreach oem, $(OEM_LIST),$(TARGET_DIR_debugv)/$(oem)/OEMDefs.h) :  $(TARGET_DIR_debugv)/%/OEMDefs.h : $(SRCBASE)/wl/locale/english/%.txt
	mkdir -p $(@D)
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' >$@

## For MH
$(foreach oem, $(OEM_LIST),$(TARGET_DIR_debugmh)/$(oem)/OEMDefs.h) :  $(TARGET_DIR_debugmh)/%/OEMDefs.h : $(SRCBASE)/wl/locale/english/%.txt
	mkdir -p $(@D)
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' >$@

## TARGET_PATH is supplied by caller project
## Current callers ServerApp, ClientApp, ControlPanel, Remoting, wltrysvc
$(foreach oem, $(OEM_LIST),$(oem)_debugv_copy) :
	@if [ "$(OEM)" == "bcm" -a "$(TRAY_POST_BUILD_DISABLED)" == "" ]; then \
	    tgt_path=$(subst \,/,$(TARGET_PATH)); \
	    tgt_dir=$(dir $(subst \,/,$(TARGET_PATH))); \
	    tgt_name=$(basename $(notdir $(subst \,/,$(TARGET_PATH)))); \
	    if echo "$${tgt_path}" | grep "x64" > /dev/null 2>&1; then \
		destwindir="X:/windows/system32"; \
	    else \
		destwindir="Y:/windows/system32"; \
	    fi; \
	    echo "Copying $${tgt_dir}$${tgt_name}.* to $${destwindir}"; \
	    cp -v $${tgt_path} $${destwindir}; \
	    cp -v $${tgt_dir}$${tgt_name}.pdb $${destwindir}; \
	else \
	    echo "Post-build copying suppressed for $@"; \
	fi

##
$(UNICONV): 
	@echo "Building Uniconv: $(@)"
	$(MAKE) -C $(SRCBASE)/tools/locale SRCFILE=sources TTYPE=OPT all

ifeq ($(VSVER),VS2005)

dotfuscate:
ifeq ($(SOLUTION),WLVista_VS05.sln)
	@for oem in $(OEM_LIST); do \
	    for cfg in $(PROJECT_CONFIGS); do \
	        $(MAKE) -C $(SRCBASE)/wl/cpl/vista/Obfuscator/Dotfuscator PROJCFG="$${cfg}" OEM=$${oem} $(findstring clean,$(ACTION)); \
	    done; \
	 done
else # SOLUTION = WLVista_VS05
	@echo "No dotfuscation done for $(VSVER) solution $(SOLUTION)"
endif # SOLUTION = WLVista_VS05

else # VSVER

dotfuscate:
	@echo "No dotfuscation done for $(VSVER) solutions"

endif # VSVER

## ###########################################################################
## misc make targets
##

showenv:
	@echo -e "=========================================================="
	@echo "SOLUTION = $(SOLUTION)"
	@echo "VSVER    = $(VSVER)"
	@echo "OEM_LIST = $(OEM_LIST)"
	@echo "PATH     = $(PATH)"
	@echo "INCLUDE  = $(INCLUDE)"
	@echo "LIB      = $(LIB)"
	@echo "LIBPATH  = $(LIBPATH)"
	@echo "PATH_X64 = $(PATH_X64)"
	@echo "LIB_X64  = $(LIB_X64)"
	@echo "LIBPATH_X64  = $(LIBPATH_X64)"
	@echo "LOGOPT   = $(LOGOPT)"
	@echo -e "=========================================================="
#	@echo -e "BUILD_AMD64     = $(BUILD_AMD64)"
#	@echo -e "PROJECT_CONFIGS        = $(subst |,/,$(PROJECT_CONFIGS))"
#	@echo -e "PROJECT_CONFIGS_AMD64  = $(subst |,/,$(PROJECT_CONFIGS_AMD64))"
#	@echo -e "PROJECT_CONFIGS_MH     = $(subst |,/,$(PROJECT_CONFIGS_MH))"
#	@echo -e "OEM                    = $(OEM)"
#	@set > $(SRCBASE)/wl/cpl/gmake_env.txt

##
## End of msvs_rules.mk
##
