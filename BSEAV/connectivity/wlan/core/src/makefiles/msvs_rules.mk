## ######################################################################
##
## Common rules for Visual Studio solutions and projects
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## $Id$
##
## ######################################################################

include $(SRCBASE)/makefiles/msvs_defs.mk

## PROJECT and SOLUTION definitions come from caller makefile
## some basic definitions are left in this file instead of 
## msvs_defs.mk, for ready reference to understand the rules

ACTION            := build
PROJCFG_RELEASE   ?= Release
PROJCFG_DEBUG     ?= Debug
PROJCFG_RELEASEU  ?= ReleaseU
PROJCFG_DEBUGU    ?= DebugU
MAKE              := $(MAKE) --no-print-directory
WINDIR            := $(subst \,/,$(WINDIR))
LOGFILE           ?= $(ACTION)_$(basename $(PROJECT)).log
NULL              ?= /dev/null
ifeq ($(findstring emake,$(MAKE)),)
      LOGOPT       = /out $(ACTION)_$(@F)_$(OEM).log
endif
UNICONV_WIN = $(subst /,\\,$(UNICONV))

## PROJCFGDIR is passed from VS pre/post-build events
##
## WARNING WARNING WARNING
## This variable must not contain any spaces or shell
## metacharacters, like brackets, otherwise MAKE will
## be utterly uncooporative completing your build!
## Futhermore, the below assignment should remain unquoted
## otherwise the targets won't be matched correctly.
##
ifneq ($(PROJCFGDIR),)
  override PROJCFGDIR      := $(subst \,/,$(PROJCFGDIR))
endif

# Inherit the brand/oem list from calling makefile
OEM               ?= bcm
OEM_LIST          ?= $(OEM)

## List of configurations to build by default
## Individual modules can override this locally

ifndef PROJECT_CONFIGS
     PROJECT_CONFIGS          := "$(PROJCFG_DEBUG)|win32"
     PROJECT_CONFIGS          += "$(PROJCFG_RELEASE)|win32"
     PROJECT_CONFIGS          += "$(PROJCFG_DEBUG)|x64"
     PROJECT_CONFIGS          += "$(PROJCFG_RELEASE)|x64"
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

# Macro to set env paths specific to certain project configuration
SET_PROJCFG_ENV=\
	    if echo "$${cfg}" | grep -q -i "win32" > $(NULL) 2>&1; then \
	       if [ "$${setnewenv}" == "1" ]; then \
	          echo -e "\nINFO: Resetting env paths for 32bit '$$cfg'\n"; \
	          PATH="$(PATH_WIN32)"; INCLUDE="$(INCLUDE_WIN32)"; \
	          LIB="$(LIB_WIN32)"; LIBPATH="$(LIBPATH_WIN32)"; \
	          setnewenv=0; \
	       fi; \
	    elif echo "$${cfg}" | grep -q -i "x64" > $(NULL) 2>&1; then \
	       echo -e "\nINFO: Setting env paths for 64bit '$$cfg'\n"; \
	       PATH="$(PATH_X64)"; LIB="$(LIB_X64)"; LIBPATH="$(LIBPATH_X64)"; \
	       setnewenv=1; \
	    elif echo "$${cfg}" | grep -q -i "ARMV4I" > $(NULL) 2>&1; then \
	       echo -e "\nINFO: Setting env paths for ARMV4I '$$cfg'\n"; \
	       PATH="$(PATH_CE)"; INCLUDE="$(INCLUDE_CE)" LIB="$(LIB_CE)"; \
	       setnewenv=1; \
	    fi; \
	    if [ ! "$${PATH}" ]; then echo "ERROR: empty PATH"; exit 1; fi; \
	    if [ "$${setnewenv}" == "1" -o -n "$(DBG)" ]; then \
	       echo "PROJCFG ($$cfg) new paths:"; echo "--------------"; \
	       echo "PROJECT PATH = $${PATH}";    echo "--------------"; \
	       echo "PROJECT LIB  = $${LIB}";     echo "--------------"; \
	       echo "PROJECT INC  = $${INCLUDE}"; echo "--------------"; \
	    fi

## build or rebuild or clean a single project for a given OEM setting
##
$(PROJECT) project: log=$(ACTION)_$(@F)_$(OEM).log
$(PROJECT) project:
	@date +"Start: $@, %D %T [OEM=$(OEM)])"
	@if [ ! -n $(BUILD_BRANDS) ]; then rm -f $(log); fi
	@for cfg in $(PROJECT_CONFIGS); do \
	    $(SET_PROJCFG_ENV); \
	    echo "$(DEVENV) $(SOLUTION) /$(ACTION) \"$$cfg\" /project $(@F) \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS)"; \
	    $(DEVENV) $(SOLUTION) /$(ACTION) "$$cfg" /project $(@F) \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS); \
	done
	$(MAKE) LOGFILE=$(log) ACTION=$(ACTION) scan_log
	@date +"End:   $@, %D %T [OEM=$(OEM)])"

