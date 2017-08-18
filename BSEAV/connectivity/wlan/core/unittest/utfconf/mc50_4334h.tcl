# -*-tcl-*-
#
# testbed configuration for Peter Kwan MC50 testbed
# based on mc32
# created 7/29/10
# changes 11/2/10:
#  added device info on top of file 
# ====================================================================================================
# Test rig ID: SIG MC50
# MC50 testbed configuration
#
# mc50end1  10.19.86.198   Endpoint1   		- lan0
# mc50end2  10.19.86.199   Endpoint2       	- lan1
# mc50tst1  10.19.86.200   <Available>		- Cube 24018E
# mc50tst2  10.19.86.201   <Available>		- NPC-22 for HSIC bring up
# mc50tst3  10.19.86.202   BCM43224			- 43224Vista
# mc50tst4  10.19.86.203   BCM94319SDELNA6L	- 4319sdio
# mc28tst5	10.19.84.248   BCM94336SDGFC B1	- 4336b1
# mc50tst6  10.19.86.206   sniffer			- snif
# 			172.16.50.10   npc1
# 			172.16.50.11   npc2
# 			172.16.50.22   npc3
# 			172.16.50.21   Aeroflex			- af

# Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD
package require UTF::WinBT
# package require UTF::Sniffer

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut BTCohost4334h		      	;# BlueTooth device under test
set ::bt_ref BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 4334hInnsbruck  			;# HND WLAN device under test
set ::wlan_rtr mc50_AP1   				;# HND WLAN router
set ::wlan_tg lan0              		;# HND WLAN traffic generator

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}
# -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

# SummaryDir sets the location for test results
### set ::UTF::SummaryDir "/projects/hnd_sig_ext4/$::env(LOGNAME)/mc50"
# moved to dedicated filer volume 11/1/13
set ::UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc50"

# list of devices and IP addresses assigned
# 10.19.86.198	mc50end1
# 10.19.86.199	mc50end2
# 10.19.86.200	mc50tst1
# 10.19.86.201	mc50tst2
# 10.19.86.202	mc50tst3 <-- 43224Vista
# 10.19.86.203	mc50tst4 <-- 4319sdio
# 10.19.86.204	mc50tst5 <-- 4336b1
# 10.19.86.205	mc50tst6 <-- sniffer

# NPC definitions
UTF::Power::Synaccess npc1 -lan_ip 172.16.50.10
UTF::Power::Synaccess npc2 -lan_ip 172.16.50.11 ;# enclosure 1 --> 4319SDIO
UTF::Power::Synaccess npc3 -lan_ip 172.16.50.22 -rev 1 ;# enclosure 2 --> 43224Vista (replaced with new Rev 1 unit 12/10/12)
UTF::Power::Synaccess mc28_npc3 -lan_ip 172.16.0.3 ;# for sniffer when installed in MC28

UTF::Power::WebRelay 172.16.50.30 ;# webrelay for 4334b1 sdio
UTF::Power::WebRelay 172.16.50.40 ;# webrelay for BTRef

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.50.21 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}

