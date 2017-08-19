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
# UTF::Power::Synaccess tmpCtl -lan_ip 192.168.1.200
### UTF::Power::Synaccess tmpCtl -lan_ip 10.19.86.201

UTF::Power::WebRelay 172.16.50.30 ;# webrelay for 4334b1 sdio
UTF::Power::WebRelay 172.16.50.40 ;# webrelay for BTRef

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.50.21 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}

# Test bed initialization 
# set Aeroflex attenuation, and turn off radio on APs
set ::UTF::SetupTestBed {
	
  	G1 attn 20 ;# changed to 10 from 20 9/20/13
	G2 attn 20 ;# changed to 20 from 103 10/5/13
	G3 attn 103
#     ALL attn 0

    foreach S {4334b1 4334h} {
	  catch {$S wl down}
	  $S deinit
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

UTF::WinBT BLECOHOST\
	-lan_ip 10.19.85.153\
	-sta "BLECohost"\
	-bt_comm "com3@3000000"\
	-bt_comm_startup_speed 115200\
    -user user
    
UTF::WinBT BTCOHOST\
	-lan_ip 10.19.86.201\
	-sta "BTCohostXP"\
	-bt_comm "com4@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -device_reset "172.16.50.30 2"

BTCohostXP clone BTCohost4334B1\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_ss_location 0x00210000\
    -type BCM4334/B1\
    -brand Generic/37_4MHz/fcbga_extLna\
    -version *\
    -file *.cgs

BTCohostXP clone BTCohost4334h\
	-bt_comm "com5@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_ss_location 0x00210000\
    -image /home/pkwan/BCM4334/B1/BCM4334B1_11.4.1516.1529_Cent_OS_MUR_20131021/BCM4334B1_11.4.1516.1529_Cent_OS_MUR_20131021.cgs
    
BTCohost4334B1 clone BTCohost4334h_0830\
    -image /home/pkwan/BCM4334/B1/BCM4334B1_11.1.1421.1436_Cent_OS_MUR_20130830.cgs
			
UTF::WinBT BTREFXP\
     -lan_ip 10.19.86.202\
     -sta "BTRefXP"\
     -type "BCM2046"\
     -bt_comm "usb0"\
     -user user\
     -device_reset "172.16.50.40 1"

# # # UTF::WinBT BTREFXP_RESET\
# # #      -lan_ip 10.19.86.202\
# # #      -sta "BTRefXP_reset"\
# # #      -type "BCM2046"\
# # #      -bt_comm "usb0"\
# # #      -user user\
# # #      -device_reset "172.16.50.40 1"

### device reset statements:
### webrelayshell 172.16.50.40 relay1State=1 (close)
### webrelayshell 172.16.50.40 relay1State=0 (open)

# debug 10/30/13
#UTF::WinBT mc32tst2\
#    -lan_ip 10.19.85.246\
#    -sta BTRefXP\
#    -type "BCM2046"\
#    -bt_comm "usb0"\
#    -user user\
#    -device_reset "172.16.50.40 1"
# end of debug 10/30/13
    
# # # # 	-lan_ip 10.19.87.254 \
# 	-lan_ip 10.19.86.201 \


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

#
#  this is UNO3
# 

UTF::DHD mc59end14\
	-lan_ip 10.19.85.142\
	-sta "4335b0 eth1"\
	-tag AARDVARK_REL_6_30_171\
    -brand linux-external-dongle-sdio\
    -type "4335b0-roml/sdio.bin"\
    -customer "bcm"\
	-nvram bcm943362sdgn.txt
	
UTF::DHD mc59end15\
	-lan_ip 10.19.85.143\
	-sta "4335c0 eth1"\
	-tag AARDVARK_BRANCH_6_30\
    -brand linux-mfgtest-dongle-sdio\
    -type "4335c0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn.bin"\
    -customer "bcm"\
	-nvram src/shared/nvram/bcm94335wlipa.txt\
	-dhd_tag DHD_REL_1_91
	
4335c0 clone 4335c0_epa\
    -tag AARDVARK_BRANCH_6_30 \
    -dhd_tag NIGHTLY -slowassoc 5 \
    -brand linux-external-dongle-sdio \
    -dhd_brand linux-internal-dongle \
    -nvram "bcm94335wlbgaFEM_AA.txt" \
    -nvram_add {} \
    -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
    -type 4335c0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-autoabn-ampduhostreorder-ve-proxd-p2po-okc-txbf-err-assert \
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1\
    -postinstall {dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 10} \
    -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3} \
    -tcpwindow 1152k -udp 400m \
    -yart {-attn5g 7-63 -attn2g 17-63 -pad -7 -frameburst 1}
    	
# # # 	Firmware
# # #                 http://www.sj.broadcom.com/projects/hnd_swbuild/build_linux/AARDVARK_BRANCH_6_30/linux-mfgtest-dongle-sdio/2013.3.8.0/release/bcm/firmware/4335c0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn.bin
# # # Nvram
# # #                 http://www.sj.broadcom.com/projects/hnd_swbuild/build_linux/AARDVARK_BRANCH_6_30/linux-mfgtest-dongle-sdio/2013.3.8.0/src/shared/nvram/bcm94335wlipa.txt
# # # Apps (wl, wl_server_socket)
# # #                 http://www.sj.broadcom.com/projects/hnd_swbuild/build_linux/AARDVARK_BRANCH_6_30/linux-mfgtest-dongle-sdio/2013.3.8.0/release/bcm/apps/
# # # dhd
# # #                 http://www.sj.broadcom.com/projects/hnd_swbuild/build_linux/DHD_REL_1_91/linux-mfgtest-dongle-sdio/2013.2.25.0/release/bcm/apps/dhd
# # # dhd.ko
# # #                 http://www.sj.broadcom.com/projects/hnd_swbuild/build_linux/DHD_REL_1_91/linux-mfgtest-dongle-sdio/2013.2.25.0/release/bcm/host/dhd-cdc-sdstd-debug-2.6.29.4-167.fc11.i686.PAE/dhd.ko

UTF::DHD sdio3\
    -lan_ip 10.19.86.200\
    -sta "4339wlipa eth0"\
	-tag AARDVARK_BRANCH_6_30\
    -brand linux-external-dongle-sdio\
    -type 4339a*-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-ampduhostreorder-ve-mfp-okc-txbf-relmcast*-idsup-idauth.bin\
    -customer "bcm"\
	-nvram src/shared/nvram/bcm94339wlipa.txt\
	-dhd_tag NIGHTLY\
	-dhd_brand linux-internal-dongle\
	-nocal 1 -noaes 1 -notkip 1\
	-modopts {sd_uhsimode=2 sd_txglom=1 sd_tuning_period=10}\
 	-postinstall {dhd -i eth0 sd_blocksize 2 256; dhd -i eth0 txglomsize 10} \
	-wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl bw_cap 2g -1} \
	-tcpwindow 1152k -udp 400m\
    -datarate {-i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3}
 
UTF::DHD mc50tst1 \
	-lan_ip 10.19.86.200 \
	-sta "4330Exp eth0" \
	-tag FALCON_BRANCH_5_90 \
	-nvram "src/shared/nvram/bcm94330OlympicUNO3.txt" \
    -brand linux-internal-dongle \
    -type "4330b0-roml/sdio-g*.bin"\
    -customer "bcm"
    
