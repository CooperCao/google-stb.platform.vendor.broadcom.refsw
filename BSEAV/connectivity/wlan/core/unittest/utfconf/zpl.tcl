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
package require UTF::Sniffer

# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::FTTR
    package require UTF::Test::ConfigBridge
    catch {source utfconf/kpiscripts.tcl}
}

set ::afmaxattn 95
set ::UTF::MSTimeStamps 1

UTF::Linux mc80-utf -lan_ip 10.19.87.1
UTF::Linux sfast-utf -lan_ip 10.19.85.171

UTF::WebRelay RamseyLx1 -lan_ip 172.16.2.184 -port 1
UTF::WebRelay ioslx1pwr -lan_ip 172.16.2.184 -port 2

UTF::Power::PicoPSU pwr_ap -lan_ip 172.16.2.190
UTF::Power::PicoPSU pwr_br1 -lan_ip 172.16.2.191
UTF::Power::PicoPSU pwr_br2 -lan_ip 172.16.2.193
UTF::Power::PicoPSU pwr_4708 -lan_ip 172.16.2.195
UTF::Power::PicoPSU pwr_43602lx1 -lan_ip 172.16.2.174

UTF::Power::Synaccess mc80npc1 -lan_ip 172.16.1.20 -rev 1
UTF::Power::Synaccess mc80npc2 -lan_ip 172.16.1.21 -rev 1
UTF::Power::Synaccess mc80npc3 -lan_ip 172.16.2.173

UTF::Sniffer SNIF -lan_ip 10.19.85.230 -user root -tag BISON_BRANCH_7_10 -sta {sta4360 enp3s0}

UTF::Linux sfast-lx2 -lan_ip 10.19.85.169 \
                    -sta {sflx2 em2.177 sflx2-170 em2.170 10G-B em2.202}

UTF::Linux sfast-lx1 -lan_ip 10.19.87.176 \
                    -sta {10G-A em2.202}


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
    -console 10.19.87.169:40000 \
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
	wl_country_code=Q1
	wl_country_rev=45
	wl0_country_code=Q1
	wl0_country_rev=45
	wl1_country_code=Q1
	wl1_country_rev=45
    }

UTF::Router _4708br1/4360mchpsta -name PSTA-4708 \
    -sta {4360mch5br1 eth1 4360mch2br1 eth2} \
    -brand "linux-2.6.36-arm-internal-router" \
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
	wl0_ssid=MyBSS0/4706
	wl1_ssid=MyBSS1/4706
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
	wl_country_code=Q1
	wl_country_rev=45
	wl0_country_code=Q1
	wl0_country_rev=45
	wl1_country_code=Q1
	wl1_country_rev=45
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
	wl0_ssid=MyBSS/4706A
	wl1_ssid=4706/4331
	wl1_channel=36
	wl1_radio=0
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
	boardtype=0x0617; # 4706nr2hmc
	wl_country_code=Q1
	wl_country_rev=45
	wl0_country_code=Q1
	wl0_country_rev=45
	wl1_country_code=Q1
	wl1_country_rev=45
    }

UTF::Router _4708/4708 -name AP-4708 \
    -sta {4708ap eth1 4331ap2 eth2} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_4708} \
    -relay "sflx2" \
    -lanpeer sflx2 \
    -lan_ip 192.168.1.5 \
    -console 10.19.85.169:40000 \
    -trx linux \
    -portmirror mirror1 \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.5
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=NetG7K_5G
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
	wl_country_code=Q1
	wl_country_rev=45
	wl0_country_code=Q1
	wl0_country_rev=45
	wl1_country_code=Q1
	wl1_country_rev=45
    }

UTF::Router _4708/4708retail -name AP-4708 \
    -sta {4708ap_retail eth1 4331ap2_retail eth2} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_4708} \
    -relay "mc80-lx4" \
    -lanpeer vlan177_if \
    -lan_ip 192.168.1.5 \
    -console 10.19.87.169:40000 \
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
	wl_country_code=Q1
	wl_country_rev=45
	wl0_country_code=Q1
	wl0_country_rev=45
	wl1_country_code=Q1
	wl1_country_rev=45
	wl_reg_mode=h
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
	wl1_radio=0
	wl1_obss_coex=0
	router_disable=0
	pktc_enable=0
	wl_country_code=Q1
	wl_country_rev=45
	wl0_country_code=Q1
	wl0_country_rev=45
	wl1_country_code=Q1
	wl1_country_rev=45
	wl_reg_mode=h
    }

#    -tag "BISON_BRANCH_7_10" \
#    -lanpeer vlan173_if
UTF::Router _4709 -name 4709 \
    -sta {4360_2G eth1 4360_5G eth2} \
    -brand linux-2.6.36-arm-internal-router \
    -tag "EAGLE_BRANCH_10_10" \
    -power {mc80npc3 1} \
    -bootwait 30 \
    -relay "sflx2" \
    -lanpeer sflx2 \
    -console 10.19.85.169:40001 \
    -lan_ip 192.168.1.15 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        wl_msglevel=0x101
        fw_disable=0
        lan_ipaddr=192.168.1.15
        wl0_ssid=43602
        watchdog=6000
    }

UTF::Router _4709B -name 4709 \
    -sta {4360R eth1 4360_5G eth2} \
    -brand linux-2.6.36-arm-internal-router \
    -tag "EAGLE_BRANCH_10_10" \
    -power {mc80npc3 1} \
    -bootwait 30 \
    -relay "sflx2" \
    -lanpeer sflx2 \
    -console 10.19.85.169:40001 \
    -lan_ip 192.168.1.15 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -portmirror mirror2 \
    -nvram {
        wl_msglevel=0x101
        fw_disable=0
        lan_ipaddr=192.168.1.15
        wl0_ssid=MyBSS0
        watchdog=6000
    }

4360R configure -attngrp L7 -date 2015.9.24.1

UTF::AeroflexDirect AF-A -lan_ip 172.16.2.117 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 -group {L1 {1 2 3} L2 {4 5 6} L3 {7 8 9} L4 {10 11 12} ALLC {1 2 3 4 5 6 7 8 9 10 11 12} C1 {1} C2 {2} C3 {3} C4 {4} C5 {5} C6 {6} C7 {7} C8 {8} C9 {9} C10 {10} C11 {11} C12 {12}}
UTF::AeroflexDirect AF-B -lan_ip 172.16.2.116 -retries 0 -concurrent 0 -silent 0 -group {L5 {1 2 3} L6 {4 5 6} L7 {7 8 9} ALLD {1 2 3 4 5 6 7 8 9} D1 {1} D2 {2} D3 {3} D4 {4} D5 {5} D6 {6} D7 {7} D8 {8} D9 {9}}

#    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl dump txbf}} \
#    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}}

