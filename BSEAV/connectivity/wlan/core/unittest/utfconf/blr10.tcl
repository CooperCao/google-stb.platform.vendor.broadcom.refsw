#########################################################################
# UTF configuration for BLR10 (IPTV) testbed Location Campus 1A DVT Lab 
# Created  by Sumesh Nair
#
# Controller  blr10end1.ban.broadcom  10.131.80.112
#
# Router  4708/43602mch   Ramseylx1  STE2900       NPC port 1     
# blr10-iptv-lx1.ban.broadcom.com  10.131.80.113   NPC port 2
#
# Router  4708/4360mc     Ramseylx2   STE2900      NPC port 3     
# blr10-iptv-lx2.ban.broadcom.com  10.131.80.114   NPC port 4
#
# Router  4706/43602mch   Ramseylx3  STE2900       NPC port 5   
# blr10-iptv-lx3.ban.broadcom.com  10.131.80.115   NPC port 6
#
# Router  4706/4360mc     Ramseylx4  STE2900       NPC port 7   
# blr10-iptv-lx4.ban.broadcom.com  10.131.80.116   NPC port 8
#
# Attenuator Aefoflex       172.1.1.100
# Power   NPC 8 port        172.1.1.11
#
################################################################

set ::UTF::SummaryDir "/projects/hnd_sig_ext18/sumesh/blr10"
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
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr10end1.ban.broadcom.com -rev 1

####################### Attenuator  ######################

UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr10end1.ban.broadcom.com" \
        -group {
                L1 {1 2 3}
                L2 {4 5 6}
                L3 {7 8 9}
                ALL {1 2 3 4 5 6 7 8 9}
                }
L1 configure -default 0 
L2 configure -default 0
L3 configure -default 0 
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    L1 attn 0
    catch {L3 attn 0;}
    catch {L2 attn 0;}
    catch {L1 attn 0;}

::UTF::Streamslib::force_cleanup "4709/43602ap 4708/4360proap  4709/43602sta  4708/4360prosta"

}

#################### Controller and hosts #################

UTF::Linux blr10end1.ban.broadcom.com -lan_ip 10.131.80.112

UTF::Linux blr10-iptv-lx1.ban.broadcom.com \
        -sta "lx1 p16p2" \
        -power_button "auto" \
        -power "npc11 2"\

UTF::Linux blr10-iptv-lx2.ban.broadcom.com \
        -sta "lx2 p16p2" \
        -power_button "auto" \
        -power "npc11 4" \

UTF::Linux blr10-iptv-lx3.ban.broadcom.com \
        -sta "lx3 p16p2" \
        -power_button "auto" \
        -power "npc11 6" \

UTF::Linux blr10-iptv-lx4.ban.broadcom.com \
        -sta "lx4 p16p2" \
        -power "npc11 8" \
        -power_button "auto" \

#################### Routers ############################

UTF::Router iptv_ap1 \
     -sta "4709/43602ap eth1 " \
     -lan_ip 192.168.1.2 \
     -relay "blr10-iptv-lx1.ban.broadcom.com" \
     -lanpeer "lx1 " \
     -power "npc11 1" \
     -console "blr10-iptv-lx1.ban.broadcom.com:40000" \
     -brand linux-2.6.36-arm-internal-router \
     -trx linux-apsta \
     -nvram {
	 wl_msglevel=0x101
	 fw_disable=1
         wl0_radio=1
	 wl0_obss_coex=0
	 wl0_ssid=BSS
	 wl0_channel=36
	 wl1_radio=1
         wl1_ssid=BSS_AP
	 wl1_obss_coex=0
         router_disable=0
         lan_ipaddr=192.168.1.2
         lan_gateway=192.168.1.2
         dhcp_start=192.168.1.110
         dhcp_stop=192.168.1.150
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
         }

UTF::Router iptv_ap2 \
    -sta "4708/4360proap eth1" \
    -lan_ip 192.168.1.3 \
    -relay "blr10-iptv-lx2.ban.broadcom.com" \
    -lanpeer "lx2" \
    -console "blr10-iptv-lx2.ban.broadcom.com:40000" \
    -power "npc11 3" \
    -brand linux-2.6.36-arm-internal-router \
    -trx linux-apsta \
    -nvram {
         wl_msglevel=0x101
         fw_disable=1
         wl0_radio=1
         wl0_obss_coex=0
         wl0_ssid=PROXIMATE_AP
         wl1_channel=161
         wl1_radio=1
         wl1_obss_coex=0
         router_disable=0
         lan_ipaddr=192.168.1.3
         lan_gateway=192.168.1.3
         dhcp_start=192.168.1.111
         dhcp_stop=192.168.1.150
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
         }

