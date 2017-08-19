## #########################################################################
## #
## # Copyright 2000, Broadcom Corporation.
## #
## # This makefile contains miscelleneous configs, macros and targets
## # needed to do release brand builds. This is copy of old swrls.mk
## #
## # This file is intended to be included in individual release.mk files
## #
## # Author: Prakash Dhavali
## #
## # $Id$
## #########################################################################

include WLAN.usf

# List of dirs to skip from build workspace creation and packaging steps
SKIP_PATHS   :=  CVS
SKIP_PATHS   +=  \.svn

# List of dirs to skip from grep
GREP_SKIPCMD :=  grep -v "/CVS[/]*$$" |
GREP_SKIPCMD +=  grep -v "/\.svn[/]*$$"

# List of dirs to skip from tar
TAR_SKIPCMD  :=  $(SKIP_PATHS:%=--exclude=*/%)

# List of dirs to skip from pax
PAX_SKIPCMD  :=  $(SKIP_PATHS:%=-s "/.*\/%\/.*//g")
PAX_SKIPCMD  +=  $(SKIP_PATHS:%=-s "/.*\/%$$//g")

# List of dirs to skip from find
FIND_SKIPCMD :=  $(SKIP_PATHS:%=-path '%' -prune -o)
FIND_SKIPCMD +=  $(SKIP_PATHS:%=-name '%' -prune -o)

# Make target start and end markers
override MARKSTART    = date +"[%D %T] MARK-START: $@"
override MARKEND      = date +"[%D %T] MARK-END  : $@"

# Whole build brand start and end markers. BRAND is defined
# mandatorily by build scripts and TAG is optional
override MARKSTART_BRAND = date +"[%D %T] MARK-START: BRAND=$(BRAND) $(if $(TAG),TAG=$(TAG))"
override MARKEND_BRAND   = date +"[%D %T] MARK-END  : BRAND=$(BRAND) $(if $(TAG),TAG=$(TAG))"

#to-do# PWD                 := $(shell pwd)
#to-do# BUILD_BASE          := $(PWD)
#to-do# RELEASEDIR          := $(BUILD_BASE)/release
#to-do# UNAME               := $(shell uname -a)
#to-do# NULL                := /dev/null
#to-do# DEFAULT_TARGET      := default
#to-do# ALL_TARGET          := all

#to-do# MOGRIFY_RULES_MK    := mogrify_common_rules.mk
#to-do# FEATURE_LIST_MK     := src-features-master-list.mk
#to-do# UNRELEASED_CHIPS_MK := unreleased-chiplist.mk
#to-do# DONGLE_RULES_MK     := linux-dongle-image-launch.mk

#to-do# # Derive RELNUM and/or RELDATE from given TAG or BRANCH
#to-do# empty:=
#to-do# space:= $(empty) $(empty)
#to-do# comma:= $(empty),$(empty)
#to-do#
#to-do# ifneq ($(origin TAG), undefined)
#to-do#     export TAG
#to-do#     vinfo := $(subst _,$(space),$(TAG))
#to-do# else
#to-do#     vinfo := $(shell date '+D11 REL %Y %m %d')
#to-do# endif
#to-do#
#to-do# maj:=$(word 3,$(vinfo))
#to-do# min:=$(word 4,$(vinfo))
#to-do# rcnum:=$(word 5,$(vinfo))
#to-do# rcnum:=$(patsubst RC%,%,$(rcnum))
#to-do# ifeq ($(rcnum),)
#to-do#   rcnum:=0
#to-do# endif
#to-do# incremental:=$(word 6,$(vinfo))
#to-do# ifeq ($(incremental),)
#to-do#   incremental:=0
#to-do# endif
#to-do# RELNUM  := $(maj).$(min).$(rcnum).$(incremental)
#to-do# RELDATE := $(shell date '+%m/%d/%Y')

#to-do# include $(FEATURE_LIST_MK)
#to-do# include $(UNRELEASED_CHIPS_MK)
#to-do# include $(DONGLE_RULES_MK)
#to-do# include $(MOGRIFY_RULES)

ifeq ($(DEBUG),1)
show_skipcmds:
	@echo "======================================"
	@echo "GREP_SKIPCMD = $(GREP_SKIPCMD)"
	@echo "TAR_SKIPCMD  = $(TAR_SKIPCMD)"
	@echo "PAX_SKIPCMD  = $(PAX_SKIPCMD)"
	@echo "FIND_SKIPCMD = $(FIND_SKIPCMD)"
	@echo "======================================"
endif # DEBUG
