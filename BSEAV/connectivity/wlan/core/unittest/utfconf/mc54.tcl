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
	G2 attn 15 ;# changed from 22 to 15 on 3/2/12 for debugging
	G3 attn 103
#     ALL attn 0

# turn off 4334 radio
	if {[catch {4334sdio wl down} ret]} {
        error "SetupTestBed: 4334sdio wl down failed with $ret"
    }

# # #     sample code   
# # #     foreach S {4334b1 4334h} {
# # # 	  catch {$S wl down}
# # # 	  $S deinit
# # #     }
	
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)

UTF::Linux mc54end1 -sta "lan0 eth1"
# -lan_ip 10.19.87.21  
#

# wlan DUT: mc54end2

UTF::DHD mc54DUT \
	-lan_ip 10.19.87.22 \
	-sta "43291sdio eth1" \
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
    
# 	-power_sta "10.19.87.25 4"\
# 	-device_reset "10.19.87.25 2"\

# 	-type "4329c0/sdio-g-cdc-roml-reclaim-full11n-wme-idsup-apsta-assoc-assert.bin"\#     \

# 	-postinstall {dhd -i eth1 serial 1}
#  	-power_sta "10.19.85.200 3" \
# 	-power "10.19.85.207 1" \
# added pre_ and post_perf_hooks on 11/3/10, per JohnB's sample in MC46.tcl
#     -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup.bin" \ ; for 4329B1-N18 only
    # preferred nvram file location
#     -nvram   "src/shared/nvram/bcm94329OLYMPICN18.txt" 
#    -nvram   "bcm94329OLYMPICN18.txt" \ for 4329b1-N18 only
# alternate nvram file location
#     -nvram "4329b1/bcm94329OLYMPICN18.txt"
#     -type "4329c0/sdio-g-cdc-roml-reclaim-full11n-wme-idsup-pool-assert.bin" \

# clones
43291sdio clone 43291DASH -tag ROMTERM3_REL_4_220_??
43291sdio clone 43291RT2Twig\
    -tag RT2TWIG46_REL_4_221_??\
    -type "4329c0/sdio-g-cdc-full11n-reclaim-wme-idsup-assert.bin"

# Olympic external image with Olympic 43291 ref board
#     
43291sdio clone 43291RT2Olympic\
    -customer "olympic"\
    -brand "linux-external-dongle-sdio"\
    -tag RT2TWIG46_REL_4_221_???\
    -nvram "4329c0/bcm94329OLYMPICUNO.txt"\
    -type "4329c0/sdio-g-cdc-full11n-reclaim-roml-idsup-nocis-wme-apsta-memredux16-pno-aoe-pktfilter-medioctl-proptxstatus-uno-wapi-keepalive.bin"    
#    -type "4329c0/sdio-g-cdc-full11n-reclaim-wme-idsup-assert.bin"    

# for Sundance release: RC120, current tab RC121

43291sdio clone 43291RT2Olympic_Sundance\
    -customer "olympic"\
    -brand "linux-external-dongle-sdio"\
    -tag RT2TWIG46_REL_4_221_120\
    -nvram "4329c0/bcm94329OLYMPICUNO.txt"\
    -type "4329c0/sdio-g-cdc-full11n-reclaim-roml-idsup-nocis-wme-apsta-memredux16-pno-aoe-pktfilter-medioctl-proptxstatus-uno-wapi-keepalive.bin"    
#    -type "4329c0/sdio-g-cdc-full11n-reclaim-wme-idsup-assert.bin"   

43291RT2Olympic_Sundance clone 43291RT2Olympic_Brighton -tag RT2TWIG46_REL_4_221_122

43291sdio clone 43291RT2Olympic_Dbg\
    -customer "olympic"\
    -brand "linux-external-dongle-sdio"\
    -tag RT2TWIG46_REL_4_221_94\
    -nvram "4329c0/bcm94329OLYMPICUNO.txt"\
    -type "4329c0/sdio-g-cdc-full11n-reclaim-roml-idsup-nocis-wme-apsta-memredux16-pno-aoe-pktfilter-medioctl-proptxstatus-uno-wapi-keepalive.bin"  

# 4345 epa sdio BCM94345FCSAGB_1 rev 05   
        
43291sdio clone 4345sdio\
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

