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

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex

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


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
# moved to dedicated filer volume 10/30/13
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc54/43430"

# Define power controllers on cart.
# package require UTF::Power
UTF::Power::WebRelay 10.19.87.25 ;# Enclosure 1: DUT and BTCohostXP
# - dut webrelay 10.19.85.200 bt & wl reset lines
UTF::Power::WebRelay 10.19.87.26
UTF::Power::WebRelay 10.19.87.110 ;# webrelay DUT2

# AP pwr control & console
UTF::Power::Synaccess 10.19.87.27 ;# Encloser 1: DUT and BTCohostXP
UTF::Power::Synaccess 10.19.87.28
UTF::Power::Synaccess 172.16.54.25 ;# npc_DUT2


# Attenuator
UTF::Aeroflex af -lan_ip 172.16.54.10 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
	
  	G1 attn 35 ;# changed to 35 from 20 8/21/13
	G2 attn 23 ;# changed to 23 from 15 8/21/13 ;#changed from 22 to 15 on 3/2/12 for debugging
	G3 attn 103

	# turn off 4334 radio
	if {[catch {4334sdio wl down} ret]} {
        error "SetupTestBed: 4334sdio wl down failed with $ret"
    }

    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)

UTF::Linux mc54end1 -sta "lan0 eth1"
# -lan_ip 10.19.87.21  
#

# wlan DUT: mc54end2 ;# upgraded from FC11 to FC15 4/4/14

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
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {AP1-4717 wl dump ampdu}}
    
# 4345 epa sdio BCM94345FCSAGB_1 rev 05   
        
dut_sdio clone 4345sdio\
    -tag AARDVARK_BRANCH_6_30 \
    -brand linux-mfgtest-dongle-sdio\
    -nvram "src/shared/nvram/bcm94345fcsagb_epa.txt"\
    -type 4345a0-roml/sdio-ag-mfgtest-seqcmds\
    -dhd_tag TRUNK \
    -dhd_brand linux-internal-dongle \
    -slowassoc 5 -noaes 1 -notkip 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -tcpwindow 1152k -udp 400m -nocal 1\
	-nopm1 1 -nopm2 1\
    -wlinitcmds {wl vht_features 3}

# bcm943457fcsagb_1 epa Rev04
        
4345sdio clone 43457sdio\
 -nvram "src/shared/nvram/bcm943457fcsagb_epa.txt"\
 -brand linux-external-dongle-sdio\
 -customer bcm\
 -type 43457a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-lpc-wl11u-txbf-assert-err\
 -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
 -postinstall {dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 10}
 
43457sdio clone 43457coex -wlinitcmds {wl vht_features 3; wl btc_mode 1}
43457coex clone 43457coex_rel321\
 -dhd_tag DHD_REL_1_126\
 -dhd_brand linux-external-dongle-sdio\
 -tag AARDVARK_REL_6_30_321\
 -brand linux-external-dongle-sdio\
 -type 43457a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-lpc-wl11u-txbf.bin\
 -wlinitcmds {wl vht_features 3; wl PM 3}
 
43457coex_rel321 clone 43457coex_rel336\
 -tag AARDVARK_REL_6_30_336

43457coex_rel321 clone 43457coex_rel339\
 -tag AARDVARK_REL_6_30_339
 
 # bcm943430wlselgs_26MHz P110 4/4/14
 
43457sdio clone 43430a0\
	 -tag BISON_REL_7_10_226_*\
	 -type 43430a*-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus*-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-idsup-idauth.bin\
	 -nvram "src/shared/nvram/bcm943430wlselgs_26MHz.txt"\
	 -modopts {sd_uhsimode=2 sd_txglom=1 sd_tuning_period=10}\
	 -postinstall {} -wlinitcmds {}\
	 -dhd_tag DHD_REL_1_174\
	 -dhd_brand linux-external-dongle-sdio\
	 -nomaxmem 1

