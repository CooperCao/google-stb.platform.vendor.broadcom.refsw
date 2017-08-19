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
set ::UTF::MSTimeStamps 1

# To setup UTFD port for this rig
#set ::env(UTFDPORT) 9988

# To use wl from trunk (set default); Use -app_tag to modify.
set UTF::TrunkApps 1

# To disable/enable automatic restore of UTFD previous state
#set ::UTFD::norestore 0


set ::UTF::SummaryDir "/projects/hnd_sig_ext18/sumesh/blr17"
set ::env(UTFDPORT) 9977
package require UTF::Power
package require UTF::Aeroflex
package require UTF::WebRelay
package require UTFD
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
                L6 {1  2  3 4 }
                L1 {5  6  7 8 }
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


L5 configure -default 90  
L3 configure -default 90
L1 configure -default 90
L2 configure -default 90
L4 configure -default 90
L6 configure -default 90

set UTF::SetupTestBed {
    L1 attn default
    L2 attn default
    L3 attn default
	L4 attn default 
	L5 attn default
        L6 attn default
    foreach S {4709/4366ap  4709/43602proap 4709/4366sta} {
                catch {$S wl down}
                $S deinit
    }
    return
}


::UTF::Streamslib::force_cleanup "4709/4366ap 4709/4366proap 4709/43602proap 4709/4366sta 4366a 4366b"


#################### Controller and hosts #################

UTF::Linux blr17end1.ban.broadcom.com -lan_ip 10.131.80.163

UTF::Linux blr17-iptv-lx1.ban.broadcom.com \
        -sta "lx1 p16p2" \
        -power_button "auto" \
        -power "npc8 1"\

UTF::Linux blr17-iptv-lx2.ban.broadcom.com \
        -sta "lx2 p16p2" \
        -power_button "auto" \
        -power "npc8 2" \

UTF::Linux blr17-iptv-lx3.ban.broadcom.com \
        -sta "lx3 p16p2" \
        -power_button "auto" \
        -power "npc8 5" \

UTF::Linux blr17-iptv-lx4.ban.broadcom.com \
        -sta "lx4 p16p2" \
        -power "npc11 6" \
        -power_button "auto" \

UTF::Linux blr17-iptv-lx5.ban.broadcom.com \
        -sta "lx5 p5p1" \

UTF::Linux blr17-iptv-lx7.ban.broadcom.com \
        -sta "lx7 em2" \

#################### Routers ############################

UTF::Router iptv_ap1 \
     -sta "4709/4366ap eth1 " \
     -lan_ip 192.168.1.2 \
     -relay "blr17-iptv-lx1.ban.broadcom.com" \
     -lanpeer "lx1 lx5" \
     -power "npc8 3" \
     -console "blr17-iptv-lx1.ban.broadcom.com:40000" \
     -brand linux-2.6.36-arm-internal-router \
     -trx linux-apsta \
     -nvram {
	 wl_msglevel=0x101
	 fw_disable=1
         wl0_radio=1
	 wl0_obss_coex=0
	 wl0_ssid=BSS
	 wl0_channel=161/80
	 wl0_vht_features=7
         router_disable=0
         lan_ipaddr=192.168.1.2
         lan_gateway=192.168.1.2
         dhcp_start=192.168.1.121
         dhcp_stop=192.168.1.122
         wl_frameburst=1
         wl_reg_mode=h
	 wl_wmf_bss_enable=1
	 wl_wmf_ucigmp_query=1
	 wl_wmf_ucast_upnp=1
	 wl_wmf_acs_fcs_mode=1
	 wl_dcs_csa_unicast=1
	 wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	 wl_pspretend_retry_limit=5
	 wl_country_code=US
	 wl_country_rev=0
	 wl0_country_code=US
	 wl0_country_rev=0
          }\
    -udp 1.3G \
    -pre_perf_hook {{%S wl assoc} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1} 




UTF::Router iptv_ap2 \
    -sta "4709/43602proap eth1" \
    -lan_ip 192.168.1.3 \
    -relay "blr17-iptv-lx2.ban.broadcom.com" \
    -lanpeer "lx2" \
    -console "blr17-iptv-lx2.ban.broadcom.com:40000" \
    -power "npc3 1" \
    -brand linux-2.6.36-arm-internal-router \
    -trx linux-apsta \
    -nvram {
         wl_msglevel=0x101
         fw_disable=1
         wl0_radio=1
         wl0_obss_coex=0
         wl0_ssid=ProxBSS
         wl0_vht_features=7
         wl1_channel=161
         router_disable=0
         lan_ipaddr=192.168.1.3
         lan_gateway=192.168.1.3
         dhcp_start=192.168.1.123
         dhcp_stop=192.168.1.124
         wl_reg_mode=h
         wl_wmf_bss_enable=1
         wl_wet_tunnel=1
         wl_wmf_ucigmp_query=1
         wl_wmf_ucast_upnp=1
         wl_wmf_acs_fcs_mode=1
         wl_dcs_csa_unicast=1
         wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
         wl_pspretend_retry_limit=5
         wl_country_code=US
         wl_country_rev=0
         wl0_country_code=US
         wl0_country_rev=0
         }\
    -datarate {-i 1 -frameburst 1} -udp 2.0G \
    -pre_perf_hook {{%S wl assoc} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}




