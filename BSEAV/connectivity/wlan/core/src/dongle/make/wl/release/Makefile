# Front-end subdir level makefile for hndrte based standalone programs.
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
# Invoked from src/dongle/make/wl/Makefile.
#

ifeq ($(TOPDIR),)
$(error TOPDIR is not defined)
endif

SUBDIR := $(shell /bin/pwd)
SUBDIR := $(SUBDIR:$(TOPDIR)/%=%)

SUBBLDDIR ?= $(SUBDIR)

# record all make targets in $(IMGLIST) file first, remove any duplicates,
# and issue an unique list of targets to the top level make at the end.
IMGLIST := _all_image_
TGTLIST := _all_target_
MFLIST := _all_mf_

# use this as the 'subdir' of nvram files in manifests
NVSUBDIR := _nvram_

# action
BUILD := build
CLEAN := clean
COPY := copy

# default make target
.PHONY: default
default: all

#####################################################################################

ifeq ($(RECORD),)

.PHONY: Makefile
Makefile:

.PHONY: showbrands showtgts showmaxsize_tgts
showbrands showtgts showmaxsize_tgts:
	$(MAKE) RECORD=1 ACTION=$(BUILD) $@

.PHONY: copy
copy:
	$(MAKE) RECORD=1 ACTION=$(COPY) _begin_ all _end_

.PHONY: %/copy
%/copy:
	$(MAKE) RECORD=1 ACTION=$(COPY) _begin_ $(@:/copy=) _end_

.PHONY: clean
clean:
	$(MAKE) RECORD=1 ACTION=$(CLEAN) _begin_ all _end_
	-$(RM) $(IMGLIST)
	-$(RM) $(TGTLIST)
	-$(RM) $(MFLIST)

.PHONY: %/clean
%/clean:
	$(MAKE) RECORD=1 ACTION=$(CLEAN) _begin_ $(@:/clean=) _end_

.PHONY: all
all:
	$(MAKE) RECORD=1 ACTION=$(BUILD) _begin_ $@ _end_

%:: FORCE
	$(MAKE) RECORD=1 ACTION=$(BUILD) _begin_ $@ _end_

endif	# !RECORD

#####################################################################################

ifneq ($(RECORD),)

# keep _begin_ ... _end_ targets all processed in order
.NOTPARALLEL:

# begin of target list
.PHONY: _begin_
_begin_:
	@echo -n "" > $(IMGLIST)
ifeq ($(ACTION),$(COPY))
	@echo -n "" > $(MFLIST)
endif
ifeq ($(ACTION),$(BUILD))
	@echo -n "" > $(TGTLIST)
endif
ifeq ($(ACTION),$(CLEAN))
	@echo -n "" > $(TGTLIST)
endif

#####################################################################################

ifeq ($(RBRAND),)

BRANDS := $(patsubst %.mf,%,$(wildcard *.mf))

# to get all .mk file created
ifneq ($(BRANDS),)
%.mk: %.mf
	@echo "TARGETS := $(shell perl mf-parser.pl $< _bldtgts_)" > $@
-include $(BRANDS:=.mk)
endif

# show all brands
.PHONY: showbrands
showbrands:
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# show all targets
.PHONY: showtgts
showtgts: $(BRANDS:=.mk)
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# show all targets in a brand
.PHONY: $(BRANDS:=/showtgts)
$(BRANDS:=/showtgts):
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# show all targets
.PHONY: showmaxsize_tgts
showmaxsize_tgts:
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# show all targets in a brand
.PHONY: $(BRANDS:=/showmaxsize_tgts)
$(BRANDS:=/showmaxsize_tgts):
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# build all releases
.PHONY: all
all: $(BRANDS)

# build all targets in a release
$(BRANDS): FORCE
	$(MAKE) RECORD=$(RECORD) ACTION=$(ACTION) RBRAND=$@
ifeq ($(ACTION),$(CLEAN))
	-$(RM) $@.mk
endif

# build a specific target in a release
$(BRANDS:=/%): _BRAND = $(firstword $(subst /, ,$@))
$(BRANDS:=/%): FORCE
	$(MAKE) RECORD=$(RECORD) ACTION=$(ACTION) RBRAND=$(_BRAND) $(@:$(_BRAND)/%=%)

endif	# !RBRAND

#####################################################################################

ifneq ($(RBRAND),)

include $(RBRAND).mk

ifeq ($(TARGETS),)
$(error TARGETS is not defined)
endif

# build all targets
.PHONY: all
all: $(TARGETS)