43430a0 clone 43430a0_Nightly -dhd_tag NIGHTLY
43430a0 clone 43430a0_coex -wlinitcmds {wl btc_mode 1}
43430a0_coex clone 43430a0_coex_rc31p -image "/projects/hnd_software_ext6/work/yashd/driver/BISON_TWIG_7_10_226_TEST_051114/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo.bin"
43430a0_coex clone 43430a0_coex_rc31p_r -image "/projects/hnd_software_ext6/work/yashd/driver/BISON_TWIG_7_10_226_TEST_RADIO_ON/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-radio_on.bin"
	 
43430a0 clone 43430a0_twig\
	-tag BISON_TWIG_7_10_*
	
43430a0_twig clone 43430a0_twig_coex -wlinitcmds {wl btc_mode 1}
	
43430a0_twig clone 43430a0_mfg\
	-type 43430a0-roml/sdio-g-mfgtest-seqcmds.bin\
	-brand linux-mfgtest-dongle-sdio\
	-nvram "bcm943430wlselgs_26MHz.txt"
	
43430a0 clone 43430a0_ToB -tag BISON_BRANCH_7_10 -dhd_tag NIGHTLY\
	 -type 43430a*-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus*-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-*idsup-idauth.bin
43430a0_ToB clone 43430a0_ToB_coex -wlinitcmds {wl btc_mode 1}

# build moved from  7.10 to 7.35 as of 8/27/14
43430a0_ToB clone 43430a1_ToB \
	-nvram bcm943430fsdng.txt\
	-type 43430a*-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus*-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srfast-err-assert.bin\
	 -modopts {}
43430a1_ToB clone 43430a1_ToB_coex -wlinitcmds {wl btc_mode 1}

43430a1_ToB clone 43430a1_05T_ToB \
	-tag BISON05T_BRANCH_7_35

43430a1_05T_ToB clone 43430a1_05T_ToB_coex -wlinitcmds {wl btc_mode 1}

43430a1_ToB clone 43430a1_06T_ToB \
	-tag BISON06T_BRANCH_7_45

43430a1_06T_ToB clone 43430a1_06T_ToB_coex -wlinitcmds {wl btc_mode 1}

43430a1_ToB clone 43430a1_twig \
	-tag BISON_REL_7_10_323_*

43430a1_twig clone 43430a1_twig_coex -wlinitcmds {wl btc_mode 1}

dut_sdio clone 43434b0\
    -tag DINGO07T_BRANCH_9_35 \
    -brand hndrte-dongle-wl\
    -nvram "src/shared/nvram/bcm943430fsdng_Bx.txt"\
    -type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-hs20sta/rtecdc.bin\
    -dhd_tag DHD_REL_1_363_17 \
    -dhd_brand linux-external-dongle-sdio \
    -app_tag trunk \
    -slowassoc 5 -noaes 1 -notkip 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -tcpwindow 1520k -udp 200m -nocal 1\
	-nopm1 1 -nopm2 1

43434b0 clone 43434b0_coex -wlinitcmds {wl btc_mode 1}

dut_sdio clone 43438a0\
	-tag BISON_BRANCH_7_10\
	-brand linux-external-dongle-sdio\
	-type 43430a0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr.bin\
	-nvram src/shared/nvram/bcm943438wlpth_26MHz.txt\
	-dhd_tag DHD_REL_1_201_34 -driver dhd-cdc-sdstd-debug\
	-postinstall {} -wlinitcmds {} -slowassoc 5\
	-nobighammer 1 -nocal 1 -noaes 1 -notkip 1
	
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

UTF::WinBT MC54_BTCOHOSTXP\
	-lan_ip mc54tst2\
	-sta "mc54_BTCohost"\
	-power "10.19.87.27 1"\
	-device_reset "10.19.87.25 1"\
	-bt_comm "com9@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -type BCM43291/A0\
    -brand Generic/38_4MHz\
    -bt_ss_location 0x00088000\
    -version *
    
