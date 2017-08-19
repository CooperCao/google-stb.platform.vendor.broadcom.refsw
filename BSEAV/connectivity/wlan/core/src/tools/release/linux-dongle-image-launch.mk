
# This makefile remotely launches dongle image targets on linux target
#
# This makefile is intended to be included by other eSTA DHD brand builds
#

# Caller must have following variables set before including this makefile
# BRAND           = <build-brand-name>
# HNDRTE_DEPS     = <make-targets-that-must-be-built-before-dongle-images-built>
# HNDRTE_IMGFN    = <dongle-image-filename>; rtecdc.bin or rterndis.bin
# ALL_DNGL_IMAGES = <list-of-dongle-images-to-build>
# HNDRTE_PRIVATE_DIR = Optional name of subdirectory under misc that used for local temporary files. By default misc itself is used
#
# $Id$

UNAME                := $(shell uname)
NULL                 := /dev/null
TODAY                ?= $(strip $(shell date +'%Y.%-m.%-d'))
BUILD_BASE           ?= $(shell pwd)
export VCTOOL=svn

# Exit if dongle image file name format is not specified by caller
# It should be either rtecdc.h or rtecdcdhd.h
ifeq ($(HNDRTE_IMGFN),)
   $(error ERROR: HNDRTE_IMGFN=$(HNDRTE_IMGFN) is empty)
endif

# Warn if all dongle images list is empty. This is a dynamic/derived variable
# and requires checkout to have happened
ifeq ($(strip $(ALL_DNGL_IMAGES)),)
   $(warn WARN: ALL_DNGL_IMAGES=$(ALL_DNGL_IMAGES) is empty)
endif

ifeq ($(BRAND),)
   $(warning WARN: BRAND=$(BRAND) is empty)
   BRAND             ?= UNKNOWN_BRAND_$(TODAY)
   $(warning WARN: Setting BRAND=$(BRAND))
endif

BUILD_LINUX_DIR      ?= /projects/hnd_swbuild/build_linux
BUILD_LINUX_DIR_WIN  ?= Z:$(BUILD_LINUX_DIR)
BUILD_LINUX_TEMP     ?= $(BUILD_LINUX_DIR)/TEMP/prebuild/$(BRAND)
BUILD_LINUX_TEMP_WIN ?= Z:$(BUILD_LINUX_TEMP)
FORCE_HNDRTE_BUILD   ?= 0
HNDRTE_BRAND         ?= hndrte-dongle-wl
HNDRTE_PRIVATE_DIR   ?=
HNDRTE_FLAG          := $(BUILD_BASE)/misc/$(if $(HNDRTE_PRIVATE_DIR),$(HNDRTE_PRIVATE_DIR)/,)dongle_image_temp_build_spawned
HNDRTE_JOBID_FILE    := $(BUILD_BASE)/misc/$(if $(HNDRTE_PRIVATE_DIR),$(HNDRTE_PRIVATE_DIR)/,)dongle_image_temp_build_jobid
SSHHOST              ?= wlansw-cron.sj.broadcom.com
SSHOPTS              ?= -o StrictHostKeyChecking=no -o ConnectTimeout=60 -o ConnectionAttempts=10
SSHCMD               := ssh -q $(SSHOPTS) hwnbuild@$(SSHHOST) --
# Linux and Windows build nodes can run lsf commands so no need to ssh.
SSH                  := $(if $(filter Linux% CYGWIN%,$(UNAME)),$(SSHCMD))

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

# ITER refers to <date> subdir
# For all builds use an existing $(HNDRTE_BRAND) build, use any date
ITER   := 2*

# Except for following
# Override for TOT and TOB builds (i.e. rebuild $(HNDRTE_BRAND) every time)
ifeq ($(TAG),)
  ITER := $(TODAY).{?,??}
endif
ifeq ($(findstring _BRANCH_,$(TAG)),_BRANCH_)
  ITER := $(TODAY).{?,??}
endif
ifeq ($(findstring _TWIG_,$(TAG)),_TWIG_)
  ITER := $(TODAY).{?,??}
endif
ITER_DIR := $(or $(TAG),trunk)/$(HNDRTE_BRAND)/$(ITER)

# If regular nightly tot or tob build has not built a dongle image, then
# use hndrte_work_dir as a temp build space to generate private dongle image
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
  HNDRTE_PVT_DIR_ITER    = $(BUILD_LINUX_TEMP_WIN)/$(ITER_DIR)
  HNDRTE_SERVER_DIR_ITER  = $(BUILD_LINUX_DIR_WIN)/$(ITER_DIR)
