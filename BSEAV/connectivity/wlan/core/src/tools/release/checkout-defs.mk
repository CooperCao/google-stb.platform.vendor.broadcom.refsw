#
# Central CVS or SVN makefile defs
#
# This is intended to be included by release makefiles and checkout
# rules
#
# Author: Prakash Dhavali
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

empty :=
space := $(empty) $(empty)

SVNROOT ?= http://svn.sj.broadcom.com/svn/wlansvn/proj
SVNTZ   ?= $(shell date '+%z')
WLANSVN ?= http://svn.sj.broadcom.com/svn/wlansvn

# NOTE: WLAN Production Repository - but accessed from alternate server
# WARN: Talk to SVN Admins before using this server
# WARN: Using this server needs credentials to be cached
# export SVNROOT="http://engcm-sj1-06.sj.broadcom.com/svn/wlansvn/proj"

#
# On windows platform set path before calling hndcvs or sparse
# TODO: These are temporary changes, until we have a wrapper for
# TODO: sparse that configures its path before launching itself
#
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
   CYGDIR := $(or $(CYGWIN_DIRECTORY),tools/win32)

   # Some PATH additions are used only for older Cygwins.
   # This conditional may seem strange but we need to be careful
   # about case ("Tools/Win32") and Cygwin 2.x.
   ifeq ($(findstring /,$(CYGDIR)),/)
     $(info INFO: Configuring for old Cygwin ($(CYGDIR)))
     # Sparse checkout on Cygwin 1.5 can't use SVN version from cygwin dist.
     # So use collabnet 1.6.x ver
     # $(SVNSRVDIR) need to expand to a dir that has svn.exe
     SVNSRVDIR    := Z:/projects/hnd_tools/win/Subversion
     SVNDIR        = $(strip $(firstword $(wildcard \
 			  C:/tools/Subversion \
 			  $(SVNSRVDIR))))
     # Sparse checkout on Cygwin 1.5 can't use Python version from cygwin dist.
     # So use standalone ver
     PYTHONSRVDIR := Z:/projects/hnd_tools/win/Python
     PYTHONDIR     = $(strip $(firstword $(wildcard \
 			  C:/tools/Python \
 			  $(PYTHONSRVDIR))))

     HNDSVN_CMD ?= sparse.bat
   else
     $(info INFO: Configuring for modern Cygwin ($(CYGDIR)))
     HNDSVN_CMD ?= sparse.sh
   endif

  BLDSCRIPTSSRVDIR = Z:/home/hwnbuild/src/tools/build
  BLDSCRIPTSDIR = $(strip $(firstword $(wildcard \
                        C:/tools/build \
                        $(BLDSCRIPTSSRVDIR) \
                        Z:/projects/hnd_software/gallery-svn/release \
                        Z:/projects/hnd_software/gallery/src/tools/build)))

  ifeq ($(findstring sparse_dbg,$(HNDSVN_CMD)),)
    SPARSECODIR  = Z:/projects/hnd_software/gallery-svn/release
  else
    SPARSECODIR  = Z:/projects/hnd_software/sparse_rev_patch
  endif

  # In this directory hnd_depot_tools is updated with SVN/GIT copy periodically
  HND_DEPOT_TOOLS = Z:/projects/hnd_software/gallery-gclient/hnd_depot_tools

  HNDCVS_PATH = /usr/bin:$(shell cygpath -p "$(BLDSCRIPTSDIR);$(BLDSCRIPTSSRVDIR)"):$(PATH)
  HNDSVN_PATH = $(shell cygpath -p "$(SVNDIR);$(SVNSRVDIR);$(PYTHONDIR);$(PYTHONSRVDIR);$(BLDSCRIPTSDIR);$(BLDSCRIPTSSRVDIR);$(SPARSECODIR)"):/usr/bin:$(PATH)
  HNDGC_PATH = $(shell cygpath -p "$(SVNDIR);$(SVNSRVDIR);$(PYTHONDIR);$(PYTHONSRVDIR);$(BLDSCRIPTSDIR);$(BLDSCRIPTSSRVDIR);$(HND_DEPOT_TOOLS)"):/usr/bin:$(PATH)
  HNDGC_CMD  ?= hnd_gclient