#    -wlinitcmds {wl msglevel +assoc; wl btc_mode 0; wl mimo_bw_cap 1; wl ack_ratio 2; wl amsdu 1; wl vht_features 1; wl msglevel +scan; wl msglevel +rate}
UTF::Linux itx-lx1 -lan_ip 10.19.87.10 \
    -sta {43602lx1 enp1s0} \
    -tag NIGHTLY -date 2015.8.7.0 \
    -power {pwr_43602lx1} \
    -udp 1.2G \
    -wlinitcmds {wl msglevel +assoc} \
    -brand "linux-internal-wl"

UTF::Linux itx-lx2 -lan_ip 10.19.87.9 \
    -sta {43602lx2 eth0} \
    -power RamseyLx1 \
    -tag NIGHTLY -date 2015.8.7.0 \
    -wlinitcmds {wl msglevel +assoc; wl btc_mode 0; wl mimo_bw_cap 1; wl ack_ratio 2; wl amsdu 1} \
    -udp 1.2G \
    -brand "linux-internal-wl"

43602lx1 clone 43602sta_AC -tag EAGLE_BRANCH_10_10 -name "AC" -wlinitcmds {wl msglevel +assoc; wl btc_mode 0; wl mimo_bw_cap 1; wl ack_ratio 0; wl amsdu 1; wl vht_features 1; wl msglevel +scan; wl vhtmode 1}
43602lx2 clone 43602sta_11N -tag EAGLE_BRANCH_10_10 -name "11N" -wlinitcmds {wl msglevel +assoc; wl btc_mode 0; wl mimo_bw_cap 1; wl ack_ratio 0; wl amsdu 1; wl vht_features 1; wl msglevel +scan; wl vhtmode 0}
43602sta_11N configure -attngrp L1
43602sta_AC configure -attngrp L2

43602lx2 clone 43602lx2tot -tag NIGHTLY
#43602lx1 configure -ipaddr 192.168.1.11 -ap 1 -hasdhcpd 1 -date 2015.7.8.0
43602lx1 clone 4360SoftAP
4360SoftAP configure -ipaddr 192.168.1.11 -ap 1 -hasdhcpd 1 -date 2015.7.8.0 -power RamseyLx1

UTF::Linux brix1 -lan_ip 10.19.85.137 \
    -sta {43602brix1 enp2s0} \
    -tag BISON_BRANCH_7_10 \
    -power {} \
    -console 10.19.85.138:10001 \
    -wlinitcmds {wl msglevel +assoc} \
    -brand "linux-internal-wl"


UTF::MacOS MacDVT -lan_ip 10.19.85.137 \
    -sta {mac en0} \
    -tag NIGHTLY \
    -power {} \
    -wlinitcmds {wl msglevel +assoc; apple80211 -debug=err,rsn,scan} \
    -brand "linux-internal-wl"

mac configure -attngrp L4

43602brix1 configure -ipaddr 192.168.1.181 -attngrp L4
43602brix1 configure -wlinitcmds "ifconfig [43602brix1 cget -device] 192.168.1.181"

UTF::DHD lx1Host \
        -lan_ip 10.19.87.10 \
        -sta {43602lx1dhd eth0} \
        -dhd_tag trunk \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tcpwindow 4m -udp 1.2g \
        -datarate {-i 0.5 -frameburst 1} \
        -slowassoc 5 -nocal 1 -docpu 1 \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +err}

#43602lx2dhd clone 43602lx2dhdb -tag BISON_BRANCH_7_10 \
#	-type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus/rtecdc.bin

43602lx1dhd clone 43602lx1dhdb -tag BISON_BRANCH_7_10 \
	-type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus/rtecdc.bin

43602lx1dhd clone 43602lx1dhdn -tag NIGHTLY \
	-type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus/rtecdc.bin

UTF::DHD lx2Host \
        -lan_ip 10.19.87.9 \
        -sta {43602lx2dhd eth0} \
        -dhd_tag trunk \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tcpwindow 4m -udp 1.2g \
        -datarate {-i 0.5 -frameburst 1} \
        -slowassoc 5 -nocal 1 -docpu 1 \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel 0x0}

#        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3;}

43602lx2dhd clone 43602lx2dhdb -tag BISON_BRANCH_7_10 \
	-type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus/rtecdc.bin


# Console: 10.19.85.138
#    -dhd_tag DHD_BRANCH_1_359
#     -dhd_tag trunk
#    -type 4350c5-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv-ecounters-bssinfo

UTF::DHD itx-lx3 -lan_ip 10.19.87.8 \
    -sta {ioslx1 eth0} \
    -power ioslx1pwr \
    -extsup 1 \
    -app_tag BIS120RC4_BRANCH_7_15 \
    -tag BIS120RC4_BRANCH_7_15 \
    -dhd_tag DHD_BRANCH_1_359 \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94350RieslingBMurataMT.txt" \
    -clm_blob 4350_riesling_b.clm_blob \
    -type 4350c5-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv.bin \
    -customer olympic \
    -wlinitcmds {wl msglevel 0x101; wl ampdu_mpdu 24;wl ampdu_release 12;wl vht_features 3} \
    -slowassoc 5 -docpu 1 -escan 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -tcpwindow 2m -udp 800m -nocal 1

ioslx1 configure -attngrp L3

ioslx1 clone ioslx1x \
    -customer olympic \
    -type ../C-4350__s-C2/rieslingb.trx -clm_blob rieslingb.clmb

ioslx1 clone ios_stowe \
    -name "STOWE" \
    -type 4350c5-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv.bin \
    -app_tag BIS120RC4_REL_7_15_153_71 \
    -tag  BIS120RC4_REL_7_15_153_71

ioslx1 clone ios_monarch_int \
    -name "MONARCH" \
    -nvram "bcm94350ZinfandelBMurataMT.txt" \
    -type 4350c5-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv-ecounters-bssinfo \
    -app_tag BIS715T185RC1_REL_7_35_217_3 \
    -tag BIS715T185RC1_REL_7_35_217_3

ios_monarch_int clone ios_monarch \
    -customer olympic \
    -type ../C-4350__s-C5/zinfandelb.trx -clm_blob zinfandelb.clmb


ioslx1 clone ioslx1t \
    -dhd_tag trunk
# Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan
UTF::MacOS mc80tst7 -sta {MacX en0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan;} \
        -brand  "macos-internal-wl-gala" \
        -name "MacX-GalaGM" \
        -power {mc80npc2 1} \
        -type Debug_10_11 \
        -lan_ip 10.19.85.233 \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -console "/var/log/system.log" \
        -nobighammer 1 -tcpwindow 3640K -custom 1