mc54_BTCohost clone BTCohost43457\
	-bt_comm "com10@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -type BCM43457/A0\
    -brand Generic/UART/37_4MHz/TSMC_A0_FCBGA_ref_eLNA\
    -bt_ss_location 0x0021B000\
    -version *\
    -file *.cgs

BTCohost43457 clone BTCohost43457_cfg12 \
   -image /home/pkwan/BCM43457/A0/BCM43457A0_001.001.024.0012.0000_Generic_UART_37_4MHz_TSMC_A0_FCBGA_ref_eLNA.cgs
        
BTCohost43457 clone BTCohost43457_local \
   -image /home/pkwan/BCM43457/A0/BCM43457A0_001.001.024.0013.0000_Generic_UART_37_4MHz_TSMC_A0_FCBGA_ref_eLNA.cgs

BTCohost43457 clone BTCohost43457_cfg8 \
   -image /home/pkwan/BCM43457/A0/BCM43457A0_001.001.024.0008.0000_Generic_UART_37_4MHz_TSMC_A0_FCBGA_ref_eLNA.cgs

mc54_BTCohost clone BTCohost43430\
	-bt_comm "com10@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
	-type BCM4343/A0\
    -bt_ss_location 0x00210750\
    -brand Generic/UART/26MHz/wlbga_eLG\
	-version *\
    -file *.hcd

BTCohost43430 clone BTCohost43430_37 -brand Generic/UART/37_4MHz/wlbga_eLG

BTCohost43430 clone BTCohost43430_ref\
    -brand Generic/UART/26MHz/wlbga_ref
    
BTCohost43430_ref clone BTCohost43430_local\
	-project_dir "/home/pkwan"


BTCohost43430 clone BTCohost43430A1 -type BCM4343/A1 -bt_ss_location 0x00211700 -brand Generic/UART/37_4MHz/fcbga_eLG
BTCohost43430 clone BTCohost43434B0 -project_dir "/home/pkwan" -type BCM4343B0 -bt_ss_location 0x00211700 -brand Generic/UART/37_4MHz/fcbga_iLNA_ePA

BTCohost43430_local clone BTCohost43430A1_local -type BCM4343/A1 -bt_ss_location 0x00211700 -brand Generic/UART/37_4MHz/wlbga_ref

BTCohost43430A1 clone BLECohostBT

BTCohost43430 clone BTCohost43438 -brand Generic/UART/26MHz/wlbga_PTH

# BT Ref - WinXP laptop BlueTooth 2046 Reference
# mc24tst1  10.19.85.197   BtRef_XP     - TB6BTREF
#	-power_sta "10.19.85.199 1"\

UTF::WinBT MC54_BTREFXP\
	-lan_ip 10.19.87.23\
    -sta "mc54_BTRefXP"\
	-device_reset "10.19.87.26 1"\
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user

UTF::WinBT BLERef\
	-lan_ip 10.19.87.23\
    -sta "mc54_BLERef"\
	-device_reset "10.19.87.26 2"\
    -type "BCM2070"\
    -bt_comm "usb1"\
    -user user
    
mc54_BLERef clone BLERefBT

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
		lan_stp=0
		lan1_stp=0
    }

# Specify router antenna selection to use
set ::wlan_rtr_antsel 0x01

# clone AP1s
AP1-4717 clone MC54_AP1
AP1-4717 clone mc54_AP1
AP1-4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
AP1-4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
AP1-4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_??" -brand linux-external-router
AP1-4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_??" -brand linux-internal-router
AP1-4717 clone 4717MILLAUInt_Alt -tag "MILLAU_REL_5_70_48_27" -brand linux-internal-router
AP1-4717 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
AP1-4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
AP1-4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
AP1-4717 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router
AP1-4717 clone mc54_AP1_ToB -tag NIGHTLY

# experimental   
unset UTF::TcpReadStats