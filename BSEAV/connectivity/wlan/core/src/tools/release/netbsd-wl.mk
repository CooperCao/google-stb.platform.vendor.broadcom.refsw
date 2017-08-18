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
# $Id: netbsd-wl.mk 260174 2011-05-17 22:41:16Z $
#

HOSTOS      := $(shell uname -s)
PWD         := $(shell pwd)
RELEASE     := release
RELEASEDIR  := release/netbsd
RELSRCDIR   := release/netbsd/sys/dev/ic/bwl
HNDGC_BOM ?= netbsd-i386-wl
SRCFILELIST_ORG = src/tools/release/bsd-filelist.txt
SRCFILELIST = bsd-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/wlphy-filelist.txt

# These symbols will be UNDEFINED in the source code by the transmogrifier
UNDEFS      += WLNINTENDO WLNINTENDO2 BCMSDIO BCMPCMCIA MSFT INTEL NETGEAR WLMOTOROLALJ WLNOKIA WLNOKIA BCMWAPI_WPI BCMWAPI_WAI

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
     PATH := /usr/pkg/bin:$(PATH)
endif

ifdef GEN_CLM
include src/makefiles/WLAN_Common.mk
# compile the clm xml file to generate the wlc_clm_data.c
CLM_TYPE := J28_4360
$(call WLAN_GenClmCompilerRule,$(WLAN_SrcBaseA)/../components/clm-api/src,$(WLAN_SrcBaseA))

else
# beginning of else

#all: release_archive
all: release_archive

clean:
	rm -rf src build release

# How to filter the cvs diff for override patch
OVPATCH = perl src/tools/build/ovpatch.pl $(OVERRIDE)
SRCRELEASE = src/include src/common/include src/shared src/wl/sys src/wl/keymgmt

# Checkout and mogrify sources
checkout: $(CHECKOUT_TGT)

filelists :
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
	src/tools/release/rem_comments.awk > master_filelist.h
	gcc -E -undef $(GCCFILESDEFS) -Ulinux -o $(SRCFILELIST) master_filelist.h


## ------------------------------------------------------------------
## Mogrify sources.
## ------------------------------------------------------------------
mogrify: src/.mogrified

src/.mogrified: checkout filelists
	date +"%c -- MARK mogrify --"
        # Mogrify all text files
	find src/wl -name .svn -prune -o -type f -print | file -f - | grep text | sed -ne 's/^\([^:]*\):.*/\1/p' > $@
	xargs $(MOGRIFY) < $@
	find src/shared -name .svn -prune -o -type f -print | file -f - | grep text | sed -ne 's/^\([^:]*\):.*/\1/p' > $@
	xargs $(MOGRIFY) < $@
	find src/include -name .svn -prune -o -type f -print | file -f - | grep text | sed -ne 's/^\([^:]*\):.*/\1/p' > $@
	xargs $(MOGRIFY) < $@
	find src/common -name .svn -prune -o -type f -print | file -f - | grep text | sed -ne 's/^\([^:]*\):.*/\1/p' > $@
	xargs $(MOGRIFY) < $@
	find src/bcmcrypto -name .svn -prune -o -type f -print | file -f - | grep text | sed -ne 's/^\([^:]*\):.*/\1/p' > $@
	xargs $(MOGRIFY) < $@