UTF::Router iptv_ap3 \
    -sta " 4709/4366proap eth1 " \
    -lan_ip 192.168.1.4 \
    -relay "blr17-iptv-lx3.ban.broadcom.com" \
    -lanpeer "lx3" \
    -console "blr17-iptv-lx3.ban.broadcom.com:40000" \
    -power "npc8 7" \
    -brand linux-2.6.36-arm-internal-router \
    -trx linux-apsta \
    -nvram {
        wl_msglevel=0x101
        fw_disable=1
        watchdog=6000
        wl0_channel=136/80
        wl0_radio=1
        wl0_obss_coex=0
        wl0_ssid=43602ProxBSS
        wl0_vht_features=3 
        router_disable=0
        lan_ipaddr=192.168.1.4
        lan_gateway=192.168.1.4
        dhcp_start=192.168.1.125
        dhcp_stop=192.168.1.126
        boardtype=0x0665  
        wl_reg_mode=h
        wl_wmf_ucigmp_query=1
        wl_wmf_ucast_upnp=1
        wl_wmf_acs_fcs_mode=1
        wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
        wl_pspretend_retry_limit=5
        wl_country_code=US
        wl_country_rev=0
        wl0_country_code=US
        wl0_country_rev=0
        }\
    -datarate {-i 1 -frameburst 1} -udp 2.0G \
    -pre_perf_hook {{%S wl assoc} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters} }\
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}


######################## SoftAP ##########################################

UTF::Linux blr17softap1 -sta {4366softap enp1s0} \
    -lan_ip 10.132.116.123 \
    -power "npc8 7" \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
    -wlinitcmds {
                 wl down ; wl msglevel 0x101; wl amsdu 0; wl mbss 1 ; wl ap 1 ; wl spect 0 ; wl frameburst 1 ; wl mimo_bw_cap 1
                }
4366softap configure -ipaddr 192.168.1.132 -attngrp L4 -ap 1 \

4366softap clone 4360softap \

#########################PSTA################################################

UTF::Router iptv_ap4 \
    -sta "4709/4366sta eth1" \
    -lan_ip 192.168.1.5 \
    -relay "blr17-iptv-lx4.ban.broadcom.com" \
    -lanpeer "lx4 lx7" \
    -console "blr17-iptv-lx4.ban.broadcom.com:40000" \
    -power "npc8 8" \
    -brand linux-2.6.36-arm-internal-router \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
	watchdog=6000
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=STA
	wl0_vht_features=7
	router_disable=0
        lan_ipaddr=192.168.1.5
        lan_gateway=192.168.1.5
        dhcp_start=192.168.1.127
        dhcp_stop=192.168.1.128
        boardtype=0x0665
        wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
	wl_country_code=US
	wl_country_rev=0
	wl0_country_code=US
	wl0_country_rev=0
      }\
    -datarate {-i 1 -frameburst 1} -udp 2.0G \
    -pre_perf_hook {{%S wl assoc} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters} }\
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

	
############################## blr17tst1 #####################################
# blr17tst1      - 4366
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19
# Adapter        - Adex PCIe Half  Miniflex
# RF Enclosure 3 - Ramsey STE 2900
# Power          - npc2 port 1     172.1.1.21
################################################################################

UTF::Linux blr17tst1 \
        -lan_ip 10.132.116.124 \
        -sta {4366a enp3s0} \
        -power "npc2 1"\
        -power_button "auto" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "EAGLE_BRANCH_10_10" \
        -udp 1.3G \
        -wlinitcmds {wl down; wl msglevel +assoc ; wl msglevel +rate ; wl mpc 0 ; wl frameburst 1 ; wl up } \
        -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

4366a configure -ipaddr 192.168.1.130 \


