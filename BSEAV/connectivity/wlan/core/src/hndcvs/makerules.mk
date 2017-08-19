CVS_EXTRA=-z3

ifneq ("$(HNDCVS_DBG)","FALSE")
CVS_CMD := cvs $(CVS_EXTRA)
else
CVS_CMD := cvs -Q  $(CVS_EXTRA)
endif


#If this is set, we are being included by another instance ...
ifeq ("$(RULES_INCLUDED)","TRUE")
# .... then,  copy the sources list
# for diffing. See MK_TYPE = DIFF below. 
DIFF2_LIST := $(SOURCES)
LOCAL_DIFF2_LIST := $(LOCAL_SOURCES) 
# Regular case : this is the only (or first) instance of makerules.mk 

else
RULES_INCLUDED := TRUE

ifeq ("$(MK_TYPE)", "TAG_REC")
SOURCES :=
endif

# By default, use ToC or ToP
TAG_NAME =

# Used only when MK_TYPE = BRANCH
BRANCH_NAME =

# existing STATIC TAG of the mk file, if it exists
# Note: "override PROD_TAG = $Name: not supported by cvs2svn $" MUST be included in ALL 
# components mk files.

MK_TAG :=  $(word 2, $(PROD_TAG))

# MK_TYPE can be 
#    SRC (extract sources)
#    REL_TAG (tag a release)
#    REL_SRC : checkout released source (MUST have a STATIC tag)
#    BRANCH 
#    DIFF  
# default : SRC

MK_TYPE=SRC

ifneq ("$(TAG_NAME)", "")
	RTAG=-r $(TAG_NAME)
endif

ifneq ("$(BRANCH_NAME)", "")
	RBRANCH=-r $(BRANCH_NAME)
endif

ifneq ("$(DATE)", "")
	DATE_CMD =-D "$(DATE)"
endif



ifeq ("$(MK_TYPE)", "DIFF")

ifneq ("$(HNDCVS_DBG)","TRUE")
CVS_CMD := cvs -Q  $(CVS_EXTRA)
endif

DIFF1_LIST := $(SOURCES)
LOCAL_DIFF1_LIST := $(LOCAL_SOURCES) 
include $(DIFF_MAKE)
m2_list := $(filter-out $(DIFF1_LIST), $(DIFF2_LIST))
m1_list := $(filter-out $(DIFF2_LIST), $(DIFF1_LIST))
m_common := $(filter $(DIFF1_LIST), $(DIFF2_LIST))

loc_m2_list := $(filter-out $(LOCAL_DIFF1_LIST), $(LOCAL_DIFF2_LIST))
loc_m1_list := $(filter-out $(LOCAL_DIFF2_LIST), $(LOCAL_DIFF1_LIST))
loc_m_common := $(filter $(LOCAL_DIFF1_LIST), $(LOCAL_DIFF2_LIST))

all : $(m1_list) $(m2_list) $(m_common) $(loc_m2_list) $(loc_m1_list) $(loc_m_common)

$(m1_list) : phony
	@echo "$@ is only in $(TAG_NAME) version"

$(loc_m1_list) : phony
	@echo "$@ is only in $(TAG_NAME) version"

$(m2_list) : 
	@echo "$@ is only in $(BRANCH_NAME) version"

$(loc_m2_list) : 
	@echo "$@ is only in $(BRANCH_NAME) version"

$(m_common) : phony
ifeq ($(diff_opt),-rlog)
	@echo "cvs rlog $(CVS_OPT) -S -N -r$(TAG_NAME)::$(BRANCH_NAME) $@"
	-$(CVS_CMD) rlog  $(CVS_OPT) -S -N -r$(TAG_NAME)::$(BRANCH_NAME) $@
	#-cvs -Q rlog  $(CVS_OPT) -S -N -r$(TAG_NAME)::$(BRANCH_NAME) $@
else
	@echo "cvs rdiff $(CVS_OPT)-s -r $(TAG_NAME) -r $(BRANCH_NAME) $@"
	-$(CVS_CMD) rdiff  $(CVS_OPT) -s -r $(TAG_NAME) -r $(BRANCH_NAME) $@
endif

$(loc_m_common) : phony
ifeq ($(diff_opt),-rlog)
	@echo "cvs rlog $(CVS_OPT) -N -S -r$(TAG_NAME)::$(BRANCH_NAME) $@"
	-$(CVS_CMD) rlog  $(CVS_OPT) -N -S -r$(TAG_NAME)::$(BRANCH_NAME) $@
else
	@echo "cvs $(diff_opt) rdiff -s $(CVS_OPT) -r $(TAG_NAME) -r $(BRANCH_NAME) $@"
	-$(CVS_CMD) rdiff  $(CVS_OPT) -s -r $(TAG_NAME) -r $(BRANCH_NAME) $@
