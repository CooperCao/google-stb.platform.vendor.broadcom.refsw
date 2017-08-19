#
# Coverity Rules to build coverity targets specified in platform
# specific coverity brand makefiles.
#
# WARN: THIS IS A CENTRALIZED MAKEFILE AND EVEN IF IT EXISTS ON
# WARN: A BRANCH, COVERITY BUILD PROCESS STILL GETS IT FROM TRUNK
#
# WARN: SO BRANCH COPY OF coverity-rules.mk IS NOT USED
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id: coverity-rules.mk 356607 2012-09-13 00:35:35Z $
#
# NOTE: Do not update this makefile or add any new targets. Contact
# NOTE: prakashd for any questions
# NOTE: Run "make -f <thisfile> help" see available targets
#

# Once a week (on Sunday), we can run additional coverity targets
ifeq ($(strip $(shell date '+%a')),Sun)
  WEEKLYBLD=1
endif # DAY

export VCTOOL=svn
UNAME      := $(shell uname -s)
NULL       := /dev/null
CURTIME    := $(shell date '+%Y%m%d_%H%M')
CURDATE    := $(shell date '+%Y%m%d')
# Dummy timestamp embedded in dummy firmware image header file
DNGLTIME   := $(shell date '+%Y/%m/%d %H:%M:%S')
CURUSER    := $(strip $(notdir $(subst \,/,$(shell whoami))))
ifneq ($(filter TOT NIGHTLY HEAD TRUNK,$(TAG)),)
  TAG      :=
endif
TOOLSDIR   := /projects/hnd/tools
SVTZ       ?= $(shell date '+%z')
SVNCMD     ?= svn --non-interactive
COVANALYZE_THREADS ?= auto
WORKSPACE_DIR := $(CURDIR)
HND_DEPOT_DIR := $(WORKSPACE_DIR)/hnd_depot_tools
STRIP_PATH_DIR := $(WORKSPACE_DIR)

# PREVENTVER is overriden by calling build makefile
PREVENTVER ?= 7.6.1

MYTAG    ?= $(if $(TAG),$(TAG),TRUNK)


ifeq ($(findstring coverity,$(BRAND)),)
$(error ERROR: This makefile cannot be run by non-coverity brands)
endif

# In debug mode, the coverity commands become dummy no-op operations
ifneq ($(DBG)$(DEBUG),)
  override DBG=echo
endif

SVN    ?= $(DBG) svn
TOUCH  ?= $(DBG) touch
# Set COVBUILD_PROCESSES only for local builds, not for nightly or precommit builds
#COVBUILD_PROCESSES  ?= 4
COVMAKE?= $(DBG) $(MAKE) \
          $(if $(COVBUILD_PROCESSES),-j $(COVBUILD_PROCESSES))

INSTALL?= $(DBG) install

MARKSTART   = date +"[%D %T]   start: $@"
MARKEND     = date +"[%D %T]   end  : $@"

empty      :=
space      := $(empty) $(empty)
comma      := $(empty),$(empty)

vinfo      := $(if $(TAG),$(subst _,$(space),$(TAG)),$(shell date '+D11 REL %Y %m %d'))

maj        := $(word 3,$(vinfo))
min        := $(word 4,$(vinfo))
rcnum      := $(word 5,$(vinfo))
rcnum      := $(patsubst RC%,%,$(rcnum))
ifeq ($(rcnum),)
  rcnum    :=0
endif
incr       :=$(word 6,$(vinfo))
ifeq ($(incr),)
  incr     := $(shell date '+%H%M')
endif
RELNUM     := $(maj).$(min).$(rcnum).$(incr)

SVNROOT    ?= http://svn.sj.broadcom.com/svn/wlansvn/proj
GCLIENT_SVNROOT ?= http://svn.sj.broadcom.com/svn/wlansvn/components/deps
HND_DEPOT_TOOLS ?= http://svn.sj.broadcom.com/svn/wlansvn/groups/software/tools/hnd_depot_tools

# svn co -r <svnrev> captured here. It can contain svn rev number
# or checkout cutoff date/time
SVNREV     ?= $(SVNCUTOFF)

# Non-existant TAG means it is trunk
ifeq ($(TAG),)
    REPO_URL  ?= $(if $(SVNREV),-r $(SVNREV)) $(SVNROOT)/trunk/$(TAG)
    GCLIENT_REPO_URL  := $(GCLIENT_SVNROOT)/trunk/$(TAG)
endif

# Tags are group with tagname prefix as dirname
ifneq ($(findstring _REL_,$(TAG)),)
    TAG_PREFIX := $(firstword $(subst _,$(space),$(TAG)))
    REPO_URL  ?= $(if $(SVNREV),-r $(SVNREV)) $(SVNROOT)/tags/$(TAG_PREFIX)/$(TAG)
    GCLIENT_REPO_URL  := $(GCLIENT_SVNROOT)/tags/$(TAG_PREFIX)/$(TAG)
endif

# Branches don't have prefix
ifneq ($(findstring _BRANCH_,$(TAG))$(findstring _TWIG_,$(TAG)),)
    REPO_URL  ?= $(if $(SVNREV),-r $(SVNREV)) $(SVNROOT)/branches/$(TAG)
    GCLIENT_REPO_URL  ?= $(GCLIENT_SVNROOT)/branches/$(TAG)
endif

COVADMIN            ?= hwnbuild

## Windows coverity run specific settings
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
  COV-WINBUILD        := 1
  AUTH                ?= Z:/home/$(COVADMIN)/.restricted/passwd
  COV_PWD             ?= $(shell cat $(AUTH))
  HNDSVN              := sparse.bat
  HNDSVN_GCLIENT      := python $(HND_DEPOT_DIR)/hnd_gclient.py
  GALLERY             := Z:/projects/hnd_software/gallery
  HNDSVN_BOMS         ?= hndrte.sparse hndrte-dongle-wl.sparse wl-src.sparse wl-build.sparse
  HNDSVN_BOMS_GCLIENT ?= hndrte-dongle-wl wl-build

  IS64                ?= $(shell systeminfo | grep -c x64)
  ifeq  ($(IS64), 0)
    PREVENTDIR          ?= Z:$(TOOLSDIR)/win/Coverity/cov-sa-win32-$(PREVENTVER)
  else
    PREVENTDIR          ?= Z:$(TOOLSDIR)/win/Coverity/cov-sa-win64-$(PREVENTVER)
  endif

  EXE                 ?= .exe
  TEMP                := c:/temp
  CURARCH              = $(if $(findstring _x86,$@),x86,$(if $(findstring _x64,$@),x64,$(if $(findstring _amd64,$@),amd64)))
  export WIND_HOST    ?= C:/tools/Tornado2.1
  export WIND_HOST_TYPE:= x86-win64
  export WIND_BASE    := $(subst /,\,$(shell cygpath -w $(shell pwd))/src/vxWorks)
  export WIND_HOST    := $(subst /,\,$(shell cygpath -w $(WIND_HOST)))
  export PATH         := /cygdrive/c/tools/win32/bin:$(PATH)
  STRIP_PATH_DIR      := $(subst /cygdrive/c,,$(WORKSPACE_DIR))
endif # CYGWIN