4345sdio clone 4345sdio_mfg -brand linux-mfgtest-dongle-sdio -type 4345a0-roml/sdio-ag-mfgtest-seqcmds
4345sdio clone 43451sdio -type 43451a0-roml/sdio-ag-mfgtest-seqcmds
43451sdio clone 43451sdio_ext -brand linux-external-dongle-sdio -type 43451a0-roml/sdio-ag
4345sdio clone 43451sdio_mfg -type 43451a0-roml/sdio-ag-mfgtest-seqcmds
	
4345sdio clone 4345sdio_ext -brand linux-external-dongle-sdio -type 4345a0-roml/sdio-ag.bin

4345sdio clone 4345sdio_p\
	-type /home/pkwan/Frank_4345/Frank_4345/rtecdc_4345.bin\
	-dhd_image /home/pkwan/Frank_4345/Frank_4345/dhd.ko

4345sdio clone 4345x -brand linux-external-dongle-sdio -type 4345a0-roml/sdio-ag.bin -dhd_tag NIGHTLY -dhd_brand linux-external-dongle-sdio 
# # # -driver dhd-cdc-std
# # # -dhd_brand linux-external-dhd 

# # #     -wlinitcmds {wl vht_features 3} \    
# # #     -channelsweep {-bw 20}\
# # #     -perfchans {36 3} -nopm1 1 -nopm2 1

# # # 	-dhd_brand linux-internal-dongle\    
# # #     -driver dhd-msgbuf-pciefd-debug \
# # #     -dhd_tag NIGHTLY\

4345sdio clone 43457sdio\
 -nvram "src/shared/nvram/bcm943457fcsagb_epa.txt"\
 -brand linux-external-dongle-sdio\
 -customer bcm\
 -type 43457a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-lpc-wl11u-txbf-assert-err
 
43457sdio clone 43457coex -wlinitcmds {wl vht_features 3; wl btc_mode 1}
    
# hostname mc54tst2, ip_add 10.19.87.24

UTF::WinBT MC54_BTCOHOSTXP\
	-lan_ip 10.19.87.24\
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
    
#     changed on 9/30/11 to pick up the latest BT builds
#     -version *112.0000
    
# 	-power "10.19.85.207 2"\
#   -device_reset "10.19.85.200 2" \  
   	  
#   -version * \;# -> picking up the latest firmware
# 	-version *0672.0000

# this is latest firmware 

mc54_BTCohost clone BTCohost_Nok -brand Nokia/Module_TDK -version *
# mc54_BTCohost clone BTCohost_new -version *
mc54_BTCohost clone BTCohost_new -version *111.0000
mc54_BTCohost clone BTCohost_latest -version * -file *.cgs
# # mc54_BTCohost clone BTCohost_Olympic -version * -file *.cgs -brand Olympic_Uno/Murata
mc54_BTCohost clone BTCohost_Olympic -version * -file *.cgs -brand Olympic*/{Uno/}Murata
mc54_BTCohost clone BTCohost_Olympic_Sundance -version BCM43291A0*_MUR_* -brand "" -file *.cgs

mc54_BTCohost clone BTCohost43457\
	-bt_comm "com10@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -project_dir "/home/pkwan"\
    -type BCM4345/A0\
    -brand Generic/UART/37_4MHz/fcbga_BU\
    -bt_ss_location 0x0021B000\
    -version *\
    -file *.cgs
    
BTCohost43457 clone BTCohost43457_local -image /home/pkwan/BCM43457/A0/BCM43457A0_001.001.024.0008.0000_Generic_UART_37_4MHz_TSMC_A0_FCBGA_ref_eLNA.cgs

# 	ben's request starting 1/10/11; test result sent 1/10/11
# watch for MOS score below 3.5, as seen in run on 1/11/11: 20110111061012/summary.html
# 	-brand Generic_FMRX/38_4MHz \
# 	-version *0672.0000

# 	Ben's request from 12/8/10 till 1/10/11
    # 	-brand Generic_FMRX/38_4MHz \
    # 	-version *0589.0000
	
# 	-bt_comm "com3@3000000" \; DUCKS2 uses com4

# ben's request, 12/7/10; results gathered and reported 12/8/10
#     -brand Generic_FMRxTx/38_4MHz \
# 	-version *0672.0000
		
