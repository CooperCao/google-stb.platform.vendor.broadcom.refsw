#
# NetBSD ap Makefile
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

include WLAN.usf

HOSTOS      := $(shell uname -s)
PWD         := $(shell pwd)
RELEASEDIR  := release/netbsd

SRCFILELISTS_COMPONENTS+= src/tools/release/components/wlphy-filelist.txt
SRCFILELIST_ORG         = src/tools/release/bsd-filelist.txt
SRCFILELIST             = bsd-filelist.txt

# These are module names and directories that will be checked out of CVS.
CVSMODULES  := wl-bsd-build src/hndcvs

# These symbols will be UNDEFINED in the source code by the transmogrifier
UNDEFS      += WLNINTENDO WLNINTENDO2 BCMSDIO BCMPCMCIA MSFT INTEL NETGEAR WLNOKIA BCMWAPI_WPI BCMWAPI_WAI

include unreleased-chiplist.mk

# These symbols will be DEFINED in the source code by the transmogrifier
DEFS        += WLPHY

# for file list pre-processing
FILESDEFS = $(DEFS)
FILESUNDEFS =$(patsubst %,NO_%,$(UNDEFS))
FILESDEFS += FILESUNDEFS
GCCFILESDEFS = $(patsubst %,-D%_SRC,$(FILESDEFS))

# How to run the mogrifier
MOGRIFY = perl src/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))

SRCFILTER =  perl -e 'open(INPUT, "<$$ARGV[0]"); while (<INPUT>) { chomp; next if /^\#/; next unless /\S/;  print "$$_\n"; } close(INPUT);'

# Available toolchains
ifeq ($(HOSTOS),Linux)
     PATH := $(PATH):/projects/hnd/tools/linux/hndtools-x86-netbsd/bin:/tools/bin
else
     PATH := $(PATH):/usr/pkg/bin
endif

all: release_archive

clean:
	rm -rf src build release

SRCRELEASE := src/include src/shared src/wl/sys src/bcmcrypto src/wl/keymgmt src/wl/bsd/objdir
SRCRELEASE += $(patsubst %,%/include %/src,$(WLAN_COMPONENT_PATHS))

# Checkout and mogrify sources
checkout: $(CHECKOUT_TGT)

filelists :
        cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
                src/tools/release/rem_comments.awk > master_filelist.h
        gcc -E -undef $(GCCFILESDEFS) -o -Ulinux $(SRCFILELIST) master_filelist.h

src/.mogrified: checkout filelists
	date +"%c -- MARK mogrify --"
        # Mogrify all text files
	find src components -name .svn -prune -o -type f -print | file -f - | grep text | sed -ne 's/^\([^:]*\):.*/\1/p' > $@
	xargs $(MOGRIFY) < $@

mogrify: src/.mogrified

## Build netbsd modules here
prebuild:
	date +"%c -- MARK $@ --"
	$(MAKE) -C src/wl/bsd

# Install mogrified sources in the release directory
release_src: mogrify prebuild
	date +"%c -- MARK release_src --"
	@date +"START: $@, %D %T" | tee -a profile.log
	mkdir -p $(RELEASEDIR)
        # Install VxWorks
	find $(SRCRELEASE) | $(SRCFILTER) $(SRCFILELIST) | \
		grep -v "/CVS[/]*$$" | \
		grep -v "/\.svn[/]*$$" | \
		pax -rw -s "/.*\/CVS\/.*//g" -s "/.*\/CVS$$//" \
		        -s "/.*\/\.svn\/.*//g" -s "/.*\/\.svn$$//" \
		$(RELEASEDIR)
	mv $(RELEASEDIR)/src/wl/bsd/objdir/wlconf.h $(RELEASEDIR)/src/include
	# Move wl tool sources to different directory
	install -d $(RELEASEDIR)/wl_tool
	mv $(RELEASEDIR)/src/wl/exe/* $(RELEASEDIR)/wl_tool/
	rm -rf $(RELEASEDIR)/src/wl/bsd/objdir/
        # Install documentation
	install -d $(RELEASEDIR)/doc
#	cp src/doc/bcm47xx/VxReadme.txt $(RELEASEDIR)/README.TXT
	@date +"END: $@, %D %T" | tee -a profile.log

# Release and archive
release: release_src
	date +"%c -- MARK $@ --"
	install src/wl/bsd/objdir/kernel/wl.o     $(RELEASEDIR)
	install src/wl/bsd/objdir/kernel/pcilkm.o $(RELEASEDIR)
	install src/wl/bsd/objdir/usr/wl          $(RELEASEDIR)
	install src/wl/bsd/objdir/rmmod           $(RELEASEDIR)
	install src/wl/bsd/objdir/insmod          $(RELEASEDIR)
	install src/wl/bsd/objdir/start_ap        $(RELEASEDIR)

release_archive: ROUTER_PACKAGE_VERSION = $(shell perl -lne '/ROUTER_PACKAGE_VERSION\s+(.*)/ && print $$1' < components/router/shared/router_version.h)
release_archive: release
	@date +"%c -- MARK release_archive--"
	cd $(RELEASEDIR) && tar czf ../src.tar.gz --exclude=*/.svn *
	install src/tools/release/install_bsd.sh $(RELEASEDIR)/../install.sh
	cd $(RELEASEDIR)/../ && tar czf src-$(ROUTER_PACKAGE_VERSION).tar.gz --exclude=*/.svn src.tar.gz install.sh

.PHONY: all clean mogrify prebuild release
