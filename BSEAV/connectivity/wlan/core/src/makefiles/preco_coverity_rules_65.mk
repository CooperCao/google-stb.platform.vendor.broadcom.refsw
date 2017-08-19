#
# Coverity Rules to build coverity targets specified in preco
# infrastructure
#
# PRECO_COVERITY_BASELINE_BUILD when set, creates only
# baseline builds and submits to existing streams and projects
#
#
# $Id: preco_coverity_rules.mk 356607 2012-09-13 00:35:35Z $
#
# NOTE: Do not update this makefile or add any new targets. Contact 
# NOTE: prakashd for any questions
#
# Author: Bob Quinn
# Contact: hnd-software-scm-list
#

UNAME                := $(shell uname -s)
SRCBASE              := ..
BUILD_BASE           := $(shell cd $(SRCBASE); pwd -P)
PPID                 := $(shell perl -le 'print getppid()')
COVADMIN             ?= preco_user
COV_PWD              ?= precopw10
COVSERVER            ?= wlansw-coverity.sj.broadcom.com
COVPORT              ?= 8080
COVURL               ?= http://$(COVSERVER)

# These have to be supplied by precommit infrastructure
PRECO_BRANCH         ?= NOBRANCH
PRECO_USER           ?= NOUSER

# For non-baseline usage, PRECO_USER and PRECO_BRANCH are needed
ifeq ($(PRECO_COVERITY_BASELINE_BUILD),)
     ifeq ($(PRECO_BRANCH),NOBRANCH)
          $(error ERROR: PRECO_BRANCH cmd line option missing or not set)
     endif

     ifeq ($(PRECO_USER),NOUSER)
          $(error ERROR: PRECO_USER cmd line option missing or not set)
     endif
endif # PRECO_COVERITY_BASELINE_BUILD

ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
  GALLERY            := Z:/projects/hnd_software/gallery
else
  GALLERY            := /projects/hnd_software/gallery
endif

PRECO_TWIKI          := http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/CoverityStaticCodeAnalysis\#Precommit_Coverity_Defect_Resolu

# To debug set MK_LOC to any custom preco_coverity_rules.mk file
# PRECO_COVERITY_MK_LOC:= preco_coverity_rules.mk

PRECO_COVERITY_MK    := $(firstword $(wildcard \
                        $(if $(PRECO_COVERITY_MK_LOC),$(PRECO_COVERITY_MK_LOC)) \
                        $(GALLERY)/src/makefiles/preco_coverity_rules_65.mk \
                        $(SRCBASE)/makefiles/preco_coverity_rules_65.mk))
PRECO_RULES_MK       := $(firstword $(wildcard \
                        preco_rules.mk \
                        $(GALLERY)/src/makefiles/preco_rules.mk \
                        $(SRCBASE)/makefiles/preco_rules.mk))

NODENAME             := $(shell uname -n)
NULL                 := /dev/null
CURDATE              := $(shell date '+%m/%d/%Y')
CURTIME              := $(shell date '+%H:%M:%S')
CURTSTAMP            := $(shell date '+%Y%m%d_%H%M%S')
#CURUSER             := $(strip $(shell id -un))
CURUSER              := $(strip $(notdir $(subst \,/,$(shell whoami))))
TOOLSDIR             := /projects/hnd/tools
MARKSTART             = date +"[%D %T $(NODENAME)]   START: $@"
MARKEND               = date +"[%D %T $(NODENAME)]   END  : $@"
PREVENTVER           ?= 6.5.1
TOUCH                ?= touch

# In debug mode, the coverity commands become dummy no-op operations
ifneq ($(DBG)$(DEBUG)$(NOOP),)
  override NOOP=echo
endif

## Windows coverity run specific settings
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
  COV-WINBUILD        := 1
  COVBUILD_CORES      ?= $(NUMBER_OF_PROCESSORS)
  COVBUILD_PROCESSES  ?= -j $(COVBUILD_CORES)
  PREVENTDIR          ?= Z:$(TOOLSDIR)/win/Coverity/cov-sa-win32-6.5.1
  EXE                 ?= .exe
  TEMP                := c:/temp
  CURARCH              = $(if $(findstring _x86,$@),x86,$(if $(findstring _x64,$@),x64,$(if $(findstring _amd64,$@),amd64)))
endif # CYGWIN

## Linux coverity run specific settings
ifeq ($(findstring Linux,$(UNAME)),Linux)
  TEMP                := /tmp
  COV-LINUXBUILD      := 1
  COVBUILD_CORES      ?= $(shell cat /proc/cpuinfo | grep -c "^processor")
