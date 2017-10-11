# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC54
# CoEx_TB10 testbed configuration
#
# TB10	4329C0	
# mc54end1  10.19.87.21   Linux UTF Host   - TB10UTF
# mc54end2  10.19.87.22   DUT Linux        - TB10DUT
# mc54tst1  10.19.87.23   Vista_BtRef      - TB10BTREF
# mc54tst2  10.19.87.24   Vista_BtCoHost   - TB10BTCOHOST
# mc54tst3  10.19.87.25   web_relay_BtRef
# mc54tst4  10.19.87.26   web_relay_DUT
# mc54tst5  10.19.87.27   web_relay_DUT
# mc54tst6  10.19.87.28	  NPC_AP

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut mc54_BTCohost		      	;# BlueTooth device under test
set ::bt_ref mc54_BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 43291sdio  			;# HND WLAN device under test
set ::wlan_rtr AP1-4717   				;# HND WLAN router
set ::wlan_tg lan0              		;# HND WLAN traffic generator

# loaned ip addresses from MC59:
# 10.19.87.102 -- bcm94334 host
# 10.19.87.101 -- BTCohost
# 10.19.87.100 -- webrelay DUT2


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
# UTF::Logfile "~/utf_test.log"
### set UTF::SummaryDir "/projects/hnd_sig_ext3/$::env(LOGNAME)/mc54"
# moved to dedicated filer volume 10/30/13
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc54"

# Define power controllers on cart.
# package require UTF::Power
# UTF::Power::Synaccess 5.1.1.105
# UTF::Power::Agilent ag1 -lan_ip 5.1.1.58 -voltage 3.3 -ssh ssh
# - btref webrelay 10.19.85.199 pwr for 2046B1 ref
UTF::Power::WebRelay 10.19.87.25 ;# Enclosure 1: DUT and BTCohostXP
# - dut webrelay 10.19.85.200 bt & wl reset lines
UTF::Power::WebRelay 10.19.87.26
UTF::Power::WebRelay 10.19.87.110 ;# webrelay DUT2

# AP pwr control & console
UTF::Power::Synaccess 10.19.87.27 ;# Encloser 1: DUT and BTCohostXP
UTF::Power::Synaccess 10.19.87.28
UTF::Power::Synaccess 172.16.54.25 ;# npc_DUT2


# Attenuator
UTF::Aeroflex af -lan_ip 172.16.54.10 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
	
  	G1 attn 20
	G2 attn 0 ;# changed from 10 on 1/4/12 as an experiment
	G3 attn 103
#     ALL attn 0

	# turn off dut_sdio radio
	if {[catch {dut_sdio wl down} ret]} {
        error "SetupTestBed: dut_sdio wl down failed with $ret"
    }
    
    # experimental
	unset UTF::TcpReadStats
    	
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# mc24end1  10.19.85.195   Linux UTF Host   - TB6UTF
# -sta "lan eth0"

UTF::Linux mc54end1 -sta "lan0 eth1"
# -lan_ip 10.19.87.21  
#

UTF::DHD mc54DUT \
	-lan_ip 10.19.87.22 \
	-sta "dut_sdio eth0" \
	-power "10.19.87.27 2" \
	-console "mc54end2:40001"\
	-device_reset "10.19.87.25 2"\
	-tag "ROMTERM3_BRANCH_4_220" \
    -brand "linux-internal-dongle" \
    -customer "bcm" \
	-type "4329c0/sdio-g-cdc-roml-reclaim-full11n-wme-idsup-pool-assert.bin"\
    -nvram "4329c0/bcm94329sdagb.txt" \
    -pre_perf_hook {{AP1-4717 wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4717 wl dump ampdu}}
    
UTF::WinBT MC54_BTCOHOSTXP\
	-lan_ip 10.19.87.24\
	-sta "mc54_BTCohost"\
	-power "10.19.87.27 1"\
	-device_reset "10.19.87.25 1"\
	-bt_comm "com5@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -type BCM43291/A0\
    -brand Generic/38_4MHz\
    -bt_ss_location 0x00088000\
    -version *

mc54_BTCohost clone BTCohost_Nok -brand Nokia/Module_TDK -version *
# mc54_BTCohost clone BTCohost_new -version *
mc54_BTCohost clone BTCohost_new -version *111.0000
mc54_BTCohost clone BTCohost_latest -version * -file *.cgs


UTF::WinBT MC54_BTREFXP\
	-lan_ip 10.19.87.23\
    -sta "mc54_BTRefXP"\
	-device_reset "10.19.87.26 1"\
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user

# 4334a2 installed 8/16/2011: sta=mc59end8; # upgraded to FC15 2/20/14
# changed to 4334B1 as of 11/10/2011
		
