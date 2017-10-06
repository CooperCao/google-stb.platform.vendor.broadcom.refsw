
# Defining 3 sets of files, each with their own tag :

# SHARED_FILES is the base branch we are diverging from.
# SHARED_FILES modules are not supposed to be
# customized for that particular project. If changes happen here, they should
# be intended to be part of the SHARED_TAG.

# MODULE_FILES is this specific branch. In current method, we would have put everything
# under that tag. All branch customization should happen only on those files

# TOT_FILES (no tag). Useful for third party software. It allows "import" when
# a new update comes from the third pary.

# As many new sets as necessary can be created depending on the need of a specific project.
# For example, a specific version (i.e. with a RELEASE TAG) of nas could be
# desired instead of a TOB. There is an example in WPSDEV_DISPY2_BRANCH
# where ses, nas and eapd are from their own branch, while dipsy2 is used for the driver
# and TOT is used for router+kernel

# This method can also be used to save space (and compile time) by allowing the use
# of symbolic links to  shared code. A sym link to a unique version of the kernel seem
# a good application of that !


### IMPORTANT ###

# There is still an issue with adding a module outside its tagged scope (i.e. adding a BRANCH_FILE directory
# inside a "SHARED_FILE" directory. There is a way to do it, CVS is doesn't allow this naturally.
# I'll add a tarxget to do that very soon. Meanwhile, please ask me (pmoutarlier@broadcom.com).

include WLAN.usf

#PROD_TAG = STAGURR_REL_4_150_10_15

# Note, PROD_TAG here can only be used if this file has been
# extracted using the -kv flag. Otherwise, PROD_TAG can be specified on the command line.

PROD_TAG = $Name: not supported by cvs2svn $
MOD_TAG =

ifneq ("$(PROD_TAG)", "")
SHARED_TAG = -r $(PROD_TAG)
endif
ifneq ("$(MOD_TAG)", "")
MODULE_TAG = -r $(MOD_TAG)
endif

# for release safety, define here the BRANCH names. "make release_tag" will ensure
# that none of the tags (SHARED_TAG, BRANCH_TAG ...) are left at TOT or TOB before
# proceeding.

SHARED_BRANCH_NAME =
MODULE_BRANCH_NAME =

ifneq ("$(SHARED_BRANCH_NAME)", "")
SHARED_BRANCH = -r $(SHARED_BRANCH_NAME)
endif
ifneq ("$(MODULE_BRANCH_NAME)", "")
MODULE_BRANCH = -r $(MODULE_BRANCH_NAME)
endif

# we define the branch for the product BOM, i.e. this very Makefile and the release directory

PRODUCT_BRANCH_NAME =


TAG_MSG = "commiting makefile for tagging release $(MOD_TAG)"


MAKE_TYPE := SOURCE


PRODUCT_FILES = src/tools/release

# high level modules. Some modules are expanded further to allow
# finer granularity.


# high level modules. Some modules are expanded further to allow
# finer granularity.
TOPSRC_FILES := \
	src/bcm57xx \
	components/cfe \
	src/doc \
	src/makefiles \
	src/rts \
	src/ses \
	src/usbdev \
	src/branding.inc \
	src/Makerules \
	src/Makerules.env \
	src/GNUmakefile.inc

# In order to allow full flexibility, we list all modules under components/router.
# If one of these need to be customized for a specific branch, rtag it and move
# it under BRANCH_FILES

ROUTER_SHARED := 	src/bcmcrypto \
	src/linux/linux \
        components/router/ash components/router/b57ldiag components/router/bridge \
	components/router/busybox components/router/dialog components/router/disktype components/router/e2fsprogs \
	components/router/emf components/router/etc components/router/ethtool components/router/fdisk components/router/gdbserver \
	components/router/goahead components/router/hotplug components/router/httpd components/router/igmp \
	components/router/iptables components/router/iptraf components/router/iputils components/router/libbcm \
	components/router/libbcmcrypto components/router/libpcap components/router/lltd components/router/mini_httpd \
	components/router/misc components/router/ncurses components/router/netconf components/router/ntpclient \
	components/router/nvram components/router/pcmcia-cs components/router/ppp components/router/rte \
	components/router/tcpdump components/router/terminfo components/router/trueffs components/router/udhcpd \
	components/router/utelnetd components/router/utils components/router/aput components/router/vlan components/router/wcn \
	components/router/wireless_tools components/router/wlconf components/router/wsc \
	components/router/lib \
	components/router/config \
	components/router/dnsmasq src/wps components/router/bcmupnp \
	components/router/ses components/router/nas components/router/rc \
	components/router/www components/router/Makefile components/router/shared components/router/upnp

TOOLS_SHARED := src/tools/47xxtcl src/tools/build src/tools/misc src/tools/profile \
	src/tools/upnp src/tools/visionice

WL_FILES := src/wl/config src/wl/sys src/wl/exe src/include src/shared components/et
WL_FILES += $(patsubst %,%/include %/src,$(WLAN_COMPONENT_PATHS))

SHARED_FILES = $(ROUTER_SHARED)

MODULE_FILES := $(WL_FILES) $(TOPSRC_FILES) $(TOOLS_SHARED)

all: extract_src

extract_src: extract_shared extract_branch

# extract source from tags as defined by SHARED_TAG and MODULE_TAG
extract_tag:
	make MAKE_TYPE=TAG_SRC -f src/tools/release/linux-router-extract extract_src	
export:
	make MAKE_TYPE=EXPORT -f src/tools/release/linux-router-extract extract_src	

release_tag :
	make MAKE_TYPE=REL_TAG -f src/tools/release/linux-router-extract extract_src

build: phony
	cd src/wps/linux; make CC=$(CC) -f wps_enr_app.mk

#base branch we are diverging from
extract_shared: $(SHARED_FILES)

#override with modules specific to this branch
extract_branch: $(MODULE_FILES)

#extract files directly from TOT (nice for third party modules)
extract_tot: $(TOT_FILES)

phony:


# top of branch code. Note that the "branch tags" could in
# fact be static tags as well.

ifeq ("$(MAKE_TYPE)", "SOURCE")
$(SHARED_FILES) : phony
	cvs co  $(SHARED_BRANCH) $@

$(MODULE_FILES) : phony
	cvs co  $(MODULE_BRANCH) $@

$(TOT_FILES) : phony
	cvs co $@

endif

# tagged source.

ifeq ("$(MAKE_TYPE)", "TAG_SRC")
$(SHARED_FILES) : phony
	cvs co  $(SHARED_TAG) $@

$(MODULE_FILES) : phony
	cvs co  $(MODULE_TAG) $@

$(TOT_FILES) : phony
	cvs co $@

endif


# EXPORT applies to tagged source
ifeq ("$(MAKE_TYPE)", "EXPORT")

$(SHARED_FILES) :
#phony
#-cvs -Q export $(SHARED_TAG) -kk $@

$(MODULE_FILES) : phony
	cvs -Q export $(MODULE_TAG) -kk $@
endif


ifeq ($(MAKE_TYPE),REL_TAG)
ifeq ("$(MODULE_BRANCH_NAME)", "$(MOD_TAG)")
	@echo "MOD_TAG refers to MODULE_BRANCH_NAME !!"
else
$(SHARED_FILES) :

$(MODULE_FILES) : phony
	cd src; cvs commit -m $(TAG_MSG) tools/release/linux-wps-enrollee.mk
	@echo "Tagging $@ on   $(MODULE_BRANCH_NAME) with rtag $(MOD_TAG)"
	cvs rtag $(MODULE_BRANCH_NAME) $(MOD_TAG) $@
endif
endif