else # Unix/linux platforms

  BLDSCRIPTSDIR = $(strip $(firstword $(wildcard \
                        /home/hwnbuild/src/tools/build \
                        /projects/hnd_software/gallery/src/tools/build)))

  # In this directory hnd_depot_tools is updated with SVN/GIT copy periodically
  HND_DEPOT_TOOLS = /projects/hnd_software/gallery-gclient/hnd_depot_tools

  # Unfortunately /tools/bin is not supported everywhere and is often
  # present even when not supported. Currently supported only on RedHat.
  # Even though it passes along to the native utility where not supported,
  # better to keep it off PATH to retain control over utility versions.
  ifeq (,$(wildcard /etc/redhat-release))
    HNDCVS_PATH = $(BLDSCRIPTSDIR):$(PATH)
    HNDSVN_PATH = $(BLDSCRIPTSDIR):$(PATH)
    HNDGC_PATH  = $(HND_DEPOT_TOOLS):$(PATH)
  else
    HNDCVS_PATH = /tools/bin:$(BLDSCRIPTSDIR):$(PATH)
    HNDSVN_PATH = /tools/bin:$(BLDSCRIPTSDIR):$(PATH)
    HNDGC_PATH  = /tools/bin:$(HND_DEPOT_TOOLS):$(PATH)
  endif

  HNDSVN_CMD ?= sparse.sh
  HNDGC_CMD  ?= hnd_gclient

endif # UNAME

# Ensure gclient doesn't try to update hnd_depot_tools for centralized builds
export DEPOT_TOOLS_UPDATE=0

HNDCVS_CMD ?= hndcvs
HNDCVS      = PATH="$(HNDCVS_PATH)" && $(HNDCVS_CMD)
HNDSVN      = PATH="$(HNDSVN_PATH)" && $(HNDSVN_CMD) --version && $(HNDSVN_CMD)
HNDGC       = PATH="$(HNDGC_PATH)"  && $(HNDGC_CMD)

# Any of these keywords, mean it is trunk/tot
ifneq ($(filter HEAD TOT NIGHTLY DAILY TRUNK,$(TAG)),)
    TAG=
endif

# svn co -r <svnrev> captured here. It can contain svn rev number
# or checkout cutoff date/time
SVNREVINFO=$(if $(SVNREV),-r $(SVNREV),$(if $(SVNCUTOFF),-r {"$(SVNCUTOFF) $(SVNTZ)"}))

# Derive gclient cutoff time from svn cutoff time
GCLIENTCUTOFF=$(if $(SVNCUTOFF),$(subst $(space),T,$(SVNCUTOFF))$(SVNTZ))
GCLIENTREVINFO=$(if $(GCLIENTCUTOFF),--transitive --revision {$(GCLIENTCUTOFF)})
# $(info GCLIENT REVINFO = $(GCLIENTREVINFO))

# Non-existant TAG means it is trunk
ifeq ($(TAG),)
    REPO_URL = $(SVNREVINFO) $(SVNROOT)/trunk/
    DEPS_URL = $(WLANSVN)/components/deps/trunk/
endif

# If TAG is explicitly set as trunk
ifeq ($(TAG),trunk)
    REPO_URL = $(SVNREVINFO) $(SVNROOT)/trunk/
    DEPS_URL = $(WLANSVN)/components/deps/trunk/
endif

# Tags are group with tagname prefix as dirname
ifneq ($(findstring _REL_,$(TAG)),)
    TAG_PREFIX := $(firstword $(subst _,$(space),$(TAG)))
    REPO_URL = $(SVNREVINFO) $(SVNROOT)/tags/$(TAG_PREFIX)/$(TAG)
    DEPS_URL = $(WLANSVN)/components/deps/tags/$(TAG_PREFIX)/$(TAG)
endif

# Branches don't have prefix
ifneq ($(findstring _BRANCH_,$(TAG))$(findstring _TWIG_,$(TAG)),)
    REPO_URL = $(SVNREVINFO) $(SVNROOT)/branches/$(TAG)
    DEPS_URL = $(WLANSVN)/components/deps/branches/$(TAG)
endif