## Linux coverity run specific settings
ifeq ($(findstring Linux,$(UNAME)),Linux)
  TEMP                := /tmp
  COV-LINUXBUILD      := 1
  AUTH                ?= /home/$(COVADMIN)/.restricted/passwd
  COV_PWD             ?= $(shell cat $(AUTH))
  HNDSVN              := sparse.sh
  HNDSVN_GCLIENT      := python $(HND_DEPOT_DIR)/hnd_gclient.py
  GALLERY             := /projects/hnd_software/gallery
  ifeq ($(findstring x86_64,$(shell uname -i)),x86_64)
    COV-LX64BUILD     := 1
    PREVENTDIR        ?= $(TOOLSDIR)/linux/Coverity/cov-sa-linux64-$(PREVENTVER)
    HNDSVN_BOMS       ?= hndrte.sparse hndrte-dongle-wl.sparse wl-src.sparse
    HNDSVN_BOMS_GCLIENT ?= hndrte-dongle-wl wl-build dhd

    # Cross compile 32bit wl/dhd and wl/dhd utils on a 64bit system
    32BIT             := -32 ARCH=i386 32ON64=1
  else  # Linux
    COV-LX32BUILD     := 1
    PREVENTDIR        ?= $(TOOLSDIR)/linux/Coverity/cov-sa-linux32-$(PREVENTVER)
    HNDSVN_BOMS       ?= hndrte.sparse hndrte-dongle-wl.sparse wl-src.sparse cxnnapi.sparse
    HNDSVN_BOMS_GCLIENT ?= hndrte-dongle-wl wl-build dhd
    32BIT             :=
  endif # uname -i
  LINUX_WL_OPTS       := LINUXVER=3.11.1-200.fc19.i686.PAE
  LINUX_WL_OPTS       += CROSS_COMPILE=/tools/bin/
  # Show WL flag and gcc options in the logs
  LINUX_WL_OPTS       += SHOWWLCONF=1

  # To be enabled after coverity targets are fully refreshed
  #LINUX_WL_OPTS       := LINUXVER=2.6.38.6-26.rc1.fc15.i686.PAE
  #LINUX_WL_OPTS       += GCCVER=4.8.2
  #LINUX_WL_OPTS       += CROSS_COMPILE=/tools/bin/
  # Show WL flag and gcc options in the logs
  #LINUX_WL_OPTS       += SHOWWLCONF=1

  LINUX_DHD_OPTS      := LINUXVER=3.11.1-200.fc19.x86_64
  LINUX_DHD_OPTS      += CROSS_COMPILE=/tools/bin/
  LINUX_DHD_OPTS      += TARGET_OS=fc19
  LINUX_DHD_OPTS      += TARGET_ARCH=x86_64

  # Android ICS Panda target for Android and cfg80211 support
  LINUX_DHD_OPTS_ANDROID_ARM := LINUXVER=3.2.0-panda
  LINUX_DHD_OPTS_ANDROID_ARM += CROSS_COMPILE=arm-eabi-
  LINUX_DHD_OPTS_ANDROID_ARM += ARCH=arm
  LINUX_DHD_OPTS_ANDROID_ARM += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin:$(PATH)"

  # Android ICS Panda target for Android and cfg80211 support
  LINUX_DHD_OPTS_ANDROID_BRIX := OEM_ANDROID=1
  LINUX_DHD_OPTS_ANDROID_BRIX += LINUXVER=3.10.11-aia1+-androidx86
  LINUX_DHD_OPTS_ANDROID_BRIX += CROSS_COMPILE=x86_64-linux-
  LINUX_DHD_OPTS_ANDROID_BRIX += ARCH=x86
  LINUX_DHD_OPTS_ANDROID_BRIX += PATH="/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin:$(PATH)"

ifneq ($(filter DHD_BRANCH_1_359,$(TAG)),)
  LINUX_DHD_OPTS      := LINUXVER=3.11.1-200.fc19.x86_64
  LINUX_DHD_OPTS      += CROSS_COMPILE=/tools/bin/
  LINUX_DHD_OPTS      += TARGET_OS=fc19
  LINUX_DHD_OPTS      += TARGET_ARCH=x86_64
endif # DHD_BRANCH_1_359

ifneq ($(filter DHD_BRANCH_1_363,$(TAG)),)
  LINUX_DHD_OPTS      := LINUXVER=3.11.1-200.fc19.x86_64
  LINUX_DHD_OPTS      += CROSS_COMPILE=/tools/bin/
  LINUX_DHD_OPTS      += TARGET_OS=fc19
  LINUX_DHD_OPTS      += TARGET_ARCH=x86_64

  LINUX_DHD_OPTS_ANDROID_ARM :=
  LINUX_DHD_OPTS_ANDROID_ARM += LINUXVER=3.10.20-gcc264a6-androidx86
  LINUX_DHD_OPTS_ANDROID_ARM += CROSS_COMPILE=x86_64-linux-
  LINUX_DHD_OPTS_ANDROID_ARM += PATH="/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin:$(PATH)"
endif # DHD_BRANCH_1_363

ifneq ($(filter DHD_BRANCH_1_579,$(TAG)),)
  LINUX_DHD_OPTS      := LINUXVER=3.11.1-200.fc19.x86_64
  LINUX_DHD_OPTS      += CROSS_COMPILE=/tools/bin/
  LINUX_DHD_OPTS      += TARGET_OS=fc19
  LINUX_DHD_OPTS      += TARGET_ARCH=x86_64

  LINUX_DHD_OPTS_ANDROID_ARM :=
  LINUX_DHD_OPTS_ANDROID_ARM += LINUXVER=3.2.0-panda
  LINUX_DHD_OPTS_ANDROID_ARM += CROSS_COMPILE=arm-eabi- ARCH=arm
  LINUX_DHD_OPTS_ANDROID_ARM += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin:$(PATH)"

  # Android ICS Panda target for Android and cfg80211 support
  LINUX_DHD_OPTS_ANDROID_BRIX := OEM_ANDROID=1
  LINUX_DHD_OPTS_ANDROID_BRIX += LINUXVER=3.10.20-gcc264a6-androidx86
  LINUX_DHD_OPTS_ANDROID_BRIX += CROSS_COMPILE=x86_64-linux-
  LINUX_DHD_OPTS_ANDROID_BRIX += ARCH=x86
  LINUX_DHD_OPTS_ANDROID_BRIX += PATH="/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin:$(PATH)"
endif # DHD_BRANCH_1_579

ifneq ($(filter BISON06T_BRANCH_7_45,$(TAG)),)
  LINUX_WL_OPTS       := LINUXVER=2.6.38.6-26.rc1.fc15.i686.PAE
  LINUX_WL_OPTS       += CROSS_COMPILE=/tools/bin/
  LINUX_WL_OPTS       += GCCVER=4.6.0-32
  # Show WL flag and gcc options in the logs
  LINUX_WL_OPTS       += SHOWWLCONF=1
endif # BISON06T_BRANCH_7_45

endif # Linux

## MacOS coverity run specific settings
ifeq ($(findstring Darwin,$(UNAME)),Darwin)
  COV-MACBUILD                := 1
  SW_VERS := $(shell sw_vers -productVersion)
  ifeq ($(findstring 10.8,$(SW_VERS)),10.8)
    COV-MACBUILD-10_8 := 1
  endif
  ifeq ($(findstring 10.9,$(SW_VERS)),10.9)
    COV-MACBUILD-10_9 := 1
  endif
  ifeq ($(findstring 10.10,$(SW_VERS)),10.10)
    COV-MACBUILD-10_10 := 1
  endif
  ifeq ($(findstring 10.11,$(SW_VERS)),10.11)
    COV-MACBUILD-10_11 := 1
  endif
  ifeq ($(findstring 10.12,$(SW_VERS)),10.12)
    COV-MACBUILD-10_12 := 1
  endif
  HNDSVN                      := sparse.sh
  HNDSVN_BOMS                 ?= wl-src.sparse
  HNDSVN_GCLIENT              := python $(HND_DEPOT_DIR)/hnd_gclient.py
  HNDSVN_BOMS_GCLIENT         ?= wl-build dhd
  TEMP                        := /tmp
  PREVENTDIR                  ?= $(TOOLSDIR)/macos/Coverity/cov-sa-macosx-7.6.1
  AUTH                        ?= /home/$(COVADMIN)/.restricted/passwd
  COV_PWD                     ?= $(shell cat $(AUTH))
endif # Darwin

# WebAPI scripts and config files
WEBAPIDIR           ?= $(GALLERY)/src/tools/coverity/webapi
WEBAPIDIR_CONF      ?= $(WEBAPIDIR)/config

## Global Coverity variables
export COVCONFIG    ?= $(PREVENTDIR)/config/coverity_config.xml
export COVSTREAM    ?= $(shell echo $(MYTAG)__$(subst /,_,$@) | cut -c1-256)
export COVERRLVL    ?= 90
COVTMPDIR_KEEP      := true
export COV_TEMP     := $(shell mktemp -u XXXXXXXXXX)
export COVIMED      ?= $(shell echo imed_cov$(subst .,,$(PREVENTVER))_$(MYTAG)_$(COV_TEMP)_$(CURUSER)_$(subst /,_,$@) | cut -c1-240)
export COVTMPDIR    ?= $(TEMP)/coverity/$(COVIMED)
export COVBUILD     ?= $(DBG) $(PREVENTDIR)/bin/cov-build$(EXE) --dir $(COVTMPDIR) \
                       --return-emit-failures --parse-error-threshold \
                       $(COVERRLVL) \
                       --config $(COVCONFIG)