else
  HNDRTE_PVT_DIR_ITER    = $(BUILD_LINUX_TEMP)/$(ITER_DIR)
  HNDRTE_SERVER_DIR_ITER  = $(BUILD_LINUX_DIR)/$(ITER_DIR)
endif

# File to search for LSF job-id in the launched build's environment log
ENV_LOG       = misc/,env.log
# If a private lsf job is submitted JOBID is defined
HNDRTE_JOBID=$(strip $(shell cat $(HNDRTE_JOBID_FILE) 2> $(NULL)))
HNDRTE_PVT_DIR_OLD=$(filter-out .,$(strip $(firstword \
			$(shell find $(HNDRTE_PVT_DIR_ITER) \
			-maxdepth 0 -mindepth 0 2> $(NULL) |  \
			xargs ls -1td))))

# If more than one private firmware build is in progress, fetch correct
# build dir that match launched lsf job-id
HNDRTE_PVT_DIR_NEW=$(strip \
		$(if $(HNDRTE_JOBID),\
			$(subst $(ENV_LOG),,\
				$(shell egrep -l "LSB_JOBID.*=.*$(HNDRTE_JOBID)$$" \
			   		$(HNDRTE_PVT_DIR_ITER)/$(ENV_LOG) 2> $(NULL) \
			   )\
			)\
		)\
	)

# If a private build is launched, use its lsf jobid for HNDRTE_PVT_DIR
# otherwise take the latest private build
HNDRTE_PVT_DIR=$(strip $(if $(HNDRTE_PVT_DIR_NEW),$(HNDRTE_PVT_DIR_NEW),$(HNDRTE_PVT_DIR_OLD)))

HNDRTE_SERVER_DIR=$(strip $(filter-out .,$(strip $(firstword \
			$(shell find $(HNDRTE_SERVER_DIR_ITER) \
			-maxdepth 0 -mindepth 0 2> $(NULL) |  \
			xargs ls -1td)))))

# Derive final effective HNDRTE_DIR variable

# If HNDRTE_FLAG exists, that indicates a private dongle build was spawned
# and it should be HNDRTE_DIR of choice.
# If HNDRTE_FLAG isn't found, then pick latest of pvt or public dongle dir
# TODO: For TAG builds, since the sources are static, we can share common
# TODO: firmware build area to speed up TAG builds. That requires some work
# TODO: in appropriate directory timestamp searching and built firmware
# TODO: staging at a common directory

HNDRTE_DIR=$(strip $(shell \
             if [ -f "$(HNDRTE_FLAG)" ]; then \
                echo "$(HNDRTE_PVT_DIR)"; \
             else \
                if [ "$(HNDRTE_PVT_DIR)" -nt "$(HNDRTE_SERVER_DIR)" ]; then \
                   echo "$(HNDRTE_PVT_DIR)"; \
                else \
                   echo "$(HNDRTE_SERVER_DIR)"; \
                fi; \
             fi; \
           ))

DNGL_IMGDIR ?= build/dongle

ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
  # HNDRTE_IMGDIR_FULL is created only for logging/recording purposes
  HNDRTE_IMGDIR_FULL=$(HNDRTE_DIR)/$(DNGL_IMGDIR)
  HNDRTE_IMGDIR=$(shell cygpath -m $(HNDRTE_DIR)/$(DNGL_IMGDIR))
else  # UNAME
  HNDRTE_IMGDIR=$(HNDRTE_DIR)/$(DNGL_IMGDIR)
endif # UNAME

HNDRTE_MAKEFLAGS_EXTRA=$(if $(COTOOL),COTOOL=$(COTOOL))
HNDRTE_MAKEFLAGS=BUILD_TARGET="$(subst $(space),$(comma),$(sort $(ALL_DNGL_IMAGES)))" $(HNDRTE_MAKEFLAGS_EXTRA)

# Wait time between lsf query. (Default is 3min. i.e. 180seconds)
HNDRTE_WAITTIME  ?= 60
# How many times to check build status, default 60times $(HNDRTE_WAITTIME)
# Per Wait loop = 90mins (HNDRTE_WAITTIME * HNDRTE_LOOPITERS)
# TODO: Temporary change until TOT fixes for firmware synchronization are
# TODO: are merged
HNDRTE_LOOPITERS ?= 60

# Most private firmware builds must finish within one loop. However
# sometimes some builds take longer, so above loop is repeated few times
# Default: Don't wait for more than 9 hours (180 sec X 30 X 6)!
# Any build going beyond this time, indicates serious error with build/LSF
HNDRTE_REPEAT_LOOPS   ?= 6

