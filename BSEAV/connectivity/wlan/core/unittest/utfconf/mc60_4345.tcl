# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC60
# MC60 testbed configuration
#
# mc60end1  10.19.87.105   Linux UTF Host   - mc60UTF
# mc60end2  10.19.87.106   DUT Linux        - mc60DUT
# mc60end3  10.19.87.107   BtRefXP      	- mc60BTREF
# mc60end4  10.19.87.108   BtCoHostXP   	- mc60BTCOHOST
# mc60end9  10.19.87.113   web_relay_BtRef
# mc60end10 10.19.87.114   web_relay_DUT
#			172.16.60.10   Aeroflex attenuator
#           172.16.60.40   NPC_AP
#           172.16.60.30   NPC_DUT2
#           172.16.60.20   NPC_DUT

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex
# package require UTF::AeroflexDirect

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!

set ::bt_dut BTCohost4345				;# BlueTooth device under test
set ::bt_ref mc60_BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 4345pcie		  			;# HND WLAN device under test
set ::wlan_rtr mc60_AP1					;# HND WLAN router
set ::wlan_tg lan0              		;# HND WLAN traffic generator

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc60"

# Define power controllers on cart.
# package require UTF::Power
UTF::Power::WebRelay 10.19.87.113
UTF::Power::WebRelay 10.19.87.114
# AP pwr control & console
UTF::Power::Synaccess 172.16.60.40
# 4360/4706 11ac AP pwr control & console
UTF::Power::Synaccess 172.16.60.50 -rev 1
# RPC-22 for DUT and Cohost
UTF::Power::Synaccess 172.16.60.30
UTF::Power::Synaccess 172.16.60.20
UTF::Power::Synaccess 172.16.60.21 ;# NPC-22 for DUT power control

# Attenuator
### G1: 2x2 MIMO wlan; G2: BT path; G3: 3x3 11ac MIMO wlan
# # # UTF::AeroflexDirect af -lan_ip 172.16.60.10 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}
UTF::Aeroflex af -lan_ip 172.16.60.10 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}
# fixed attn = 20

set ::UTF::SetupTestBed {
# changed setting: using 4360 on G3 (4717 on G1)	
	G1 attn 103 
    G2 attn 20
	G3 attn 0 ;# changed to 0 from 30 for 4358a1B85 9/18/14 ;# changed to 30 from 10 8/18/14

	# turn off 4354 radio
	if {[catch {4354a1 wl down} ret]} {
        error "SetupTestBed: 4354a1 wl down failed with $ret"
    }
    4354a1 deinit
	
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)

UTF::Linux mc60UTF  -lan_ip 10.19.87.105  -sta "lan0 eth1"

### device: 4345a0 ipa pcie interface
### hostname mc50tst5; lan_ip 10.19.86.204; updated to FC15 2/13/14
### changed DNS hostname to mc60end5 ip_addr 10.19.85.90 8/8/14

UTF::DHD mc60DUT2\
	-lan_ip mc60end5\
	-sta "4345 eth0"\
	-power "172.16.60.30 2"

# # # -lan_ip mc50tst5\ ;# assignment prior to 8/8/14
# # # 	\
# # # 	-device_reset "10.19.87.113 2" 
# # # 	;# working around a wierd issue 10/11/13: memory leak after device_reset

# 4358a0 pcie; host name mc50tst5

4345 clone 4358\
	-tag BISON_BRANCH_7_10\
    -brand linux-external-dongle-pcie\
    -type "4358a0-roml/pcie-ag-msgbuf-splitrx-pktctx-ampduhostreorder-amsdutx-txbf-p2p-txpwr-dbgam-phydbg.bin"\
    -customer "bcm"\
	-nvram src/shared/nvram/bcm94358pciemdlSS_SS.txt\
	-dhd_brand linux-internal-dongle-pcie\
	-driver dhd-msgbuf-pciefd-debug\
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1 \
    -tcpwindow 2m -udp 1.2g \
    -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}}

4358 clone 4358_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}

4358 clone 4358a1\
	-nvram src/shared/nvram/bcm94358wlspie_p106.txt\
    -type "4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-mchandump.bin"
