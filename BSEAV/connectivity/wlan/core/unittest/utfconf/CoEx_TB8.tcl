# -*-tcl-*-
# ====================================================================================================
# 
# MC22: coex testbed TB8 configuration
#
# device = BCM94330A1 Dev (BCM94330FCBGABU P103)
# Changed to 4330B2 as of 10/3/11

# TB8	4330A1 Dev	
# mc22end1  10.19.85.183   Linux UTF Host   - TB8UTF
# mc22end2  10.19.85.184   DUT Linux        - TB8DUT
# mc22tst1  10.19.85.185   Vista_BtRef      - TB8BTREF
# mc22tst2  10.19.85.186   XP_BtCoHost   	- TB8BTCOHOST
# mc22tst3  10.19.85.187   web_relay_BtRef
# mc22tst4  10.19.85.188   web_relay_DUT
#           10.19.85.211   npc22_AP

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::wlan_dut B2Uno3ExtTwig  		;# HND WLAN device under test
set ::bt_dut BTCohost2				;# BlueTooth device under test using generic cgr
set ::bt_ref BTRef_2046USB      	;# BlueTooth reference board
set ::wlan_tg lan              		;# HND WLAN traffic generator
set ::wlan_rtr AP1-4717   			;# HND WLAN router

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD

# SummaryDir sets the location for test results
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/CoExTB8"

# Define power controllers on cart.
UTF::Power::WebRelay 10.19.85.187
UTF::Power::WebRelay 10.19.85.188
# UTF::Power::WebRelay 172.16.22.40 ;# webrelay for DUT2 and Cohost2
UTF::Power::WebRelay 10.19.87.112 ;# webrelay for DUT2 WLAN and BT resets
# UTF::Power::WebRelay 192.168.1.207
UTF::Power::Synaccess 10.19.85.211
UTF::Power::Synaccess npc22_BT -lan_ip 172.16.22.21 -rev 1;# npc for DUT and Cohost; replaced with Rev 1 unit 5/15/14
UTF::Power::Synaccess npc22_DUT2 -lan_ip 172.16.22.30 ;# npc for DUT2 and Cohost2
UTF::Power::Synaccess npc22_DUT2EmbAdpt -lan_ip 172.16.22.35 ;# npc for DUT2 embedded adapter
# UTF::Power::Synaccess npc3 -lan_ip 172.16.50.22 ;# enclosure 2 --> 43224Vista


# Attenuator
UTF::Aeroflex af -lan_ip 172.16.22.10 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}

# Test bed initialization 
# set Aeroflex attenuation, and turn off radio on APs
set ::UTF::SetupTestBed {
	
  	G1 attn 15 ;# from 25
    G2 attn 30
	G3 attn 103
	
    foreach S {4350c4 4350c5} {
	  catch {$S wl down}
	  $S deinit
    }

    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
UTF::Linux TB8UTF -lan_ip 10.19.85.183 -sta "lan eth1"

# # # installed mc22tst6 FC15 host with 4350c4 stella/cidre PCIe module 10/14/14

UTF::DHD TB8DUT2\
	-lan_ip mc22tst6\
	-sta {4350c4 eth0}\
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
	-power "npc22_DUT2 1" 
# # # 	\
# # # 	-console mc22tst6:40001
    
    
4350c4 clone 4350c4_rel\
	-tag BIS120RC4_REL_7_15_*\
	-nvram ../C-4350__s-C4/P-rieslinga_M-CIDR_V-m__m-2.13.txt\
	-type ../C-4350__s-C2/rieslinga.trx -clm_blob rieslinga.clmb 
	
4350c4_rel clone 4350c4_rel_coex -wlinitcmds "wl btc_mode 1"
# 4350c4_rel clone 4350c4_mon -tag BIS715T185RC1_REL_7_35_* # moved to 7.47.x twigs 6/18/15
4350c4_rel clone 4350c4_mon -tag BIS735T255RC1_REL_7_47_* -dhd_tag DHD_REL_1_359_39
4350c4_mon clone 4350c4_mon_coex -wlinitcmds "wl btc_mode 1"
# 4350c4_rel clone 4350c4_mon_ToB -tag BIS735T255RC1_BRANCH_7_47 -dhd_tag DHD_REL_1_359_39
# 4350c4_mon_ToB clone 4350c4_mon_ToB_coex -wlinitcmds "wl btc_mode 1"
# monarch and castlerock: 7.59 ToB
4350c4_rel clone 4350c4_mon_ToB -tag BIS747T64RC49_BRANCH_7_59 -dhd_tag DHD_REL_1_359_77
4350c4_mon_ToB clone 4350c4_mon_ToB_coex -wlinitcmds "wl btc_mode 1"

# # # BTCohost2: Cohost for 4350c4, hostname mc59end7, ip_addr 10.19.87.101
# # # # Cohost for DUT2; IP address taken from MC59 range
# # # # NIS name mc59end7, ip address 10.19.87.101

UTF::WinBT TB8BTCOHOST2 \
	-lan_ip 10.19.87.101 \
	-sta "BTCohost2_4350c4" \
	-power "npc22_DUT2 2" \
	-bt_comm "com11@3000000" \
	-bt_comm_startup_speed 115200 \
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-user user\
	-bt_xtal_freq 40\
	-bt_power_adjust 40\
    -bt_ss_location 0x0021E000 \
    -type BCM4350/C*\
	-brand ""\
	-version BCM4350C*_Riesling_OS_MUR_STC_*


UTF::DHD TB8DUT \
	-lan_ip 10.19.85.184 \
	-sta "STA-Linux-4330sdio eth1" \
	-tag FALCON_BRANCH_5_90 \
	-nvram "src/shared/nvram/bcm94330fcbgabu.txt" \
    -brand linux-internal-dongle \
    -customer "bcm" \
    -type "4330a0-romlsim-43362a0/sdio-pool.bin" \
    -wlinitcmds {wl mpc 0; wl btc_mode 0} \
    -postinstall {dhd -i eth1 ioctl_timeout 10000} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}}