# # # 	-sta "4330Exp eth1" \;# changed to eth0 -- FC15 installed as of 8/23/13
# # # 	-console "mc50tst1:40001" \
    
#      \
#     -wlinitcmds {wl mpc 0} 
#     -postinstall {dhd -i eth1 serial 1}
#     -type "4330b1-roml/sdio-g-pool.bin" \
         
#     -console "mc22end2:40000" \
#     -postinstall {dhd -i eth1 serial 1}


4330Exp clone 4330B1Dual \
	-console "mc50tst1:40001"\
	-type "4330b1-roml/sdio-g-pool.bin"\
	-nvram "4330b1-roml/bcm94330fcbga_McLaren.txt"\
    -noafterburner 1\
    -wlinitcmds {wl mpc 0}\
    -postinstall {dhd -i eth1 serial 1}

# UTF::DHD mc23end2 \
# 	-lan_ip 10.19.85.190\
# 	-sta "4330B1Dual eth1"\

4330Exp clone 4330B2\
	-tag FALCON_{BRANCH,REL}_5_90_*\
	-console "mc50tst1:40000"\
	-type "4330b2-roml/sdio-ag.bin"\
	-nvram "4330b2-roml/bcm94330fcbga_McLaren.txt"\
	-postinstall {dhd -i eth1 serial 1}
	    
4330Exp clone 4330B1\
	 -tag FALCON_{BRANCH,REL}_5_90_*\
	 -type "4330b1-roml/sdio-g-pool.bin"\
	 -nvram "4330b1-roml/bcm94330fcbga_McLaren.txt"\
	 -wlinitcmds {wl mpc 0}\
	 -postinstall {dhd -i eth1 serial 1}
# 	 \
# 	 -console "mc50tst1:40000"
	 
4330Exp clone 4330B0\
	 -tag FALCON_{BRANCH,REL}_5_90_*\
	 -type "4330b0-roml/sdio-g*.bin"\
	 -nvram "4330b0-roml/bcm94330fcbga_McLaren.txt"
	 
# # 4330Exp clone 4330B0\
# # 	 -tag FALCON_BRANCH_5_90\
# # 	 -type "4330b0-roml/sdio-g*.bin"\
# # 	 -nvram "4330b0-roml/bcm94330fcbga_McLaren.txt"
	 
4330Exp clone Uno3\
	 -tag FALCON_BRANCH_5_90\
	 -type "4330b1-roml/sdio-g-pool.bin"\
	 -nvram "src/shared/nvram/bcm94330OlympicUNO3.txt"

#    4329B1 ref board ; load successful with DUCKS2; PCI2SDIO board 3.3v jumper shunted.

4330Exp clone 4330B2_Uno3\
	 -tag FALCON_BRANCH_5_90\
	 -type "4330b2-roml/sdio-g.bin"\
	 -nvram "src/shared/nvram/bcm94330OlympicUNO3.txt"\
	 -wlinitcmds {wl mpc 0}\
	 -postinstall {dhd -i eth1 serial 1}

4330Exp clone 4330B2_Uno3_twig\
	 -tag FALCON_TWIG_5_90_156\
	 -type "4330b2-roml/sdio-g.bin"\
	 -nvram "src/shared/nvram/bcm94330OlympicUNO3.txt"\
	 -wlinitcmds {wl mpc 0}\
	 -postinstall {dhd -i eth1 serial 1}
	 
4330Exp clone 4330B2_WLSD\
	 -tag FALCON_BRANCH_5_90\
	 -type "4330b2-roml/sdio-ag.bin"\
	 -nvram "src/shared/nvram/bcm94330wlsdagb_P300.txt"\
	 -wlinitcmds {wl mpc 0}\
	 -postinstall {dhd -i eth1 serial 1}
	 
4330Exp clone B2Uno3Hoodoo\
	-tag FAL156RC30_REL_5_95_??\
	-nvram "bcm94330OlympicUNO3.txt"\
	-brand linux-external-dongle-sdio\
	-customer "olympic"\
	-type "4330b2-roml/sdio-g-apsta-pno-nocis.bin"\
	-postinstall {}		 
	 	 
4330Exp clone 4329B1 \
	-tag "ROMTERM3_BRANCH_4_220"\
	-type "4329b1/sdio-ag-cdc-full11n-reclaim-roml-wme-idsup.bin"\
	-nvram   "src/shared/nvram/bcm94329sdagb.txt"\
	-console "mc50tst1:40000"\
	-wlinitcmds {wl mpc 0}
	
#   -tag "ROMTERM2_BRANCH_4_219" \
#   -type "4329b1/sdio-g-cdc-roml-reclaim-full11n-wme-idsup-pool-assert.bin" \
#   -nvram   "src/shared/nvram/bcm94329sdagb.txt" 

4330Exp clone 43291\
	 -console ""\
	 -tag "ROMTERM3_BRANCH_4_220"\
	 -type "4329c0/sdio-g-cdc-roml-reclaim-full11n-wme-idsup-apsta-assoc-assert.bin"\
	 -nvram "4329c0/bcm94329sdagb.txt"
# 	 \
# 	 -postinstall {dhd -i eth1 serial 1}

4330Exp clone 43291Olympic\
	 -console "mc50tst1:40000"\
	 -tag "RT2TWIG46_REL_4_221_??"\
	 -customer "olympic"\
	 -brand "linux-external-dongle-sdio"\
	 -type "4329c0/sdio-ag-cdc-full11n-reclaim-roml-idsup-nocis-wme-memredux24-pno-aoe-pktfilter-medioctl-sec-wapi-keepalive.bin"\
	 -nvram "4329c0/bcm94329OLYMPICUNO.txt"
	 # \
	 # -postinstall {dhd -i eth1 serial 1}
	 
#     
# # 43291sdio clone 43291RT2Olympic\
# #     -customer "olympic"\
# #     -brand "linux-external-dongle-sdio"\
# #     -tag RT2TWIG46_REL_4_221_??\
#    -type "4329c0/sdio-g-cdc-full11n-reclaim-roml-idsup-nocis-wme-memredux24-pno-aoe-pktfilter-minioctl-loco-wapi-keepalive.bin"    

  
4330Exp clone 4329N18 \
    -tag "ROMTERM_BRANCH_4_218" \
    -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup.bin" \
    -nvram   "bcm94329OLYMPICN18.txt" 
#     -pre_perf_hook {{AP1-4322-4717 wl ampdu_clear_dump}} \
#     -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4322-4717 wl dump ampdu}}
	
# 	-brand linux-internal-dongle \
# 	-type "4330a0-roml/sdio-g-pool.bin" \
# 	-nvram "src/shared/nvram/bcm94330fcbgabu.txt" \
#     -customer "bcm" \
#     -wlinitcmds {wl mpc 0; wl btc_mode 0} \
#     -postinstall {dhd -i eth1 serial 1}
# 
4330Exp clone 43239sim \
	-tag FALCON_BRANCH_5_90 \
	-postinstall {dhd -i eth1 socdevram 1 0; dhd -i eth1 serial 1} \
 	-channelsweep {-band b} \
 	-type 4330a0-romlsim-43239a0/sdio-g-pool.romlsim.trx	

