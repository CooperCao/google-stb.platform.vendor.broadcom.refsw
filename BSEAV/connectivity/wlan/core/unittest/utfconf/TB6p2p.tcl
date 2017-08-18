# -*-tcl-*-
# ====================================================================================================

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

# set ::bt_dut BTCohost_4329USB      	;# BlueTooth device under test
set ::bt_dut BTCohost               	;# BlueTooth device under test
set ::bt_ref BTRefXP      			    ;# BlueTooth reference board
set ::wlan_dut 4329b1_Ref       	    ;# HND WLAN device under test
# set ::wlan_dut STA-Linux-4329sdio  	;# HND WLAN device under test
set ::wlan_rtr AP1-4322-4717   		    ;# HND WLAN router
set ::wlan_tg lan              		    ;# HND WLAN traffic generator

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/CoExTB6"

# Define power controllers on cart.
# package require UTF::Power
# - btref webrelay 10.19.85.199 pwr for 2046B1 ref
# device_reset case setting
UTF::Power::WebRelay 10.19.85.199 -invert "1"
# power_sta case setting
# UTF::Power::WebRelay 10.19.85.199
# - dut webrelay 10.19.85.200 bt & wl reset lines
UTF::Power::WebRelay 10.19.85.200  ;# -invert "1 2"
# - GC bt & wl reset lines
UTF::Power::WebRelay 172.16.24.20
# AP pwr control & console
UTF::Power::Synaccess 10.19.85.209
# replaced old dead NPC-22 to rev 1 7/24/14
UTF::Power::Synaccess 10.19.85.207 -rev 1
UTF::Power::Synaccess pwrCtl -lan_ip 10.19.85.152 ;# hostname mc61end14

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.24.10 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {

    G1 attn 0 ;# 0 from 25 01/28/15 ;# changed to 25 from 5 12/15/14; changed to 5 from 15 6/6/14 ;# 
	G2 attn 20 ;# changed to 20 from 0 12/15/14 ;
	G3 attn 103
#     ALL attn 0

	# turn off STA radios
    foreach S {DUT3 DUT4} {
		if {[catch {$S wl down} ret]} {
	        error "SetupTestBed: $S wl down failed with $ret"
	    }
	    $S deinit
	}
	
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# mc24end1  10.19.85.195   Linux UTF Host   - TB6UTF
# -sta "lan eth0"

UTF::Linux TB6UTF  -lan_ip 10.19.85.195  -sta "lan eth1"

UTF::WinBT TB6_BTCOHOSTXP2\
	-lan_ip mc61end12\
	-sta "BTCohost4334"\
	-bt_comm "com4@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_ss_location 0x00210000\
    -type BCM4334/B1\
    -brand ""\
    -version BCM4334*_Olympic_Cent_OS_MUR_*\
    -file *.cg*\
	-device_reset "172.16.24.20 2"
    
BTCohost4334 clone Cohost4350c0_GC\
	-bt_comm "com6@3000000"\
	-bt_comm_startup_speed 115200\
	-bt_ss_location 0x0021E000\
	-bt_xtal_freq 40\
    -type BCM4350/C0\
	-version BCM4350C*\
	-brand "Generic/UART/37_4MHz/wlbga_BU"\
	-file *.cg*

Cohost4350c0_GC clone Cohost4350c0_GC_reset -device_reset "10.19.87.110 2"
Cohost4350c0_GC clone Cohost4350c0_GC_cfg85 -version BCM4350C*0085*
Cohost4350c0_GC clone Cohost4350c4_stella -bt_comm "com8@3000000" -type BCM4350/C* -brand "" -version BCM4350C*_Riesling_OS_USI_ST_*
# Cohost4350c4_stella clone Cohost4350c4_stella_mon -bt_comm "com8@3000000" -type BCM4350/C* -brand "" -version BCM4350C2_*311.*Riesling_OS_USI_ST_*
Cohost4350c4_stella clone Cohost4350c4_stella_mon -bt_comm "com8@3000000" -type BCM4350/C* -brand "" -version BCM4350C2_*Riesling_OS_USI_ST_*

# # # 	-lan_ip 10.19.85.149\ ;# host name mc61end11
### 4350c4 Stella ES3.9

UTF::DHD TB6DUT3\
	-lan_ip mc61end11\
	-sta "DUT3 eth0"
	
DUT3 clone stella\
	-app_tag BIS120RC4PHY_BRANCH_7_16 \
    -tag BIS120RC4PHY_BRANCH_7_16 \
    -dhd_tag trunk \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
	-nvram bcm94350GamayMurataMT.txt\
    -customer olympic \
    -type ../C-4350__s-C2/rieslingb.trx -clm_blob rieslingb.clmb \
    -postinstall {dhd -i eth0 dma_ring_indices 3}\
	-power "pwrCtl 2"\
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1 \
    -tcpwindow 1152k -udp 400m \
    -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}

