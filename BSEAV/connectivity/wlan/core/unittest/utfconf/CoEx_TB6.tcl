# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC24
# CoEx_TB6 testbed configuration
#
# TB6	4329-N18	
# mc24end1  10.19.85.195   Linux UTF Host   - TB6UTF
# mc24end2  10.19.85.196   DUT Linux        - TB6DUT
# mc24tst1  10.19.85.197   Vista_BtRef      - TB6BTREF
# mc24tst2  10.19.85.198   Vista_BtCoHost   - TB6BTCOHOST
# mc24tst3  10.19.85.199   web_relay_BtRef
# mc24tst4  10.19.85.200   web_relay_DUT
#           10.19.85.209   NPC_AP

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut BTCohost_4329USB      	;# BlueTooth device under test
set ::bt_ref BTRefXP      			;# BlueTooth reference board
set ::wlan_dut STA-Linux-4329sdio  	;# HND WLAN device under test
set ::wlan_rtr AP1-4322-4717   		;# HND WLAN router
set ::wlan_tg lan              		;# HND WLAN traffic generator


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
# UTF::Logfile "~/utf_test.log"
set UTF::SummaryDir "/projects/hnd_sig_ext3/$::env(LOGNAME)/CoExTB6"

# Define power controllers on cart.
# package require UTF::Power
# UTF::Power::Synaccess 5.1.1.105
# UTF::Power::Agilent ag1 -lan_ip 5.1.1.58 -voltage 3.3 -ssh ssh
# - btref webrelay 10.19.85.199 pwr for 2046B1 ref
UTF::Power::WebRelay 10.19.85.199
# - dut webrelay 10.19.85.200 bt & wl reset lines
UTF::Power::WebRelay 10.19.85.200
# AP pwr control & console
UTF::Power::Synaccess 10.19.85.209
UTF::Power::Synaccess 10.19.85.207

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.24.10 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

# UTF Endpoint - Traffic generators (no wireless cards)
# mc24end1  10.19.85.195   Linux UTF Host   - TB6UTF
# -sta "lan eth0"
UTF::Linux TB6UTF  -lan_ip 10.19.85.195  -sta "lan eth1"

#
# Chariot (Windows:Cygwin system)
# UTF::Cygwin TB5EP1 -lan_ip 192.168.1.156 -user user -sta {main}

# STA - FC9 desktop with 4325-N88 SDIO using Arasan SDIO card.
# mc24end2  10.19.85.196   DUT Linux        - TB6DUT
#
#   -brand linux-internal-dongle\
#   -customer "bcm" \
#   -type "4329b1/sdio-g-cdc-reclaim-idsup-apsta-roml-assoc.bin"
#
#    -brand linux-external-dongle-sdio\
#    -customer "olympic" \
#    -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-idsup-nocis-p2p-memredux16-pno-aoe-pktfilter-minioctl-proptxstatus-n18.bin"

UTF::DHD TB6DUT \
	-lan_ip 10.19.85.196 \
	-sta "STA-Linux-4329sdio eth1" \
	-tag "ROMTERM2_BRANCH_4_219" \
 	-power_sta "10.19.85.200 3" \
	-power "10.19.85.207 1" \
    -brand "linux-internal-dongle" \
    -customer "bcm" \
    -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup.bin" \
    -nvram   "bcm94329OLYMPICN18.txt" \
    -pre_perf_hook {{AP1-4322-4717 wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4322-4717 wl dump ampdu}}
# added pre_ and post_perf_hooks on 11/3/10, per JohnB's sample in MC46.tcl
# preferred nvram file location
#     -nvram   "src/shared/nvram/bcm94329OLYMPICN18.txt" 

# alternate nvram file location
#     -nvram "4329b1/bcm94329OLYMPICN18.txt"
    
# clones of TB6DUT
STA-Linux-4329sdio clone 4329sdio_ROMTERM3 -tag "ROMTERM3_BRANCH_4_220"
STA-Linux-4329sdio clone 4329sdio_RT3Ext -tag "ROMTERM3_BRANCH_4_220" -brand "linux-external-dongle-sdio"
STA-Linux-4329sdio clone 4329sdio_ROMTERM -tag "ROMTERM_BRANCH_4_218"


# BT Cohost - WinXP laptop BlueTooth 4325 USB
# -device_reset "10.19.85.182 1" -- Using webrelay mc21tst4  10.19.85.182   web_relay_DUT port 1 1NO
# mc24tst2  10.19.85.198   Vista_BtCoHost   - TB6BTCOHOST
# 	-device_reset "10.19.85.200 1"\
# The line below this must remain blank or you will get this msg: invalid command name "TB6BTCOHOST"!!

UTF::WinBT TB6BTCOHOST \
	-lan_ip 10.19.85.198 \
	-sta "BTCohost_4329USB" \
	-bt_comm "com3@3000000" \
	-power "10.19.85.207 2"\
   	-device_reset "10.19.85.200 1" \
	-bt_comm_startup_speed 115200 \
    -user user \
    -bt_xtal_freq 26 \
    -bt_power_adjust 40 \
    -type BCM4329/B1 \
    -brand Olympic_N18/Module \
    -bt_ss_location 0x00087410 \
    -version *0389.0000
    
#   -power "10.19.85.207 2"\

# BT Ref - WinXP laptop BlueTooth 2046 Reference
# mc24tst1  10.19.85.197   BtRef_XP     - TB6BTREF
#	-power_sta "10.19.85.199 1"\

UTF::WinBT BTREFXP \
	-lan_ip 10.19.85.197 \
    -sta "BTRefXP" \
    -type "BCM2046" \
	-power_sta "10.19.85.199 1"\
    -bt_comm "usb0" \
    -user user

#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router TB6AP1 \
    -lan_ip 192.168.1.1 \
    -sta "AP1-4322-4717 eth1" \
    -relay "TB6UTF" \
    -lanpeer lan \
    -console "10.19.85.195:40000" \
    -power "10.19.85.209 1"\
    -brand "linux-external-router-combo" \
    -tag "COMANCHE2_REL_5_22_\?\?" \
    -date * \
   -nvram {
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestCoex6
	wl0_channel=1
	w10_nmode=1
    }

# Specify router antenna selection to use
set ::wlan_rtr_antsel 0x01