# device already installed into mc60 as of 6/20/2011 	
# # 
# # 4330Exp clone 43239a0\
# # 	 -tag FALCON_BRANCH_5_90\
# # 	 -type "43239a0-roml/sdio-ag*pno.bin"\
# # 	 -nvram "src/shared/nvram/bcm943239elhsdb_p200.txt"\
# # 	 -console "mc50tst1:40001"\
# # 	 -postinstall {dhd -i eth1 serial 1}

# This is 43239a0combo with 2076B1 chip

# # 4330Exp clone 2076B1\
# # 	 -tag FALCON_REL_5_90_195_3\
# # 	 -brand linux-mfgtest-dongle-sdio\
# # 	 -type "43239a0-roml/sdio-ag-mfgtest.bin"\
# # 	 -nvram "src/shared/nvram/bcm943239sdnb6.txt"
# # # 	 -nvram "/projects/hnd/swbuild/build_linux/FALCON_REL_5_90_195_3/linux-mfgtest-dongle-sdio/2011.11.1.0/src/shared/nvram/bcm943239sdnb6.txt"
# # # 	 -nvram "src/shared/nvram/bcm943239sdnb6.txt"
# # # 	 -type "43239a0-roml/sdio-ag-p2p-idauth-pno-dsta-mchan-ve.bin"\
# 	 \
# 	 -dhd_subdir "release/bcm/host/dhd-cdc-sdstd-apsta-2.6.25-14.fc9.i686"
	 
4330Exp clone 2076B1Local\
	-type "/home/pkwan/tot/src/tools/wlan/drivers/FALCON_REL_5_90_195_1/43239a0-roml/sdio-ag-mfgtest.bin"\
	-nvram "/home/pkwan/tot/src/tools/wlan/drivers/FALCON_REL_5_90_195_1/bcm943239sdnb6.txt"
# 	\
# 	-dhd_subdir "/home/pkwan/tot/src/tools/wlan/drivers/FALCON_REL_5_90_195_1"
# 	\
# 	-dhd_file "/home/ddave/tot/src/tools/wlan/drivers/FALCON_REL_5_90_195_1/dhd_fc9.ko"
# 
# 43236bmac clone 43236full \
# #     -tag FALCON_BRANCH_5_90 \
# #     -brand "linux-internal-dongle" \
# #     -type "43239*-roml/sdio-ag*pno.bin"\
# #     -dhd_brand "linux-internal-dongle"\
# #     -dhd_subdir "release/bcm/host"\
# #     -dhd_type "dhd-cdc-sdstd"\
# #     -dhd_file "dhd.ko" \
# #     -wl_brand "linux-internal-dongle"\
# #     -wl_subdir release/bcm/apps \
# #     -nvram_subdir src/shared/nvram \
# #     -nvram_file bcm943239elhsdb_p200.txt
    
#     -subdir "release/bcm/firmware" \    
#     \
#     -post_perf_hook {{%S rte mu} {%S wl rssi} {%S wl nrate}}
    	 	 
#  		-console "mc50tst1:40001" \
# 	-tag FALCON_BRANCH_5_90 \
# 	-nvram "src/shared/nvram/bcm94330OlympicUNO3.txt" \
#     -brand linux-internal-dongle \
#     -customer "bcm" \
#     -type "4330b1-roml/sdio-g-pool.bin" \
#     -wlinitcmds {wl mpc 0} 
    

	# 	-channelsweep {-band b} \	
# 	-tag FALCON_BRANCH_5_90 \
# 	-brand linux-internal-dongle \
# 	-customer "bcm" \
# 	-type "4330b1-roml/sdio-ag-pool.bin" \
# 	-nvram "4330b1-roml/bcm94330fcbga_McLaren.txt" \
#     -noafterburner 1 \
#     -wlinitcmds {wl mpc 0}
#     
# 
# 4330Exp clone 4330B1 -type "4330b1-roml/sdio-g-pool.bin" -nvram "4330b1-roml/bcm94330fcbga_McLaren.txt"	
# 4330Exp clone 4330B0 -type "4330b0-roml/sdio-g.bin" -nvram "4330b0-roml/bcm94330fcbga_McLaren.txt"
	
# 	-tag F16_BRANCH_5_80 \
# 	-brand linux-internal-dongle \
# 	-type "4330a0-roml/sdio-g-pool.bin" \
# 	-nvram "src/shared/nvram/bcm94330fcbgabu.txt" \
#     -customer "bcm" \
#     -wlinitcmds {wl mpc 0; wl btc_mode 0} \
#     -postinstall {dhd -i eth1 ioctl_timeout 10000}
# 
# 4330Exp clone 43239sim \
# 	-tag FALCON_BRANCH_5_90 \
# 	-postinstall {dhd -i eth1 socdevram 1 0} \
# 	-channelsweep {-band b} \
# 	-type 4330a0-romlsim-43239a0/sdio-g-pool.romlsim.trx
    
# workaround for no console output at local apshell and log file: take out -console option    
#     -console "mc22end2:40000" \; this is USB :40001 is RS232
#   -postinstall {dhd -i eth1 ioctl_timeout 1000; dhd -i eth1 serial 1}

# 4330Exp clone 4330-F16 -tag F16_BRANCH_5_80 -brand linux-internal-dongle -type "4330a0-roml/sdio-g-pool.bin"
	
#
# 4330B1, single or dual band
#
	
# 	-tag FALCON_BRANCH_5_90 \
# 	-brand linux-internal-dongle \
# 	-customer "bcm" \
# 	-type "4330b1-roml/sdio-ag-pool.bin" \
# 	-nvram "4330b1-roml/bcm94330fcbga_McLaren.txt" \
#     -noafterburner 1 \
#     -wlinitcmds {wl mpc 0}
    
# 
# 4330Exp clone 4330B1 -type "4330b1-roml/sdio-g-pool.bin" -nvram "4330b1-roml/bcm94330fcbga_McLaren.txt"

#    4329B1 ref board ; load successful with DUCKS2; PCI2SDIO board 3.3v jumper shunted.
#   -tag "ROMTERM2_BRANCH_4_219" \
#   -brand "linux-internal-dongle" \
#   -customer "bcm" \
#   -type "4329b1/sdio-ag-cdc-full11n-reclaim-roml-wme-idsup.bin" \
#   -nvram   "src/shared/nvram/bcm94329sdagb.txt" 

#  	-power_sta "10.19.85.200 3" \
# 	-power "10.19.85.207 1" \    
    	
#
# 4330b0 UNO3; load successful with DUCKS2, with PCI2SDIO adapter 3.3v jumper shunted.
#	
# 	-tag FALCON_BRANCH_5_90 \
# 	-nvram "src/shared/nvram/bcm94330OlympicUNO3.txt" \
#     -brand linux-internal-dongle \
#     -customer "bcm" \
#     -type "4330b0-roml/sdio-g.bin" \
#     -wlinitcmds {wl mpc 0} 
    
#     -console "mc22end2:40000" \
#     -postinstall {dhd -i eth1 serial 1}

#	
# this is 4330b0 McLaren; no power should be supplied from PCI2SDIO; remove all jumpers on J11
#
# 	-tag FALCON_BRANCH_5_90 \
# 	-brand linux-internal-dongle \
# 	-customer "bcm" \
# 	-type "4330b0-roml/sdio-g.bin" \
# 	-nvram "4330b0-roml/bcm94330fcbga_McLaren.txt" \
#     -noafterburner 1 \
#     -wlinitcmds {wl mpc 0}
    