# # # 	-device_reset "172.16.24.20 1"\
# # #     -nvram "bcm94350RieslingBMurataMT.txt" \
    
    
stella clone stella_rel\
	-tag BIS120RC4_REL_7_15_*\
	-nvram ../C-4350__s-C4/P-rieslingb_M-STEL_V-m__m-4.13.txt\
	-type ../C-4350__s-C4/rieslingb.trx -clm_blob rieslingb.clmb
# 	-type ../C-4350__s-C2/rieslingb.trx -clm_blob rieslingb.clmb 
	
stella_rel clone stella_rel_coex -wlinitcmds "wl btc_mode 1"

# stella_rel clone stella_mon -tag BIS715T185RC1_REL_7_35_???
stella_rel clone stella_mon -tag BIS735T255RC1_REL_7_47_* -dhd_tag DHD_REL_1_359_39
stella_mon clone stella_mon_coex -wlinitcmds "wl btc_mode 1"
stella_mon clone stella_mon_dbg -dhd_date 2015.5.11.

stella_rel clone stella_ToB -tag BIS715T185RC1_BRANCH_7_35
stella_ToB clone stell_ToB_coex -wlinitcmds "wl btc_mode 1"


UTF::DHD TB6DUT\
	-lan_ip 10.19.85.196\
	-sta "STA-Linux-4329sdio eth1"\
	-tag "ROMTERM2_BRANCH_4_219"\
	-power "10.19.85.207 1"\
    -brand "linux-internal-dongle"\
    -customer "bcm"\
    -type "4329b1/sdio-ag-cdc-full11n-reclaim-roml-wme-idsup.bin"\
    -nvram "src/shared/nvram/bcm94329sdagb.txt"\
    -pre_perf_hook {{AP1-4322-4717 wl ampdu_clear_dump}}\
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4322-4717 wl dump ampdu}}

# # #   -power_sta "10.19.85.200 3"\ ; controls USB power to DUCKS2
# # # 	-device_reset "10.19.85.200 2"\
    
# clones of TB6DUT
STA-Linux-4329sdio clone 4329sdio_ROMTERM3 -tag "ROMTERM3_BRANCH_4_220" -nvram "bcm94329sdagb.txt"
STA-Linux-4329sdio clone 4329sdio_RT3Ext -tag "ROMTERM3_BRANCH_4_220" -brand "linux-external-dongle-sdio"
STA-Linux-4329sdio clone 4329sdio_ROMTERM -tag "ROMTERM_BRANCH_4_218"
STA-Linux-4329sdio clone 4329b1_Ref -tag "ROMTERM_REL_4_218_???"
STA-Linux-4329sdio clone 4329b1_Ref_RvR -tag "ROMTERM_REL_4_218_???" -wlinitcmds {wl btc_mode 0}
STA-Linux-4329sdio clone 4329b1_DASH -tag ROMTERM3_REL_4_220_??
STA-Linux-4329sdio clone 4329b1_RT2Twig\
  -tag RT2TWIG46_REL_4_221_???\
  -type "4329b1/sdio-ag-cdc-full11n-reclaim-roml-idsup-nocis-wme-memredux16-pno-aoe-pktfilter-minioctl-29agbf-keepalive.bin"
  
STA-Linux-4329sdio clone 43241b0\
   -tag PHOENIX2_REL_6_10_116_*\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-opt2.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -wlinitcmds {wl btc_mode 0}\
   -datarate {-skiptx 32}\
   -postinstall {dhd -i eth1 sd_divisor 1}

43241b0 clone 43241b0x -tag PHOENIX2_REL_6_10_116_6
43241b0 clone 43241b0d\
   -dhd_image "/projects/hnd_software_ext7/work/maheshd/code/tot/trunk/src/dhd/linux/dhd-cdc-sdstd-2.6.25-14.fc9.i686/dhd.ko"
