#
# $Id$
#

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
# desired instead of a TOB.

# This method can also be used to save space (and compile time) by allowing the use
# of symbolic links to  shared code. A sym link to a unique version of the kernel seem
# a good application of that !

######################################################################################
# Usage :

# to check files out, from the location including  the top level "src"  :

# make -f this-makefile.mk extract_src

# to check files out, from the location including  the top level "src", then build
# the linux application for i386

# make -f this-makefile.mk

# to build only

# make -f this-makefile.mk build

# to build only with a cross-compiler

# make CC="arm-linux-gcc" -f this-makefile.mk

# to tag a release :

# edit this makefile
# make sure SHARED_TAG_NAME and MOD_TAG are DIFFERENT than SHARED_BRANCH_NAME
# and MODULE_BRANCH_NAME, respectively. They should both refer to a static tag.
# The SHARED_TAG_NAME is assumed to be from an external tag and will not be applied
# The MOD_TAG will be applied to this module filelist.
# then call :
# make -f this-makefile.mk release_tag

# this call will first commit this makefile, then apply the MOD_TAG.

# to get the source corresponding to the last tag

# make  -f this-makefile.mk extract_tag_src

# to make the package (after checkout and mogrification under src directory)

# make -f this-makefile.mk package

###############################################################################################

### IMPORTANT ###

# There is still an issue with adding a module outside its tagged scope (i.e. adding a BRANCH_FILE directory
# inside a "SHARED_FILE" directory. There is a way to do it, CVS is doesn't allow this naturally.
# I'll add a target to do that very soon. Meanwhile, please ask me (pmoutarlier@broadcom.com).


# tags applied when calling extract_tag_src

#PROD_TAG = STAGURR_REL_4_150_10_15

# Note, PROD_TAG here can only be used if this file has been
# extracted using the -kv flag. Otherwise, prevent the PROD_TAG
# to be ovveriden as we would not be sure that this file
# was in sync with the tag

override PROD_TAG = $Name: not supported by cvs2svn $

SHARED_TAG_NAME =
MOD_TAG = WPS_ENROLLEE_LIB_1_0_57

ifneq ("$(SHARED_TAG_NAME)", "")
SHARED_TAG = -r $(SHARED_TAG_NAME)
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

TAG_MSG = "commiting makefile for tagging release $(MOD_TAG)"


MAKE_TYPE := SOURCE


# high level modules. Some modules are expanded further to allow
# finer granularity.

SHARED_FILES := \
	src/bcmcrypto/aes.c \
	src/bcmcrypto/rijndael-alg-fst.c \
	src/bcmcrypto/dh.c \
	src/bcmcrypto/sha256.c \
	src/bcmcrypto/hmac_sha256.c \
	src/bcmcrypto/random.c \
	src/bcmcrypto/bn.c \
	src/bcmcrypto/bn_lcl.h \
	src/include/bcmcrypto/md32_common.h \
	src/include/bcmcrypto/aes.h \
	src/include/bcmcrypto/aeskeywrap.h \
	src/include/bcmcrypto/bn.h \
	src/include/bcmcrypto/dh.h \
	src/include/bcmcrypto/hmac_sha256.h \
	src/include/bcmcrypto/rijndael-alg-fst.h \
	src/include/bcmcrypto/sha1.h \
	src/include/bcmcrypto/sha256.h \
	src/include/bcmdefs.h \
	src/include/bcmendian.h \
	src/include/bcmutils.h \
	src/include/bcmiov.h \
	src/include/bcmtlv.h \
	src/shared/bcmwifi/include/bcmwifi_channels.h \
	src/shared/bcmwifi/include/bcmwifi_rclass.h \
	src/include/bcmcdc.h \
	src/include/wlioctl.h \
	../components/shared/devctrl_if/wlioctl_defs.h \
	src/include/typedefs.h \
	src/include/packed_section_start.h \
	src/include/packed_section_end.h \
	../components/shared/proto/eap.h \
	../components/shared/proto/eapol.h \
	../components/shared/proto/ethernet.h \
	../components/shared/proto/bcmevent.h \
	../components/shared/proto/wpa.h \
	../components/shared/proto/bcmeth.h \
	../components/shared/proto/802.11.h \
	../components/shared/proto/802.11r.h