4366a clone 4331t -tag trunk \

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
        -sta {4366b enp3s0} \
        -power "npc2 2"\
        -power_button "auto" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "EAGLE_BRANCH_10_10" \
        -udp 1.3G \
        -wlinitcmds {wl down ; wl msglevel +assoc; wl msglevel +rate ;wl mpc 0 ; wl frameburst 1 ; wl up} \
        -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

4366b configure -ipaddr 192.168.1.131 \


4366b clone 4331t -tag trunk \


4366b clone 4360 \

################### Clone #######################

set DUTS "4709/4366ap 4709/4366proap 4709/43602proap 4366softap 4709/4366sta 4366a 4366b 4360 4331t"

foreach DUT $DUTS {
$DUT clone ${DUT}_AARD86    -tag AARDVARK01T_REL_6_37_14_86
$DUT clone ${DUT}_bis33     -tag BISON04T_REL_7_14_89_33
$DUT clone ${DUT}_bis39     -tag BISON04T_REL_7_14_43_39
$DUT clone ${DUT}_bis04t    -tag BISON04T_REL_7_14_32
$DUT clone ${DUT}_bis14     -tag BISON04T_BRANCH_7_14  -brand linux-2.6.36-arm-internal-router-dhdap  
$DUT clone ${DUT}_bis14a    -tag BISON04T_BRANCH_7_14
$DUT clone ${DUT}_bis37     -tag BISON04T_REL_7_14_89_37
$DUT clone ${DUT}_bis10     -tag BISON_BRANCH_7_10
$DUT clone ${DUT}_bis7_14_131_6e   -tag BISON04T_REL_7_14_131_6  -brand linux-2.6.36-arm-external-vista-router-dhdap-full-src 
$DUT clone ${DUT}_bis7_14_131_6i   -tag BISON04T_REL_7_14_131_6  -brand linux-2.6.36-arm-internal-router-dhdap-atlas 
$DUT clone ${DUT}_bis7_14_131_6a  -tag BISON04T_REL_7_14_131_6  -brand linux-internal-wl
$DUT clone ${DUT}_bis7_14_131_7e   -tag BISON04T_REL_7_14_131_7  -brand linux-2.6.36-arm-external-vista-router-dhdap-full-src
$DUT clone ${DUT}_bis7_14_131_7i   -tag BISON04T_REL_7_14_131_7  -brand linux-2.6.36-arm-internal-router-dhdap-atlas 
$DUT clone ${DUT}_bis7_14_131_7a  -tag BISON04T_REL_7_14_131_7  -brand linux-internal-wl
$DUT clone ${DUT}_eagle10_70 -tag EAGLE_REL_10_10_70
$DUT clone ${DUT}_eagle10   -tag EAGLE_BRANCH_10_10
$DUT clone ${DUT}_eagle10_11n  -tag EAGLE_BRANCH_10_10  -wlinitcmds {wl down ;wl phymsglevel +0x800; wl msglevel +assoc; wl mimo_bw_cap 1; wl ack_ratio 2; wl amsdu 1; wl msglevel +rate;  wl vhtmode 0 ; wl mpc 0;  frameburst 1; wl up}
$DUT clone ${DUT}_eagle10_11ac -tag EAGLE_BRANCH_10_10  -wlinitcmds {wl down ;wl phymsglevel +0x800; wl msglevel +assoc; wl mimo_bw_cap 1; wl ack_ratio 2; wl amsdu 1; wl msglevel +rate;  wl vhtmode 1 ; wl mpc 0; frameburst 1; wl up}
                 }

#4709/43602_DHD configure -attngrp L2  -wlinitcmds {wl down ; wl -i eth1 bsscfg_mode 0 ; wl up}
#4709/43602_NIC configure -attngrp L2  -wlinitcmds {wl down ; wl -i eth1 bsscfg_mode 1 ; wl up}


#################### Attenuator  Bindings  ############################

################L1##########L4#######
#         #           #             #
#  AP     # PROX AP1  #  PROX AP2   #
#         #           #             #
#####################################
#         #           #             #
# PSTA    # PROX STA1 #  PROX STA2  #
#         #           #             #
####L2##########L3##########L5#######




4709/43602proap         configure -attngrp L1
4709/43602proap_eagle10 configure -attngrp L1
4709/43602proap_bis14   configure -attngrp L1

4709/4366sta            configure -attngrp L2
4709/4366sta_bis14      configure -attngrp L2
4709/4366sta_eagle10    configure -attngrp L2
4709/4366sta            configure -attngrp L2
4709/4366sta_bis7_14_131_6i  configure -attngrp L2
4709/4366sta_bis7_14_131_6e  configure -attngrp L2 
4709/4366sta_bis7_14_131_7i  configure -attngrp L2
4709/4366sta_bis7_14_131_7e  configure -attngrp L2


