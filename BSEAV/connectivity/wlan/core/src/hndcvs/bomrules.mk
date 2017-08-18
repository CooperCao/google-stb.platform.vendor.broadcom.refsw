#
# $Id$
#
############################################################################
# Nothing below is to be edited except by hndcvs or BOM developers.
############################################################################

HNDCVS_CMD = src/hndcvs/hndcvs_backend $(NO_CO) 



# If this is set, we are being included by another instance ...
ifneq ("$(RULES_INCLUDED)","TRUE")
# Regular case : this is the only (or first) instance of bomrules.mk 
RULES_INCLUDED := TRUE

ifneq ("$(DEFS)","")
HNDCVS_CMD += -defs "$(DEFS)"
endif

ifneq ("$(CVS_OPT)","")
HNDCVS_CMD += -opt "$(CVS_OPT)"
endif

#MOD_FILE := $(SRCBASE)/tools/release/module_list.mk
#include $(MOD_FILE)

MK_TYPE := SRC

# eliminate duplicates for finding files
FILE_LIST = $(sort $(FILE))

ifneq ("$(DATE)","")
DATE_CMD = -date "$(DATE)"
endif

# by default, HEAD option, when specified, is on
HEAD_OPT=-h
# release don't need an option by default
REL_OPT=

# if the global head is requested, release option turns to head
ifeq ("$(HEAD_REQ)", "Y")
REL_OPT=-h
else
# if rel req and not global head, turn head off
ifeq ("$(REL_REQ)", "Y")
HEAD_OPT=
endif
endif


# covers all variant of find and log
define hndcvs_find
$(1):
ifeq ("$(FILE_LIST)","")
	$(HNDCVS_CMD) $(LOCAL) $(2)  $(FIND_OP)  $(1) $(3)
else
	$(HNDCVS_CMD) $(LOCAL) $(2)  $(FIND_OP)  "$(FILE_LIST)" $(1) $(3)
endif
endef

define hndcvs_get_release
$(1) :
	$(HNDCVS_CMD) $(DATE_CMD) $(1) $(2)
endef

define hndcvs_tag_release
$(1) :
ifeq ("$(FORCE_TAG)","1")
	$(HNDCVS_CMD) -t $(1) $(2)
else
	$(HNDCVS_CMD) -r $(1) $(2)
endif
endef

define hndcvs_get_dev
$(1) :
	$(HNDCVS_CMD) $(DATE_CMD) -h $(1) $(2)
endef

define hndcvs_diff_head
$(1) :
	$(HNDCVS_CMD) -q $(diff_opt) -dh $(1) $(2)	
endef

define hndcvs_diff_rel
$(1) :
	$(HNDCVS_CMD) -q $(diff_opt) -dr $(1) $(2) $(3)	
endef

define hndcvs_diff_dev
$(1) :
	$(HNDCVS_CMD) -q $(diff_opt) -dd $(1) $(2) $(3)	
endef

define hndcvs_diff_rel_dev
$(1) :
	$(HNDCVS_CMD) -q $(diff_opt) -drd $(1) $(2) $(3)	
endef

define hndcvs_diff_dev_rel
$(1) :
	$(HNDCVS_CMD) -q $(diff_opt) -ddr $(1) $(2) $(3)	
endef

define hndcvs_generic
$(1) :
	$(HNDCVS_CMD) $(LOCAL) $(2)  $(1) $(3)	
endef


define hndcvs_get_head
$(1) :
	$(HNDCVS_CMD) $(DATE_CMD) -h $(1) $(2)	
endef

%:

display_diff :
	@echo
	@echo "dev 1 rel 2 :" 
	@echo "             $(dev_rel)"
	@echo "dev 2 rel 1 :" 
	@echo "             $(rel_dev)"
	@echo "dev common  :" 
	@echo "             $(dev_common)"
	@echo "rel common  :"
	@echo "             $(rel_common)"
	@echo "rel only in 1 :"
	@echo "             $(rel1)"
	@echo "rel only in 2 :"
	@echo "             $(rel2)"
	@echo "dev only in 1 :"
	@echo "             $(dev1)"
	@echo "dev only in 2 :"
	@echo "             $(dev2)"


get_tag=$(word 2, $(subst ..., , $(strip $(1))))
get_module=$(word 1, $(subst ..., , $(1)))

# list of dev modules, without tags 
dev_mod_list := $(foreach mod, $(PROD_DEV_LIST), $(call get_module, $(mod)))

# list of all modules without tags
all_mod_list := $(foreach mod, $(ALL_PROD_LIST), $(call get_module, $(mod)))

# list of all modules without tags
rel_mod_list := $(foreach mod, $(PROD_REL_LIST), $(call get_module, $(mod)))

#module override list
ow_mod_list := $(foreach mod, $(OVERRIDE_LIST), $(call get_module, $(mod)))