##
prebuild: mogrify
	date +"%c -- MARK $@ --"
	@echo "===> Checking netbsd symlinks"
	@cd $(PWD)/src/netbsd/ && for i in $(PWD)/components/opensource/netbsd4/*; do \
		if [ ! -e  $(PWD)/src/netbsd/`basename $$i` ]; then \
			ln -sfv	$$i . ; \
		fi ; \
	done
	@cd $(PWD)/components/opensource && for i in bcm57xx et include common shared wl bcmcrypto; do \
		if [ ! -e  $(PWD)/components/opensource/$$i ]; then \
			ln -sfv $(PWD)/src/$$i $$i ; \
		fi ; \
	done
	$(MAKE) -C src/wl/bsd wlconf
	# Add symlinks to Broadcom sources
	@if [ ! -d $(PWD)/src/netbsd/sys/dev/ic/bwl/include ]; then \
		ln -s $(PWD)/$(RELSRCDIR)/include $(PWD)/src/netbsd/sys/dev/ic/bwl/include ; \
	fi
	@if [ ! -d $(PWD)/src/netbsd/sys/dev/ic/bwl/common ]; then \
		ln -s $(PWD)/$(RELSRCDIR)/common $(PWD)/src/netbsd/sys/dev/ic/bwl/common ; \
	fi
	@if [ ! -d $(PWD)/src/netbsd/sys/dev/ic/bwl/shared ]; then \
		ln -s $(PWD)/$(RELSRCDIR)/shared $(PWD)/src/netbsd/sys/dev/ic/bwl/shared ; \
	fi
	@if [ ! -d $(PWD)/src/netbsd/sys/dev/ic/bwl/wl ]; then \
		ln -s $(PWD)/$(RELSRCDIR)/wl $(PWD)/src/netbsd/sys/dev/ic/bwl/wl ; \
	fi
	@if [ ! -d $(PWD)/src/netbsd/sys/dev/ic/bwl/bcmcrypto ]; then \
		ln -s $(PWD)/$(RELSRCDIR)/bcmcrypto $(PWD)/src/netbsd/sys/dev/ic/bwl/bcmcrypto ; \
	fi
	@if [ ! -d $(PWD)/src/netbsd/sys/net80211 ]; then \
		cd $(PWD)/src/netbsd/sys && ln -sfv $(PWD)/components/vendor/apple/net80211 net80211 ; \
	fi
	$(MAKE) GEN_CLM=1 -f netbsd-wl.mk

build: release_src
	which make
	set
	cd $(PWD)/src/netbsd/sys/arch/i386/conf && config BRCM
# NetBSD make is being used here but MAKEFLAGS may carry incompatible GNU make flags.
	cd $(PWD)/src/netbsd/sys/arch/i386/compile/BRCM && MAKEFLAGS= make depend
	cd $(PWD)/src/netbsd/sys/arch/i386/compile/BRCM && MAKEFLAGS= make

# Install mogrified sources in the release directory
release_src: prebuild
	date +"%c -- MARK release_src --"
	@date +"START: $@, %D %T" | tee -a profile.log
	mkdir -p $(RELEASEDIR)
	find $(SRCRELEASE) | $(SRCFILTER) $(SRCFILELIST) | \
		grep -v "/CVS[/]*$$" | \
		pax -rw -s "/.*\/CVS\/.*//g" -s "/.*\/CVS$$//" $(RELEASEDIR)
	mkdir -p $(RELSRCDIR)
	mv $(RELEASEDIR)/src/wl $(RELSRCDIR)/wl
	mv $(RELEASEDIR)/src/shared $(RELSRCDIR)/shared
	mv $(RELEASEDIR)/src/include $(RELSRCDIR)/include
	mv $(RELEASEDIR)/src/common $(RELSRCDIR)/common
	mv $(RELEASEDIR)/src/bcmcrypto $(RELSRCDIR)/bcmcrypto
	rm -rf $(RELEASEDIR)/src
	#mv $(RELSRCDIR)/wl/bsd/objdir/wlconf.h $(RELSRCDIR)/include
	mv src/wl/bsd/objdir/wlconf.h $(RELSRCDIR)/include
	rm -rf $(RELSRCDIR)/wl/bsd/objdir/
	# Move wl tool sources to different directory
	install -d $(RELSRCDIR)/wl_tool
	mv $(RELSRCDIR)/wl/exe/* $(RELSRCDIR)/wl_tool/
        # Install documentation
	install -d $(RELSRCDIR)/doc
	@date +"END: $@, %D %T" | tee -a profile.log

# Release and archive
release: build
	date +"%c -- MARK $@ --"

release_archive: release
	@date +"%c -- MARK release_archive--"
ifeq ($(TAG),)
	cd $(RELEASE) && tar czf src.tgz *
else # TAG
	cd $(RELEASE) && tar czf $(TAG).tgz *
endif # TAG
	# install src/tools/release/install_bsd.sh $(RELEASEDIR)/../install.sh
	# cd $(RELEASEDIR)/../ && tar czf src-$(ROUTER_PACKAGE_VERSION).tar.gz src.tar.gz install.sh

.PHONY: all clean mogrify prebuild release release_archive

endif # end of else
