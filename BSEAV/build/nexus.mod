############################################################
#
# Nexus
#
############################################################

MAKE_MODULES += nexus_utils nexus_examples

nexus_utils_HELP = "Nexus Utilities"
nexus_examples_HELP = "Nexus Example Applications"

nexus_utils:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/nexus/utils NEXUS_BIN_DIR=$(B_REFSW_BIN_DIR)

nexus_utils_clean:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/nexus/utils clean

nexus_examples:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/nexus/examples NEXUS_BIN_DIR=$(B_REFSW_BIN_DIR)

nexus_examples_clean:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/nexus/examples clean