#   -brand Generic_FMRX/38_4MHz \ ;# this should go with the line, below
# 	-version *0589.0000 ;# changed to 0672 on 12/7/10, at Ben's request
	
#    -brand Olympic_N18/Module \; for 4329B1-N18 only
#    -version *0389.0000 ;for 4329b1-N18 only
    
#   -power "10.19.85.207 2"\

# BT Ref - WinXP laptop BlueTooth 2046 Reference
# mc24tst1  10.19.85.197   BtRef_XP     - TB6BTREF
#	-power_sta "10.19.85.199 1"\

UTF::WinBT MC54_BTREFXP\
	-lan_ip 10.19.87.23\
    -sta "mc54_BTRefXP"\
	-power_sta "10.19.87.26 1"\
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user

# 4334a2 installed 8/16/2011: sta=mc59end8
# changed to 4334B1 as of 11/10/2011
		
UTF::DHD mc54DUT2 \
	-lan_ip 10.19.87.102 \
	-sta "4334sdio eth1" \
	-power "172.16.54.25 2" \
	-power_sta "172.16.54.25 1"\
	-device_reset "10.19.87.110 1"\
 	-console "mc59end8:40000"\
	-tag "PHOENIX_BRANCH_5_120" \
    -brand "linux-internal-dongle" \
    -customer "bcm" \
	-type "4334a0-roml/sdio-ag-pool-idsup*.bin"\
    -nvram "src/shared/nvram/bcm94334fcagb.txt"\
    -postinstall {dhd -i eth1 serial 1}\
    -pre_perf_hook {{AP1-4717 wl ampdu_clear_dump}}\
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4717 wl dump ampdu}}\
    -wlinitcmds {wl phy_crs_war 1}
#  	-console "mc61end15:40000"\
#     	-console "mc59end6:40000"\    
# 	-console "mc59end8:40001"\
#   -wlinitcmds {wl phy_crs_war 1} ;# added as a workaround for 4334 issue PR#97114, per TimA
    
#   -power --> power to DUT; -power_sta --> power to STA host
    
# 	-device_reset "10.19.87.100 2"\   
# 	-power_sta "172.16.54.25 1"\
 
4334sdio clone 4334rel -tag "PHOENIX_REL_5_120_37"
4334sdio clone 4334g -type "4334a0-roml/sdio-g-pool.bin"

# received 4334B1 board on 10/19/11
# 4334sdio clone 4334B1 -tag PHOENIX2_REL_6_10_15 -type "4334b1-roml/sdio-ag-idsup.bin" -wlinitcmds ""
4334sdio clone 4334B1 -tag PHOENIX2_REL_6_10_?? -type "4334b1-roml/sdio-g-idsup.bin" -wlinitcmds ""
4334sdio clone 4334B1DASH -tag PHOENIX2_REL_6_10_17 -type "4334b1-roml/sdio-g-idsup-p2p.bin" -wlinitcmds "" -console "mc59end8:40001" -postinstall "dhd -i eth1 serial 1"
4334sdio clone 4334B1ToB -tag PHOENIX2_BRANCH_6_10 -type "4334b1-roml/sdio-g-idsup-p2p.bin" -wlinitcmds "" -console "mc59end8:40000" -postinstall "dhd -i eth1 serial 1"

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

#   -bt_comm "com4@3000000"  ;# embedded adapter com4; DUCKS2 com6
# 	-lan_ip 10.19.85.153\ ;# ip address for mc61end15 -- temporary
        
# 	-bt_comm "com4@3000000"\ ;# com6 in mc61end15
#     -type BCM4334/A0\ ;# device replaced with 4334B1 as of 10/21/11 
#    -brand Generic_37_4MHz/fcbga\ ;# B1 device is fcbga_extLna   
        
#  	-type bcm4334a0\;# 4334 BT firmware online as of 10/14/11
#     -brand Generic_37_4MHz/fcbga\
#     -bt_ss_location 0x00210000\
#     -version *
#     \;# 4334 BT firmware online as of 10/14/11
#     -project_dir "/home/pkwan"

#    -type BCM4334/A0\

mc54_BTCohost2 clone BTCohost2DASH
mc54_BTCohost2 clone BTCohost2 -brand Generic/37_4MHz/fcbga_extLna

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
AP1-4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
AP1-4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
AP1-4717 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router