MODULE_FILES := src/wps/common/enrollee  src/wps/common/include  src/wps/common/makefile  \
	src/wps/common/registrar  src/wps/common/shared  src/wps/common/sources  \
	src/wps/common/sta  src/wps/common/ap  \
	src/wps/common/wps_ap_lib.mk  src/wps/common/wps_common_lib.mk \
	src/wps/common/wps_enr_lib.mk src/wps/linux/enr src/wps/linux/inc


#override this on the command line like this make CC="arm-linux-gcc"
#for cross-compiling

CC = gcc

PRODUCT_FILES :=

# check if src/tools/release/linux-wps-enrollee.mk is there
RELEASE_MAKE := $(wildcard src/tools/release/linux-wps-enrollee.mk)


# if linux-wps-enrollee.mk  not present, check it out
ifeq ($(RELEASE_MAKE),)
PRODUCT_FILES := src/tools/release/linux-wps-enrollee.mk
RELEASE_MAKE := src/tools/release/linux-wps-enrollee.mk

#if tag specified (called from build script)
ifneq ($(TAG),)
# is it a release tag ?
ifneq ("$(PROD_TAG)","ame:  ")
PTAG := -r $(word 2, $(PROD_TAG))
all: extract_tag build

$(PRODUCT_FILES):
	cvs co $(PTAG) $@
else
PTAG := -r $(TAG)

all: extract_tob build
$(PRODUCT_FILES):
	cvs co $(PTAG) $@
endif
# tag not specified, use TOT
else
all: extract_tob build

$(PRODUCT_FILES):
	cvs co $@
endif
# called directly from directory. Use targets to refine
else
$(PRODUCT_FILES):

all: extract_tob build
endif

extract_src: extract_shared extract_tot extract_branch
	cvs co src/tools/release/mkversion.sh
	bash src/tools/release/mkversion.sh src/wps/common/include/version.h.in src/wps/common/include/wps_version.h "$(PROD_TAG)"
extract_tob: $(PRODUCT_FILES)
	make MAKE_TYPE=SOURCE -f  $(RELEASE_MAKE) extract_src
# extract source from tags as defined by SHARED_TAG and MODULE_TAG
extract_tag: $(PRODUCT_FILES)
	make MAKE_TYPE=TAG_SRC -f $(RELEASE_MAKE) extract_src	
export: $(PRODUCT_FILES)
	make MAKE_TYPE=EXPORT -f $(RELEASE_MAKE) extract_src	

release_tag :
	cd src; cvs commit -m $(TAG_MSG) tools/release/linux-wps-enrollee.mk
	make MAKE_TYPE=REL_TAG -f $(RELEASE_MAKE) extract_src

package :
	tar czvf wps_enrollee.tgz --exclude=*/.svn $(SHARED_FILES) $(MODULE_FILES)

build: phony
	cd src/wps/linux/enr; make CC=$(CC) -f wps_enr_app.mk



#base branch we are diverging from
extract_shared: $(SHARED_FILES)

#override with modules specific to this branch
extract_branch: $(MODULE_FILES)

#extract files directly from TOT (nice for third party modules)
extract_tot: $(TOT_FILES)

phony:


# Process files differently if checking out sources or tagging

ifeq ("$(MAKE_TYPE)", "BUILD_ONLY")

$(SHARED_FILES) :

$(MODULE_FILES) :

$(TOT_FILES) :

endif


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
	@echo "Tagging $@ on   $(MODULE_BRANCH_NAME) with rtag $(MOD_TAG)"
	cvs rtag $(MODULE_BRANCH_NAME) $(MOD_TAG) $@
endif
endif