# COVBUILD_CORES      ?= 4
  COVBUILD_PROCESSES  ?= -j $(COVBUILD_CORES)
  ifeq ($(findstring x86_64,$(shell uname -i)),x86_64)
    COV-LX64BUILD     := 1
    PREVENTDIR        ?= $(TOOLSDIR)/linux/Coverity/cov-sa-linux64-6.5.1
    # Cross compile 32bit wl/dhd and wl/dhd utils on a 64bit system
    32BIT              = -32 ARCH=i386 32ON64=1
    # Following is for kernel makefile, like /tools/linux/src/linux-2.6.9-intc1/Makefile
    export 32ON64      = 1
  else  # Linux
    COV-LX32BUILD     := 1
    PREVENTDIR        ?= $(TOOLSDIR)/linux/Coverity/cov-sa-linux32-6.5.1
  endif # uname -i
  LINUX_WL_OPTS       := LINUXVER=2.6.29.4-167.fc11.i686.PAE
  LINUX_WL_OPTS       += GCCVER=4.3.0$(32BIT)
  LINUX_WL_OPTS       += CROSS_COMPILE=/tools/gcc/4.3.0/i686-2.6/bin/
  LINUX_WL_OPTS       += SHOWWLCONF=1
endif # Linux

LINUX_WL_TARGETS     += debug-apdef-stadef
LINUX_UTILS_TARGETS  += linux-wl-exe

## MacOS coverity run specific settings
ifeq ($(findstring Darwin,$(UNAME)),Darwin)
  COV-MACBUILD        := 1
  TEMP                := /tmp
  PREVENTDIR          ?= $(TOOLSDIR)/macos/Coverity/cov-sa-macosx-6.5.1
  COVBUILD_CORES      ?= $(shell sysctl hw.ncpu | awk '{print $$2}')
  COVBUILD_PROCESSES  ?= -j $(COVBUILD_CORES)
endif # Darwin

## Global Coverity variables
export CURBLDTARGET  = $(if $(BLDTARGET),$(BLDTARGET),$(subst /,_,$@))
export CURCOVTARGET  = $(if $(COVTARGET),$(COVTARGET),$@)
# Temporary streams created for precommit have zzz prefixed to put them down
# when seeing streams and projects in Coverity UI
ifdef UPDATE_BASELINE
export COVSTREAM    ?= precommit_integration_$(PRECO_BRANCH)__$(CURBLDTARGET)
export COVPROJECT   ?= Precommit_Integration
else
export COVSTREAM    ?= precommit_$(PRECO_USER)_$(CURTSTAMP)_$(PRECO_BRANCH)__$(CURBLDTARGET)
export COVPROJECT   ?= Precommit_Test
endif
# Baselines to compare against
export COVSTREAM_BL ?= precommit_integration_$(PRECO_BRANCH)__$(CURBLDTARGET)
export COVPROJECT_BL?= $(COVSTREAM_BL)
export COVERRLVL    ?= 100
export COVTMPDIR    ?= $(TEMP)/precoverity/$(COVSTREAM)

export WLANCOMPMAP  ?= hndwlan

# Coverity Build Step options
COVBUILD_CMD        ?= $(NOOP) \
                       $(PREVENTDIR)/bin/cov-build$(EXE) \
                       --dir $(COVTMPDIR) \
                       --return-emit-failures \
                       --parse-error-threshold $(COVERRLVL)

# Coverity Analyze Step options
export COVANALYZE_THREADS ?= auto
export COVPWCONF          ?= $(PREVENTDIR)/config/parse_warnings.conf
export COVCHECKERS        ?= --security \
                             --concurrency \
                             --enable-parse-warnings \
                             --enable-constraint-fpp \
                             --enable missing_lock \
                             --enable STACK_USE \
                             --enable ASSERT_SIDE_EFFECT \
                             --enable INTEGER_OVERFLOW \
                             --disable UNUSED_VALUE \
                             --parse-warnings-config $(COVPWCONF)

# Effect Coverity analyze command. Checkers are
COVANALYZE_CMD      ?= $(NOOP) \
                       $(PREVENTDIR)/bin/cov-analyze$(EXE) \
                       $(COVCHECKERS) \
                       $(if $(COVANALYZE_THREADS),-j $(COVANALYZE_THREADS))

COVFORMATERRORS_CMD ?= $(NOOP) \
                       $(PREVENTDIR)/bin/cov-format-errors$(EXE) 

