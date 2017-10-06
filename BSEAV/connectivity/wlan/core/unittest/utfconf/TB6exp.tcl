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
### set UTF::SummaryDir "/projects/hnd_sig_ext3/$::env(LOGNAME)/CoExTB6" 
# moved to dedicated filer volume 10/30/13
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/CoExTB6"

# Define power controllers on cart.
# package require UTF::Power
# - btref webrelay 10.19.85.199 pwr for 2046B1 ref
UTF::Power::WebRelay 10.19.85.199
# - dut webrelay 10.19.85.200 bt & wl reset lines
UTF::Power::WebRelay 10.19.85.200 -invert {1 2}
# - dut webrelay 172.16.24.20 DUT3 BT & wl reset lines
UTF::Power::WebRelay 172.16.24.20
# AP pwr control & console
UTF::Power::Synaccess 10.19.85.209
UTF::Power::Synaccess 10.19.85.207
UTF::Power::Synaccess pwrCtl -lan_ip 10.19.85.152

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.24.10 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {

    G1 attn 15 ;# changed to 15 from 0 on 1/25/13 ;# changed to 18 after changing to conducted RF on 10/21/11
# # #   	G1 attn 20 ;# changed to 30 from 10 on 11/16/12 ;# changed to 0 from 10 on 7/20/12 ;# changed from 18 to 10 on 5/17/12
#	G2 changed from 10 to 20 on 8/16/11
	G2 attn 5 ;# changed to 10 from 0 7/26/12 ;# changed to 0 from 40 7/20/12
	G3 attn 103
#     ALL attn 0

    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# mc24end1  10.19.85.195   Linux UTF Host   - TB6UTF
# -sta "lan eth0"

UTF::Linux TB6UTF  -lan_ip 10.19.85.195  -sta "lan eth1"

package require UTF::HSIC

# device name mc61end11, ip_add 10.19.85.149
# SDIO 3.0 device replaced HSIC as of 9/18/13

UTF::DHD TB6DUT3 \
	-lan_ip 10.19.85.149\
	-sta "sdio3 eth0"\
	-power "pwrCtl 2"\
	-device_reset "172.16.24.20 1"
    
sdio3 clone 4339wlipa\
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
    
4339wlipa clone 4335c0_epa -nvram "bcm94339wlbgaFEM_AM.txt" 

# device name mc61end11, ip_add 10.19.85.149

# # # UTF::HSIC hExp\
# # # 	-sta {4334h eth1}\
# # # 	-lan_ip 192.168.1.1\
# # # 	-power {pwrCtl 1}\
# # #     -relay 4334HSIC\
# # #     -lanpeer 4334HSIC\
# # #     -console "mc61end11:40000"\
# # #     -nvram "Sundance/centennial.txt"\
# # #     -tag "PHOENIX2_BRANCH_6_10"\
# # #     -type "Sundance/centennial.bin.trx"\
# # #     -brand "linux-external-dongle-usb" -customer "olympic"\
# # #     -host_brand "linux26-internal-hsic-olympic"\
# # #     -host_tag "RTRFALCON_REL_5_130_28"\
# # #     -host_nvram {
# # #         lan_ipaddr=192.168.1.1
# # #         wandevs=dummy
# # #         clkfreq=530,176,88
# # #         watchdog=6000
# # #         console_loglevel=7
# # #         lan_stp=0
# # #         lan1_stp=0
# # #     }\
# # #     -wlinitcmds {wl event_msgs 0x10000000000000}\
# # #     -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
# # #     -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}}\
# # #     -nocal 1
# # #     
# # # 4334h clone 4334hSundance\
# # # 	-tag "PHOENIX2_REL_6_10_56_???"\
# # # 	-type "Sundance/centennial.trx"\
# # # 	-nvram "/home/pkwan/BCM4334/centennial.txt"
# # #     
# # # 4334hSundance clone 4334hBrighton\
# # # 	-type "Brighton/centennial.trx"
# # # 	
# # # 4334hSundance clone 4334hInnsbruck\
# # # 	-tag "PHO2203RC1_BRANCH_6_25"\
# # # 	-type "Innsbruck/{,4334b1/}centennial.trx"\
# # # 	-host_tag RTRFALCON_REL_5_130_48
# # # 	
# # # # # # 	-tag "PHO2178RC40_REL_6_21_?"
# # # # # # 	-tag "PHO2203RC1_REL_6_25_*"

# # # 4334hInnsbruck clone 4334hInnsbruck_0628 -tag "PHO2203RC1_REL_6_25_61_8"
# # # 4334hInnsbruck clone 4334hInnsbruck_0712 -tag "PHO2203RC1_REL_6_25_63_9"
# # # 4334hInnsbruck clone 4334hInnsbruck_0726 -tag "PHO2203RC1_REL_6_25_63_*"
# # # 4334hInnsbruck clone 4334hInnsbruck_twig63 -tag "PHO2203RC1_REL_6_25_63_*"
# # # 4334hInnsbruck clone 4334hInnsbruck_current -tag "PHO2203RC1_REL_6_25_*"
# # # 	
# # # 4334hSundance clone 4334hInnsbruck_old\
# # # 	-tag "PHOENIX2_REL_6_10_178_??"\
# # # 	-type "Innsbruck/centennial.trx"\
# # # 	-host_tag RTRFALCON_REL_5_130_48
# # # 	
# # # # # # 		-host_tag RTRFALCON_REL_5_130_32 ;# original P220 on-board rtr host build ;# updated to 5.130.48 on 2/8/13

# # # 4334h clone 43342h\
# # # 	-nvram "bcm94334fcagb.txt"\
# # # 	-tag ""\
# # # 	-type ""\
# # # 	-brand ""
# # # 		
# # # #     -nvram "bcm94330OlympicUNO3.txt"\
# # # #     -tag "FALCON_TWIG_5_90_156"\
# # # #     -brand "linux-external-dongle-usb"\
# # # #     -customer "olympic"\
# # # #     -type "4330b2-roml/usb-g-p2p-pno-nocis-oob-idsup.bin.trx"\
# # # #     -host_tag "RTRFALCON_REL_5_130_14"\
# # # #     -host_brand linux26-internal-hsic\
# # # #     -host_nvram {
# # # # 		lan_ipaddr=192.168.1.1
# # # # 		wandevs=dummy
# # # # 		clkfreq=530,176,88
# # # # 		watchdog=6000
# # # # 		console_loglevel=7
# # # #     }

# details for BTCohost4334 TBD; hostname mc61end12, ip 10.19.85.150

# # # UTF::WinBT TB6_BTCOHOSTXP2\
# # # 	-lan_ip 10.19.85.150\
# # # 	-sta "BTCohost4334"\
# # # 	-bt_comm "com4@3000000"\
# # # 	-bt_comm_startup_speed 115200\
# # #     -user user\
# # #     -bt_xtal_freq 26\
# # #     -bt_power_adjust 40\
# # #     -bt_ss_location 0x00210000\
# # #     -type BCM4334/B1\
# # #     -brand ""\
# # #     -version BCM4334B1_*Cent_OS_MUR_*\
# # #     -file *.cg*

# BTCohost for 4339 GC in upper enclosure
    
UTF::WinBT TB6_BTCOHOSTXP2\
 -lan_ip 10.19.85.150\
 -sta "BTCohost4339_gc"\
 -bt_comm "com6@3000000"\
 -bt_comm_startup_speed 115200\
 -user user\
 -device_reset "172.16.24.20 2"\
 -bt_rw_mode "Cortex M3 HCI"\
 -bt_w_size 200\
 -bt_xtal_freq 37.4\
 -bt_power_adjust 40\
 -bt_ss_location 0x00218000\
 -type BCM4339\
 -brand Generic/UART/37_4MHz/wlbga_iPA\
 -version *
 
BTCohost4339_gc clone BTCohost4339_epa -brand Generic/UART/37_4MHz/wlbga_ePA

# # #     	-bt_comm "com4@3000000"\ ;# 4334b1HSIC setting ;#com6 is for 4339wlipa
        
# # #     -version BCM4334*_Olympic_Cent_OS_MUR_*\
    
# # # BTCohost4334 clone BTCohost4334_RoW\
# # # 	-bt_comm "com5@3000000"\
# # # 	-bt_comm_startup_speed 115200\
# # #     -user user\
# # #     -bt_xtal_freq 26\
# # #     -bt_power_adjust 40\
# # #     -bt_ss_location 0x00210000\
# # #     -type BCM4334/B1\
# # #     -brand Generic/37_4MHz/fcbga_extLna\
# # #     -version *
    
# # #  bt_comm moved to com4 from com3 with changing HSIC P210 to P220 on 1/2/13
    
# 	-power "172.16.54.25 1"\
# 	-device_reset "10.19.87.110 2"\

# 4330h clone 4334B1hCent\
# 	-nvram "Sundance/centennial.txt" \
#     -tag "PHOENIX2_BRANCH_6_10" \
#     -type "Sundance/centennial.bin.trx" \
#     -brand "linux-external-dongle-usb" -customer "olympic" \
#     -host_brand "linux26-internal-hsic-olympic" \
#     -host_tag "RTRFALCON_REL_5_130_27" \
#     -host_nvram {
#         lan_ipaddr=192.168.1.1
#         wandevs=dummy
#         clkfreq=530,176,88
#         watchdog=6000
#     }\
#     -wlinitcmds {wl event_msgs 0x10000000000000}

UTF::DHD TB6DUT2\
	-lan_ip 10.19.85.149\
	-sta "4334HSIC eth1"\
	-tag "FALCON_BRANCH_5_90"\
 	-power_sta "10.19.85.200 3"\
	-power "pwrCtl 2"\
    -brand "linux-internal-dongle"\
    -customer "bcm"\
    -type "4329b1/sdio-ag-cdc-full11n-reclaim-roml-wme-idsup.bin"\
    -nvram "src/shared/nvram/bcm94329sdagb.txt"\
    -pre_perf_hook {{AP1-4322-4717 wl ampdu_clear_dump}}\
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4322-4717 wl dump ampdu}}
    