export COVPWCONF    ?= $(PREVENTDIR)/config/parse_warnings.conf
export COVCHECKERS  ?= --security \
                       --concurrency \
                       --enable-parse-warnings \
                       --enable-constraint-fpp \
                       --enable missing_lock \
                       --enable STACK_USE \
                       --enable ASSERT_SIDE_EFFECT \
                       --enable INTEGER_OVERFLOW \
                       --enable DIVIDE_BY_ZERO \
                       --disable UNUSED_VALUE \
                       --checker-option DEADCODE:no_dead_default:true \
                       --parse-warnings-config $(COVPWCONF)

export COVSTREAM_ALLOC_VOID ?= $(shell echo $(MYTAG)__alloc_void_$(subst /,_,$@) | cut -c1-256)
export COVSTREAM_BANNED_API ?= $(shell echo $(MYTAG)__banned_api_$(subst /,_,$@) | cut -c1-256)

# NOTE: If Checker Options/customizations are needed, then -j 4 needs to be
# NOTE: removed
#disabled#             --checker-option DEADCODE:no_dead_default:true

# COVCOMMIT_ARGS     = $(shell cat $(COVTMPDIR)/c/output/commit-args.txt)

COVCOMMIT_ARGS       ?= --dir $(COVTMPDIR)

# Skip committing xrefs to diminish DB bloat
COVCOMMIT_EXTRA_ARGS ?= --noxrefs

ifeq ($(PREVENTVER), 5.3.0)
  export COVSERVER           ?= "cov-sj1-08.sj.broadcom.com"
  export COVANALYZE_THREADS  ?= 4

  COVCOMMIT_EXTRA_ARGS       ?= -xo $(COVTMPDIR)/c/output2 \
                                -xo $(COVTMPDIR)/c/output3 \
                                -xo $(COVTMPDIR)/c/output4
else
  export COVSERVER           ?= "wlansw-cov75.sj.broadcom.com"

endif # PREVENTVER

export COVANALYZE   ?= $(DBG) $(PREVENTDIR)/bin/cov-analyze$(EXE) \
		       $(COVCHECKERS) \
		       $(if $(COVANALYZE_THREADS),-j $(COVANALYZE_THREADS)) \
		       --strip-path $(STRIP_PATH_DIR)/$(BUILD_TREE) $(if $(COV-MACBUILD),--strip-path $(subst /private/tmp,/tmp,$(STRIP_PATH_DIR))/$(BUILD_TREE))

export COVALLOCVOID   ?= $(DBG) $(PREVENTDIR)/bin/alloc_void_returns$(EXE) \
		       $(if $(COVANALYZE_THREADS),-j $(COVANALYZE_THREADS)) \
		       --strip-path $(STRIP_PATH_DIR)/$(BUILD_TREE) $(if $(COV-MACBUILD),--strip-path $(subst /private/tmp,/tmp,$(STRIP_PATH_DIR))/$(BUILD_TREE))

export COVBANNEDAPI   ?= $(DBG) $(PREVENTDIR)/bin/banned_apis$(EXE) \
		       $(if $(COVANALYZE_THREADS),-j $(COVANALYZE_THREADS)) \
		       --strip-path $(STRIP_PATH_DIR)/$(BUILD_TREE) $(if $(COV-MACBUILD),--strip-path $(subst /private/tmp,/tmp,$(STRIP_PATH_DIR))/$(BUILD_TREE))

# Coverity description comes from a shared coverity targets config file
#  coverity-targets-info.mk file
COVTARGETINFO        = $(if $($@),$($@),$@)
COVTARGETDESC        = $(MYTAG) $(COVTARGETINFO) $(CURTIME)
COVCOMMIT           ?= $(DBG) $(PREVENTDIR)/bin/cov-commit-defects$(EXE) \
		       --version "$(RELNUM)" \
                       --host "$(COVSERVER)" --port 8080 \
                       --user $(COVADMIN) --password $(COV_PWD)
export COVDMODELDIR ?= $(PREVENTDIR)/../prevent-derived-models

ifeq ($(CURUSER),hwnbuild)
  COVCOMMITFLAG     := true
else  # CURUSER
  COVCOMMIT         += --private
endif # CURUSER

export PREVENTDIR
export COVCOMMIT
export BRAND

## Coverity specific user defined Function definitions
## Function names tell what they do

## SVN checkout macro
## Args: $1=SVN module to checkout
define SVN_CHECKOUT_SPARSE
	@echo "#- $0 $1"
	$(HNDSVN) --version
	$(HNDSVN) checkout \
		$(if $(SVNCUTOFF),-r "{$(SVNCUTOFF) $(SVTZ)}") \
		$(REPO_URL)/$(firstword $(1)) .
	-@for bom in $(filter-out $(firstword $(1)),$(1)); do \
		echo $(HNDSVN) append \
			$(if $(SVNCUTOFF),-r "{$(SVNCUTOFF) $(SVTZ)}") \
			./$$bom; \
		$(HNDSVN) append \
			$(if $(SVNCUTOFF),-r "{$(SVNCUTOFF) $(SVTZ)}") \
			./$$bom; \
	done
	$(MAKE) -C src/include
	@echo $(REPO_URL) > _SUBVERSION_BUILD
	@$(SVNCMD) info | grep "^Revision: " | \
		awk -F': ' '{printf "%s",$$NF}' > _SUBVERSION_REVISION
	# Record svn info for easier tracking of svn rev per build brand
	@echo "-------------------------------"
	$(SVNCMD) info
	@echo "-------------------------------"
	mkdir -pv misc/boms
	-mv $$(find *.sparse $(HNDSVN_BOMS:%=-name % -prune -o) -print) misc/boms
endef  # SVN_CHECKOUT_SPARSE

## SVN checkout macro
## Args: $1=SVN module to checkout
define SVN_CHECKOUT_GCLIENT
	@echo "#- $0 $1"
	$(SVNCMD) export --force \
		$(if $(SVNCUTOFF),-r "{$(SVNCUTOFF) $(SVTZ)}") \
		$(HND_DEPOT_TOOLS) $(HND_DEPOT_DIR)
	$(HNDSVN_GCLIENT) --version
	-@export SAVEDIR=$(CURDIR); \
	for bom in $(1) ; do \
		mkdir -p $$SAVEDIR/$$bom; \
		cd $$SAVEDIR/$$bom; \
		echo $(HNDSVN_GCLIENT) config $(GCLIENT_REPO_URL)/$$bom; \
		$(HNDSVN_GCLIENT) config $(GCLIENT_REPO_URL)/$$bom; \
		echo $(HNDSVN_GCLIENT) sync $(if $(SVNCUTOFF),--revision "{$(SVNCUTOFF) $(SVTZ)}" -d "{$(SVNCUTOFF) $(SVTZ)}" --trans); \
		$(HNDSVN_GCLIENT) sync $(if $(SVNCUTOFF),--revision "{$(SVNCUTOFF) $(SVTZ)}" -d "{$(SVNCUTOFF) $(SVTZ)}" --trans); \
		$(MAKE) -C src/include ; \
		echo $(GCLIENT_REPO_URL) > ../_SUBVERSION_BUILD ; \
		$(SVNCMD) info src | grep "^Revision: " | \
		awk -F': ' '{printf "%s",$$NF}' > ../_SUBVERSION_REVISION ; \
	done
	@echo "-------------------------------"
endef  # SVN_CHECKOUT_GCLIENT

## SVN checkout macro
## Args: $1=SVN module to checkout
define SVN_CHECKOUT_GCLIENT_MAIN_ROUTER
	@echo "#- $0 $1"
	$(SVNCMD) export --force \
		$(if $(SVNCUTOFF),-r "{$(SVNCUTOFF) $(SVTZ)}") \
		$(HND_DEPOT_TOOLS) $(HND_DEPOT_DIR)
	$(HNDSVN_GCLIENT) --version
	-@export SAVEDIR=$(CURDIR); \
	for bom in $(1) ; do \
		mkdir -p $$SAVEDIR/$$bom; \
		cd $$SAVEDIR/$$bom; \
		echo $(HNDSVN_GCLIENT) config $(GCLIENT_REPO_URL)/$$bom; \
		$(HNDSVN_GCLIENT) config $(GCLIENT_REPO_URL)/$$bom; \
		echo $(HNDSVN_GCLIENT) sync $(if $(SVNCUTOFF),--revision "{$(SVNCUTOFF) $(SVTZ)}" -d "{$(SVNCUTOFF) $(SVTZ)}" --trans); \
		$(HNDSVN_GCLIENT) sync $(if $(SVNCUTOFF),--revision "{$(SVNCUTOFF) $(SVTZ)}" -d "{$(SVNCUTOFF) $(SVTZ)}" --trans); \
		$(MAKE) -C sys/src/include ; \
		$(MAKE) -C main/src/include ; \
		echo $(GCLIENT_REPO_URL) > ../_SUBVERSION_BUILD ; \
		$(SVNCMD) info sys/src | grep "^Revision: " | \
		awk -F': ' '{printf "%s",$$NF}' > ../_SUBVERSION_REVISION ; \
	done
	@echo "-------------------------------"