UTF::Router iptv_ap3 \
    -sta " 4709/43602sta eth1" \
    -lan_ip 192.168.1.4 \
    -relay "blr10-iptv-lx3.ban.broadcom.com" \
    -lanpeer "lx3" \
    -console "blr10-iptv-lx3.ban.broadcom.com:40000" \
    -power "npc11 5" \
    -brand linux-2.6.36-arm-internal-router \
    -trx linux-apsta \
    -nvram {
        wl_msglevel=0x101
        fw_disable=1
        watchdog=6000
        wl0_channel=1
        wl0_radio=1
        wl0_obss_coex=0
        wl0_ssid=BSS_STA
        wl1_ssid=STA
        wl1_channel=36
        wl1_radio=1
        wl1_obss_coex=0
        router_disable=0
        lan_ipaddr=192.168.1.4
        lan_gateway=192.168.1.4
        dhcp_start=192.168.1.112
        dhcp_stop=192.168.1.150
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
    }


UTF::Router iptv_ap4 \
    -sta "4708/4360prosta eth1" \
    -lan_ip 192.168.1.5 \
    -relay "blr10-iptv-lx4.ban.broadcom.com" \
    -lanpeer "lx4" \
    -console "blr10-iptv-lx4.ban.broadcom.com:40000" \
    -power "npc11 7" \
    -brand linux-2.6.36-arm-internal-router \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
	watchdog=6000
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4706/4331b
	wl1_ssid=Proximate_STA
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
        lan_ipaddr=192.168.1.5
        lan_gateway=192.168.1.5
        dhcp_start=192.168.1.113
        dhcp_stop=192.168.1.150
	boardtype=0x0646
        wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
	wl_country_code=US
	wl_country_rev=0
	wl0_country_code=US
	wl0_country_rev=0
    }



################### Clone #######################

set DUTS "4709/43602ap  4708/4360proap  4709/43602sta  4708/4360prosta "

foreach DUT $DUTS {
$DUT clone ${DUT}_AARD86    -tag AARDVARK01T_REL_6_37_14_86 
$DUT clone ${DUT}_bis33     -tag BISON04T_REL_7_14_89_33
$DUT clone ${DUT}_bis39     -tag BISON04T_REL_7_14_43_39
$DUT clone ${DUT}_bis04t    -tag BISON04T_REL_7_14_32
$DUT clone ${DUT}_bis14     -tag BISON04T_BRANCH_7_14  -brand linux-2.6.36-arm-internal-router-dhdap  
$DUT clone ${DUT}_bis37     -tag BISON04T_REL_7_14_89_37
$DUT clone ${DUT}_bis10     -tag BISON_BRANCH_7_10
$DUT clone ${DUT}_eagle10   -tag EAGLE_BRANCH_10_10 
                 }

#################### Attenuator  Bindings  ############################

4708/4360proap_eagle10   configure -attngrp L1
4708/4360proap_bis14     configure -attngrp L1
4709/43602sta_eagle10    configure -attngrp L2
4709/43602sta_bis14      configure -attngrp L2
4708/4360prosta_eagle10  configure -attngrp L3
4708/4360prosta_bis14    configure -attngrp L3


#4709/43602_DHD configure -attngrp L2  -wlinitcmds {wl down ; wl -i eth1 bsscfg_mode 0 ; wl up}
#4709/43602_NIC configure -attngrp L2  -wlinitcmds {wl down ; wl -i eth1 bsscfg_mode 1 ; wl up}


#################### Helper  Procs ############################

set ::UTFD::testscripts {
    
   UTFD::metascript %AUTO% -watch 4709/43602ap_bis14 -script "/home/sumesh/svn/unittest/Test/ProximateBSS.test -ap 4709/43602ap_bis14 -proximateap 4708/4360proap_bis14 -sta 4709/43602sta_bis14 -proximatesta 4708/4360prosta_bis14 -devicemode 1 -5g_rate auto -scansuppress -towards -email hnd-utf-list -graphtype canvas -K3 30 -title {BLR10 AP 4709/43602 PSTA 4709/43602 DHD K3 30} " -type triggered  -watchinterval 600


 UTFD::metascript %AUTO% -watch 4709/43602ap_eagle10 -script "/home/sumesh/svn/unittest/Test/ProximateBSS.test -ap 4709/43602ap_eagle10 -proximateap 4708/4360proap_eagle10  -sta 4709/43602sta_eagle10  -proximatesta 4708/4360prosta_eagle10  -devicemode 1 -5g_rate auto -scansuppress -towards -email hnd-utf-list -graphtype canvas -K3 30 -title {BLR10 AP 4709/43602 PSTA 4709/43602 NIC K3 30} " -type triggered  -watchinterval 600








}

proc loadall {} {
    UTFD::metascript %AUTO%  -script "./UTF.tcl 4708/43602 load; ./UTF.tcl  4708/4360 load; ./UTF.tcl 4709/43602 load; ./UTF.tcl 4709/4360 load;" -concurrent 1 -type now

 UTFD::metascript %AUTO%  -script "./UTF.tcl 4708/43602_eagle10 load; ./UTF.tcl  4708/4360_eagle10 load; ./UTF.tcl 4709/43602_eagle10 load; ./UTF.tcl 4709/4360_eagle10 load;" -concurrent 1 -type now
}

###################PTPD configuration #####################

array set ::ptpinterfaces [list lx1 em1 lx2 em1 lx3 em1 lx4 em1]
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