COVCOMPAREERRORS_CMD ?= $(NOOP) \
                       $(PREVENTDIR)/jre/bin/java -Xms1g -Xmx1g -jar $(TOOLSDIR)/linux/Coverity/cov-compare-errors-html/target/cov-compare-errors-html.jar --dir $(COVTMPDIR) -cim $(COVURL) --port $(COVPORT) --user $(COVADMIN) --password $(COV_PWD) --stream $(COVSTREAM_BL)

# Coverity Commit Step options
COVCOMMIT_CMD        = $(NOOP) \
                       $(PREVENTDIR)/bin/cov-commit-defects$(EXE) \
                       --host $(COVSERVER) --port $(COVPORT) --user $(COVADMIN) --password $(COV_PWD) \
                       --dir $(COVTMPDIR)

# Match Coverity target description for reference baselines to
# nightly coverity target description convention (as needed by reporting)
ifdef PRECO_COVERITY_BASELINE_BUILD
      # Get descriptive names for coverity targets from nightly database
      COVTARGETS   := $(GALLERY)/src/tools/release/coverity-target-info.mk
      COVTARGETINFO = $(subst Preco ,,$(subst preco_,,$(shell gmake --no-print-directory -f $(COVTARGETS) TAG=$(PRECO_BRANCH) preco_$(CURCOVTARGET) 2> $(NULL))))
      COVTARGETDESC = Preco $(PRECO_BRANCH) $(COVTARGETINFO) $(CURTSTAMP)
else
      COVTARGETDESC = $(subst _, ,$(COVSTREAM))
endif # PRECO_COVERITY_BASELINE_BUILD

COVCOMMITFLAG       := true

# Command to create new stream per precommit user or run
CMD_CREATE_STREAM        ?= $(NOOP) $(PREVENTDIR)/bin/cov-manage-im$(EXE) \
                            --host $(COVSERVER) --port $(COVPORT) --user $(COVADMIN) --password $(COV_PWD) \
                            --mode streams --add \
                            --set name:$(COVSTREAM) \
                            --set lang:cpp \
                            --set triage:DRIVER \
                            --set "desc:$(COVSTREAM)"

# Command to join new stream to project
CMD_JOIN_STREAM_PROJECT  ?= $(NOOP) $(PREVENTDIR)/bin/cov-manage-im$(EXE) \
                            --host $(COVSERVER) --port $(COVPORT) --user $(COVADMIN) --password $(COV_PWD) \
                            --mode projects --update \
                            --name $(COVPROJECT) \
                            --insert stream:$(COVSTREAM)

define RUN_COVERITY_ANALYZE
	@date +"[%D %T] start $0 for $@"
	@echo "#- $0"; \
	echo "$(COVANALYZE_CMD) --dir $(COVTMPDIR) $(if $1,--derived-model-file $(COVDMODELDIR)/$1)"; \
	$(COVANALYZE_CMD) --dir $(COVTMPDIR) $(if $1,--derived-model-file $(COVDMODELDIR)/$1); \
	cov_analyze_ec=$$?; \
	if [ "$$cov_analyze_ec" != "0" ]; then \
	     echo "ERROR: '$@' cov-analyze failed (exit code: $$cov_analyze_ec)"; \
	     echo "ERROR: Skipping commit to coverity db"; \
	     $(TOUCH) $(COVTMPDIR)/status-cov-analyze-failed;\
	else \
	     echo "INFO: '$@' cov-analyze exit code: $$cov_analyze_ec"; \
	     echo "INFO: Coverity analyze step done, format errors will run next"; \
	     $(TOUCH) $(COVTMPDIR)/status-cov-analyze-done; \
	fi
	date +"[%D %T] end $0 for $@"
endef # RUN_COVERITY_ANALYZE

define RUN_COVERITY_FORMAT_ERRORS
	@date +"[%D %T] start $0 for $@"
	@echo "#- $0"; \
	echo "$(COVFORMATERRORS_CMD) --dir $(COVTMPDIR) -X -x"; \
	$(COVFORMATERRORS_CMD) --dir $(COVTMPDIR) -X -x; \
	cov_format_ec=$$?; \
	if [ "$$cov_format_ec" != "0" ]; then \
	     echo "ERROR: '$@' cov-format-errors failed (exit code: $$cov_format_ec)"; \
	     echo "ERROR: Skipping commit to coverity db"; \
	     $(TOUCH) $(COVTMPDIR)/status-cov-analyze-failed; \
	else \
	     echo "INFO: '$@' cov-format-errors exit code: $$cov_format_ec"; \
	     echo "INFO: Coverity format errors step done, compare errors will run next"; \
	     $(TOUCH) $(COVTMPDIR)/status-cov-format-errors-done; \
	fi
	date +"[%D %T] end $0 for $@"