#     -console {mc32tst4:40001} \
#     -postinstall {dhd -i eth1 ioctl_timeout 1000; dhd -i eth1 serial 1}

#
# This 4330A1 Dev Board; F-16 branch used to work
#

# UTF::DHD mc32tst4 \
# 	-sta "4330sdio eth1" \
# 	-tag FALCON_BRANCH_5_90 \
# 	-nvram "src/shared/nvram/bcm94330fcbgabu.txt" \
#     -brand linux-internal-dongle \
#     -customer "bcm" \
#     -type "4330a0-romlsim-43362a0/sdio-pool.bin" \
#     -wlinitcmds {wl mpc 0; wl btc_mode 0} \
#     -postinstall {dhd -i eth1 ioctl_timeout 10000} \
#     -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
#     -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}}
#     
# workaround for no console output at local apshell and log file: take out -console option    
#     -console "mc22end2:40000" \; this is USB :40001 is RS232
#   -postinstall {dhd -i eth1 ioctl_timeout 1000; dhd -i eth1 serial 1}

# 4330sdio clone 4330-43239a0 -type "4330a0-romlsim-43239a0/sdio-ag-pool.bin"
# 4330sdio clone 4330-F16 -tag F16_BRANCH_5_80 -brand linux-internal-dongle -type "4330a0-roml/sdio-g-pool.bin"
# 4330sdio clone 4330a0 -type "4330b0-roml/sdio-g.bin"

4330Exp clone 4325 \
    -tag NIGHTLY \
    -type "4325b0/sdio-g-cdc-reclaim-pool.bin" \
    -nvram ""
    
4330Exp clone 4325R \
    -tag RAPTOR3_BRANCH_4_230 \
    -type "4325b0/sdio-g-cdc-reclaim-idsup-wme-pktfilter.bin"\
    -nvram ""    

4330Exp clone N18 \
	-tag "ROMTERM_BRANCH_4_218" \
    -brand "linux-internal-dongle" \
    -customer "bcm" \
    -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup.bin" \
    -nvram "4329b1/bcm94329OLYMPICN18.txt" \
    -pre_perf_hook {{AP1-4322-4717 wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {AP1-4322-4717 wl dump ampdu}}
    
4330Exp clone N18RT3 \
	-tag "ROMTERM3_BRANCH_4_220" \
	-type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup.bin" \
	-nvram "src/shared/nvram/bcm94329OLYMPICN18.txt"
	
4330Exp clone 4334sdio\
	-tag "PHOENIX_BRANCH_5_120"\
    -brand "linux-internal-dongle" \
    -customer "bcm"\
	-type "4334a0-roml/sdio-ag-pool-idsup-p2p.bin"\
    -nvram "src/shared/nvram/bcm94334fcagb.txt"\
    -postinstall {dhd -i eth1 serial 1}

4330Exp clone 4334B1\
	-tag PHOENIX2_BRANCH_6_10\
	-type "4334b1-roml/sdio-g-idsup-p2p.bin"\
	-nvram "src/shared/nvram/bcm94334fcagb.txt"\
	-postinstall "dhd -i eth1 serial 1"	
		
# 	-tag PHOENIX2_REL_6_10_??\
# 	-brand "linux-internal-dongle"\
# 	-customer "bcm"\
# 	-type "4334b1-roml/sdio-g-idsup.bin"\
# 	-nvram "src/shared/nvram/bcm94334fcagb.txt"\
# 	-wlinitcmds ""\
# 	-console "mc50tst1:40000"\
#  	-postinstall {dhd -i eth1 serial 1}

4330Exp clone 4334B1WL\
	-tag PHOENIX2_BRANCH_6_10\
	-type "4334b1-roml/sdio-ag-idsup-p2p.bin"\
	-nvram "src/shared/nvram/bcm94334wlagbi.txt"\
	-postinstall "dhd -i eth1 serial 1"	

4330Exp clone 4334B1ToB -tag PHOENIX2_BRANCH_6_10\
	-type "4334b1-roml/sdio-g-idsup-p2p.bin"\
	-wlinitcmds ""\
	-postinstall "dhd -i eth1 serial 1"\
	-console "mc50tst1:40000" 

# 	-console "mc59end6:40000"\
# 	-tag "PHOENIX_BRANCH_5_120"\
#     -brand "linux-internal-dongle" \
#     -customer "bcm"\
# 	-type "4334a0-roml/sdio-ag-pool-idsup-p2p.bin"\
#     -nvram "src/shared/nvram/bcm94334fcagb.txt"\
#     -postinstall {dhd -i eth1 serial 1}  
        
#     -postinstall {dhd -i eth1 serial 1; dhd -i eth1 sd_divisor 1}
    
#     -wlinitcmds {dhd -i eth1 serial 1}
    
# 	-tag "PHOENIX_REL_5_120_37" \
#         
#  from TimA 8/11/2011
# -tag PHOENIX_BRANCH_5_120 \
# -type 4334a0-roml/sdio-ag-pool-idsup-p2p \
# -postinstall {dhd -i eth1 serial 1; dhd -i eth1 sd_divisor 1}

# 	-power "172.16.54.25 2" \
# 	-power_sta "172.16.54.25 1"\
# 4330Exp clone endPt -sta {lan eth2}

# # # 4330Exp clone 43342A0ToB -tag PHOENIX2_BRANCH_6_10\
# # # 	-type "43342a0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc.bin"\
# # # 	-nvram "src/shared/nvram/bcm943342fcagb.txt"\
# # # 	-wlinitcmds ""\
# # # 	-postinstall "dhd -i eth1 serial 1"\
# # # 	-console "mc50tst1:40000" 
	
4330Exp clone 4350c0\
  -tag BISON_{REL,TWIG}_7_10_*\
  -type 4350c0-ram/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth-assert\
  -nvram "bcm94350wlagbe_KA.txt"\
  -brand linux-external-dongle-sdio\
  -dhd_tag NIGHTLY -slowassoc 5\
  -dhd_brand linux-internal-dongle\
  -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10}\
  -wlinitcmds "wl ampdu_mdpu 16; wl vht_features 3; wl bw_cap 2g -1"\
  -tcpwindow 2m -udp 1.2g \
  -nobighammer 1 -nocal 1\
  -postinstall "dhd -i eth0 sd_blocksize 2 256; dhd -i eth0 txglomsize 40"\
  -datarate {-i 0.5 -frameburst 1}

4330Exp clone 43342A0ToB -tag PHO2203RC1_BRANCH_6_25\
    -brand linux-external-dongle-sdio\
    -dhd_tag DHD_REL_1_88_27\
    -dhd_brand linux-external-dongle-sdio\
    -customer "{olympic,bcm}"\
	-type "Innsbruck/43342a0/southpaw.bin"\
	-nvram "Innsbruck/43342a0/southpaw-u-kk.txt"\
	-wlinitcmds ""\
	-console "mc50tst1:40000"

