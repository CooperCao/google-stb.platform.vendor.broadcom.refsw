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
# package require UTF::Aeroflex
package require UTF::AeroflexDirect

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!

set ::bt_dut Cohost4335c0				;# BlueTooth device under test
set ::bt_ref mc60_BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 4335wlipab	  			;# HND WLAN device under test
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
UTF::AeroflexDirect af -lan_ip 172.16.60.10 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}
# fixed attn = 20

set ::UTF::SetupTestBed {
	
	G1 attn 103
    G2 attn 5 ;# from 15 6/3/16
#     G2 attn 25 ;# changed to 25 from 10 8/27/14
	G3 attn 0 ;# changed to 25 from 15 8/27/14

	# turn off 4358 radio
	if {[catch {4358 wl down} ret]} {
        error "SetupTestBed: 4358 wl down failed with $ret"
    }
    4358 deinit
	
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)

UTF::Linux mc60UTF  -lan_ip 10.19.87.105  -sta "lan0 eth1"

### temporary device

UTF::DHD mc60DUTtmp\
	-lan_ip 10.19.60.29\
	-sta "tmpDUT eth0"\
	-power "172.16.60.30 1"\
	-device_reset "10.19.87.113 2"
	
tmpDUT clone 43452\
	-tag BIS715T254_BRANCH_7_52\
	-brand linux-external-dongle-pcie\
	-type ../C-43452__s-A2/hans.trx -clm_blob ../C-43452__s-A2/hans.clmb\
	-nvram ../C-43452__s-A2/P-hans_M-COR2_V-t__m-1.3.txt\
	-customer olympic\
	-dhd_tag trunk\
	-dhd_brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -postinstall {dhd -i eth0 dma_ring_indices 3} \
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1 -nokpps 1 \
    -tcpwindow 1152k -udp 400m \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}\
	-power ""

43452 clone 43452_coex -wlinitcmds {wl btc_mode 1} -device_reset ""
43452 clone 43452_747 -tag BIS735T255RC1_BRANCH_7_47
43452_747 clone 43452_747_coex -wlinitcmds {wl btc_mode 1} -device_reset ""
43452 clone 43452_eagle -tag BIS735T255RC1_REL_7_47_1??
43452_eagle clone 43452_eagle_coex -wlinitcmds {wl btc_mode 1} -device_reset ""
43452 clone 43452_764 -tag BIS747T144RC2_BRANCH_7_64
43452_764 clone 43452_764_coex -wlinitcmds {wl btc_mode 1} -device_reset ""
43452_764 clone 43452_764_eagle -tag BIS747T144RC2_REL_7_64_* -dhd_tag DHD_REL_1_359_115 -app_tag DHD_REL_1_359_115
43452_764_eagle clone 43452_764_eagle_coex -wlinitcmds {wl btc_mode 1} -device_reset ""
# 43452 clone 43452_941_eagle -tag DIN2915T165R6_REL_9_41_*
# 43452_941_eagle clone 43452_941_eagle_coex -wlinitcmds {wl btc_mode 1} -device_reset ""
43452_764 clone 43452_764_whitetail \
  -tag BIS747T144RC2_REL_7_64_* \
  -dhd_tag DHD_REL_1_359_154 \
  -app_tag DHD_REL_1_359_154 \
  -type ../C-43452__s-A*/hans.trx -clm_blob ../C-43452__s-A*/hans.clmb\
   -nvram ../C-43452__s-A*/P-hans_M-COR2_V-t__m-1.3.txt
43452_764_whitetail clone 43452_764_whitetail_coex -wlinitcmds {wl btc_mode 1} -device_reset ""


### device: 4345a0 ipa pcie interface; eth0 -- FC15
### hostname mc50tst5 ip_addr 10.19.86.204 prior to 8/8/14; 10.19.84.202 since
### changed to DNS hostname mc60end5 ip_addr 10.19.85.190 as to 8/8/14

UTF::DHD mc60DUT2\
	-lan_ip mc60end5\
	-sta "4345 eth0"\
	-power "172.16.60.30 1"\
	-device_reset "10.19.87.113 2"
	
4345 clone 43452a3\
	-tag BIS715T254_BRANCH_7_52\
	-brand linux-external-dongle-pcie\
	-type ../C-43452__s-A3/hans.trx -clm_blob ../C-43452__s-A3/hans.clmb\
	-nvram ../C-43452__s-A3/P-hans_M-COR2_V-t__m-1.5.txt\
	-customer olympic\
	-dhd_tag trunk\
	-dhd_brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -postinstall {dhd -i eth0 dma_ring_indices 3} \
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1 -nokpps 1 \
    -tcpwindow 1152k -udp 400m \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}
    
# 	-power "172.16.60.30 1"\
#     -postinstall {dhd -i eth0 dma_ring_indices 3}	
#     -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc} 

