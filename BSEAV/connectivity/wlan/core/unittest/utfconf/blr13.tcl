#############################BLR13###############################
## CONTROLLER blr13end1 10.132.116.100
##
##
# Router1  4331/4360  R6300                         NPC port 4     
# Router2  4331/4360  R6300                         NPC port 5 
# Router3  4331/4360  R6300                         NPC port 8  
# Router4  4331/4360  R6300                         NPC port 6  
# Attenuator Aefoflex       172.1.1.100
# Attenuator Aefoflex       172.1.1.91
# Power   NPC 4 port        172.1.1.11
# Power   NPC 4 port        172.1.1.21
# Power   NPC 8 port        172.1.1.31
# Power   NPC 8 port        172.1.1.41
################################################################
#
#
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
package require UTF::WebRelay
package require UTF::Streams
package require UTF::Router

################## Aeroflex attenuator 1 ###################
# UTF::Aeroflex af -lan_ip 172.1.1.100 \
        # -relay "blr13end1.ban.broadcom.com" \
        # -group {
                # L1 {1 2 3}
                # L2 {4 5 6}
                # L3 {7 8 9}
                # ALL {1 2 3 4 5 6 7 8 9}
                # }
# L1 configure -default 0 
# L2 configure -default 0
# L3 configure -default 0 

# set ::UTF::SetupTestBed {
    
    #Make Sure Attenuators are set to 0 value

    # L1 attn 0
    # catch {L3 attn 0;}
    # catch {L2 attn 0;}
    # catch {L1 attn 0;}
	# }

#################### Aeroflex attenuator 2 ########################

UTF::Aeroflex af1 -lan_ip 172.1.1.91 \
        -relay "blr13end1.ban.broadcom.com" \
        -group {
                L1 {1 2 3}
                L2 {4 5 6}
                L3 {7 8 9}
                ALL {1 2 3}
                }
L1 configure -default 30 
L2 configure -default 0
L3 configure -default 0 

#Default TestBed Configuration Options

set ::UTF::SetupTestBed {
    
    # Make Sure Attenuators are set to 0 value
    
    L1 attn 0
    catch {L3 attn 0;}
    catch {L2 attn 0;}
    catch {L1 attn 0;}

	
 }
set UTF::SetupTestBed {
    L1 attn default
	L2 attn default
	L3 attn default
    foreach S {SOI AP1 AP2  AP4 AP5 AP6 AP7} {
	catch {$S wl down;UTF::set_ap_nvram $S wl0_radio=0 wl1_radio=0}
	$S deinit
    }
    return
}

############################################################
# SummaryDir sets the location for test results

set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr13"


####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.11 -relay blr13end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr13end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr13end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.41 -relay blr13end1.ban.broadcom.com -rev 1

########################### Test Manager #############################

UTF::Linux blr13end1.ban.broadcom.com \
     -lan_ip 10.132.116.100 \
     -sta {lan p16p2}
	 
########################## SOI ########################################
UTF::Router SOI -name SOI-4331 \
    -sta "SOI_2G eth1 SOI_5G eth2" \
    -lan_ip 192.168.1.2 \
    -relay "blr13end1.ban.broadcom.com" \
    -lanpeer lan \
    -console "blr13end1.ban.broadcom.com:40002" \
    -power "npc31 4" \
    -brand linux-2.6.36-arm-internal-router \
	-tag AARDVARK01T_REL_6_37_14_44 \
    -trx linux \
    -nvram {
    wl_msglevel=0x101
	fw_disable=1
    lan_ipaddr=192.168.1.2
	watchdog=6000; # PR#90439
	wl0_channel=4
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=SOI_2G
	wl1_ssid=SOI_5G
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
	boardtype=0x0617; # 4706nr2hmc
	wl_country_code=US
    wl_country_rev=0
    wl0_country_code=US
    wl0_country_rev=0
       
    }