endef  # SVN_CHECKOUT_GCLIENT_MAIN_ROUTER

## Check exit status from coverity build
## Args: none
CHECK_COVBUILD_STATUS=\
	cov_build_ec=$$?; \
	$(INSTALL) -d misc/cov-$(notdir $(COVTMPDIR))/; \
	$(INSTALL) -pv $(COVTMPDIR)/build-log.txt \
	    misc/cov-$(notdir $(COVTMPDIR))/; \
	if [ "$$cov_build_ec" != "0" ]; then \
	   echo "ERROR: '$@' cov-build failed (exit code: $$cov_build_ec). Verify log at:"; \
	   echo "ERROR: misc/cov-$(notdir $(COVTMPDIR))/build-log.txt"; \
	   echo "ERROR: Skipping analysis and commit to coverity db"; \
	   $(TOUCH) $(COVTMPDIR)/status-cov-build-failed \
	       misc/cov-$(notdir $(COVTMPDIR))/status-cov-build-failed; \
	   echo "rm -rf $(COVTMPDIR)"; rm -rf $(COVTMPDIR); \
	else \
	   echo "INFO: '$@' cov-build exit code: $$cov_build_ec"; \
	   echo "INFO: Coverity build step done, analyze will run next"; \
	   $(TOUCH) $(COVTMPDIR)/status-cov-build-done \
	       misc/cov-$(notdir $(COVTMPDIR))/status-cov-build-done; \
	fi

## derived model is used to provide representation of functions which we
## don't want to analyze, yet we need them to compile and build the code
## Args: $1 = derived model file name
define RUN_COVERITY_ANALYZE
	@date +"[%D %T] start $0 for $@"
	@if [ -f "$(COVTMPDIR)/status-cov-build-done" ]; then \
	    echo "#- $0"; \
	    echo "$(COVANALYZE) --dir $(COVTMPDIR) \
	        $(if $1,--derived-model-file $(COVDMODELDIR)/$1)"; \
	    $(COVANALYZE) --dir $(COVTMPDIR) \
	        $(if $1,--derived-model-file $(COVDMODELDIR)/$1); \
	    cov_analyze_ec=$$?; \
	    if [ "$$cov_analyze_ec" != "0" ]; then \
	       echo "ERROR: '$@' cov-analyze failed (exit code: $$cov_analyze_ec)"; \
	       echo "ERROR: Skipping commit to coverity db"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-analyze-failed \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-analyze-failed; \
	    else \
	       echo "INFO: '$@' cov-analyze exit code: $$cov_analyze_ec"; \
	       echo "INFO: Coverity analyze step done, commit will run next"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-analyze-done \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-analyze-done; \
	    fi; \
	fi
	@date +"[%D %T] end $0 for $@"
endef # RUN_COVERITY_ANALYZE

## derived model is used to provide representation of functions which we
## don't want to analyze, yet we need them to compile and build the code
## Args: $1 = derived model file name
define RUN_COVERITY_ALLOC_VOID
	@date +"[%D %T] start $0 for $@"
	@if [ -f "$(COVTMPDIR)/status-cov-build-done" ]; then \
	    echo "#- $0"; \
	    echo "$(COVALLOCVOID) --dir $(COVTMPDIR) \
	        $(if $1,--derived-model-file $(COVDMODELDIR)/$1)"; \
	    $(COVALLOCVOID) --dir $(COVTMPDIR) \
	        $(if $1,--derived-model-file $(COVDMODELDIR)/$1); \
	    cov_alloc_void_ec=$$?; \
	    if [ "$$cov_alloc_void_ec" != "0" ]; then \
	       echo "ERROR: '$@' cov_alloc_void failed (exit code: $$cov_alloc_void_ec)"; \
	       echo "ERROR: Skipping commit to coverity db"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-alloc-void-failed \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-alloc-void-failed; \
	    else \
	       echo "INFO: '$@' cov-alloc-void exit code: $$cov_alloc_void_ec"; \
	       echo "INFO: Coverity alloc void step done, commit will run next"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-alloc-void-done \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-alloc-void-done; \
	    fi; \
	fi
	@date +"[%D %T] end $0 for $@"
endef # RUN_COVERITY_ALLOC_VOID

define RUN_COVERITY_BANNED_API
	@date +"[%D %T] start $0 for $@"
	@if [ -f "$(COVTMPDIR)/status-cov-build-done" ]; then \
	    echo "#- $0"; \
	    echo "$(COVBANNEDAPI) --dir $(COVTMPDIR) \
	        $(if $1,--derived-model-file $(COVDMODELDIR)/$1)"; \
	    $(COVBANNEDAPI) --dir $(COVTMPDIR) \
	        $(if $1,--derived-model-file $(COVDMODELDIR)/$1); \
	    cov_banned_api_ec=$$?; \
	    if [ "$$cov_banned_api_ec" != "0" ]; then \
	       echo "ERROR: '$@' cov_banned_api failed (exit code: $$cov_banned_api_ec)"; \
	       echo "ERROR: Skipping commit to coverity db"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-banned_api-failed \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-banned-api-failed; \
	    else \
	       echo "INFO: '$@' cov-banned_api exit code: $$cov_banned_api_ec"; \
	       echo "INFO: Coverity banned api step done, commit will run next"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-banned-api-done \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-banned-api-done; \
	    fi; \
	fi
	@date +"[%D %T] end $0 for $@"
endef # RUN_COVERITY_BANNED_API

## Commit the coverity analyzed results. For non-hwnbuild users, the
## committed run appears as private run
## Args: $1=target $2=description suffix
ifeq ($(COVCOMMITFLAG),true)
define RUN_COVERITY_COMMIT
	@date +"[%D %T] start $0 for $@"
	@if [ -f "$(COVTMPDIR)/status-cov-analyze-done" ]; then \
	    desc=`echo "$(COVTARGETDESC)" | sed -e 's/[[:space:]]+/ /g'`; \
	    echo "#- $0"; \
	    echo "$(COVCOMMIT) $(COVCOMMIT_ARGS) \
                $(COVCOMMIT_EXTRA_ARGS) \
	        $(if $(COVSTREAM),--stream $(COVSTREAM)) \
	        --description "$(COVTARGETDESC) $2" \
	        --target \"$(subst /,_,$1)\""; \
	    $(COVCOMMIT) $(COVCOMMIT_ARGS) \
                $(COVCOMMIT_EXTRA_ARGS) \
	        $(if $(COVSTREAM),--stream $(COVSTREAM)) \
	        --description "$$desc $2" \
	        --target "$(subst /,_,$1)"; \
	    cov_commit_ec=$$?; \
	    if [ "$$cov_commit_ec" != "0" ]; then \
	       echo "ERROR: '$@' cov-commit failed (exit code: $$cov_commit_ec)"; \
	       echo "ERROR: Skipping commit to coverity db"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-commit-failed \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-commit-failed; \
	    else \
	       echo "INFO: '$@' cov-commit exit code: $$cov_commit_ec"; \
	       echo "INFO: Coverity commit step done"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-commit-done \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-commit-done; \
	    fi; \
	fi
	@date +"[%D %T] end $0 for $@"
	@echo "==================================================="
	@echo ""
endef # RUN_COVERITY_COMMIT
else  # COVCOMMITFLAG
define RUN_COVERITY_COMMIT
	@echo "#- $0"
	@echo "NOTE: Skipping database commit"
	@echo "NOTE: To force commit this private analysis, you can run:"
	@echo "$(COVCOMMIT) --dir $(COVTMPDIR) \
	    $(if $(COVSTREAM),--stream PRIVATE_$(COVSTREAM)) \
	    --description \"PRIVATE $(COVTARGETDESC)\" \
	    --target \"$(subst /,_,$1)\""
	$(TOUCH) $(COVTMPDIR)/status-cov-commit-skipped \
		misc/cov-$(notdir $(COVTMPDIR))/status-cov-commit-skipped
	@echo "==================================================="
	@echo ""
endef # RUN_COVERITY_COMMIT
endif # COVCOMMITFLAG

