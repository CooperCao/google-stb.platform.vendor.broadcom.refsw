# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC59
# CoEx testbed configuration
#
# DUT: 43362 ;# replaces 4336B1 which is EoL

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut mc59_BTCohost2		      	;# BlueTooth device under test
set ::bt_ref mc59_BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 4335c0_epa_01T 		    ;# HND WLAN device under test
set ::wlan_rtr mc59_AP1   				;# HND WLAN router
set ::wlan_tg lan0              		;# HND WLAN traffic generator

# loaned ip addresses to MC54:
# 10.19.87.102 -- bcm94334 host
# 10.19.87.101 -- BTCohost
# 10.19.87.124 -- WebRelay DUT2

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
# moved to dedicated filer volume 10/30/13
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc59"

# Define power controllers on cart.
# package require UTF::Power
UTF::Power::Synaccess npc_DUT -lan_ip 172.16.59.20
UTF::Power::Synaccess npc_DUT2 -lan_ip 172.16.59.25
UTF::Power::Synaccess npc_Ref -lan_ip 172.16.59.30
UTF::Power::Synaccess npc_AP -lan_ip 172.16.59.40
UTF::Power::Synaccess npc_AP2 -lan_ip 172.16.59.60 -rev 1
UTF::Power::WebRelay 10.19.87.104 ;# WebRelay DUT
UTF::Power::WebRelay 10.19.87.103
UTF::Power::WebRelay 10.19.87.124 ;# WebRelay DUT2

# Attenuator
# G1: WLAN
# G2: BT
# G3: 11ac
UTF::Aeroflex af -lan_ip 172.16.59.10 -group {G1 {1 2 3} G2 {6} G3 {4 5} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
	G1 attn 20
    G2 attn 3
	G3 attn 103

   foreach A {mc59_AP1} {
	  catch {$A restart wl0_radio=0}
    }
    
    # silence DUT radios
    foreach S {43342h 4356} {
		if {[catch {$S wl down} ret]} {
	        error "SetupTestBed: $S wl down failed with $ret"
	    }
	    $S deinit
	}
    
	catch {mc59DUT rexec initctl start consoleloggerUSB}	
    
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# mc59end1  10.19.87.95   Linux UTF Host

UTF::Linux mc59end1 -sta "lan0 eth1" -lan_ip 10.19.87.95

### lan0 configure -ipaddr 192.168.1.144
# -lan_ip 10.19.87.95
#

# mc59DUT2: 4335c0 epa SDIO, mc60end5
#


# hostname: mc59end3

UTF::WinBT MC59_BTCOHOSTXP2 \
	-lan_ip 10.19.87.97 \
	-sta "mc59_BTCohost2"\
	-power "npc_DUT2 1"\
	-device_reset "10.19.87.124 2"\
	-bt_comm "com8@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -brand ""\
    -type BCM43362\
    -bt_ss_location 0x00226000\
	-type BCM2076/B1\
	-brand Nokia/UART/BringupBoard_3Wire\
    -version *\
    -file *.cgs
    
mc59_BTCohost2 clone BTCohost2
mc59_BTCohost2 clone BTCohost43362    
   
mc59_BTCohost2 clone BTCohost2_local\
    -project_dir "/home/pkwan"\
    -type BCM43362\
    -version *\
    -brand ""\
    -file "BCM2076B1_002.002.004.0032.0000_3wire.cgs"

mc59_BTCohost2 clone Cohost4335c0_ipa\
	-bt_comm "com15@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -bt_ss_location 0x00218000\
	-type BCM4339\
	-brand "Generic/UART/37_4MHz/wlbga_iPA"\
	-version *

Cohost4335c0_ipa clone Cohost4335c0_epa\
	-bt_comm "com15@3000000"\
	-brand "Generic/UART/37_4MHz/wlbga_ePA"\
	-type BCM4339\
	-version *	

Cohost4335c0_epa clone Cohost4335c0_epa_cfg60\
	-version *60.*
	
Cohost4335c0_epa clone Cohost4335c0_epa_cfg61\
	-version *61.*
	
Cohost4335c0_epa clone Cohost4335c0_epa_local\
	-project_dir "/home/pkwan"\
	-brand "*/UART/37_4MHz/wlbga_ePA"\
	-type BCM4339\
	-version *	
	
# #  hostname mc59end2; ip 10.19.87.96

UTF::DHD mc59DUT \
	-lan_ip 10.19.87.96\
 	-sta "4364 eth0"\
 	-power "npc_DUT 1"\
	-device_reset "10.19.87.104 1"

# clones of mc59DUT

4364 clone 4364_trunk\
    -dhd_brand linux-internal-media \
    -driver dhd-msgbuf-pciefd-media-debug \
    -nvram "bcm94364fcpagb_2.txt" \
    -type 4364a0-ram/config_pcie_release_norsdb_row/rtecdc.bin \
    -tcpwindow 4M \
    -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3} \
    -datarate {-i 0.5} -udp 1.2g \
    -perfchans {36/80 3}