43452a3 clone 43452a3_ref\
	-type 43452a3-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-clm_min-txpwrcap-bssinfo-txcal.bin\
	-nvram src/shared/nvram/bcm94345fcpagbi.txt\
	-clm_blob ""\
	-power ""

43452a3 clone 43452a2_mod\
	-type ../C-43452__s-A2/hans.trx -clm_blob ../C-43452__s-A2/hans.clmb\
	-nvram ../C-43452__s-A2/P-hans_M-COR2_V-t__m-1.3.txt\
	-power ""
43452a2_mod clone 43452a2_mod_coex -wlinitcmds {wl btc_mode 1} -device_reset ""

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
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}}
# # #     -yart {-attn5g 20-63 -attn2g 0-63 -pad 42 -frameburst 1} \
# # #     -type "4358a0-roml/pcie-ag-msgbuf-splitrx-pktctx-ampduhostreorder-amsdutx-txbf-p2p-txpwr-dbgam-phydbg.bin"\

4358 clone 4358_coex -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}
4358 clone 4358a1\
	-nvram src/shared/nvram/bcm94358wlspie_p106.txt\
    -type "4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-mchandump.bin"
4358a1 clone 4358a1_coex -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}
# assumes 4358a1 with 05T drivers
4358 clone 4358_05T -tag BISON05T_BRANCH_7_35\
	-nvram bcm94358wlspie_p106.txt\
	-dhd_tag DHD_REL_1_201_*\
	-type 4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-btcdyn.bin
4358_05T clone 4358_05T_coex -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}
4358_05T clone 4358a1_twig -tag BISON05T_REL_7_35_*
4358a1_twig clone 4358a1_twig_coex -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}

4358_05T clone 4358a1B85\
	-nvram src/shared/nvram/bcm94358pciemdlB85SS_SS.txt\
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-proxd-nan-shif-tbow-btcdyn-ndoe-idsup.bin
4358a1B85 clone 4358a1B85_coex -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc; wl vht_features 3; wl btc_mode 1}

# # # experiment for 4350c4 with BCM9EMB2CPIEAD Rev05: Rev07 boards received do not work with 4350c4 Stella boards 10/17/14
# # #  
4358 clone 4350c4\
	-app_tag BIS120RC4PHY_BRANCH_7_16 \
    -tag BIS120RC4PHY_BRANCH_7_16 \
    -dhd_tag trunk \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94350RieslingBMurataMT.txt" \
    -customer olympic \
    -type ../C-4350__s-C2/rieslingb.trx -clm_blob rieslingb.clmb \
    -postinstall {dhd -i eth0 dma_ring_indices 3}\
    
# # # 4350c4 clone 4350c4_rel\
# # # 	-tag BIS120RC4_REL_7_15_*\
# # # 	-nvram ../C-4350__s-C4/P-rieslingb_M-STEL_V-m__m-4.13.txt\
# # # 	-type ../C-4350__s-C2/rieslingb.trx -clm_blob rieslingb.clmb 

4350c4 clone 4350c4_rel\
	-tag BIS120RC4_REL_7_15_*\
	-nvram ../C-4350__s-C4/P-rieslingb_M-STEL_V-u__m-3.9.txt\
	-type ../C-4350__s-C2/rieslingb.trx -clm_blob rieslingb.clmb
	
# # # 4350c4 clone 4350c4_rel\
# # # 	-tag BIS120RC4_REL_7_15_*\
# # # 	-nvram ../C-4350__s-C4/P-rieslinga_M-CIDR_V-u__m-2.9.txt\
# # # 	-type ../C-4350__s-C2/rieslinga.trx -clm_blob rieslinga.clmb
 
4350c4_rel clone 4350c4_rel_coex -wlinitcmds "wl btc_mode 1"
# # #  
# # # end 4350c4 configuration statements

# BTCohost mc60end5 10.19.85.190 as of 8/7/14 (prior: mc59end5 10.19.87.99 --> DNS mapping has changed)
# changed to mc60end6 ip_addr 10.19.85.208 8/8/14

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

BTCohost4345 clone BTCohost4345_local -project_dir "/home/pkwan"
	
BTCohost4345 clone BTCohost43452\
	-bt_comm "com5@3000000"\
	-bt_ss_location 0x0021AFC0\
	-type BCM43452\
	-version * \
	-brand *OS*TDK*\
	-file *.hcd\
	-device_reset ""
		
BTCohost43452 clone BTCohost43452_local -version "*OS*TDK*"  -brand "" -project_dir "/home/pkwan"
BTCohost43452 clone BTCohost43452_irv -version "" -project_dir "//brcm-irv/dfs/projects/blueth_release/BroadcomInternal"
BTCohost43452 clone BTCohost43452_eagle -brand BCM43452A2_13*OS*TDK*
BTCohost43452 clone BTCohost43452_cur -brand BCM43452A2_*OS*TDK*