UTF::DHD 4334b1sdio\
   -lan_ip 10.19.85.149\
   -sta "4334b1_GC eth2"\
   -console "mc61end11:40034"\
   -power {pwrCtl 2}\
   -tag PHOENIX2_REL_6_10_58_*\
   -brand linux-external-dongle-sdio\
   -type "4334b1min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan-sr.bin"\
   -wlinitcmds ""\
   -postinstall "dhd -i eth2 sd_divisor 1"\
   -nvram "src/shared/nvram/bcm94334fcagb.txt"\
   -pre_perf_hook {{AP1-4322-4717 wl ampdu_clear_dump}}\
   -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4322-4717 wl dump ampdu}}
	
UTF::DHD TB6DUT\
	-lan_ip 10.19.85.196\
	-sta "STA-Linux-4329sdio eth1"\
	-tag "ROMTERM2_BRANCH_4_219"\
	-device_reset "10.19.85.200 2"\
	-power "10.19.85.207 1"\
    -brand "linux-internal-dongle"\
    -customer "bcm"\
    -type "4329b1/sdio-ag-cdc-full11n-reclaim-roml-wme-idsup.bin"\
    -nvram "src/shared/nvram/bcm94329sdagb.txt"\
    -pre_perf_hook {{AP1-4322-4717 wl ampdu_clear_dump}}\
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4322-4717 wl dump ampdu}}

