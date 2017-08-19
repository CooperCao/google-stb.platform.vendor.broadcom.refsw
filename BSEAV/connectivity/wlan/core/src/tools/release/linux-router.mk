#
# BCM947xx Linux Router build/release Makefile
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

DATE := $(shell date -I)
BUILD_BASE := $(shell pwd)
RELEASEDIR := $(BUILD_BASE)/release
empty      :=
space      := $(empty) $(empty)
comma      := $(empty),$(empty)

ROUTER_OS ?= linux
ARCH      ?= mips
PLT       ?= mipsel

# Add platfrom ARM identifier
ifeq ($(findstring -arm-,$(BRAND)),-arm-)
ARCH = arm
PLT  = arm
endif

SRCFILELISTS_COMPONENTS += src/tools/release/components/wlphy-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/wlolpc-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/bcmcrypto-filelist.txt

# Mogrifier exclude for components
MOGRIFY_EXCLUDE_COMPONENTS =
SRCDIRS = src
ifeq ("$(ROUTER_OS)", "linux-2.6.36")
  SRCFILELIST_ORG = src/tools/release/linux-2.6.36-router-filelist.txt
  SRCFILELIST = linux-2.6.36-router-filelist.txt
  ifeq ($(PLT),arm)
    PATH := /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin:$(PATH)
  else
    PATH := /projects/hnd/tools/linux/hndtools-mipsel-linux-2.6.36-uclibc-4.3.6/bin:$(PATH)
  endif
  CONFIG_GLIBC := false
  LINUX_VERSION = 2_6_36
  LINUX_DIR = components/opensource/linux/linux-2.6.36
  GPL_FILELIST = src/tools/release/linux-2.6.36-gpl-filelist.txt
  SRCDIRS += components
  MOGRIFY_EXCLUDE_COMPONENTS = components/opensource/dnsmasq components/opensource/netfilter components/opensource/xz
else
  SRCFILELIST_ORG = src/tools/release/linux-router-filelist.txt
  SRCFILELIST = linux-router-filelist.txt
  PATH := $(PATH):/projects/hnd/tools/linux/hndtools-mipsel-linux/bin
  LINUX_VERSION = 2_4
  LINUX_DIR = src/linux/linux
  GPL_FILELIST = src/tools/release/gpl-filelist.txt
endif

# Tell mogrifier what directories need to check
MOGRIFY_DIRS = $(SRCDIRS)

#
# HOW THIS MAKEFILE WORKS
#
# The release consists of 8 stages:
#
# 1. Checkout source from CVS to src/. The directories and modules to
# check out are in $(HNDCVS_BOM).
#
# 2. Run the transmogrifier on all source code to remove some selected
# defines and to force the expression of others. Also some comments
# undefined by the transmogrifier are in $(UNDEFS). The symbols to be
# defined by the transmogrifier are in $(DEFS).
#
# 3. Copy selected portions of the source into a new customer release
# directory release/. As much as possible, exclude source files
# unrelated to this release and CVS cruft. This is done in the rule
# 'release_src'. The list of files to be released is constructed from
# linux-router-filelist.txt and cfe-filelist.txt
#
# 4. Prebuild binaries under src/ that are not to be released to the
# customer. Copy these binaries to the customer source release
# directory release/src/. Re-mogrify all text files to remove code
# bounded by PREBUILD.
#
# 5. Copy the customer source release directory release/ to
# build/.
#
# 6. Build the copy of the customer source release by running gmake
# under build/src/.
#
# 7. Copy select prebuilt images out of build/ into release/.
#
# 8. Archive release directory, tar all files under release/, filter
#    more files using release.sh script. Delete the original release
#    directory, remake it and just keep the tar file. Also delete the
#    original prebuild src/ directory. Test build the tar file above

# These are module names and directories that will be checked out of CVS.

ifeq ("$(ROUTER_OS)", "linux-2.6.36")
  HNDSVN_BOM ?= linux2636-router.sparse
else ifeq ("$(ROUTER_OS)", "linux-2.6")
  HNDSVN_BOM ?= linux26-router.sparse
else
  HNDSVN_BOM ?= linux-router.sparse
