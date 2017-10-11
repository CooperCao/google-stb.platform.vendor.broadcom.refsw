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
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc54"

#package require UTF::Android

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut BTCohost43430A1		      	;# BlueTooth device under test
set ::bt_ref mc54_BTRefXP      			;# BlueTooth 2046 reference board
# # # set ::bt_ref mc54_BLERef						;# BCM2070 board
set ::wlan_dut 43430a1_ToB_coex  		;# HND WLAN device under test
set ::wlan_rtr AP1-4717   				;# HND WLAN router
set ::wlan_tg lan0              		;# HND WLAN traffic generator

set ::UTF::SetupTestBed {
	
  	G1 attn 30 ;# changed to 35 from 20 8/21/13
	G2 attn 0 ;# changed to 23 from 15 8/21/13 ;#changed from 22 to 15 on 3/2/12 for debugging
	G3 attn 103

	# turn off 4334 radio
	if {[catch {4334sdio wl down} ret]} {
        error "SetupTestBed: 4334sdio wl down failed with $ret"
    }

    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# loaned ip addresses from MC59:
# 10.19.87.102 -- bcm94334 host
# 10.19.87.101 -- BTCohost
# 10.19.87.100 -- webrelay DUT2

# UTF Endpoint - Traffic generators (no wireless cards)

#UTF::Linux mc54end1 -sta "lan0 eth1"
# -lan_ip 10.19.87.21  
#

# 4357a0/4361a0 Olympic Guinness module

UTF::DHD mc54DUT \
	-lan_ip mc50tst4 \
    -sta {4357 eth0} \
	-power {10.19.87.27 2} \
	-tag IGUANA_BRANCH_13_10 \
	-brand hndrte-dongle-wl \
    -dhd_brand linux-external-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
    -dhd_tag trunk -app_tag trunk\
	-clm_blob 4347a0.clm_blob \
    -nvram src/shared/nvram/bcm94357GuinnessMurataMM.txt \
    -type 4357a0-ram/config_pcie_release/rtecdc.bin \
    -wlinitcmds {
		dhd -i eth0 msglevel -msgtrace;
		wl down;
		dhd -i eth0 wdtick 10 ;
		dhd -i eth0 dconpoll 10 ;
		wl vht_features 3
    } \
	-noaes 1 -nocal 1 -datarate {-i 0.5 -auto -sgi} -udp 400m -tcpwindow 1152k -slowassoc 5 \
	-perfchans {36l 3} \
	-pre_perf_hook {{%S wl reset_cnts} {%S wl dump_clear ampdu}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl nrate} {%S wl counters}}
	
4357 clone 4357_tigris -tag IGUANA1310R307_BRANCH_13_20
4357 clone 4357b0 \
	-type 4357b0-roml/config_pcie_release/rtecdc.bin \
	-nvram src/shared/nvram/bcm94357GuinnessMurataMM.txt \
	-clm_blob 4357.clm_blob \
	-dhd_tag DHD_BRANCH_1_579 -app_tag APPS_REL_1_*
4357b0 clone 4357b0_tigris -dhd_tag DHD_BRANCH_1_579 -app_tag APPS_REL_1_*
# tigris efforts have migrated to 13.10, 13.20 is no longer target for release
#4357b0 clone 4357b0_tigris -tag IGUANA1310R307_BRANCH_13_20
# 	-clm_blob 4347b0.clm_blob \

# 4357/4361 chip

UTF::WinBT MC54_BTCOHOSTXP\
	-lan_ip mc54tst2\
	-sta "mc54_BTCohost"\
	-power "10.19.87.27 1"\
	-device_reset "10.19.87.25 1"\
	-bt_comm "com14@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -type BCM4347\
    -brand Generic/UART/37_4MHz/fcbga_ref_dLNA\
    -bt_ss_location 0x00215614\
    -version BCM4347A0*\
    -project_dir "/home/pkwan"\
    -file *.cgr

mc54_BTCohost clone BTCohost_4357
BTCohost_4357 clone BTCohost_4357_BU -brand Generic/UART/37_4MHz/fcbga_BU_dLNA
# use this for Guinness module
BTCohost_4357 clone BTCohost_4357_Olympic\
    -brand Generic/UART/37_4MHz/Olympic/fcbga_dLNA -file *.hcd
BTCohost_4357 clone BTCohost_4357b0 -version BCM4347B0*
BTCohost_4357b0 clone BTCohost_4357b0_Olympic\
    -brand Generic/UART/37_4MHz/Olympic/fcbga_dLNA -file *.cgr
    
# experimental
unset UTF::TcpReadStats