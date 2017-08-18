#testbed configuration file for blr16end1
#Created on 22-7-2015
#Last checkin 
# Controller section:
# blr16end1: FC19


# SOFTAP section:

# AP1:4360c
# AP2:4360d
####### STA section:
# blr16tst1:43458 eth0(10.131.80.194)
# blr16tst4: 43458c0 eth0 (10.131.80.197) ---kept in DUT2
# blr03tst2:  eth0 (10.)
# blr03tst3: eth0 (10.)
# blr16ref0: 4360c (10.132.30.191)
# blr16ref1: 4360d (10.131.80.192)  
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
package require UTF::STB

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

###############################################
# Load Packages
# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::FTTR
    package require UTF::Test::ConfigBridge
    package require UTF::Power
}

UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr16end1.ban.broadcom.com" \
        -group {
                G1 {1 2 3 4}
                G2 {5 6 7 8}
                G3 { 9}
				G4 {1 2 3 4 5 6 7 8}
		ALL {1 2 3 4 5 6 7 8 9}
                }
ALL configure -default 25
G4 configure -default 25
G1 configure -default 25
G2 configure -default 25
#G2 configure default 0
#G3 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn default
	G4 attn default
    
}


# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr16"

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr16end1.ban.broadcom.com -rev 1

UTF::Linux blr16end1.ban.broadcom.com \
     -lan_ip 10.132.116.110 \
    -sta {lan p7p1}

UTF::Linux blr16ref1 \
	     -lan_ip 10.132.116.112 \
        -sta {4366ref1 enp1s0} \
		-power_button "auto" \
		-power {npc11 3} \
        -brand linux-internal-wl \
	-tag EAGLE_TWIG_10_10_69 \
	-reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc; wl country US/0; wl down; wl vht_features 7;} \
	-modopts {assert_type=1 nompc=1} \
    	-msgactions {
            {ai_core_reset: Failed to take core} {
            	$self worry $msg;
            	$self power cycle;
            	return 1
            }
    	}

4366ref1 configure -ipaddr 192.168.1.91 -attngrp G2 \

4366ref1 clone 4366bref1 \
	-post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3} 

4366ref1 clone 4366cref1 \
	 -wlinitcmds {wl msglevel +assoc; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3}

4366ref1 clone 4366ref11x1 \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 7; wl txchain 1; wl rxchain 1;} \

4366ref1 clone 4366ref11x1-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 1; wl rxchain 1; wl vhtmode 0;} \

4366ref1 clone 4366ref11x1-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 1; wl rxchain 1; wl vhtmode 0; wl nmode 0;} \

4366ref1 clone 4366ref12x2su \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3; wl txbf_bfe_cap 1;} \

4366ref1 clone 4366ref12x2su-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0; wl txbf_bfe_cap 1;} \

4366ref1 clone 4366ref12x2su-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0; wl nmode 0; wl txbf_bfe_cap 1;} \

4366ref1 clone 4366ref12x2 \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3;} \

4366ref1 clone 4366ref12x2-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0;} \

4366ref1 clone 4366ref12x2-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0; wl nmode 0;} \

4366ref1 clone 4366ref14x4-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 15; wl rxchain 15; wl vhtmode 0;} \

4366ref1 clone 4366ref14x4-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 15; wl rxchain 15; wl vhtmode 0; wl nmode 0;} \


UTF::Linux blr16ref0 \
    -lan_ip 10.132.116.111 \
        -sta {4366ref0 enp1s0} \
		-power_button "auto" \
        -brand linux-internal-wl \
		-power {npc11 2} \
	-tag EAGLE_TWIG_10_10_69 \
	-reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc; wl country US/0; wl down; wl vht_features 7;} \
	-modopts {assert_type=1 nompc=1} \
    	-msgactions {
            {ai_core_reset: Failed to take core} {
            	$self worry $msg;
            	$self power cycle;
            	return 1
            }
    	}

4366ref0 configure -ipaddr 192.168.1.92 -attngrp G1 \

4366ref0 clone 4366bref0 \
	-post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3} \

4366ref0 clone 4366ref01x1 \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 7; wl txchain 1; wl rxchain 1;} \
	
4366ref0 clone 4366cref0 \
	 -wlinitcmds {wl msglevel +assoc; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3}

4366ref0 clone 4366ref01x1-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 1; wl rxchain 1; wl vhtmode 0;} \

4366ref0 clone 4366ref01x1-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 1; wl rxchain 1; wl vhtmode 0; wl nmode 0;} \