endif

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_OPENSOURCE_FLAGS = $(MOGRIFY_OPENSOURCE_UNDEFS:%=-U%) $(MOGRIFY_OPENSOURCE_DEFS:%=-D%)
export MOGRIFY_EXCLUDE   = $(MOGRIFY_EXCLUDE_COMPONENTS)
# Custom mogrification filetypes for routers
export MOGRIFY_FILETYPES+= [^\/]*\.asp
export MOGRIFY_FILETYPES+= [^\/]*defconfig.*

# These symbols will be UNDEFINED in the source code by the transmogrifier
UNDEFS += \
CONFIG_BCM93725 CONFIG_BCM93725_VJ CONFIG_BCM93730 CONFIG_BCM933XX \
BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ VX_BSD4_3 \
MICROSOFT MSFT vxworks DSLCPE LINUXSIM BCMSIM BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 \
_RTE_ __ARM_ARCH_4T_ WLNOKIA BCM_APSDSTD BCMDONGLEHOST \
BCMNODOWN BCMROMOFFLOAD BCMSDIODEV WLPFN_AUTO_CONNECT BCMCCX BCMEXTCCX \
WLFIPS EXT_STA BCMWAPI_WPI BCMWAPI_WAI BCMP2P \
FULL BCMVOIP __CONFIG_PPTP__ __CONFIG_L2TP__ BCMASSERT_LOG BCM_MFGTEST

# Unreleased-chiplist is included only for non-internal builds
ifeq ($(findstring BCMINTERNAL,$(DEFS))$(findstring internal,$(BRAND)),)
include unreleased-chiplist.mk
endif # BCMINTERNAL

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

# These symbols will be DEFINED in the source code by the transmogrifier
DEFS += \
CONFIG_MIPS_BRCM BCM47XX_CHOPS BCM4710A0 BCM47XX BCMENET __CONFIG_SES__ __CONFIG_SES_CL__ LINUX ROUTER \
BCMEMF LINUX_ROUTER

ifeq ($(PLT),arm)
DEFS += PLTARM
endif

# These symbols will be defined for opensource distribution.
MOGRIFY_OPENSOURCE_DEFS += $(empty)

# List of open source directories to mogrify (limited scope to minimize overhead)
MOGRIFY_OPENSOURCE_DIRS += components/router

# Source mogrification command
MOGRIFY = perl src/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES) src\/router\/config\/Config
SRCFILTER = perl src/tools/build/srcfilter.pl

# Add "release/" prefix for prebuild mogrify.
RELEASE_MOGRIFY_EXCLUDE = $(patsubst %,-path 'release/%' -prune -o, $(MOGRIFY_SKIP))

# defines for filtering master filelist
# for file list pre-processing
FILESDEFS = $(DEFS)
FILESUNDEFS =$(patsubst %,NO_%,$(UNDEFS))
FILESDEFS += $(FILESUNDEFS)
GCCFILESDEFS = $(patsubst %,-D%_SRC,$(DEFS))

# These are the extensions of files that will have unix2dos run on them.
# this only happens for the files in the tools/visionice directory because they
# are expected to be used on Windows.
UNIX2DOS_EXT := .reg .ini .bat .dtl .def .TXT

# These customize the router build
KERNELCFG ?= defconfig-bcm947xx-router
ROUTERCFG ?= defconfig-router

MAKE +=  "LINUX_VERSION=$(LINUX_VERSION)"

export PATH

all: build_start build_open build_end
	@$(MARKSTART)
	# Remove the build source directory
	cd build && rm -rf src
	@$(MARKEND)

checkdlna:
	@$(MARKSTART)
ifeq ($(DLNA),1)
	@perl -pi -e "s/# CONFIG_DLNA_DMS is not set/CONFIG_DLNA_DMS=y/; s/# CONFIG_FFMPEG is not set/CONFIG_FFMPEG=y/; s/# CONFIG_LIBZ is not set/CONFIG_LIBZ=y/" components/router/config/$(ROUTERCFG)
