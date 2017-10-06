###########################################################################
##
## Broadcom Proprietary and Confidential. Copyright (C) 2017,
## All Rights Reserved.
## 
## This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
## the contents of this file may not be disclosed to third parties, copied
## or duplicated in any form, in whole or in part, without the prior
## written permission of Broadcom.
##
## This is a common makefile for defining mogrifier related variables
## and make targets
##
## This file has to be included in individual release brand files that
## need to perform source code mogrification
##
## User of this file need to define following before including this file
## 1) MOGRIFY_DEPS    : List of dependencies on which this mogrification
##                      needs to wait (like checkout) (REQUIRED)
## 2) MOGRIFY_FLAGS   : Custom mogrify cmd line switches that need to be
##                      passed (OPTIONAL)
## 3) MOGRIFY_FILETYPES
##                    : Additional brand specific file types to mogrify
##                    : (OPTIONAL)
## 4) MOGRIFY_EXCLUDE : List of dirs to exclude from mogrification
##                    : (OPTIONAL)
##
## Caller can also override default values in this makefile
## 1) MOGRIFY_CHUNK_SIZE   : Number of files to process at a time
## 2) MOGRIFY_PARALLEL_RUNS: Number of parallel mogrification invocations
##
## $Id$
##########################################################################

ifndef MOGRIFY_COMMON_RULES_DISABLED

NULL  ?= /dev/null
UNAME ?= $(shell uname)

# On macos echo -e is integrated into regular echo and also split takes diff
# options, somehow /bin/echo doesn't work properly on macos
ifneq ($(findstring Darwin,$(UNAME)),)
	export MYECHO=echo
	export SPLIT_OPTS=-a 3
else
	export MYECHO=/bin/echo -e
	export SPLIT_OPTS=-a 3 -d
endif

empty ?=
space ?= $(empty) $(empty)
comma ?= $(empty),$(empty)

## These are types of files that need to mogrified
COMMON_MOGRIFY_FILETYPES  = [^\/]*\.c
COMMON_MOGRIFY_FILETYPES += [^\/]*\.h
COMMON_MOGRIFY_FILETYPES += [^\/]*\.mk
COMMON_MOGRIFY_FILETYPES += [^\/]*\.mak
COMMON_MOGRIFY_FILETYPES += [^\/]*\.inc
COMMON_MOGRIFY_FILETYPES += [^\/]*\.s
COMMON_MOGRIFY_FILETYPES += [^\/]*\.tcl
COMMON_MOGRIFY_FILETYPES += [^\/]*\.cpp
COMMON_MOGRIFY_FILETYPES += [^\/]*\.usf
COMMON_MOGRIFY_FILETYPES += [^\/]*\.h\.in
COMMON_MOGRIFY_FILETYPES += sources[^\/]*
COMMON_MOGRIFY_FILETYPES += [^\/]*filelist\.txt
COMMON_MOGRIFY_FILETYPES += [^\/]*readme.*txt
COMMON_MOGRIFY_FILETYPES += [^\/]*wl\.wlex
COMMON_MOGRIFY_FILETYPES += [^\/]*akefile.*
COMMON_MOGRIFY_FILETYPES += [^\/]*akerule.*
COMMON_MOGRIFY_FILETYPES += [^\/]*\.sh
COMMON_MOGRIFY_FILETYPES += \/config\/wlconfig.*
COMMON_MOGRIFY_FILETYPES += \/config\/wl_.*
# These new filetype changes need to be tested further
#COMMON_MOGRIFY_FILETYPES += [^\/]*\.bat
#COMMON_MOGRIFY_FILETYPES += [^\/]*\.pl
#COMMON_MOGRIFY_FILETYPES += [^\/]*defconfig.*

ifneq ($(MOGRIFY_FILETYPES),)
      $(warning INFO: Found Custom Filetypes to mogrify)
      $(warning INFO: $(MOGRIFY_FILETYPES))
endif

# MOGRIFY_FILETYPES come from included brand makefile
ALL_MOGRIFY_FILETYPES     = $(COMMON_MOGRIFY_FILETYPES) $(MOGRIFY_FILETYPES)
# Reg Ex to process filetypes
ALL_MOGRIFY_FILETYPES_RE  = $(subst $(space),|,$(foreach ext,$(ALL_MOGRIFY_FILETYPES),$(ext)))

# Mogrify rules debug disabled by default
# MOGRIFY_DEBUG ?= true

# Pre-requisite make dependencies before running mogrification step
#disabled# MOGRIFY_DEPS  ?=  checkout

# Directories to run mogrification on (default is src)
MOGRIFY_DIRS  ?=  src

