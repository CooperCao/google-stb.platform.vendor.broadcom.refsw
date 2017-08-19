#
# Coverity Rules to build coverity targets specified in preco
# infrastructure
#
# PRECO_COVERITY_BASELINE_BUILD when set, creates only
# baseline builds and submits to existing streams and projects
#
#
# $Id$
#
# NOTE: Do not update this makefile or add any new targets. Contact 
# NOTE: prakashd for any questions
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#

UNAME                := $(shell uname -s)
SRCBASE              := ..
BUILD_BASE           := $(shell cd $(SRCBASE); pwd -P)
PPID                 := $(shell perl -le 'print getppid()')

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

ifeq ($(findstring Linux,$(UNAME)),Linux)
  GALLERY            := /projects/hnd_software/gallery
  PERL512DIR         := /projects/hnd/tools/linux/Perl/bin
else
  GALLERY            := Z:/projects/hnd_software/gallery
  PERL512DIR         := Z:/projects/hnd/tools/win/Perl/bin
endif

COVURL               := http://cov-sj1-08.sj.broadcom.com:8080
PRECO_TWIKI          := http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/CoverityStaticCodeAnalysis\#Precommit_Coverity_Defect_Resolu

# To debug set MK_LOC to any custom preco_coverity_rules.mk file
# PRECO_COVERITY_MK_LOC:= preco_coverity_rules.mk

PRECO_COVERITY_MK    := $(firstword $(wildcard \
                        $(if $(PRECO_COVERITY_MK_LOC),$(PRECO_COVERITY_MK_LOC)) \
                        $(GALLERY)/src/makefiles/preco_coverity_rules.mk \
                        $(SRCBASE)/makefiles/preco_coverity_rules.mk))
PRECO_RULES_MK       := $(firstword $(wildcard \
                        preco_rules.mk \
                        $(GALLERY)/src/makefiles/preco_rules.mk \
                        $(SRCBASE)/makefiles/preco_rules.mk))

WEBAPIDIR            := $(GALLERY)/src/tools/coverity/webapi
WEBAPIDIR_CONF       := $(WEBAPIDIR)/config
WEBAPIDIR_PERL       := $(WEBAPIDIR)/perl/scripts
WEBAPIDIR_PYTHON     := $(WEBAPIDIR)/python
PERL512              := $(PERL512DIR)/perl

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
PREVENTVER           ?= 5.3.0

# In debug mode, the coverity commands become dummy no-op operations
ifneq ($(DBG)$(DEBUG)$(NOOP),)
  override NOOP=echo
endif

## Windows coverity run specific settings
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
  COV-WINBUILD        := 1
  PREVENTDIR          ?= Z:$(TOOLSDIR)/win/Coverity/cov-sa-win32-5.3.0
  EXE                 ?= .exe
  TEMP                := c:/temp
  CURARCH              = $(if $(findstring _x86,$@),x86,$(if $(findstring _x64,$@),x64,$(if $(findstring _amd64,$@),amd64)))
endif # CYGWIN

## Linux coverity run specific settings
ifeq ($(findstring Linux,$(UNAME)),Linux)
  TEMP                := /tmp
  COV-LINUXBUILD      := 1
  ifeq ($(findstring x86_64,$(shell uname -i)),x86_64)
    COV-LX64BUILD     := 1
    PREVENTDIR        ?= $(TOOLSDIR)/linux/Coverity/cov-sa-linux64-5.3.0
    # Cross compile 32bit wl/dhd and wl/dhd utils on a 64bit system
    32BIT              = -32 ARCH=i386
    # Following is for kernel makefile, like /tools/linux/src/linux-2.6.9-intc1/Makefile
    export 32ON64      = 1
  else  # Linux
    COV-LX32BUILD     := 1
    PREVENTDIR        ?= $(TOOLSDIR)/linux/Coverity/cov-sa-linux32-5.3.0
  endif # uname -i
endif # Linux

LINUX_WL_TARGETS   += debug-apdef-stadef
LINUX_WL_TARGETS   += debug-apdef-stadef-high

## MacOS coverity run specific settings
ifeq ($(findstring Darwin,$(UNAME)),Darwin)
  COV-MACBUILD        := 1
  TEMP                := /tmp
  PREVENTDIR          ?= $(TOOLSDIR)/macos/Coverity/CurrentPPC
  # MacOS 5.3 tool is not yet deployed
  PREVENTDIR          ?= $(TOOLSDIR)/macos/Coverity/cov-sa-xxx-5.3.0
endif # Darwin

## Global Coverity variables
export CURBLDTARGET  = $(if $(BLDTARGET),$(BLDTARGET),$(subst /,_,$@))
export CURCOVTARGET  = $(if $(COVTARGET),$(COVTARGET),$@)
# Temporary streams created for precommit have zzz prefixed to put them down
# when seeing streams and projects in Coverity UI
export COVSTREAM    ?= zzzpreco_$(PRECO_USER)_$(CURTSTAMP)_$(PRECO_BRANCH)__$(CURBLDTARGET)
export COVPROJECT   ?= $(COVSTREAM)
# Baselines to compare against
export COVSTREAM_BL ?= preco_$(PRECO_BRANCH)__$(CURBLDTARGET)
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
THREADS             := 1 2 3 4
THREAD_COUNT        := 4
COVANALYZE_THREADS   = $(foreach thread,$(THREADS),$(foreach bldtarget,$(BLDTARGET),cov-analyze-thread$(thread)-$(bldtarget)))
COVOUTDIR_thread2   ?= $(COVTMPDIR)/c/output2
COVOUTDIR_thread3   ?= $(COVTMPDIR)/c/output3
COVOUTDIR_thread4   ?= $(COVTMPDIR)/c/output4

COVCHECKERS_thread1 ?= OVERRUN_DYNAMIC \
                       EVALUATION_ORDER \
                       NO_EFFECT
COVCHECKERS_thread2 ?= OVERRUN_STATIC \
                       NEGATIVE_RETURNS
COVCHECKERS_thread3 ?= UNINIT
COVCHECKERS_thread4 ?= FORWARD_NULL \
                       REVERSE_INULL \
                       NULL_RETURNS \
                       BUFFER_SIZE \
                       USE_AFTER_FREE \
                       RESOURCE_LEAK \
                       BAD_SIZEOF \
                       SIZEOF_MISMATCH

# Effect Coverity analyze command. Checkers are
COVANALYZE_CMD      ?= $(NOOP) \
                       $(PREVENTDIR)/bin/cov-analyze$(EXE) \
                       --disable-default \
                       --enable-constraint-fpp \

COVCOMMIT_ARGS      ?= --dir $(COVTMPDIR) \
                       -xo $(COVTMPDIR)/c/output2 \
                       -xo $(COVTMPDIR)/c/output3 \
                       -xo $(COVTMPDIR)/c/output4

# Coverity Commit Step options
COVCOMMIT_CMD        = $(NOOP) \
                       $(PREVENTDIR)/bin/cov-commit-defects$(EXE) \
                       --config $(WEBAPIDIR_CONF)/coverity_preco_config.xml \
                       $(COVCOMMIT_ARGS) \
                       --version "1.0.0"

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
CMD_CREATE_STREAM   ?= $(NOOP) \
                       $(PREVENTDIR)/bin/cov-manage-im$(EXE) \
                       --config $(WEBAPIDIR_CONF)/coverity_preco_config_im.xml \
                       --mode streams --add \
                       --set name:$(COVSTREAM) \
                       --set type:source \
                       --set component-map:$(WLANCOMPMAP) \
                       --set lang:cpp \
                       --set "desc:$(COVSTREAM)" \
                       --set name:$(COVSTREAM) \
                       --set type:static \
                       --set "desc:$(COVSTREAM)"

# Command to create new project per precommit user or run
# Ensure that user's private precommit stream and recurring baseline streams
# are both inserted into the stream
CMD_CREATE_PROJECT   ?=$(NOOP) \
                       $(PREVENTDIR)/bin/cov-manage-im$(EXE) \
                       --config $(WEBAPIDIR_CONF)/coverity_preco_config_im.xml \
                       --mode projects --add \
                       --set name:$(COVPROJECT) \
                       --set "desc:$(COVPROJECT)" \
                       --insert stream:$(COVSTREAM) \
                       --insert stream:$(COVSTREAM_BL)

# Command to remove new stream per precommit user or run
CMD_RM_STREAM       ?= $(NOOP) \
                       $(PREVENTDIR)/bin/cov-manage-im$(EXE) \
                       --config $(WEBAPIDIR_CONF)/coverity_preco_config_im.xml \
                       --mode streams --delete \
                       --name $(COVSTREAM)

# Command to remove new project per precommit user or run
CMD_RM_PROJECT       ?=$(NOOP) \
                       $(PREVENTDIR)/bin/cov-manage-im$(EXE) \
                       --config $(WEBAPIDIR_CONF)/coverity_preco_config_im.xml \
                       --mode projects --delete \
                       --name $(COVPROJECT)

