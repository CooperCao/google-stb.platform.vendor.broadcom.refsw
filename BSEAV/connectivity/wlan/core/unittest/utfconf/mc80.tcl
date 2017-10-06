#
# UTF configuration for MC80 (4360) testing
# Robert J. McMahon (setup next to my cube)
# March 2012
#
# IP Address range 10.19.87.1-10
#
# controlbyweb relay 2 172.16.2.181
# consoles: 172.176.2.174, 175
# SummaryDir sets the location for test results in nightly testing.
# set ::UTF::SummaryDir "/projects/hnd_sig_ext4/rmcmahon/mc80"
# set ::UTF::SummaryDir "/projects/hnd_sig_ext10/rmcmahon/mc80"
set ::UTF::SummaryDir "/projects/hnd_sig_ext16/rmcmahon/mc80"
set ::env(UTFDPORT) 9977
package require UTF::Power
package require UTF::AeroflexDirect
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

UTF::Linux mc80-utf -lan_ip 10.19.87.1
UTF::Linux sfast-utf -lan_ip 10.19.85.171

UTF::WebRelay RamseyLx1 -lan_ip 172.16.2.184 -port 1
UTF::WebRelay RamseyLx2 -lan_ip 172.16.2.184 -port 2

UTF::Power::PicoPSU pwr_ap -lan_ip 172.16.2.190
UTF::Power::PicoPSU pwr_br1 -lan_ip 172.16.2.191
UTF::Power::PicoPSU pwr_br2 -lan_ip 172.16.2.193
UTF::Power::PicoPSU pwr_4331lx -lan_ip 172.16.2.194
UTF::Power::PicoPSU pwr_4708 -lan_ip 172.16.2.195

UTF::Linux sfast-lx2 -lan_ip 10.19.85.169 \
                    -sta {sflx2 em2.177 sflx2-170 em2.170}


sflx2 configure -ipaddr 192.168.1.42
sflx2-170 configure -ipaddr 192.168.1.52

UTF::Router _4708/4708psta -name PSTA-4708 \
    -sta {4708psta eth1 4331psta eth2} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "AARDVARK_BRANCH_6_30" \
    -power {pwr_4708} \
    -relay "mc80-lx4" \
    -lanpeer vlan177_if \
    -lan_ip 192.168.1.5 \
    -console 10.19.87.5:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.5
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4708/4360
	wl1_ssid=4708/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
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
	wl_country_rev=45
	wl0_country_code=US
	wl0_country_rev=45
    }

UTF::Router _4706/4360mchpsta -name PSTA-4706 \
    -sta {4706ap eth1 4331psta2 eth2} \
    -brand linux26-internal-router \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_ap} \
    -relay "mc80-lx1" \
    -lanpeer vlan170_if \
    -lan_ip 192.168.1.2 \
    -console 10.19.87.2:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.2
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4706/4360
	wl1_ssid=4706/4331
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
	wl_country_rev=45
	wl0_country_code=US
	wl0_country_rev=45
    }

UTF::Router _4706/4360mch -name AP-MCH \
    -sta {4360ap eth1 4331ap eth2} \
    -brand linux26-internal-router \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_ap} \
    -relay "mc80-lx1" \
    -lanpeer vlan170_if \
    -lan_ip 192.168.1.2 \
    -console 10.19.87.2:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.2
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4706/4360
	wl1_ssid=4706/4331
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
	wl_country_rev=45
	wl0_country_code=US
	wl0_country_rev=45
    }

UTF::Router _4708/4708 -name AP-4708 \
    -sta {4708ap eth1 4331ap2 eth2} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_4708} \
    -relay "mc80-lx4" \
    -lanpeer vlan177_if \
    -lan_ip 192.168.1.5 \
    -console 10.19.87.5:40000 \
    -trx linux\
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.5
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=MyBSS0
	wl1_ssid=MyBSS1
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
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
	wl_country_rev=45
	wl0_country_code=US
	wl0_country_rev=45
    }

UTF::Router _4708/4708retail -name AP-4708 \
    -sta {4708ap_retail eth1 4331ap2_retail eth2} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_4708} \
    -relay "mc80-lx4" \
    -lanpeer vlan177_if \
    -lan_ip 192.168.1.5 \
    -console 10.19.87.5:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.5
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=4708/4331
	wl0_chanspec=3
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
	wl1_ssid=4708/4360
	wl1_chanspec=36
	wl1_obss_coex=0
	wl1_bw_cap=-1
	wl1_radio=0
	fw_disable=1
	wl_country_code=US
	wl_country_rev=45
	wl0_country_code=US
	wl0_country_rev=45
    }


UTF::Router _4708br1 -name Br1 \
    -sta {4331br1 eth1 4360br1 eth2} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_br1} \
    -relay "mc80-lx2" \
    -bootwait 30 \
    -lanpeer vlan171_if \
    -lan_ip 192.168.1.3 \
    -console 10.19.87.3:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.3
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=ProxBSS0
	wl1_ssid=ProxBSS1
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	pktc_enable=0
	wl_country_code=US
	wl_country_rev=45
	wl1_country_code=US
	wl1_country_rev=45
    }

UTF::Router _4706br2 -name Br2-MCH \
    -sta {4360br2 eth1 4331br2 eth2} \
    -brand linux26-internal-router \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_br2} \
    -bootwait 30 \
    -relay "mc80-lx3" \
    -lanpeer vlan173_if \
    -lan_ip 192.168.1.4 \
    -console 10.19.87.4:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.4
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4706/4360
	wl1_ssid=4706/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	boardtype=0x0617 # 4706nr2hmc
	# pktc_enable=0
	wl_country_code=US
	wl_country_rev=45
	wl0_country_code=US
	wl0_country_rev=45
    }

UTF::MacOS mc80tst7 -sta {MacX en0} \
        -power {X29CPower} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacX clone MacX-DOME -tag "BIS715GALA_REL_7_21_*" -custom 1

UTF::AeroflexDirect AF-A -lan_ip 172.16.2.117  -retries 0 -concurrent 0 -silent 0 -group {L1 {1 2 3} L2 {4 5 6} L3 {7 8 9} L4 {10 11 12} ALL {1 2 3 4 5 6} C1 {1} C2 {2} C3 {3} C4 {4} C5 {5} C6 {6}}
UTF::AeroflexDirect AF-B -lan_ip 172.16.2.116 -retries 0 -concurrent 0 -silent 0 -group {L5 {1 2 3} L6 {4 5 6} L7 {7 8 9} L8 {10 11 12} ALL {1 2 3 4 5 6} C7 {1} C8 {2} C9 {3} C10 {4} C11 {5} C12 {6}}

#    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl dump txbf}} \
#    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}}

# after 1000 ::UTFD::restorestate