43241b0 clone 43241b0roml\
   -type "43241b0-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-opt2-phydbg-autoabn.bin"
43241b0roml clone 43241b0romld\
   -dhd_image "/projects/hnd_software_ext7/work/maheshd/code/tot/trunk/src/dhd/linux/dhd-cdc-sdstd-2.6.25-14.fc9.i686/dhd.ko"
43241b0 clone 43241b0_pwr -power_sta "10.19.85.200 3" 

# ip_addr 10.19.85.196, host name mc24end2


UTF::DHD TB6DUT4\
	-lan_ip mc24end2\
	-sta {DUT4 eth0}
	
DUT4 clone 4350c4\
	-app_tag BIS120RC4PHY_BRANCH_7_16 \
    -tag BIS120RC4PHY_BRANCH_7_16 \
    -dhd_tag trunk \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram ../C-4350__s-C4/P-rieslinga_M-CIDR_V-u__m-2.9.txt \
    -customer olympic \
    -type ../C-4350__s-C2/rieslinga.trx -clm_blob rieslinga.clmb \
    -postinstall {dhd -i eth0 dma_ring_indices 3}\
	-power "10.19.85.207 1"\
	-datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1 \
    -tcpwindow 1152k -udp 400m \
    -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}
# # # 	-device_reset "10.19.85.200 2"\
    
4350c4 clone 4350c4_coex -wlinitcmds "wl btc_mode 1"
4350c4 clone 4350c4_7.15\
    -app_tag BIS120RC4_BRANCH_7_15 \
    -tag BIS120RC4_BRANCH_7_15\
    -type ../C-4350__s-C4/rieslinga.trx -clm_blob rieslinga.clmb 
4350c4_7.15 clone 4350c4_7.15_coex -wlinitcmds "wl btc_mode 1"

4350c4 clone 4350c4_rel\
	-tag BIS120RC4_REL_7_15_*\
	-type ../C-4350__s-C4/rieslinga.trx -clm_blob rieslinga.clmb
	
4350c4_rel clone 4350c4_rel_coex -wlinitcmds "wl btc_mode 1"

# Monarch twig changed to 7.47.x 
# 4350c4_rel clone 4350c4_mon -tag BIS715T185RC1_REL_7_35_???
4350c4_rel clone 4350c4_mon -tag BIS735T255RC1_REL_7_47_* -dhd_tag DHD_REL_1_359_39
4350c4_mon clone 4350c4_mon_coex -wlinitcmds "wl btc_mode 1"
4350c4_mon clone 4350c4_mon_dbg -dhd_date 2015.5.11.

# changed ToB from 7.35 to 7.47
# 4350c4_rel clone 4350c4_ToB -tag BIS715T185RC1_BRANCH_7_35
4350c4_rel clone 4350c4_ToB -tag BIS735T255RC1_BRANCH_7_47
4350c4_ToB clone 4350c4_ToB_coex -wlinitcmds "wl btc_mode 1"

# BT Cohost - WinXP laptop BlueTooth 4329 USB
# mc24tst2  10.19.85.198   Vista_BtCoHost   - TB6BTCOHOST
# The line below this must remain blank or you will get this msg: invalid command name "TB6BTCOHOST"!!

UTF::WinBT TB6BTCOHOST\
	-lan_ip mc24tst2\
	-sta "BTCohost_4329USB"\
	-bt_comm "com3@3000000"\
	-power "10.19.85.207 2"\
   	-device_reset "10.19.85.200 1"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -type BCM4329/B1\
    -brand Generic_FMRX/38_4MHz\
    -bt_ss_location 0x00087410\
    -version *672.0000

# this is latest firmware

BTCohost_4329USB clone BTCohost -version *
BTCohost_4329USB clone BT831 -version *831.0000
BTCohost_4329USB clone BT875 -version *875.0000

BTCohost_4329USB clone BTCohost43241\
	-bt_comm "com4@3000000"\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -type BCM43241/B0\
    -brand "Generic/37_4MHzClass1"\
    -bt_ss_location 0x00210000\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
    -file *.cgr \
    -version *

BTCohost43241 clone BTCohost43241_local\
	-project_dir "/home/pkwan"\
	-type BCM43241\
	-brand ""\
	-version *\
	-file *.cg*

# # # clones
		
