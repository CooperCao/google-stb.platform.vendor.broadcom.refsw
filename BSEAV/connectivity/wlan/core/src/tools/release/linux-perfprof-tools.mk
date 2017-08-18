#
# BCM947xx Performance Profiling Tools build/release Makefile
#
# Copyright (C) 2002 Broadcom Corporation
#
# $Id$
#

BUILD_BASE := $(PWD)

BUILDDIR := $(BUILD_BASE)/build
RELEASEDIR := $(BUILD_BASE)/release

# These symbols will be UNDEFINED in the source code by the transmogrifier
UNDEFS =

# These symbols will be DEFINED in the source code by the transmogrifier
DEFS = PROFSRC

# Source mogrification command
MOGRIFY = perl src/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))

# These are module names and directories that will be checked out of CVS.
#
CVSMODULES = perfprof-tools

all: $(CVSMODULES)
# 	MOGRIFY	the src
	@date +"%c -- MARK mogrify --"
	find src components -type f | file -f - | grep text | sed -ne 's/^\([^:]*\):.*/\1/p' > $@
	xargs $(MOGRIFY) < $@
#	Build the tools
	date +"%c -- MARK build performance profiling host tools --"
	$(MAKE) -C src/tools/profile/sbprof/host BUILDDIR="$(BUILDDIR)" INSTALLDIR="$(RELEASEDIR)" install
#       Keep src directory as it is for future debugging
	date +"%c -- MARK archive src --"
	tar -czf src.tar.gz src
#	Remove source and intermediate files
	rm -rf build
	rm -rf src

# Check out modules using export -kv
# This removes the CVS keyword escape syntax around CVS keyword expansions
# This messes up binary files, so follow up with a normal checkout of files
# used in the VxWorks build

# How to filter the cvs diff for override patch
OVPATCH = perl src/tools/build/ovpatch.pl $(OVERRIDE)

$(CVSMODULES):
ifeq ($(TAG),)
	cvs -Q co -P $(if $(CVSCUTOFF),-D "$(CVSCUTOFF)") $@
else
	cvs -Q export -kv $(if $(TAG),-r $(TAG)) $@
endif
ifneq ($(OVERRIDE),)
#	Apply any changes from override files
	cvs status -l $(OVERRIDE) > /dev/null
	-cp $(OVERRIDE)/tools/build/ovpatch.pl src/tools/build/ovpatch.pl
	cvs diff -Nu $(OVERRIDE) | $(OVPATCH) > patch.cvs
	patch -E -p0 < patch.cvs
#	rm -f patch.cvs
endif

.PHONY: all