MacX configure -attngrp L4
MacX clone MacX-GalaGM -name MacX-GalaGM -tag "BIS715GALA_REL_7_21_94_33" -custom 1
MacX clone MacX-GalaGMp -name MacX-GalaGMp -tag "BIS715GALA_REL_7_21_94_33" -custom 1
MacX clone MacX-GalaGM_22 -name MacX-GalaGM_22 -tag "BIS715GALA_REL_7_21_94_33" -custom 1
MacX clone MacX-GalaGM_25 -name MacX-GalaGM_25 -tag "BIS715GALA_REL_7_21_94_25" -custom 1
MacX clone MacX-DOME-ext -tag "BIS120RC4_REL_7_15_166_24" -custom 1 -brand "macos-external-wl-syr"
#MacX-GalaGM configure -ipaddr 192.168.1.243
#PMacX-Gala configure -ipaddr 192.168.5.236

UTF::MacOS mc80tst8 -sta {MacXG en0 PMacXG awdl0} \
    -wlinitcmds {Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-internal-wl-gala" \
        -name "Mac-Gala" \
        -power {mc80npc2 2} \
        -type Debug_10_11 \
        -kextload true \
        -slowassoc 5 \
        -lan_ip 10.19.85.234 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacXG configure -attngrp L4
MacXG clone MacX-Gala  -sta {MacX-Gala en0 PMacX-Gala awdl0} -name MacX-Gala -tag "BIS715GALA_REL_7_21_94_25" -custom 1
MacX-Gala clone MacX-Gala_22 -name MacX-Gala_22 -tag "BIS715GALA_REL_7_21_94_22" -custom 1
MacXG clone MacX-Gala-ext -tag "BIS715GALA_REL_7_21_94_125" -custom 1 -brand "macos-external-wl-gala"
#MacX-Gala configure -ipaddr 192.168.1.236
#PMacX-Gala configure -ipaddr 192.168.5.236

UTF::MacOS mc80tst5 -sta {MacAirX en0} \
    -wlinitcmds {Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-internal-wl-gala" \
        -name "MacAirX-GalaGM" \
        -power {mc80npc1 1} \
        -type Debug_10_11 \
        -kextload true \
        -lan_ip 10.19.85.231 \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacAirX configure -attngrp L5
MacAirX clone MacAirX-GalaGM -name MacAirX-GalaGM -tag "BIS715GALA_REL_7_21_94_33" -custom 1
MacAirX clone MacAirX-DOME-ext -tag "BIS120RC4_REL_7_15_166_24" -custom 1 -brand "macos-external-wl-syr"

UTF::MacOS mc80tst6 -sta {MacAirXG en0 PMacAirXG awdl0} \
    -wlinitcmds {Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-internal-wl-gala" \
        -name "MacAirX-Gala" \
        -power {mc80npc1 2} \
        -type Debug_10_11 \
        -lan_ip 10.19.85.232 \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacAirXG configure -attngrp L5
MacAirXG clone MacAirX-Gala  -sta {MacAirX-Gala en0 PMacAirX-Gala awdl0} -name MacAirX-Gala -tag "BIS715GALA_REL_7_21_94_25" -custom 1
MacAirXG clone MacAirX-Gala-ext -tag "BIS715GALA_REL_7_21_94_125" -custom 1 -brand "macos-external-wl-gala"
MacAirX-Gala configure -ipaddr 192.168.1.234
PMacAirX-Gala configure -ipaddr 192.168.5.234

# Common attributes across media and retail
# 4708ap configure -attngrp L4 ;#4708 no connect
4708ap configure -attngrp "" -lanpeer sflx2;#4708 no connect
4360mch5br1 configure -attngrp L5  ;#4708p
4360mch2br1 configure -attngrp L5  ;#4708p
4360br1 configure -attngrp L7 ;#4706
4360_2G configure -attngrp L7 ;#4706p
43602lx1 configure -attngrp L1
43602lx2 configure -attngrp L2
ioslx1 configure -attngrp L3

UTF::Linux mc80-lx1 -lan_ip 10.19.87.2 \
                    -sta {vlan170_if p21p1.170 mirror1 p17p1 mirror2 em1}
UTF::Linux mc80-lx2 -lan_ip 10.19.87.3 \
                    -sta {vlan171_if p21p1.171 wan em1.1000}
#UTF::Linux mc80-lx3 -lan_ip 10.19.87.4 \
#                    -sta {vlan173_if p21p1.173 vlan_a em1.2000 wired_a p21p1.1000}
UTF::Linux mc80-lx3 -lan_ip 10.19.87.4 \
                    -sta {vlan173_if p21p1.177 vlan_a em1.2000 wired_a p21p1.1000}
#UTF::Linux mc80-lx4 -lan_ip 10.19.87.5 \
#                    -sta {vlan177_if p1p1.177}
UTF::Linux mc80-lx4 -lan_ip 10.19.87.5 \
                     -sta {vlan177_if em1.177}
UTF::Linux mc80-lx5 -lan_ip 10.19.85.5 \
                    -sta {vlan177_intc em1.177}


vlan170_if configure -ipaddr 192.168.1.170
vlan171_if configure -ipaddr 192.168.1.171
vlan173_if configure -ipaddr 192.168.1.173
vlan177_if configure -ipaddr 192.168.1.177

#
set ARELEASE AARDVARK01T_REL_6_37_14_86
set BRELEASE AARDVARK01T_REL_6_37_14_105
set MAGNETOREL BISON04T_REL_7_14_89_42
set SVTOLD BISON04T_REL_7_14_32
set SVT BISON04T_REL_7_14_39
set MAGNETO89 BISON04T_REL_7_14_89
set TWIG89 BISON04T_TWIG_7_14_89
set BISON BISON_BRANCH_7_10
set TOT NIGHTLY
set TRAIN BISON05T_BRANCH_7_35 

# Brands to be tested
set BRANCHEXT linux26-external-vista-router-full-src
set MEXT linux26-external-router-media-full-src
set MINT linux26-internal-router-media
set MBASE linux26-internal-router
set MEXT $BRANCHEXT
set MINT $MBASE

4708ap clone 4708bisondhd -lanpeer sflx2 -tag $BISON -brand linux-2.6.36-arm-internal-router-dhdap -name AP-BSS
4708ap clone 4708bison -lanpeer sflx2 -tag $BISON -brand linux-2.6.36-arm-internal-router -name AP-BSS
4708ap clone 4708bisondhde -lanpeer sflx2 -tag $BISON -brand linux-2.6.36-arm-external-vista-router-dhdap-full-src -name AP-BSS
4708ap clone 4708bisone -lanpeer sflx2 -tag $BISON -brand linux-2.6.36-arm-external-vista-router-full-src -name AP-BSS
4708ap clone 4708bisont -lanpeer sflx2 -tag $MAGNETO89  -brand linux-2.6.36-arm-internal-router-dhdap -name AP-BSS
4708ap clone 4708btwig -lanpeer sflx2 -tag $TWIG89 -brand linux-2.6.36-arm-internal-router -name AP-BSS

#SVT verification
4708ap clone 4708m -lanpeer sflx2 -tag $MAGNETOREL -brand linux-2.6.36-arm-internal-router -name AP-BSS
4708ap clone 4708svt -lanpeer sflx2 -tag $SVT -brand linux-2.6.36-arm-internal-router -name AP-BSS
4360br1 clone 4708pm -tag $MAGNETOREL -brand linux-2.6.36-arm-internal-router -name AP-ProxBSS
4360mch5br1 clone 4708pstapm -tag $MAGNETO89 -brand linux26-internal-router -name PSTA-ProxBSS

# Known good release
set SVT $MAGNETOREL
4708ap clone ap_rel -lanpeer sflx2 -tag $SVT -brand linux-2.6.36-arm-internal-router -name AP-BSS-REL
4360br1 clone psta1_rel -tag $SVT -brand linux-2.6.36-arm-internal-router -name 4708-PSTA1-REL
4360mch5br1 clone psta3_rel -tag $SVT -brand linux-2.6.36-arm-internal-router -name 4708-PSTA3-REL

# BISON train release
4708ap clone ap_bt -lanpeer sflx2 -tag $TRAIN -brand linux-2.6.36-arm-internal-router -name AP-BSS
4360br1 clone psta1_bt -tag $TRAIN -brand linux-2.6.36-arm-internal-router -name 4708-PSTA1
4360mch5br1 clone psta3_bt -tag $TRAIN -brand linux-2.6.36-arm-internal-router -name 4708-PSTA3


#Tom private/base
4708ap clone ap_tom -lanpeer sflx2 -image /projects/hnd_swa_access_ext2/work/tomtang/igmp/src/router/arm-uclibc/linux.trx.veriwave-aes -name AP-BSS-REL
4360br1 clone psta1_tom -image /projects/hnd_swa_access_ext2/work/tomtang/igmp/src/router/arm-uclibc/linux.trx.veriwave-aes -name PSTA1
4360mch2br1 clone psta3_tom -image /projects/hnd_swa_access_ext2/work/tomtang/igmp/src/router/arm-uclibc/linux.trx.veriwave-aes -name PSTA2
# RJM FIX THIS, WHY discovering wrong?
# psta3_tom configure -sta "4360mch5br1 eth1 4360mch2br1 eth2"

# 7.14.43.39 base
set SVT BISON04T_REL_7_14_43_39
4708ap clone ap_tomb -lanpeer sflx2 -tag $SVT -brand linux-2.6.36-arm-internal-router -name AP-REL-43-39
4360br1 clone psta1_tomb -tag $SVT -brand linux-2.6.36-arm-internal-router -name 4708-PSTA1-REL-43-49
4360mch5br1 clone psta3_tomb -tag $SVT -brand linux-2.6.36-arm-internal-router -name 4708-PSTA3-REL-43-39
psta3_tomb configure -sta "4360mch5br1 eth1 4360mch2br1 eth2"


# 7.14.43.xx
set SVT BISON04T_REL_7_14_43*
4708ap clone ap_rel43 -lanpeer sflx2 -tag $SVT -brand linux-2.6.36-arm-internal-router -name AP-BSS-REL
4360br1 clone psta1_rel43 -tag $SVT -brand linux-2.6.36-arm-internal-router -name 4708-PSTA1-REL
4360mch5br1 clone psta3_rel43 -tag $SVT -brand linux-2.6.36-arm-internal-router -name 4708-PSTA3-REL


4708ap clone 4708m89bisondhd -lanpeer sflx2 -tag $MAGNETO89 -brand linux-2.6.36-arm-internal-router-dhdap -name AP-BSS
4708ap clone 4708m89bison -lanpeer sflx2 -tag $MAGNETO89 -brand linux-2.6.36-arm-internal-router -name AP-BSS
4360br1 clone 4708pm89 -tag $MAGNETO89 -brand linux-2.6.36-arm-internal-router -name AP-ProxBSS
4360mch5br1 clone 4706pm89 -tag $MAGNETO89 -brand linux26-internal-router -name PSTA-ProxBSS

4708ap clone 4708ap -lanpeer sflx2 -tag $ARELEASE -brand linux-2.6.36-arm-internal-router-dhdap -name AP-BSS
4360br1 clone 4708pa -tag $ARELEASE -brand linux-2.6.36-arm-internal-router -name AP-ProxBSS
4360mch5br1 clone 4706pa -tag $ARELEASE -brand linux26-internal-router -name PSTA-ProxBSS

4708ap clone 4708b -lanpeer sflx2 -tag $BRELEASE -brand linux-2.6.36-arm-internal-router -name AP-BSS
4360br1 clone 4708pb -tag $BRELEASE -brand linux-2.6.36-arm-internal-router -name AP-ProxBSS
4360mch5br1 clone 4708Bpb -tag $MAGNETO89 -brand linux-2.6.36-arm-internal-router -name PSTA-ProxBSS


proc loadall {ver} {
    set loadscript [UTFD::metascript %AUTO% -script "./UTF.tcl 4708$ver load -force; ./UTF.tcl 4706$ver load -force; ./UTF.tcl 4708p$ver load -force; ./UTF.tcl 4706p$ver load -force" -concurrent 1 -type now]
    while {![catch {$loadscript mystate} state] && $state ne "COMPLETED"} {
	UTF::Sleep 1 quiet
	puts -nonewline "."
	flush stdout
    }
    puts "\nLoad done"
    flush stdout
}

proc loadtom {which} {
    set loadscript [UTFD::metascript %AUTO% -script "./UTF.tcl ap_$which load -force; ./UTF.tcl psta1_$which load -force; ./UTF.tcl psta3_$which load -force" -concurrent 1 -type now]
    while {![catch {$loadscript mystate} state] && $state ne "COMPLETED"} {
	UTF::Sleep 1 quiet
	puts -nonewline "."
	flush stdout
    }
    puts "\nLoad done"
    flush stdout
}



L1 configure -default 0
L2 configure -default 0
L3 configure -default 0
L4 configure -default 0
L5 configure -default 0
L6 configure -default 0
L7 configure -default 0

set ::attnmax 95
set ::UTF::SetupTestBed {
    L1 attn $::attnmax
    L2 attn $::attnmax
    L3 attn $::attnmax
    L4 attn $::attnmax
    L5 attn $::attnmax
    L6 attn $::attnmax
    L7 attn $::attnmax

    set SECONDAP "4360R"
    set AP "NetG7K"
    set stressor "43602lx2"
    set macstas "MacX-GalaGM MacX-Gala MacAirX-GalaGM MacAirX-Gala"
    set linuxstas "43602lx1"
    set linuxdhdstas "ios_monarch ios_stowe"

    $AP attngrp attn 95
    $SECONDAP attngrp attn 95
    foreach STA $macstas {
	$STA power on
    }
    return
}

#
#        AccessPoint
#             |
#             | L1
#             |
#             +S1
#            /  \
#        L3 /    \L4
#          /      \
#         +S2     +S3
#        / \     / | \
#       /   \   /  |  \
#      H1   H2 H3  H4 H5
#
# +  = splitter/combiner
# Lx = variableattenuator leg
# Hx = Sta/Host
#
# When hidden node on
#      H1,H2 hidden from H3,H4 via ~(L2+L3+S1) attenuation
#      H1,H2 ~L2 from AP
#      H3,H4 ~L3 from AP
# When hidden node off
#      H1,H2,H3,H4 L1 from AP
#      H1,H2 ~S1 from H3,H4
#
# Test script example: Test/HiddenNode.test
#

proc ::txbeamforming {state {diff 0}} {
    L1 attn 5
    L2 attn 0
    L3 attn 12
    L4 attn 0
#    C7 attn 9
#    C8 attn 23
#    C9 attn 22
#    C7 attn 22
#    C8 attn 22
#    C9 attn 22
}

proc ::setrssi {value} {
    set l4adj [expr {$value - 48}]
    set l3adj [expr {$value - 49}]
    if {$l4adj > $l3adj} {
	set l1adj [expr {$l4adj - $l3adj}]
	set l4adj [expr {$l4adj - $l1adj}]
	set l3adj 0
    } elseif {$l4adj < $l3adj} {
	set l1adj [expr {$l3adj - $l4adj}]
	set l3adj [expr {$l3adj - $l1adj}]
	set l4adj 0
    } else {
	set l1adj $l3adj
	set l3adj 0
	set l4adj 0
    }
    L1 attn $l1adj
    L2 attn 0
    L3 attn $l3adj
    L4 attn $l4adj
    L1 attn 0
    L2 attn 0
    L3 attn 0
    L4 attn 10
#    L4 attn 10
#    L1 attn 10
#    L3 attn 0
#    L4 attn 0
}

# L1-L3 Lx1
# L4 4708ap
# L5-L7 Router
proc ::hidden_node {state} {
    set state [string tolower $state]
    switch -exact $state {
	"on" {
	    L1 attn 0
	    L2 attn 0
	    L3 attn 30
	    L4 attn 30
	    set ::hidden_node_state "ON"
	}
	"off" {
	    4708ap cget -attngrp
	    L1 attn 0
	    L2 attn 0
	    L3 attn 95
	    L4 attn 95
	    L5 attn 20
	    L6 attn 20
	    L7 attn 20
	    set ::hidden_node_state "OFF"
	}
	default {
	    error "unknown state $state - should be on | off"
	}
    }
}

proc ::balance {root args} {
    foreach dut [concat $root $args] {
	[$device cget -attngrp] configure -offset 0
	[$device cget -attngrp] attn 0
    }
    foreach device $args {
	set stream [UTF::stream %AUTO% -tx $root -rx $device -pktsize 1460 -reportinterval 0.025]
	$stream start
	if {![catch {$stream linkcheck -now}]} {
	    UTF::Sleep 0.25 quiet
	    set separation($device) [expr {abs([$device wl rssi])}]
	    if {![info exists furthest] || $separation($device) > $furthest} {
		set furthest $separation($device)
	    }
	}
	$stream destroy
	$device deinit
    }
    set msg "Best: $furthest "
    foreach device [array names separation] {
	set delta [expr {$furthest - $separation($device)}]
	append msg "${device}:$delta "
	if {$delta > 0} {
	    [$device cget -attngrp] configure -offset $delta
	} else {
	    [$device cget -attngrp] configure -offset 0
	}
	[$device cget -attngrp] attn 0
    }
    foreach device $args {
	set stream [UTF::stream %AUTO% -tx $root -rx $device -pktsize 1460 -reportinterval 0.025]
    }
    UTF::stream allstreams start
    UTF::Sleep 0.25 quiet
    foreach device $args {
	$device wl dump rssi
	$device wl rssi
    }
    UTF::stream allstreams destroy
    UTF::Message INFO "" $msg
}

proc ::console_logger_enable {{devices "vlan177_if vlan170_if vlan171_if vlan173_if"}} {
    foreach device $devices {
	catch {$device rexec systemctl start consolelogger.service}
    }
    foreach device $devices {
	catch {$device rexec systemctl status consolelogger.service}
    }
}

# array set ::ptpinterfaces [list sflx2 em1.10 vlan170_if p21p1.10 vlan171_if em1.10 vlan173_if em1.10 vlan177_if em1.10 43602lx1 em1 43602lx2 em1 43602brix1 p3p1]
array set ::ptpinterfaces [list sflx2 em1.10 vlan170_if p21p1.10 vlan171_if em1.10 vlan173_if em1.10 vlan177_if em1.10 43602lx1 em1 43602lx2 em1 ioslx1 p3p1 MacX-GalaGM en4 MacX-Gala en3 MacAirX-GalaGM en2 MacAirX-Gala en2]
proc ::enable_ptp {args} {
    if {![llength $args]} {
	set devices [array names ::ptpinterfaces]
    }  else {
	set devices $args
    }
    foreach dut $devices {
	if {[catch {$dut uname -r} err]} {
	    continue
	}
	set interface $::ptpinterfaces([namespace tail $dut])
	if {[$dut hostis MacOS]} {
	    catch {$dut rexec pkill ptpd2}
	    catch {$dut rexec ntpdate -b -p1 10.19.85.171}
	    catch {$dut rexec rm /var/log/ptp*.log}
	    catch {$dut rexec /usr/local/bin/ptpd2 -L -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface}
	    catch {$dut rexec route add -host 224.0.0.107 -interface $interface}
	    catch {$dut rexec route add -host 224.0.1.129 -interface $interface}
	} else {
	    catch {$dut rexec systemctl stop ntpd.service}
	    catch {$dut rexec ntpdate -b -p1 10.19.85.171}
	    catch {$dut rexec pkill ptpd2}
	    catch {$dut rm /var/log/ptp*.log}
	    catch {$dut rexec /projects/hnd_sig_ext16/rmcmahon/Code/ptp/ptpd-2.3.0/[$dut uname -r]/ptpd2 -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface}
	    catch {$dut ip route replace multicast 224.0.0.107/32 dev $interface}
	    catch {$dut ip route replace multicast 224.0.1.129/32 dev $interface}
	}
    }
    UTF::Sleep 120
    foreach dut $devices {
	catch {$dut lan rexec tail -10 /var/log/ptpstats.log}
	$dut deinit
    }
}
proc ::update_iperf {devices} {
    foreach DUT $devices {
	$DUT lan rexec  cp -f /projects/hnd_sig_ext16/rmcmahon/Code/iperf/sourceforge/git/commit/iperf2-code/src/iperf /usr/local/bin/iperf_custom
	# $DUT lan rexec  cp -f /projects/hnd_sig_ext16/rmcmahon/Code/iperf/iperf-2.0.5-improved/iperf/src/iperf /usr/local/bin/iperf_custom
	# $DUT lan rexec  cp -f /projects/hnd_sig_ext16/rmcmahon/Code/iperf/sourceforge/iperf2/iperf2-code-0/iperf2/src/iperf /usr/local/bin/iperf_custom
	catch { $DUT lan rexec  mv /usr/local/bin/iperf /usr/local/bin/iperf_original }
	$DUT lan rexec  ln -s /usr/local/bin/iperf_custom /usr/local/bin/iperf
    }
}

proc ::update_mac_tools {{devices {MacX-Gala MacAirX-Gala MacX-GalaGM MacAirX-GalaGM}}} {
    foreach DUT $devices {
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/iperf/iperf2-code/src/iperf /usr/bin/iperf]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/ptpd-2.3.1-rc3/src/ptpd2 /usr/bin/ptpd2]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/bt_tools/applebt /usr/local/bin/applebt]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/bt_tools/applebt /usr/bin/applebt]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/wget-1.16/src/wget /usr/bin/wget]}
	$DUT deinit
    }
}