endef # RUN_COVERITY_FORMAT_ERRORS

define RUN_COVERITY_COMPARE_ERRORS
	@date +"[%D %T] start $0 for $@"
	@echo "#- $0"; \
	echo "$(COVCOMPAREERRORS_CMD)"; \
	$(COVCOMPAREERRORS_CMD); \
	cov_compare_ec=$$?; \
	if [ "$$cov_compare_ec" == "0" ]; then \
	     echo "INFO: '$@' cov-compare-errors exit code: $$cov_compare_ec"; \
	     echo "ERROR: Skipping commit to coverity db"; \
	     $(TOUCH) $(COVTMPDIR)/status-cov-compare-errors-done; \
	else \
	     echo "ERROR: '$@' cov-compare-errors failed (exit code: $$cov_compare_ec)"; \
	     echo "INFO: Coverity compare errors step done, commit errors will run next"; \
	     $(TOUCH) $(COVTMPDIR)/status-cov-compare-failed; \
	fi
	date +"[%D %T] end $0 for $@"
endef # RUN_COVERITY_COMPARE_ERRORS

## Commit the coverity analyzed results to custom stream
## Args: $1=target $2=description suffix
ifeq ($(COVCOMMITFLAG),true)
define RUN_COVERITY_COMMIT
	@date +"[%D %T] start $0 for $(BLDTARGET)"
	@echo "#- $0($1,$2)"
	@echo "Target Name        = $(subst /,_,$1)"
	@if [ "$(PRECO_COVERITY_BASELINE_BUILD)" != "" -a "$(COVTARGETINFO)" == "" ]; then \
		echo "ERROR: Coverity Target Description couldn't be derived"; \
		echo "ERROR: Ensure that $(COVTARGETS) has preco_$(CURCOVTARGET) entry"; \
		exit 1; \
	fi
	@echo "Target Description = $(COVTARGETDESC)"
	desc=`echo "$(COVTARGETDESC)" | sed -e 's/[[:space:]]+/ /g'`; \
	$(COVCOMMIT_CMD) \
	     --stream $(COVSTREAM) \
	     --description "$$desc $2" \
	     --target "$(subst /,_,$1)"; \
	cov_commit_ec=$$?; \
	if [ "$$cov_commit_ec" != "0" ]; then \
	       echo "ERROR: '$@' cov-commit failed (exit code: $$cov_commit_ec)"; \
	       echo "ERROR: Skipping commit to coverity db"; \
	       exit 1; \
	else \
	       echo "INFO: '$@' cov-commit exit code: $$cov_commit_ec"; \
	       echo "INFO: Coverity commit step done"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-commit-done; \
	fi
	@date +"[%D %T] end $0 for $@"
	@echo "==================================================="
	@echo ""
endef # RUN_COVERITY_COMMIT
endif # COVCOMMITFLAG

## Create dummy rtecdc.h or rtecdc_<chiprev>.h for bmac driver builds
define CREATE_DUMMY_DONGLE_IMAGE
	@date +"[%D %T] start $0 for $@"
	$(MAKE) -f $(PRECO_RULES_MK) \
		generate_dummy_dongle \
		generate_dummy_dongles
	@date +"[%D %T] end $0 for $@"
endef #CREATE_DUMMY_DONGLE_IMAGE

# If no coverity target is specified on command line, it is
# an error, as any target needs a backend support in cov db
all:
	@echo "ERROR: No default coverity targets specified"
	@echo "ERROR: Available coverity targets are:"
	@echo "Linux WL    : $(LINUX_WL_TARGETS)"
	@echo "Linux Utils : $(LINUX_UTILS_TARGETS)"
	@echo "Linux Dongle: $(LINUX_DNGL_TARGETS)"
	exit 1

# Pre-requisite target
build_include:
	@[ -s "$(SRCBASE)/include/epivers.h" ]  || \
		$(MAKE) -C $(SRCBASE)/include

# 
# Build Linux WL Drivers
# 
$(LINUX_WL_TARGETS): build_include
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	@echo "MAKEFLAGS = $(MAKEFLAGS)"
	rm -rf $(COVTMPDIR)
	$(call CREATE_DUMMY_DONGLE_IMAGE,$@)
	@date +"[%D %T] start cov-build for $@ =================="
ifdef UPDATE_BASELINE
	$(MAKE) -C $(SRCBASE)/wl/linux clean
