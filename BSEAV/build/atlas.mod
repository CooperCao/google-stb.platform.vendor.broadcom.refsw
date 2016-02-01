############################################################
#
# Atlas
#
############################################################

MAKE_MODULES += atlas
atlas_HELP = "Atlas application"

atlas:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/BSEAV/app/atlas/build BINDIR=$(B_REFSW_BIN_DIR)

atlas_clean:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/BSEAV/app/atlas/build clean