4366ref0 clone 4366ref02x2 \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3;} \

4366ref0 clone 4366ref02x2-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0;} \

4366ref0 clone 4366ref02x2-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0; wl nmode 0;} \

4366ref0 clone 4366ref02x2su \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3; wl txbf_bfe_cap 1;} \

4366ref0 clone 4366ref02x2su-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0; wl txbf_bfe_cap 1;} \

4366ref0 clone 4366ref02x2su-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0; wl nmode 0; wl txbf_bfe_cap 1;} \

4366ref0 clone 4366ref04x4-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 15; wl rxchain 15; wl vhtmode 0;} \

4366ref0 clone 4366ref04x4-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 15; wl rxchain 15; wl vhtmode 0; wl nmode 0;} \

#temp######

UTF::Router 47189b \
        -power_button "auto" \
        -sta "47189bMC5 eth2 47189bMC5.%15 wl0.% 47189bMC2 eth3 47189bMC2.%15 wl1.%" \
        -lan_ip 192.168.1.1 \
        -relay blr16end1.ban.broadcom.com \
        -lanpeer lan -web 2 \
        -console "blr16end1.ban.broadcom.com:40003" \
        -power {npc11 5} \
        -brand linux-2.6.36-arm-up-internal-router-dhdap-moca \
        -tag "DINGO_TWIG_9_10_178" \
        -nvram {
	watchdog=2000
    wl0_country_code=US
    wl0_ssid=47189MC5
    wl0_chanspec=36
    wl0_radio=0
	wl1_country_code=US
	wl1_ssid=47189MC2
	wl1_chanspec=6
	wl1_radio=0
    wl1_vht_features=3
    wl0_vht_features=2
		} \
		-docpu 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.6g \
    -noradio_pwrsave 1 \
    -yart {-attn5g {20-83} -frameburst 1} \
    -coldboot 1 \
	-perfchans {1 1l} \
	-wlinitcmds {wl ver; wl -i eth2 band a} \
	-pre_perf_hook {{%S wl txbf} {%S wl tx_bfr_cap} {%S wl txchain} {%S wl rxchain}} \
	-post_perf_hook {{%S wl txchain} {%S wl rxchain}}


47189bMC5 configure -attngrp G1 \
	-channelsweep {-band a} \
	-yart {-attn5g {20-83} -frameburst 1} \
	-datarate {-i 0.5 -frameburst 1} -udp 1.2g

 
47189bMC2 configure -attngrp G1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -channelsweep {-band b} \
     -noradio_pwrsave 1 \
    -yart {-attn2g {20-83} -frameburst 1} \
    -wlinitcmds {sleep 2; wl ver}


####################temp############

UTF::Router 47189 \
        -power_button "auto" \
        -sta "47189MC5 eth2 47189MC5.%15 wl0.% 47189MC2 eth3 47189MC2.%15 wl1.%" \
        -lan_ip 192.168.1.1 \
        -relay blr16end1.ban.broadcom.com \
        -lanpeer lan -web 2 \
        -console "blr16end1.ban.broadcom.com:40003" \
        -power {npc11 5} \
        -brand linux-2.6.36-arm-up-internal-router-dhdap-moca \
        -tag "DINGO_TWIG_9_10_178" \
        -nvram {
	watchdog=2000
    wl0_country_code=US
    wl0_ssid=47189MC5
    wl0_chanspec=36
    wl0_radio=0
	wl1_country_code=US
	wl1_ssid=47189MC2
	wl1_chanspec=6
	wl1_radio=0
    wl1_vht_features=3
    wl0_vht_features=2
		} \
		-docpu 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.6g \
    -noradio_pwrsave 1 \
    -yart {-attn5g {20-83} -frameburst 1} \
    -coldboot 1 \
	-perfchans {36/80 36l 36} \
	-wlinitcmds {wl ver; wl -i eth2 band a} \
	-pre_perf_hook {{%S wl txbf} {%S wl tx_bfr_cap} {%S wl txchain} {%S wl rxchain}} \
	-post_perf_hook {{%S wl txchain} {%S wl rxchain}}

# dummy clone 47189MC5 -sta "47189MC5 eth2 47189MC5.%15 wl0.%" \
	# -channelsweep {-band a} -perfchans {36/80 36l 36}

# dummy clone 47189MC2 -sta "47189MC2 eth3 47189MC2.%15 wl1.%" \
	# -channelsweep {-band b} -perfchans {1 1l} \
	