##
## Build or rebuild or clean a solution for a given OEM setting
##
$(SOLUTION) solution: log=$(ACTION)_$(@F)_$(OEM).log
$(SOLUTION) solution:
	@date +"Start: $@, %D %T [SOLUTION=$(SOLUTION) OEM=$(OEM)])"
	@for cfg in $(PROJECT_CONFIGS); do \
	    $(SET_PROJCFG_ENV); \
	    echo "[OEM=$(OEM)] $(DEVENV) $(SOLUTION) /$(ACTION) \"$$cfg\" \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS)"; \
	    $(DEVENV) $(SOLUTION) /$(ACTION) "$$cfg" \
	              $(LOGOPT) $(DEVENV_EXTRA_FLAGS); \
	done
	@date +"End:   $@, %D %T [SOLUTION=$(SOLUTION) OEM=$(OEM)])"

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
	    $(MAKE) OEM=$$oem ACTION=$(ACTION) $(PROJECT); \
	done
	@for oem in $(OEM_LIST); do \
	    echo "Scanning for errors in $(ACTION)_project_$${oem}.log"; \
	    $(MAKE) LOGFILE=$(ACTION)_project_$${oem}.log scan_log; \
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
	    $(MAKE) OEM=$$oem ACTION=$(ACTION) solution; \
	done
	@for oem in $(OEM_LIST); do \
	    echo "Scanning for errors in $(ACTION)_solution_$${oem}.log"; \
	    $(MAKE) LOGFILE=$(ACTION)_solution_$${oem}.log scan_log; \
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
Debug Release:
	$(MAKE) PROJECT_CONFIGS=$@ all

##
## Scan the build log from the solution build
##

showerrors scan_log scanlog:
	@echo "============================================================="
	@echo "BUILD RESULTS:"
	@if [ -f $(LOGFILE) -a "$(LOGOPT)" != "" ]; then \
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
	@[ -d "$(@D)" ] || mkdir -pv $(@D)
	@cat $^ > $(@D)/temp_$(OEM)_genres2;
	$(UNICONV) -s utf8 -d utf16 -i $(@D)/temp_$(OEM)_genres2 -o $@
	rm -f $(@D)/temp_$(OEM)_genres2
endef

log_prebuild_env:
	@env | sort > $(SRCBASE)/wl/cpl/$(notdir $(PROJECT))_$(notdir $(SOLUTION))_env.log


ifeq ($(VS_PRE_BUILD_ENABLED),)

# Force echo statements into logfile but don't fail targets if missing headers
.PHONY: prebuild_rc $(PROJCFGDIR)/$(OEM)/$(PROJECTRC2) $(PROJCFGDIR)/$(OEM)/$(BRANDINGRC2) $(PROJCFGDIR)/$(OEM)/OEMDefs.h $(PROJCFGDIR)/$(OEM)/autores.h $(PROJCFGDIR)/$(OEM)/oem_change.sh

prebuild_rc: $(PROJCFGDIR)/$(OEM)/$(PROJECTRC2) $(PROJCFGDIR)/$(OEM)/$(BRANDINGRC2) $(PROJCFGDIR)/$(OEM)/OEMDefs.h $(PROJCFGDIR)/$(OEM)/autores.h 
	@echo "VS_PRE_BUILD_ENABLED was empty, skipping target $@"

$(PROJCFGDIR)/$(OEM)/$(PROJECTRC2):
	@echo "VS_PRE_BUILD_ENABLED was empty, skipping target $@"

$(PROJCFGDIR)/$(OEM)/$(BRANDINGRC2):
	@echo "VS_PRE_BUILD_ENABLED was empty, skipping target $@"

$(PROJCFGDIR)/$(OEM)/OEMDefs.h:
	@echo "VS_PRE_BUILD_ENABLED was empty, skipping target $@"

$(PROJCFGDIR)/$(OEM)/autores.h:
	@echo "VS_PRE_BUILD_ENABLED was empty, skipping target $@"

$(PROJCFGDIR)/$(OEM)/oem_change.sh:
	@echo "VS_PRE_BUILD_ENABLED was empty, skipping target $@"

else

prebuild_rc: $(PROJCFGDIR)/$(OEM)/$(PROJECTRC2) $(PROJCFGDIR)/$(OEM)/$(BRANDINGRC2) $(PROJCFGDIR)/$(OEM)/OEMDefs.h $(PROJCFGDIR)/$(OEM)/autores.h