## Create dummy rtecdc.h or rtecdc_<chiprev>.h for bmac driver builds
## Commit the coverity analyzed results. For non-hwnbuild users, the
## committed run appears as private run
## Args: $1=target $2=description suffix
ifeq ($(COVCOMMITFLAG),true)
define RUN_COVERITY_COMMIT_ALLOC_VOID
	@date +"[%D %T] start $0 for $@"
	@if [ -f "$(COVTMPDIR)/status-cov-alloc-void-done" ]; then \
	    desc=`echo "$(COVTARGETDESC)" | sed -e 's/[[:space:]]+/ /g'`; \
	    echo "#- $0"; \
	    echo "$(COVCOMMIT) $(COVCOMMIT_ARGS) \
                $(COVCOMMIT_EXTRA_ARGS) \
	        $(if $(COVSTREAM_ALLOC_VOID),--stream $(COVSTREAM_ALLOC_VOID)) \
	        --description "$(COVTARGETDESC) $2 alloc void" \
	        --target \"$(subst /,_,$1)\""; \
	    $(COVCOMMIT) $(COVCOMMIT_ARGS) \
                $(COVCOMMIT_EXTRA_ARGS) \
	        $(if $(COVSTREAM_ALLOC_VOID),--stream $(COVSTREAM_ALLOC_VOID)) \
	        --description "$$desc $2 alloc void" \
	        --target "$(subst /,_,$1)"; \
	    cov_commit_alloc_void_ec=$$?; \
	    if [ "$$cov_commit_alloc_void_ec" != "0" ]; then \
	       echo "ERROR: '$@' cov-commit-alloc-void failed (exit code: $$cov_commit_alloc_void_ec)"; \
	       echo "ERROR: Skipping commit to coverity db"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-commit-alloc-void-failed \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-commit-alloc-void-failed; \
	    else \
	       echo "INFO: '$@' cov-commit-alloc-void exit code: $$cov_commit_alloc_void_ec"; \
	       echo "INFO: Coverity commit alloc void step done"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-commit-alloc-void-done \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-commit-alloc-void-done; \
	    fi; \
	fi
	@date +"[%D %T] end $0 for $@"
	@echo "==================================================="
	@echo ""
endef # RUN_COVERITY_COMMIT_ALLOC_VOID
else  # COVCOMMITFLAG
define RUN_COVERITY_COMMIT_ALLOC_VOID
	@echo "#- $0"
	@echo "NOTE: Skipping database commit"
	@echo "NOTE: To force commit this private analysis, you can run:"
	@echo "$(COVCOMMIT) --dir $(COVTMPDIR) \
	    $(if $(COVSTREAM_ALLOC_VOID),--stream PRIVATE_$(COVSTREAM_ALLOC_VOID)) \
	    --description \"PRIVATE $(COVTARGETDESC) alloc void\" \
	    --target \"$(subst /,_,$1)\""
	$(TOUCH) $(COVTMPDIR)/status-cov-commit-alloc-void-skipped \
		misc/cov-$(notdir $(COVTMPDIR))/status-cov-commit-alloc-void-skipped
	@echo "==================================================="
	@echo ""
endef # RUN_COVERITY_COMMIT_ALLOC_VOID
endif # COVCOMMITFLAG

ifeq ($(COVCOMMITFLAG),true)
define RUN_COVERITY_COMMIT_BANNED_API
	@date +"[%D %T] start $0 for $@"
	@if [ -f "$(COVTMPDIR)/status-cov-banned-api-done" ]; then \
	    desc=`echo "$(COVTARGETDESC)" | sed -e 's/[[:space:]]+/ /g'`; \
	    echo "#- $0"; \
	    echo "$(COVCOMMIT) $(COVCOMMIT_ARGS) \
                $(COVCOMMIT_EXTRA_ARGS) \
	        $(if $(COVSTREAM_BANNED_API),--stream $(COVSTREAM_BANNED_API)) \
	        --description "$(COVTARGETDESC) $2 banned api" \
	        --target \"$(subst /,_,$1)\""; \
	    $(COVCOMMIT) $(COVCOMMIT_ARGS) \
                $(COVCOMMIT_EXTRA_ARGS) \
	        $(if $(COVSTREAM_BANNED_API),--stream $(COVSTREAM_BANNED_API)) \
	        --description "$$desc $2 banned api" \
	        --target "$(subst /,_,$1)"; \
	    cov_commit_banned_api_ec=$$?; \
	    if [ "$$cov_commit_banned_api_ec" != "0" ]; then \
	       echo "ERROR: '$@' cov-commit-banned-api failed (exit code: $$cov_commit_banned_api_ec)"; \
	       echo "ERROR: Skipping commit to coverity db"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-commit-banned-api-failed \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-commit-banned-api-failed; \
	    else \
	       echo "INFO: '$@' cov-commit-banned-api exit code: $$cov_commit_banned_api_ec"; \
	       echo "INFO: Coverity commit banned api step done"; \
	       $(TOUCH) $(COVTMPDIR)/status-cov-commit-banned-api-done \
	           misc/cov-$(notdir $(COVTMPDIR))/status-cov-commit-banned-api-done; \
	    fi; \
	fi
	@date +"[%D %T] end $0 for $@"
	@echo "==================================================="
	@echo ""
endef # RUN_COVERITY_COMMIT_BANNED_API
else  # COVCOMMITFLAG
define RUN_COVERITY_COMMIT_BANNED_API
	@echo "#- $0"
	@echo "NOTE: Skipping database commit"
	@echo "NOTE: To force commit this private analysis, you can run:"
	@echo "$(COVCOMMIT) --dir $(COVTMPDIR) \
	    $(if $(COVSTREAM_BANNED_API),--stream PRIVATE_$(COVSTREAM_BANNED_API)) \
	    --description \"PRIVATE $(COVTARGETDESC) banned api\" \
	    --target \"$(subst /,_,$1)\""
	$(TOUCH) $(COVTMPDIR)/status-cov-commit-banned-api-skipped \
		misc/cov-$(notdir $(COVTMPDIR))/status-cov-commit-banned-api-skipped
	@echo "==================================================="
	@echo ""
endef # RUN_COVERITY_COMMIT_BANNED_API
endif # COVCOMMITFLAG

define RUN_COVERITY_CLEANUP
	@date +"[%D %T] start $0 for $@"
	if [ "$(COVTMPDIR_KEEP)" != "" ]; then \
	       echo "DBG: Keeping $(COVTMPDIR)"; \
	       builddir=$$(pwd); \
	       pushd $(dir $(COVTMPDIR)); \
	       find $(notdir $(COVTMPDIR)) -print | \
	            cpio -pmudv $${builddir}/misc/imed; \
	       popd; \
	       chmod ugo+rx $${builddir}/misc/imed/$(notdir $(COVTMPDIR)); \
	       gzip -9 -r $${builddir}/misc/imed/$(notdir $(COVTMPDIR)); \
	fi; \
	echo "rm -rf $(COVTMPDIR)"; \
	rm -rf $(COVTMPDIR)
	@date +"[%D %T] end $0 for $@"
endef

define RUN_COVERITY_CUSTOM_CHECKERS
	@date +"[%D %T] start $0 for $@"
	$(call RUN_COVERITY_ALLOC_VOID)
	$(call RUN_COVERITY_COMMIT_ALLOC_VOID,$@)
	$(call RUN_COVERITY_BANNED_API)
	$(call RUN_COVERITY_COMMIT_BANNED_API,$@)
	@date +"[%D %T] end $0 for $@"
endef # RUN_COVERITY_CUSTOM_CHECKERS

