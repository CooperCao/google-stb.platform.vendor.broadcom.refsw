#
# Common CVS or SVN Checkout makefile rules
#
# These are intended to be included by release makefiles and also provide
# hooks to enable or disable certain steps (like patching or checkout itself)
# that can be used in build scripts. Checkout step is separated from release
# makefile to allow for subsequent build automation efforts
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#
# VCTOOL  = cvs or svn or gclient [Used to identify version control tool]
# BRAND   = Release build name [Used to identify build brand name]
# TAG     = null or TAG or BRANCH [Used to identify trunk or _REL_ or TOB]
# SRC_CHECKOUT_DISABLED = 0 or 1  [Used to suppress checkout]
# OVERRIDE= <path> [Used to specify location of private workspace]
#
# CVSROOT = cvs repository path
# SVNROOT = SVN repository root path in automerger model
# WLANSVN = SVN repository root path in gclient model
# REPO_URL= Parent path to automerger model svn sparse file
# DEPS_URL= Parent path to gclient model deps file
#

ifndef CHECKOUT_RULES_DISABLED

UNAME        := $(shell uname -s)

CHECKOUT_DEFS?= checkout-defs.mk
CVSROOT_PROD  = /projects/cvsroot

# Set svn checkout as default unless overriden by VCTOOL in
# build scripts
export VCTOOL?= svn

# Default checkout commands, that can be overriden by called makefile
GCLIENT_CHECKOUT ?= gclient_checkout
SVN_CHECKOUT     ?= svn_checkout
CVS_CHECKOUT     ?= cvs_checkout

CHECKOUT_TGT     ?= $(SVN_CHECKOUT)
CHECKOUT         ?= $(CHECKOUT_TGT)
SVNTZ            ?= $(shell date '+%z')
SVNCMD           ?= svn --non-interactive

# For all non-internal builds .svn folders are preserved
ifeq ($(findstring internal,$(BRAND)),)
  DOT_SVN_REMOVAL_ENABLED := 1
else  # DOT_SVN_REMOVAL_ENABLED
  DOT_SVN_REMOVAL_ENABLED := 0
endif # DOT_SVN_REMOVAL_ENABLED

# Override via DISABLED flag
ifneq ($(DOT_SVN_REMOVAL_DISABLED),)
  DOT_SVN_REMOVAL_ENABLED := 0
endif # DOT_SVN_REMOVAL_ENABLED

# Default xargs option
XARGS_OPTS   := -t -l1

# Override for Macos and netbsd platforms
ifneq ($(filter Darwin NetBSD,$(UNAME)),)
   XARGS_OPTS:= -t -L1
endif

# Debug message to show effective VCTOOL value before running checkout rules
# $(warning INFO: VCTOOL set to "$(VCTOOL)")

# CVS patch cleanup script
OVPATCH      = perl src/tools/build/ovpatch.pl $(OVERRIDE)
OVFILE       = $(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)

# Section start and end labels
MARKSTART    = date +"[%D %T] MARK-START: $@"
MARKEND      = date +"[%D %T] MARK-END  : $@"

# VCTOOL is passed by build scripts and release makefile or on command line
ifeq ($(findstring svn,$(VCTOOL)),svn)

  ifeq ($(findstring gclient,$(COTOOL)),gclient)

    CHECKOUT_TGT  = $(GCLIENT_CHECKOUT)
    HNDGC_DEFS  ?=
    HNDGC_SUBCMD?= config

    # Note GCLIENT Deps path is derived from sparse setting in release.mk
    # otherwise look for HNDGC_BOM setting
    HNDGC_BOM_URL ?= $(if $(HNDGC_BOM),$(DEPS_URL)/$(HNDGC_BOM),$(DEPS_URL)/$(subst .sparse,,$(strip $(HNDSVN_BOM))))

  else # COTOOL

    CHECKOUT_TGT  = $(SVN_CHECKOUT)
    HNDSVN_DEFS  ?=
    HNDSVN_SUBCMD?= checkout
    HNDSVN_FLAGS ?=

    HNDSVN_BOM_URL  ?= $(REPO_URL)/$(subst .sparse,,$(strip $(HNDSVN_BOM))).sparse
  endif # COTOOL

else # VCTOOL==cvs

    CHECKOUT_TGT  = $(CVS_CHECKOUT)

endif # VCTOOL

