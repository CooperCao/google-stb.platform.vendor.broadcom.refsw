# -*-tcl-*-
# ====================================================================================================

# TB7	4325-D1
# mc21end1  10.19.85.177   Linux UTF Host   - TB7UTF
# mc21end2  10.19.85.178   DUT Linux        - TB7DUT
# mc21tst1  10.19.85.179   Vista_BtRef      - TB7BTREF
# mc21tst2  10.19.85.180   Vista_BtCoHost   - TB7BTCOHOST
# mc21tst3  10.19.85.181   web_relay_BtRef
# mc21tst4  10.19.85.182   web_relay_DUT
#           192.168.1.207  web_relay_AP

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut BTCohost_4325USB      	;# BlueTooth device under test
set ::bt_ref BTRef_2046USB      	;# BlueTooth reference board
set ::wlan_dut STA-Linux-4325sdio  	;# HND WLAN device under test
set ::wlan_rtr AP1-4322-4717   		;# HND WLAN router
set ::wlan_tg lan              		;# HND WLAN traffic generator

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex
package require UTF::Linux

# SummaryDir sets the location for test results
#set UTF::SummaryDir /projects/hnd_sig/temp
#UTF::Logfile "~/utf_test.log"
set ::UTF::SummaryDir "/projects/hnd_sig_ext7/$::env(LOGNAME)/mc21"

# Define power controllers on cart.
# UTF::Power::Agilent ag1 -lan_ip 5.1.1.58 -voltage 3.3 -ssh ssh
UTF::Power::WebRelay 10.19.85.181
UTF::Power::WebRelay 10.19.85.182
UTF::Power::Synaccess 10.19.85.210
UTF::Power::Synaccess npc2 -lan_ip 172.16.1.23
UTF::Power::Synaccess npc4 -lan_ip 10.19.85.208
UTF::Power::Synaccess npc5 -lan_ip 172.16.1.52

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.16.1.33 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

UTF::Linux TB7UTF -lan_ip mc21end1 -sta "lan eth1"

UTF::DHD TB7DUT -lan_ip 10.19.85.178 -sta {STA-Linux-4325sdio eth1} \
    -tag NIGHTLY \
    -type "4325b0/sdio-g-cdc-reclaim-pool.bin" \
    -device_reset "10.19.85.182 2" \
    -power "10.19.85.208 1"\
    -hostconsole mc21end1:40001 \
    -console mc21end1:40002

#    -type "4325b0/sdio-g-cdc-reclaim-pool.bin" \
#    -type "4325b0/sdio-g-cdc-reclaim-idsup-wme.bin" \
#    -type "4325b0/sdio-g-cdc-reclaim-idsup-wme-pno-acn-aoe-toe.bin" \

# BT Cohost - WinXP laptop BlueTooth 4325 USB
UTF::WinBT TB7BTCOHOST \
    -lan_ip 10.19.85.180 \
    -sta "BTCohost_4325USB" \
    -bt_comm "com3@3000000"\
    -device_reset "10.19.85.182 1" \
    -power "10.19.85.208 2"\
    -bt_comm_startup_speed 115200 \
    -user user\
    -bt_xtal_freq 26 \
    -bt_power_adjust 40 \
    -type BCM4325/D1 \
    -brand "" \
    -bt_ss_location 0x00086800 \
    -file Generic_EPC_noFM/26MHz/Class1_5/*.cgs \
    -version *0262.0000

# BT Ref - WinXP laptop BlueTooth 2046 Reference
UTF::WinBT TB7BTREF \
	-lan_ip 10.19.85.179 \
    -sta "BTRef_2046USB" \
    -type "BCM2046"\
    -device_reset "10.19.85.181 1" \
    -bt_comm "usb0"\
    -user user

#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
UTF::Router TB7AP1 \
    -lan_ip 192.168.1.1 \
    -sta "AP1-4322-4717 eth1" \
    -relay "TB7UTF" \
    -lanpeer lan \
    -console "mc21end1:40000" \
    -power "npc5 1"\
    -brand "linux-external-router-combo" \
    -tag "MILLAU_REL_5_70_48_*" \
    -date * \
    -nvram {
        dhcp_start=192.168.1.3
        dhcp_end=192.168.1.120
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestRoaming
	wl0_channel=1
	antswitch=0
	wl0_radio=0
        wl0_obss_coex=0
    }

# Specify router antenna selection to use
#set ::wlan_rtr_antsel 0x01

set ::UTF::SetupTestBed {
    af setGrpAttn G1 0
    af setGrpAttn G2 0
    if {[catch {43224Win7 wl down} ret]} {UTF::Message ERROR "" "Error shutting down 43224Win7 radio"}
    if {[catch {43224FC9 wl down} ret]} {UTF::Message ERROR "" "Error shutting down 43224FC9 radio"}
    BTRef_2046USB device_reset
    BTCohost_4325USB device_reset
    unset ::UTF::SetupTestBed
    return
}