SOI_2G configure -attngrp L1 
SOI_5G configure -attngrp L1
########################## AP1 ########################################
UTF::Router AP1 -name AP1-4331 \
    -sta "AP1_2G eth1 AP1_5G eth2" \
    -lan_ip 192.168.1.3 \
	-console "blr13end1.ban.broadcom.com:40003" \
    -relay "blr13end1.ban.broadcom.com" \
    -lanpeer lan \
    -power "npc31 5" \
    -brand linux-2.6.36-arm-internal-router \
	-tag AARDVARK01T_REL_6_37_14_44 \
    -trx linux \
    -nvram {
        wl_msglevel=0x101
        fw_disable=1
        watchdog=6000
        wl0_channel=1
        wl0_radio=1
        wl0_obss_coex=0
        wl0_ssid=AP1_2G
        wl1_ssid=AP1_5G
        wl1_channel=36
        wl1_radio=1
        wl1_obss_coex=0
        router_disable=0
        lan_ipaddr=192.168.1.3
        wl_reg_mode=h
        wl_wmf_ucigmp_query=1
        wl_wmf_ucast_upnp=1
        wl_wmf_acs_fcs_mode=1
        wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
        wl_pspretend_retry_limit=5
        boardtype=0x0617
       wl_country_code=US
        wl_country_rev=45
        wl0_country_code=US
        wl0_country_rev=45
    }
AP1_2G configure -attngrp L2
AP1_5G configure -attngrp L2
########################## AP2 ########################################
UTF::Router AP2\
    -sta "AP2_2G eth1 AP2_5G eth2" \
	-lan_ip 192.168.1.4 \
	-lanpeer lan \
    -relay "blr13end1.ban.broadcom.com" \
    -console "blr13end1.ban.broadcom.com:40000" \
    -power "npc31 8" \
	-brand linux-2.6.36-arm-internal-router \
	-tag AARDVARK01T_REL_6_37_14_44 \
    -trx linux \
	-nvram {
        wl_msglevel=0x101
        fw_disable=1
        watchdog=6000
        wl0_channel=1
        wl0_radio=1
        wl0_obss_coex=0
        wl0_ssid=AP2_2G
        wl1_ssid=AP2_5G
        wl1_channel=36
        wl1_radio=1
        wl1_obss_coex=0
        router_disable=0
        lan_ipaddr=192.168.1.4
        wl_reg_mode=h
        wl_wmf_ucigmp_query=1
        wl_wmf_ucast_upnp=1
        wl_wmf_acs_fcs_mode=1
        wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
        wl_pspretend_retry_limit=5
        boardtype=0x0617
        wl_country_code=US
        wl_country_rev=45
        wl0_country_code=US
        wl0_country_rev=45
    }
AP2_2G configure -attngrp L2 -ap 1
AP2_5G configure -attngrp L2

######################### AP3 ########################################
UTF::Router AP3 \
    -sta "AP3_2G eth1 AP3_5G eth2" \
    -lan_ip 192.168.1.5 \
    -relay "blr13end1.ban.broadcom.com" \
	-lanpeer lan \
    -console "blr13end1.ban.broadcom.com:40001" \
    -power "npc41 7" \
	-brand linux-2.6.36-arm-internal-router \
	-tag AARDVARK01T_REL_6_37_14_44 \
    -trx linux \
	-nvram {
        wl_msglevel=0x101
        fw_disable=1
        watchdog=6000
        wl0_channel=6
        wl0_radio=1
        wl0_obss_coex=0
        wl0_ssid=AP3_2G
        wl1_ssid=AP3_5G
        wl1_channel=36
        wl1_radio=1
        wl1_obss_coex=0
        router_disable=0
        lan_ipaddr=192.168.1.5
        wl_reg_mode=h
        wl_wmf_ucigmp_query=1
        wl_wmf_ucast_upnp=1
        wl_wmf_acs_fcs_mode=1
        wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
        wl_pspretend_retry_limit=5
        boardtype=0x0617
        wl_country_code=US
        wl_country_rev=0
        wl0_country_code=US
        wl0_country_rev=0
    }
