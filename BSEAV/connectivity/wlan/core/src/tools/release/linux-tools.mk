#
# Linux tools Makefile
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

PARENT_MAKEFILE :=
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

# There is no parent makefile for this
# include $(PARENT_MAKEFILE)

DATE := $(shell date +"%Y%m%d")

# So that .rpmmacros is found
export HOME := $(shell pwd)

# CVS modules to check out
CVSMODULES := ethereal
HNDSVN_BOM := ethereal.sparse

# Miscellaneous tools

all: ethereal

# Checkout sources
checkout: $(CHECKOUT_TGT)
	@find src components -type d -name "CVS" | xargs -t -l1 rm -rf

rpm:
	mkdir -p rpm/BUILD rpm/RPMS rpm/SOURCES rpm/SPECS rpm/SRPMS
	echo "%_topdir `cd rpm && pwd`" > .rpmmacros

# Create spec file
rpm/SPECS/ethereal.spec: checkout rpm
	echo "%define _unpackaged_files_terminate_build 0" > $@
	sed -e "s/@DATE@/$(DATE)/" < src/tools/ethereal/packaging/rpm/SPECS/ethereal.spec.in >> $@

# Copy sources
rpm/SOURCES/ethereal.tar.bz2: checkout rpm
	(cd src/tools && tar --create --exclude="win32-libs" --exclude=*/.svn --bzip2 --file=- ethereal) > $@
	cp -f src/tools/ethereal/packaging/rpm/SOURCES/* rpm/SOURCES/

# Build RPM and SRPM
ethereal: rpm/SPECS/ethereal.spec rpm/SOURCES/ethereal.tar.bz2
        # Build binary and source packages and remove the build tree when done
	rpmbuild -ba --clean $<
        # Remove source directory
	rm -rf src

.PHONY: all