4364 clone 4364b0\
	-tag DIN2915T250RC1_BRANCH_9_30\
	-dhd_tag DHD_REL_1_359_1*\
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-nvram "src/shared/nvram/bcm94364fcpagb_2.txt" \
	-type 4364b0-roml/config_pcie_debug/rtecdc.bin -clm_blob 4364b0.clm_blob \
	-app_tag trunk\
	-tcpwindow 4M \
	-wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3} \
	-datarate {-i 0.5} -udp 1.2g -channelsweep {-no2g40}
	
# # # 	-yart {-pad 23 -attn5g 25-63 -attn2g 40-63} \

4364b0 clone 4364b0_coex -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3; wl btc_mode 1}
# 4364b0 clone 4364b0_mimo_udm -type 4364b0-roml/config_pcie_release_mimo_udm/rtecdc.bin -clm_blob 4364b0.clm_blob
4364b0 clone 4364b0_mimo_udm \
	-brand hndrte-dongle-wl\
	-type 4364b0-roml/config_pcie_release_mimo_udm/rtecdc.bin \
	-clm_blob 4364b0.clm_blob \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag APPS_REL_1_*
4364b0_mimo_udm clone 4364b0_mimo_udm_coex -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3; wl btc_mode 1}
4364b0_mimo_udm clone 4364b0_mimo_udm_dbg -app_tag trunk
4364b0_mimo_udm clone 4364b0_mimo_udm_ext -brand linux-external-dongle-pcie

#4364b1 Harpoon module

4364 clone 4364b1 \
	-tag DIN2915T250RC1_BRANCH_9_30\
	-brand hndrte-dongle-wl\
	-dhd_tag DHD_BRANCH_1_579\
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-nvram "src/shared/nvram/bcm94364HarpoonMurataMM_ES4.txt" \
	-type 4364b1-roml/config_pcie_release_mimo_udm/rtecdc.bin \
	-clm_blob 4364b0.clm_blob \
	-app_tag APPS_REL_1_*\
	-tcpwindow 4M \
	-wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3} \
	-datarate {-i 0.5} -udp 1.2g -channelsweep {-no2g40}
	
4364b1 clone 4364b1_coex -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3; wl btc_mode 1}
	
	

UTF::WinBT MC59_BTCOHOSTXP \
    -lan_ip 10.19.85.153\
	-sta "mc59_BTCohost"\
	-bt_comm "com6@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -type BCM4324\
    -brand ""\
    -bt_ss_location 0x00210000\
    -file *.cgs \
    -version *

# # #     	-device_reset "10.19.87.104 2"\

# # #  as of 9/20/12 mc59_BTCohost is a WinXP desktop
# # #     -lan_ip 10.19.85.153 ;# replaced with 10.19.87.97 on 9/17/12 for debugging
    
mc59_BTCohost clone BTCohost43241\
	-type BCM43241/B0\
	-version *\
	-brand "Generic/37_4MHzClass1"\
	-file *.cgr
	
mc59_BTCohost clone BTCohost43241_local\
	-project_dir "/home/pkwan"\
	-type BCM43241\
	-brand ""\
	-version *\
	-file *.cg*
	
BTCohost43241 clone BTCohost43241_cfg53 -version BCM43241B0_002.001.013.0053.0000
BTCohost43241_local clone BTCohost43241_cfg55 -version BCM43241B0_002.001.013.0055.0000
BTCohost43241_local clone BTCohost43241_cfg32 -version BCM43241B0_002.001.013.0032.0000
BTCohost43241_local clone BTCohost43241_cfg31 -version BCM43241B0_002.001.013.0031.0000
BTCohost43241_local clone BTCohost43241_cfg30 -version BCM43241B0_002.001.013.0030.0000
BTCohost43241_local clone BTCohost43241_cfg29 -version BCM43241B0_002.001.013.0029.0000
BTCohost43241_local clone BTCohost43241_cfg24BigBuff -version BCM43241B0_002.001.013.0024.0000
BTCohost43241 clone BTCohost43241_cfg24 -version BCM43241B0_002.001.013.0024.0000
mc59_BTCohost clone BTCohost_pwr -power_sta "10.19.87.104 4"


BTCohost43241 clone BTCohost43342A1\
	-bt_comm "com9@3000000"\
    -bt_ss_location 0x00210000\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
	-type BCM43342/A1\
	-version BCM43342A1*Chardonnay_OS_TDK*\
	-brand ""\
	-file *.cg*

