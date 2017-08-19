	#Testbed configuration file for blr18end1 UTF  Teststation
#Created by Anuj Gupta on 30 March 2015 05:00 PM  
#Last checkin 19Nov2014 
####### Controller section:
# blr18end1: FC15
# IP ADDR 10.132.116.101
# NETMASK 255.255.254.0 
# GATEWAY 10.132.116.1
#
####### SOFTAP section:
#
#  
# 
#
####### STA section:
#
# blr18tst1: 
# blr18tst2: 
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

set UTF::ChannelPerf 1
set UTF::Use11 1
set UTF::Use11h 1
set UTF::UseCSA 1
set UTF::Web 2

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr18end1.ban.broadcom.com" \
        -group {
                 G1 {1 2 3 4}
		ALL {1 2 3 4}
                }
 G1 configure -default 18
#G2 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value

	foreach S {4366ref0 4366ref1} {
	catch {$S wl down}
	$S deinit
	G1 attn default
    }
    return
}


# SummaryDir sets the location for test results
set k $::env(LOGNAME)
regexp {/home/(.*)/} $k m k
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr18"

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \


# Turn off most RvR initialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up} {%S wl roam_trigger -100 all}} 
set ::rvr_ap_init "" 

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr18end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr18end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr18end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.1.1.42 -relay blr18end1 -rev 1
UTF::Power::WebRelay  web43 -lan_ip 172.1.1.43 -invert 1
########################### Test Manager ################

UTF::Linux blr18end1.ban.broadcom.com \
     -lan_ip 10.132.116.126 \
     -sta {lan p5p1}

# Power          - npc21 port 1    172.1.1.21
################################################################################

UTF::Router 4709ap \
	 -sta {dummy dummy} \
     -lan_ip 192.168.1.2 \
     -relay "blr18end1.ban.broadcom.com" \
     -lanpeer lan -web 2 \
     -power "npc11 2" \
	 -tag EAGLE_BRANCH_10_10 \
     -console "blr18end1.ban.broadcom.com:40000" \
     -brand linux-2.6.36-arm-internal-router \
     -trx linux \
	 -yart {-attn5g {20-83} -frameburst 1} \
	-embeddedimage {4366c} \
	-datarate {-i 0.5} -udp 1.8g \
	    -noradio_pwrsave 1 \
	-nvram { \
	watchdog=3000
        wl1_ssid=4709/4366ap
        wl1_chanspec=36
        wl1_radio=0
        wl1_bw_cap=-1
        wl1_country_code=Q1
	wl1_country_rev=137
        wl1_atf=0
        wl1_vht_features=7
        wl1_mu_features="-1"
        wl0_ssid=4709/4360g
        wl0_chanspec=6
        wl0_radio=0
        wl0_atf=0
        wl0_vht_features=5
		} \
    -udp 1.8G \
    -pre_perf_hook {{%S wl assoc} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters}}

dummy clone 4709/4366ap -sta {4709/4366ap eth2 4709/4366ap.%15 wl1.%} \
    -channelsweep {-min 100 -pretest 2 -bw {20 40 80}} -perfchans {36/80}
dummy clone 4709/4360g -sta {4709/4360g eth1 4709/4360g.%15 wl0.%} \
    -channelsweep {-band b} -perfchans {6}

4709/4366ap configure -attngrp L1 \
        -channelsweep {-band a} \
		-perfchans {36/80} \
        -yart {-attn5g {20-83} -frameburst 1} \
        -datarate {-i 0.5 -frameburst 1} -udp 2.2g

4709/4360g configure -attngrp L1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8g \
    -noradio_pwrsave 1 \
	-channelsweep {-band b} \
	-perfchans {6} \
    -yart {-attn2g {20-83} -frameburst 1} \
    -wlinitcmds {sleep 2; wl ver}

4709/4366ap clone 4709rtr-mutx0
4709/4366ap clone 4709rtr-mutx1 -rvrnightly {-mumode mu}