## Create dummy rtecdc.h or rtecdc_<chiprev>.h for bmac driver builds
## TO-DO: Replace following with src/makefiles/preco_rules.mk to create
## TO-DO: dummy firmware images
## Args: $1=dhdname
## Args: $2=workspace
define CREATE_DUMMY_DONGLE_IMAGE
	@date +"[%D %T] start $0 for $@"
	@echo "#- $0"
	@echo "/* Dummy rtecdc.h for dhd build */"      > $(2)src/include/rtecdc.h
	@echo "/* dhd name = $1 */"                    >> $(2)src/include/rtecdc.h
	@echo "#ifdef MEMBLOCK"                        >> $(2)src/include/rtecdc.h
	@echo "#define MYMEMBLOCK MEMBLOCK"            >> $(2)src/include/rtecdc.h
	@echo "#else"                                  >> $(2)src/include/rtecdc.h
	@echo "#define MYMEMBLOCK 2048"                >> $(2)src/include/rtecdc.h
	@echo "#endif"                                 >> $(2)src/include/rtecdc.h
	@echo "char dlimagename[] = \"dummy_$@\";"     >> $(2)src/include/rtecdc.h
	@echo "char dlimagever[]  = \"$(RELNUM)\";"    >> $(2)src/include/rtecdc.h
	@echo "char dlimagedate[] = \"$(DNGLTIME)\";"  >> $(2)src/include/rtecdc.h
	@echo ""                                       >> $(2)src/include/rtecdc.h
	@echo "unsigned char dlarray[MYMEMBLOCK + 1];" >> $(2)src/include/rtecdc.h
	@echo "#undef MYMEMBLOCK"                      >> $(2)src/include/rtecdc.h
	@echo -e "\nDummy rtecdc.h contents:\n"
	@cat $(2)src/include/rtecdc.h
	# Query dbus implementation to get a list of firmware header files
	# bmac/full-dongle-dhd driver expects for the build to go through.
	@for chiprev in `grep "#include.*rtecdc_.*.h" $(2)src/shared/dbus*.c | awk '{print $$NF}' | sed -e 's/"//g' -e 's/rtecdc_//g' -e 's/\.h//g'`; do \
		cat $(2)src/include/rtecdc.h | \
			sed -e "s/dlarray/dlarray_$${chiprev}/g" \
			    -e "s/dlimagename/dlimagename_$${chiprev}/g" \
			    -e "s/dlimagever/dlimagever_$${chiprev}/g" \
			    -e "s/dlimagedate/dlimagedate_$${chiprev}/g" \
			    -e "s/dummy/dummy_$${chiprev}/g" \
			    -e "s/rtecdc.h/rtecdc_$${chiprev}.h/g" \
			> $(2)src/include/rtecdc_$${chiprev}.h; \
		echo -e "\nDummy rtecdc_$${chiprev}.h contents:\n"; \
		cat $(2)src/include/rtecdc_$${chiprev}.h; \
	done
	@date +"[%D %T] end $0 for $@"
endef #CREATE_DUMMY_DONGLE_IMAGE

# Derive current list of targets based on branch/tag name
CURRENT_TARGETS=$(ALL_COVERITY_TARGETS)

all: build_start help checkout $(CURRENT_TARGETS) build_status build_end

checkout:
ifeq ($(SRC_CHECKOUT_DISABLED),)
	@$(MARKSTART)
ifeq ($(TREES),GCLIENT)
	$(call SVN_CHECKOUT_GCLIENT,$(HNDSVN_BOMS_GCLIENT))
endif # GCLIENT
ifeq ($(TREES),SPARSE)
	$(call SVN_CHECKOUT_SPARSE,$(HNDSVN_BOMS))
endif # SPARSE
	@$(MARKEND)
endif # SRC_CHECKOUT_DISABLED

## -------------------------------------------------------------------
## All Windows coverity build targets
## -------------------------------------------------------------------
#
# WINOS and WDK_VER required for Win8+
#
$(filter build_win8_%,$(WIN_WL_TARGETS)):: WINOS=WIN8
$(filter build_win8_%,$(WIN_WL_TARGETS)):: WDK_VER=9200
$(filter build_win8x_%,$(WIN_WL_TARGETS)):: WDK_VER=9600
$(filter build_win8x_%,$(WIN_WL_TARGETS)):: WINOS=WIN8X
$(filter build_win8x_%,$(WIN_WL_TARGETS)):: VS_VER=2013
$(filter build_win8x_%,$(WIN_WL_TARGETS)):: BCMCCX=0
$(filter build_winblue_%,$(WIN_WL_TARGETS)):: WINOS=WINBLUE
$(filter build_winblue_%,$(WIN_WL_TARGETS)):: WDK_VER=9391