4358a1 clone 4358a1_coex -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}
# assumes 4358a1 with 05T drivers
4358 clone 4358_05T -tag BISON05T_BRANCH_7_35\
	-nvram bcm94358wlspie_p106.txt\
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-btcdyn-ndoe-idsup.bin\
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}
### changed target 8/13/14 to:
# # # 	pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-btcdyn-ndoe-idsup.bin
### reverted back to msgbuf target per sharepoint info 8/14/14, *** -ndoe-idsup targets no longer available in nightly builds:
###	-type 4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-btcdyn.bin
# # # 		-dhd_tag DHD_REL_1_201_*\; use trunk DHD until SWWLAN-53801 is resolved
4358_05T clone 4358_05T_coex -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}
4358_05T_coex clone 4358_05T_a2dp -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1; wl btc_params 82 0}
4358_05T_coex clone 4358_05T_a2dp_dbg -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc}
4358_05T clone 4358a1_twig -tag BISON05T_REL_7_35_*
4358a1_twig clone 4358a1_twig_coex -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}
4358_05T_coex clone 4358_05T_coex5 -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 5}
4358_05T_coex5 clone 4358_05T_coex5_nodesense -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 5; wl btc_dynctl -f 0x00}
4358_05T_coex clone 4358_05T_coex_nodesense -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1; wl btc_dynctl -f 0x00}
# # # wl btc_dynctl -f 0x00
4358_05T_coex clone 4358_05T_coex3 -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 3}
4358_05T_coex3 clone 4358_05T_coex3_nodesense -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 3; wl btc_dynctl -f 0x00}

4358_05T clone 4358a1B85\
	-nvram src/shared/nvram/bcm94358pciemdlB85SS_SS.txt\
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-btcdyn.bin

# # # original target prior to 9/11/14
# # # 	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-btcdyn-ndoe-idsup.bin
# # # 	target per sharpoint info, msgbuf-splitrx target no longer available in nightly builds as of 8/14/14:
# # # pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-btcdyn.bin
# # # pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-btcdyn-ndoe-idsup.bin
4358a1B85 clone 4358a1B85_coex -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}
4358a1B85_coex clone 4358a1B85_coex_nodesense -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1; wl btc_dynctl -f 0x00}
4358a1B85_coex clone 4358a1B85_coex3 -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 3}
4358a1B85_coex3 clone 4358a1B85_coex3_nodesense -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 3; wl btc_dynctl -f 0x00}
4358a1B85_coex clone 4358a1B85_coex5 -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 5}
4358a1B85_coex5 clone 4358a1B85_coex5_nodesense -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1; wl btc_dynctl -f 0x00}

# 4358a1B85 clone 4358a1B85x -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-logstrs.bin
4358a1B85 clone 4358a1B85x -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-proxd-nan-shif-btcdyn-wepso-rcc-noccxaka-sarctrl-abt.bin

# cohost2: hostname mc59end5, lap ip: 10.19.87.99
# changed to mc60end5 10.19.85.190 8/7/14; should be mc60end6 10.19.85.208
# changed to mc60end6 as of 8/8/14

UTF::WinBT mc60BTCOHOST2\
	-lan_ip mc60end6\
	-sta "BTCohost4345"\
    -device_reset "10.19.87.113 3"\
   	-bt_comm "com4@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -type BCM4345/A0\
    -bt_ss_location 0x0021B000\
	-brand "Generic/UART/37_4MHz/fcbga_BU"\
	-version BCM4345A0*\
	-file *.cg*
		
# # #     -project_dir "/home/pkwan"\

BTCohost4345 clone BTCohost4345_tmp -image /home/pkwan/BCM4345/A0/BCM4345A0_001.001.035.0048.0000_Generic_UART_37_4MHz_fcbga_BU_eLNA.cgs	
BTCohost4345 clone BTCohost4345_corona -bt_comm "com5@3000000" 
### changed to B0 eLNA as of 1/7/14
BTCohost4345_corona clone BTCohost4345_corona_b0 -type BCM4345/B* -brand "Generic/UART/37_4MHz/fcbga_BU_eLNA" -version BCM4345B* -bt_ss_location 0x0021C000
BTCohost4345_corona_b0 clone BTCohost4345_barbera -type BCM4345/B* -brand "" -version BCM4345B*Barbera*OS*USI*

