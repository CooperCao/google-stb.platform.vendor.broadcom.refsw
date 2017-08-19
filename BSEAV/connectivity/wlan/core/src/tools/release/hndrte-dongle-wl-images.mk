# Makefile for use in finding required firmware images from dependent buildables
# (firmware marked as ALL_DNGL_IMAGES in default buildables on current branch).
## The BUILD_TARGET option is intended to build specific images
## for other dependent builds to use (e.g. windows dongle build).

IMAGE_LIST_MK ?= /home/hwnbuild/src/tools/release/show-dongle-images.mk
ifdef BUILD_TARGET
  comma := ,
  space :=
  space +=
  DNGL_MAKE_IMAGES := $(subst $(comma),$(space),$(BUILD_TARGET))
else
  TAG ?= trunk
  DNGL_MAKE_IMAGES := $(shell set -x; make -s -f $(IMAGE_LIST_MK) TAG=$(TAG) DNGLSRCH=$(CURDIR))
endif
$(info DNGL_MAKE_IMAGES=$(DNGL_MAKE_IMAGES))