#	-bt_ss_location 0x0021C104\	; prior to 1/6/15
# hostname: mc60end2, ip_addr 10.19.87.106	
	
UTF::DHD mc60DUT \
	-lan_ip mc60end2\
	-sta "4330B1 eth0"\
	-power "172.16.60.20 2"\
	-device_reset "10.19.87.114 1"
# # # 	\
# # #     -console mc60end2:40001
# 4354A0    
    
4330B1 clone 4355c0 \
	-tag DIN2930R18_BRANCH_9_44 \
	-app_tag DHD_BRANCH_1_359\
	-dhd_tag DHD_BRANCH_1_359\
	-brand linux-external-dongle-pcie \
	-type 4355c0-roml/config_pcie_release/rtecdc.bin \
	-clm_blob 4355_olaf.clm_blob \
	-dhd_brand linux-external-dongle-pcie \
	-nvram "../../olympic/C-4355__s-C0/P-olaf_M-PRNL_V-m__m-5.7.txt" \
    -driver dhd-msgbuf-pciefd-debug \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}\
    -postinstall {dhd -i eth0 dma_ring_indices 3}\
    -datarate {-i 0.5 -frameburst 1}\
    -nocal 1 -noaes 1 -notkip 1 -nokpps 1 -nomaxmem 1 \
    -tcpwindow 1152k -udp 800m \
    -device_reset {}

# Yebisu module: angerb; YebisuCider module: angera

4355c0 clone 4355c0_930 -tag DIN2915T250RC1_BRANCH_9_30 \
	-customer olympic\
	-type ../C-4355__s-C0/angerb.trx -clm_blob ../C-4355__s-C0/angerb.clmb\
	-nvram "../../olympic/C-4355__s-C0/P-angerb_M-YSBU_V-m__m-2.1.txt"

4355c0_930 clone 4355c0_930_coex -wlinitcmds {wl mpc 0; wl btc_mode 1}

4355c0_930 clone 4355c0_930j -type ../C-4355__s-C0/joyb.trx -clm_blob ../C-4355__s-C0/joyb.clmb\
	-nvram "../../olympic/C-4355__s-C0/P-joyb_M-YSBU_V-m__m-2.1.txt"
	
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
	
mc60_BTCohost clone Cohost4355c0\
	-bt_comm "com15@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_ss_location 0x00220000\
	-bt_xtal_freq 37.4\
    -bt_power_adjust 40\
	-type BCM4355/C0\
	-version BCM4355C0_*_AngerB_OS_MUR_MCC_TRS_*\
	-brand ""\
	-file *.cgr
	
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
	

Cohost4335c0 clone Cohost4350\
	-bt_comm "com6@3000000"\
	-bt_ss_location 0x0021C000\
	-bt_xtal_freq 40\
    -type BCM4350B0\
	-version BCM4350B0_*\
	-brand "Generic/UART/37_4MHz/wlbga_BU"\
	-file *.cg*
	
# # # Cohost4350 clone Cohost4350c0 -type BCM4350/C0 -version BCM4350C*
# 4354a* uses 4350c0 configurations
Cohost4350 clone Cohost4350c0 -bt_ss_location 0x0021E000 -type BCM4350/C0 -version BCM4350C*
Cohost4350c0 clone Cohost4350c0_eLNA -brand "Generic/UART/37_4MHz/wlbga_BU_eLNA"
Cohost4350c0 clone Cohost4350c0_cfg51 -version *0051*
Cohost4350c0 clone Cohost4350c0_cfg148 -version *0148*
		
UTF::WinBT mc60BTREFXP \
	-lan_ip 10.19.87.107 \
    -sta "mc60_BTRefXP" \
	-device_reset "10.19.87.113 1"\
    -type "BCM2046" \
    -bt_comm "usb0" \
    -user user

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
    -tag "MILLAU_REL_5_70*" \
    -date * \
   -nvram {
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestMC60
	    wl0_channel=1
	    antswitch=0
	    wl0_obss_coex=0
	    wl0_radio=0
    }


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
		wl1_bw_cap=-1
		wl1_radio=0
		wl1_obss_coex=0
    }\
    -datarate {-b 1.2G -i 0.5 -frameburst 1}\
    -noradio_pwrsave 1\
    -date 2014.10.2.

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
# 4360_AARDVARK clone 4360_current -date *
# # # 4360_current clone 4360_twig -tag "AARDVARK_{REL,TWIG}_6_30_???{,_*}"
# 4360_current clone 4360_twig -tag "AARDVARK_REL_6_30_???_*"
# 4360_current clone 4360_current_twig -tag "AARDVARK_TWIG_6_30_???{_*}"

unset UTF::TcpReadStats