# Test bed initialization 
# set Aeroflex attenuation, and turn off radio on APs
set ::UTF::SetupTestBed {
	
  	G1 attn 15 ;# changed to 10 from 20 9/20/13; t0 15 from 5 12/4/14
	G2 attn 20 ;# changed to 20 from 103 10/5/13
	G3 attn 103

    foreach S {4334b1 4334h} {
	  catch {$S wl down}
	  $S deinit
    }
    
    foreach H {BTCohost BTRefXP} {
	  $H deinit
  }
	
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# # # G1 createGroup G1h {1 2}

# UTF Endpoint - Traffic generators (no wireless cards)
# use GigE interface eth2 to connect to LAN side of AP
# both end points are FC-9, tcpslowstart 4 does not apply
UTF::Linux mc50end1 \
    -sta {lan0 eth1} 

# second endpoint
UTF::Linux mc50end2 \
	-sta {lan1 eth1}

# UTF::Linux mc50tst1 \
#     -sta {lan2 eth1} 
# lan2 configure -ipaddr 192.168.1.239

UTF::Linux mc60end14 -sta "lan2 eth0"

# sniffer
# uses wireless interface eth1
# # # package require UTF::Sniffer ;# already included at top of file
# # # UTF::Sniffer mc50tst6 -sta {snif eth1} \
# # # 	-power {mc28_npc3 2} \
# # # 	-power_button {auto} \
# # # 	-tag BASS_REL_5_60_106 

# use KIRIN ToB, instead of ToT, as of 10/12/10
	#-power {npc3 2} \ ;# local npc22 in MC50

# clone snif; sniffer is not cloneable
# snif clone snif-BASS-REL -tag BASS_REL_5_60_106
# snif clone snif-KIRIN-REL -tag KIRIN_REL_5_100_82_12
# snif clone snif-KIRIN -tag KIRIN_BRANCH_5_100

# hostname mc50tst2, ipadd 10.19.86.201
   
UTF::WinBT BTCOHOST\
	-lan_ip 10.19.86.201\
	-bt_comm "com4@3000000"\
	-bt_comm_startup_speed 115200\
	-sta "BTCohostXP"\
    -user user\
    -device_reset "172.16.50.30 2"

BTCohostXP clone BTCohost43455\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -bt_ss_location 0x00219000\
    -type BCM4345/C0\
    -brand Generic/UART/37_4MHz/wlbga_ref_iLNA_iTR_eLG\
    -version *\
    -file *.cgr

BTCohost43455 clone BTCohost43455_cfg22 -version BCM4345C0_*22.0000
BTCohost43455 clone BTCohost43455_cfg33 -version BCM4345C0_*33.0000

BTCohost43455 clone BTCohost43455_local\
	-image /home/pkwan/BCM4345/C0/BCM4345C0_003.001.025.0021.0000_Generic_UART_37_4MHz_wlbga_ref_iLNA_iTR_eLG.hcd
    
BTCohostXP clone BTCohost4334h\
	-bt_comm "com5@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_ss_location 0x00210000\
    -type BCM4334/B1\
    -brand ""\
    -version BCM4334B1*Cent*OS_MUR*\
    -file *.cgs

# hostname mc50tst3, ip_add 10.19.86.202    
    			
UTF::WinBT BTREFXP\
     -lan_ip mc50tst3\
     -sta "BTRefXP"\
     -type "BCM2046"\
     -bt_comm "usb0"\
     -user user

UTF::WinBT BTReset\
 -lan_ip mc50tst3\
 -sta "BTRefXP_reset"\
 -type "BCM2046"\
 -bt_comm "usb0"\
 -user user\
 -device_reset "172.16.50.40 2"
# \
# -power_sta "172.16.50.40 1"

# hostname mc61end8, ipadd 10.19.87.122

UTF::WinBT BTCOHOST2\
	-lan_ip 10.19.87.122\
	-sta "BTCohost2"\
   	-bt_comm "com24@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -type BCM4330/B1\
    -bt_ss_location 0x00210000\
	-brand Generic/37_4MHz/fcbga\
	-version BCM4330B1_*\
	-file *.cgs

BTCohost2 clone Cohost2WLSD -version BCM4330B1_* -brand Generic/37_4MHz/wlbga -bt_comm "com21@3000000"

# configuration used for device name mc61end11, ip_add 10.19.85.149
# HSIC host is: mc28tst5 IPADDR 10.19.84.248

UTF::HSIC hExp\
	-sta {4334h eth1}\
	-lan_ip 192.168.1.1\
	-power {npc3 2}\
    -relay 4334HSIC\
    -lanpeer 4334HSIC\
    -console "mc28tst5:40000"\
    -nvram "Sundance/centennial.txt"\
    -tag "PHOENIX2_BRANCH_6_10"\
    -type "Sundance/centennial.bin.trx"\
    -brand "linux-external-dongle-usb" -customer "olympic"\
    -host_brand "linux26-internal-hsic-olympic"\
    -host_tag "RTRFALCON_REL_5_130_28"\
    -host_nvram {
        lan_ipaddr=192.168.1.1
        wandevs=dummy
        clkfreq=530,176,88
        watchdog=6000
        console_loglevel=7
        lan_stp=0
        lan1_stp=0
    }\
    -wlinitcmds {wl event_msgs 0x10000000000000}\
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}}\
    -nocal 1

4334h clone 4334hSundance\
	-tag "PHOENIX2_REL_6_10_56_???"\
	-type "Sundance/centennial.trx"\
	-nvram "/home/pkwan/BCM4334/centennial.txt"
    
# # # 4334hSundance clone 4334hBrighton\
# # # 	-type "Brighton/centennial.trx"
	
4334hSundance clone 4334hInnsbruck\
	-tag "PHO2203RC1_BRANCH_6_25"\
	-type "Innsbruck/{,4334b1/}centennial.trx"\
	-host_tag RTRFALCON_REL_5_130_48

# # # 4334hInnsbruck configure -attngrp G1h

