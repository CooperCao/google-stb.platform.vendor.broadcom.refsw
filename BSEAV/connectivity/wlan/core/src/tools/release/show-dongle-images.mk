####################################################################
#
# Show Dongle Images used or needed for a TOB or TAG or TOT build
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$

####################################################################
UNAME   := $(shell uname -s)
NULL    := /dev/null

DNGLSRCH     := $(or $(CVSSRC),/projects/hnd_software/gallery)
ifneq ($(findstring CYGWIN,$(UNAME)),)
    DNGLSRCH   := Z:$(DNGLSRCH)
endif

SKIP        := hndrte-dongle-wl linux-olympic-dongle-src
BUILDABLES  := $(or $(BRANDS),$(filter-out $(SKIP),$(shell hnd query enabled_buildables -t $(or $(TAG),trunk))))

# Many dhd or dongle targets can use this showimages to build only
# needed dongle images. Building all possible images takes a lot of
# time. So this target is called automatically
# Any commented line is filtered out automatically.
.PHONY: showimages
showimages:
	@echo -e "# Scanning buildables in $(DNGLSRCH):\n# $(BUILDABLES)" >&2; \
	for brand in $(BUILDABLES); do \
	  mkfile=$(DNGLSRCH)/src/tools/release/$${brand}.mk; \
	  if [ -s "$$mkfile" ]; then \
	    echo -e "include $$mkfile\nshowmyimages:; echo \0044(ALL_DNGL_IMAGES)" | \
	      make -s -f - -I $(DNGLSRCH)/src/tools/release \
	      BRAND=$${brand} SHELL=/bin/bash UNAME= showmyimages 2> $(NULL); \
	  fi; \
	done | fmt -1 | grep "^4" | sort -u

.PHONY: help
help:
	@echo "========================================================"
	@echo "By default this makefile shows dongle images for TOT"
	@echo "To show dongle images used in other branches"
	@echo "point DNGLSRCH below to a checkout from that branch as follows:"
	@echo "(If you don't have a checkout point to a branch build dir)"
	@echo ""
	@echo "Examples:"
	@echo "1. To find dongle images used on IGUANA_BRANCH_13_10"
	@echo "   make -f$(MAKEFILE_LIST) DNGLSRCH=/projects/hnd/swbuild/build_linux/IGUANA_BRANCH_13_10/linux-internal-dongle/<date>"
	@echo ""
	@echo "2. To find dongle images used on IGUANA_BRANCH_13_10"
	@echo "   make -f$(MAKEFILE_LIST) DNGLSRCH=/projects/hnd/swbuild/build_linux/IGUANA_BRANCH_13_10/linux-internal-dongle/<date>"
	@echo ""
	@echo "3. To find dongle images used on branch you are on"
	@echo "   make -f$(MAKEFILE_LIST) DNGLSRCH=<base-of-checkout>"
	@echo ""
	@echo "4. To find dongle images used on TOT"
	@echo "   make -f$(MAKEFILE_LIST)"
	@echo "========================================================"