proc softap {{sta 43602lx1dhdb} } {
    set (ap) 43602lx2
    set (sta) $sta
    set STAS $(sta)
    # Use SoftAP for case of STA/STA
    if {![$(ap) hostis Router]} {
	$(ap) configure -ap 1
	$(ap) configure -ipaddr 192.168.1.60
	$(ap) configure -ssid "SoftAP"
	if {![$(sta) hostis Router]} {
	    $(sta) configure -ipaddr 192.168.1.61
	    $(sta) configure -wlinitcmds "ifconfig [$(sta) cget -device] 192.168.1.61"
	}
	$(ap) configure -wlinitcmds "ifconfig [$(ap) cget -device] 192.168.1.60"
	set APMSG "(soft)AP"
    } else {
	if {[$(ap) nvram get wl_mode] ne "ap"} {
	    append testnvram($dut) "wl_mode=ap "
	}
    }
    foreach STA $STAS {
	catch {$STA lan tcptune 8M}
	catch {$STA lan iptables -F}
    }
    catch {$(ap) lan tcptune 8M}
    catch {$(ap) lan iptables -F}
    UTF::Test::APChanspec $(ap) 36/80
    UTF::Test::ConnectAPSTA $(ap) $(sta)
}

proc runpps {args} {
    UTF::Getopts {
	{sta.arg "43602lx1dhdb" "STA under test"}
	{rwin.arg "64K 32K 16K 8K 2K" "Read sizes"}
    }
    if {[regexp {43602lx1} $(sta)]} {
	set ap 43602lx2
    } else {
	set ap 43602lx1
    }
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/PerfPPS.test -title MC80-PPS -ap $ap -sta $(sta) -email {hnd-utf-list rmcmahon} -chanspec 36/80 -aggregation {1/1/1/64} -direction both -tos 0x0 -nounload -rwin [list $(rwin)] -utfconf zpl" -type now
}