# # # 		-bt_comm "com8@3000000"\ ;# P220 HSIC router board, replaced by P407 com9 7/18/14
# # # 	-version BCM43342A1*Chardonnay_OS_USI*\ ;# current Imperial reworked board as of 7/22/14: TDK, not USI
BTCohost43342A1 clone BTCohost43342A1_cfg1055 -version BCM43342A1*1055*Chardonnay_OS_TDK*
		
BTCohost43342A1 clone BTCohost43342A1_local\
	-image "/home/pkwan/BCM43342/BCM43342A1_11.1.702.720_Chardonnay_OS_USI_20130830.cgs"
	
BTCohost43342A1 clone BTCohost4364\
	-bt_ss_location 0x0021B500 \
	-bt_comm "com13@3000000"\
	-bt_xtal_freq 37.4\
	-project_dir "/home/pkwan" \
	-type BCM4364 -version * -brand Generic/UART/37_4MHz/fcbga_ref_dLNA_ePA_bypass\
	-file *.cgr

BTCohost4364 clone BTCohost4364_alt
# -version BCM4364B0* -brand Generic/UART/37_4MHz/fcbga_ref_dLNA_ePA_bypass

# BT Ref - WinXP laptop BlueTooth 2046 Reference
# mc59end3  10.19.87.98   mc59_BTRefXP
#	-power_sta "10.19.85.199 1"\

UTF::WinBT MC59_BTREFXP\
    -lan_ip 10.19.87.98\
    -sta "mc59_BTRefXP"\
    -power "172.16.59.30 2"\
    -power_sta "172.16.59.30 1"\
    -device_reset "10.19.87.103 1"\
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user

#     -power "npc_Ref 1"\
#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router MC59AP2 \
    -lan_ip 192.168.1.3 \
    -sta "mc59_AP2 eth1" \
    -power "npc_AP 1"\
    -relay "mc59end1" \
    -lanpeer lan0 \
    -console "mc59end1:40000" \
    -tag "AKASHI_BRANCH_5_110"\
	-brand "linux-internal-router"\
   -nvram {
	   	lan_ipaddr=192.168.1.3
       	fw_disable=1
       	wl_msglevel=0x101
		dhcp_start=192.168.1.144
	    dhcp_end=192.168.1.149
       	wl0_ssid=TestMC59
      	wl0_radio=0
		wl0_channel=1
		w10_nmode=1
		wl0_antdiv=0
		antswitch=0
		wl0_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }

# 4706 AP with 4360 and 4331 cards ;# debug 12/16/14: backdate build to 2014.10.2.0

UTF::Router MC59AP1 -sta {
    4706/4360 eth1 4706/4360.%15 wl0.% 
    4706/4331 eth2
    } \
    -lan_ip 192.168.1.6 \
    -power "npc_AP2 1"\
    -relay "mc59end1" \
    -lanpeer lan0 \
    -console "mc59end1:40036" \
    -nvram {
	   	lan_ipaddr=192.168.1.6
		lan_gateway=192.168.1.6
		dhcp_start=192.168.1.150
	    dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.6
		lan1_gateway=192.168.2.6
		dhcp1_start=192.168.2.150
	    dhcp1_end=192.168.2.199
       	fw_disable=1
       	wl_msglevel=0x101
        boardtype=0x05b2; # 4706nr
        console_loglevel=7
       	wl0_ssid=TestMC59_4360
		wl0_channel=36
		w10_bw_cap=-1
		wl0_radio=0
		wl0_obss_coex=0
		wl1_ssid=TestMC59_4331
		wl1_channel=1
		wl1_bw_cap=-1
		wl1_radio=0
		wl1_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }\
    -datarate {-b 1.2G -i 0.5 -frameburst 1}\
    -noradio_pwrsave 1\
    -date 2014.10.2.

4706/4360 clone 4360_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
4706/4331 clone 4331_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
# 4360_AARDVARK clone mc59_AP1
4360_AARDVARK clone mc59_AP1_BISON -tag "BISON_BRANCH_7_10" 
4331_AARDVARK clone mc59_AP1_bg_BISON -tag "BISON_BRANCH_7_10"
4360_AARDVARK clone mc59_AP1_twig -tag "AARDVARK_REL_6_30_???_*"
4331_AARDVARK clone mc59_AP1_bg_twig -tag "AARDVARK_REL_6_30_???_*"
4360_AARDVARK clone mc59_AP1_trunk -tag TRUNK
4331_AARDVARK clone mc59_AP1_bg_trunk -tag TRUNK
mc59_AP1_BISON clone mc59_AP1
mc59_AP1_bg_BISON clone mc59_AP1_bg

unset UTF::TcpReadStats