$(PROJCFGDIR)/$(OEM)/$(PROJECTRC2):  $(foreach locale,$(LOCALES),$(PROJECTSTRINGS:%=$(SRCBASE)/wl/locale/$(locale)/%))
	@[ -d "$(@D)" ] || mkdir -pv $(@D)
	cmd /c " $(foreach locale,$(LOCALES), echo $(locale) && $(UNICONV_WIN) -s utf8 -d mb -p $(CODEPAGE_$(locale)) -i $(PROJECTSTRINGS:%=$(SRCBASE)/wl/locale/$(locale)/%) -o .temp_$(OEM)_locale_test &&) true "
	rm -f .temp_$(OEM)_locale_test
	$(GENERATE_RES2_FILE)

$(PROJCFGDIR)/$(OEM)/$(BRANDINGRC2): $(foreach locale,$(LOCALES),$(SRCBASE)/wl/locale/$(locale)/$(OEM).txt)
	@[ -d "$(@D)" ] || mkdir -pv $(@D)
	$(GENERATE_RES2_FILE)

$(PROJCFGDIR)/$(OEM)/OEMDefs.h: $(SRCBASE)/wl/locale/english/$(OEM).txt
	@[ -d "$(@D)" ] || mkdir -pv $(@D)
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' > $@

$(PROJCFGDIR)/$(OEM)/autores.h: $(PROJECTSTRINGS:%=$(SRCBASE)/wl/locale/english/%) $(foreach oem, $(OEM_LIST),$(SRCBASE)/wl/locale/english/$(oem).txt)
	@[ -d "$(@D)" ] || mkdir -pv $(@D)
	cat $^ | perl $(SRCBASE)/tools/locale/gen_autores.pl  > $@

$(PROJCFGDIR)/$(OEM)/oem_change.sh: $(SRCBASE)/wl/locale/english/$(OEM).txt GNUmakefile
	@[ -d "$(@D)" ] || mkdir -pv $(@D)
	perl -w $(dir $(lastword $(MAKEFILE_LIST)))oem_change.pl $< > $@
endif
	
## TARGET_PATH is supplied by caller project
## TODO: Simplify vista_copy rule similar to xp_copy
postbuild_vista_copy:
	@if [ "$(OEM)" == "bcm" -a "$(TRAY_POST_BUILD_DISABLED)" == "" ]; then \
	    tgt_path=$(subst \,/,$(TARGET_PATH)); \
	    tgt_dir=$(dir $(subst \,/,$(TARGET_PATH))); \
	    tgt_name=$(basename $(notdir $(subst \,/,$(TARGET_PATH)))); \
	    if echo "$${tgt_path}" | grep "x64" > $(NULL) 2>&1; then \
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

postbuild_xp_copy:
ifeq ($(OEM),bcm)
ifeq ($(TRAY_POST_BUILD_DISABLED),)
	@cp -pv $(subst \,/,$(TARGET_PATH)) $(WINDIR)/system32
else  # TRAY_POST_BUILD_DISABLED
	@echo "Post-build copying suppressed for TARGET_PATH=$(TARGET_PATH)"
endif # TRAY_POST_BUILD_DISABLED
endif # OEM

##
$(UNICONV): 
	$(warning WARN:  $(UNICONV) is NOT needed to be built)
	$(warning WARN:  instead a checked in uniconv.exe is used)
	$(error   ERROR: Exiting)
#disabled#	@echo "Building Uniconv: $(@)"
#disabled#	$(MAKE) -C $(SRCBASE)/tools/locale SRCFILE=sources TTYPE=OPT all

## ###########################################################################
## misc make targets
##

showenv:
	@echo "=========================================================="
	@echo "SOLUTION = $(SOLUTION)"
#	@echo 'PROJECTS = $(PROJECT_CONFIGS)'
	@echo "VSVER    = $(VSVER)"
	@echo "OEM_LIST = $(OEM_LIST)"
	@echo "OEM      = $(OEM)"
	@echo "LOGOPT   = $(LOGOPT)"
	@echo "PATH     = $(PATH)"
	@echo "INCLUDE  = $(INCLUDE)"
	@echo "LIB      = $(LIB)"
	@echo "LIBPATH  = $(LIBPATH)"
	@echo "PATH_X64 = $(PATH_X64)"
	@echo "LIB_X64  = $(LIB_X64)"
	@echo "LIBP_X64 = $(LIBPATH_X64)"

showenv_debug:
	@echo "=========================================================="
	@echo -e "MYVSINCLUDE= $(MYVSINCLUDE)\n"
	@echo -e "NEWINCLUDE = $(NEWINCLUDE)\n"
	@echo -e "INCLUDE    = $(INCLUDE)\n"
	@echo -e "MYVSLIB    = $(MYVSLIB)\n"
	@echo -e "NEWLIB     = $(NEWLIB)\n"
	@echo -e "LIB        = $(LIB)\n"
	@echo -e "MYCYGPATH  = $(MYCYGPATH)\n"
	@echo "=========================================================="

##
## End of msvs_rules.mk
##