#
# Windows WL NIC Drivers
#
$(WIN_WL_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(WIN_WL_TARGETS): BUILD_TYPES=$(if $(USE_BUILD_TYPES),$(USE_BUILD_TYPES),free)
$(WIN_WL_TARGETS): COVERRLVL=82
$(WIN_WL_TARGETS): checkout
ifdef COV-WINBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	$(call CREATE_DUMMY_DONGLE_IMAGE,$@,$(BUILD_TREE))
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/sys/wdm BUILD_ARCHS=$(CURARCH) BUILD_TYPES=$(BUILD_TYPES) $(if $(WINOS),WINOS=$(WINOS) WIN8_WDK_VER=$(WDK_VER) WDK_VER=7600,) $(if $(VS_VER),VS_VER=$(VS_VER)) $(if $(BCMCCX),BCMCCX=$(BCMCCX)) $(subst _wl_$(CURARCH),,$@)"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/sys/wdm BUILD_ARCHS=$(CURARCH) BUILD_TYPES=$(BUILD_TYPES) $(if $(WINOS),WINOS=$(WINOS) WIN8_WDK_VER=$(WDK_VER) WDK_VER=7600,) $(if $(VS_VER),VS_VER=$(VS_VER)) $(if $(BCMCCX),BCMCCX=$(BCMCCX)) $(subst _wl_$(CURARCH),,$@); \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE)
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-WINBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-WINBUILD

#
# Windows WL NIC Drivers
#
$(WIN10_WL_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(WIN10_WL_TARGETS): BUILD_TYPES=$(if $(USE_BUILD_TYPES),$(USE_BUILD_TYPES),free)
$(WIN10_WL_TARGETS): COVERRLVL=82
$(WIN10_WL_TARGETS): WINOS=WIN10
$(WIN10_WL_TARGETS): WDK_VER=10240
$(WIN10_WL_TARGETS): VS_VER=2015
$(WIN10_WL_TARGETS): checkout
ifdef COV-WINBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	$(call CREATE_DUMMY_DONGLE_IMAGE,$@,$(BUILD_TREE))
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/sys/wdm WIN8_WDK_VER=$(WDK_VER) WIN10_WDK_VER=$(WDK_VER) VS_VER=$(VS_VER) VCPROJ=wdidriver.vcxproj WINOS=$(WINOS) BUILD_TYPES=$(BUILD_TYPES) $(subst _wl,,$@)"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/sys/wdm WIN8_WDK_VER=$(WDK_VER) WIN10_WDK_VER=$(WDK_VER) VS_VER=$(VS_VER) VCPROJ=wdidriver.vcxproj WINOS=$(WINOS) BUILD_TYPES=$(BUILD_TYPES) $(subst _wl,,$@); \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE)
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-WINBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-WINBUILD

#
# Windows Prefast Run for WL NIC Drivers
#
$(WIN_PREFAST_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(WIN_PREFAST_TARGETS):
ifdef COV-WINBUILD
	@$(MARKSTART)
	-$(COVMAKE) -C $(BUILD_TREE)src/wl/sys/wdm BUILD_ARCHS="$(BUILD_ARCHS)" BUILD_TYPES="$(BUILD_TYPES)" $@
	@$(MARKEND)
else  # COV-WINBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-WINBUILD

#
# Windows DHD Drivers
#
$(WIN_DHD_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(WIN_DHD_TARGETS): checkout
ifdef COV-WINBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	$(call CREATE_DUMMY_DONGLE_IMAGE,$@,$(BUILD_TREE))
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/wdm -f sdio.mk DNGL_IMAGE_NAME=FakeDongle DNGL_IMAGE_PATH=../../include BUILD_ARCHS=$(CURARCH)"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/wdm -f sdio.mk DNGL_IMAGE_NAME=FakeDongle DNGL_IMAGE_PATH=../../include BUILD_ARCHS=$(CURARCH); \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE)
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-WINBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-WINBUILD

#
# Windows DHD Drivers
#
$(WIN10_DHD_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(WIN10_DHD_TARGETS): WINOS=WIN10
$(WIN10_DHD_TARGETS): VS_VER=2015
$(WIN10_DHD_TARGETS): WDK_VER=10240
$(WIN10_DHD_TARGETS): BUILD_TYPES=free
$(WIN10_DHD_TARGETS): checkout
ifdef COV-WINBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	$(call CREATE_DUMMY_DONGLE_IMAGE,$@,$(BUILD_TREE))
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/wdm WINOS=$(WINOS) VS_VER=$(VS_VER) WDK_VER=$(WDK_VER) BUILD_TYPES=$(BUILD_TYPES) $(if $(findstring pcie,$@),BUS=pcie) $(if $(findstring sdio,$@),BUS=sdio) BUILD_ARCHS="x86 x64" -f win8xdriver.mk all"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/wdm WINOS=$(WINOS) VS_VER=$(VS_VER) WDK_VER=$(WDK_VER) BUILD_TYPES=$(BUILD_TYPES) $(if $(findstring pcie,$@),BUS=pcie) $(if $(findstring sdio,$@),BUS=sdio) BUILD_ARCHS="x86 x64" -f win8xdriver.mk all; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE)
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-WINBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-WINBUILD

#
# WinCE DHD Drivers
#
$(WIN_CE_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(WIN_CE_TARGETS): checkout
ifdef COV-WINBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	$(call CREATE_DUMMY_DONGLE_IMAGE,$@,$(BUILD_TREE))
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/ce TARGETS='$@' TTYPE=OPT DNGL_IMAGE_PATH=../../include ce_dhd"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/ce TARGETS="$@" TTYPE=OPT DNGL_IMAGE_PATH=../../include ce_dhd; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-WINBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-WINBUILD

#
# Win WL.EXE and DHD.EXE utils
#
$(WIN_UTILS_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(WIN_UTILS_TARGETS): CURTGT=$(if $(findstring -wl-,$@),wl,dhd)
$(WIN_UTILS_TARGETS): checkout
$(WIN_UTILS_TARGETS):
ifdef COV-WINBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/$(CURTGT)/exe RWL=0 ASD=0 \
		$(if $(findstring winxp-wl-,$@),OTHER_SOURCES=) \
		$(if $(findstring winvista-wl-,$@),SRCFILE=sources_vista.external TTYPE=OPT)"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/$(CURTGT)/exe RWL=0 ASD=0 \
		$(if $(findstring winxp-wl-,$@),OTHER_SOURCES=) \
		$(if $(findstring winvista-wl-,$@),SRCFILE=sources_vista.external TTYPE=OPT); \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-WINBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-WINBUILD

$(WIN_IHV_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(WIN_IHV_TARGETS): BUILD_CONFS=Releasewb
$(WIN_IHV_TARGETS): BUILD_ARCH="x86 x64"
$(WIN_IHV_TARGETS): checkout
ifdef COV-WINBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	export PATH="/usr/local/bin:$(PATH)"; \
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)$(WIN_IHV_DIR) -f build_vs13.mk BUILD_CONFS=$(BUILD_CONFS) BUILD_ARCH=$(BUILD_ARCH)"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)$(WIN_IHV_DIR) -f build_vs13.mk BUILD_CONFS=$(BUILD_CONFS) BUILD_ARCH=$(BUILD_ARCH); \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-WINBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-WINBUILD

## -------------------------------------------------------------------
## All Linux coverity build targets
## -------------------------------------------------------------------
#
# Linux DHD Drivers
#
$(LINUX_DHD_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),dhd/)
$(LINUX_DHD_TARGETS): checkout
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/linux $(if $(findstring android,$@),$(if $(findstring mips,$@),$(LINUX_DHD_OPTS_ANDROID_MIPS),$(LINUX_DHD_OPTS_ANDROID_ARM)),$(LINUX_DHD_OPTS)) $@"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/linux $(if $(findstring android,$@),$(if $(findstring mips,$@),$(LINUX_DHD_OPTS_ANDROID_MIPS),$(LINUX_DHD_OPTS_ANDROID_ARM)),$(LINUX_DHD_OPTS)) $@; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Linux PCIE DHD Drivers
#
$(LINUX_DHD_BRIX_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),dhd/)
$(LINUX_DHD_BRIX_TARGETS): checkout
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/linux $(LINUX_DHD_OPTS_ANDROID_BRIX) $@"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/linux $(LINUX_DHD_OPTS_ANDROID_BRIX) $@; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Linux WL Drivers
#
$(LINUX_WL_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(LINUX_WL_TARGETS): checkout
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/linux $(LINUX_WL_OPTS) $@"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/linux $(LINUX_WL_OPTS) $@; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Linux WL and DHD Utility
#
$(LINUX_UTILS_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(LINUX_UTILS_TARGETS): CURTGT=$(if $(findstring -wl-,$@),wl,dhd)
$(LINUX_UTILS_TARGETS): checkout
$(LINUX_UTILS_TARGETS):
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/$(CURTGT)/exe"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/$(CURTGT)/exe; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Mac WL Utility
#
$(MAC_UTILS_TARGETS_10_10): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(MAC_UTILS_TARGETS_10_10): CURTGT=$(if $(findstring -wl-,$@),wl,dhd)
$(MAC_UTILS_TARGETS_10_10): COVERRLVL=87
$(MAC_UTILS_TARGETS_10_10): checkout
$(MAC_UTILS_TARGETS_10_10):
ifdef COV-MACBUILD-10_10
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/$(CURTGT)/exe CC=/usr/bin/clang"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/$(CURTGT)/exe CC=/usr/bin/clang; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-MACBUILD-10_10
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-MACBUILD-10_10

#
#
# Linux Dongle Images
#
# Current OSTYPE and BUSTYPE are derived per firmware image
#
$(LINUX_DONGLE_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),hndrte-dongle-wl/)
$(LINUX_DONGLE_TARGETS): checkout
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)$(LINUX_DONGLE_DIR) $@"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)$(LINUX_DONGLE_DIR) $@; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
#	# TO-DO: Firmware keyword needs to be appended to firmware targets to help
#	# TO-DO: Coverity run search, as firmware names are sometimes very cryptic
#	$(call RUN_COVERITY_COMMIT,$@,Firmware)
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Linux CXAPI
#
$(LINUX_CXAPI_TARGETS): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(LINUX_CXAPI_TARGETS): checkout
ifdef COV-LX32BUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/cx/source/linux $(LINUX_CXAPI_OPTS)"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/cx/source/linux $(LINUX_CXAPI_OPTS); \
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/cx/cnClient/linux $(LINUX_CXAPI_OPTS)"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/cx/cnClient/linux $(LINUX_CXAPI_OPTS); \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-LX32BUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LX32BUILD

#
# Linux-2.6.36 ARM Router
#
linux-2.6.36-router: BUILD_TREE=linux-2.6.36-router/
linux-2.6.36-router: KERNELCFG := defconfig-2.6-bcm947xx-router-ipv6-up
linux-2.6.36-router: ROUTERCFG := defconfig-2.6-vista-router
linux-2.6.36-router: LINUX_VERSION := 2_6_36
linux-2.6.36-router: LINUX_VER = $(subst _,.,$(LINUX_VERSION))
linux-2.6.36-router: HNDSVN_ARM_ROUTER_BOMS := linux-2.6.36-router
linux-2.6.36-router: LINUX_ROUTER_ARM_PATH := PATH="/projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin:$(PATH)"
linux-2.6.36-router: checkout
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	$(call SVN_CHECKOUT_GCLIENT,$(HNDSVN_ARM_ROUTER_BOMS))
	rm -rf $(COVTMPDIR)
	$(INSTALL) $(BUILD_TREE)components/opensource/linux/linux-$(LINUX_VER)/arch/arm/$(KERNELCFG) $(BUILD_TREE)components/opensource/linux/linux-$(LINUX_VER)/.config
	cat $(BUILD_TREE)src/router/config/$(ROUTERCFG) | \
		sed -e "s/# CONFIG_LIBCRYPT is not set/CONFIG_LIBCRYPT=y/g" \
		    -e "s/CONFIG_LIBOPT=y/# CONFIG_LIBOPT is not set/g" \
		    -e "s/# CONFIG_UCLIBC is not set/CONFIG_UCLIBC=y/g" \
		    -e "s/CONFIG_GLIBC=y/# CONFIG_GLIBC is not set/g" > $(BUILD_TREE)src/router/.config
	$(LINUX_ROUTER_ARM_PATH) $(COVMAKE) -j 4 LINUX_VERSION=$(LINUX_VERSION) -C $(BUILD_TREE)src/router oldconfig PLT=arm
	@echo "$(LINUX_ROUTER_ARM_PATH) $(COVBUILD) $(COVMAKE) -j 4 LINUX_VERSION=$(LINUX_VERSION) -C $(BUILD_TREE)src/router all install PLT=arm"; \
	       $(LINUX_ROUTER_ARM_PATH) $(COVBUILD) $(COVMAKE) -j 4 LINUX_VERSION=$(LINUX_VERSION) -C $(BUILD_TREE)src/router all install PLT=arm; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Linux-2.6.36 ARM Router
#
linux-2.6.36-router-main: BUILD_TREE=linux-2.6.36-router/main/
linux-2.6.36-router-main: KERNELCFG := defconfig-2.6-bcm947xx-router-ipv6-up
linux-2.6.36-router-main: ROUTERCFG := defconfig-2.6-vista-router
linux-2.6.36-router-main: LINUX_VERSION := 2_6_36
linux-2.6.36-router-main: LINUX_VER = $(subst _,.,$(LINUX_VERSION))
linux-2.6.36-router-main: HNDSVN_ARM_ROUTER_BOMS := linux-2.6.36-router
linux-2.6.36-router-main: LINUX_ROUTER_ARM_PATH := PATH="/projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin:$(PATH)"
linux-2.6.36-router-main: checkout
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	$(call SVN_CHECKOUT_GCLIENT_MAIN_ROUTER,$(HNDSVN_ARM_ROUTER_BOMS))
	rm -rf $(COVTMPDIR)
	$(INSTALL) $(BUILD_TREE)components/opensource/linux/linux-$(LINUX_VER)/arch/arm/$(KERNELCFG) $(BUILD_TREE)components/opensource/linux/linux-$(LINUX_VER)/.config
	cat $(BUILD_TREE)$(ROUTER_TREE)/config/$(ROUTERCFG) | \
		sed -e "s/# CONFIG_LIBCRYPT is not set/CONFIG_LIBCRYPT=y/g" \
		    -e "s/CONFIG_LIBOPT=y/# CONFIG_LIBOPT is not set/g" \
		    -e "s/# CONFIG_UCLIBC is not set/CONFIG_UCLIBC=y/g" \
		    -e "s/CONFIG_GLIBC=y/# CONFIG_GLIBC is not set/g" > $(BUILD_TREE)$(ROUTER_TREE)/.config
	$(LINUX_ROUTER_ARM_PATH) $(COVMAKE) -j 4 LINUX_VERSION=$(LINUX_VERSION) -C $(BUILD_TREE)$(ROUTER_TREE) oldconfig PLT=arm
	@echo "$(LINUX_ROUTER_ARM_PATH) $(COVBUILD) $(COVMAKE) -j 4 LINUX_VERSION=$(LINUX_VERSION) -C $(BUILD_TREE)$(ROUTER_TREE) all install PLT=arm"; \
	       $(LINUX_ROUTER_ARM_PATH) $(COVBUILD) $(COVMAKE) -j 4 LINUX_VERSION=$(LINUX_VERSION) -C $(BUILD_TREE)$(ROUTER_TREE) all install PLT=arm; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-LINUXBUILD

#
# Linux-2.6.36 ARM dhdap Router
#
linux-2.6.36-router-dhdap: BUILD_TREE=linux-2.6.36-router-dhdap/main/
linux-2.6.36-router-dhdap: KERNELCFG := defconfig-2.6-bcm947xx-router-ipv6-dhdap
linux-2.6.36-router-dhdap: ROUTERCFG := defconfig-2.6-vista-dhdap-router
linux-2.6.36-router-dhdap: LINUX_VERSION := 2_6_36
linux-2.6.36-router-dhdap: LINUX_VER = $(subst _,.,$(LINUX_VERSION))
linux-2.6.36-router-dhdap: LINUX_ARM_PATH := PATH="/projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin:$(PATH)"
linux-2.6.36-router-dhdap: MAKE_JOBS := 4
linux-2.6.36-router-dhdap: checkout
ifdef COV-LINUXBUILD
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	$(SVNCMD) info $(BUILD_TREE)src | grep "^Revision: " | awk -F': ' '{printf "%s",$$NF}' > _SUBVERSION_REVISION
	$(INSTALL) $(BUILD_TREE)components/opensource/linux/linux-$(LINUX_VER)/arch/arm/$(KERNELCFG) $(BUILD_TREE)components/opensource/linux/linux-$(LINUX_VER)/.config
	cat $(BUILD_TREE)$(ROUTER_TREE)/config/$(ROUTERCFG) | \
		sed -e "s/# CONFIG_DLNA_DMS is not set/CONFIG_DLNA_DMS=y/;" \
		    -e "s/# CONFIG_FFMPEG is not set/CONFIG_FFMPEG=y/;" \
		    -e "s/# CONFIG_LIBZ is not set/CONFIG_LIBZ=y/;" \
                    -e "s/CONFIG_LIBOPT=y/# CONFIG_LIBOPT is not set/;" > $(BUILD_TREE)$(ROUTER_TREE)/.config
	$(LINUX_ARM_PATH) $(COVMAKE) -j $(MAKE_JOBS) LINUX_VERSION=$(LINUX_VERSION) -C $(BUILD_TREE)$(ROUTER_TREE) oldconfig PLT=arm
	@echo "$(LINUX_ARM_PATH) $(COVBUILD) $(COVMAKE) -j $(MAKE_JOBS) LINUX_VERSION=$(LINUX_VERSION) -C $(BUILD_TREE)$(ROUTER_TREE) all install PLT=arm"; \
	       $(LINUX_ARM_PATH) $(COVBUILD) $(COVMAKE) -j $(MAKE_JOBS) LINUX_VERSION=$(LINUX_VERSION) -C $(BUILD_TREE)$(ROUTER_TREE) all install PLT=arm; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CUSTOM_CHECKERS,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-LINUXBUILD
	@echo "INFO: Skipping $@ on $(UNAME) platform8
endif # COV-LINUXBUILD

#
# Mac WL Drivers 10.10
#
$(MAC_WL_TARGETS_10_10): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(MAC_WL_TARGETS_10_10): checkout
ifdef COV-MACBUILD-10_10
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/macos $(MAC_WL_OPTS) $@"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/macos $(MAC_WL_OPTS) $@; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-MACBUILD-10_10
        @echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-MACBUILD-10_10

#
# Mac WL Drivers 10.11
#
$(MAC_WL_TARGETS_10_11): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(MAC_WL_TARGETS_10_11): checkout
ifdef COV-MACBUILD-10_11
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/macos $(MAC_WL_OPTS) $@"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/macos $(MAC_WL_OPTS) $@; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-MACBUILD-10_11
        @echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-MACBUILD-10_11

#
# Mac WL Drivers 10.12
#
$(MAC_WL_TARGETS_10_12): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),wl-build/)
$(MAC_WL_TARGETS_10_12): checkout
ifdef COV-MACBUILD-10_12
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/macos $(MAC_WL_OPTS) $@"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/wl/macos $(MAC_WL_OPTS) $@; \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-MACBUILD-10_12
        @echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-MACBUILD-10_12

#
# Mac DHD 10.11
#
$(MAC_DHD_TARGETS_10_11): BUILD_TREE=$(if $(findstring GCLIENT, $(TREES)),dhd/)
$(MAC_DHD_TARGETS_10_11): COV_TARGET=$(subst DHD_,,$@)
$(MAC_DHD_TARGETS_10_11): checkout
ifdef COV-MACBUILD-10_11
	@$(MARKSTART)
	rm -rf $(COVTMPDIR)
	@echo "$(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/macos $(MAC_WL_OPTS) $(COV_TARGET)"; \
	       $(COVBUILD) $(COVMAKE) -C $(BUILD_TREE)src/dhd/macos $(MAC_WL_OPTS) $(COV_TARGET); \
	       $(CHECK_COVBUILD_STATUS)
	$(call RUN_COVERITY_ANALYZE);
	$(call RUN_COVERITY_COMMIT,$@)
	$(call RUN_COVERITY_CLEANUP,$@)
	@$(MARKEND)
else  # COV-MACBUILD-10_11
        @echo "INFO: Skipping $@ on $(UNAME) platform"
endif # COV-MACBUILD-10_11

build_status:
	@found_failed=`find misc -name "*failed*" -print 2> $(NULL)`; \
	if [ ! -z "$${found_failed}" ]; then \
	   echo "ERROR: ---------------------------------------"; \
	   echo "ERROR: Some coverity build steps failed for:"; \
	   echo "ERROR: $${found_failed}"; \
	   echo "ERROR: Check misc/*/build-log.txt log"; \
	   echo "ERROR: ---------------------------------------"; \
	   exit 1; \
	fi

# Clean intermediate objects
build_clean: $(CURRENT_TARGETS)
	-@find src components -type f -name "*.obj" -o -name "*.OBJ" -o -name "*.o" | \
		xargs rm -f

build_end: build_clean
	@date +"[%D %T] * END  : BRAND $(BRAND) $(TAG)"

help:
	@echo "---------------------------------------------------"
	@echo "Coverity targets available on $(if $(TAG),$(TAG),TOT):"
	@echo -e "\n $(foreach tgt,$(CURRENT_TARGETS),+ $(tgt) $(if $($(tgt)),[$($(tgt))])\n)"
	@echo "---------------------------------------------------"

.PHONY:
