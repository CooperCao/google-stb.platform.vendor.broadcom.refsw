#
# UTF configuration for KPI 1 rig
# Robert J. McMahon (setup next to my cube)
# October 2015
#
# IP Address range 10.19.87.1-10
#
# controlbyweb relay 2 172.16.2.181
# consoles: 172.176.2.174, 175
# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig_ext16/rmcmahon/kpi1"
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
}

set ::UTF::MSTimeStamps 1

UTF::Linux sfast-utf -lan_ip 10.19.85.171

UTF::WebRelay RamseyLx1 -lan_ip 172.16.2.184 -port 1
UTF::WebRelay ioslx1pwr -lan_ip 172.16.2.184 -port 2

UTF::Power::PicoPSU pwr_ap1 -lan_ip 172.16.2.195

UTF::Power::Synaccess mc80npc1 -lan_ip 172.16.1.20 -rev 1
UTF::Power::Synaccess mc80npc2 -lan_ip 172.16.1.21 -rev 1
UTF::Power::Synaccess pwr_ap2 -lan_ip 172.16.2.173

UTF::Sniffer SNIFHOST -lan_ip 10.19.85.230 -user root -tag BISON_BRANCH_7_10 -sta {SNIF enp3s0}

UTF::Linux sfast-lx2 -lan_ip 10.19.85.169 \
                    -sta {sflx2 enp1s0f1.177 sflx2-170 em2.170 10G-B em2.202}

sflx2 configure -ipaddr 192.168.1.42
sflx2-170 configure -ipaddr 192.168.1.52

UTF::AeroflexDirect AF-A -lan_ip 172.16.2.117 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 -group {L1 {1 2 3} L2 {4 5 6} L3 {7 8 9} L4 {10 11 12} C1 {1} C2 {2} C3 {3} C4 {4} C5 {5} C6 {6} C7 {7} C8 {8} C9 {9}}

# UTF::AeroflexDirect AF-A -lan_ip 172.16.2.117 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 -group {L1 {1 2 3} L2 {4 5 6} L3 {7 8 9} L4 {10 11 12} ALLC {1 2 3 4 5 6 7 8 9 10 11 12} C1 {1} C2 {2} C3 {3} C4 {4} C5 {5} C6 {6} C7 {7} C8 {8} C9 {9} C10 {10} C11 {11} C12 {12}}

# UTF::AeroflexDirect AF-B -lan_ip 172.16.2.116 -retries 0 -concurrent 0 -silent 0 -group {L5 {1 2 3} L6 {4 5 6} L7 {7 8 9} ALLD {1 2 3 4 5 6 7 8 9} D1 {1} D2 {2} D3 {3} D4 {4} D5 {5} D6 {6} D7 {7} D8 {8} D9 {9}}

UTF::AeroflexDirect AF-B -lan_ip 172.16.2.116 -retries 0 -concurrent 0 -silent 0 -group {L5 {1 2 3} L6 {4 5 6} L7 {7 8 9} D4 {4} D5 {5} D6 {6}}

UTF::Router KPI1-AP1 -name KPI1-AP1 \
    -sta {4708ap1 eth1 4331ap1 eth2} \
    -brand linux-2.6.36-arm-internal-router \
    -tag "EAGLE_BRANCH_10_10" \
    -power {pwr_ap1} \
    -relay "sflx2" \
    -lanpeer sflx2 \
    -lan_ip 192.168.1.5 \
    -console 10.19.85.169:40000 \
    -trx linux \
    -portmirror mirror1 \
    -bootwait 10 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -portmirror mirror1 \
    -nvram {
        wl_msglevel=0x101
        fw_disable=0
        lan_ipaddr=192.168.1.2
        wl0_ssid=4708ap1
        watchdog=6000
        lan_stp=0
    }

4708ap1 configure -attngrp L6

UTF::Router _KPI1-AP2 -name KPI1-AP2 \
    -sta {4360ap2 eth1 4360_2nd eth2} \
    -brand linux-2.6.36-arm-internal-router \
    -tag "EAGLE_BRANCH_10_10" \
    -power {pwr_ap2 1} \
    -bootwait 10 \
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

4360ap2 configure -attngrp L7

# Port Mirrors

UTF::Linux mc80-lx1 -lan_ip 10.19.87.2 \
                    -sta {vlan170_if p21p1.170 mirror1 p17p1 mirror2 em1}