UTF::DHD mc54DUT2 \
	-lan_ip 10.19.87.102 \
	-sta "4334sdio eth0" \
	-power "172.16.54.25 2" \
	-power_sta "172.16.54.25 1"\
	-device_reset "10.19.87.110 1"\
 	-console "mc59end8:40000"\
	-tag "PHOENIX_BRANCH_5_120" \
    -brand "linux-internal-dongle" \
    -customer "bcm" \
	-type "4334a0-roml/sdio-ag-pool-idsup*.bin"\
    -nvram "src/shared/nvram/bcm94334fcagb.txt"\
    -postinstall {dhd -i eth0 serial 1}\
    -pre_perf_hook {{AP1-4717 wl ampdu_clear_dump}}\
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {AP1-4717 wl dump ampdu}}\
    -wlinitcmds {wl phy_crs_war 1}

 
4334sdio clone 4334rel -tag "PHOENIX_REL_5_120_37"
4334sdio clone 4334g -type "4334a0-roml/sdio-g-pool.bin"


# 4334sdio clone 4334B2Tmp -tag PHOENIX2_TWIG_6_10_56 -brand linux-external-dongle-sdio -type "4334b1-roml/sdio-ag-pool-idsup-p2p.bin" -wlinitcmds "" -postinstall ""
4334sdio clone 4334B2Tmp -tag PHOENIX2_REL_6_10_58_28 -brand linux-external-dongle-sdio -type "4334b1-roml/sdio-ag-pool-idsup-p2p.bin" -wlinitcmds "" -postinstall ""
4334sdio clone 4334B3Rel -tag PHOENIX2_REL_6_10_56_?? -brand linux-external-dongle-sdio -type "4334b1-roml/sdio-ag-pool-idsup-p2p.bin" -wlinitcmds "" -postinstall ""
4334sdio clone 4334B3RelF -tag PHOENIX2_REL_6_10_56_?? -brand linux-external-dongle-sdio -type "4334b1-roml/sdio-ag-pool-idsup-p2p.bin" -wlinitcmds "" -postinstall "dhd -i eth0 sd_divisor 1"
4334sdio clone 4334B3minRelF -tag PHOENIX2_REL_6_10_56_* -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pool-idsup-p2p.bin" -wlinitcmds "" -postinstall "dhd -i eth0 sd_divisor 1"
4334sdio clone 4334B3Rel58 -tag PHOENIX2_REL_6_10_58_55 -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan.bin" -wlinitcmds "" -postinstall ""
# \ -nvram bcm94334fcagb_B2.txt
4334sdio clone 4334B3hSam -tag PHOENIX2_REL_6_10_58_?? -brand linux-external-dongle-sdio -type "4334b1-roml/sdio-ag-pool-idsup-p2p.bin" -wlinitcmds "" -postinstall ""
# sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe.bin 
4334sdio clone 4334B3Sam -tag PHOENIX2_REL_6_10_58_28 -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth.bin"  -wlinitcmds "" -postinstall ""
4334sdio clone 4334B3min -tag PHOENIX2_REL_6_10_56_?? -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe.bin"  -wlinitcmds "" -postinstall ""
4334sdio clone 4334B3RoWminExt -tag PHOENIX2_REL_6_10_58_?? -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan.bin"  -wlinitcmds "" -postinstall ""
4334sdio clone 4334B3RoWminExtF -tag PHOENIX2_REL_6_10_58_?? -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan.bin"  -wlinitcmds "wl btc_mode 0" -postinstall "dhd -i eth0 sd_divisor 1"
4334sdio clone 4334B3RoWminExtRvR -tag PHOENIX2_REL_6_10_58_?? -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan.bin"  -wlinitcmds "wl btc_mode 0" -postinstall ""
4334sdio clone 4334B3RoW -tag PHOENIX2_REL_6_10_58_?? -brand linux-external-dongle-sdio -type "4334b1-roml/sdio-ag-ccx-btamp-p2p-idsup-idauth-pno.bin"  -wlinitcmds "" -postinstall ""
4334sdio clone 4334B3RoWmin -tag PHOENIX2_REL_6_10_116_* -brand linux-internal-dongle -type "4334b1min-roml/sdio-ag-idsup-p2p-err-assert-sr-dmatxrc.bin"  -wlinitcmds "" -postinstall ""
4334sdio clone 4334B3RoWminSVT -tag PHOENIX2_REL_6_10_58_* -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan.bin" -wlinitcmds "" -postinstall ""
4334sdio clone 4334B3RoWminSVTF -tag PHOENIX2_REL_6_10_58_* -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan.bin" -wlinitcmds "wl btc_mode 1" -postinstall "dhd -i eth0 sd_divisor 1"
4334sdio clone 4334B3RoWminRvR -tag PHOENIX2_REL_6_10_116_* -brand linux-internal-dongle -type "4334b1min-roml/sdio-ag-idsup-p2p-err-assert-sr-dmatxrc.bin"  -wlinitcmds "wl btc_mode 0" -postinstall ""
4334sdio clone 4334B3RoWminSVTRvR -tag PHOENIX2_REL_6_10_58_* -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan.bin" -wlinitcmds "wl btc_mode 0" -postinstall ""
4334sdio clone 4334B3RoWminSVTRvRF -tag PHOENIX2_REL_6_10_58_* -brand linux-external-dongle-sdio -type "4334b1min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan.bin" -wlinitcmds "wl btc_mode 0" -postinstall "dhd -i eth0 sd_divisor 1"
4334sdio clone 4334B3RoWminF -tag PHOENIX2_REL_6_10_116_* -brand linux-internal-dongle -type "4334b1min-roml/sdio-ag-idsup-p2p-err-assert-sr-dmatxrc.bin"  -wlinitcmds "" -postinstall "dhd -i eth0 sd_divisor 1"
4334sdio clone 4334B3RoWminFRvR -tag PHOENIX2_REL_6_10_116_* -brand linux-internal-dongle -type "4334b1min-roml/sdio-ag-idsup-p2p-err-assert-sr-dmatxrc.bin"  -wlinitcmds "wl btc_mode 0" -postinstall "dhd -i eth0 sd_divisor 1"