# # #      	-power_sta "10.19.85.200 3"\ ; controls USB power to DUCKS2

    
# clones of TB6DUT
# # # STA-Linux-4329sdio clone 4329sdio_ROMTERM3 -tag "ROMTERM3_BRANCH_4_220" -nvram "bcm94329sdagb.txt"
# # # STA-Linux-4329sdio clone 4329sdio_RT3Ext -tag "ROMTERM3_BRANCH_4_220" -brand "linux-external-dongle-sdio"
# # # STA-Linux-4329sdio clone 4329sdio_ROMTERM -tag "ROMTERM_BRANCH_4_218"
# # # STA-Linux-4329sdio clone 4329b1_Ref -tag "ROMTERM_REL_4_218_???"
# # # STA-Linux-4329sdio clone 4329b1_Ref_RvR -tag "ROMTERM_REL_4_218_???" -wlinitcmds {wl btc_mode 0}
# # # STA-Linux-4329sdio clone 4329b1_DASH -tag ROMTERM3_REL_4_220_??
# # # STA-Linux-4329sdio clone 4329b1_RT2Twig\
# # #   -tag RT2TWIG46_REL_4_221_???\
# # #   -type "4329b1/sdio-ag-cdc-full11n-reclaim-roml-idsup-nocis-wme-memredux16-pno-aoe-pktfilter-minioctl-29agbf-keepalive.bin"
# # #   
# # # STA-Linux-4329sdio clone 43241b0\
# # #    -tag PHOENIX2_REL_6_10_116_*\
# # #    -brand linux-external-dongle-sdio\
# # #    -type "43241b0min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-opt2.bin"\
# # #    -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
# # #    -wlinitcmds {wl btc_mode 0}\
# # #    -datarate {-skiptx 32}\
# # #    -postinstall {dhd -i eth1 sd_divisor 1}