#new release list without the override list
upd_rel_list := $(filter-out $(ow_mod_list), $(all_mod_list))

#undefined modules being passed

undef_mods:=$(filter-out $(all_mod_list), $(MAKECMDGOALS))

#re-construct the non-override module list with tags associated

UPD_RELEASE_LIST := $(foreach f, $(upd_rel_list), $(filter $(strip $(f))...%, $(RELEASE_LIST)))  

RELEASE_LIST_2 := $(OVERRIDE_LIST) $(UPD_RELEASE_LIST) 

get_rel_tag= $(call get_tag, $(filter $(strip $(1))...%, $(RELEASE_LIST_2)))

REL_MOD_TAG_LIST =$(foreach f, $(rel_mod_list), $(f)...$(call get_rel_tag, $(f)))

DEV_MOD_TAG_LIST =$(foreach f, $(dev_mod_list), $(f)...$(call get_rel_tag, $(f)))

# check that there are no intersections between REL and DEV list

COMMON_MODS=$(filter $(rel_mod_list), $(dev_mod_list))

ifneq ($(COMMON_MODS),)
$(error modules $(COMMON_MODS) in REL and DEV list in this BOM !!)
endif

show_dev_list :
	@echo
	@echo file : $(FILE)
	@echo
	@echo $(DEV_MOD_TAG_LIST)
	@echo

show_rel_list :
	@echo
	@echo file : $(FILE)
	@echo
	@echo $(REL_MOD_TAG_LIST)
	@echo

$(undef_mods) :

ifeq ($(MK_TYPE),SRC)
# if a checkout based on the local scripts is requested,
# remove the scripts from the lists
ifeq ($(LOCAL),1)
all_mod_list := $(filter-out release_scripts, $(all_mod_list))
dev_mod_list := $(filter-out release_scripts, $(dev_mod_list))
rel_mod_list := $(filter-out release_scripts, $(rel_mod_list))
endif
# get the head of all components branches
ifeq ("$(HEAD_REQ)", "Y")
get_release := $(foreach f, $(all_mod_list), $(eval $(call hndcvs_get_head, $(f), $(call get_rel_tag,$(f)))))
target : $(all_mod_list) $(PROD_VERSION) $(CONFIG_SETUP)
else
get_release := $(foreach f, $(rel_mod_list), $(eval $(call hndcvs_get_release, $(f), $(call get_rel_tag,$(f)))))
get_dev := $(foreach f, $(dev_mod_list), $(eval $(call hndcvs_get_dev, $(f), $(call get_rel_tag,$(f)))))
target : $(dev_mod_list) $(rel_mod_list) $(PROD_VERSION) $(CONFIG_SETUP) 
endif
endif

ifneq ($(FIND_OP),)
ifeq ("$(HEAD_REQ)", "Y")
get_release := $(foreach f, $(all_mod_list), $(eval $(call hndcvs_find, $(f), -h,$(call get_rel_tag,$(f)))))
target : $(all_mod_list)
else
get_release := $(foreach f, $(rel_mod_list), $(eval $(call hndcvs_find, $(f),, $(call get_rel_tag,$(f)))))
get_dev := $(foreach f, $(dev_mod_list), $(eval $(call hndcvs_find, $(f),-h, $(call get_rel_tag,$(f)))))
target : $(dev_mod_list) $(rel_mod_list)
endif
endif

ifeq ("$(MK_TYPE)", "FILELIST")
get_release := $(foreach f, $(rel_mod_list), $(eval $(call hndcvs_generic, $(f),-of $(OUT_FILE) $(REL_OPT) -flst, $(call get_rel_tag,$(f)))))
get_dev := $(foreach f, $(dev_mod_list), $(eval $(call hndcvs_generic, $(f),-of $(OUT_FILE) $(HEAD_OPT) -flst, $(call get_rel_tag,$(f)))))
target : $(dev_mod_list) $(rel_mod_list)
endif

# getting a release from the BOM. We need to get the bom itself from whatever tag is 
# passed instead of the static tag to be able to extract the release 
# corresponding to a TOB bom
ifeq ("$(MK_TYPE)", "REL_SRC")
all_mod_list := $(filter-out release_scripts, $(all_mod_list)) 
get_release := $(foreach f, $(all_mod_list), $(eval $(call hndcvs_get_release, $(f), $(call get_rel_tag,$(f)))))
bom_script := $(eval $(call hndcvs_get_release, release_scripts, $(TAG_NAME))) 
ifneq ($(LOCAL),1)
target : release_scripts $(all_mod_list) $(PROD_VERSION) $(CONFIG_SETUP)
else
target : $(all_mod_list) $(PROD_VERSION) $(CONFIG_SETUP)
endif
endif