ifneq ($(shell grep "CONFIG_SOUND is not set" components/router/config/$(ROUTERCFG)),)
	@perl -pi -e "s/# CONFIG_SOUND is not set/CONFIG_SOUND=y/" components/router/config/$(ROUTERCFG)
	@echo "CONFIG_SALSA=y" >> components/router/config/$(ROUTERCFG)
	@echo "CONFIG_APLAY=y" >> components/router/config/$(ROUTERCFG)
	@echo "CONFIG_LIBMAD=y" >> components/router/config/$(ROUTERCFG)
	@echo "CONFIG_LIBID3TAG=y" >> components/router/config/$(ROUTERCFG)
	@echo "CONFIG_MADPLAY=y" >> components/router/config/$(ROUTERCFG)
endif
endif
	@$(MARKEND)

checkout: $(CHECKOUT_TGT) checkdlna
	@$(MARKSTART)
#     # When a directory has checked-in autotools code, in a fresh checkout
#     # it's essentially random whether configure gets rebuilt locally since
#     # here is to "touch" all configure scripts to make sure they're newer
#     # than their prereqs. This isn't the right fix because the problem
#     # exists at the developer level but until it's fixed this should work
#     # for automated builds.
	find . $(FIND_SKIPCMD) -name configure -type f -print0 | xargs -0 touch
	# calling "make" on the router directory creates a default .config file if there is none.
	# make sure we have the right one first.
	cp $(LINUX_DIR)/arch/$(ARCH)/$(KERNELCFG) $(LINUX_DIR)/.config
	cp components/router/config/$(ROUTERCFG) components/router/.config
	@$(MARKEND)

build_start:
	@$(MARKSTART_BRAND)

filelists:
	@$(MARKSTART)
	# Temporary filelist generation for SVN build test
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist.h
	gcc -E -undef $(GCCFILESDEFS) -Ulinux -o $(SRCFILELIST) master_filelist.h
	@$(MARKEND)

mogrify: checkout
	@$(MARKSTART)	
	for dir in $(MOGRIFY_DIRS) ; do \
		$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS=$${dir} ; \
	done
	@$(MARKEND)

# Install mogrified sources from src/ to the release/ directory
release_src: checkout filelists mogrify
	@$(MARKSTART)
	install -d $(RELEASEDIR)
	# Install CFE
	find components/cfe $(FIND_SKIPCMD) -print | \
		$(SRCFILTER) src/tools/release/cfe-filelist.txt | \
		tee ,copied_files.log | \
		$(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD) $(RELEASEDIR)
ifeq ($(findstring linux-2.6,$(ROUTER_OS)),)
	# Install CFE
	find components/cfe $(FIND_SKIPCMD) -print | \
		$(SRCFILTER) src/tools/release/gpl-filelist.txt | \
		tee ,copied_files.log | \
		$(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD) $(RELEASEDIR)
endif # ROUTER_OS
	# Install Linux
	find $(SRCDIRS) $(FIND_SKIPCMD) -print | \
		$(SRCFILTER) -v $(SRCFILELIST) | \
		tee -a ,copied_files.log | \
		$(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD) $(RELEASEDIR)
ifneq ($(findstring linux-2.6,$(ROUTER_OS)),)
	cp $(LINUX_DIR)/arch/$(ARCH)/$(KERNELCFG) $(RELEASEDIR)/$(LINUX_DIR)/arch/$(ARCH)/defconfig-2.6-bcm947xx
else
	cp $(LINUX_DIR)/arch/$(ARCH)/$(KERNELCFG) $(RELEASEDIR)/$(LINUX_DIR)/arch/$(ARCH)/defconfig-bcm947xx
endif
ifneq ($(findstring linux-2.6,$(ROUTER_OS)),)
	perl -pe "s/# CONFIG_LIBCRYPT is not set/CONFIG_LIBCRYPT=y/; s/CONFIG_LIBOPT=y/# CONFIG_LIBOPT is not set/" components/router/config/$(ROUTERCFG) > $(RELEASEDIR)/components/router/config/defconfig-2.6
else
	cp components/router/config/$(ROUTERCFG) $(RELEASEDIR)/components/router/config/defconfig
endif
        # Install visionICE tools
	install -d $(RELEASEDIR)/tools
        # Install documentation
	install -d $(RELEASEDIR)/doc
	cp src/doc/bcm47xx/LinuxReadme.txt $(RELEASEDIR)/README.TXT
	cp src/doc/BCMLogo.gif $(RELEASEDIR)/
	install src/tools/release/release_linux.sh $(RELEASEDIR)/release.sh
	install $(GPL_FILELIST) $(RELEASEDIR)/gpl-filelist.txt
	install src/tools/release/validate_gpl.sh $(BUILD_BASE)/validate_gpl.sh
	dos2unix $(RELEASEDIR)/gpl-filelist.txt
        # Install toplevel Makefile
	cp components/router/misc/toplevel-release.mk $(RELEASEDIR)/src/Makefile
	@$(MARKEND)

