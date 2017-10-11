# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC54
# CoEx_TB10 testbed configuration
#
# mc54end1  10.19.87.21   Linux UTF Host   - TB10UTF
# mc54end2  10.19.87.22   DUT Linux        - TB10DUT
# mc54tst1  10.19.87.23   Vista_BtRef      - TB10BTREF
# mc54tst2  10.19.87.24   Vista_BtCoHost   - TB10BTCOHOST
# mc54tst3  10.19.87.25   web_relay_BtRef
# mc54tst4  10.19.87.26   web_relay_DUT
# mc54tst5  10.19.87.27   web_relay_DUT
# mc54tst6  10.19.87.28	  NPC_AP

source utfconf/mc54_base.tcl

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::Android 

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut BTCohost43430A1		      	;# BlueTooth device under test
set ::bt_ref mc54_BTRefXP      			;# BlueTooth 2046 reference board
# # # set ::bt_ref mc54_BLERef						;# BCM2070 board
set ::wlan_dut 43430a1_ToB_coex  		;# HND WLAN device under test
set ::wlan_rtr AP1-4717   				;# HND WLAN router
set ::wlan_tg lan0              		;# HND WLAN traffic generator

# loaned ip addresses from MC59:
# 10.19.87.102 -- bcm94334 host
# 10.19.87.101 -- BTCohost
# 10.19.87.100 -- webrelay DUT2

# UTF Endpoint - Traffic generators (no wireless cards)

#UTF::Linux mc54end1 -sta "lan0 eth1"
# -lan_ip 10.19.87.21  
#

UTF::Android mc54DUT \
	-relay mc54end1\
	-adbdevice 10.19.87.22 \
	-lan_ip "-s 10.19.87.22:5555 shell" \
    -sta {4361a wlan0} \
	-power {10.19.87.27 2} \
	-extsup 1 \
	-tag IGUANA_BRANCH_13_10 \
	-brand hndrte-dongle-wl \
    -dhd_brand android-external-dongle \
	-driver dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4-debug-3.10.20-vendorext-gcc264a6-androidx86 \
    -dhd_tag DHD_BRANCH_1_579 -app_tag trunk\
	-clm_blob ss_mimo.clm_blob \
    -nvram src/shared/nvram/bcm94357fcpagbe.txt \
    -type 4361a0-roml/config_pcie_release/rtecdc.bin \
    -wlinitcmds {
	dhd -i wlan0 msglevel -msgtrace;
	wl down;
	dhd -i wlan0 wdtick 10 ;
	dhd -i wlan0 dconpoll 10 ;
	wl vht_features 7
    } \
	-nocal 1 -datarate {-i 0.5 -auto -sgi} -udp 1.5G -tcpwindow 4m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
	-perfchans {36/80 3} -channelsweep {-band a} \
	-pre_perf_hook {{%S wl reset_cnts} {%S wl dump_clear ampdu}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl nrate} {%S wl counters}}

# UTF::DHD mc54DUT \
# 	-lan_ip 10.19.87.22 \
# 	-sta "dut_sdio eth0" \
# 	-power "10.19.87.27 2" \
# 	-console "mc54end2:40001"\
# 	-device_reset "10.19.87.25 2"\
# 	-tag "ROMTERM3_BRANCH_4_220" \
#     -brand "linux-internal-dongle" \
#     -customer "bcm" \
# 	-type "4329c0/sdio-g-cdc-roml-reclaim-full11n-wme-idsup-pool-assert.bin"\
#     -nvram "4329c0/bcm94329sdagb.txt" \
#     -pre_perf_hook {{AP1-4717 wl ampdu_clear_dump}} \
#     -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {AP1-4717 wl dump ampdu}}
#     
# # 4345 epa sdio BCM94345FCSAGB_1 rev 05   
#         
# dut_sdio clone 4345sdio\
#     -tag AARDVARK_BRANCH_6_30 \
#     -brand linux-mfgtest-dongle-sdio\
#     -nvram "src/shared/nvram/bcm94345fcsagb_epa.txt"\
#     -type 4345a0-roml/sdio-ag-mfgtest-seqcmds\
#     -dhd_tag TRUNK \
#     -dhd_brand linux-internal-dongle \
#     -slowassoc 5 -noaes 1 -notkip 1 \
#     -datarate {-i 0.5 -frameburst 1} \
#     -tcpwindow 1152k -udp 400m -nocal 1\
# 	-nopm1 1 -nopm2 1\
#     -wlinitcmds {wl vht_features 3}

# bcm943430wlselgs_26MHz P110 4/4/14

# UTF::WinBT MC54_BTCOHOSTXP\
# 	-lan_ip mc54tst2\
# 	-sta "mc54_BTCohost"\
# 	-power "10.19.87.27 1"\
# 	-device_reset "10.19.87.25 1"\
# 	-bt_comm "com9@3000000"\
# 	-bt_comm_startup_speed 115200\
#     -user user\
#     -bt_xtal_freq 26\
#     -bt_power_adjust 40\
#     -type BCM43291/A0\
#     -brand Generic/38_4MHz\
#     -bt_ss_location 0x00088000\
#     -version *

# experimental   
# unset UTF::TcpReadStats