AP3_2G configure -attngrp L2
AP3_5G configure -attngrp L2
######################### AP4 ########################################
UTF::Router AP4 \
    -sta "AP4_2G eth1 AP4_5G eth2" \
    -lan_ip 192.168.1.6 \
    -relay "blr13end1.ban.broadcom.com" \
	-lanpeer lan \
    -console "blr13end1.ban.broadcom.com:40004" \
    -power "npc31 6" \
	-brand linux-2.6.36-arm-internal-router \
	-tag AARDVARK01T_REL_6_37_14_44 \
    -trx linux \
	-nvram {
        wl_msglevel=0x101
        fw_disable=1
        watchdog=6000
        wl0_channel=6
        wl0_radio=1
        wl0_obss_coex=0
        wl0_ssid=AP3_2G
        wl1_ssid=AP3_5G
        wl1_channel=36
        wl1_radio=1
        wl1_obss_coex=0
        router_disable=0
        lan_ipaddr=192.168.1.6
        wl_reg_mode=h
        wl_wmf_ucigmp_query=1
        wl_wmf_ucast_upnp=1
        wl_wmf_acs_fcs_mode=1
        wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
        wl_pspretend_retry_limit=5
        boardtype=0x0617
        wl_country_code=US
        wl_country_rev=0
        wl0_country_code=US
        wl0_country_rev=0
    }
AP4_2G configure -attngrp L2
AP4_5G configure -attngrp L2
###################### AP5 #######################################################
UTF::Router AP5 \
    -sta "AP5_2G eth1 AP5_5G eth2" \
    -lan_ip 192.168.1.7 \
    -relay "blr13end1.ban.broadcom.com" \
	-lanpeer lan \
    -console "blr13end1.ban.broadcom.com:40005" \
    -power "npc31 5" \
	-brand linux-2.6.36-arm-internal-router \
	-tag AARDVARK01T_REL_6_37_14_44 \
    -trx linux \
	-nvram {
        wl_msglevel=0x101
        fw_disable=1
        watchdog=6000
        wl0_channel=6
        wl0_radio=1
        wl0_obss_coex=0
        wl0_ssid=AP3_2G
        wl1_ssid=AP3_5G
        wl1_channel=36
        wl1_radio=1
        wl1_obss_coex=0
        router_disable=0
        lan_ipaddr=192.168.1.7
        wl_reg_mode=h
        wl_wmf_ucigmp_query=1
        wl_wmf_ucast_upnp=1
        wl_wmf_acs_fcs_mode=1
        wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
        wl_pspretend_retry_limit=5
        boardtype=0x0617
        wl_country_code=US
        wl_country_rev=0
        wl0_country_code=US
        wl0_country_rev=0
    }
AP5_2G configure -attngrp L3
AP5_5G configure -attngrp L3
############################# AP6 ##############################
UTF::Router AP6 \
    -sta "AP6_2G eth1 AP6_5G eth2" \
    -lan_ip 192.168.1.8 \
    -relay "blr13end1.ban.broadcom.com" \
	-lanpeer lan \
    -console "blr13end1.ban.broadcom.com:40006" \
    -power "npc31 7" \
	-brand linux-2.6.36-arm-internal-router \
	-tag AARDVARK01T_REL_6_37_14_44 \
    -trx linux \
	-nvram {
        wl_msglevel=0x101
        fw_disable=1
        watchdog=6000
        wl0_channel=6
        wl0_radio=1
        wl0_obss_coex=0
        wl0_ssid=AP3_2G
        wl1_ssid=AP3_5G
        wl1_channel=36
        wl1_radio=1
        wl1_obss_coex=0
        router_disable=0
        lan_ipaddr=192.168.1.8
        wl_reg_mode=h
        wl_wmf_ucigmp_query=1
        wl_wmf_ucast_upnp=1
        wl_wmf_acs_fcs_mode=1
        wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
        wl_pspretend_retry_limit=5
        boardtype=0x0617
        wl_country_code=US
        wl_country_rev=0
        wl0_country_code=US
        wl0_country_rev=0
    }