# Prebuild linux kernel/router using unreleased sources in src/ directory
prebuild: release_src
	@$(MARKSTART)
        # Configure Linux for prebuild
	cp $(LINUX_DIR)/arch/$(ARCH)/$(KERNELCFG) $(LINUX_DIR)/.config
ifeq ($(CONFIG_GLIBC),true)
        #---------Prebuild against GNU libc
	perl -pe "s/# CONFIG_GLIBC is not set/CONFIG_GLIBC=y/; s/CONFIG_UCLIBC=y/# CONFIG_UCLIBC is not set/" components/router/config/$(ROUTERCFG) > components/router/.config
	$(MAKE) -C components/router oldconfig PLT=$(PLT)
	$(MAKE) -C components/router all
        # Install and clean router (apps)
	$(MAKE) -C components/router install
	$(MAKE) -C components/router router-clean
endif # CONFIG_GLIBC
        #---------Prebuild against uClibc
ifneq ($(findstring linux-2.6,$(ROUTER_OS)),)
	perl -pe "s/# CONFIG_LIBCRYPT is not set/CONFIG_LIBCRYPT=y/" components/router/config/$(ROUTERCFG) > components/router/.config.tmp1
	perl -pe "s/CONFIG_LIBOPT=y/# CONFIG_LIBOPT is not set/" components/router/.config.tmp1 > components/router/.config.tmp2
	perl -pe "s/# CONFIG_UCLIBC is not set/CONFIG_UCLIBC=y/; s/CONFIG_GLIBC=y/# CONFIG_GLIBC is not set/" components/router/.config.tmp2 > components/router/.config
	rm components/router/.config.tmp1 components/router/.config.tmp2
else
	perl -pe "s/# CONFIG_UCLIBC is not set/CONFIG_UCLIBC=y/; s/CONFIG_GLIBC=y/# CONFIG_GLIBC is not set/" components/router/config/$(ROUTERCFG) > components/router/.config
endif
	$(MAKE) -C components/router oldconfig PLT=$(PLT)
	$(MAKE) -C components/router all
        # Install and clean router (apps)
	$(MAKE) -C components/router install
	[ ! -f $(RELEASEDIR)/components/clm-api/include/wlc_clm_data.h ] || cp -v components/clm-api/src/wlc_clm_data.c $(RELEASEDIR)/components/clm-api/src/
	$(MAKE) -C components/router router-clean
        # --------Prebuild crypto shared lib
	$(MAKE) -C components/router libbcmcrypto
        # --------Prebuild ses objects
	$(MAKE) -C components/router ses/ses
        # --------Prebuild ses client objects
	$(MAKE) -C components/router ses/ses_cl
        # --------Prebuild ezc.o
	$(MAKE) -C components/router httpd
        #---------Prebuild CFE
	$(MAKE) -C components/cfe ARCH=$(ARCH)
        #---------Prebuild tools
	$(MAKE) -C src/tools/misc addhdr swap trx nvserial lzma lzma_4k
        #---------Prebuild EMF, IGS & IGMP Proxy
	$(MAKE) -C components/router emf
	$(MAKE) -C components/router igs
	$(MAKE) -C components/router igmp
	#---------Prebuild bcmupnp
	$(MAKE) -C components/router bcmupnp
	#---------Prebuild libupnp
	$(MAKE) -C components/router libupnp
	#---------Prebuild igd
	$(MAKE) -C components/router igd
	#---------Prebuild bmac dongle images
	$(MAKE) -C components/router bmac
	#---------Prebuild nfc
	$(MAKE) -C components/router nfc
	#---------Prebuild wps
	$(MAKE) -C components/router wps
	#prebuild voipd
	$(MAKE) -C components/router voipd
	#---------Prebuild hspot_ap
	$(MAKE) -C components/router hspot_ap
	@$(MARKEND)