proc run10G {{loop 1}} {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/Perf10G.test -web -utfconf zpl -i $loop" -type now
}


# You can refer to svn copy here
# (http://svn.sj.broadcom.com/viewvc/wlansvn/proj/branches/AARDVARK01T_TWIG_6_37_14/src/router/shared/defaults.c?view=markup )
# and look for " router_defaults_override_type1"
#
# For quick reference I have pasted the settings below.
#
# /* nvram override default setting for Media Router */
# struct nvram_tuple router_defaults_override_type1[] = {
#	{ "router_disable", "1", 0 },		/* lan_proto=static lan_stp=0 wan_proto=disabled */
#	{ "lan_stp", "0", 0 },			/* LAN spanning tree protocol */
#	{ "wl_wmf_bss_enable", "1", 0 },	/* WMF Enable for IPTV Media or WiFi+PLC */
#	{ "wl_reg_mode", "h", 0 },		/* Regulatory: 802.11H(h) */
#	{ "wl_taf_enable", "1", 0  },		/* Enable TAF */
#	{ "wl_taf_rule", "0x15", 0  },		/* Default TAF rule on SSID, RATE and AID */
#
#	/* EBOS feature Media router default */
#	{ "wl_ebos_enable", "0", 0 },		/* EBOS feature on */
#	{ "wl_ebos_flags", "0x68", 0 },		/* 104(0x68) video links */
#	{ "wl_ebos_transit", "-1", 0 },	         /* transit limit for video links */
#	{ "wl_ebos_prr_flags", "0xa41", 0 },	/* pseudo-round robin data links */
#	{ "wl_ebos_prr_threshold", "0x0f000000", 0 },	/* pseudo-round robin threshold */
#	{ "wl_ebos_prr_transit", "-1", 0 },	/* pseudo-round robin transit limit */
#
#	/* Airtime fairness */
#	{ "wl_atf", "1", 0 },		/* ATF feature on */
#
# #ifdef __CONFIG_EMF__
#	{ "emf_enable", "1", 0 },		/* Enable EMF by default */
#	{ "wl_wmf_ucigmp_query", "1", 0 },	/* Enable Converting IGMP Query to ucast */
#	{ "wl_wmf_ucast_upnp", "1", 0 },	/* Enable upnp to ucast conversion */
#	{ "wl_wmf_igmpq_filter", "1", 0 },	/* Enable igmp query filter */
# #endif
#	{ "wl_acs_fcs_mode", "1", 0 },		/* Enable acsd fcs mode */
#	{ "wl_acs_dfs", "2", 0 },		/* Enable first DFS chan Selection */
#	{ "wl_dcs_csa_unicast", "1", 0 },	/* Enable unicast CSA */
#	/* Exclude ACSD to select 140l, 144u, 140/80, 144/80 to compatible with Ducati 11N */
#	{ "wl_acs_excl_chans", "0xd98e,0xd88e,0xe28a,0xe38a", 0 },
#	{ "wl_pspretend_retry_limit", "5", 0 }, /* Enable PsPretend */
#	{ "wl_pspretend_threshold", "0", 0 },	/* Disable PsPretend Threshold */
#	{ "wl_acs_chan_dwell_time", "70", 0 },	/* WAR for AP to stay on DFS chan */
#	{ "wl_frameburst", "on", 0 },		/* BRCM Frambursting mode (off|on) */
#	{ "frameburst_dyn", "0", 0 },           /* Frameburst controlled dynamically if on */
#	{ "wl_amsdu", "off", 0 },		/* Disable AMSDU Tx by default */
#	{ "wl_rx_amsdu_in_ampdu", "off", 0 },	/* Disable AMSDU Rx by default */
#	{ "wl_cal_period", "0", 0 },			/* Disable periodic cal */
#	{ "wl_psta_inact", "0", 0 },		/* PSTA inactivity timer */
#	{ 0, 0, 0 }
# };
#

