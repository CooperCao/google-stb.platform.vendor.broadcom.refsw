#########################################################################
# UTF configuration for BLR17 (IPTV) testbed Location Campus 1A DVT Lab 
# Created  by Sumesh Nair
#
############### Station Generic Information ################# 
#
# Controller  blr17end1.ban.broadcom  10.131.80.163
#
# Router  4709/4366b0    Ramseylx1  STE2900        NPC8 port 3     
# blr17-iptv-lx1.ban.broadcom.com  10.131.80.164   NPC8 port 1
# blr17-iptv-lx5.ban.broadcom.com  10.131.80.173   2nd host connected to Ap
# 
# Router  4709/4366b0    Ramseylx2   STE2900       NPC8 port 4     
# blr17-iptv-lx2.ban.broadcom.com  10.131.80.165   NPC8 port 2
# blr17-iptv-lx6.ban.broadcom.com                  2nd host connected to AP
# 
# Router  4709/43602     Ramseylx3  STE2900        NPC8 port 7   
# blr17-iptv-lx3.ban.broadcom.com  10.131.80.166   NPC8 port 5
#
# PSTA    4709/4366b0    Ramseylx4  STE2900        NPC8 port 8   TF
# blr17-iptv-lx4.ban.broadcom.com  10.131.80.167   NPC8 port 6
# blr17-iptv-lx7.ban.broadcom.com  10.131.80.175   2nd host connected to AP
#
# STA    4366/FC19       Ramseylx5  STE2900        NPC2 port 1
# blr17tst1.ban.broadcom.com       10.131.80.168    
#
# STA    43602/FC19      Ramseylx6  STE2900        NPC2 port 2
# blr17tst2.ban.broadcom.com       10.131.80.169  
# 
# Attenuator Aefoflex AF1   172.1.1.100
# Attenuator Aeroflex AF2   172.1.1.101
# Power   NPC8 port         172.1.1.11
# Power   NPC2 port         172.1.1.21 
################################################################

# To enable the log to display milliseconds on timestamp
# set ::UTF::MSTimeStamps 1

# To setup UTFD port for this rig
#set ::env(UTFDPORT) 9988

# To use wl from trunk (set default); Use -app_tag to modify.
set UTF::TrunkApps 1

# To disable/enable automatic restore of UTFD previous state
#set ::UTFD::norestore 0

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

set ::UTF::SummaryDir "/projects/hnd_sig_ext21/gopikrir/blr17"
# set ::env(UTFDPORT) 9977
package require UTF::Power
package require UTF::Aeroflex
package require UTF::WebRelay
# package require UTFD
package require UTF::Streams

# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::FTTR
    package require UTF::Test::ConfigBridge
}

######################## Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc8 -lan_ip 172.1.1.11 -relay blr17end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc2 -lan_ip 172.1.1.21 -relay blr17end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc3 -lan_ip 172.1.1.31 -relay blr17end1.ban.broadcom.com -rev 1

####################### Attenuator  ######################

UTF::Aeroflex af1  -lan_ip 172.1.1.100 \
        -relay "blr17end1.ban.broadcom.com" \
        -group {
                L6 {1  2  3  4 }
                L1 {5  6  7  8 }
                L4 {9 10 11 12}
                ALL {1 2 3 4 5 6 7 8 9 10 11 12}
                }

UTF::Aeroflex af2  -lan_ip 172.1.1.101 \
        -relay "blr17end1.ban.broadcom.com" \
        -group {
                L2 {1 2 3 4}
                L3 {5 6 7 8}
                L5 {9 10 11 12}
                ALL {1 2 3 4 5 6 7 8 9 10 11 12}
                }


L5 configure -default 0
L3 configure -default 10
L1 configure -default 10
L2 configure -default 0
L4 configure -default 0
L6 configure -default 10