# # #     From WLAN SVT 9/26/13:
# # # Firmware you can take from the release folder. For example if you need Southpaw firmware for release 6.25.87.35. Following is the complete path.
# # # Firmware & Nvram Path: Z:\projects\hnd_swbuild\build_linux\PHO2203RC1_REL_6_25_87_35\linux-external-dongle-sdio\2013.9.19.0\release\olympic\firmware\Innsbruck\43342a0\
# # # WL: Z:\projects\hnd_swbuild\build_linux\PHO2203RC1_REL_6_25_87_35\linux-mfgtest-dongle-usb\2013.9.19.0\release\bcm\apps
# # # For FC-11: Z:\projects\hnd_swbuild\build_linux\DHD_REL_1_88_27\linux-external-dongle-sdio\2013.6.24.0\release\bcm\host\dhd-cdc-sdstd-2.6.29.4-167.fc11.i686.PAE\
# # # For FC-15: Z:\projects\hnd_swbuild\build_linux\DHD_REL_1_88_27\linux-external-dongle-sdio\2013.6.24.0\release\bcm\host\dhd-cdc-sdstd-2.6.38.6-26.rc1.fc15.i686.PAE\
# # # DHD: Z:\projects\hnd_swbuild\build_linux\DHD_REL_1_88_27\linux-external-dongle-sdio\2013.6.24.0\release\bcm\apps\
  
43342A0ToB clone southpaw \
    -customer "{olympic,bcm}"\
	-tag PHO2203RC1_REL_6_25_87_*\
    -type "Innsbruck/43342a0/southpaw.bin"\
    -nvram "Innsbruck/43342a0/southpaw-u-kk.txt"\
    -dhd_tag DHD_REL_1_88_27\
    -dhd_image /projects/hnd_swbuild/build_linux/DHD_REL_1_88_27/linux-external-dongle-sdio/2013.6.24.0/release/bcm/host/dhd-cdc-sdstd-debug-2.6.38.6-26.rc1.fc15.i686.PAE/dhd.ko
# # #     \
# # #     -driver "/projects/hnd_swbuild/build_linux/PHO2203RC1_REL_6_25_87_35/linux-mfgtest-dongle-usb/*/release/bcm/apps/wl.ko"
    
# # #     /projects/hnd_swbuild/build_linux/PHO2203RC1_REL_6_25_87_35/linux-mfgtest-dongle-usb/*/release/bcm/apps
# # #     \
# # #     -dhd /projects/hnd_swbuild/build_linux/DHD_REL_1_88_27/linux-external-dongle-sdio/2013.6.24.0/release/bcm/apps
		
# # # 	-dhd_tag NIGHTLY\
# # #     -customer olympic\
# # # 	-type "Innsbruck/43342a0/southpaw.bin"\
# # # 	-nvram "southpaw-u-kk.txt"\
# 	-type "43342a0min-roml/sdio-g-idsup-p2p.bin"\

# # # 	-nvram "bcm943342SouthpawUSIK.txt"\
# # # 	-type "43342a0-roml/sdio-g-p2p-pno-nocis-keepalive-aoe-idsup-wapi-ve-awdl-ndoe-pf2-proptxstatus-wlfcts-cca-pwrstats-wnm-wl11u-anqpo-noclminc-logtrace-srscan-clm_43342_olympic.bin"\	
	
4330Exp clone 43362sdio\
	-tag FALCON_BRANCH_5_90\
	-type "43362a2-roml/sdio.bin"\
	-nvram bcm943362sdg.txt
	
# 	-nvram bcm943362sdgn.txt ; # sdg.txt for Jeff's board
# 	\
# 	-postinstall {dhd -i eth1 serial 1}
	
# 	-nvram "src/shared/nvram/bcm943362sdgn.txt"

4330Exp clone 43362A2\
	-tag FALCON_REL_5_90_195_1\
	-brand linux-mfgtest-dongle-sdio\
	-nvram bcm943362sdg.txt\
	-type "43362a2-roml/sdio-mfgtest.bin"
# 	-type "43362a2-roml/sdio-p2p-idsup-idauth-pno.bin"	

4330Exp clone 43241b0\
   -tag PHOENIX2_REL_6_10_116_*\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-opt2.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -wlinitcmds {wl btc_mode 0}\
   -datarate {-skiptx 32}\
   -postinstall {dhd -i eth1 sd_divisor 1}

# # # 43241b0 clone 43241b4\
# # #    -tag PHOENIX2_REL_6_10_197_*\
# # #    -nvram "bcm943241ipaagb_p304.txt"\
# # #    -type "43241b4-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-autoabn.bin"\
# # #    -wlinitcmds {}

# # # 4330Exp clone 43341a0\

UTF::DHD mc61end7\
 -sta "43341a0 eth1"\
 -lan_ip 10.19.87.121\
 -tag PHOENIX2_REL_6_10_130_??\
 -brand linux-external-dongle-sdio\
 -type "43341a0min-roml/sdio-ag-pno-pktfilter-keepalive-aoe-idsup-idauth-wme.bin"\
 -nvram "src/shared/nvram/bcm943341wlagb.txt"\
 -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}}\
 -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate}}\
 -datarate {-skiptx 32}\
 -nopm1 1 -nopm2 1 -noaes 1 -notkip 1
 
43341a0 clone 43341a0b\
 -dhd_tag NIGHTLY\
 -driver dhd-cdc-sdstd\
 -type 43341a0min-roml/sdio-ag-pno-p2p-pktfilter-keepalive-aoe-idsup-idauth*
 
43341a0 clone 43341b0\
 -type 43341b0min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-sr-wapi-wl11d.bin\
 -dhd_tag NIGHTLY\
 -driver dhd-cdc-sdstd
 
43341a0 clone 43241b4\
   -tag PHOENIX2_REL_6_10_197\
   -nvram "bcm943241ipaagb_p304.txt"\
   -type "43241b4-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-autoabn.bin"\
   -wlinitcmds {}\
   -dhd_tag NIGHTLY
   
# # #    sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-autoabn.bin
# # # sdio-ag-mbss-idsup-idauth-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-wl11d-sr-autoabn 
# # #  -type 43341a0min-roml/sdio-ag-pno-p2p-pktfilter-keepalive-aoe-idsup-idauth-sr-wme
# # #  -dhd_tag NIGHTLY\ 
# # #  -dhd_tag DHD_REL_1_52\

  
# # 4330Exp clone 4335a0sdio\
# # 	-tag AARDVARK_{TWIG,REL}_6_30_69{,_*}\
# # 	-brand linux-external-dongle-sdio\
# # 	-nvram bcm94335wlbgaFEM_AA.txt\
# # 	-type "4335a0min-roml/sdio-ag-idsup-assert-err.bin"\
# # 	-console mc50tst1:40000\
# # 	-datarate {-b 350m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3}\
# # 	-nocal 1 -nopm1 1 -nopm2 1\
# # 	-postinstall {}\
# # 	-wlinitcmds {wl ampdu_mpdu 24}\
# # 	-tcpwindow 1152k -rwlrelay lan0
# 		-tag AARDVARK_TWIG_6_30_*\

# # sample 4335 conf from mc41b by TimA; fc15 host

# # UTF::DHD mc41end4 -sta {4335a0 eth0} \
# #     -power {webswitch1 2} \
# #     -hostconsole "mc41end1:40005" \
# #     -tag AARDVARK_{TWIG,REL}_6_30_69{,_*} \
# #     -dhd_tag NIGHTLY \
# #     -brand linux-external-dongle-sdio \
# #     -dhd_brand linux-internal-dongle \
# #     -nvram "bcm94335wlbgaFEM_AA.txt" \
# #     -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
# #     -type 4335a0min-roml/sdio-ag-idsup-assert-err \
# #     -datarate {-b 350m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3} \
# #     -nocal 1 -nopm1 1 -nopm2 1 \
# #     -postinstall {dhd -i eth0 msglevel 0x20001; dhd -i eth0 txglomsize 10} \
# #     -wlinitcmds {wl ampdu_mpdu 24} \
# #     -tcpwindow 1152k -rwlrelay lan