BTCohost4345 clone BTCohost4358\
   	-bt_comm "com5@3000000"\
    -bt_xtal_freq 40\
    -bt_power_adjust 40\
	-type BCM43569\
	-brand "Generic/UART/37_4MHz/fcbga_BU"\
	-bt_ss_location 0x0021E000\
	-version BCM43569*\
	-file *.cg*
# 	-project_dir "/home/pkwan/src/tools/unittest"\	
BTCohost4358 clone BTCohost4358_40\
	-brand "Generic/UART/40MHz/fcbga_BU"

BTCohost4358 clone BTCohost4358a1\
	-type BCM4358 -version BCM4358A1*\
	
BTCohost4358 clone BTCohost4358a1_B85\
	-type BCM4358\
	-version BCM4358A1*\
	-brand "Generic/UART/37_4MHz/wlcsp_SEMCO*"

BTCohost4358a1_B85 clone BTCohost4358a1_B85_sam\
	-brand ""
	
BTCohost4358 clone BLECohostBT
	
UTF::DHD mc60DUT \
	-lan_ip 10.19.87.106\
	-sta "4330B1 eth0"\
	-power "172.16.60.20 2"\
	-device_reset "10.19.87.114 1"\
    
    # 4354A0/A1    
    
4330B1 clone 4354a0\
	-tag BISON_REL_7_10_82_*\
    -brand linux-external-dongle-sdio\
    -type "4354a0-ram/sdio-ag-p2p-pno-aoe-sr-mchan-proptxstatus-ampduhostreorder-pktctx-dmatxrc-idsup-idauth.bin"\
    -customer "bcm"\
	-nvram bcm94354wlsagbi.txt\
	-dhd_tag DHD_REL_1_141_3*\
	-dhd_brand linux-external-dongle-sdio\
	-nocal 1 -noaes 1 -notkip 1 -nomaxmem 1\
	-modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10}\
 	-postinstall {dhd -i eth0 sd_blocksize 2 256; dhd -i eth0 txglomsize 10} \
	-wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl bw_cap 2g -1} \
	-tcpwindow 3640k -udp 600m\
    -datarate {-i 0.5 -frameburst 1 -b 1.2g}    
    
4354a0 clone 4354a0_coex -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl bw_cap 2g -1; wl btc_mode 1}
4354a0 clone 4354a1 -nvram bcm94354wlsagbl.txt -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-mchandump.bin
4354a1 clone 4354a1_coex -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl bw_cap 2g -1; wl btc_mode 1}

# hostname mc60end4, ip_add 10.19.87.108
    	     
UTF::WinBT mc60BTCOHOST\
	-lan_ip 10.19.87.108\
	-sta "mc60_BTCohost"\
    -power "172.16.60.20 1"\
    -device_reset "10.19.87.114 2"\
   	-bt_comm "com8@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -type BCM20710A0\
    -bt_ss_location 0x00090000\
	-brand "Olympic/Sulley/Module"\
	-version BCM20710A0_001.001.024.*.0000\
	-file *.cg*
			
mc60_BTCohost clone Cohost4335c0\
	-bt_comm "com6@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -bt_ss_location 0x00218000\
	-type BCM4339\
	-brand Generic/UART/37_4MHz/wlbga_iPA\
	-version *
	
Cohost4335c0 clone Cohost4335c0_local\
	-project_dir "/home/pkwan"\
	-brand "UART/37_4MHz/wlbga_iPA"\
	-type BCM4339\
	-version *
	
Cohost4335c0 clone Cohost4335c0_epa_local\
	-bt_comm "com6@3000000"\
	-project_dir "/home/pkwan"\
	-brand "UART/37_4MHz/wlbga_ePA"\
	-type BCM4339\
	-version *	
	
Cohost4335c0 clone Cohost4335c0_epa\
	-bt_comm "com10@3000000"\
	-brand "*/UART/37_4MHz/wlbga_ePA"\
	-type BCM4339\
	-version *	

