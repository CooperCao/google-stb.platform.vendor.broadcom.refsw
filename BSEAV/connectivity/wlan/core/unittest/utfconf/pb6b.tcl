#
# UTF configuration for lab 2005 chamber PB6B
#
# Where is sr2end03 (PB6B)?
# sr2end03 is the 3rd one counting from the bottom on the 2nd server rack (the one on your left when facing wall) in Lab 2005.
#
# KVM access (login admin, passwd usual one, use IE, select port 3):
# http://sr2kvm01.sj.broadcom.com/home.asp

# To get STA details, type: UTF.tcl <sta> whatami

# Miscellaneous setup.
set UTF::SummaryDir /projects/hnd_sig_ext/$::env(LOGNAME)/pb6b
UTF::Logfile "~/utf_test.log"
# set UTF::controlchart_cmds {{4717_1 nvram show |grep antswitch} {4717_2 nvram show |grep antswitch}}

# Define power controllers inside phone booth
set power_ctl pb6bips1 
UTF::Power::Synaccess $power_ctl
UTF::Power::WebRelay pb6brly1 -invertcycle "1"
UTF::Power::WebRelay pb6brly2 -invertcycle "1"
UTF::Power::WebRelay pb6brly3
UTF::Power::WebRelay pb6btstb
UTF::Power::WebSwitch pb6bws1

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl nrate}}}

# DUT1 Dell E6400 Win7 Laptop
UTF::WinDHD pb6btst1 \
    -osver 7 \
    -sta "tba1" \
    -power_button "$power_ctl 6" \
    -power_sta "$power_ctl 7" \
    -console "sr2end03:40009"\
    -user user \
    -tag AARDVARK_BRANCH_6_30 \
    -brand win_internal_wl \
    -type checked/DriverOnly_BMac \
    -sys bcmwlhigh6.sys \
    -wlinitcmds {wl mpc 0} \
    -post_perf_hook {{%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}

# DUT2 Dell E6400 F15 laptop with 43224NIC
UTF::Linux pb6btst2 \
    -sta "43224nic eth0 43224nic.0 wl0.1" \
    -power_button "$power_ctl 1" \
    -console "sr2end03:40006"\
    -tag AARDVARK_BRANCH_6_30 \
    -brand "linux-internal-wl" \
    -type obj-debug-p2p-mchan \
    -wlinitcmds {wl mpc 0} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S wl dump ampdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}}