# # fc15 host mc61end7, ip 10.19.87.121
# #  Note combination of external driver and internal host/dongle driver
   
UTF::DHD mc61end7 \
	-sta "4335a0 eth0" \
	-console "mc61end7:40001"\
    -tag AARDVARK_{TWIG,REL}_6_30_69{,_*} \
    -dhd_tag NIGHTLY \
    -brand linux-external-dongle-sdio \
    -dhd_brand linux-internal-dongle \
    -nvram "bcm94335wlbgaFEM_AA.txt" \
    -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
    -type 4335a0min-roml/sdio-ag-idsup-assert-err \
    -datarate {-b 350m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3} \
    -nocal 1 -nopm1 1 -nopm2 1 \
    -postinstall {dhd -i eth0 msglevel 0x20001; dhd -i eth0 txglomsize 10} \
    -wlinitcmds {wl msglevel +assoc; wl ampdu_mpdu 24; wl vht_features 1} \
    -tcpwindow 1152k -rwlrelay lan0 
    
4335a0 clone 4335a0x -dhd_tag DHD_REL_1_61 -dhd_brand linux-external-dongle-sdio
    
4335a0 clone 4335b0\
 -tag AARDVARK_{TWIG,REL}_6_30_171_*\
 -dhd_tag DHD_REL_1_61\
 -dhd_brand linux-external-dongle-sdio\
 -type 4335b0-roml/sdio-ag-pool-p2p-idsup-idauth-pno-pktfilter-keepalive-aoe-ccx-sr-{vsdb,mchan}-proptxstatus-lpc-autoabn-txbf 
   
 
# # #     -postinstall {dhd -i eth1 msglevel 0x20001; dhd -i eth1 txglomsize 10} \
# # #     -wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1; wl ampdu_mpdu 24; wl vht_features 1} \
     
# # # UTF::DHD mc61end7 \
# # #    -sta "43241b0 eth1"\
# # #    -tag PHOENIX2_REL_6_10_116_*\
# # #    -brand linux-external-dongle-sdio\
# # #    -type "43241b0min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-opt2.bin"\
# # #    -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
# # #    -console mc61end7:40001\
# # #    -wlinitcmds {wl btc_mode 0}\
# # #    -postinstall {dhd -i eth1 sd_divisor 1}\
# # #    -datarate {-skiptx 32}   
    
UTF::DHD mc60end12 -sta "xSDIO eth1"

# 47186
package require UTF::HSIC
# UTF::HSIC hExp \
# 	-sta {4330h eth1} \
#     -coldboot 1 \
#     -relay mc50end1 \
#     -lanpeer lan0 \
#     -console "mc50tst1:40001" \
#     -nvram "bcm94330OlympicUNO3.txt" \
#     -tag "FALCON_REL_5_90_*" \
#     -brand "linux-external-dongle-usb" \
#     -type "4330b2-roml/usb-g-pool-nodis.bin.trx" \
#     -driver "dhd-cdc-usb-hsic-gpl-2.6.22" \
#     -host_tag "MILLAU_REL_5_70_55_16" \
#     -host_brand linux26-internal-hsic \
#     -host_nvram {
# 	"lan_ifnames=vlan1 eth1 eth2"
# 	"clkfreq=530,176,88"
# 	watchdog=6000
# 	console_loglevel=7
#     }
    
# 	-lan_ip 192.168.1.70 \
#     -rteconsole "mc50tst1:40001" \
# 	-lan_ip 10.19.86.200\
#     -tag "FALCON_REL_5_90_*" \

# 4330h is targeted for 4330B2 Uno3 single-band HSIC strapped board
# # # 9/23/13
# # # UTF::HSIC hExp\
# # # 	-sta {4330h eth1}\
# # # 	-lan_ip 192.168.1.1\
# # # 	-power {tmpCtl 1}\
# # #     -relay 4330Exp \
# # #     -lanpeer 4330Exp \
# # #     -console "mc50tst1:40002"\
# # #     -nvram "bcm94330OlympicUNO3.txt"\
# # #     -tag "FALCON_TWIG_5_90_156"\
# # #     -brand "linux-external-dongle-usb"\
# # #     -customer "olympic"\
# # #     -type "4330b2-roml/usb-g-p2p-pno-nocis-oob-idsup.bin.trx"\
# # #     -host_tag "RTRFALCON_REL_5_130_28"\
# # #     -host_brand linux26-internal-hsic\
# # #     -host_nvram {
# # # 		lan_ipaddr=192.168.1.1
# # # 		wandevs=dummy
# # # 		clkfreq=530,176,88
# # # 		watchdog=6000
# # # 		console_loglevel=7
# # #     }

#   updated to RC28 on 7/27/12      ;# -host_tag "RTRFALCON_REL_5_130_14"    
    
#     -tag "FALCON_BRANCH_5_90"\ ; tag loaded OK 8/3/11    
#     -tag "FALCON_REL_5_90_156_14"\ or _13
#    -coldboot 0\
# 43239h configure -ipaddr 192.168.1.70    
# 4330h configure -ipaddr 192.168.1.70 
# 		console_loglevel=7 ;# from host_nvram section

# # # 4330h clone 4330b1h\
# # #     -console "mc50tst1:40001"\
# # #     -tag FALCON_{BRANCH,REL}_5_90_*\
# # #     -type "4330b1-roml/*.bin.trx"

# # # 4330h clone 4330b0h\
# # #     -tag FALCON_{BRANCH,REL}_5_90_*\
# # #     -type "4330b0-roml/*.bin.trx"

# # # # Celia is dual-band

# # # 4330h clone 4330B2Celia\
# # # 	-tag FALCON_BRANCH_5_90\
# # #     -type "4330b2-roml/usb-ag-p2p-pno-nocis-oob-idsup.bin.trx"\
# # # 	-nvram "4330b2-roml/bcm94330OlympicCelia.txt"
# # # 	
#     -tag "FALCON_REL_5_90_156_13"\ ; alternate tag	

# uno3h is 4330B1 UNO3 single-band; no current image found as of 10/14/11

# 4330h clone uno3h \
# 	-host_tag "RTRFRTRFALCON_REL_5_130_10" \
#     -host_brand linux26-internal-hsic-phoenix \
#     -tag "FALCON_BRANCH_5_90"\
#     -type "4330b1-roml/usb-ag-p2p*.bin.trx"
# 	-type "4330b1-roml/usb-ag-p2p-pno-nocis-oob-idsup.bin.trx"

# 4330h clone uno3h \
# 	-host_tag "RTRFRTRFALCON_REL_5_130_14" \
#     -host_brand linux26-internal-hsic \
#     -tag "FALCON_REL_5_90_130"\
# 	-type "4330b1-roml/usb-ag-p2p*.bin.trx"
	
# 	-type "4330b1-roml/usb-g-p2p-pno-nocis-keepalive-proptxstatus-aoe-pool-oob-idsup-wapi.bin.trx"
	