# Include checkout definitions
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)

include $(strip $(firstword $(wildcard \
	$(CHECKOUT_DEFS) \
	Z:/home/hwnbuild/src/tools/release/$(CHECKOUT_DEFS) \
	Z:/projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_DEFS))))

else # Non-cygwin

include $(strip $(firstword $(wildcard \
	$(CHECKOUT_DEFS) \
	/home/hwnbuild/src/tools/release/$(CHECKOUT_DEFS) \
	/projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_DEFS))))

endif # UNAME

# SVN and GCLIENT Checkout sources using sparse
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
#pragma runlocal
endif
$(SVN_CHECKOUT) $(GCLIENT_CHECKOUT): $(PRE_CHECKOUT_DEPENDS)
ifneq ($(SRC_CHECKOUT_DISABLED),1)
	@$(MARKSTART)
	@echo "INFO: $(VCTOOL) Checkout on: `uname -a`"
ifeq ($(findstring gclient,$(COTOOL)),gclient)
	@echo "DEPS_FILE = $(HNDGC_BOM)"
	@echo "$(HNDGC_BOM_URL)/DEPS" > _GCLIENT_BUILD
	# If a brand specifies HNDGC_BOM_DIR, use it as workspace co dir
	$(if $(HNDGC_BOM_DIR),mkdir -pv $(HNDGC_BOM_DIR))
	$(if $(HNDGC_BOM_DIR),cd $(HNDGC_BOM_DIR) &&) \
		$(HNDGC) $(HNDGC_SUBCMD) $(HNDGC_DEFS) \
			$(HNDGC_BOM_URL)
	$(if $(HNDGC_BOM_DIR),cd $(HNDGC_BOM_DIR) &&) \
		$(HNDGC) sync $(GCLIENTREVINFO)
	# Record svn info of deps for debugging
	@$(SVNCMD) info $(HNDGC_BOM_URL) | grep "^Last Changed Rev: " | \
		awk -F': ' '{printf "%s\n",$$NF}' > \
		_SUBVERSION_REVISION$(if $(SVNREV),_CUSTOM)
	@echo "-------------------------------"
	@echo ""
	@$(SVNCMD) info $(HNDGC_BOM_URL)
	@echo "-------------------------------"
else # SVN checkout
	@echo "SPARSE_FILE = $(HNDSVN_BOM)"
	@echo "$(REPO_URL)" > _SUBVERSION_BUILD$(if $(SVNREV),_CUSTOM)
	$(HNDSVN) $(HNDSVN_SUBCMD) $(HNDSVN_DEFS) \
		$(HNDSVN_BOM_URL) .
	# Record svn info for easier tracking of svn rev per build brand
	@$(SVNCMD) info src | grep "^Last Changed Rev: " | \
		awk -F': ' '{printf "%s\n",$$NF}' > \
		_SUBVERSION_REVISION$(if $(SVNREV),_CUSTOM)
	@echo "-------------------------------"
	@echo ""
	@$(SVNCMD) info src
	@echo "-------------------------------"
endif # COTOOL
	@$(MARKEND)
else  # SRC_CHECKOUT_DISABLED
	@echo "WARN: $(VCTOOL) Checkout disabled"
endif # SRC_CHECKOUT_DISABLED
ifeq ($(findstring gclient,$(COTOOL)),)
#	# Once checkout is completed for release brand, non-relevant .sparse
#	# files aren't needed at root dir, as no updates are run
#	# So move non-relevant boms to misc/boms folder
#	# But in a GUB environment this is obsolete.
ifndef GUB_DEPTH
	@install -d misc/boms
	-@mv $$(find *.sparse $(HNDSVN_BOM:%=-name % -prune -o) -print) misc/boms
endif # GUB_DEPTH
endif # COTOOL
	# Pre-requisite step before doing any build
	@if [ -d "src/include" ]; then \
	   echo "$(MAKE) -C src/include"; \
	   $(MAKE) -C src/include; \
	else \
	   echo "INFO: Missing src/include. Skipping epiversion generation"; \
	fi
ifeq ($(DOT_SVN_REMOVAL_ENABLED),1)
#	# Cleanup .svn folders after checkout except for internal brands
#	# This step needs some work as sparse export isn't supported
	@date +"[%D %T] MARK-START: $@ [clean .svn]"
	find src components -type d -name ".svn" -print0 | \
		xargs $(XARGS_OPTS) -0 rm -rf
	@date +"[%D %T] MARK-END  : $@ [clean .svn]"
