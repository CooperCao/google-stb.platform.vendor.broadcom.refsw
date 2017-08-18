ifneq ($(findstring _BRANCH_,$(TAG)),)
  BRANCH_TAG_FOUND=1
endif
ifneq ($(findstring _TWIG_,$(TAG)),)
  BRANCH_TAG_FOUND=1
endif

ifdef BRANCH_TAG_FOUND
  override PROD_TAG = $(TAG)
else
  override PROD_TAG = $Name: not supported by cvs2svn $
endif

#by default, we are above src. Can be overriden by caller
SRCBASE := src

#default module list file
MOD_FILE = $(SRCBASE)/tools/release/module_list.mk

include $(MOD_FILE)

# Modules that should be taken from the release list even for development
BSD_ROUTER_REL := \
	build_scripts \
	linux_router/misc \
	linux_router/nvram

# all driver related stuff is considered "under development"
# so this list is empty.
DRIVER_REL := \
	wlphy

PROD_REL_LIST := $(BSD_ROUTER_REL) $(DRIVER_REL)

# Driver modules to use for development
# If no tag/branch specified, use the branch corresponding to
# the released version in module_list
# example of  override :
# linux_router/shared...SHARED_REL_1_2_3

DRIVERS_DEV := \
	wl/sys \
	wl/exe \
	wl/bsd \
	src_include \
	bcm57xx \
	bcmcrypto \
	et \
	wl_shared \
	makefiles \
	src_makerules

ROUTER_DEV := \
	release_scripts \
	shared_nvram \
	netbsd5/kernel \
	netbsd5/net80211 \
	linux_router/www \
	linux_router/shared \
	tools \
	tools/unittest_src


PROD_DEV_LIST := $(ROUTER_DEV) $(DRIVERS_DEV)
#list all components
ALL_PROD_LIST := $(PROD_DEV_LIST) $(PROD_REL_LIST)

# Router Version Strings
version: $(SRCBASE)/tools/release/mkversion.sh $(SRCBASE)/router/shared/version.h.in
	bash $(SRCBASE)/tools/release/mkversion.sh $(SRCBASE)/router/shared/version.h.in \
	$(SRCBASE)/router/shared/router_version.h "$(PROD_TAG)"
	make -C  $(SRCBASE)/include
	[ ! -e $(SRCBASE)/wps/common/include/version.h.in ] || \
		bash $(SRCBASE)/tools/release/mkversion.sh \
			$(SRCBASE)/wps/common/include/version.h.in \
			$(SRCBASE)/wps/common/include/wps_version.h "$(PROD_TAG)"
	
PROD_VERSION += version

#get the work done ...
include $(BOM_RULES)