endif
endif

ifeq ("$(MK_TYPE)", "SRC")

ifneq ("$(SOURCES)","")

TARGET += rec_checkout
endif

ifneq ("$(LOCAL_SOURCES)","")
TARGET += loc_checkout
endif

all : $(TARGET) $(MOD_VERSION)

rec_checkout:
	$(CVS_CMD) co $(CVS_OPT) -AP $(DATE_CMD) $(RTAG) $(SOURCES)

loc_checkout:
	$(CVS_CMD) co -l $(CVS_OPT) -AP $(DATE_CMD) $(RTAG) $(LOCAL_SOURCES)
endif

ifeq ("$(MK_TYPE)", "REL_SRC")
ifneq ("$(SOURCES)","")
TARGET += rec_checkout
endif

ifneq ("$(LOCAL_SOURCES)","")
TARGET += loc_checkout
endif

all : $(TARGET) $(MOD_VERSION)

rec_checkout:
	$(CVS_CMD) co $(CVS_OPT) -AP $(DATE_CMD) $(RTAG) $(SOURCES)

loc_checkout:
	$(CVS_CMD) co -l $(CVS_OPT) -AP $(DATE_CMD) $(RTAG) $(LOCAL_SOURCES)
endif

ifeq ("$(MK_TYPE)", "TAG")
all : $(SOURCES) $(LOCAL_SOURCES)
ifeq ("$(TAG_NAME)", "")
	@echo "TAG not defined !!"
else
$(SOURCES): phony
	@echo "Tagging  $@ with $(TAG_NAME) on branch $(RBRANCH) "
	cvs rtag $(CVS_OPT) $(RBRANCH) $(TAG_NAME) $@

$(LOCAL_SOURCES): phony
	@echo "Tagging  $@ with $(TAG_NAME) on branch $(RBRANCH)"
	cvs rtag -l $(CVS_OPT) $(RBRANCH) $(TAG_NAME) $@
endif
endif

ifeq ("$(MK_TYPE)", "FORCE_TAG")
all : $(SOURCES) $(LOCAL_SOURCES)
ifeq ("$(TAG_NAME)", "")
	@echo "TAG not defined !!"
else
$(SOURCES): phony
	@echo "Tagging  $@ with $(TAG_NAME) on branch $(RBRANCH) "
	cvs rtag $(CVS_OPT) $(RBRANCH) $(TAG_NAME) $@

$(LOCAL_SOURCES): phony
	@echo "Tagging  $@ with $(TAG_NAME) on branch $(RBRANCH) "
	cvs rtag -l $(CVS_OPT) $(RBRANCH) $(TAG_NAME) $@
endif
endif


ifeq ("$(MK_TYPE)", "BRANCH")
all : $(SOURCES) $(LOCAL_SOURCES)
ifeq ("$(TAG_NAME)", "")
	@echo "TAG not defined !!"
else
ifeq ("$(BRANCH_NAME)", "")
	@echo "BRANCH NAME not defined !!"
else

$(SOURCES): phony
	@echo "Branching $@ with $(BRANCH_NAME) at $(TAG_NAME)"
	cvs rtag $(CVS_OPT) -b -r $(TAG_NAME) $(BRANCH_NAME) $@

$(LOCAL_SOURCES): phony
	@echo "Branching $@ with $(BRANCH_NAME) at $(TAG_NAME)"
	cvs rtag -l $(CVS_OPT) -b -r $(TAG_NAME) $(BRANCH_NAME) $@
endif
endif
endif

# find a directory
ifeq ("$(MK_TYPE)", "FIND")

ifneq ($(FILE),)

ifeq ($(FIND_OP),-mf)
find_file=1
find_match=1
endif
ifeq ($(FIND_OP),-md)
find_file=0
find_match=1
endif
ifeq ($(FIND_OP),-ff)
find_file=1
find_match=0
endif
ifeq ($(FIND_OP),-fd)
find_file=0
find_match=0
endif
ifeq ($(FIND_OP),-logd)
show_log=1
find_file=0
find_match=0
endif
ifeq ($(FIND_OP),-logf)
show_log=1
find_file=1
find_match=0
endif

ifeq ("$(TAG_NAME)","")
MATCH_TAG= TOT
else
MATCH_TAG= $(TAG_NAME)
endif

#if trying to match a tag, filter out non-matching tag and eliminate the 
# tag extension
ifeq ($(find_match),1)

match_list := $(filter %...$(MATCH_TAG),$(FILE))
match_files := $(patsubst %...$(MATCH_TAG),%,$(match_list))

