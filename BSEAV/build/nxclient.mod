############################################################
#
# NxClient
#
############################################################

MAKE_MODULES += nxclient
nxclient_NOT_LOADED := $(shell test -d $(B_REFSW_ROOT_DIR)/nexus/nxclient || echo "WARNING: nexus/nxclient not loaded")

ifeq ($(nxclient_NOT_LOADED),)
nxclient_HELP = "NxClient server and client"
nxclient:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/nexus/nxclient NEXUS_BIN_DIR=$(B_REFSW_BIN_DIR)

nxclient_clean:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/nexus/nxclient clean
else
nxclient_HELP = "NxClient server and client $(nxclient_NOT_LOADED)"
nxclient:
	@echo $(nxclient_NOT_LOADED)
nxclient_clean:
endif