endif # DOT_SVN_REMOVAL_ENABLED
	@echo " -- MARK BUILD BEGINS HERE --"

# CVS Checkout sources using hndcvs or legacy cvs
$(CVS_CHECKOUT):
ifeq ($(findstring svn,$(VCTOOL)),)
	@$(MARKSTART)
ifneq ($(SRC_CHECKOUT_DISABLED),1)
	@echo "INFO: CVS Checkout on: `uname -a`"
	@echo "INFO: CVSROOT set to: $(CVSROOT)"
	@if echo "$(CVSROOT)" | grep -q "$(CVSROOT_PROD)"; then \
	    echo "$(CVSROOT)" > _CVS_BUILD; \
	else \
	    echo "$(CVSROOT)" > _CVS_SNAPSHOT_BUILD; \
	fi
	@if [ "$(HNDCVS_BOM)" != "" ]; then \
	    echo "$(HNDCVS) $(HNDCVS_FLAGS) $(HNDCVS_DEFS) \
		$(if $(CVSCUTOFF),-date '$(CVSCUTOFF)') \
		$(HNDCVS_BOM) $(TAG)"; \
	    $(HNDCVS) $(HNDCVSSVN_FLAGS) $(HNDCVS_DEFS) \
		$(if $(CVSCUTOFF),-date "$(CVSCUTOFF)") \
		$(HNDCVS_BOM) $(TAG); \
	fi
#	# If it is legacy release brand, use cvs to checkout
	-@if [ "$(HNDCVS_BOM)" == "" -a "$CVSMODULES" != "" ]; then \
	    echo "cvs co $(if $(TAG),-r $(TAG)) \
	       $(if $(CVSCUTOFF),-D '$(CVSCUTOFF)') \
		$(CVSMODULES)"; \
	    cvs co $(if $(TAG),-r $(TAG)) \
	       $(if $(CVSCUTOFF),-D "$(CVSCUTOFF)") \
		$(CVSMODULES); \
	fi
ifndef ($(OVERRIDE_TARGET),)
ifneq ($(OVERRIDE),)
	# Apply any changes from override files
	cvs status -l $(OVERRIDE) > /dev/null
	-cp $(OVERRIDE)/tools/build/ovpatch.pl src/tools/build/ovpatch.pl
	cvs diff -Nu $(OVERRIDE) | $(OVPATCH) > patch.cvs
	patch -E -p0 < patch.cvs
	rm -f patch.cvs
endif # OVERRIDE
endif # OVERRIDE_TARGET
	# Remove CVS cruft
	find src components -name '.cvsignore' | xargs rm -rf
else  # SRC_CHECKOUT_DISABLED
	@echo "WARN: CVS Checkout disabled"
endif # SRC_CHECKOUT_DISABLED
	@$(MARKEND)
endif # VCTOOL

# Rule to checkout included makefiles or any other config files
# Since this is used by release makefile, the included files are
# expected to come from src/tools/release
# If override is set, copy that release makefile instead for cvs
#pragma runlocal
%.mk %.usf:
	@echo "Found VCTOOL=$(VCTOOL) COTOOL=$(COTOOL)"
ifneq ($(OVERRIDE),)
	@if [ -f "$(OVFILE)" ]; then \
		 echo "cp -pv $(OVFILE) $@"; \
		 cp -pv $(OVFILE) $@; \
	fi
else  # OVERRIDE
ifneq ($(filter svn,$(VCTOOL))$(filter gclient,$(COTOOL)),)
ifneq ($(REPO_URL)$(DEPS_URL),)
	@echo "SVN: $$(which svn); version: $$(svn -q --version)"
	$(SVNCMD) export $(REPO_URL)/src/tools/release/$@ $@
else # REPO_URL
	@echo "ERROR: SVN Repository URL path couldn't be derived"
	exit 1
endif # REPO_URL
else # VCTOOL
	cvs co $(if $(TAG),-r $(TAG)) -p src/tools/release/$@ > $@
endif # VCTOOL
endif # OVERRIDE

else  # CHECKOUT_RULES_DISABLED

# Keep this from being passed through to child builds.
unexport CHECKOUT_RULES_DISABLED =

endif # CHECKOUT_RULES_DISABLED