else
# otherwise use the original files/dirs
match_files:=$(FILE)
endif

DIR_F :=$(dir $(match_files)) 


# look for files in sub-directories of recursive dirs
match :=$(foreach  dir, $(SOURCES), $(filter $(dir)/%,$(match_files)))
# look for files/dirs mtaching exactly recursive dirs
match +=$(foreach  dir, $(SOURCES), $(filter $(dir),$(match_files)))
# look for dirs/files matching exactly non-recursive dirs
match +=$(foreach  dir, $(LOCAL_SOURCES), $(filter $(dir),$(match_files)))
# if looking for files only, look right under non-recursive dirs 
ifeq ($(find_file),1)
match +=$(foreach  dir, $(LOCAL_SOURCES), $(filter $(dir)/,$(DIR_F)))
endif

# need to sort to clean up the list otherwise anyting matches the filter below (???)
match2=$(sort $(match))

ifneq ($(strip $(match)),)
found := $(filter $(match2)%,$(match_files))
endif

ifneq ($(strip $(found)),)
.PHONY: $(found) 
all : $(found)
$(found) :
	@echo file $@ found in $(MOD) with TAG $(MATCH_TAG)
ifeq ($(show_log),1)
ifeq ($(TAG_NAME),)
	@echo cvs rlog $(CVS_OPT) -N -S $@	
	cvs rlog $(CVS_OPT) -N -S $@ 
else
	@echo cvs rlog $(CVS_OPT) -N -S -r$(TAG_NAME) $@
	cvs rlog $(CVS_OPT) -N -S -r$(TAG_NAME) $@
endif
endif
else
all :
	@echo  -n
endif
endif
endif

# find a file
ifeq ("$(MK_TYPE)", "FIND_F")
DIR_F :=$(dir $(FILE)) 
#all: notfound
ifneq ($(FILE),)
dir_file:=$(foreach  dir, $(SOURCES), $(filter $(dir)/%,$(FILE)))
dir_file+=$(foreach  dir, $(SOURCES), $(filter $(dir),$(FILE)))
dir_file+=$(foreach  dir, $(LOCAL_SOURCES), $(filter $(dir),$(FILE)))
found = $(dir_file)
ifeq ($(strip $(dir_file)),)
dir_file+=$(foreach  dir, $(LOCAL_SOURCES), $(filter $(dir)/,$(DIR_F)))
found = $(filter $(dir_file)%,$(FILE))
endif

ifneq ($(strip $(dir_file)),)
all: $(found)
.PHONY: $(found) 
$(found):
	@echo file $(found)  found in $(MOD)
else
all:
	@echo -n
endif
endif
endif

# find a directory
ifeq ("$(MK_TYPE)", "MATCH_D")
ifneq ($(FILE),)
dir_file:=$(foreach  dir, $(SOURCES), $(filter $(dir)/%,$(FILE)))
dir_file+=$(foreach  dir, $(SOURCES), $(filter $(dir),$(FILE)))
dir_file+=$(foreach  dir, $(LOCAL_SOURCES), $(filter $(dir),$(FILE)))

ifneq ($(strip $(dir_file)),)
.PHONY: $(dir_file) 
all : $(dir_file)
$(dir_file) :
	@echo file $@ found in $(MOD)
else
all : :
	@echo -n
endif
endif
endif

# find a file
ifeq ("$(MK_TYPE)", "MATCH_F")
ifeq ("$(TAG_NAME)","")
MATCH_TAG= TOT
else
MATCH_TAG= $(TAG_NAME)
endif

match_list := $(filter %...$(MATCH_TAG),$(FILE))
match2 := $(patsubst %...$(MATCH_TAG),%,$(match_list))

DIR_F :=$(dir $(match2)) 

ifneq ($(match_list),)

dir_file:=$(foreach  dir, $(SOURCES), $(filter $(dir)/%,$(match2)))
dir_file+=$(foreach  dir, $(SOURCES), $(filter $(dir),$(match2)))
dir_file+=$(foreach  dir, $(LOCAL_SOURCES), $(filter $(dir),$(match2)))
found = $(dir_file)

ifeq ($(strip $(dir_file)),)
dir_file+=$(foreach  dir, $(LOCAL_SOURCES), $(filter $(dir)/,$(DIR_F)))
found = $(filter $(dir_file)%,$(match2))
endif

ifneq ($(strip $(dir_file)),)
all: $(found)
.PHONY: $(found) 
$(found):
	@echo file $@  found in $(MOD) with TAG $(MATCH_TAG)
else
all:
	@echo -n
endif
endif
endif


phony :
#	@echo "MK_TYPE = $(MK_TYPE)"

endif
