# URL=http://edge.cinemanow.com.edgesuite.net/adaptive/studio/shutter_island_907dafa7_wv_SD.vob
export URL=http://edge.cinemanow.com.edgesuite.net/adaptive/studio/get_smart_c68b8145_wv_SD.vob
# URL=http://edge.cinemanow.com.edgesuite.net/adaptive/studio/state_of_play_77c60696_wv_SD.vob
# URL=http://edge.cinemanow.com.edgesuite.net/adaptive/studio/the_international_2009_fb588b1d_wv_SD.vob

#export URL=http://edge.cinemanow.com.edgesuite.net/adaptive/studio/ice_harvest_wv_SD_TRAILER.vob

export WV_DRM=http://wstfcps005.shibboleth.tv/widevine/cypherpc/cgi-bin

#export  msg_modules=BHSM,crypto_common,drm_widevine

#export LD_LIBRARY_PATH=.:/nexus.bin:/refsw
./nexus ./TestWVPlayback -url $URL drm_server_url=$WV_DRM/GetEMMs.cgi drm_ack_server_url=$WV_DRM/Ack.cgi drm_heartbeat_url=$WV_DRM/Heartbeat.cgi portal=OEM drm_key_bin=./drm.bin
#export LD_LIBRARY_PATH= /nexus.bin : /refsw : /nexus
#./TestWVPlayback -url $URL drm_server_url=$WV_DRM/GetEMMs.cgi drm_ack_server_url=$WV_DRM/Ack.cgi drm_heartbeat_url=$WV_DRM/Heartbeat.cgi