# MAC Devices

UTF::MacOS mc80tst5 -sta {MacAir-A en0} \
    -wlinitcmds {Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-internal-wl-gala" \
        -name "MacAir-A" \
        -power {mc80npc1 1} \
        -type Debug_10_11 \
        -kextload true \
        -lan_ip 10.19.85.231 \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -udp 800M \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacAir-A configure -attngrp L5 -tag "BIS715GALA_REL_7_21_94_33" -custom 1

UTF::MacOS mc80tst6 -sta {MacAir-B en0 PMacAirXG awdl0} \
    -wlinitcmds {Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-internal-wl-gala" \
        -name "MacAir-B" \
        -power {mc80npc1 2} \
        -type Debug_10_11 \
        -lan_ip 10.19.85.232 \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -udp 800M \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacAir-B configure -attngrp L5 -tag "BIS715GALA_REL_7_21_94_25" -custom 1

UTF::MacOS mc80tst7 -sta {MacBook-A en0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan;} \
        -brand  "macos-internal-wl-gala" \
        -name "MacBook-A" \
        -power {mc80npc2 1} \
        -type Debug_10_11 \
        -lan_ip 10.19.85.233 \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -udp 1.2G \
        -nativetools 1 \
        -console "/var/log/system.log" \
        -nobighammer 1 -tcpwindow 3640K -custom 1



MacBook-A configure -attngrp L4

UTF::MacOS mc80tst8 -sta {MacBook-B en0 PMacXG awdl0} \
    -wlinitcmds {Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-internal-wl-gala" \
        -name "MacBook-B" \
        -power {mc80npc2 2} \
        -type Debug_10_11 \
        -kextload true \
        -slowassoc 5 \
        -lan_ip 10.19.85.234 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -udp 1.2G \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacBook-B configure -attngrp L4

############
if {0} {
    UTF::MacOS mc80tst10 -sta {MacMini-A en0 PMacXG awdl0} \
	-wlinitcmds {Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-internal-wl-gala" \
        -name "MacMini-A" \
        -power {mc80npc2 2} \
        -type Debug_10_11 \
        -kextload true \
        -slowassoc 5 \
        -lan_ip 10.19.87.7 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1

    MacMini-A configure -attngrp L4
}
############


# DHD Devices

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

# Bluetooth Objects

if {![catch {package require UTF::MacBT} err]} {
    UTF::BTSpeaker SpeakerA -address  04-88-e2-81-24-76 -attngrp "" -sta MacBook-A -btpath /usr/local/bin/applebt
    UTF::BTSpeaker SpeakerB -address 04-88-E2-88-E4-35 -attngrp "" -sta MacBook-B -btpath /usr/local/bin
} else {
    UTF::Message ERROR "" $err
}

# Linux objects

UTF::Linux itx-lx1 -lan_ip 10.19.87.10 -name cci-lx1\
    -sta {43602lx1 enp1s0} \
    -tag NIGHTLY \
    -power {pwr_43602lx1} \
    -udp 1.2G \
    -wlinitcmds {wl msglevel +assoc} \
    -brand "linux-internal-wl"

43602lx1 configure -attngrp L1

43602lx1 clone 4360SoftAP -name "4360SoftAP"
4360SoftAP configure  -ssid "4360SoftAP" -iperf /projects/hnd_sig_ext16/rmcmahon/Code/iperf/iperf2-code/src/iperf

UTF::Linux itx-lx2 -lan_ip 10.19.87.9 \
    -sta {43602lx2 enp1s0} \
    -power RamseyLx1 \
    -tag NIGHTLY  \
    -wlinitcmds {wl msglevel +assoc; wl btc_mode 0; wl mimo_bw_cap 1; wl ack_ratio 2; wl amsdu 1} \
    -udp 1.2G \
    -brand "linux-internal-wl"

43602lx2 configure -attngrp L2

UTF::Linux skylake-lx1 -lan_ip 10.19.87.6 \
    -sta {skylake eth1} \
    -tag NIGHTLY \
    -power {} \
    -udp 1.2G \
    -wlinitcmds {wl msglevel +assoc} \
    -brand "linux-internal-wl"

skylake configure -attngrp L3 -ipaddr 192.168.1.122

#################################################
# Olympic Latency

set franky {/projects/hnd_sw_mobhost/work/frankyl/timestamp-test/dhd-tx-ts.ko}
#set frankyfw {/projects/hnd_sw_mobhost/work/frankyl/timestamp-test/fw-timestamping}
set frankyfw {/projects/hnd_sw_mobhost/work/frankyl/timestamp-test/fw-timestamping-0.1}
set frankyfw2 {/projects/hnd_sw_mobhost/work/frankyl/timestamp-test/fw-timestamping-0.2}

set sdbdhd /projects/hnd_swbuild/build_linux/DHD_REL_1_579_53/linux-external-dongle-pcie/2016.5.19.0/release/bcm/host/dhd-msgbuf-pciefd-debug-3.11.1-200.fc19.x86_64/dhd.ko
set sdbfw /projects/hnd_swbuild/build_linux/IGUANA_REL_13_10_148/linux-external-dongle-pcie/2016.6.9.0/release/bcm/firmware/4357a0-ram/config_pcie_awdl.bin

UTF::DHD kpi14357DUTA \
	-lan_ip 10.19.87.8 \
	-sta {4357A eth0} \
    -name "4357A-DUT" \
    -nvram_add {macaddr=00:90:4C:12:D0:02} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4357a0.clm_blob \
    -type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl country US/0;
	wl band a;
	wl vht_features 7;
	wl -w 1 interface_create sta;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -perfchans {36/80} -channelsweep {-band a}

4357A configure -attngrp L2

4357A clone 4357At \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {} \
    -tag IGUANA_BRANCH_13_10

4357At clone 4357Ait -tag IGUANA_BRANCH_13_10 -iperf  /projects/hnd_sig_ext16/rmcmahon/Code/iperf/Latency/iperf2-code/src/iperf

4357Ait clone 4357Aifr \
  -dhd_image $franky \
  -nvram "bcm94357fcpagbe_p203.txt" \
  -clm_blob 4357a1.clm_blob \
  -type 4357a1-ram/config_pcie_release/rtecdc.bin

4357Ait clone 4357Aif \
  -dhd_image $franky

4357Ait clone 4357Aifts  \
    -clm_blob 4357a1.clm_blob \
    -image [list -dhd_image $franky $frankyfw] -udp enhanced


#-type 4357a0-ram/config_pcie_release/rtecdc.bin

UTF::DHD kpi14357DUTB \
	-lan_ip 10.19.87.9 \
	-sta {4357B eth0} \
    -name "4357B-DUT" \
    -nvram_add {macaddr=00:90:4C:12:D0:03} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4357a0.clm_blob \
    -type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl country US/0;
	wl band a;
	wl vht_features 7;
	wl -w 1 interface_create sta;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -perfchans {36/80} -channelsweep {-band a}

4357B configure -attngrp L3

4357B clone 4357Bt \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

4357Bt clone 4357Bit -tag IGUANA_BRANCH_13_10 -iperf  /projects/hnd_sig_ext16/rmcmahon/Code/iperf/Latency/iperf2-code/src/iperf

4357Bit clone 4357Bifr \
 -dhd_image $franky \
     -nvram "bcm94357fcpagbe_p203.txt" \
    -clm_blob 4357a1.clm_blob \
    -type 4357a1-ram/config_pcie_release/rtecdc.bin

4357Bit clone 4357Bif \
 -dhd_image $franky

4357Bit clone 4357Bifts  \
    -clm_blob 4357a1.clm_blob \
    -image [list -dhd_image $franky $frankyfw] -udp enhanced


4357Ait clone 4357Afw2  \
    -clm_blob 4357a1.clm_blob \
    -image [list -dhd_image $franky $frankyfw2] -udp enhanced

4357Bit clone 4357Bfw2  \
    -clm_blob 4357a1.clm_blob \
    -image [list -dhd_image $franky $frankyfw2] -udp enhanced


## SDB Clones
# http://www.sj.broadcom.com/projects/hnd_swbuild/build_linux/DHD_REL_1_359_163/linux-external-dongle-pcie/2016.5.18.1/release/bcm/apps/wl

4357A clone 4357A-SDB -image [list $sdbdhd $sdbfw] -iperf  /projects/hnd_sig_ext16/rmcmahon/Code/iperf/Latency/iperf2-code/src/iperf -app_tag DHD_REL_1_359_163 -app_date 2016.5.18.1 -app_brand linux-external-dongle-pcie -dhd_tag DHD_REL_1_359_163 -dhd_date 2016.5.18.1 -dhd_brand linux-external-dongle-pcie  -dhd_image $sdbdhd -wlinitcmds {wl down; wl country US/0; wl band b}

4357B clone 4357B-SDB -image [list $sdbdhd $sdbfw] -iperf  /projects/hnd_sig_ext16/rmcmahon/Code/iperf/Latency/iperf2-code/src/iperf -app_tag DHD_REL_1_359_163 -app_date 2016.5.18.1 -app_brand linux-external-dongle-pcie -dhd_tag DHD_REL_1_359_163 -dhd_date 2016.5.18.1 -dhd_brand linux-external-dongle-pcie -dhd_image $sdbdhd  -wlinitcmds {wl down; wl country US/0; wl band b}

4357A clone 4357A-SDB5G -image [list $sdbdhd $sdbfw] -iperf  /projects/hnd_sig_ext16/rmcmahon/Code/iperf/Latency/iperf2-code/src/iperf -app_tag DHD_REL_1_359_163 -app_date 2016.5.18.1 -app_brand linux-external-dongle-pcie -dhd_tag DHD_REL_1_359_163 -dhd_date 2016.5.18.1 -dhd_brand linux-external-dongle-pcie  -dhd_image $sdbdhd -wlinitcmds {wl down; wl country US/0; wl band a}

4357B clone 4357B-SDB5G -image [list $sdbdhd $sdbfw] -iperf  /projects/hnd_sig_ext16/rmcmahon/Code/iperf/Latency/iperf2-code/src/iperf -app_tag DHD_REL_1_359_163 -app_date 2016.5.18.1 -app_brand linux-external-dongle-pcie -dhd_tag DHD_REL_1_359_163 -dhd_date 2016.5.18.1 -dhd_brand linux-external-dongle-pcie -dhd_image $sdbdhd  -wlinitcmds {wl down; wl country US/0; wl band a}

## Adaptive power save
UTF::DHD kpi14359DUTB \
	-lan_ip 10.19.87.9 \
	-sta {4359B eth0} \
    -name "4359B-DUT" \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -tag DIN975T155RC31_BRANCH_9_87 \
    -dhd_tag DHD_BRANCH_1_579 \
    -app_tag DHD_BRANCH_1_579 \
    -nvram "bcm943596fcpagbss.txt" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
    -clm_blob "ss_mimo.clm_blob" \
    -wlinitcmds {wl down;} \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -perfchans {36/80} -channelsweep {-band a}

4359B configure -attngrp L3 -iperf /projects/hnd_sig_ext16/rmcmahon/Code/iperf/iperf2-code/src/iperf

4359B clone 4359 -type 43596a0-roml/config_pcie_mfg/rtecdc.bin

set heyoksan /projects/hnd_swbuild/USERS/hyeoksan/build_linux/DIN975T155RC31_BRANCH_9_87/hndrte-dongle-wl/2016.8.18.3/build/dongle/

4359B clone 4359ADSP -type 43596a0-roml/config_pcie_release/rtecdc.bin -image $heyoksan


######## AWDL Master (GO) ########
4357A-SDB clone 4357-AMaster-WLAN \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
    } \
	-name "4357-AMaster" \
	-sta {4357-AMaster-WLAN eth0 4357-AMaster-AWDL wl0.2}
#	-dhd_tag DHD_REL_1_579_53 \
#    -app_tag DHD_REL_1_359_163 \
 #   -type 4357a0-ram/config_pcie_awdl/rtecdc.bin	
4357-AMaster-WLAN configure -ipaddr 192.168.1.234    
4357-AMaster-AWDL configure -ipaddr 192.168.5.234
######################
4357-AMaster-WLAN clone 4357i-AMaster-WLAN \
	-sta {4357i-AMaster-WLAN eth0 4357i-AMaster-AWDL wl0.2} \
	-tag IGUANA_BRANCH_13_10
4357i-AMaster-WLAN configure -ipaddr 192.168.1.234    
4357i-AMaster-AWDL configure -ipaddr 192.168.5.234
######################



######## AWDL Slave (GC) ########
4357B-SDB clone 4357-ASlave-WLAN \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
    } \
	-name "4357-ASlave" \
	-sta {4357-ASlave-WLAN eth0 4357-ASlave-AWDL wl0.2} 
#	-dhd_tag DHD_REL_1_579_53 \
 #   -app_tag DHD_REL_1_359_163 \
 #   -type 4357a0-ram/config_pcie_awdl/rtecdc.bin	
4357-ASlave-WLAN configure -ipaddr 192.168.1.236    
4357-ASlave-AWDL configure -ipaddr 192.168.5.236 -attngrp L3
######################
4357-ASlave-WLAN clone 4357i-ASlave-WLAN \
	-sta {4357i-ASlave-WLAN eth0 4357i-ASlave-AWDL wl0.2} \
	-tag IGUANA_BRANCH_13_10
4357i-ASlave-WLAN configure -ipaddr 192.168.1.236    
4357i-ASlave-AWDL configure -ipaddr 192.168.5.236 -attngrp L2
######################

proc sdbscript {} {

    UTFD::metascript %AUTO% -type now -script "/home/rmcmahon/UTF/svn/unittest/Test/P2PQoSNightly.test -utfconf kpi1 -title {KPI1 4357 Iguana AWDL Only Video bi-dir Long Test AWDLCh=149} -sta_gc 4357i-ASlave-WLAN -sta_go 4357i-AMaster-WLAN -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '\[P2P:VI:BI:0:60\]' -p2p_chan 149 -multicore_mode 0 -awdl"

    UTFD::metascript %AUTO% -type now -script "/home/rmcmahon/UTF/svn/unittest/Test/P2PQoSNightly.test -utfconf kpi1 -title {SDB 4357 Iguana STA+AWDL UDP Video bi-dir Long Test SCC APCh=3 AWDLCh=3} -ap 4360ap2 -sta_gc 4357i-ASlave-WLAN -sta_go 4357i-AMaster-WLAN -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -qos_tests '\[WLAN:VI:BI:0:60\]\[P2P:VI:BI:0:60\]' -ap_chan 3 -p2p_chan 3 -multicore_mode 0 -awdl -noapload"
}

proc latency {} {
    UTFD::metascript %AUTO% -type now -script "/home/rmcmahon/UTF/svn/unittest/Test/udpperf.test -utfconf kpi1 -sta {4357A-SDB5G 4357B-SDB5G}"
}

# END SDB

#-type 4357a0-ram/config_pcie_release/rtecdc.bin

#################################################


# Console: 192.16.2.193 10001
UTF::Linux itx-lx3 -lan_ip 10.19.85.230 \
    -sta {SoftAP enp3s0} \
    -tag NIGHTLY \
    -ap 1 \
    -power {pwr_softap} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl vht_features 3} \
    -udp 1.2G \
    -brand "linux-internal-wl"

SoftAP configure  -ipaddr 192.168.1.70 -ssid "SoftAP"

# Sniffer

UTF::Sniffer SNIF -lan_ip 10.19.85.230 -user root -tag BISON_BRANCH_7_10 -sta {sta4360 enp3s0}

# Physical devices (no clones)

set ::UTFD::physicalmacstas {}
set ::UTFD::physicamacs {}
set ::UTFD::physicalrouterstas {}
foreach STA [UTF::STA info instances] {
    if {[$STA hostis Router]} {
	lappend ::UTFD::physicalrouterstas $STA
    }
}
foreach STA [UTF::MacOS info instances] {
    if {$STA eq "::AppleCore"} {
	continue
    } else {
	lappend ::UTFD::physicalmacs $STA
    }
}
foreach STA [UTF::STA info instances] {
    if {![$STA hostis MacOS]} {
	continue
    } else {
	lappend ::UTFD::physicalmacstas $STA
    }
}

### Setup test bed ######

set UTF::SetupTestBed {
    L1 attn $::attnmax
    L2 attn $::attnmax
    L3 attn $::attnmax
    L4 attn $::attnmax
    L5 attn $::attnmax
    L6 attn $::attnmax
    L7 attn $::attnmax
    foreach STA $::UTFD::physicalmacstas {
	set tag INFO
	if {[catch {$STA power on}]} {
	    set tag ERROR
	}
	UTF::Message $tag "" "AC power on for $STA"
    }
    foreach S [concat $::UTFD::physicalmacstas $::UTFD::physicalrouterstas] {
	catch {$S wl down}
	$S deinit
    }

    set sniffers [UTF::Sniffer info instances]
    foreach sniffer $sniffers {
	catch ($sniffer unload)
    }
    foreach DUT {4357A-SDB 4357B-SDB 4360ap2} {
	$DUT attngrp attn default
    }
    return
}


##################### PTP Configurations #######################################

# SNIF em1  vlan177_if em1.10
# array set ::ptpinterfaces [list sflx2 em1.10 MacBook-B en3 MacBook-A en3 MacAir-A en2 MacAir-B en2 vlan170_if p21p1.10 43602lx1 em1 43602lx2 em1 ioslx1 p3p1 SNIF em1]
array set ::ptpinterfaces [list sflx2 em1.10 MacBook-B en3 MacBook-A en3 MacAir-A en2 MacAir-B en2 vlan170_if p21p1.10 43602lx1 em1 43602lx2 em1 SNIF em1]
array unset ::ptpinterfaces *; array set ::ptpinterfaces [list 4357A p3p1 4357B p3p1 43602lx1 em1 sflx2 enp1s0f0.10]

proc ::enable_ptp {args} {
    UTF::Getopts {
	{wait.arg "120" "Time in seconds to let PTP run before sampling it's stats"}
    }

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
	if {![catch {$dut hostis MacOS}] && [$dut hostis MacOS] && ![catch {$dut uname -r}]} {
	    catch {$dut rexec pkill ptpd2}
	    catch {$dut rexec rm /var/log/ptp*.log}
	    catch {$dut rexec /usr/bin/ptpd2 -L -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface}
	    catch {$dut rexec route add -host 224.0.0.107 -interface $interface}
	    catch {$dut rexec route add -host 224.0.1.129 -interface $interface}
	} else {
	    catch {$dut rexec systemctl stop ntpd.service}
	    catch {$dut rexec pkill ptpd2}
	    catch {$dut rm /var/log/ptp*.log}
	    if {![$dut hostis DHD]} {
		catch {$dut rexec /projects/hnd_sig_ext16/rmcmahon/Code/ptp/ptpd-2.3.0/[$dut uname -r]/ptpd2 -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface}
	    } else {
		catch {$dut rexec /projects/hnd_sig_ext16/rmcmahon/Code/ptp/sourceforge/ptpd-code/src/ptpd2 -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface}
	    }
	    catch {$dut ip route replace multicast 224.0.0.107/32 dev $interface}
	    catch {$dut ip route replace multicast 224.0.1.129/32 dev $interface}
	}
    }
    UTF::Sleep $(wait)
    foreach dut $devices {
	catch {$dut rexec tail -10 /var/log/ptpstats.log}
	$dut deinit
    }
}