# Given a temp precommit stream/project, find if any new defects were
# introduced and report them. If none, then temp stream/project is teared
# down, otherwise coverity url is passed to user for triage and owner is
# set to whoever submitted for precommit coverity
# Exit code from webapi when new defects are found
EXIT_CODE_NEW_DEFECT     := 142
CMD_GET_NEW_DEFECTS      := $(NOOP) \
                            $(PERL512) $(WEBAPIDIR_PERL)/get-new-project-defects.pl \
                            --config $(WEBAPIDIR_CONF)/coverity_webapi_config.xml \
                            --owner $(PRECO_USER)

# Given a temp precommit stream/project, remove snapshots in it, if no
# new defects are found there.
CMD_RM_STREAM_SNAPSHOTS  := $(NOOP) \
                            $(PERL512) $(WEBAPIDIR_PERL)/remove-stream-snapshots.pl \
                            --config $(WEBAPIDIR_CONF)/coverity_webapi_config.xml \
                            --stream $(COVSTREAM)

# Given a temp precommit stream/project, merge defects in it with baseline
# When this happens, all triaged defects from baseline appear as triaged
# in new precommit stream and then new ones can be searched explicitly
# by CMD_GET_NEW_DEFECTS step.
CMD_MIGRATE_DEFECTS      := $(NOOP) \
                            $(PERL512) $(WEBAPIDIR_PERL)/migrate-stream-defects.pl \
                            -config $(WEBAPIDIR_CONF)/coverity_webapi_config.xml \
                            -stream1 $(COVSTREAM_BL) \
                            -stream2 $(COVSTREAM) \
                            -update

## Args: $1 = emit-dir; $2 = checkers; $3 == thread-analyze-dir
define RUN_COVERITY_ANALYZE
	@date +"[%D %T] start $0 for $@"
	@echo "#- $0($1,$2,$3)"; \
	xodir=""; \
	if [ -n "$(strip $3)" ]; then \
	   xodir="--outputdir $3"; \
	fi; \
	echo "$(COVANALYZE_CMD) --dir $1 $2 $$xodir "; \
	$(COVANALYZE_CMD) --dir $1 $2 $$xodir; \
	cov_analyze_ec=$$?; \
	if [ "$$cov_analyze_ec" != "0" ]; then \
	   echo "ERROR: '$@' cov-analyze failed (exit code: $$cov_analyze_ec)"; \
	   echo "ERROR: Skipping commit to coverity db"; \
	   exit 1; \
	else \
	   echo "INFO: '$@' cov-analyze exit code: $$cov_analyze_ec"; \
	fi
	@date +"[%D %T] end $0 for $@"
endef # RUN_COVERITY_ANALYZE

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
	     $(if $(COVSTREAM),--stream $(COVSTREAM)) \
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
	fi
	@date +"[%D %T] end $0 for $@"
	@echo "==================================================="
	@echo ""
endef # RUN_COVERITY_COMMIT
endif # COVCOMMITFLAG

## INFO: RUN_FIND_NEW_DEFECTS was separated out intentionally from commit
## INFO: make target to allow for special exit codes to treat coverity
## INFO: errors as warnings if needed in precommit infra
## Args: $1=stream $2=project
define RUN_FIND_NEW_DEFECTS
	@echo "#- $0($1,$2)"
	@echo "Analyzing newly submitted snapshots for new defects"
	@target=$(subst /,_,$@); stream=$1; project=$2;         \
	 echo "$(CMD_GET_NEW_DEFECTS) --stream $$stream --project $$project";       \
	 out=$$($(CMD_GET_NEW_DEFECTS) --stream $$stream --project $$project 2>&1); \
	 ec_query=$$?;                                           \
	 echo "DBG: New defect exit code   : $$ec_query";        \
	 echo "DBG: New defect query output: $$out";             \
	 if [ "$$ec_query" == "$(EXIT_CODE_NEW_DEFECT)" ]; then  \
	    url=$$(echo $$out | fmt -1 | grep "$(COVURL)");      \
	    echo "";                                             \
	    echo "------------------------------------------";   \
	    echo "ERROR: New Coverity defect in precommit run";  \
	    echo "";                                             \
	    echo "Source Directory: $(BUILD_BASE)";              \
	    echo "Branch          : $(PRECO_BRANCH)";            \
	    echo "Target          : $$target";                   \
	    echo "Date            : $(CURDATE)";                 \
	    echo "Time            : $(CURTIME)";                 \
	    echo "User            : $(PRECO_USER)";              \
	    echo "See Triage URL  : $$url";                      \
	    echo "Resolution Twiki: $(PRECO_TWIKI)";             \
	    echo "------------------------------------------";   \
	    sleep 10; sync;                                      \
	    kill -ALRM $(PPID);                                  \
	 fi;                                                     \
	 if [ "$$ec_query" != "0" ]; then                        \
	    echo "ERROR: New defect webapi error";               \
	    exit $$ec_query;                                     \
	 fi