set UTF::SetupTestBed {
    L1 attn default
    L2 attn default
    L3 attn default
	L4 attn default 
	L5 attn default
        L6 attn default
    foreach S {4709/4366ap 4366a 4366b} {
                catch {$S wl down}
                $S deinit
    }
    return
}


::UTF::Streamslib::force_cleanup "4709/4366ap 4366sta1 4366sta2"


#################### Controller and hosts #################

UTF::Linux blr17end1.ban.broadcom.com \
	-lan_ip 10.132.116.117 \
	-sta "tst p7p1" 
	
tst configure -ipaddr 192.168.1.254 \

# UTF::Linux blr17-iptv-lx1.ban.broadcom.com \
        # -sta "lx1 p16p2" \
        # -power_button "auto" \
        # -power "npc8 1"\

# UTF::Linux blr17-iptv-lx2.ban.broadcom.com \
        # -sta "lx2 p16p2" \
        # -power_button "auto" \
        # -power "npc8 2" \

UTF::Linux blr17-iptv-lx3.ban.broadcom.com \
         -sta "lx3 p16p2" \
         -power_button "auto" \
         -power "npc8 5" \

lx3 configure -ipaddr 192.168.1.115 \

# UTF::Linux blr17-iptv-lx4.ban.broadcom.com \
        # -sta "lx4 p16p2" \
        # -power "npc11 6" \
        -power_button "auto" \

# UTF::Linux blr17-iptv-lx5.ban.broadcom.com \
        # -sta "lx5 p5p1" \

# UTF::Linux blr17-iptv-lx7.ban.broadcom.com \
        # -sta "lx7 eno2" \

#################### Routers ############################

UTF::Router 4709ap \
     -sta "4709/4366ap eth1 4709/4366ap.%15 wl0.% 4709/4366g eth2 4709/4366g.%15 wl1.%" \
     -lan_ip 192.168.1.1 \
     -relay "blr17end1.ban.broadcom.com" \
     -lanpeer {tst lx3} -web 2 \
     -power "npc8 3" \
	 -tag BISON04T_REL_7_14_164 \
     -console "blr17end1.ban.broadcom.com:40000" \
     -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
     -trx linux \
	 -yart {-attn5g {20-83} -frameburst 1} \
	-embeddedimage {4366c} \
	-datarate {-i 0.5} -udp 1.8g \
	    -noradio_pwrsave 1 \
	-nvram { \
	watchdog=3000
        wl0_ssid=4709/4366ap
        wl0_chanspec=36
        wl0_radio=0
        wl0_bw_cap=-1
        wl0_country_code=Q1
	wl0_country_rev=137
        wl0_atf=0
        wl0_vht_features=7
        wl0_mu_features="-1"
        wl1_ssid=4709/4366g
        wl1_chanspec=3
        wl1_radio=0
        wl1_atf=0
        wl1_vht_features=5
		} \
    -udp 1.8G \
    -pre_perf_hook {{%S wl assoc} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters}}

4709/4366ap configure -attngrp L1 \
        -channelsweep {-band a} \
        -yart {-attn5g {20-83} -frameburst 1} \
        -datarate {-i 0.5 -frameburst 1} -udp 2.2g

4709/4366g configure -attngrp L1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8g \
    -noradio_pwrsave 1 \
    -yart {-attn2g {20-83} -frameburst 1} \
    -wlinitcmds {sleep 2; wl ver}

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

4709/4366ap clone 4709/4366ap-tot-NIC \
	-tag trunk \
	-brand linux-2.6.36-arm-up-internal-router

4709/4366g clone 4709/4366g-tot-NIC \
	-tag trunk \
	-brand linux-2.6.36-arm-up-internal-router

4709/4366ap clone 4709/4366ap-10.10-NIC \
	-tag EAGLE_BRANCH_10_10 \
	-brand linux-2.6.36-arm-up-internal-router

4709/4366g clone 4709/4366g-10.10-NIC \
	-tag EAGLE_BRANCH_10_10 \
	-brand linux-2.6.36-arm-up-internal-router