# # # 43241b0 clone 43241b0x -tag PHOENIX2_REL_6_10_116_6
# # # 43241b0 clone 43241b0d\
# # #    -dhd_image "/projects/hnd_software_ext7/work/maheshd/code/tot/trunk/src/dhd/linux/dhd-cdc-sdstd-2.6.25-14.fc9.i686/dhd.ko"
# # # 43241b0 clone 43241b0roml\
# # #    -type "43241b0-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-opt2-phydbg-autoabn.bin"
# # # 43241b0roml clone 43241b0romld\
# # #    -dhd_image "/projects/hnd_software_ext7/work/maheshd/code/tot/trunk/src/dhd/linux/dhd-cdc-sdstd-2.6.25-14.fc9.i686/dhd.ko"
# # # 43241b0 clone 43241b0_pwr -power_sta "10.19.85.200 3" 

STA-Linux-4329sdio clone 4334b1\
   -tag PHOENIX2_REL_6_10_*\
   -brand linux-external-dongle-sdio\
   -type "4334b1min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan-sr.bin"\
   -wlinitcmds ""\
   -postinstall "dhd -i eth1 sd_divisor 1"\
   -nvram "src/shared/nvram/bcm94334fcagb.txt"
     
# # #    sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-mchan.bin   
# # #    -wlinitcmds {wl btc_mode 0}\
# # #    -datarate {-skiptx 32}\
# # #    -postinstall {dhd -i eth1 sd_divisor 1}

### replaced 4334b1 with 4345b1 7/26/13
# # # STA-Linux-4329sdio clone 4350b1\
# # #    -tag AARDVARK_BRANCH_6_30\
# # #    -brand linux-external-dongle-sdio\
# # #    -dhd_tag NIGHTLY -slowassoc 5\
# # #    -dhd_brand linux-internal-dongle\
# # #    -modopts {sd_uhsimode=2 sd_txglom=1 sd_tuning_period=10}\
# # #    -type "4350b1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth-assert-err.bin"\
# # #    -wlinitcmds "wl ampdu_mdpu 16; wl vht_features 1; wl bw_cap 2g -1"\
# # #    -tcpwindow 2m -udp 400m \
# # #    -nobighammer 1\
# # #    -postinstall "dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 40"\
# # #    -nvram "src/shared/nvram/bcm94350wlagbe_KA.txt"
# # #    
# # # 4350b1 clone 4350b1_sdio2 -wlinitcmds "" -postinstall "dhd -i eth1 sd_divisor 1" -modopts ""  

UTF::DHD TB6DUT4 \
	-lan_ip 10.19.85.196\
	-sta "sdio3_go eth0"\
	-device_reset "10.19.85.200 2"\
	-power "10.19.85.207 1"
    
sdio3_go clone 4339wlipa_go\
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

# # # new device added 12/6/12; replacing 43241b0