# build a single target
$(sort $(TARGETS)): FORCE
	@echo "$@" >> $(IMGLIST)
ifeq ($(ACTION),$(COPY))
	( \
	 _COPYTGTS=`perl mf-parser.pl $(RBRAND).mf $@` && \
	 for _COPYTGT in $$_COPYTGTS; do \
	  echo "$@ $$_COPYTGT" >> $(MFLIST); \
	 done \
	)
endif

endif	# RBRAND

#####################################################################################

ifeq ($(ACTION),$(COPY))
ifeq ($(TOPCPDIR),)
$(error TOPCPDIR is not defined)
endif
endif

NVFILEDIR := $(TOPDIR)/../../../shared/nvram

# end of target list
.PHONY: _end_
_end_:
	cat $(IMGLIST) | sort -u > .tmp.$(IMGLIST)
	mv .tmp.$(IMGLIST) $(IMGLIST)
ifeq ($(ACTION),$(COPY))
	( \
	 cat $(MFLIST) | sort -u > .tmp.$(MFLIST) && \
	 mv .tmp.$(MFLIST) $(MFLIST) && \
	 install -d $(TOPCPDIR) && \
	 _IMAGES=`/bin/cat $(IMGLIST)` && \
	 for _IMAGE in $$_IMAGES; do \
	  _DESTS=`perl mf-parser.pl $(MFLIST) $$_IMAGE` && \
	  _TARGET=`dirname $$_IMAGE` && \
	  if [ "$$_TARGET" != "$(NVSUBDIR)" ]; then \
	   $(MAKE) -C $(TOPDIR) $$_IMAGE; \
	   if [ "$$?" != "0" ]; then \
	    exit $$?; \
	   fi; \
	  fi && \
	  for _DEST in $$_DESTS; do \
	   _DIR=`dirname $$_DEST` && \
	   if [ "$$_TARGET" == "$(NVSUBDIR)" ]; then \
	    _FILE=`basename $$_IMAGE`; \
	   fi && \
	   install -d $(TOPCPDIR)/$$_DIR && \
	   if [ "$$_TARGET" == "$(NVSUBDIR)" ]; then \
	    echo "cp $(NVFILEDIR)/$$_FILE $(TOPCPDIR)/$$_DEST" && \
	    cp $(NVFILEDIR)/$$_FILE $(TOPCPDIR)/$$_DEST; \
	    if [ "$$?" != "0" ]; then \
	     exit $$?; \
	    fi; \
	   else \
	    echo "cp $(TOPBLDDIR)/$$_IMAGE $(TOPCPDIR)/$$_DEST" && \
	    cp $(TOPBLDDIR)/$$_IMAGE $(TOPCPDIR)/$$_DEST; \
	    if [ "$$?" != "0" ]; then \
	     exit $$?; \
	    fi; \
	   fi; \
	  done; \
	 done \
	)
endif
ifeq ($(ACTION),$(BUILD))
	( \
	 _IMAGES=`/bin/cat $(IMGLIST)` && \
	 for _IMAGE in $$_IMAGES; do \
	  _TARGET=`dirname $$_IMAGE` && \
	  if [ "$$_TARGET" != "$(NVSUBDIR)" ]; then \
	   echo $$_TARGET; \
	  fi; \
	 done \
	) | sort -u > $(TGTLIST)
	( \
	 _TARGETS=`/bin/cat $(TGTLIST)` && \
	 if [ "$$_TARGETS" != "" ]; then \
	  $(MAKE) -C $(TOPDIR) $$_TARGETS; \
	  if [ "$$?" != "0" ]; then \
	   exit $$?; \
	  fi; \
	 fi \
	)
endif
ifeq ($(ACTION),$(CLEAN))
	( \
	 _IMAGES=`/bin/cat $(IMGLIST)` && \
	 for _IMAGE in $$_IMAGES; do \
	  _TARGET=`dirname $$_IMAGE` && \
	  if [ "$$_TARGET" != "$(NVSUBDIR)" ]; then \
	   echo $$_TARGET; \
	  fi; \
	 done \
	) | sort -u > $(TGTLIST)
	( \
	 _TARGETS=`/bin/cat $(TGTLIST)` && \
	 if [ "$$_TARGETS" != "" ]; then \
	  for _TARGET in $$_TARGETS; do \
	   $(MAKE) -C $(TOPDIR) $$_TARGET/clean; \
	   if [ "$$?" != "0" ]; then \
	    exit $$?; \
	   fi; \
	  done; \
	 fi \
	)
endif

endif	# RECORD

.PHONY: FORCE
FORCE:
