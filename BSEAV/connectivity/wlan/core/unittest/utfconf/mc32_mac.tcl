# -*-tcl-*-
#
# testbed configuration for MC32_mac testbed
# ====================================================================================================

source utfconf/mc32_dev.tcl

#####################################################################
# Hostname: 
# Platform: MacBook Air
# OS:       Dome
# Device:   4360X29c_P409
#####################################################################
# MacBook Air with Dome OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
### UTF::Power::Laptop TST3Power -button {web55 3}
###        -power {TST3Power} \
###        -power_button {auto} \

UTF::MacOS MacBook \
		-lan_ip 10.19.85.229 \
		-sta {MacX29c en0} \
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

# Clones for mc95tst3
MacX29c clone MacX29c-b715 -tag "BIS120RC4_BRANCH_7_15"
MacX29c-b715 clone MacX29c-b715_coex -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 ; wl btc_mode 1 }
MacX29c clone MacX29c-REL-BIS10RC4 -tag "BIS120RC4_REL_7_15_*" -custom 1
MacX29c clone MacX29c_coex -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 ; wl btc_mode 1 }
MacX29c-REL-BIS10RC4 clone MacX29c-REL-BIS10RC4_coex -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 ; wl btc_mode 1 }


BTCohost clone BTCohost_X29c\
	-lan_ip mc61end8\
    -bt_comm "usb0" \
	-bt_ds_location 0x00004000\
	-type BCM4360\
	-project_dir "/home/pkwan"\
	-version "BCM20702B0/*/v105" \
	-brand "Generic/USB"\
    -file *.cgr

#BT environment
# set ::bt_dut BTCohost_stella		;# BlueTooth device under test
# set ::bt_ref BLERef					;# BlueTooth reference board
set ::bt_ref BTRef					;# BlueTooth reference board
# set ::wlan_dut stella				;# HND WLAN device under test
# set ::wlan_rtr 4360_ToB				;# HND WLAN router
# set ::wlan_tg lan1					;# 4336 <-> 4335