#   Used with a normal makefile for distributed (lsmake) use. This file is needed
# because to derive the optimal -j value we need to know how many slots we were
# given by LSF. If we ask for too many slots the build job might hang waiting
# for them to become available, while if we ask for too few we'll starve ourself
# of needed resources. The solution is to ask for a range (e.g. bsub -n 12,64) and
# calculate MAKEJOBS (-j) dynamically based on how many slots we were actually given.
# This in turn requires a 2-pass build: on the first pass we re-invoke ourselves
# using bsub -n x,y, then on the second pass we can use the value of the LSB_HOST
# environment variable to determine MAKEJOBS and then do the build for real.
#   This makefile may be entered in 3 different scenarios:
#     A. LSMAKE_DYNAMIC is unset (or set to anything other than 1 or 2). In this case
# this makefile simply includes the underlying $(REAL_MAKEFILE) and does nothing
# else, becoming a nop.
#     B. LSMAKE_DYNAMIC is set to 1. This means we need to initiate setup by relaunching
# the same build using bsub and lsmake, and setting LSMAKE_DYNAMIC=2 so the new
# iteration will fall into case C.
#     C. LSMAKE_DYNAMIC is set to 2. This means setup is complete and we can start
# the build by including the underlying $(REAL_MAKEFILE), just as in case A,
# but having now derived an appropriate value of MAKEJOBS based on the number of
# slots we were given by LSF.
#   Bottom line, to enable an lsmake build with makefile foobar.mk, invoke:
#     % make -f lsmake-dynamic.mk REAL_MAKEFILE=foobar.mk LSMAKE_DYNAMIC=1 ...
# while the same line without the LSMAKE_DYNAMIC setting would behave just like:
#     % make -f foobar.mk ...
#   This design inverts the normal model; rather than having the "real" makefile
# include the helper, the helper includes the real makefile. The primary reason is
# the restriction that a make target may have only one recipe. Since the real
# makefile presumably has recipes for "all" etc we could not add them again within
# an include file. But with this inversion we can hide the real makefile until
# it's needed (in pass 2). It also means we can potentially add distributed
# capability to an existing makefile without modifying it.
# $Id: $

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))

LSMAKE_SLOT_REQ_PER_JOB ?= 2

ifeq ($(LSMAKE_DYNAMIC),1)
  $(if $(LSMAKE_JOB_COUNT),,$(error missing LSMAKE_JOB_COUNT))
  LSMAKE_SLOT_MAX_MIN ?= 36
  SLOT_REQ_MAX := $(shell echo $$(( $(LSMAKE_JOB_COUNT) * $(LSMAKE_SLOT_REQ_PER_JOB) )))
  SLOT_REQ_MIN := $(shell echo $$(( $(LSMAKE_JOB_COUNT) > $(LSMAKE_SLOT_MAX_MIN) ? $(LSMAKE_SLOT_MAX_MIN) : $(LSMAKE_JOB_COUNT) )))
  LSF_RES_REQ := $(filter osmajor=%,$(LSB_SUB_RES_REQ))
  ifndef LSF_RES_REQ
    LSF_RES_REQ := osmajor=RHEL6
  endif
  LSF_RES_REQ += span[ptile=$(LSMAKE_SLOT_REQ_PER_JOB)]
  LSMAKE_FLAGS ?= -V
  BSUB_CMD := bsub -I -q $(or $(LSMAKE_QUEUE),$(LSB_QUEUE),sj-wlanswbuild)
  BSUB_CMD += -R "'$(LSF_RES_REQ)'"
  BSUB_CMD += -n $(SLOT_REQ_MIN),$(SLOT_REQ_MAX)
  BSUB_CMD += -sp $(or $(LSMAKE_PRIORITY),$(LSF_PRIORITY),140)
  BSUB_CMD := $(strip $(BSUB_CMD))

  # wlan-lsmake has only been ported to RH Linux so far.
  ifneq (,$(wildcard /etc/redhat-release))
    LSMAKE := wlan-lsmake
    # The -N flag is a local wlan-lsmake enhancement to flush NFS caches on failure.
    ifeq (,$(findstring -N,$(LSMAKE_FLAGS)))
      LSMAKE_FLAGS += -N :
    endif
  else
    LSMAKE := lsmake
  endif

  # This is a list of make variables which must be preserved from pass 1 to pass 2.
  # The calling makefile may add to it.
  LSMAKE_DYNAMIC_PARAMS := $(sort $(LSMAKE_DYNAMIC_PARAMS) REAL_MAKEFILE LSMAKE_JOB_COUNT)

  # This should be safe as long as the default goal of the $(REAL_MAKEFILE) is "all".
  MAKECMDGOALS ?= all
  .PHONY: $(MAKECMDGOALS)
  $(MAKECMDGOALS):
	@echo Using: $(LSMAKE) $$($(LSMAKE) --version | egrep 'GNU Make|Platform Make|Broadcom')
	+$(strip $(BSUB_CMD) -- $(LSMAKE) -f $(THIS_MAKEFILE) \
	  $(foreach param,$(LSMAKE_DYNAMIC_PARAMS),$(param)='$($(param))') \
	  HOSTMAKE=$(MAKE) $(LSMAKE_FLAGS) LSMAKE_DYNAMIC=2)

else # LSMAKE_DYNAMIC

  ifeq ($(LSMAKE_DYNAMIC),2)
    # All we do in pass 2 is derive a value for MAKEJOBS and fall into the real makefile.
    # This dynamic value may need to override a static MAKEJOBS setting provided by the build tool.
    # When determining the number of jobs per image we round up.
    LSF_JOBS_MULTIPLIER ?= 2
    SLOTS_GIVEN := $(words $(LSB_HOSTS))
    SLOTS_GIVEN_PER_JOB := $(shell echo $$(( ($(SLOTS_GIVEN) + $(LSMAKE_JOB_COUNT) - 1) / $(LSMAKE_JOB_COUNT) )))
    export MAKEJOBS := -j$(shell echo $$(( $(SLOTS_GIVEN_PER_JOB) * $(LSF_JOBS_MULTIPLIER) )))
    $(info LSMAKE_JOB_COUNT=$(LSMAKE_JOB_COUNT))
    $(info LSMAKE_SLOT_REQ_PER_JOB=$(LSMAKE_SLOT_REQ_PER_JOB))
    $(info SLOTS_GIVEN=$(SLOTS_GIVEN))
    $(info SLOTS_GIVEN_PER_JOB=$(SLOTS_GIVEN_PER_JOB))
    $(info MAKEJOBS=$(MAKEJOBS))
    $(info MAKE=$(MAKE))
    $(info HOSTMAKE=$(HOSTMAKE))
    $(info REAL_MAKEFILE=$(REAL_MAKEFILE))
  endif # LSMAKE_DYNAMIC == 2

  # Last, fall into the real makefile whether we're using lsmake (LSMAKE_DYNAMIC==2) or GNU make.
  $(if $(REAL_MAKEFILE),,$(error missing REAL_MAKEFILE))
  include $(REAL_MAKEFILE)

endif # LSMAKE_DYNAMIC