#     

STA-Linux-4330sdio clone 4330-43239a0 -type "4330a0-romlsim-43239a0/sdio-ag-pool.bin"
STA-Linux-4330sdio clone 4330-F16 -tag F16_BRANCH_5_80 -brand linux-internal-dongle -type "4330a0-roml/sdio-g-pool.bin"
STA-Linux-4330sdio clone 4330a0 -type "4330b0-roml/sdio-g.bin"

# changed from ip 10.19.85.184 (mc22end2) to 10.19.85.143 (mc59end15) on 10/5/12
# # # 	-lan_ip 10.19.85.143 \ ;# changed to (mc61end7) 10.19.87.121 2/14/13 for debugging; updated to FC15 2/20/14

UTF::DHD 43341DUT\
	-lan_ip 10.19.87.121\
	-sta "43341sdio eth0"\
	-power "npc22_BT 1"\
	-power_button {auto}\
	-power_sta "10.19.85.188 3"\
	-device_reset "10.19.85.188 2"\
	-tag PHOENIX2_REL_6_10_130_3\
	-nvram "src/shared/nvram/bcm943341wlagb.txt"\
    -brand linux-external-dongle-sdio\
    -customer "bcm"\
    -type "43341a0min-roml/sdio-ag-pno-pktfilter-keepalive-aoe-idsup-idauth-wme-apsta.bin"\
    -wlinitcmds {wl mpc 0}

43341sdio clone 4350c5\
    -tag BIS735T255RC1_REL_7_47_100_*\
    -dhd_tag DHD_REL_1_359_70\
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram ../C-4350__s-C5/P-albarossa_M-BLMA_V-m__m-5.5.txt \
    -customer olympic \
    -type ../C-4350__s-C5/albarossa.trx -clm_blob albarossa.clmb \
    -postinstall {dhd -i eth0 dma_ring_indices 3}\
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}}\
    -nobighammer 1
    
4350c5 clone 4350c5_coex -wlinitcmds {wl mpc 0; wl btc_mode 1}
4350c5 clone 4350c5_cstlrk -tag BIS747T99RC11_REL_7_54_* -dhd_tag DHD_REL_1_359_77
4350c5_cstlrk clone 4350c5_cstlrk_coex -wlinitcmds {wl mpc 0; wl btc_mode 1}

# using same setting as 4329B1_Ref in TB6Exp.tcl 
# host name: mc22tst2
    
UTF::WinBT TB8BTCOHOST \
	-lan_ip 10.19.85.186 \
	-sta "BTCohost_4330" \
	-power "npc22_BT 2" \
	-power_button {auto} \
 	-device_reset "10.19.85.188 1" \
	-bt_comm "com3@3000000" \
	-bt_comm_startup_speed 115200 \
	-user user\
	-bt_xtal_freq 26\
	-bt_power_adjust 40\
    -bt_ss_location 0x00210000 \
    -type BCM4330/B1 \
	-brand Olympic_Uno3/Murata\
	-version *
	
BTCohost_4330 clone BTCohost4350c5\
	-bt_comm "com18@3000000" \
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-user user\
	-bt_xtal_freq 40\
	-bt_power_adjust 40\
    -bt_ss_location 0x0021E000 \
    -type BCM4350/C*\
	-brand ""\
	-version BCM4350C*_Albarossa_OS_MUR_BM_MCC_*

# BT Ref - WinXP laptop BlueTooth 2046 Reference
# mc22tst1  10.19.85.185   Vista_BtRef      - TB8BTREF
#     -power_sta "10.19.85.181 1"\

UTF::WinBT TB8BTREF \
	-lan_ip 10.19.85.185 \
	-sta "BTRef_2046USB" \
   	-type "BCM2046"\
	-bt_comm "usb0"\
	-user user\
	-device_reset "10.19.85.187 1"

#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#
UTF::Router TB8AP1 \
    -lan_ip 192.168.1.2 \
    -sta "AP1-4717 eth1" \
    -relay "TB8UTF" \
    -lanpeer lan \
    -console "10.19.85.183:40000" \
    -power "10.19.85.211 1"\
    -brand "linux-internal-router" \
    -tag "MILLAU_REL_5_70*" \
    -date * \
   -nvram {
	    lan_ipaddr=192.168.1.2
	    lan_gateway=192.168.1.2
       	fw_disable=1
       	wl_msglevel=0x101
       	clkfreq=300,133,37
       	wl0_ssid=CoexTB8
		wl0_channel=1
		w10_nmode=1
		wl0_obss_coex=0
		antswitch=0
    }
# # #     -tag "COMANCHE2_REL_5_22_\?\?" \

# Specify router antenna selection to use
set ::wlan_rtr_antsel 0x01

# changed to linux-internal-router for better debug on 12/17/10
#     -brand "linux-external-router-combo" \

AP1-4717 clone MC22_AP1
AP1-4717 clone mc22_AP1
AP1-4717 clone TB8_AP1
AP1-4717 clone tb8_AP1

AP1-4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
AP1-4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
AP1-4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
AP1-4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
AP1-4717 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
AP1-4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
AP1-4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
AP1-4717 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

unset UTF::TcpReadStats