endef # RUN_FIND_NEW_DEFECTS

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
ifneq ($(PRECO_COVERITY_BASELINE_BUILD),)
	$(MAKE) -C $(SRCBASE)/wl/linux clean
endif # PRECO_COVERITY_BASELINE_BUILD
	$(COVBUILD_CMD) $(MAKE) -C $(SRCBASE)/wl/linux SHOWWLCONF=1 $@
	@date +"[%D %T] end   cov-build for $@ =================="
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) -j $(THREAD_COUNT) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-analyze
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-commit
ifeq ($(PRECO_COVERITY_BASELINE_BUILD),)
	$(call RUN_FIND_NEW_DEFECTS,$(COVSTREAM),$(COVPROJECT))
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-commit-clean
endif # PRECO_COVERITY_BASELINE_BUILD
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
ifneq ($(PRECO_COVERITY_BASELINE_BUILD),)
#	$(MAKE) -C $(SRCBASE)/dongle/rte/wl clean
	rm -rf $(SRCBASE)/dongle/rte/wl/builds/$(@D)
endif # PRECO_COVERITY_BASELINE_BUILD
	$(COVBUILD_CMD) $(MAKE) -C $(SRCBASE)/dongle/rte/wl $@
	@date +"[%D %T] end   cov-build for $@ =================="
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) -j $(THREAD_COUNT) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-analyze
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-commit
ifeq ($(PRECO_COVERITY_BASELINE_BUILD),)
	$(call RUN_FIND_NEW_DEFECTS,$(COVSTREAM),$(COVPROJECT))
	$(NOOP) $(MAKE) -f $(PRECO_COVERITY_MK) \
		BLDTARGET=$(subst /,_,$@) \
		COVTARGET=$@ \
		cov-commit-clean
endif # PRECO_COVERITY_BASELINE_BUILD
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Common Steps for all targets
#

# Coverity analyze pseudo target. This is called after cov-build
# and needs BLDTARGET to be set to a valid coverity build target
cov-analyze: $(COVANALYZE_THREADS)

# Parallel analysis steps with different checkers
#disabled# $(COVANALYZE_THREADS): $(subst _,/,$(BLDTARGET))
$(COVANALYZE_THREADS): THREAD=$(subst -,,$(subst $(BLDTARGET),,$(subst cov-analyze,,$@)))
$(COVANALYZE_THREADS):
	@echo "[$(THREAD)] Running $@";
	$(call RUN_COVERITY_ANALYZE, \
		$(COVTMPDIR), \
		$(COVCHECKERS_$(THREAD):%=-en %), \
		$(COVOUTDIR_$(THREAD)))

# Commit the recent precommit run to database
cov-commit: cov-commit-prep
	@$(MARKSTART)
	$(call RUN_COVERITY_COMMIT,$(BLDTARGET))
ifeq ($(PRECO_COVERITY_BASELINE_BUILD),)
	@echo "Sync up triaged defects from baseline to current stream"
	$(CMD_MIGRATE_DEFECTS)
endif # PRECO_COVERITY_BASELINE_BUILD

# After commit is done, clean up temporary streams and projects in coverity db
cov-commit-clean:
ifeq ($(PRECO_COVERITY_BASELINE_BUILD),)
	@echo "Disabled: Removing unused snapshots now"
	#disabled#$(CMD_RM_STREAM_SNAPSHOTS)
	@echo "Disabled: Removing unused stream now"
	#disabled#$(CMD_RM_STREAM)
	@echo "Disabled: Removing unused project now"
	#disabled#$(CMD_RM_PROJECT)
endif # PRECO_COVERITY_BASELINE_BUILD
	@$(MARKEND)

# Prereqs for commit step
cov-commit-prep:
ifeq ($(PRECO_COVERITY_BASELINE_BUILD),)
	@$(MARKSTART)
	@echo "Creating new stream $(COVSTREAM)"
	$(CMD_CREATE_STREAM)
	@echo "Creating new project $(COVPROJECT)"
	$(CMD_CREATE_PROJECT)
	@$(MARKEND)
endif # PRECO_COVERITY_BASELINE_BUILD

.PHONY: all cov-analyze cov-commit cov-commit-prep $(COVANALYZE_THREADS)