# Gear2 4334w epa/elna

4334sdio clone 4334w\
    -customer "bcm" \
	-nvram bcm94334wlagb.txt\
	-tag PHO2203RC1_REL_6_25_112_*\
	-brand linux-mfgtest-dongle-sdio\
    -type "43342a0-roml/sdio-ag-mfgtest-nodis-swdiv-oob-seqcmds-sr-idsup.bin"\
	-wlinitcmds "wl btc_mode 1"\
	-dhd_tag NIGHTLY
#     -type "43342a0-roml/sdio-ag-mfgtest-nodis-swdiv-seqcmds-idsup.bin"\

	
4334w clone 4334w_ToB -tag PHO2203RC1_BRANCH_6_25 -postinstall {} -clm_blob 43342_southpaw.clm_blob
4334w clone 4334w_bu -image /home/pkwan/BCM43342/Gear2/rtecdc.bin
4334w_ToB clone 4334w_ToB_coex -wlinitcmds {wl phy_crs_war 1; wl btc_mode 1}
4334w clone 4334w_coex -wlinitcmds {wl phy_crs_war 1; wl btc_mode 1}
4334w clone 4334wx -tag PHO2203RC1_REL_6_25*
4334wx clone 4334wx_coex -wlinitcmds {wl phy_crs_war 1; wl btc_mode 1}
		
# mc54 cohost2: mc59end6 10.19.87.100 
# 	-lan_ip 10.19.87.100\

UTF::WinBT MC54_BTCOHOSTXP2\
	-lan_ip 10.19.87.100\
	-sta "mc54_BTCohost2"\
	-power "172.16.54.25 1"\
	-device_reset "10.19.87.110 2"\
	-bt_comm "com6@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_ss_location 0x00210000\
    -type BCM4334/B1\
    -brand Generic_37_4MHz/fcbga_extLna\
    -version *

mc54_BTCohost2 clone BTCohost2DASH
mc54_BTCohost2 clone BTCohost2 -brand Generic/37_4MHz/fcbga*
# mc54_BTCohost2 clone BTCohost2 -brand Generic/37_4MHz/fcbga_extLna ;# older tree structure before 1/4/12

mc54_BTCohost2 clone BTCohost2Local -project_dir "/home/pkwan" -type "BCM4334/test" -version "*" -file "4334_fcbga_extlna_ev3_2retran_fix.cgs"
# pending confirmation 1/13/14
mc54_BTCohost2 clone BTCohost_4334w\
    -bt_ss_location 0x00210000\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
    -type BCM43342/A1_All_Builds\
    -version *\
    -brand Generic/37_4MHz/wlbga\
    -file *.cg*

BTCohost_4334w clone BTCohost_4334w_local -image /home/pkwan/BCM4334w/BCM43342A1_001.002.003.0979.0000_Generic_37_4MHz_wlbga.cgs

#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router MC54AP1 \
    -lan_ip 192.168.1.1 \
    -sta "AP1-4717 eth1" \
    -power "10.19.87.28 1"\
    -relay "mc54end1" \
    -lanpeer lan0 \
    -console "mc54end1:40000" \
	-brand "linux-internal-router"\
    -tag "COMANCHE2_REL_5_22_\?\?" \
    -date * \
   -nvram {
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestMC54
		wl0_channel=1
		w10_nmode=1
		wl0_antdiv=0
		antswitch=0
		wl0_obss_coex=0
    }
#     -brand "linux-external-router-combo" \
# Specify router antenna selection to use
set ::wlan_rtr_antsel 0x01

# clone AP1s
AP1-4717 clone MC54_AP1
AP1-4717 clone mc54_AP1
AP1-4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
AP1-4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
# 5.70.38 depleted as of 8/21/11
# AP1-4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
# AP1-4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
AP1-4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_??" -brand linux-external-router
AP1-4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_??" -brand linux-internal-router
AP1-4717 clone 4717MILLAUInt_Alt -tag "MILLAU_REL_5_70_48_27" -brand linux-internal-router
AP1-4717 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
AP1-4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router-combo
AP1-4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
AP1-4717 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