proc ::devicemode1 {device} {
    set wlname [$device wlname]
    set nvram "router_disable=0 "
    append nvram "wl_wmf_bss_enable=1 "
    append nvram "wl_reg_mode=h "
    append nvram "wl_taf_enable=1 "
    append nvram "wl_taf_enable=0x15 "
    append nvram "wl_ebos_enable=0 "
    append nvram "wl_ebos_flags=0x68 "
    append nvram "wl_ebos_prr_flags=0xa41 "
    append nvram "wl_ebos_prr_threshold=0x0f0000001 "
    append nvram "wl_ebos_prr_transit=-1 "
    append nvram "wl_atf=1 "
    append nvram "emf_enable=1 "
    append nvram "wl_wmf_ucigmp_query=1 "
    append nvram "wl_wmf_ucast_upnp=1 "
    append nvram "wl_wmf_igmpq_filter=1 "
    append nvram "wl_acs_fcs_mode=1 "
    append nvram "${wlname}_acs_fcs_mode=1 "
    append nvram "wl_acs_dfs=2 "
    append nvram "${wlname}_acs_dfs=2 "
    append nvram "wl_dcs_csa_unicast=1 "
    append nvram "${wlname}_dcs_csa_unicast=1 "
    append nvram "wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a "
    append nvram "${wlname}_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a "
    append nvram "wl_pspretend_retry_limit=5 "
    append nvram "${wlname}_pspretend_retry_limit=5 "
    append nvram "wl_pspretend_threshold=0 "
    append nvram "${wlname}_pspretend_threshold=0 "
    append nvram "wl_acs_chan_dwell_time=70 "
    append nvram "${wlname}_acs_chan_dwell_time=70 "
    append nvram "wl_frameburst=on "
    append nvram "${wlname}_frameburst=on "
    append nvram "frameburst_dyn=0 "
    append nvram "wl_amsdu=off "
    append nvram "${wlname}_amsdu=off "
    append nvram "wl_rx_amsdu_in_ampdu=off "
    append nvram "${wlname}_rx_amsdu_in_ampdu=off "
    append nvram "wl_psta_inact=0"
    append nvram "${wlname}_psta_inact=0 "
    append nvram "wl_cal_period=0 "
    append nvram "${wlname}_cal_period=0 "
    append nvram "wl_msglevel2=0x80000000 "
    append nvram "${wlname}_msglevel2=0x80000000 "
    append nvram "wl_msglevel=0x21280101 "
    append nvram "${wlname}_msglevel=0x21280101 "
    eval [concat $device restart $nvram]
}

