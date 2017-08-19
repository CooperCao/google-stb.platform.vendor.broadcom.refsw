#
# Build the windows wireless drivers and tools
#
# $Id: win10_internal_wl.mk 594886 2015-10-24 01:12:19Z $
#
# HOW THIS MAKEFILE WORKS
#
# The release consists of 6 stages:
#
# 1. Checkout source from SVN.
#
# 2. Run the transmogrifier on all source code to remove some selected
# defines and to force the expression of others. Also some comments
# undefined by the transmogrifier are in $(UNDEFS). The symbols to be
# defined by the transmogrifier are in $(DEFS).
#
# 3. Build binaries includes driver
#
# 4. copy binary files to release folder
#
# There is a convenient target [driver] to save time for testing build.  
# This SHOULD NOT be released.
#

empty  :=
space  := $(empty) $(empty)
comma  := $(empty),$(empty)

PARENT_MAKEFILE :=
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

DATE                = $(shell date -I)
BUILD_BASE          = $(shell pwd)
RELEASEDIR          = $(BUILD_BASE)/release
SHELL               = bash.exe
FIND                = /bin/find
INSTALL            := install -p
MKDIR              := mkdir -pv
MAKE_MODE          := unix
NULL               := /dev/null
BRAND              ?= win10_internal_wl
RETRYCMD           ?= $(firstword \
                      $(wildcard \
                       Z:/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                   ))

export VS_VER      := 2015
VERSION_NAME       := Win10
WIN10_WDK_VERSION  ?= 10240
WIN10_WDK_INSTALL   ?= $(WIN10_WDK_VERSION)wdksdk
WIN10_WDK_LOCAL     ?= c:/tools/msdev
WIN10_WDK_GLOBAL    ?= z:/projects/hnd/tools/win/msdev
WIN10_WDK_ROOT      := $(firstword \
                         $(wildcard $(WIN10_WDK_LOCAL)/$(WIN10_WDK_INSTALL) \
                                    $(WIN10_WDK_GLOBAL)/$(WIN10_WDK_INSTALL) ) \
                         Makefile-Did-Not-Find-$(WIN10_WDK_INSTALL)-Error)
export WIN10_WDK_ROOT

WARN_FILE    := _WARNING_PARTIAL_BUILD_DO_NOT_USE

UPPER_VERSION_NAME = $(subst $(space),$(empty), $(call .toupper, $(VERSION_NAME)))
LOWER_VERSION_NAME = $(subst $(space),$(empty), $(call .tolower, $(VERSION_NAME)))

WINOS=$(UPPER_VERSION_NAME)

export MOGRIFY_RULES     = mogrify_common_rules.mk
export BRAND_RULES       = brand_common_rules.mk

# Win10 wl drivers and inf
WIN10_x86_WLDRIVER = src/wl/sys/wdm/obj/win10_nic/checked/x86/bcmwl63.sys
WIN10_x64_WLDRIVER = src/wl/sys/wdm/obj/win10_nic/checked/x64/bcmwl63a.sys
WIN10_WLINF          = bcmwl63.inf

# App brands and variants to build and package
export OEM_LIST := bcm

ifneq ($(origin TAG), undefined)
    export TAG
    vinfo := $(subst _,$(space),$(TAG))
else
    vinfo := $(shell date '+D11 REL %Y %m %d')
endif

maj:=$(word 3,$(vinfo))
min:=$(word 4,$(vinfo))
rcnum:=$(word 5,$(vinfo))
rcnum:=$(patsubst RC%,%,$(rcnum))
ifeq ($(rcnum),)
rcnum:=0
endif
incremental:=$(word 6,$(vinfo))
ifeq ($(incremental),)
incremental:=0
endif
RELNUM  := $(maj).$(min).$(rcnum).$(incremental)
RELDATE := $(shell date '+%m/%d/%Y')

# These symbols will be UNDEFINED in the source code by the transmogirifier
#
UNDEFS= CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
        CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
        CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
        INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ \
        VX_BSD4_3 ROUTER BCMENET BCM4710A0 CONFIG_BCM4710 \
        CONFIG_MIPS_BRCM POCKET_PC DSLCPE LINUXSIM BCMSIM \
        BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 WLNOKIA

# These symbols will be DEFINED in the source code by the transmogrifier
#
DEFS=BCM47XX BCM47XX_CHOPS BCMINTERNAL

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

SIGNTOOL_TIME  := "http://timestamp.verisign.com/scripts/timstamp.dll"
SIGNTOOL_OPTS  ?= /a /v /s my /n "Broadcom Corporation" /t $(SIGNTOOL_TIME)
SIGNTOOL_CMND_32  := $(RETRYCMD) $(WIN10_WDK_ROOT)/bin/x86/signtool sign $(SIGNTOOL_OPTS)
SIGNTOOL_CMND_64  := $(RETRYCMD) $(WIN10_WDK_ROOT)/bin/x64/signtool sign $(SIGNTOOL_OPTS)
export SIGNTOOL_OPTS