AP6_2G configure -attngrp L3
AP6_5G configure -attngrp L3
######################### AP7 ################################################
UTF::Router AP7 \
    -sta "AP7_2G eth1 AP7_5G eth2" \
    -lan_ip 192.168.1.9 \
    -relay "blr13end1.ban.broadcom.com" \
	-lanpeer lan \
    -console "blr13end1.ban.broadcom.com:40007" \
    -power "npc31 1"\
	-brand linux-2.6.36-arm-internal-router \
	-tag AARDVARK01T_REL_6_37_14_44 \
    -trx linux \
	-nvram {
        wl_msglevel=0x101
        fw_disable=1
        watchdog=6000
        wl0_channel=6
        wl0_radio=1
        wl0_obss_coex=0
        wl0_ssid=AP3_2G
        wl1_ssid=AP3_5G
        wl1_channel=36
        wl1_radio=1
        wl1_obss_coex=0
        router_disable=0
        lan_ipaddr=192.168.1.9
        wl_reg_mode=h
        wl_wmf_ucigmp_query=1
        wl_wmf_ucast_upnp=1
        wl_wmf_acs_fcs_mode=1
        wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
        wl_pspretend_retry_limit=5
        boardtype=0x0617
        wl_country_code=US
        wl_country_rev=0
        wl0_country_code=US
        wl0_country_rev=0
    }
AP7_2G configure -attngrp L3
AP7_5G configure -attngrp L3

####################### TEST1-4360 ###############################################

UTF::Linux blr13tst1 -lan_ip 10.132.116.101  \
		-sta {43909b0 eth0} \
        -power "npc11 1"\
        -power_button "auto" \
        -app_tag BIS120RC4_TWIG_7_15_168 \
        -dhd_tag DHD_REL_1_201_12_5  \
		-dhd_brand linux-external-dongle-sdio \
		-tag BIS120RC4_TWIG_7_15_168 \
        -brand linux-external-dongle-sdio \
		-modopts {sd_uhsimode=2} \
        -nvram bcm943909wcd1_p308.txt \
		-type 43909b0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth-dfsradar-mfp-swdiv.bin \
		-slowassoc 5 \
		-datarate {-i 0.5 -frameburst 1} \
		-perfchans {36/80 36l 36 3l 3} \
        -tcpwindow 2m  \
		-udp 800m \
	    -yart {-attn5g 05-95 -attn2g 30-95} \
        -nocal 1 \
        -wlinitcmds { wl down; wl vht_features 3; wl country US; wl txchain 1; wl rxchain 1} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal}} \

43909b0 configure -ipaddr 192.168.1.96  \


43909b0 clone 43909b -tag BIS120RC4_TWIG_7_15_168 \
                   -type 43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin  \
	
43909b0 clone 43909bb -tag BIS120RC4_REL_7_15_168_10 \
                   -type  43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \


43909b0 clone 43909bb0 -tag BIS120RC4_REL_7_15_168_10 \
                   -type 43909b0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
				   -perfchans {36/80,1}

43909b0 clone 43909b0_mfgc -tag BIS120RC4_REL_7_15_168_44 \
					 -brand linux-mfgtest-dongle-sdio \
	                 -type 43909b0-roml/sdio-ag-mfgtest-seqcmds-sr-dbgsr.bin \

       
43909b0 clone 43909b0_prev 	-tag BIS120RC4_TWIG_7_15_168 \
	-type 43909b0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
        # -sta {43217 enp1s0} \
        # -power "npc12 2"\
        # -power_button "auto" \
        # -tcpwindow 4M \
        # -slowassoc 5 -reloadoncrash 1 \
        # -nobighammer 1 \
        # -brand linux-internal-wl \
        # -tag "BISON_BRANCH_7_10" \
		# -wlinitcmds {wl phymsglevel 0x800} \
        # -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        # -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

# 43217 configure -ipaddr 192.168.1.91 \

# 43217 clone 43217t -tag trunk \

# 43217 clone 43217b -tag BISON05T_BRANCH_7_35 \

# 43217 clone 43217BIS -tag  BISON04T_BRANCH_7_14 \
	# -brand linux-mfgtest-wl \


# 43217 clone 43217a  -tag BISON04T_TWIG_7_14_89 \

# 43217 clone 43217a87   -tag AARDVARK01T_REL_6_37_14_87 \



######################################################################################################################################
# UTF::Sniffer sniffer \
         # -lan_ip 10.131.80.57 \
         # -sta {snif eth1} \
         # -power_button "auto" \
		 # -tag "BISON_BRANCH_7_10" \
		 # -brand "linux-internal-wl" \