set build NIGHTLY
4708ap clone 4708 -lanpeer sflx2 -tag $build -brand linux-2.6.36-arm-internal-router -name AP-BSS
4708ap clone 4708dhd -lanpeer sflx2 -tag $build -brand linux-2.6.36-arm-internal-router-dhdap -name AP-BSS
4360mch2br1 clone 4708pap -tag $build -brand linux-2.6.36-arm-internal-router -name PSTA-ProxBSS
4360br1 clone 4708p -tag $build -brand linux-2.6.36-arm-internal-router -name AP-ProxBSS

set build BISON04T_REL_7_14_89_33
4708ap clone 4708quickap -tag $build -brand linux-2.6.36-arm-internal-router -name 4708AP
4360br1 clone 4708quickpap -tag $build -brand linux-2.6.36-arm-internal-router -name 4708proxAP
4360mch5br1 clone 4708quicksta -tag $build -brand linux-2.6.36-arm-internal-router -name 4708-PSTA3-REL
4708quicksta configure -attngrp L5 ;#4706
4708quickpap configure -attngrp L7

set build $TRAIN
4708ap clone 4708tap -tag $build -brand linux-2.6.36-arm-internal-router -name 4708AP
4360br1 clone 4708tpap -tag $build -brand linux-2.6.36-arm-internal-router -name 4708proxAP
4360mch5br1 clone 4708tsta -tag $build -brand linux-2.6.36-arm-internal-router -name 4708-PSTA3-REL
4708tsta configure -attngrp L5 ;#4706
4708tpap configure -attngrp L7


set build EAGLE_BRANCH_10_10
4708ap clone ap_e -tag $build -brand linux-2.6.36-arm-internal-router -name 4708AP
4360br1 clone proxap_e -tag $build -brand linux-2.6.36-arm-internal-router -name 4708proxAP
4360mch2br1 clone psta_e -tag $build -brand linux-2.6.36-arm-internal-router -name 4708-PSTA3-REL
psta_e configure -attngrp L5 ;#4706
proxap_e configure -attngrp L7
43602lx1 clone 43602lx1e -tag $build
43602lx2 clone 43602lx2e -tag $build

4708ap clone NetG7K -tag BISON04T_REL_7_14_43_22 -brand linux-2.6.36-arm-internal-router -name NetG7KAP
NetG7K configure -attngrp L6

proc ptest {build} {
    UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708 -sta 4706 -proximateap 4708p -proximatesta 4706p -devicemode 1 -5g_rate auto -scansuppress -towards -staattn 60" -type now
    UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708dhd -sta 4706 -proximateap 4708p -proximatesta 4706p -devicemode 1 -5g_rate auto -scansuppress -towards -staattn 60" -type now
    
    UTFD::metascript proxtest -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708bison -sta 4706m89 -proximateap 4708pm89 -proximatesta 4706pm89 -devicemode 1 -5g_rate auto -scansuppress -towards -staattn 60" -type now -createonly 1
    
    UTFD::metascript proxtestb -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708m89bisondhd -sta 4706m89 -proximateap 4708pm89 -proximatesta 4706pm89 -devicemode 1 -5g_rate auto -scansuppress -towards -staattn 60" -type now -createonly 1
    
    UTFD::metascript proxtest2 -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708bisont -sta 4706m -proximateap 4708pm -proximatesta 4706pm -devicemode 1 -5g_rate auto -scansuppress -towards -staattn 60" -type now -createonly 1
    
    UTFD::metascript proxtest3 -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708btwig -sta 4706m -proximateap 4708pm -proximatesta 4706pm -devicemode 1 -5g_rate auto -scansuppress -towards -staattn 60" -type now -createonly 1
}

set ::UTFD::eagle {
    UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap ap_e -sta psta_e -proximateap proxap_e -proximatesta proxpsta_e -devicemode 1 -5g_rate auto -scansuppress -towards -staattn 60" -type now
    UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap ap_e -sta psta_e -proximateap proxap_e -proximatesta proxpsta_e -devicemode 1 -5g_rate auto -scansuppress -towards" -type now
}