endif # UPDATE_BASELINE
	$(COVBUILD_CMD) $(MAKE) -C $(SRCBASE)/wl/linux $(LINUX_WL_OPTS) $@
	@date +"[%D %T] end   cov-build for $@ =================="
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-analyze
ifdef UPDATE_BASELINE
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
	        BLDTARGET=$(subst /,_,$@) \
	        COVTARGET=$@ \
	        cov-commit;
else  # UPDATE_BASELINE
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-format-errors
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-compare-errors
	if [ -f $(COVTMPDIR)/status-cov-compare-failed ]; then \
	      $(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
	         BLDTARGET=$(subst /,_,$@) \
	         COVTARGET=$@ \
	         cov-commit; \
	      echo "For defects see URL $(COVURL):$(COVPORT)/query/defects.htm?stream=$(COVSTREAM)&outstanding=true"; \
	      rm -rf $(COVTMPDIR); \
	      false; \
	fi
endif # UPDATE_BASELINE
	rm -rf $(COVTMPDIR)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Linux WL and DHD Utility
#
$(LINUX_UTILS_TARGETS): CURTGT=$(if $(findstring -wl-,$@),wl,dhd)
$(LINUX_UTILS_TARGETS): build_include
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	@echo "MAKEFLAGS = $(MAKEFLAGS)"
	rm -rf $(COVTMPDIR)
	@date +"[%D %T] start cov-build for $@ =================="
ifdef UPDATE_BASELINE
	$(MAKE) -C $(SRCBASE)/$(CURTGT)/exe clean
endif # UPDATE_BASELINE
	$(COVBUILD_CMD) $(MAKE) -C $(SRCBASE)/$(CURTGT)/exe
	@date +"[%D %T] end   cov-build for $@ =================="
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-analyze
ifdef UPDATE_BASELINE
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
	        BLDTARGET=$(subst /,_,$@) \
	        COVTARGET=$@ \
	        cov-commit;
else  # UPDATE_BASELINE
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-format-errors
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-compare-errors
	if [ -f $(COVTMPDIR)/status-cov-compare-failed ]; then \
	      $(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
	         BLDTARGET=$(subst /,_,$@) \
	         COVTARGET=$@ \
	         cov-commit; \
	      echo "For defects see URL $(COVURL):$(COVPORT)/query/defects.htm?stream=$(COVSTREAM)&outstanding=true"; \
	      rm -rf $(COVTMPDIR); \
	      false; \
	fi
endif # UPDATE_BASELINE
	rm -rf $(COVTMPDIR)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

# 
# Build Linux Dongle Images
# 
$(LINUX_DNGL_TARGETS): build_include
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	@echo "MAKEFLAGS = $(MAKEFLAGS)"
	rm -rf $(COVTMPDIR)
	@date +"[%D %T] start cov-build for $@ =================="
ifdef UPDATE_BASELINE
	$(MAKE) -C $(SRCBASE)/dongle/rte/wl clean
endif # UPDATE_BASELINE
ifdef ($(PRECO_COVERITY_BASELINE_BUILD),)
#	$(MAKE) -C $(SRCBASE)/dongle/rte/wl clean
	rm -rf $(SRCBASE)/dongle/rte/wl/builds/$(@D)
endif # PRECO_COVERITY_BASELINE_BUILD
	$(COVBUILD_CMD) $(MAKE) $(COVBUILD_PROCESSES) -C $(SRCBASE)/dongle/rte/wl $@
	@date +"[%D %T] end   cov-build for $@ =================="
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-analyze
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-commit
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Common Steps for all targets
#

cov-analyze:
	$(call RUN_COVERITY_ANALYZE)

cov-format-errors:
	$(call RUN_COVERITY_FORMAT_ERRORS)

cov-compare-errors:
	$(call RUN_COVERITY_COMPARE_ERRORS)

ifdef UPDATE_BASELINE
cov-commit:
else
cov-commit: cov-commit-prep
endif
	@$(MARKSTART)
	@echo "Committing Coverity Defects"
	$(call RUN_COVERITY_COMMIT,$(BLDTARGET))
	@$(MARKEND)

# Prereqs for commit step
cov-commit-prep:
	@$(MARKSTART)
	@echo "Creating new stream $(COVSTREAM) in project $(COVPROJECT)"
	$(CMD_CREATE_STREAM)
	$(CMD_JOIN_STREAM_PROJECT)
	@$(MARKEND)

.PHONY: all cov-analyze cov-commit cov-commit-prep $(COVANALYZE_THREADS)