4366softap_eagle10        configure -attngrp L4
4366softap_eagle10_70     configure -attngrp L4
4366softap_bis14a         configure -attngrp L4
4366softap_bis7_14_131_6a configure -attngrp L4
4366softap_bis7_14_131_7a configure -attngrp L4

4366a                   configure -attngrp L3
4366a_eagle10           configure -attngrp L3
4366a_eagle10_11n       configure -attngrp L3
4366a_eagle10_11ac      configure -attngrp L3
4366a_bis14             configure -attngrp L3


4366b_eagle10           configure -attngrp L5
4366b_eagle10_11n       configure -attngrp L5
4366b_eagle10_11ac      configure -attngrp L5
4366b                   configure -attngrp L5
4331t                   configure -attngrp L5
4331t_eagle10_70        configure -attngrp L5




###################PTPD configuration #####################

array set ::ptpinterfaces [list lx1 em1 lx2 em1 lx3 em1 lx4 em1 lx5 em1 lx7 em1 4366softap em1 4366a em1  4366b em1]
proc ::enable_ptp {args} {
    if {![llength $args]} {
        set devices [array names ::ptpinterfaces]
    }  else {
        set devices $args
    }
    foreach dut $devices {
        # set device [lindex [split $dut _] 0]
        set device $dut
        set interface $::ptpinterfaces([namespace tail $device])
        catch {$device rexec service ntpd stop}
        catch {$device rexec pkill ptpd2}
        #  catch {$device rexec /root/ptpd-2.3.1-rc3/src/[$device uname -r]/ptpd2 -b em1 -W -S}
        catch {$device rm /var/log/ptp*.log}
        catch {$device rexec /root/ptpd-2.3.1-rc3/src/ptpd2 -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface}
        catch {$device ip route replace multicast 224.0.0.107/32 dev $interface}
        catch {$device ip route replace multicast 224.0.1.129/32 dev $interface}
    }
    UTF::Sleep 120
    foreach dut $devices {
        catch {$device lan rexec tail -10 /var/log/ptpstats.log}
        $device deinit
    }
}



##################### Metascripts  #########################################