proc ::update_mac_tools {{devices ""}} {
    # set MACIPERF "/projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/iperf/iperf2-code/src/iperf"
    set MACIPERF "/projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/iperf208c/iperf2-code/src/iperf"
    if {$devices eq {}} {
	set devices $::UTFD::physicalmacstas
    }
    foreach DUT $devices {
	catch {eval [concat $DUT copyto $MACIPERF /usr/bin/iperf]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/iperf/iperf_awdl /usr/bin/iperf_awdl]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/ptpd-2.3.1-rc3/src/ptpd2 /usr/bin/ptpd2]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/bt_tools/applebt /usr/local/bin/applebt]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/bt_tools/applebt /usr/bin/applebt]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/wget-1.16/src/wget /usr/bin/wget]}
	$DUT deinit
    }
}

L1 configure -default 20
L2 configure -default 0
L3 configure -default 0
L4 configure -default 0
L5 configure -default 0
L6 configure -default 15
L7 configure -default 0

set ::attnmax 95

proc ::isalive {{devices ""}} {
    if {$devices eq {}} {
	foreach DUT [UTF::MacOS info instances] {
	    if {$DUT eq "::AppleCore"} {
		continue
	    } else {
		lappend devices $DUT
	    }
	}
    }
    foreach DUT $devices {
	if {[catch {$DUT os_version} results]} {
	    UTF::Message ERROR ISALIVE "$DUT [$DUT cget -lan_ip] not responding"
	} else {
	    UTF::Message OK ISALIVE "$DUT ([$DUT cget -lan_ip]) seems OK, OS Ver: $results"
	}
	$DUT deinit
    }
}

#  Source the logical configuration

source "utfconf/kpi1logical.tcl"