# # # 4330h clone 4334h\
# # # 	-nvram "src/shared/nvram/bcm94334fcagb.txt" \
# # #     -tag "PHOENIX2_{BRANCH,REL}_6_10_*" \
# # #     -type "4334b1-roml/*.bin.trx" \
# # #     -brand "linux-external-dongle-usb" -customer "olympic" \
# # #     -host_brand "linux26-internal-hsic-phoenix2" \
# # #     -host_tag "RTRFALCON_REL_5_130_28" \
# # #     -host_nvram {
# # #         lan_ipaddr=192.168.1.1
# # #         wandevs=dummy
# # #         clkfreq=530,176,88
# # #         watchdog=6000
# # #     }\
# # #     -wlinitcmds {wl event_msgs 0x10000000000000}
# # #     
#     -tag "PHOENIX_{BRANCH,REL}_5_120_*" \
#     -type "4334a0-roml/usb-ag-nodis-suspres-msgtrace-p2p-pno-nocis-keepalive-proptxstatus-aoe-pool-oob-idsup-wapi.bin.trx" \
#     -type "4334a0min-roml/usb-ag-p2p-pno-nocis-keepalive-aoe-oob-idsup-wapi-sr.bin.trx" \

# # # 4330h clone 4334hTmp\
# # # 	-nvram "4334a0-roml/bcm94334fcagb.txt" \
# # #     -tag "PHOENIX_BRANCH_5_120" \
# # #     -type "4334a0-roml/usb-g-suspres-msgtrace-pool-oob.bin.trx" \
# # #     -brand "linux-external-dongle-usb" -customer "olympic" \
# # #     -host_brand "linux26-internal-hsic-phoenix" \
# # #     -host_tag "RTRFALCON_REL_5_130_14" \
# # #     -host_nvram {
# # #         lan_ipaddr=192.168.1.1
# # #         wandevs=dummy
# # #         clkfreq=530,176,88
# # #         watchdog=6000
# # #     }\
# # #     -wlinitcmds {wl event_msgs 0x10000000000000}

# # # #         -type "4334a0-roml/usb-g-nodis-suspres-oob-msgtrace.bin.trx" \

# # # 4330h clone 4334B1hCent\
# # # 	-nvram "Sundance/centennial.txt" \
# # #     -tag "PHOENIX2_BRANCH_6_10" \
# # #     -type "Sundance/centennial.bin.trx" \
# # #     -brand "linux-external-dongle-usb" -customer "olympic" \
# # #     -host_brand "linux26-internal-hsic-olympic" \
# # #     -host_tag "RTRFALCON_REL_5_130_27" \
# # #     -host_nvram {
# # #         lan_ipaddr=192.168.1.1
# # #         wandevs=dummy
# # #         clkfreq=530,176,88
# # #         watchdog=6000
# # #     }\
# # #     -wlinitcmds {wl event_msgs 0x10000000000000}

# # # #         -type "4334a0-roml/usb-g-nodis-suspres-oob-msgtrace.bin.trx" \

# # # ## setting up for 43342 HSIC: 

# # # 4330h clone 43342h\
# # #     -console "mc50tst1:40001"\
# # # 	-nvram "src/shared/nvram/bcm943342fcagb.txt" \
# # #     -tag "PHOENIX2_BRANCH_6_10" \
# # #     -type "43342a0min-roml/usb-g.bin.trx" \
# # #     -brand "linux-external-dongle-usb" -customer "bcm" \
# # #     -host_brand "linux26-internal-hsic-phoenix2" \
# # #     -host_tag "RTRFALCON_REL_5_130_28" \
# # #     -host_nvram {
# # #         lan_ipaddr=192.168.1.1
# # #         wandevs=dummy
# # #         clkfreq=530,176,88
# # #         watchdog=6000
# # #     }\
# # #     -wlinitcmds {wl event_msgs 0x10000000000000}\
# # #     -postinstall {dhd -i eth1 serial 1}
# # #     
# # # 4330h clone 4334hStd\
# # # 	-lan_ip 10.19.87.99\
# # # 	-lanpeer ""\
# # # 	-relay "4330Exp"\
# # # 	-nvram "4334a0-roml/bcm94334fcagb.txt" \
# # #     -tag "PHOENIX_BRANCH_5_120" \
# # #     -type "4334a0-roml/usb-g-nodis-suspres-msgtrace-pool-oob.bin.trx" \
# # #     -brand "linux-external-dongle-usb" -customer "olympic" \
# # #     -host_brand "linux26-internal-hsic-phoenix" \
# # #     -host_tag "RTRFALCON_REL_5_130_14" \
# # #     -host_nvram {
# # #         lan_ipaddr=10.19.87.99
# # #         lan_netmask=255.255.252.0
# # #         lan_gateway=10.19.84.1
# # #         wandevs=dummy
# # #         clkfreq=530,176,88
# # #         watchdog=6000
# # #     }\
# # #     -wlinitcmds {wl event_msgs 0x10000000000000}\
# # #     -nocal 1
# # # #     -type "4334a0-roml/usb-g-nodis-suspres-oob-msgtrace.bin.trx" \
        
#     -host_nvram {
#         lan_ipaddr=192.168.1.1
#         wandevs=dummy
#         clkfreq=530,176,88
#         watchdog=6000
#     }\
#     -wlinitcmds {wl event_msgs 0x10000000000000}    
    
#     -nvram "4334a0-roml/bcm94334fcagb.txt" \
#     -tag "PHOENIX_BRANCH_5_120" \
#     -type "4334a0-roml/usb-ag-nodis-suspres-msgtrace-p2p-pno-nocis-keepalive-proptxstatus-aoe-pool-oob-idsup-wapi.bin.trx" \
#     -brand "linux-external-dongle-usb" -customer "olympic" \
#     -host_brand "linux26-internal-hsic-phoenix" \
#     -host_tag "RTRFALCON_REL_5_130_10" \
#     -host_nvram {
#         lan_ipaddr=192.168.1.1
#         wandevs=dummy
#         clkfreq=530,176,88
#         watchdog=6000
#     }\
#     -wlinitcmds {wl event_msgs 0x10000000000000}
    
    
#     -tag "PHOENIX_REL_5_120_37" \ This branch did not load with 4334sdio     
#     -nocal 1 -nopm1 1 -nopm2 1 -nochannels 1 -noaes 1 -notkip 1
# 
# from TimA 8/11/2011
#     -tag "PHOENIX_BRANCH_5_120" \
#     -type "4334a0-roml/usb-ag-nodis-suspres-msgtrace-p2p-pno-nocis-keepalive-aoe-pool-oob-idsup-wapi.bin.trx" \
#     -brand "linux-external-dongle-usb" -customer "olympic" \
#     -wlinitcmds {wl event_msgs 0x10000000000000} \


    
# physically located in MC50
# replaced by 4334b1 HSIC as of 9/23/13

# # # UTF::DHD mc28tst5 \
# # #     -sta "4336b1 eth1" \
# # #     -tag FALCON_BRANCH_5_90 \
# # #     -type "4336b1-roml/sdio" \
# # #     -nvram "4336b1-roml/bcm94336sdgfc.txt" \
# # # 	-power {npc3 1} \
# # #     -console "mc28tst5:40000" \
# # #     -postinstall {dhd -i eth1 ioctl_timeout 1000; dhd -i eth1 serial 1} \
# # #     -wlinitcmds {wl mpc 0} \
# # #     -post_perf_hook {{%S wl nrate} {%S wl rate}} \
# # #     -noafterburner 1
# # # #     -post_perf_hook {{%S wl nrate} {%S wl rate} {%S rte mu}} \
# # # #     -postinstall {dhd -i eth1 ioctl_timeout 1000; dhd -i eth1 serial 1} \
# # # #     -type "4336b1-roml/sdio-pool" \;# driver name changed since 6/1/2011 -- -pool suffix dropped
# # # # 	-power_button {auto} \