4334hInnsbruck clone 4334hInnsbruck_0628 -tag "PHO2203RC1_REL_6_25_61_8"
4334hInnsbruck clone 4334hInnsbruck_0712 -tag "PHO2203RC1_REL_6_25_63_9"
4334hInnsbruck clone 4334hInnsbruck_0726 -tag "PHO2203RC1_REL_6_25_63_*"
4334hInnsbruck clone 4334hInnsbruck_twig63 -tag "PHO2203RC1_REL_6_25_63_*"
4334hInnsbruck clone 4334hInnsbruck_current -tag "PHO2203RC1_REL_6_25_*"
4334hInnsbruck clone 4334hOkemo -type "*/{,4334b1/}centennial.trx"
4334hOkemo clone 4334hOkemo_coex -wlinitcmds {wl event_msgs 0x10000000000000; wl btc_mode 1}
4334hOkemo clone 4334b1h\
	-tag PHO2203RC1_BRANCH_6_25 \
	-type "4334b1-roml/usb-ag-p2p-pno-nocis-keepalive-aoe-oob-idsup-idauth-err-assert-wapi-srsusp-ve-awdl-proptxstatus-cca-noclminc/rtecdc.bin.trx"\
	-nvram "src/shared/nvram/bcm94334OlympicCent_murata_mm.txt" \
    -nvram_add "htagc=1" \
    -brand "linux-external-dongle-usb" -customer "olympic" \
    -host_brand "linux26-internal-hsic-phoenix2" \
    -host_tag "RTRFALCON_REL_5_130_??"\
    -datarate {-skiptx 32}

4334b1h clone 4334b1h_coex -wlinitcmds {wl event_msgs 0x10000000000000; wl btc_mode 1}
4334b1h clone 4334b1h_rel -tag PHO2203RC1_REL_6_25_65_*\
	-type "4334b1-roml/usb-ag-p2p-pno-nocis-keepalive-aoe-oob-idsup-idauth-err-assert-wapi-srsusp-ve-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-wnm-noclminc-clm_4334_centennial.bin.trx"\
	-nvram bcm94334OlympicCent_murata_mm.txt
4334b1h_rel clone 4334b1h_rel_coex -wlinitcmds {wl event_msgs 0x10000000000000; wl btc_mode 1}

UTF::DHD HRELAY\
	-lan_ip 10.19.84.248\
	-sta "4334HSIC eth1" 

    
# STA4: Linux desktop; hostname mc50tst4
# replaced 4319b0 with 4334b1 SDIO 9/20/13

UTF::DHD mc50tst4 \
	-lan_ip mc50tst4 \
	-sta "sdio eth0" \
	-console "mc50tst4:40000" \
	-power_button {auto} \
	-device_reset "172.16.50.30 1"\
	-tcpwindow 1512k -udp 200m\
	-power {npc2 2} \
	-post_perf_hook {{%S wl nrate} {%S wl rate}} \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl rssi}} \
	-post_perf_hook {{%S wl scansuppress 0} {%S wl nrate} {%S wl rate} {%S rte mu}}

sdio clone 43455\
	-tag BISON_BRANCH_7_10\
	-brand hndrte-dongle-wl\
	-type 43455*-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-ltecx-wfds-sr-srfast-err-assert/rtecdc.bin\
	-nvram src/shared/nvram/bcm943457wlsagb.txt\
	-dhd_tag trunk\
	-dhd_brand linux-internal-dongle\
	-app_tag trunk\
	-noaes 1 -notkip 1 -nobighammer 1 -nocal 1\
	-datarate {-skiprx "0 32" -skiptx "0 32" -b 200m}

43455 clone 43455_06T\
        -tag BISON06T_BRANCH_7_45\
        -brand linux-external-dongle-sdio\
        -type 43455*-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-ltecx-wfds-sr-srfast-mchandump.bin
        
43455_06T clone 43455_06T_coex -wlinitcmds {wl btc_mode 1}
43455_06T clone 43455_06T_coex5 -wlinitcmds {wl btc_mode 5}

# Linksys 320N 4717/4322 wireless router.
UTF::Router AP1 \
    -sta "4717 eth1" \
	-lan_ip 192.168.1.3 \
    -relay "mc50end1" \
    -lanpeer lan0 \
    -console "mc50end1:40001" \
    -power {npc1 1} \
	-brand linux-external-router \
    -nvram {
        et0macaddr=00:90:4c:32:00:00
        macaddr=00:90:4c:32:01:0a
		lan_ipaddr=192.168.1.3
		lan_gateway=192.168.1.3
		dhcp_start=192.168.1.100
  		dhcp_end=192.168.1.149
		lan1_ipaddr=192.168.2.1
		lan1_gateway=192.169.2.1
		dhcp1_start=192.168.2.100
	    dhcp1_end=192.168.2.149
       	fw_disable=1
       	wl_msglevel=0x101
        wl0_ssid=mc50test
        wl0_channel=1
        wl0_radio=1
        antswitch=0
        wl0_antdiv=0
        wl0_obss_coex=0
}

# clone AP1s
4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_??" -brand linux-external-router
4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_??" -brand linux-internal-router
4717 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router-combo
4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
4717 clone 4717AkashiExt -tag "AKASHI_{REL,TWIG}_5_110_*" -brand linux-external-router
4717 clone 4717AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand linux-internal-router
4717Akashi clone mc50_AP1

unset UTF::TcpReadStats
