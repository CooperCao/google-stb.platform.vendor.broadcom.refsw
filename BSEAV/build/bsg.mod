############################################################
#
# BSG - Broadcom Scene Graph
#
############################################################

MAKE_MODULES += bsg

bsg_FRAMEWORK_DIR = $(B_REFSW_ROOT_DIR)/BSEAV/lib/framework3d
bsg_BUILD_DIR     = $(bsg_FRAMEWORK_DIR)/bsg_apps

bsg_NOT_LOADED    := $(shell test -d $(B_REFSW_ROOT_DIR)/BSEAV/lib/framework3d || echo "WARNING: BSEAV/lib/framework3d not loaded")

ifeq ($(bsg_NOT_LOADED),)
bsg_HELP          = "Broadcom Scene Graph - 3D Framework"

bsg:
	$(MAKE) -C $(bsg_BUILD_DIR) NEXUS_BIN_DIR=$(B_REFSW_BIN_DIR)

bsg_clean:
	$(MAKE) -C $(bsg_BUILD_DIR) NEXUS_BIN_DIR=$(B_REFSW_BIN_DIR) clean

else
bsg_HELP          = "Broadcom Scene Graph - 3D Framework $(bsg_NOT_LOADED)"

bsg:
	@echo $(bsg_NOT_LOADED)

bsg_clean:

endif