4709/4366ap clone 4709/4366ap-trunk \
	-tag trunk

4709/4360g clone 4709/4360g-trunk \
	-tag trunk

4709/4366ap clone 4709/4366ap-122 \
	-tag EAGLE_TWIG_10_10_122

4709/4360g clone 4709/4360g-122 \
	-tag EAGLE_TWIG_10_10_122


package require UTF::DSL
UTF::DSL 4908 -sta {dummy dummy} \
    -model 4908 \
    -jenkins "" \
    -tag BISON04T_BRANCH_7_14 \
    -p4tag REL502L02 \
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_int \
    -type bcm94908HND_nand_fs_image_128_ubi.w \
    -lan_ip 192.168.1.1 \
    -lanpeer {lan} \
    -console "blr18end1.ban.broadcom.com:40001" \
    -relay lan \
    -power {npc21 1} \
    -nvram {
	wl1_ssid=4908-5G
	wl0_ssid=4908-2G
	wl1_chanspec=36
	wl0_chanspec=6
	wl1_radio=0
	wl0_radio=0
	wl1_vht_features=6
	wl0_vht_features=5
	wl1_frameburst=on
	wl0_frameburst=on
	wan_ifname=
	wan_ifnames=
	{lan_ifnames=eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7}
	wl1_country_code=ALL
	wl0_country_code=US
	wl1_dyn160=1
    } \
    -datarate {-i 0.5} -udp 1.8g \
    -yart {-attn5g {0-90} -attn2g {48-90} -pad 29} \
    -noradio_pwrsave 1 \
    -wlinitcmds {
	echo 2 > /proc/irq/8/smp_affinity;
	echo 4 > /proc/irq/9/smp_affinity;
	echo 8 > /proc/irq/10/smp_affinity;
    }


dummy clone 4908-5G -sta {4908-5G eth6 4908-5G.%8 wl1.%} \
    -channelsweep {-min 100 -pretest 2 -bw {20 40 80}} -perfchans {36/80}
dummy clone 4908-2G -sta {4908-2G eth5 4908-2G.%8 wl0.%} \
    -channelsweep {-band b} -perfchans {6}
# dummy clone 4908/2 -sta {4908/2 eth7 4908/2.%8 wl2.%} \
    # -channelsweep {-band b} -perfchans {3}

dummy destroy

set external {
    -fwoverlay 1
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
    -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc-dyn160
}
set external {
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_external_full
    -type bcm94908HND_nand_cferom_fs_image_128_ubi*.w
}

4908-5G clone 4908x/0 {*}$external -perfonly 1
4908-2G clone 4908x/1 {*}$external -perfonly 1

set internal {
    -fwoverlay 1
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas
    -type 4366c0-roml/config_pcie_internal
}

4908-5G clone 4908i/0 {*}$internal
4908-2G clone 4908i/1 {*}$internal

4908-5G clone 4908-5G-164 -tag BISON04T_TWIG_7_14_164
4908-2G clone 4908-2G-164 -tag BISON04T_TWIG_7_14_164


4908x/0 configure \
    -dualband {4908x/1 -c2 36/80 -c1 161/80 -lan1 {lan lan1} -lan2 {lan2 lan3}}

##

4908-5G clone 4908b131/0 -tag BISON04T_TWIG_7_14_131
4908-2G clone 4908b131/1 -tag BISON04T_TWIG_7_14_131


4908x/0 clone 4908b164x/0 -tag BISON04T_TWIG_7_14_164
4908x/1 clone 4908b164x/1 -tag BISON04T_TWIG_7_14_164



#################################################################################