############################## blr17tst1 #####################################
# blr17tst1      - 4366
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19
# Adapter        - Adex PCIe Half  Miniflex
# RF Enclosure 3 - Ramsey STE 2900
# Power          - npc2 port 1     172.1.1.21
###########################################tcp#####################################

UTF::Linux blr17softap1 \
        -lan_ip 10.132.116.123 \
        -sta {4366a enp1s0} \
        -power "npc2 1"\
        -power_button "auto" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "EAGLE_BRANCH_10_10" \
        -udp 1.3G \
        -wlinitcmds {wl down; wl msglevel +assoc ; wl msglevel +rate ; wl mpc 0 ; wl frameburst 1 ; wl up; wl country US/0} \
        -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

4366a configure -ipaddr 192.168.1.130 -attngrp L3 \


4366a clone 4331t -tag trunk \

4366a clone 4366a1x1 \
	-wlinitcmds {wl down; wl msglevel +assoc ; wl msglevel +rate ; wl mpc 0 ; wl frameburst 1 ; wl up; wl txchain 1; wl rxchain 1; wl vht_features 7;} \

4366a clone 4366a1x1-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 1; wl rxchain 1; wl vhtmode 0;} \

4366a clone 4366a1x1-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 1; wl rxchain 1; wl vhtmode 0; wl nmode 0;} \

4366a clone 4366a2x2 \
	-wlinitcmds {wl down; wl msglevel +assoc ; wl msglevel +rate ; wl mpc 0 ; wl frameburst 1 ; wl up; wl txchain 3; wl rxchain 3; wl vht_features 7;} \

4366a clone 4366a2x2-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0;} \

4366a clone 4366a2x2-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0; wl nmode 0;} \



############################## blr17tst2 #####################################
# blr17tst2      - 43602
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19
# Adapter        - Adex PCIe Half  Miniflex
# RF Enclosure 3 - Ramsey STE 2900
# Power          - npc2 port 2     172.1.1.21
################################################################################

UTF::Linux blr17tst2 \
        -lan_ip 10.132.116.125 \
        -sta {4366b enp1s0} \
        -power "npc3 1" \
        -power_button "auto" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "EAGLE_BRANCH_10_10" \
        -udp 1.3G \
        -wlinitcmds {wl down ; wl msglevel +assoc; wl msglevel +rate ;wl mpc 0 ; wl frameburst 1 ; wl up; wl country US/0} \
        -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

4366b configure -ipaddr 192.168.1.131 -attngrp L1 \


4366b clone 4331t -tag trunk \

4366b clone 4366b1x1 \
	-wlinitcmds {wl down; wl msglevel +assoc ; wl msglevel +rate ; wl mpc 0 ; wl frameburst 1 ; wl up; wl txchain 1; wl rxchain 1; wl vht_features 7;} \

4366b clone 4366b1x1-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 1; wl rxchain 1; wl vhtmode 0;} \

4366b clone 4366b1x1-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 1; wl rxchain 1; wl vhtmode 0; wl nmode 0;} \

4366b clone 4366b2x2 \
	-wlinitcmds {wl down; wl msglevel +assoc ; wl msglevel +rate ; wl mpc 0 ; wl frameburst 1 ; wl up; wl txchain 3; wl rxchain 3; wl vht_features 7;} \

4366b clone 4366b2x2-ht \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0;} \

4366b clone 4366b2x2-lt \
	-wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl bw_cap 2g -1; wl vht_features 3; wl txchain 3; wl rxchain 3; wl vhtmode 0; wl nmode 0;} \


4366b clone 4360 \

################### Clone #######################


set UTF::RouterNightlyCustom {

   package require UTF::Test::MiniUplinks
   UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 

    package require UTF::Test::Repeaters
    UTF::Test::Repeaters $Router $STA1 -sta $STA2

}



