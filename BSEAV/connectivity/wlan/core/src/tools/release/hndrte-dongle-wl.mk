# Wrap a $(BRAND)-impl.mk dongle-building makefile for potential lsmake use.
# $Id: $

# Derive buildable name from makefile name unless overridden from above.
BRAND := $(or $(BRAND),$(basename $(notdir $(lastword $(MAKEFILE_LIST)))))

# Derive the set of images to build $(DNGL_MAKE_IMAGES).
include $(BRAND)-images.mk

# Bring in the lsmake-dynamic.mk helper file. If lsmake is not in use this will be a nop.
# It needs a few parameters from which to calculate LSF requests.
LSMAKE_JOB_COUNT := $(words $(DNGL_MAKE_IMAGES))
LSMAKE_SLOT_REQ_PER_JOB := 4
LSMAKE_DYNAMIC_PARAMS := DNGL_MAKE_IMAGES
REAL_MAKEFILE := $(BRAND)-impl.mk
include lsmake-dynamic.mk