# # # UTF::DHD TB6DUT\
# # # 	-lan_ip 10.19.85.196\
# # # 	-sta "43342 eth1"\
# # # 	-tag "PHOENIX2_BRANCH_6_10"\
# # # 	-device_reset "10.19.85.200 2"\
# # # 	-power "10.19.85.207 1"\
# # #     -customer "bcm"\
# # #     -nvram "src/shared/nvram/bcm943342fcagb.txt"\
# # #     -pre_perf_hook {{AP1-4322-4717 wl ampdu_clear_dump}}\
# # #     -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S rte mu} {AP1-4322-4717 wl dump ampdu}}

# # # 43342 clone 43342a0\
# # #  -type "43342a0min-roml/sdio-ag-pno-p2p-ccx-extsup-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-idsup-idauth-vsdb-wapi-wl11d.bin"\
# # #  -dhd_tag NIGHTLY \
# # #  -brand linux-external-dongle-sdio \
# # #  -dhd_brand linux-internal-dongle

# # # 43228XP: for RF debug only
 
# # # UTF::Cygwin mc50tst5 -user user -sta {43228XP} \
# # # 	-osver 5 \
# # # 	-installer inf \
# # # 	-tcpwindow 512k \
# # # 	-tag AARDVARK_BRANCH_6_30 \
# # # 	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
# # #     -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}} \
# # # 	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} 
	
# # # 	-power {npc4 1} \
# # # 	-power_button {auto} \ 
	    
# BT Cohost - WinXP laptop BlueTooth 4329 USB
# mc24tst2  10.19.85.198   Vista_BtCoHost   - TB6BTCOHOST
# The line below this must remain blank or you will get this msg: invalid command name "TB6BTCOHOST"!!

UTF::WinBT TB6BTCOHOST\
	-lan_ip 10.19.85.198\
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
    
BTCohost_4329USB clone BTCohost4339_go\
	-bt_comm "com10@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -bt_ss_location 0x00218000\
	-type BCM4339\
	-brand Generic/UART/37_4MHz/wlbga_iPA\
	-version *

# BT Ref - WinXP laptop BlueTooth 2046 Reference
# mc24tst1  10.19.85.197   BtRef_XP     - TB6BTREF
#	-power_sta "10.19.85.199 1"

UTF::WinBT BTREFXP\
	-lan_ip 10.19.85.197\
    -sta "BTRefXP"\
    -type "BCM2046"\
	-power_sta "10.19.85.199 1"\
    -bt_comm "usb0"\
    -user user

#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router TB6AP1\
    -lan_ip 192.168.1.2\
    -sta "AP1-4322-4717 eth1"\
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
	w10_nmode=1
	wl0_antdiv=0
	antswitch=0
	obss_coex=0
    }
# Specify router antenna selection to use
set ::wlan_rtr_antsel 0x01
    
# et0macaddr=00:90:4c:61:00:00
# macaddr=00:90:4c:61:01:0a
# lan_ipaddr=192.168.1.3
# lan_gateway=192.168.1.3
# dhcp_start=192.168.1.100
# dhcp_end=192.168.1.149
# lan1_ipaddr=192.168.2.1
# lan1_gateway=192.169.2.1
# dhcp1_start=192.168.2.100
# dhcp1_end=192.168.2.149
# fw_disable=1
# wl_msglevel=0x101
# wl0_ssid=TestMC61
# wl0_channel=1
# w10_nmode=1
# wl0_antdiv=0
# antswitch=0
# wl0_obss_coex=0

# changed over to MILLAU in ARCHIVE
# # # AP1-4322-4717 clone TB6_AP1
# # # AP1-4322-4717 clone tb6_AP1
# # # AP1-4322-4717 clone MC24_AP1
# # # AP1-4322-4717 clone mc24_AP1


AP1-4322-4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
AP1-4322-4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
# # # AP1-4322-4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
AP1-4322-4717 clone 4717MILLAU -tag "MILLAU_REL_5_70*" -brand linux-external-router
AP1-4322-4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
AP1-4322-4717 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
AP1-4322-4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
AP1-4322-4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
AP1-4322-4717 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

4717MILLAU clone TB6_AP1
4717MILLAU clone tb6_AP1
4717MILLAU clone MC24_AP1
4717MILLAU clone mc24_AP1