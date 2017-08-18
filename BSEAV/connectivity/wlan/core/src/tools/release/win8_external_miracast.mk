# Build screenovate for mircast aupport
#
# $Id: win8_external_mircast.mk 396460 2013-04-12 11:33:43Z $
#
# This makefile is a place holder that does as little as necessary to driver
# screenovate builds. It also removes all of the on-disk source code to avoid
# any accidental copy of sources to the network, protecting IP.
#
#
empty :=
space := $(empty) $(empty)
comma := $(empty),$(empty)

all:         make_sources_dir checkout build         rm_sources_dir
all_native:  make_sources_dir checkout build_native  rm_sources_dir
all_peernet: make_sources_dir checkout build_peernet rm_sources_dir

DATE                = $(shell date -I)
BUILD_BASE          = $(shell pwd)
SHELL               = /usr/bin/bash.exe --noprofile
BRAND               ?= win8_external_mircast

WINVERSION := Win8
UPPER_WINVERSION = $(call .toupper,$(WINVERSION))
LOWER_WINVERSION = $(call .tolower,$(WINVERSION))

CURDIR := `pwd`
SOURCES_DIR ?= $(CURDIR)_src

MARKSTART = date +"[%D %T] MARK-START: $@"
MARKEND   = date +"[%D %T] MARK-END  : $@"

REMOTE_MK ?= release.mk

make_sources_dir:
	mkdir -pv $(SOURCES_DIR)

rm_sources_dir:
	rm -rvf $(SOURCES_DIR)

SVN_URL ?= http://svn.sj.broadcom.com/svn/wlansvn/components/vendor/screenovate/scremote

checkout:
	@$(MARKSTART)
	cd $(SOURCES_DIR); \
           svn export --non-interactive $(SVN_URL)/$(LOWER_WINVERSION)/screenovate.mk  $(REMOTE_MK)
	@$(MARKEND)

# Screenovate makefile has all all_peernet and all_native targets
# They will place the build results into a /release folder under RELEASE_DIR
# By placing them  back int he build_

build build_native build_peernet:
	cd $(SOURCES_DIR); \
		$(MAKE) -w -f $(REMOTE_MK) TAG=$(TAG) BRAND=$(BRAND) RELEASE_DIR="$(PWD)" $(@:build%=all%)

.PHONY: FORCE
.PHONY: make_sources_dir checkout build_native build_peernet build

.tolower = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

.toupper = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))
