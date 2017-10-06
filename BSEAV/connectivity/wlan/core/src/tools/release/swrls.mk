## #########################################################################
## #
## # Copyright 2000, Broadcom Corporation.
## #
## # This makefile contains miscelleneous configs, macros needed to
## # release hnd wlan software
## #
## # This file is intended to be included in individual release.mk files
## # (e.g. mogrify details)
## #
## # Author: Prakash Dhavali
## # Contact: hnd-software-scm-list
## #
## # $Id$
## #########################################################################

# Following are superseeded by creation of central mogrify_rules.mk file
# TO-DO: Gradually eliminate all these mogrification settings from this
# TO-DO: file. This file will be used for other Release Specific common defs

ifndef COMMON_MOGRIFY_FILETYPES
  # These are file extensions that should be mogrified at-least
  # (using the parameters specified in DEFS and UNDEFS)
  COMMON_MOGRIFY_FILETYPES  = [^\/]*\.c
  COMMON_MOGRIFY_FILETYPES += [^\/]*\.h
  COMMON_MOGRIFY_FILETYPES += [^\/]*\.mk
  COMMON_MOGRIFY_FILETYPES += [^\/]*\.inc
  COMMON_MOGRIFY_FILETYPES += [^\/]*\.s
  COMMON_MOGRIFY_FILETYPES += [^\/]*\.tcl
  COMMON_MOGRIFY_FILETYPES += [^\/]*\.cpp
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
endif  # COMMON_MOGRIFY_FILETYPES

ifndef MOGRIFY_SKIP
  MOGRIFY_SKIP  :=  src/vendorlibs/BCGSoft
  MOGRIFY_SKIP  +=  src/tools/mfgc/vendor
  MOGRIFY_SKIP  +=  components/router/libid3tag
  MOGRIFY_SKIP  +=  components/router/ffmpeg
  MOGRIFY_SKIP  +=  components/router/samba
  MOGRIFY_SKIP  +=  components/router/alsa-utils
  MOGRIFY_SKIP  +=  components/router/libmad
  MOGRIFY_SKIP  +=  components/router/madplay
  MOGRIFY_SKIP  +=  components/router/salsa
  MOGRIFY_SKIP  +=  */.svn/*
# MOGRIFY_SKIP  +=  src/wl/cpl/BCGCBPRO
endif  # MOGRIFY_SKIP

ifndef MOGRIFY_EXCLUDE
  MOGRIFY_EXCLUDE  = $(patsubst %,-path '%' -prune -o, $(MOGRIFY_SKIP))
endif  # MOGRIFY_EXCLUDE

# List of directories skip from build workspace creation and packaging
# steps
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

#show_skipcmds:
#	@echo "GREP_SKIPCMD = $(GREP_SKIPCMD)"
#	@echo "TAR_SKIPCMD  = $(TAR_SKIPCMD)"
#	@echo "PAX_SKIPCMD  = $(PAX_SKIPCMD)"
#	@echo "FIND_SKIPCMD = $(FIND_SKIPCMD)"

#show_mogrify_ext:
#	@for ext in $(COMMON_MOGRIFY_FILETYPES); do \
#		echo "$$ext"; \
#	done