# Approximate/conservative time per firmware image to build (in seconds)
# NOTE: Above loops can be adjusted based on this HNDRTE_IMGTIME
HNDRTE_IMGTIME   ?= 30

ifdef DBG
$(warning INFO: HNDRTE_WAITTIME     = $(HNDRTE_WAITTIME) seconds)
$(warning INFO: HNDRTE_LOOPITERS    = $(HNDRTE_LOOPITERS))
$(warning INFO: HNDRTE_REPEAT_LOOPS = $(HNDRTE_REPEAT_LOOPS))
$(warning INFO: HNDRTE_IMGTIME      = $(HNDRTE_IMGTIME))
endif # DBG

# Built dongle image files to copy over before launching dhd builds
DNGL_IMG_PFX     ?= $(basename $(notdir $(HNDRTE_IMGFN)))
DNGL_IMG_FILES   ?= $(DNGL_IMG_PFX).h \
	            $(DNGL_IMG_PFX).bin \
	            $(DNGL_IMG_PFX).map \
	            $(DNGL_IMG_PFX).exe \
	            $(DNGL_IMG_PFX).dis \
	            logstrs.bin

HNDRTE_JOB_STATUS_CHECK=\
	lsfout=`$(SSH) bjobs $${lsfjobid} | \
		egrep "$${lsfjobid}.*DONE|$${lsfjobid}.*EXIT|$${lsfjobid}.*is not found"`; \
	if [ "$${lsfout}" != "" ]; then \
		echo "([$$(date +%T)] $$n) lsf completion output : $$lsfout"; \
		break; \
	fi

HNDRTE_WAIT_LOOP=\
	echo "Waiting for lsf job-id $$lsfjobid build to complete"; \
	for n in $(shell seq 1 $(HNDRTE_LOOPITERS)); do \
	    sleep $(HNDRTE_WAITTIME); \
	    $(HNDRTE_JOB_STATUS_CHECK); \
	    echo "  ([$$(date +%T)] $$n) Wait $(HNDRTE_WAITTIME) seconds for lsf job-id $$lsfjobid"; \
	done

#
# In an already existing dongle build exists, search for images we need
# If a private dongle build is in progress, wait for missing images to build
#
find_dongle_images: $(HNDRTE_DEPS)
	@$(MARKSTART)
	@echo "==============================="
	@echo "Requested_images  = $(sort $(ALL_DNGL_IMAGES))"
	@echo "==============================="
	@n=0; missing_images=0; found_images=0; imgdir="$(HNDRTE_IMGDIR)"; \
	echo "Checking if prebuilt dongle images exist at:"; \
  	echo "DBG: HNDRTE_JOBID           = $(HNDRTE_JOBID)"; \
  	echo "DBG: HNDRTE_PVT_DIR_ITER    = $(HNDRTE_PVT_DIR_ITER)"; \
  	echo "DBG: HNDRTE_SERVER_DIR_ITER = $(HNDRTE_SERVER_DIR_ITER)"; \
  	echo "DBG: HNDRTE_PVT_DIR_OLD     = $(HNDRTE_PVT_DIR_OLD)"; \
  	echo "DBG: HNDRTE_PVT_DIR_NEW     = $(HNDRTE_PVT_DIR_NEW)"; \
  	echo "DBG: HNDRTE_PVT_DIR         = $(HNDRTE_PVT_DIR)"; \
  	echo "DBG: HNDRTE_SERVER_DIR      = $(HNDRTE_SERVER_DIR)"; \
	echo "HNDRTE_IMGDIR=$${imgdir}"; \
	if [ ! -d $${imgdir} ]; then \
	   exit 0; \
	fi; \
	cd $${imgdir}; \
	for img in $(sort $(ALL_DNGL_IMAGES)); do \
	   if [ -s "$$img/$(DNGL_IMG_PFX).h" ]; \
	   then \
	      echo "  [$$n] Found Prebuilt Image  : $$img"; \
	      found_images=$$(($$found_images + 1)); \
	   else \
	      echo "* [$$n] Missing Prebuilt Image: $$img"; \
	      missing_images=$$(($$missing_images + 1)); \
	   fi; \
	   n=$$(($$n + 1)); \
	done; \
	if [ "$${missing_images}" != "0" ]; then \
	   build_in_progress=0; \
	   if echo $${imgdir} | egrep -q "TEMP/prebuild"; then \
	      echo "WARN: Another private dongle build in progress"; \
	      echo "WARN: at $${imgdir}"; \
	      build_in_progress=1; \
	   elif echo $${imgdir} | egrep -q "swbuild/build_linux/.*$(or $(TAG),trunk)"; then \
	      echo "WARN: Public dongle build in progress"; \
	      echo "WARN: at $${imgdir}"; \
	      build_in_progress=1; \
	   fi; \
	   if [ "$$build_in_progress" == "1" ]; then \
	      for w in $$(seq 1 $$missing_images); do \
	         echo "WARN: Waiting $(HNDRTE_IMGTIME) seconds for build to finish"; \
	         sleep $(HNDRTE_IMGTIME); \
	         if [ -f "$(HNDRTE_DIR)/,succeeded" ]; then \
	            echo "INFO: Exiting wait loop for missing images"; \
	            echo "INFO: Found $(HNDRTE_DIR)/,succeeded"; \
	            break; \
	         elif [ -f "$(HNDRTE_DIR)/,build_errors.log" ]; then \
	            echo "INFO: Exiting wait loop for missing images"; \
	            echo "INFO: Found $(HNDRTE_DIR)/,build_errors.log"; \
	            break; \
	         fi; \
	      done; \
	   fi; \
	   n=0; missing_images=0; imgdir="$${imgdir}"; \
	   echo "Re-checking if private dongle images are built now"; \
	   for img in $(sort $(ALL_DNGL_IMAGES)); do \
	      if [ -s "$$img/$(DNGL_IMG_PFX).h" ]; \
	      then \
	         echo "  [$$n] Found Prebuilt Image  : $$img"; \
	      else \
	         echo "* [$$n] Still Missing Prebuilt Image: $$img"; \
	         missing_images=$$(($$missing_images + 1)); \
	      fi; \
	      n=$$(($$n + 1)); \
	   done; \
	   if [ "$${missing_images}" != "0" ]; then \
	      echo "WARN: Some dongle images needed are missing"; \
	      echo "WARN: Can't reuse $${imgdir}"; \
	      echo "WARN: $$missing_images of $$n images missing"; \
	   fi; \
	fi
	@$(MARKEND)