# # # 4336b1 clone 4336b1Twig -tag FALCON_TWIG_5_90_126 -type "4336b1-roml/sdio-pool-apsta-idsup-idauth"
# # # 4336b1 clone 4336b1Final -tag FALCON_TWIG_5_90_100 -type "4336b1-roml/sdio-pool-apsta-ccx-idsup-idauth-pno-aoe-toe-pktfilter-keepalive-wapi.bin"


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
    
# # #     -relay 4334HSIC\
# # #     -lanpeer 4334HSIC\
    
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
 	
# # # # # # 	-tag "PHO2178RC40_REL_6_21_?"
# # # # # # 	-tag "PHO2203RC1_REL_6_25_*"

4334hInnsbruck clone 4334hInnsbruck_0628 -tag "PHO2203RC1_REL_6_25_61_8"
4334hInnsbruck clone 4334hInnsbruck_0712 -tag "PHO2203RC1_REL_6_25_63_9"
4334hInnsbruck clone 4334hInnsbruck_0726 -tag "PHO2203RC1_REL_6_25_63_*"
4334hInnsbruck clone 4334hInnsbruck_twig63 -tag "PHO2203RC1_REL_6_25_63_*"
4334hInnsbruck clone 4334hInnsbruck_current -tag "PHO2203RC1_REL_6_25_*"

UTF::DHD HRELAY\
	-lan_ip 10.19.84.248\
	-sta "4334HSIC eth1"
# # # \
# # # 	-tag "FALCON_BRANCH_5_90"\
# # #  	-power_sta "10.19.85.200 3"\
# # # 	-power "pwrCtl 2"\
# # #     -brand "linux-internal-dongle"\
# # #     -customer "bcm"\
# # #     -type "4329b1/sdio-ag-cdc-full11n-reclaim-roml-wme-idsup.bin"\
# # #     -nvram "src/shared/nvram/bcm94329sdagb.txt"\
# # #     -pre_perf_hook {{AP1-4322-4717 wl ampdu_clear_dump}}\
# # #     -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4322-4717 wl dump ampdu}}

# 
# STA3: Laptop DUT Dell E6410; installed 43224
UTF::Cygwin mc50tst3 -user user -sta {43224Vista} \
	-osver 6 \
	-installer inf \
	-tcpwindow 512k \
	-power {npc3 2} \
	-power_button {auto} \
	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}
#	-tag BASS_BRANCH_5_60
# 
# # clone STA3s
43224Vista clone 43224Vista-BASS-REL -tag BASS_REL_5_60_106
43224Vista clone 43224Vista-KIRIN -tag KIRIN_BRANCH_5_100
43224Vista clone 43224Vista-KIRIN-REL -tag KIRIN_REL_5_100_82_12
    
# STA4: Linux desktop; 
# 4319B0 board marking: BCM94319SDELNA6L
# replaced 4319b0 with 4334b1 SDIO 9/20/13; upgraded to FC15 2/21/14

UTF::DHD mc50tst4 \
	-lan_ip 10.19.86.203 \
	-sta "4319sdio eth0" \
	-tcpwindow 512k \
	-tag ROMTERM2_BRANCH_4_219 \
	-power {npc2 2} \
	-power_button {auto} \
   -device_reset "172.16.50.30 1"\
	-nvram "src/shared/nvram/bcm94319sdelna6l.txt" \
	-brand linux-internal-dongle \
    -customer "bcm" \
    -type "4319b0sdio/sdio-g-cdc-roml-wme-full11n-reclaim-idsup-promiscuous.bin" \
    -noafterburner 1 \
	-pre_perf_hook {%S wl rssi} \
    -post_perf_hook {{%S wl nrate} {%S wl rate}} \
    -console "mc50tst4:40000" \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}}
     
#     -wlinitcmds {wl mpc 0} \
#     -postinstall {dhd -i eth1 ioctl_timeout 10000} \
    
# # clone STA4s
# # # 4319sdio clone 4319sdio-Ext \
# # # 	-brand linux-external-dongle-sdio \
# # # 	-type "4319b0sdio/sdio-ag-cdc-full11n-reclaim-roml-wme-idsup-medioctl-40m-memredux40m.bin" \
# # # 	-postinstall {dhd -i eth1 ioctl_timeout 10000}

# 4334b1 replaced by 43438a0 9/8/14

4319sdio clone 43438a0\
	-tag BISON_BRANCH_7_10\
	-brand linux-external-dongle-sdio\
	-type 43430a0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr.bin\
	-nvram src/shared/nvram/bcm943438wlpth_26MHz.txt

# # # 	-dhd_brand {} -dhd_tag DHD_REL_1_201_34 -driver dhd-cdc-sdstd-debug
	
43438a0 clone 43438a0_twig -tag BISON_REL_7_10_323_*	
	
# # # 4319sdio clone 4334b1\
# # #    -tag PHOENIX2_{BRANCH,REL}_6_10{,_*}\
# # #    -brand linux-external-dongle-sdio\
# # #    -type "4334b1{,min}-roml/sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-sr-wapi-wl11d-autoabn.bin"\
# # #    -wlinitcmds ""\
# # #    -postinstall "dhd -i eth0 sd_divisor 1"\
# # #    -nvram "src/shared/nvram/bcm94334fcagb.txt"\
# # #    -noaes 1 -notkip 1\
# # #    -dhd_tag NIGHTLY
# # # 
# # # # # #    sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-sr-wapi-wl11d-autoabn ;# from last test request
# # # # # #       -type "4334b1{,min}-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan-sr.bin" ;# original 10/12/13
# # # 
# # # 4334b1 clone 4334b1_coex -tag PHOENIX2_REL_6_10* -wlinitcmds "wl btc_mode 1"
# # # 4334b1 clone 4334b1_dbg -type "4334b1{,min}-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan-sr.bin"
# # # 4334b1_dbg clone 4334b1_dbg_coex -wlinitcmds "wl btc_mode 1"

   
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
# 	-tag "COMANCHE2_REL_5_22_80" \
#     -tag COMANCHE2_REL_5_22_?? \

# clone AP1s
4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
# # # 4717COMANCHE2 clone mc50_AP1
4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
# # # 4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_??" -brand linux-external-router
4717 clone 4717MILLAU -tag "MILLAU_REL_5_70*" -brand linux-external-router
4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_??" -brand linux-internal-router
4717 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
# 4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
#changed to linux-external-router-combo on 3/31/12: linux-external-router image depleted
4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router-combo
4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
# 4717 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router ;# external build depleted in Akashi ToB
4717 clone 4717AkashiExt -tag "AKASHI_{REL,TWIG}_5_110_*" -brand linux-external-router
4717 clone 4717AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand linux-internal-router

4717MILLAU clone mc50_AP1

# 5.70.38 depleted as of 8/21/11
# 4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
# 4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
