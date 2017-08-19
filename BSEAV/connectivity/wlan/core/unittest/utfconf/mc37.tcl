#
# UTF configuration for testbed mc37
#

# The PCs & servers have been assigned static IP addresses by IT, which
# are visible in Unix NIS. This allows us to use DNS names in the
# UTF objects below.

# To get STA details, type: UTF.tcl <sta> whatami

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut 4329usb     ;# BlueTooth device under test
set ::bt_ref 2046usb     ;# BlueTooth reference board
set ::wlan_dut 4329sdio  ;# HND WLAN device under test
set ::wlan_rtr 4717      ;# HND WLAN router
set ::wlan_tg lan        ;# HND WLAN traffic generator

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT

# Miscellaneous setup.
set UTF::SummaryDir /projects/hnd_sig_ext2/$::env(LOGNAME)/mc37
UTF::Logfile "~/utf_test.log"
set UTF::controlchart_cmds {{4717 nvram show |grep antswitch}}

# Define power controllers on cart.
UTF::Power::WebRelay mc37rly1 -invertcycle "1"
UTF::Power::WebRelay mc37rly2 
UTF::Power::WebRelay mc37rly3 -invert "2 4"
UTF::Power::WebRelay mc37rly4
UTF::Power::WebRelay mc37rly5 -invertcycle "2"
UTF::Power::WebSwitch mc37ws1

# DUT1 desktop WLAN 4329 SDIO
UTF::DHD mc37tst1 \
    -sta "43342sdio eth0" \
    -power_button "mc37rly3 3"\
    -device_reset "mc37rly3 4"\
    -console "mc37tst3:40000" \
    -hostconsole "mc37tst3:40001" \
    -tag PHOENIX2_BRANCH_6_10 \
    -brand linux-internal-dongle \
    -type 43342a0min-roml/sdio-ag-idsup.bin \
    -driver dhd-cdc-sdstd \
    -nvram "src/shared/nvram/bcm943342fcagb.txt" \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {4717 wl dump ampdu}} \
    -pre_perf_hook {{4717 wl ampdu_clear_dump}}

# DUT1 4334 PKTFILTER
43342sdio clone 43342pkt \
    -type "43342a0min-roml/sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-sr-wapi-wl11d.bin"

# DUT1 WinXP laptop BlueTooth 4329 USB
UTF::WinBT mc37tst3 \
    -sta "43342usb" \
    -power_button "mc37rly3 1" \
    -device_reset "mc37rly3 2"\
    -bt_comm "com6@3000000"\
    -bt_comm_startup_speed 115200 \
    -user user\
    -bt_xtal_freq 37.4 \
    -bt_power_adjust 40 \
    -type BCM4334/B1 \
    -brand Generic/37_4MHz/fcbga \
    -bt_ss_location 0x00090000

# Items used for developing BTAMP Linux support.
# Provides enough environment for TC1 - 3 to work.
# 4329pkt clone 371btamp \
   -type "4329b1/sdio-g-cdc-reclaim-roml-dhdoid-btamp-idsup-idauth-minioctl.bin"
set ::btamp_dut 371btamp
set ::btamp_ref1 mc37tst7w
set ::btamp_ref2 5a
set ::mc37tst1_use_dhd 1
set ::mc37tst1a_use_dhd 1

set ::mc37tst1_macaddr "00904CC50034" 
set ::mc37tst1a_macaddr $::mc37tst1_macaddr
set ::mc37tst5a_macaddr "C44619A33BD3" 
set ::mc37tst7w_macaddr "904CE555C402"

# DUT2 WinXP laptop - unoccupied
UTF::WinBT mc37tst2 \
    -sta "mc372" \
    -power_button "mc37rly2 1" \
    -bt_comm "usb0"\
    -user user \

# DUT2 desktop - unoccupied
UTF::DHD mc37tst6 \
    -sta "mc376 eth1" \
    -power_button "mc37rly2 2" \
    -hostconsole "mc37tst2:40000" \
    -tag RAPTOR3_BRANCH_4_230

# 43224NIC in Sniffer
package require UTF::Sniffer
UTF::Sniffer mc37tst5 \
    -sta "sniffer eth0"\
    -power_button "mc37rly4 1"\
    -tag AARDVARK_BRANCH_6_30

# Sniffer as NIC
UTF::Linux mc37tst5a \
    -lan_ip mc37tst5 \
    -sta "sniffnic eth1" \
    -power_button "mc37rly4 1"\
    -tag AARDVARK_BRANCH_6_30

# WinXP laptop BT2046 Ref board, shared with WLAN.
UTF::WinBT mc37tst7 \
    -sta "2046usb" \
    -power_button "mc37rly5 1" \
    -power_sta "mc37rly5 2" \
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user

# WinXP laptop 43225NIC shared with BT Ref.
UTF::Cygwin mc37tst7w \
    -lan_ip mc37tst7 \
    -sta "43225nic" \
    -power_button "mc37rly5 1" \
    -user user \
    -tag KIRIN_BRANCH_5_100

43225nic configure -attngrp G3

# Traffic generators (no wireless cards)
UTF::Linux mc37tst4 \
    -sta "lan eth1" \
    -power "mc37ws1 1" \
    -power_button "auto"

# Aeroflex attenuator
package require UTF::Aeroflex
UTF::Aeroflex mc37att1 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4}}

# Attenuator power control
set ::rvr_attn_power "mc37ws1 2"

package require UTF::utils
set ::UTF::SetupTestBed {ALL attn 0; G3 attn 0;
    UTF::reset_all_bt_devices
}

# Linksys 320N 4717/4322 wireless router. Use external build
# for higher throughput. But we need ampdu data from router,
# so use internal build.
# NB: On Router, MUST have "ethN" after sta name!
UTF::Router router \
    -lan_ip 192.168.1.1 \
    -sta "4717 eth1" \
    -relay "mc37tst4" \
    -power "mc37rly1 1" \
    -lanpeer lan \
    -console "mc37tst4:40000" \
    -brand "linux-internal-router" \
    -tag "COMANCHE2_REL_5_22_90" \
    -date * \
    -nvram {
       fw_disable=1
       wl0_ssid=mc37_0
       wl1_ssid=mc37_1
       wl_msglevel=0x101
       antswitch=0
    }

set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl
set ::aux_lsf_queue sj-hnd

# set ::UTF::PostTestHook {
#     package require UTF::utils
#     UTF::do_post_test_analysis [info script] ""}

# Set testrig specific attenuation ranges for use with RvRNightly1.test
set ::cycle2G40AttnRange "30-85 85-30"
set ::cycle2G20AttnRange "30-85 85-30"

# Set testrig specific path loss for rvr1.test
set ::4329sdio_pathloss_2G 31 ;# B only
set ::4329pkt_pathloss_2G 31 ;# B only

set ::rvr_overall_timeout_min 50

# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""