# this is for tagging modules without making a product release
ifeq ("$(MK_TYPE)", "TAG_BRANCH")
get_release := $(foreach f, $(all_mod_list), $(eval $(call hndcvs_tag_release, $(f), $(call get_rel_tag,$(f)))))
target : $(all_mod_list)
endif

# making a release
ifeq ("$(MK_TYPE)", "TAG")
#for backward compatibility
BOM_REL_TAG ?= $(ROUTER_REL_TAG)
# verify the BOM_REL_TAG
ifneq ("$(TAG_NAME)", "$(BOM_REL_TAG)")
target :
	@echo Tag name $(TAG_NAME) must match $(BOM_REL_TAG)  in BOM !!
else

ifeq ("$(FORCE_TAG)", "1")
get_release := $(foreach f, $(all_mod_list), $(eval $(call hndcvs_force_tag_release, $(f),  $(call get_rel_tag,$(f)), $(TAG_NAME))))
target : $(all_mod_list)
else 
get_release := $(foreach f, $(all_mod_list), $(eval $(call hndcvs_tag_release, $(f), $(call get_rel_tag,$(f)))))
target : $(all_mod_list)
endif

endif
endif


ifeq ("$(MK_TYPE)", "DIFF_HEAD_HEAD")
include $(DIFF_MAKE)
get_rel_common := $(foreach f, $(rel_common), $(eval $(call hndcvs_diff_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_common := $(foreach f, $(dev_common), $(eval $(call hndcvs_diff_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_rel := $(foreach f, $(dev_rel), $(eval $(call hndcvs_diff_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_rel_dev := $(foreach f, $(rel_dev), $(eval $(call hndcvs_diff_dev, $(f), $(call get_rel_tag2,$(f)), $(call get_rel_tag,$(f))  )))
target : display_diff $(rel_common) $(dev_common) $(dev_rel) $(rel_dev)

endif

ifeq ("$(MK_TYPE)", "DIFF_BRANCH_HEAD")
include $(DIFF_MAKE)
get_rel_common := $(foreach f, $(rel_common), $(eval $(call hndcvs_diff_rel_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_common := $(foreach f, $(dev_common), $(eval $(call hndcvs_diff_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_rel := $(foreach f, $(dev_rel), $(eval $(call hndcvs_diff_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_rel_dev := $(foreach f, $(rel_dev), $(eval $(call hndcvs_diff_rel_dev, $(f), $(call get_rel_tag2,$(f)), $(call get_rel_tag,$(f))  )))
target : display_diff $(rel_common) $(dev_common) $(dev_rel) $(rel_dev)
endif


ifeq ("$(MK_TYPE)", "DIFF_TAG_HEAD")
include $(DIFF_MAKE)
get_rel_common := $(foreach f, $(rel_common), $(eval $(call hndcvs_diff_rel_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_common := $(foreach f, $(dev_common), $(eval $(call hndcvs_diff_rel_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_rel := $(foreach f, $(dev_rel), $(eval $(call hndcvs_diff_rel_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_rel_dev := $(foreach f, $(rel_dev), $(eval $(call hndcvs_diff_rel_dev, $(f), $(call get_rel_tag2,$(f)), $(call get_rel_tag,$(f))  )))

target : display_diff $(rel_common) $(dev_common) $(dev_rel) $(rel_dev)
endif

ifeq ("$(MK_TYPE)", "DIFF_BRANCH_BRANCH")
include $(DIFF_MAKE)
get_rel_common := $(foreach f, $(rel_common), $(eval $(call hndcvs_diff_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_common := $(foreach f, $(dev_common), $(eval $(call hndcvs_diff_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_rel := $(foreach f, $(dev_rel), $(eval $(call hndcvs_diff_dev_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_rel_dev := $(foreach f, $(rel_dev), $(eval $(call hndcvs_diff_rel_dev, $(f), $(call get_rel_tag2,$(f)), $(call get_rel_tag,$(f))  )))
target : display_diff $(rel_common) $(dev_common) $(dev_rel) $(rel_dev)

endif


ifeq ("$(MK_TYPE)", "DIFF_BRANCH_TAG")
include $(DIFF_MAKE)
get_rel_common := $(foreach f, $(rel_common), $(eval $(call hndcvs_diff_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_common := $(foreach f, $(dev_common), $(eval $(call hndcvs_diff_dev_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_rel := $(foreach f, $(dev_rel), $(eval $(call hndcvs_diff_dev_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_rel_dev := $(foreach f, $(rel_dev), $(eval $(call hndcvs_diff_rel, $(f), $(call get_rel_tag2,$(f)), $(call get_rel_tag,$(f))  )))
target : display_diff $(rel_common) $(dev_common) $(dev_rel) $(rel_dev)
endif

ifeq ("$(MK_TYPE)", "DIFF_TAG_BRANCH")
include $(DIFF_MAKE)
get_rel_common := $(foreach f, $(rel_common), $(eval $(call hndcvs_diff_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_common := $(foreach f, $(dev_common), $(eval $(call hndcvs_diff_rel_dev, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_rel := $(foreach f, $(dev_rel), $(eval $(call hndcvs_diff_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_rel_dev := $(foreach f, $(rel_dev), $(eval $(call hndcvs_diff_rel_dev, $(f), $(call get_rel_tag2,$(f)), $(call get_rel_tag,$(f))  )))

target : display_diff $(rel_common) $(dev_common) $(dev_rel) $(rel_dev)
endif

ifeq ("$(MK_TYPE)", "DIFF_TAG_TAG")
include $(DIFF_MAKE)

get_rel_common := $(foreach f, $(rel_common), $(eval $(call hndcvs_diff_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_common := $(foreach f, $(dev_common), $(eval $(call hndcvs_diff_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_dev_rel := $(foreach f, $(dev_rel), $(eval $(call hndcvs_diff_rel, $(f), $(call get_rel_tag,$(f)), $(call get_rel_tag2,$(f))  )))
get_rel_dev := $(foreach f, $(rel_dev), $(eval $(call hndcvs_diff_rel, $(f), $(call get_rel_tag2,$(f)), $(call get_rel_tag,$(f))  )))

target : display_diff $(rel_common) $(dev_common) $(dev_rel) $(rel_dev) 
endif

ifeq ("$(MK_TYPE)", "DISPLAY")
get_release := $(foreach f, $(rel_mod_list), $(eval $(call hndcvs_generic, $(f),$(REL_OPT) -v,$(call get_rel_tag,$(f)))))
get_dev := $(foreach f, $(dev_mod_list), $(eval $(call hndcvs_generic, $(f),$(HEAD_OPT) -v, $(call get_rel_tag,$(f)))))
target : $(dev_mod_list) $(rel_mod_list)
endif

#FOR DIFFS ONLY
else
# Just compute the lists that will be needed by the first instance for 
# diffing purposes and exit.
# there are no rules defined here.

#override the defualt "module_list.mk" by the one for the second bom
include $(DIFF_MOD_FILE)

# list of dev modules, without tags 
dev_mod_list2 := $(foreach mod, $(PROD_DEV_LIST), $(call get_module, $(mod)))

# list of all modules without tags
all_mod_list2 := $(foreach mod, $(ALL_PROD_LIST), $(call get_module, $(mod)))

# list of all modules without tags
rel_mod_list2 := $(foreach mod, $(PROD_REL_LIST), $(call get_module, $(mod)))

#module override list
ow_mod_list2 := $(foreach mod, $(OVERRIDE_LIST), $(call get_module, $(mod)))

#new release list without the override list
upd_rel_list2 := $(filter-out $(ow_mod_list2), $(all_mod_list2))

#re-construct the non-override module list with tags associated

UPD_RELEASE_LIST2 := $(foreach f, $(upd_rel_list2), $(filter $(strip $(f))...%, $(RELEASE_LIST)))  

RELEASE_LIST_2_2 := $(OVERRIDE_LIST) $(UPD_RELEASE_LIST2) 

get_rel_tag2= $(call get_tag, $(filter $(strip $(1))...%, $(RELEASE_LIST_2_2)))

dev_common := $(filter $(dev_mod_list), $(dev_mod_list2))
rel_common := $(filter $(rel_mod_list), $(rel_mod_list2))

# modules in release in 1 but not in 2
rel_diff1 := $(filter-out $(rel_mod_list2), $(rel_mod_list))
# modules in release in 1 and in dev 2
rel_dev := $(filter $(rel_diff1), $(dev_mod_list2))
# modules in release in 1, not in release 2 and not in dev 2 => exists only in rel 1
rel1 := $(filter-out $(rel_dev), $(rel_diff1))


dev_diff2 := $(filter-out $(dev_mod_list), $(dev_mod_list2))
# modules in dev 2 not in dev1 and not in rel 1 => exists only in dev 2
dev2 := $(filter-out $(rel_dev), $(dev_diff2))


# modules in release in 2 but not in rel 1
rel_diff2 := $(filter-out $(rel_mod_list), $(rel_mod_list2))
# modules in release in 2 and in dev 1
dev_rel := $(filter $(rel_diff2), $(dev_mod_list))
# modules in release in 2, not in release 1 and not in dev 1 => exists only in rel 2
rel2 := $(filter-out $(dev_rel), $(rel_diff2))

dev_diff1 := $(filter-out $(dev_mod_list2), $(dev_mod_list))
# modules in dev 1 not in dev2 and not in rel2 => exists only in dev 1
dev1 := $(filter-out $(dev_rel), $(dev_diff1))

endif