BTCohost43241_local clone BTCohost43241_cfg53 -version BCM43241B0_002.001.013.0053.0000 -brand "Generic/37_4MHzClass1"
BTCohost43241_local clone BTCohost43241_cfg32 -version BCM43241B0_002.001.013.0032.0000
BTCohost43241_local clone BTCohost43241_cfg31 -version BCM43241B0_002.001.013.0031.0000
BTCohost43241_local clone BTCohost43241_cfg30 -version BCM43241B0_002.001.013.0030.0000
BTCohost43241_local clone BTCohost43241_cfg29 -version BCM43241B0_002.001.013.0029.0000
BTCohost43241_local clone BTCohost43241_cfg24BigBuff -version BCM43241B0_002.001.013.0024.0000
BTCohost43241 clone BTCohost43241_cfg24 -version BCM43241B0_002.001.013.0024.0000 
         
BTCohost_4329USB clone BTCohost4334B1\
	-bt_comm "com9@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_ss_location 0x00210000\
    -type BCM4334/B1\
    -brand Generic/37_4MHz/fcbga_extLna\
    -version *
    
BTCohost_4329USB clone Cohost4350c0\
	-bt_comm "com10@3000000"\
	-bt_ss_location 0x0021E000\
	-bt_xtal_freq 40\
    -type BCM4350/C0\
	-version BCM4350C*\
	-brand "Generic/UART/37_4MHz/wlbga_BU"\
	-file *.cg*
	
# # # Cohost4350 clone Cohost4350c0 -type BCM4350/C0 -version BCM4350C*
# # # Cohost4350 clone Cohost4350c0 -bt_ss_location 0x0021E000 -type BCM4350/C0 -version BCM4350C*
Cohost4350c0 clone Cohost4350c0_cfg51 -version *0051*
Cohost4350c0 clone Cohost4350c0_cfg85 -version BCM4350C*0085*
Cohost4350c0 clone Cohost4350c0_reset -device_reset "10.19.85.200 1"
Cohost4350c0 clone Cohost4350c4 -bt_comm "com13@3000000" -type BCM4350/C* -brand "" -version BCM4350C*_Riesling_OS_USI_STC*
Cohost4350c4 clone Cohost4350c4_cfg311 -bt_comm "com13@3000000" -type BCM4350/C* -brand "" -version BCM4350C*_*311*_Riesling_OS_USI_STC*
Cohost4350c4 clone Cohost4350c4_dbg -bt_comm "com13@3000000" -type BCM4350/C* -brand "" -version BCM4350C*_Riesling_OS_USI_STC*2014*

# BT Ref - WinXP laptop BlueTooth 2046 Reference
# mc24tst1  10.19.85.197   BtRef_XP     - TB6BTREF
#	-power_sta "10.19.85.199 1"

UTF::WinBT BTREFXP\
	-lan_ip mc24tst1\
    -sta "BTRefXP"\
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user
# 	-power_sta "10.19.85.199 1"\
# 	-device_reset "10.19.85.199 1"\

#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router TB6AP1\
    -lan_ip 192.168.1.2\
    -sta "AP1-4322-4717 eth1 AP1-P2P eth2"\
    -relay "TB6UTF"\
    -lanpeer lan\
    -console "10.19.85.195:40000"\
    -power "10.19.85.209 1"\
    -brand "linux-external-router-combo"\
    -tag "COMANCHE2_REL_5_22_\?\?"\
    -date "*"\
   -nvram {
	   lan_ipaddr=192.168.1.2
	   lan_gateway=192.168.1.2
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestCoex6
		wl0_channel=1
		wl0_radio=0
		w10_nmode=1
		wl0_antdiv=0
		antswitch=0
		wl0_obss_coex=0
		wl1_ssid=TestCoex6P2P
		wl1_chanspec=36
		wl1_radio=0
		wl1_nmode=1
		lan1_ipaddr=192.168.2.10
		lan1_gateway=192.168.2.10
		dhcp1_start=192.168.2.110
		dhcp1_end=192.168.2.119
    }
   

AP1-4322-4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
AP1-4322-4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
# # # AP1-4322-4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
AP1-4322-4717 clone 4717MILLAU -tag "MILLAU_REL_5_70*" -brand linux-external-router
AP1-4322-4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
AP1-4322-4717 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
AP1-4322-4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
AP1-4322-4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
AP1-4322-4717 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

4717Akashi clone TB6_AP1
4717Akashi clone tb6_AP1
4717Akashi clone MC24_AP1
4717Akashi clone mc24_AP1

unset UTF::TcpReadStats