# DUT3 HP 6910P Laptop with WinXP & 43231 bmac dongle
UTF::WinDHD pb6btst3 \
    -sta "43231xp" \
    -power_button "$power_ctl 3" \
    -power_button_pushes 2 \
    -power_sta "$power_ctl 4" \
    -console "sr2end03:40007"\
    -user user \
    -tag AARDVARK_BRANCH_6_30 \
    -brand win_internal_wl \
    -type checked/DriverOnly_BMac \
    -sys bcmwlhigh5.sys \
    -noafterburner 1 \
    -wlinitcmds {wl mpc 0} \
    -post_perf_hook {{%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}\
    -tcptuneserver 1

# Pkteng works on mfgtest build
43231xp clone 43231xpmfg -brand win_mfgtest_wl -type Bcm/Bcm_DriverOnly_BMac

# DUT4 COEX laptop XP BT holding place
# UTF::WinBT pb6btst4 \
    -sta xxxxusb \
    -power_button "$power_ctl 5" \
    -user user \
    -bt_comm "com3@3000000"\
    -bt_comm_startup_speed 115200 \

# DUT4 COEX desktop F9 SDIO holding place
# UTF::DHD pb6btst8 \
    -sta "xxxsdio eth1" \
    -tag RAPTOR3_BRANCH_4_230\

# E4200 XP laptop BT2046 Ref board, shared with WLAN.
# UTF::WinBT pb6btst5 \
    -sta "2046usb" \
    -power_button "m 1" \
    -power_sta "m 2" \
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user

# E4200 XP laptop 43225NIC shared with BT2046.
# UTF::Cygwin pb6btst5w \
    -lan_ip pb6btst5 \
    -sta "43225nic" \
    -power_button "m 1" \
    -user user \
    -tag KIRIN_BRANCH_5_100

# Traffic generators (no wireless cards)
UTF::Linux sr2end03 \
    -sta "lan1 eth1"
lan1 configure -ipaddr 192.168.1.130

UTF::Linux sr2end04 \
    -sta "lan2 eth1"
lan2 configure -ipaddr 192.168.1.131

# 4322NIC as Sniffer
package require UTF::Sniffer
UTF::Sniffer pb6btst7 \
    -sta "sniffer eth0"\
    -power_button "pb6btstb 1"\
    -tag AARDVARK_BRANCH_6_30

# Sniffer as NIC
UTF::Linux pb6btst7nic \
    -lan_ip pb6btst7 \
    -sta "sniffnic eth0" \
    -power_button "pb6btstb 1" \
    -tag AARDVARK_BRANCH_6_30

# Aeroflex attenuator
package require UTF::Aeroflex
UTF::Aeroflex pb6batt1 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

# Attenuator power control is on pb6bws1 port 1
set ::rvr_attn_power "pb6bws1 1"

package require UTF::utils
set ::UTF::SetupTestBed {ALL attn 0;
    # Make sure AP radios are off, scripts will choose AP to use
    UTF::set_ap_nvram 4717_1 wl0_radio=0 wl1_radio=0
    UTF::set_ap_nvram 4717_2 wl0_radio=0 wl1_radio=0
}
# set ::UTF::SetupTestBed ""

# Linksys 320N 4717 wireless router. Use external build for higher throughput.
# NB: On Router, MUST have "ethN" after sta name!
# BUT: We needs internal build to get ampdu stats for 4319!
UTF::Router router1 \
    -lan_ip 192.168.1.1 \
    -sta "4717_1 eth1" \
    -power "pb6brly1 1" \
    -power_button "auto"\
    -relay "sr2end03" \
    -console "sr2end03:40020" \
    -lanpeer "lan1" \
    -tag AKASHI_REL_5_110_?? \
    -brand "linux26-internal-router" \
    -nvram {
       fw_disable=1
       wl0_ssid=pb6b_0
       wl_msglevel=0x101
       antswitch=0
    }

4717_1 clone 4717_1gzip -brand "linux26-internal-router" -trx linux-gzip

# For AP - AP tests, one router must have WET package, usually in the combo build.
# Its best if endpoints have static IP addresses.
4717_1 clone 4717wet \
    -brand "linux-internal-router-media" \
    -nvram {
       fw_disable=1
       wl0_ssid=pb6b_2
       wl_msglevel=0x101
       antswitch=0
       wl0_mode=wet
       router_disable=1
    }

# Linksys 320N 4717 wireless router. Use external build for higher throughput.
# NB: On Router, MUST have "ethN" after sta name!
UTF::Router router2 \
    -lan_ip 192.168.1.2 \
    -sta "4717_2 eth1" \
    -power "pb6brly2 1" \
    -power_button "auto"\
    -relay "sr2end04" \
    -console "sr2end03:40021" \
    -lanpeer "lan2" \
    -tag AKASHI_REL_5_110_?? \
    -brand "linux-internal-router" \
    -nvram {
       fw_disable=1
       wl0_ssid=pb6b_2
       wl_msglevel=0x101
       antswitch=0
       dhcp_start=192.168.1.150
       dhcp_end=192.168.1.200
       macaddr=00:90:4C:2F:0B:11
    }


# Set testrig specific attenuation ranges for use with RvRNightly1.test
set ::cycle5G40AttnRange "15-65 65-15"
set ::cycle5G20AttnRange "15-65 65-15"
set ::cycle2G40AttnRange "10-75 75-10"
set ::cycle2G20AttnRange "10-75 75-10"

set ::rvr_overall_timeout_min 50

# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""

set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl
set ::aux_lsf_queue sj-hnd
# set ::aux_sge_queue mc46

# Dummy object to test Apple rtr with RvR
# package require UTF::Airport
# UTF::Airport air1 \
    -lan_ip 192.168.1.1 \
    -sta "apple eth1"\
    -power "pb6brly1 1" \
    -lanpeer lan1 \
    -nvram wl0_ssid="pb6b_0"