set ::UTFD::testscripts {



# ATF DHD #


UTFD::metascript %AUTO% -watch 4709/4366ap_bis14  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4709/4366ap_bis14 -sta { 4366b_eagle10  4366a_eagle10} -title { BLR17 4709/4366 DHD  <-> STA1 4366 11n &  STA2 4366 11ac BKGND TRAFFIC=10M TCPTRACE OFF } -email hnd-utf-list -distances {15 15 0}  -tcptrace 0 -notcprate -background 10M " -type triggered  -watchinterval 600

UTFD::metascript %AUTO% -watch 4709/4366ap_bis14  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4709/4366ap_bis14 -sta { 4366b_eagle10  4366a_eagle10} -title { BLR17 4709/4366 DHD  <-> STA1 4366 11n &  STA2 4366 11ac BKGND TRAFFIC= 0 TCPTRACE OFF } -email hnd-utf-list -distances {15 15 0}  -tcptrace 0 -notcprate -background 0 " -type triggered  -watchinterval 600

UTFD::metascript %AUTO% -watch 4709/43602proap_bis14  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4709/43602proap_bis14 -sta { 4366b_eagle10  4366a_eagle10} -title { BLR17 4709/43602 DHD  <-> STA1 4366 11n &  STA2 4366 11ac BKGND TRAFFIC=10 TCPTRACE OFF} -email hnd-utf-list -distances {15 15 0} -tcptrace 0 -notcprate -background 10M " -type triggered  -watchinterval 600


UTFD::metascript %AUTO% -watch 4709/43602proap_bis14  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4709/43602proap_bis14 -sta { 4366b_eagle10  4366a_eagle10} -title { BLR17 4709/43602 DHD  <-> STA1 4366 11n &  STA2 4366 11ac BKGND TRAFFIC= 0 TCPTRACE OFF} -email hnd-utf-list -distances {15 15 0} -tcptrace 0 -notcprate -background 0 " -type triggered  -watchinterval 600


UTFD::metascript %AUTO% -watch 4366softap_bis14a  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4366softap_bis14a -sta {4366b_eagle10 4366a_eagle10} -title { BLR17 4366SoftAP DHD  <-> STA-11ac 4366 & STA-11n 4366 BKGND TRAFFIC=10M TCPTRACE OFF } -email hnd-utf-list -distances {15 15 0} -tcptrace 0 -notcprate -background 10M " -type triggered  -watchinterval 600

UTFD::metascript %AUTO% -watch 4366softap_bis14a  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4366softap_bis14a -sta {4366b_eagle10 4366a_eagle10} -title { BLR17 4366SoftAP DHD  <-> STA-11ac 4366 & STA-11n 4366 BKGND TRAFFIC= 0 TCPTRACE OFF } -email hnd-utf-list -distances {15 15 0} -tcptrace 0 -notcprate -background 0 " -type triggered  -watchinterval 600




# ATF NIC #

UTFD::metascript %AUTO% -watch 4709/4366ap_eagle10  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4709/4366ap_eagle10 -sta { 4366b_eagle10  4366a_eagle10} -title { BLR17 4709/4366 NIC  <-> STA1 4366 11n &  STA2 4366 11ac BKGND TRAFFIC=10M TCPTRACE OFF } -email hnd-utf-list -distances {15 15 0}  -tcptrace 0 -notcprate -background 10M " -type triggered  -watchinterval 600

UTFD::metascript %AUTO% -watch 4709/4366ap_eagle10  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4709/4366ap_eagle10 -sta { 4366b_eagle10  4366a_eagle10} -title { BLR17 4709/4366 NIC  <-> STA1 4366 11n &  STA2 4366 11ac BKGND TRAFFIC= 0 TCPTRACE OFF } -email hnd-utf-list -distances {15 15 0}  -tcptrace 0 -notcprate -background 0 " -type triggered  -watchinterval 600



UTFD::metascript %AUTO% -watch 4709/43602proap_eagle10  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4709/43602proap_eagle10 -sta { 4366b_eagle10  4366a_eagle10} -title { BLR17 4709/43602 NIC  <-> STA1 4366 11n &  STA2 4366 11ac BKGND TRAFFIC=10 TCPTRACE OFF} -email hnd-utf-list -distances {15 15 0} -tcptrace 0 -notcprate -background 10M " -type triggered  -watchinterval 600

UTFD::metascript %AUTO% -watch 4709/43602proap_eagle10  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4709/43602proap_eagle10 -sta { 4366b_eagle10  4366a_eagle10} -title { BLR17 4709/43602 NIC  <-> STA1 4366 11n &  STA2 4366 11ac BKGND TRAFFIC= 0 TCPTRACE OFF} -email hnd-utf-list -distances {15 15 0} -tcptrace 0 -notcprate -background 0 " -type triggered  -watchinterval 600



UTFD::metascript %AUTO% -watch 4366softap_eagle10  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4366softap_eagle10 -sta {4366b_eagle10 4366a_eagle10} -title { BLR17 4366SoftAP NIC  <-> STA-11ac 4366 & STA-11n 4366 BKGND TRAFFIC=10M TCPTRACE OFF } -email hnd-utf-list -distances {15 15 0} -tcptrace 0 -notcprate -background 10M " -type triggered  -watchinterval 600

UTFD::metascript %AUTO% -watch 4366softap_eagle10  -script "/home/sumesh/svn/unittest/Test/ATF.test -ap 4366softap_eagle10 -sta {4366b_eagle10 4366a_eagle10} -title { BLR17 4366SoftAP NIC  <-> STA-11ac 4366 & STA-11n 4366 BKGND TRAFFIC= 0 TCPTRACE OFF } -email hnd-utf-list -distances {15 15 0} -tcptrace 0 -notcprate -background 0 " -type triggered  -watchinterval 600


##### Proximate BSS ###########

# NIC #

UTFD::metascript %AUTO% -watch 4709/4366ap_eagle10 -script "/home/sumesh/svn/unittest/Test/ProximateBSS.test -ap 4709/4366ap_eagle10 -proximateap 4709/43602proap_eagle10 -sta 4709/4366sta_eagle10 -proximatesta 4366a_eagle10 -devicemode 1 -5g_rate auto -scansuppress -towards -email hnd-utf-list -graphtype canvas -title { BLR17 AP 4709+4366 NIC  PSTA 4709+4366 NIC  K3 50 } -K3 50 "  -type triggered  -watchinterval 600 

# DHD #

UTFD::metascript %AUTO% -watch 4709/4366ap_bis14 -script "/home/sumesh/svn/unittest/Test/ProximateBSS.test -ap 4709/4366ap_bis14 -proximateap 4709/43602proap_bis14 -sta 4709/4366sta_bis14 -proximatesta 4366a_eagle10 -devicemode 1 -5g_rate auto -scansuppress -towards -email hnd-utf-list -graphtype canvas -title { BLR17 AP 4709+4366 DHD  PSTA 4709+4366 DHD  K3 50 } -K3 50 "  -type triggered  -watchinterval 600 


}