INS_DIR        := components/apps/windows/install/app
INF_DIR        := $(INS_DIR)/installshield/BCMAppInstaller/is_bin

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $@/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $@/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

# Arg listing: $1=Driver-folder $2=CPU
define RELEASE_SIGN_WIN10_DRIVER
	@echo "#- $0($1,$2,$3)"

	$(MAKE) -C src/wl/sys/wdm release_sign_driver \
		SIGN_DIR=$1 SIGN_ARCH=$2 RELNUM="$(RELNUM)" \
		WINOS=$(WINOS) WIN10_WDK_VER=$(WIN10_WDK_VERSION)

endef # RELEASE_SIGN_WIN10_DRIVER

# Copy and timestamp inf files
define INSTALL_INF
	@echo "#- $0"
	$(INSTALL) $1 $2
	src/tools/release/wustamp -o -r -v "$(RELNUM)" -d $(RELDATE) "`cygpath -w $2`"
endef  # INSTALL_INF

all: build_start mogrify pre_release
all: build_driver 
all: build_install 
all: release 
all: build_end

include $(MOGRIFY_RULES)
include $(BRAND_RULES)
include unreleased-chiplist.mk

unexport VSINSTALLDIR

build_start:
	@$(MARKSTART_BRAND)

# All exported MOGRIFY_xxx variables are referenced by following steps
mogrify:
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

build_include: mogrify
	@$(MARKSTART)
	$(MAKE) -C src/include
	@$(MARKEND)

build_driver: mogrify build_include
	@$(MARKSTART)
	
	$(MAKE) -C src/wl/sys/wdm WIN10_WDK_VER=$(WIN10_WDK_VERSION) \
		WINOS=$(UPPER_VERSION_NAME) BUILD_TYPES=checked \
		$(if $(strip ${SERVER_PATH}),SERVER_PATH=$(strip ${SERVER_PATH})) \
		build_win10_driver
	
	$(MAKE) -C src/wl/exe -f wl_vs13.mk BUILD_CONFS=Debugwb all

	@$(MARKEND)

build_install: export VS_PRE_BUILD_ENABLED=0
build_install: mogrify
	@$(MARKSTART)
	$(MAKE) -C $(INS_DIR) $(if $(VERBOSE),VERBOSE=1) BRAND=$(BRAND)
	@$(MARKEND)

clean: profile.log
	@$(MARKSTART)
	$(MAKE) -C src/include clean
	$(MAKE) -C $(INS_DIR) clean
	$(MAKE) -C src/wl/exe -f wl_vs13.mk clean_all
	$(MAKE) -C src/wl/sys/wdm clean
	@$(MARKEND)

# For clustered builds, these release folders need to be pre-existing
# to ensure that individual <oem> specific release/<oem> make targets 
# run independently in parallel builds
pre_release:
	@$(MARKSTART)
	@[ -d release ] || mkdir -pv release
	@[ -d release/$(VERSION_NAME)/checked/DriverOnly ] || \
		mkdir -pv release/$(VERSION_NAME)/checked/DriverOnly
	@$(MARKEND)

release: release_win10
release_win10: pre_release
release_win10: $(RELEASEDIR)/$(VERSION_NAME)/checked/DriverOnly

##############################################################################
# release_win10 for Win10
##############################################################################

## Win10 - internal release driver-only
$(RELEASEDIR)/$(VERSION_NAME)/checked/DriverOnly: FORCE
	@$(MARKSTART)
	mkdir -pv $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(WIN10_x86_WLDRIVER) $@/
	$(INSTALL) $(WIN10_x64_WLDRIVER) $@/
	$(INSTALL) $(subst .sys,.pdb,$(WIN10_x64_WLDRIVER)) $@/
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl63,bcmwl63i,$(WIN10_WLINF)),$@/$(WIN10_WLINF))
	$(call RELEASE_SIGN_WIN10_DRIVER,$@,X86)
	$(call RELEASE_SIGN_WIN10_DRIVER,$@,X64)
	$(INSTALL) $(subst .sys,.map,$(WIN10_x86_WLDRIVER)) $@/
	$(INSTALL) $(subst .sys,.pdb,$(WIN10_x86_WLDRIVER)) $@/
	$(INSTALL) $(subst .sys,.map,$(WIN10_x64_WLDRIVER)) $@/
	$(INSTALL) src/wl/exe/Debugwb/wl.exe $@/
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win10/checked/DriverOnly

$(RELEASEDIR)/% :
	mkdir -p $@

#pragma runlocal
build_clean: release
	@$(MARKSTART)
	-@$(FIND) src components -type f -name "*\.obj" -o -name "*\.OBJ" -o -name "*\.o" -o -name "*\.O" | \
		xargs -P20 -n100 rm -f
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)

.PHONY: pre_release build_driver mogrify build_install build_include build_end FORCE



################################################################################
## Simple text substitution utilities to avoid excessive $(shell ) calls.
## Only usable for = and not := due to position at the end of this file
################################################################################

.tolower = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))
.toupper = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))