wldir=$(LINUX_DIR)/drivers/net/wl
# Install prebuilt binaries from src/ to the release/ directory
release_prebuild: prebuild
	@$(MARKSTART)
	# Install last default router config file
	install -d $(RELEASEDIR)/components/router
	install components/router/.config $(RELEASEDIR)/components/router/
	# Install last default kernel config file
	install -d $(RELEASEDIR)/$(LINUX_DIR)
	install -p $(LINUX_DIR)/.config $(RELEASEDIR)/$(LINUX_DIR)/.config
	# Install prebuilt binaries and modules
	install -d $(RELEASEDIR)/components/router/httpd/prebuilt
	-cp components/router/httpd/ezc.o $(RELEASEDIR)/components/router/httpd/prebuilt/
	install -d $(RELEASEDIR)/components/router/ses/prebuilt
	-cp components/router/ses/ses/ses_packet.o $(RELEASEDIR)/components/router/ses/prebuilt/
	-cp components/router/ses/ses/ses_linux.o $(RELEASEDIR)/components/router/ses/prebuilt/
	-cp components/router/ses/ses/ses_cfmain.o $(RELEASEDIR)/components/router/ses/prebuilt/
	-cp components/router/ses/ses/ses_fsm.o $(RELEASEDIR)/components/router/ses/prebuilt/
	-cp components/router/ses/ses/ses_cfwl.o $(RELEASEDIR)/components/router/ses/prebuilt/
	-cp components/router/ses/ses/ses_linux_eapd.o $(RELEASEDIR)/components/router/ses/prebuilt/
	install -d $(RELEASEDIR)/components/router/ses/prebuilt_cl
	-cp components/router/ses/ses_cl/ses_packet.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	-cp components/router/ses/ses_cl/ses_linux.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	-cp components/router/ses/ses_cl/ses_clmain.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	-cp components/router/ses/ses_cl/ses_cl_fsm.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	-cp components/router/ses/ses_cl/ses_wl.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	-cp components/router/ses/ses_cl/fsm_assoc.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	-cp components/router/ses/ses_cl/fsm_ds.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	-cp components/router/ses/ses_cl/fsm_main.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	-cp components/router/ses/ses_cl/fsm_sur.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	-cp components/router/ses/ses_cl/ses_linux_eapd.o $(RELEASEDIR)/components/router/ses/prebuilt_cl/
	install -d $(RELEASEDIR)/components/router/libbcmcrypto/prebuilt
	-cp components/router/libbcmcrypto/*.o $(RELEASEDIR)/components/router/libbcmcrypto/prebuilt
	install -d $(RELEASEDIR)/src/shared/linux
ifneq ($(findstring linux-2.6,$(ROUTER_OS)),)
	cp $(wildcard src/shared/*.o) $(RELEASEDIR)/src/shared/linux/
endif
	cp $(wildcard $(LINUX_DIR)/drivers/net/hnd/*.o) $(RELEASEDIR)/src/shared/linux/
	install -d $(RELEASEDIR)/components/router/emf/emfconf/prebuilt/
	-cp components/router/emf/emfconf/emf $(RELEASEDIR)/components/router/emf/emfconf/prebuilt/
	-cp components/router/emf/emfconf/Makefile $(RELEASEDIR)/components/router/emf/emfconf/
	install -d $(RELEASEDIR)/components/router/emf/igsconf/prebuilt/
	-cp components/router/emf/igsconf/igs $(RELEASEDIR)/components/router/emf/igsconf/prebuilt/
	-cp components/router/emf/igsconf/Makefile $(RELEASEDIR)/components/router/emf/igsconf/
	install -d $(RELEASEDIR)/components/router/igmp/prebuilt/
	-cp components/router/igmp/igmp $(RELEASEDIR)/components/router/igmp/prebuilt/
	-cp components/router/igmp/Makefile $(RELEASEDIR)/components/router/igmp/
	install -d $(RELEASEDIR)/components/router/bcmupnp/prebuilt/
	-cp components/router/bcmupnp/upnp/linux/upnp $(RELEASEDIR)/components/router/bcmupnp/prebuilt/
	install -d $(RELEASEDIR)/components/router/igd/prebuilt/
	-cp components/router/igd/linux/igd $(RELEASEDIR)/components/router/igd/prebuilt/
	install -d $(RELEASEDIR)/components/router/libupnp/prebuilt/
	-cp components/router/libupnp/*.o $(RELEASEDIR)/components/router/libupnp/prebuilt/
	install -d $(RELEASEDIR)/src/shared
	-cp src/shared/rtecdc_*.h $(RELEASEDIR)/src/shared/
	install -d $(RELEASEDIR)/components/router/wps/prebuilt/
	-cp components/router/wps/lib/*.a $(RELEASEDIR)/components/router/wps/prebuilt/
	[ ! -d components/router/nfc ] || install -d $(RELEASEDIR)/components/router/nfc/
	-cp components/router/nfc/*.a $(RELEASEDIR)/components/router/nfc
	[ ! -d components/apps/nfc ] || install -d $(RELEASEDIR)/components/apps/nfc/3rdparty/embedded/nsa_examples/linux/libnsa/include
	-cp components/apps/nfc/3rdparty/embedded/nsa_examples/linux/libnsa/include/*.h $(RELEASEDIR)/components/apps/nfc/3rdparty/embedded/nsa_examples/linux/libnsa/include/
	install -d $(RELEASEDIR)/components/apps/hspot/router/hspot_ap/prebuilt/
	-cp components/apps/hspot/router/hspot_ap/hspotap $(RELEASEDIR)/components/apps/hspot/router/hspot_ap/prebuilt/
	-cp components/apps/hspot/router/hspot_ap/Makefile $(RELEASEDIR)/components/apps/hspot/router/hspot_ap/
	install -d $(RELEASEDIR)/src/voip/xChange/prebuilt
	install -d $(RELEASEDIR)/components/router/voipd/prebuilt
	-cp $(LINUX_DIR)/drivers/char/endpoint/endpointdd.o $(RELEASEDIR)/src/voip/xChange/prebuilt/endpointdd_linux.o
	-cp $$(find src/voip -name pcm_bcm47xx.o) $(RELEASEDIR)/src/voip/xChange/prebuilt
	-cp $$(find src/voip -name spi_bcm47xx.o) $(RELEASEDIR)/src/voip/xChange/prebuilt
	-cp components/router/voipd/voipd $(RELEASEDIR)/components/router/voipd/prebuilt
	-cp components/router/voipd/voice $(RELEASEDIR)/components/router/voipd/prebuilt
	-cp components/router/voipd/*.so $(RELEASEDIR)/components/router/voipd/prebuilt
	install -d $(RELEASEDIR)/components/et/cfe
	-cp components/cfe/build/broadcom/bcm947xx/et*.o $(RELEASEDIR)/components/et/cfe/
	install -d $(RELEASEDIR)/components/router/ctf/linux
	install -d $(RELEASEDIR)/components/router/dpsta/linux
	-cp $(LINUX_DIR)/drivers/net/dpsta/dpsta.o $(RELEASEDIR)/components/router/dpsta/linux/
	install -d $(RELEASEDIR)/components/et/linux
	install -d $(RELEASEDIR)/src/bcm57xx/linux
	install -d $(RELEASEDIR)/src/il/linux
	install -d $(RELEASEDIR)/src/usbdev/linux
	install -d $(RELEASEDIR)/src/wl/linux
	install -d $(RELEASEDIR)/components/router/emf
ifneq ($(findstring linux-2.6,$(ROUTER_OS)),)
	-cp $(LINUX_DIR)/drivers/net/et/et.ko $(RELEASEDIR)/components/et/linux/
	-cp $(LINUX_DIR)/drivers/net/bcm57xx/bcm57xx.ko $(RELEASEDIR)/src/bcm57xx/linux/
	-cp $(wildcard $(LINUX_DIR)/drivers/net/il/*.ko) $(RELEASEDIR)/src/il/linux/
	-cp $(wildcard $(LINUX_DIR)/drivers/usb/gadget/*.ko) $(RELEASEDIR)/src/usbdev/linux/
	-(cd $(wldir) && for mod in wl_*; do [ -f $${mod}/$${mod}.ko ] && cp $${mod}/$${mod}.ko $(RELEASEDIR)/src/wl/linux; done)
	-cp $(LINUX_DIR)/drivers/net/emf/emf.ko $(RELEASEDIR)/components/router/emf/
	-cp $(LINUX_DIR)/drivers/net/igs/igs.ko $(RELEASEDIR)/components/router/emf/
	-cp $(LINUX_DIR)/drivers/net/ctf/ctf.ko $(RELEASEDIR)/components/router/ctf/linux/
endif
	-cp $(LINUX_DIR)/drivers/net/et/et.o $(RELEASEDIR)/components/et/linux/
	-cp $(LINUX_DIR)/drivers/net/bcm57xx/bcm57xx.o $(RELEASEDIR)/src/bcm57xx/linux/
	-cp $(wildcard $(LINUX_DIR)/drivers/net/il/*.o) $(RELEASEDIR)/src/il/linux/
	-cp $(wildcard $(LINUX_DIR)/drivers/usb/gadget/*.o) $(RELEASEDIR)/src/usbdev/linux/
	-(cd $(wldir) && for mod in wl_*; do [ -f $${mod}/$${mod}.o ] && cp $${mod}/$${mod}.o $(RELEASEDIR)/src/wl/linux; done)
	-cp $(LINUX_DIR)/drivers/net/emf/emf.o $(RELEASEDIR)/components/router/emf/
	-cp $(LINUX_DIR)/drivers/net/igs/igs.o $(RELEASEDIR)/components/router/emf/
	-cp $(LINUX_DIR)/drivers/net/ctf/ctf.o $(RELEASEDIR)/components/router/ctf/linux/
        # Install prebuilt apps
	find components/router/$(ARCH)* $(FIND_SKIPCMD) -print | pax -rw $(PAX_SKIPCMD) $(RELEASEDIR)
ifneq ($(findstring linux-2.6,$(ROUTER_OS)),)
        # Install compressed dir
	install -d $(RELEASEDIR)/components/router/compressed
	-cp components/router/compressed/Makefile $(RELEASEDIR)/components/router/compressed/
        # Install cramfs dir
	install -d $(RELEASEDIR)/components/router/cramfs
	-cp components/router/cramfs/GNUmakefile $(RELEASEDIR)/components/router/cramfs/
	-cp components/router/cramfs/cramfsck.c $(RELEASEDIR)/components/router/cramfs/
	-cp components/router/cramfs/mkcramfs.c $(RELEASEDIR)/components/router/cramfs/
endif
        # Install squashfs dir
ifeq ("$(ROUTER_OS)", "linux-2.6.36")
	install -d $(RELEASEDIR)/components/router/squashfs-4.2
	-cp components/router/squashfs-4.2/Makefile $(RELEASEDIR)/components/router/squashfs-4.2
	-cp components/router/squashfs-4.2/*.c $(RELEASEDIR)/components/router/squashfs-4.2
	-cp components/router/squashfs-4.2/*.h $(RELEASEDIR)/components/router/squashfs-4.2
else
	install -d $(RELEASEDIR)/components/router/squashfs
	-cp components/router/squashfs/Makefile $(RELEASEDIR)/components/router/squashfs
	-cp components/router/squashfs/*.c $(RELEASEDIR)/components/router/squashfs
	-cp components/router/squashfs/*.h $(RELEASEDIR)/components/router/squashfs
endif

        # Install tools
	install -d $(RELEASEDIR)/tools
	install src/tools/misc/addhdr $(RELEASEDIR)/tools/
	install src/tools/misc/trx $(RELEASEDIR)/tools/
	install src/tools/misc/swap $(RELEASEDIR)/tools/
	install src/tools/misc/nvserial $(RELEASEDIR)/tools/
	install src/tools/misc/lzma $(RELEASEDIR)/tools/
	install src/tools/misc/lzma_4k $(RELEASEDIR)/tools/
        # Re-mogrify all text files after prebuilding
ifneq ($(findstring linux-2.6,$(ROUTER_OS)),)
	find release $(RELEASE_MOGRIFY_EXCLUDE) -type f $(FIND_SKIPCMD) -print | perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' | grep -v "linux/$(ROUTER_OS)/Makefile" | xargs $(MOGRIFY) -UPREBUILD
else
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS=release MOGRIFY_FLAGS="-UPREBUILD"
endif
        # Perform miscellaneous sanitization
	cd release && ../src/tools/release/linux-postbuild.sh
	@$(MARKEND)

# Build in duplicate release directory
build: release_prebuild
	@$(MARKSTART)
        # Duplicate release directory
	install -d build
	cd release && find . $(FIND_SKIPCMD) -print | pax -rw $(PAX_SKIPCMD) $(BUILD_BASE)/build/	
	#Make sure old .config's are removed, If they get copied
	#from release dir
	rm -rf $(BUILD_BASE)/build/components/router/.config
	rm -rf $(BUILD_BASE)/build/$(LINUX_DIR)/.config
ifeq ($(ROUTERSRC),true)
	$(MAKE) -C build/src PLT=$(PLT)
else
	$(MAKE) -C build/src REUSE_PREBUILT_WL=1 PLT=$(PLT)
endif
        # Install prebuilt images in the release directory
	install -d $(RELEASEDIR)
	cd build && find image $(FIND_SKIPCMD) -print | pax -rw $(PAX_SKIPCMD) $(RELEASEDIR)
	@$(MARKEND)

# Archive the release
release_archive: ROUTER_PACKAGE_VERSION = $(shell perl -lne '/ROUTER_PACKAGE_VERSION\s+(.*)/ && print $$1' < $(RELEASEDIR)/components/router/shared/router_version.h)
release_archive: build
	@$(MARKSTART)
	cd $(RELEASEDIR) && \
	tar czf ../$(ROUTER_OS)-router-$(ROUTER_PACKAGE_VERSION).tar.gz $(TAR_SKIPCMD) *

ifneq ($(WLTEST),1)
        # Prepare an open release version
	find $(patsubst %, $(RELEASEDIR)/%, $(MOGRIFY_OPENSOURCE_DIRS)) $(patsubst %,-path '$(RELEASEDIR)/%' -prune -o, $(MOGRIFY_SKIP)) -type f $(FIND_SKIPCMD) -print | perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' | grep -v "linux/$(ROUTER_OS)/Makefile" | xargs $(MOGRIFY_UTIL) $(MOGRIFY_OPENSOURCE_FLAGS)
	cd $(RELEASEDIR) && ./release.sh ../linux-open-router-$(ROUTER_PACKAGE_VERSION).tar.gz
endif
	mv $(RELEASEDIR) $(RELEASEDIR).1
	install -d -m 775 $(RELEASEDIR)
	mv *.tar.gz $(RELEASEDIR)/
        # release mipsel tools
#       cd $(RELEASEDIR) && tar czf hndtools-3.0.tar.gz $(TAR_SKIPCMD) -C /opt brcm/hndtools-mipsel-linux-3.0 brcm/hndtools-mipsel-linux
	install src/tools/release/install_linux.sh $(RELEASEDIR)/install.sh
        # Keep the src directory for debugging purposes in future
ifeq ($(WLTEST),1)
        # for mfgtest ramdisk build, copy vmlinuz
ifneq ($(findstring linux-2.6,$(ROUTER_OS)),)
	install components/router/compressed/vmlinuz $(RELEASEDIR)
else
	install $(LINUX_DIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz $(RELEASEDIR)
endif
endif
	@$(MARKEND)

# Test build of open release for all external router/ap builds
build_open: release_archive
ifeq ($(findstring external,$(BRAND)),external)
	@$(MARKSTART)
        # Duplicate release directory
	[ ! -f $(RELEASEDIR)/linux-open-router-*.tar.gz ] || install -d tmp
	[ ! -d tmp ] || tar xzf $(RELEASEDIR)/linux-open-router-*.tar.gz  $(TAR_SKIPCMD) -C tmp
	# check for the GPL License here
	[ ! -d tmp ] || ./validate_gpl.sh tmp
	[ ! -d tmp ] || $(MAKE) -C tmp/src PLT=$(PLT)
	@$(MARKEND)
endif # BRAND=external

build_clean: build_open
	@$(MARKSTART)
ifndef LINUX_ROUTER_NO_CLEAN
	-find src components -type f -name "*.o" -a ! -name "*_dbgsym.o" | xargs rm -f
        # Remove release-staging directory
	rm -rf $(RELEASEDIR).1
        # Remove test directory
	rm -rf tmp
endif # LINUX_ROUTER_NO_CLEAN
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)

.PHONY: all mogrify prebuild build release_src release_prebuild release_archive