47189MC5 configure -attngrp G1 \
	-channelsweep {-band a} \
	-yart {-attn5g {20-83} -frameburst 1} \
	-datarate {-i 0.5 -frameburst 1} -udp 1.2g

 
47189MC2 configure -attngrp G1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -noradio_pwrsave 1 \
    -yart {-attn2g {20-83} -frameburst 1} \
    -wlinitcmds {sleep 2; wl ver}

47189MC5 clone 47189MC5a \
        -perfchans {36}

47189MC2 clone 47189MC2a \
        -perfchans {1l}

47189MC5 clone 47189MC5_178_56 \
	-tag DINGO_REL_9_10_178_56 \
	-brand linux-2.6.36-arm-up-external-vista-router-dhdap-moca-full-src

47189MC5_178_56 clone 47189MC5_178_53 \
	-tag DINGO_REL_9_10_178_53 
	
# 47189MC5_MOCA configure  -attngrp G1 \

# 47189MC5_MOCA clone 47189_5G_2x2 \
	# -wlinitcmds {wl txbf 1; wl tx_bfr_cap 1;wl txchain 3; wl rxchain 3; wl ver} \

# 47189MC5_MOCA clone 47189_5G_1x1_txbf1 \
	# -wlinitcmds {wl txchain 1; wl rxchain 1; wl ver; wl txbf 1; wl tx_bfr_cap 1} \
	
# 47189MC5_MOCA clone 47189_5G_1x1_txbf0 \
	# -wlinitcmds {wl txchain 1; wl rxchain 1; wl ver } \


47189MC5 clone 47189atf \
        -nvram {
	watchdog=2000
    wl0_country_code=US
    wl0_ssid=47189MC5
	wl0_nband=2
    wl0_chanspec=36
    wl0_radio=1
	wl0_atf=1
	wl0_mu_features="-1"
	wl1_country_code=US
	wl1_ssid=47189MC2
	wl1_chanspec=1
	wl1_radio=1
	wl1_nband=1
	wl1_mu_features="-1"
	wl1_atf=1
    wl1_vht_features=3
    wl0_vht_features=3
		} \

UTF::Router 4709/43602 \
        -power_button "auto" \
        -sta "43602MC5 eth1 43602MC5.%15 wl0.% 43602MC2 eth2 43602MC2.%15 wl1.%" \
        -lan_ip 192.168.1.4 \
		-power {npc11 7} \
        -relay blr16end1.ban.broadcom.com \
        -lanpeer lan -web 2 \
        -console "blr16end1.ban.broadcom.com:40000" \
        -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
        -tag "BISON04T_BRANCH_7_14" \
        -nvram {
	watchdog=2000
    wl0_country_code=US
    wl0_ssid=43602MC5
    wl0_chanspec=36
    wl0_radio=0
	wl1_country_code=US
	wl1_ssid=43602MC2
	wl1_chanspec=1
	wl1_radio=0
	wl0_atf=0
	wl0_bw_cap=-1
	wl0_chanspec=36
	wl0_obss_coex=0
	wl0_radio=1
	wl0_reg_mode=off
	wl1_bw_cap=-1
	wl1_obss_coex=0
	wl1_vht_features=7
	wl0_vht_features=6
		} \
		-docpu 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.6g \
    -noradio_pwrsave 1 \
    -yart {-attn5g {20-83} -frameburst 1} \
    -coldboot 1 \
	-pre_perf_hook {{%S wl txbf} {%S wl tx_bfr_cap} {%S wl txchain} {%S wl rxchain}} \
	-wlinitcmds {sleep 5; wl ver; wl country US; wl txchain 7; wl rxchain 7} \
	-post_perf_hook {{%S wl txchain} {%S wl rxchain}}


43602MC5 configure -attngrp G4

 
43602MC2 configure -attngrp G4 

43602MC5 clone 43602MC52 \
	-perfchans {36/80 36l 36} \

43602MC2 clone 43602MC22 \
	-perfchans {1 1l} \

43602MC5 clone 43602MC5-atf \
        -nvram {
	watchdog=2000
    wl0_country_code=US
    wl0_ssid=43602MC5
    wl0_chanspec=36
    wl0_radio=1
	wl0_atf=1
	wl0_mu_features="-1"
	wl1_country_code=US
	wl1_ssid=43602MC2
	wl1_chanspec=3
	wl1_radio=1
	wl0_bw_cap=-1
	wl0_obss_coex=0
	wl0_reg_mode=off
	wl1_bw_cap=-1
	wl1_obss_coex=0
	wl1_vht_features=7
	wl0_vht_features=6
		} \