# UTF::Router 4908b \
        # -power_button "auto" \
        # -sta "4366MC5 eth5 4366MC5.%15 wl0.% 43602MC2 eth6 43602MC2.%15 wl1.%" \
        # -lan_ip 192.168.1.1 \
		# -power {npc21 2} \
        # -relay blr18end1.ban.broadcom.com \
        # -lanpeer lan -web 2 \
        # -console "blr18end1.ban.broadcom.com:40000" \
        # -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
        # -tag "BISON04T_TWIG_7_14_164" \
        # -nvram {
	# watchdog=2000
    # wl0_country_code=US
    # wl0_ssid=4366MC5
    # wl0_chanspec=36
    # wl0_radio=0
	
	# wl1_country_code=US
	# wl1_ssid=43602MC2
	# wl1_chanspec=1
	# wl1_radio=0
	# wl0_atf=0
	# wl0_bw_cap=-1
	# wl0_chanspec=36
	# wl0_obss_coex=0
	# wl0_radio=1
	# wl0_reg_mode=off
	# wl1_bw_cap=-1
	# wl1_obss_coex=0
	# wl1_vht_features=7
	# wl0_vht_features=6
		# } \
		# -docpu 1 \
    # -datarate {-i 0.5 -frameburst 1} -udp 1.6g \
    # -noradio_pwrsave 1 \
    # -yart {-attn5g {20-83} -frameburst 1} \
    # -coldboot 1 \
	# -pre_perf_hook {{%S wl txbf} {%S wl tx_bfr_cap} {%S wl txchain} {%S wl rxchain}} \
	# -wlinitcmds {sleep 5; wl ver; wl country US; wl txchain 7; wl rxchain 7} \
	# -post_perf_hook {{%S wl txchain} {%S wl rxchain}}


# 4366MC5 configure -attngrp G3

 
# 43602MC2 configure -attngrp G3 

# 4366MC5 clone 4366MC52 \
	# -perfchans {36/80 36l 36} \

# 43602MC2 clone 43602MC22 \
	# -perfchans {1 1l} \

# 4366MC5 clone 4366-atf \
        # -nvram {
	# watchdog=2000
    # wl0_country_code=US
    # wl0_ssid=4366MC5
    # wl0_chanspec=36
    # wl0_radio=1
	# wl0_atf=1
	# wl0_mu_features="-1"
	# wl1_country_code=US
	# wl1_ssid=43602MC2
	# wl1_chanspec=3
	# wl1_radio=1
	# wl0_bw_cap=-1
	# wl0_obss_coex=0
	# wl0_reg_mode=off
	# wl1_bw_cap=-1
	# wl1_obss_coex=0
	# wl1_vht_features=7
	# wl0_vht_features=6
		# } \

###########################################
# blr18TST1
# 4360mc_P199 - 11n 3x3
###########################################

UTF::Linux blr18ref1 \
	-lan_ip 10.132.116.128 \
	-sta {4366ref1 enp1s0} \
    -power {npc31 1} \
		-power_button "auto" \
        -brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
	-reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl vht_features 7;} \
		-post_perf_hook {{%S wl phy_rssi_ant}} \
	-modopts {assert_type=1 nompc=1} \
    	-msgactions {
            {ai_core_reset: Failed to take core} {
            	$self worry $msg;
            	$self power cycle;
            	return 1
            }
    	}

4366ref1 configure -ipaddr 192.168.1.91 -attngrp G1 \

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


UTF::Linux blr18ref0 \
    -lan_ip 10.132.116.127 \
        -sta {4366ref0 enp1s0} \
        -power {npc31 2} \
		-power_button "auto" \
        -brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
	-reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl vht_features 7;} \
	-modopts {assert_type=1 nompc=1} \
	-post_perf_hook {{%S wl phy_rssi_ant}} \
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



set UTF::RouterNightlyCustom {

   package require UTF::Test::MiniUplinks
   UTF::Test::MiniUplinks $Router $STA1 -sta $STA2

    package require UTF::Test::Repeaters
    UTF::Test::Repeaters $Router $STA1 -sta $STA2

	# UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 -nowet
	# UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 -nowet
	# UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 -nowet
	# UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 -nowet
	# UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 -nowet

}


UTF::Q blr18