Cohost4335c0 clone Cohost4335c0_ipa_local\
	-bt_comm "com6@3000000"\
	-project_dir "/home/pkwan"\
	-brand "Generic/UART/37_4MHz/wlbga_iPA"\
	-type BCM4339\
	-version *		
	
Cohost4335c0 clone Cohost4335c0_ipa_local_cfg12\
	-bt_comm "com6@3000000"\
	-project_dir "/home/pkwan"\
	-brand "UART/37_4MHz/wlbga_iPA"\
	-type BCM4339\
	-version BCM4339*12*
	
Cohost4335c0 clone Cohost4335b0\
	-bt_comm "com10@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -bt_ss_location 0x00218000\
    -type BCM4335\
	-version BCM4335B0_*\
	-brand "Generic/UART/37_4MHz/wlbga_ePA"\
	-file *.cg*
		
UTF::WinBT mc60BTREFXP \
	-lan_ip 10.19.87.107 \
    -sta "mc60_BTRefXP" \
	-device_reset "10.19.87.113 1"\
    -type "BCM2046" \
    -bt_comm "usb0" \
    -user user
    
mc60_BTRefXP clone BLERefBT
    
#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router MC60AP1 \
    -lan_ip 192.168.1.10 \
    -sta "mc60_AP1 eth1" \
    -relay "mc60UTF" \
    -lanpeer lan0 \
    -console "10.19.87.105:40000" \
    -power "172.16.60.40 1"\
    -brand "linux-external-router-combo" \
    -tag "MILLAU_REL_5_70*?" \
    -date * \
   -nvram {
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestMC60
	    wl0_channel=1
	    antswitch=0
	    wl0_obss_coex=0
# 	    wl0_radio=0
    }
    
# 	wl0_antdiv=0
#   w10_nmode=1
# # #     -tag "COMANCHE2_REL_5_22_\?\?" \

# Specify router antenna selection to use
# set ::wlan_rtr_antsel 0x01

# clone AP1s
mc60_AP1 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
mc60_AP1 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
mc60_AP1 clone 4717MILLAU -tag "MILLAU_REL_5_70_56_19" -brand linux-external-router
mc60_AP1 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_??" -brand linux-internal-router
mc60_AP1 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
mc60_AP1 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
mc60_AP1 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
mc60_AP1 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

# 4706 AP with 4360 and 4331 cards ;# debug 12/16/14: backdate build to 2014.10.2.0

UTF::Router MC60AP2 -sta {
    4706/4360 eth1 4706/4360.%15 wl0.% 
    4706/4331 eth2
    } \
    -lan_ip 192.168.1.2 \
    -power "172.16.60.50 1"\
    -relay "mc60UTF" \
    -lanpeer lan0 \
    -console "10.19.87.105:40036" \
    -nvram {
	   	lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.200
	    dhcp_end=192.168.1.249
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.200
	    dhcp1_end=192.168.2.249
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestMC60_4360
		wl0_channel=36
		w10_bw_cap=-1
# 		wl0_radio=0
		wl0_obss_coex=0
		wl1_ssid=TestMC60_4331
		wl1_channel=1
		wl1_vw_cap=-1
		wl1_radio=0
		wl1_obss_coex=0
    }\
    -datarate {-b 1.2G -i 0.5 -frameburst 1}\
    -noradio_pwrsave 1\
    -date 2014.10.2.

# # # experimental statement for A2DP tests
# set ::wlan_rtr_antsel 0x01
    

4706/4360 clone 4360_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router" 
4706/4331 clone 4331_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router" 
4706/4360 clone 4360_BISON -tag "BISON_BRANCH_7_10" -brand "linux26-internal-router" 
4706/4331 clone 4331_BISON -tag "BISON_BRANCH_7_10" -brand "linux26-internal-router" 
# 4360_AARDVARK clone 4360_ToB
4360_BISON clone 4360_ToB
4331_BISON clone 4331_ToB
4360_AARDVARK clone 4360_trunk -tag TRUNK
4331_AARDVARK clone 4331_trunk -tag TRUNK
4360_AARDVARK clone 4360_CARIBOU -tag CARIBOU_BRANCH_8_10
4331_AARDVARK clone 4331_CARIBOU -tag CARIBOU_BRANCH_8_10