##4709/4366ap ######
##4709/4366########

UTF::Router 4709ap \
     -sta "4709/4366ap eth1 4709/4366ap.%15 wl0.%" \
     -lan_ip 192.168.1.3 \
     -relay "blr16end1.ban.broadcom.com" \
     -lanpeer lan -web 2 \
     -power "npc11 8" \
	 -tag BISON04T_REL_7_14_164 \
     -console "blr16end1.ban.broadcom.com:40001" \
     -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
     -trx linux \
	 -perfchans {3} \
	-embeddedimage {4366c} \
	-datarate {-i 0.5} -udp 1.8g \
	    -noradio_pwrsave 1 \
	-nvram { \
	watchdog=3000

        wl0_ssid=4709/4366
        wl0_chanspec=36
        wl0_radio=0
        wl0_bw_cap=-1
        wl0_country_code=US/0
        wl0_atf=0
        wl0_vht_features=7
        wl0_mu_features="-1"
        wl1_ssid=4709/4366g
        wl1_chanspec=3
        wl1_radio=0 
        wl1_atf=0
        wl1_vht_features=5
        wl1_mu_features="-1"
		devicemode=1
		} \
    -udp 1.8G \
    -pre_perf_hook {{%S wl assoc} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters}} \
	
4709/4366ap clone 4709rtr-mutx0
4709/4366ap clone 4709rtr-mutx1 -rvrnightly {-mumode mu}

4709/4366ap clone 4709rtr-atf-mutx0 \
    -tag BISON04T_BRANCH_7_14 \
    -nvram {
        watchdog=3000
        wl0_ssid=4709/4366
        wl0_chanspec=36
        wl0_radio=0
        wl0_bw_cap=-1
        wl0_country_code=US/0
        wl0_atf=1
        wl0_vht_features=7
	gmac3_enable=1
	fw_disable=1
	devicemode=1
    }
4709rtr-atf-mutx0 clone 4709rtr-atf-mutx1 -rvrnightly {-mumode mu}

###############
package require UTF::STBAP

UTF::STBAP 7252/4366 -sta {7252/4366a eth1 7252/4366a.%3 wl0.%} \
	-lan_ip 192.168.1.2 \
	-lanpeer lan \
	-power "npc11 6" \
    -console "blr16end1.ban.broadcom.com:40002" \
	-relay "blr16end1.ban.broadcom.com" \
    -power_button "auto" \
	    -nvram {
	wl0_mode=ap
	wl0_chanspec=36
	wl0_radio=0
	wl0_ssid=7252/4366a
	wl0_vht_features=7
	wl1_mode=ap
	wl1_chanspec=3
	wl1_radio=0
	wl1_ssid=7252/4366g
	wl1_vht_features=7
	lan1_stp=0
    } \
    -datarate {-i 0.5} \
	-udp 1.8G \
	-yart {-attn5g 8-95 -attn2g 8-95 -pad 46} \
    -noradio_pwrsave 1 \
    -perfchans {36/80 3} \
	-post_perf_hook {{%S wl channel} {%S wl chanspec}} \
    -pre_perf_hook {{%S wl channel} {%S wl chanspec}} \



UTF::STB 43602 \
	-sta {7252/43602 eth0} \
    -lan_ip 10.132.116.114 \
    -console "blr16end1.ban.broadcom.com:40001" \
    -power {npc11 3} \
    -reloadoncrash 1 \
	-tag BISON05T_REL_7_35_177_67 \
	-brand linux-external-media \
    -dhd_brand linux-external-media \
    -dhd_tag "DHD_REL_1_363_110_26" \
	-app_tag "DHD_REL_1_363_110_26" \
    -type 43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-slvradar.bin \
    -dongleimage "wl.ko" \
    -datarate {-i 0.5} \
    -yart {-attn5g 8-95 -attn2g 8-95 -pad 46} \
    -tcpwindow 4m -udp 1.7g -slowassoc 5 \
    -perfchans {36/80 3} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 7}

7252/43602 configure -ipaddr 192.168.1.103

set UTF::RouterNightlyCustom {

   package require UTF::Test::MiniUplinks
   UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 

    package require UTF::Test::Repeaters
    UTF::Test::Repeaters $Router $STA1 -sta $STA2

}

UTF::Q blr16
