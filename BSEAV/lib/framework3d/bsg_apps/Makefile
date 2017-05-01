################################################################################
#  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
################################################################################

.PHONY: bsg bsg_clean common common_clean

export BSG_PREBUILT := y

BSG_APPS := geom_viewer hello_anim hello_cube hello_fbo hello_task hello_text hello_lod logo_menu pg \
	    planets spytool_replay video_texturing_decoder

apps: bsg common $(foreach app,$(BSG_APPS),$(app))

clean: bsg_clean common_clean $(foreach app,$(BSG_APPS),$(app)_clean)

bsg:
	$(MAKE) -C ../bsg -f bsg.mk

bsg_clean:
	$(MAKE) -C ../bsg -f bsg.mk clean

common:
	$(MAKE) -C ./common -f common.mk

common_clean:
	$(MAKE) -C ./common -f common.mk clean

define BSG_APP_template
.PHONY: $1 $1_clean

$1: bsg common
	@$$(MAKE) -C $1

$1_clean: bsg_clean common_clean
	@$$(MAKE) -C $1 clean
endef

# Define rules for each app
$(foreach app,$(BSG_APPS),$(eval $(call BSG_APP_template,$(app))))