#
# If any image we want is missing launch a private build
#
build_dongle_images: $(HNDRTE_DEPS) find_dongle_images
	@$(MARKSTART)
	@echo "HNDRTE_DIR = $(HNDRTE_DIR)"
ifneq ($(HNDRTE_PRIVATE_DIR),)
	$(INSTALL) -d $(BUILD_BASE)/misc/$(HNDRTE_PRIVATE_DIR)
endif
	@if [ -d "$(HNDRTE_IMGDIR)" ]; then \
	   pushd "$(HNDRTE_IMGDIR)"; \
	   if [ $(foreach img,$(sort $(ALL_DNGL_IMAGES)), -s "$(img)/$(DNGL_IMG_PFX).h" -a) "$(FORCE_HNDRTE_BUILD)" != "1" ]; then \
	      echo "INFO: Previous HNDRTEDIR $(HNDRTE_DIR) exists. Reusing it"; \
	      exit 0; \
	   fi; \
	   popd; \
	fi; \
	echo "Private $(HNDRTE_BRAND) build is needed"; \
	if [ "$(strip $(ALL_DNGL_IMAGES))" == "" ]; then \
	   echo "ERROR: ALL_DNGL_IMAGES list empty. Can't continue"; \
	   exit 1; \
	fi; \
	rm -fv $(HNDRTE_FLAG) && \
	echo "Starting private $(HNDRTE_BRAND) linux build in:" && \
	echo "'$(BUILD_LINUX_TEMP)' directory" && \
	date "+[%D %T]: BEGIN $(HNDRTE_BRAND)" && \
	echo "Images: $(sort $(ALL_DNGL_IMAGES))" && \
	echo "Creating $(HNDRTE_FLAG) flag file" && \
	echo "HNDRTE_DIR=$(BUILD_LINUX_TEMP)" > $(HNDRTE_FLAG) && \
	set -x && \
	$(strip GUB_DEPTH=0 GUB_LAST_HOP= \
	    $(GUB_DIR)gub $(GUB_GENOPTS) -F build $(GUB_BLDOPTS) \
	    --child-build-by=$(or $(GUB_BY),$(LOGNAME)) \
	    --auto-subdir \
	    --noprompt \
	    -t $(or $(TAG),trunk) \
	    -b $(HNDRTE_BRAND) \
	    -d $(BUILD_LINUX_TEMP)/$(or $(TAG),trunk) \
	    --makeflags $(HNDRTE_MAKEFLAGS))
	@$(MARKEND)

