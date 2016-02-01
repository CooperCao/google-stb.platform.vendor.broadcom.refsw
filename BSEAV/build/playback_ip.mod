############################################################
#
# Playback_IP library
#
############################################################

MAKE_MODULES += playback_ip
playback_ip_HELP = "IP playback library and apps"

playback_ip:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/nexus/lib/playback_ip NEXUS_BIN_DIR=$(B_REFSW_BIN_DIR)

playback_ip_clean:
	$(MAKE) -C $(B_REFSW_ROOT_DIR)/nexus/lib/playback_ip clean