# Directories to skip mogrification on
  MOGRIFY_SKIP  :=  src/vendorlibs/BCGSoft
  MOGRIFY_SKIP  +=  src/toe/rsock/user/usb/libusb
  MOGRIFY_SKIP  +=  src/tools/mfgc/vendor
  MOGRIFY_SKIP  +=  src/tools/boost
  MOGRIFY_SKIP  +=  components/router/busybox-1.x
  MOGRIFY_SKIP  +=  components/router/samba
  MOGRIFY_SKIP  +=  src/tools/misc/ce_ttcp
  MOGRIFY_SKIP  +=  src/tools/misc/ethtool-5
  MOGRIFY_SKIP  +=  src/tools/misc/gdb_remote
  MOGRIFY_SKIP  +=  src/tools/misc/iperf
  MOGRIFY_SKIP  +=  src/tools/misc/lzma_src
  MOGRIFY_SKIP  +=  src/tools/misc/netbsd
  MOGRIFY_SKIP  +=  src/tools/misc/udptools
  MOGRIFY_SKIP  +=  src/tools/misc/usbtrace
  MOGRIFY_SKIP  +=  */.svn/*
# MOGRIFY_SKIP  +=  <some_other_dir_to_exclude_from_mogrification>

# Exclude command switch for find command in $(MOGRIFY_RUN)
MOGRIFY_SKIP    += $(MOGRIFY_EXCLUDE)
MOGRIFY_SKIPCMD  = $(patsubst %,-path '%' -prune -o, $(MOGRIFY_SKIP))

# Mogrify utility
MOGRIFY_UTIL ?= perl src/tools/build/mogrify.pl $(if $(MOGRIFY_DEBUG),-debug)
# For backward compatibility for standalone use of mogrify.pl define $(MOGRIFY)
MOGRIFY      ?= $(MOGRIFY_UTIL) $(MOGRIFY_FLAGS)

# DEFS or UNDEFS have to be defined by caller
ifeq ($(findstring -D,$(MOGRIFY_FLAGS))$(findstring -U,$(MOGRIFY_FLAGS))$(MOGRIFY_FLAGS),)
$(error ERROR: MOGRIFY_FLAGS is empty $(MOGRIFY_FLAGS), it needs to be supplied or exported by caller makefile)
endif

# Mogrify command with flags
MOGRIFY_CMD  ?= $(MOGRIFY_UTIL) $(MOGRIFY_FLAGS)

# Output files and dirs (unique for each instance of mogrification)
MOGRIFY_TS     := $(shell date '+%y%m%d_%H%M%S')
MOGRIFY_RUN    := misc/mogrify_run_$(MOGRIFY_TS).mk
MOGRIFY_LIST    = $(MOGRIFY_DIRS:%=%/.mogrified)
MOGRIFY_CHUNKS := misc/mogrify_chunks_$(MOGRIFY_TS)
MOGRIFY_TMPDIR := misc/mogrify_chunks_$(MOGRIFY_TS)_dir
MOGRIFY_CHUNK  := mchunk_$(MOGRIFY_TS)

# If MOGRIFY_LIST is empty, then it is an error and exit
ifeq ($(MOGRIFY_LIST),)
$(error ERROR: MOGRIFY_LIST is empty, no files found to mogrify)
endif

# Number of files in each mogrify chunk
MOGRIFY_CHUNK_SIZE    ?= 50

# Number of parallel instances of mogrification step
# This is effective only for gmake.
# Electric Cloud 'emake' automatically parallelizes
## Never specify -j in a submake
#MOGRIFY_PARALLEL_RUNS ?= 8
#MAKEARGS              ?= $(or $(MAKEJOBS),-j $(MOGRIFY_PARALLEL_RUNS))

## END OF MOGRIFIER DEFINITIONS

# For E.C make commands, reset parallelism flag. It is inherent in emake
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
  ifneq ($(findstring emake,$(MAKE))$(findstring hndmake,$(MAKE)),)
    MAKEARGS =
  endif
endif

## #########################################################################

## START OF COMMON RULES FOR ALL BUILDS

mogrify_all: $(MOGRIFY_DEPS) $(MOGRIFY_LIST) $(MOGRIFY_CHUNKS) $(MOGRIFY_RUN) mogrify_run
	@date +"END: $@, %D %T"

# Process the generated makefile that mogrifies a smaller set of files at
# a time
mogrify_run: $(MOGRIFY_RUN)
	@date +"START: $@, %D %T"
	@if [ ! -s "$(MOGRIFY_RUN)" ]; then \
		echo "ERROR: $(MOGRIFY_RUN) is empty"; \
		exit 1; \
	fi
	# Get the work done now!
	@echo "Mogrify with following command"
	@echo "$(MOGRIFY_CMD)"
	$(MAKE) $(MAKEARGS) -f $(MOGRIFY_RUN)
	@date +"END:   $@, %D %T"

# First generate $(MOGRIFY_LIST)
$(MOGRIFY_LIST): | $(MOGRIFY_DEPS)
	@date +"START: $@, %D %T"
#	# Get a list of mogrifiable text files from MOGRIFY_DIRS
#	# Skip as per MOGRIFY_SKIPCMD or filter as per MOGRIFY_FILELIST
ifdef MOGRIFY_FILELIST
	/usr/bin/find $(MOGRIFY_DIRS) $(MOGRIFY_SKIPCMD) -type f -print  |  \
		$(SRCFILTER) -v $(MOGRIFY_FILELIST) | \
		perl -ne 'print if /($(ALL_MOGRIFY_FILETYPES_RE))$$/i' > $@
else  # MOGRIFY_FILELIST
	/usr/bin/find $(MOGRIFY_DIRS) $(MOGRIFY_SKIPCMD) -type f -print  |  \
		perl -ne 'print if /($(ALL_MOGRIFY_FILETYPES_RE))$$/i' > $@
endif # MOGRIFY_FILELIST
	touch $@
	@date +"END:   $@, %D %T"

# Split $(MOGRIFY_LIST) into small $(MOGRIFY_CHUNKS)_* files
# and list in $(MOGRIFY_CHUNKS) file for $(MOGRIFY_RUN) to process it
$(MOGRIFY_CHUNKS): $(MOGRIFY_LIST)
	@date +"START: $@, %D %T"
	[ -d $(MOGRIFY_TMPDIR) ] || mkdir -pv $(MOGRIFY_TMPDIR)
	echo "Generate mogrify chunks list: $(MOGRIFY_CHUNKS)"
	cd $(MOGRIFY_TMPDIR); rm -f $(MOGRIFY_CHUNK)_*; \
		split $(SPLIT_OPTS) -l $(MOGRIFY_CHUNK_SIZE) \
		$(MOGRIFY_LIST:%=../../%) $(MOGRIFY_CHUNK)_; \
		ls -1 $(MOGRIFY_CHUNK)_* > ../../$@
	@if [ ! -s "$@" ]; then \
		echo "ERROR: $@ is empty"; \
		exit 1; \
	fi
	touch $@
	@date +"END:   $@, %D %T"

# Generate a makefile $(MOGRIFY_RUN) to process mogrify_chunk_xxx created above
$(MOGRIFY_RUN): $(MOGRIFY_DEPS) $(MOGRIFY_LIST) $(MOGRIFY_CHUNKS)
	@date +"START: $@, %D %T"
	echo "Generate a new $@, to split $(MOGRIFY_LIST)"
	@( \
	$(MYECHO) "# Auto-generated at $$(date +'%D %T')\n";        \
	$(MYECHO) "\nMOGRIFY_CMD=$(MOGRIFY_CMD)\n"; \
	$(MYECHO) "\nmogrify_all: \0044(shell cat $(MOGRIFY_CHUNKS) 2> $(NULL))";\
	$(MYECHO) "\n";                                                  \
	$(MYECHO) "$(MOGRIFY_CHUNK)_%:";                                 \
	$(MYECHO) "\t@date +\"--- start: mogrify \0044@ (%D %T)--- \"";  \
	$(MYECHO) "\t@xargs $(if $(MOGRIFY_DEBUG),-t -l1) \\0044(MOGRIFY_CMD) < $(MOGRIFY_TMPDIR)/\0044@";  \
	$(MYECHO) "#\t@rm -fv $(MOGRIFY_TMPDIR)/\0044@";                 \
	$(MYECHO) "\t@date +\"--- end  : mogrify \0044@ (%D %T)--- \"";  \
	$(MYECHO) "\n";                                                  \
	) > $@
	touch $(MOGRIFY_RUN)
	@date +"END:   $@, %D %T"

# Show info on mogrification variables
show_mogrify_info:
	@echo "Mogrify file types  = $(ALL_MOGRIFY_FILETYPES)"
	@echo "Mogrify custom types= $(MOGRIFY_FILETYPES)"
	@echo "Mogrify skip dirs   = $(MOGRIFY_SKIP)"
	@echo "Mogrify chunks      = $$(cat $(MOGRIFY_CHUNKS) 2> $(NULL))"

# Targets for on-demand selective mogrification by scripts or by developers
mogrify_one_file:
	echo $(MOGRIFIER_TARGET_FILE) > src/.mogrified
	$(MAKE) -f src/tools/release/mogrify_common_rules.mk

# Run only mogrification step in any workspace
mogrify_only:
	$(MAKE) -f src/tools/release/mogrify_common_rules.mk
	cp src/.mogrified src/.to_mogrify

.PHONY: mogrify_one_file mogrify_only

else  # MOGRIFY_COMMON_RULES_DISABLED

# Keep this from being passed through to child builds.
unexport MOGRIFY_COMMON_RULES_DISABLED =

endif # MOGRIFY_COMMON_RULES_DISABLED