ifeq (,$(filter Darwin,${UNAME}))
  .mtime = $(strip $(if $(strip $1),$(shell stat -c '%y' $1)))
else
  .mtime = $(strip \
             $(if $(strip $1), \
               $(shell \
                 func () { echo \$$9; } ; \
                 func `stat -t '%F %T.000000000 %z' $1` \
                ) \
              ) \
            )
endif

#
# Show the effective dongle build dir paths before copying happens
#
show_dongle_build_info: build_dongle_images
	@$(MARKSTART)
	@echo "  =========================================================="
	@echo "  Current_dir       = $(shell pwd)"
	@echo "  Current_host      = $(shell hostname)"
	@echo "  Hndrte_flag       = $(shell cat $(HNDRTE_FLAG) 2> $(NULL))"
	@echo "  Hndrte_pvt_dir    = $(HNDRTE_PVT_DIR)"
	@echo "  Hndrte_pvt_time   = $(call .mtime,$(HNDRTE_PVT_DIR))"
	@echo "  Hndrte_pvt_jobid  = $(HNDRTE_JOBID)"
	@echo "  Hndrte_server_dir = $(HNDRTE_SERVER_DIR)"
	@echo "  Hndrte_server_time= $(call .mtime,$(HNDRTE_SERVER_DIR))"
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
	@echo "  Final HNDRTE_PATH = $(HNDRTE_IMGDIR_FULL)"
endif # UNAME
	@echo "  Final HNDRTE_DIR  = $(HNDRTE_DIR)"
	@echo "  Targets_to_copy   = $(DNGL_IMG_FILES)"
	@echo "  Targets_found     ="
	-@n=1; \
	cd $(HNDRTE_IMGDIR); \
	for img in $(ALL_DNGL_IMAGES); do \
		echo -e "IMAGE[$$n]=$$img:\n"; \
	    $(if $(filter Darwin,$(UNAME)), \
	      stat -l , ls -gnGo --time-style="+%H:%M:%S" \
	     ) $(addprefix $$img/,$(DNGL_IMG_FILES)) ; \
		n=$$(expr $$n + 1); \
		echo "+ ----------------------------------------------"; \
	done
	@echo "  =========================================================="
	@$(MARKEND)

#
# Copy the dongle images we need from dongle build dir to dhd build dir
#
copy_dongle_images: build_dongle_images show_dongle_build_info
	@$(MARKSTART)
	@if [ "$(HNDRTE_DIR)" == "" ]; then \
		echo "ERROR: Dongle image build dir not found or failed at:"; \
		if [ -f "$(HNDRTE_FLAG)" ]; then \
			echo "$(HNDRTE_PVT_DIR_ITER)"; \
		else \
			echo "$(HNDRTE_SERVER_DIR_ITER)"; \
		fi; \
		exit 1; \
	fi
	# Ensure that build doesn't progress if a private dongle build was
	# launched and if public dongle build is picked by any chance
	@if [ -s "$(HNDRTE_FLAG)" -a "$(HNDRTE_DIR)" != "$(HNDRTE_PVT_DIR)" ]; then \
	    echo "ERROR: Wrong HNDRTE_DIR=$(HNDRTE_DIR) picked"; \
	    echo "ERROR: instead of private $(HNDRTE_PVT_DIR) build"; \
	    exit 1; \
	fi
	@failedimgs=""; imgdir="$(HNDRTE_IMGDIR)"; \
	if [ ! -d "$${imgdir}" ]; then \
	   echo "ERROR: Dongle images NOT BUILT"; \
	   exit 1; \
	fi; \
	pushd "$${imgdir}"; \
	for img in $(sort $(ALL_DNGL_IMAGES)); do \
	   if [ ! -f "$${img}/$(DNGL_IMG_PFX).h" ]; then \
	      echo "ERROR: Dongle image $${img} NOT BUILT"; \
	      failedimgs="  $${failedimgs} $${img}\n"; \
	   else \
	      install -dv $(BUILD_BASE)/$(DNGL_IMGDIR)/$${img}; \
	      for imgfile in $(DNGL_IMG_FILES:%=$${img}/%); do \
	        if [ -f "$${imgfile}" ]; then \
	           install -pv $${imgfile} $(BUILD_BASE)/$(DNGL_IMGDIR)/$${img}; \
	        fi; \
	      done; \
	   fi; \
	done; \
	popd; \
	if [ "$${failedimgs}" != "" ]; then \
	  echo -e "\nERROR: Following dongle images failed to build\n"; \
	  echo -e "\n$${failedimgs}\n"; \
	  exit 1; \
	fi
	@$(MARKEND)