set ::UTFD::mediascripts {
    foreach build "bison bisone bisondhd bisondhde" {
	::UTFD::metascript %AUTO% -watch 4708$build -script "/home/rmcmahon/UTF/svn/unittest/Test/FTTRBasic.test -title MC80-$build -ap 4708$build -sta 43602lx2 -email {hnd-utf-list rmcmahon} -tracelevel ampdu -load -testdefaults -chanspec 36/80 -multicast" -type triggered -watchinterval 600
	::UTFD::metascript %AUTO% -watch 4708$build -script "/home/rmcmahon/UTF/svn/unittest/Test/PerfPPS.test -title MC80-$build -ap 4708$build -sta 43602lx2 -email {hnd-utf-list rmcmahon} -chanspec 36/80 -aggregation {1/1/1/64 1/1/1/1} -direction both -tos {0x0 0xc0}" -type triggered -maxruntime 2 -type triggered -watchinterval 600
	::UTFD::metascript %AUTO% -watch 4708$build -script "/home/rmcmahon/UTF/svn/unittest/Test/MultiMediaMultiTV.test -title MC80-$build -ap 4708$build -wetaps {4360br1} -stepcount 18 -holdtime 15 -udprate 100M -security {open} -email {hnd-utf-list rmcmahon}" -type triggered -watchinterval 600
    }
}

set ::UTFD::nightly {
    ::UTFD::metascript %AUTO% -watch 4708 -script "/home/rmcmahon/UTF/svn/unittest/Test/MultiMediaMultiTV.test -title MC80-TOT -ap 4708 -wetaps {4708pap 4708p} -stepcount 18 -holdtime 15 -udprate 100M -security {open} -email {hnd-utf-list rmcmahon} -chanspec 161/80" -type triggered -watchinterval 600
    ::UTFD::metascript %AUTO% -watch ap_bt -script "/home/rmcmahon/UTF/svn/unittest/Test/MultiMediaMultiTV.test -title MC80-TRAIN -ap ap_bt -wetaps {psta1_bt psta2_bt psta3_bt} -stepcount 18 -holdtime 15 -udprate 100M -security {open} -email {hnd-utf-list rmcmahon} -chanspec 161/80" -type triggered -watchinterval 600
    ::UTFD::metascript %AUTO% -watch 4708 -script "/home/rmcmahon/UTF/svn/unittest/Test/FTTRBasic.test -title MC80-TOT -ap 4708 -sta 43602lx2 -email {hnd-utf-list rmcmahon} -tracelevel ampdu -load -testdefaults -chanspec 36/80 -multicast" -type triggered -watchinterval 600
    ::UTFD::metascript %AUTO% -watch 4708 -script "/home/rmcmahon/UTF/svn/unittest/Test/PerfPPS.test -title MC80-TOT -ap 4708 -sta 43602lx2 -email {hnd-utf-list rmcmahon} -chanspec 36/80 -aggregation {1/1/1/64 1/1/1/1} -direction both -tos {0x0 0xc0}" -type triggered -maxruntime 2 -type triggered -watchinterval 600
}

set ::UTFD::release {
    ::UTFD::metascript %AUTO% -watch 4708 -script "/home/rmcmahon/UTF/svn/unittest/Test/MultiMediaMultiTV.test -title MC80-REL -ap ap_rel -wetaps {psta1_rel psta2_rel psta3_rel} -stepcount 18 -holdtime 15 -udprate 100M -security {open} -email {hnd-utf-list rmcmahon} -chanspec 161/80" -type now
    ::UTFD::metascript %AUTO% -watch 4708 -script "/home/rmcmahon/UTF/svn/unittest/Test/MultiMediaMultiTV.test -title MC80-REL -ap ap_tom -wetaps {psta1_rel psta2_rel psta3_rel} -stepcount 18 -holdtime 15 -udprate 100M -security {open} -email {hnd-utf-list rmcmahon} -chanspec 161/80 -branch AARDVARK" -type now
}
set ACSD  {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708quickap -sta 4708quicksta -proximateap 4708quickpap -proximatesta 4706quickpsta -5g_rate auto -scansuppress -towards -devicemode 1 -proximatestrength {15 7} -proximatechanspec 144u -acifcs -disturbtime 2 -staattn 45" -type now
}
set ACSDTRAIN {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708tap -sta 4708tsta -proximateap 4708tpap -proximatesta 4706tpsta -5g_rate auto -towards -devicemode 1 -proximatestrength {15} -chanspec 149/80 -proximatechanspec 144u -acifcs -disturbtime 2 -staattn 45 -title ACS/FCS -disturbrate 40M" -type now
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708tap -sta 4708tsta -proximateap 4708tpap -proximatesta 4706tpsta -5g_rate auto -towards -devicemode 1 -proximatestrength {15} -chanspec 149/80 -proximatechanspec 144u -disturbtime 2 -staattn 45 -title {No ACS/FCS} -disturbrate 40M" -type now
}
set bk {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/ProximateBSS.test -utfconf zpl -ap 4708tap -sta 4708tsta -proximateap 4708tpap -proximatesta 4706tpsta -5g_rate auto -towards -devicemode 1 -proximatestrength {15} -chanspec 149/80 -proximatechanspec 144u -disturbtime 2 -staattn 45 -title {No ACS/FCS} -disturbrate 40M" -type background
}
set CSA {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/MediaCSA.test -title MC80-REL -ap ap_rel -sta psta1_rel -email {hnd-utf-list rmcmahon} -chanspec 161/80" -type now
}
set phycheck {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/PhyChanEst.test -utfconf zpl -ap ap_e -sta {43602lx2e 43602lx1e} -web -webtitle MC80-Eagle" -type now
}

set kpifp {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/FirstPacketSuite.test -utfconf zpl -ap ap_e -sta 43602lx2e" -type now -allowdups 1
}

set kpiscan {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPIScan.test -utfconf zpl -web -ap ap_e -sta 43602lx2e -stressor -msglevel '-scan'" -type now -allowdups 1
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPIScan.test -utfconf zpl -web -ap ap_e -sta 43602lx2e -stressor -msglevel '+scan'" -type now -allowdups 1
}

set airfair {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/AirFair.test -utfconf zpl -ap ap_e -sta {43602sta_AC 43602sta_11N}" -type triggered -allowdups 1
}

set kpisuite {
    ::UTFD::metascript %AUTO% -script "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPISuite.test -utfconf zpl -ap ap_e -sta 43602lx2e -secuirty " -type now -allowdups 1
}

proc closeall {} {
    foreach s [UTF::STA info instances] {
	$s deinit
    }
}

proc openrig {} {
    UTF::Message INFO "" "Connecting open air to rig"
    L3 attn 0
}

proc closerig {} {
    UTF::Message INFO "" "Closing open air from rig"
    L3 attn 95
}

