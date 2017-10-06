# -*-tcl-*-
#
# testbed configuration for Peter Kwan MC50 testbed
# created 6/28/11
# ====================================================================================================
# Test rig ID: SIG MC63
#
# Packages
package require UTF::WinBT
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut BTCohostXP		      	;# BlueTooth device under test
set ::bt_ref BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 4330B2-DUT  			    ;# HND WLAN device under test
set ::wlan_rtr 4717a   				;# HND WLAN router
set ::wlan_tg lan0              	;# HND WLAN traffic generator

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc63"

# NPC definitions
UTF::Power::Synaccess npc1 -lan_ip 172.16.1.8
UTF::Power::Synaccess npc2 -lan_ip 172.16.1.9
UTF::Power::Synaccess npc3 -lan_ip 172.16.1.10
UTF::Power::WebRelay 172.16.1.12
UTF::Power::WebRelay 172.16.1.13
UTF::Power::WebRelay 172.16.1.40 ;# webrelay in box2 for 4350c0

# Attenuator
# G1: WLAN; G2: BT; G3: No connect
UTF::Aeroflex af -lan_ip 172.16.1.11 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}

UTF::Linux mc63end1 \
    -sta {lan0 eth1} 

# this is windows sniffer
UTF::Cygwin mc63tst5 -user user -sta {43224Vista} \
	-osver 6 \
	-installer inf \
	-tcpwindow 512k \
	-power {npc3 2} \
	-power_button {auto} \
	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

# # clone STA3s
43224Vista clone 43224Vista-BASS-REL -tag BASS_REL_5_60_106
43224Vista clone 43224Vista-KIRIN -tag KIRIN_BRANCH_5_100
43224Vista clone 43224Vista-KIRIN-REL -tag KIRIN_REL_5_100_82_12
43224Vista clone 43224Vista-AARDVARK -tag AARDVARK_BRANCH_6_30

# # # mc63end2 ip_addr: 10.19.87.131 OS: FC15 9/5/14

UTF::DHD mc63end2\
	-sta "mc63DUT2 eth0"\
	-device_reset "172.16.1.40 1"

mc63DUT2 clone 43438a0\
	-tag BISON_BRANCH_7_10\
	-brand linux-external-dongle-sdio\
	-type 43430a0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr.bin\
	-nvram src/shared/nvram/bcm943438wlpth_26MHz.txt\
	-dhd_tag DHD_REL_1_201_34 -driver dhd-cdc-sdstd-debug\
	-postinstall {} -wlinitcmds {} -slowassoc 5\
	-nobighammer 1 -nocal 1 -noaes 1 -notkip 1\
	-power {npc3 1}

43438a0 clone 43438a0_coex -wlinitcmds "wl btc_mode 1"
43438a0 clone 43438a0_twig\
	-tag BISON_REL_7_10_323_*

43438a0_twig clone 43438a0_twig_coex -wlinitcmds "wl btc_mode 1"
43438a0 clone 43438a1\
	-type 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr-11nprop-tdls-hs20sta-mchandump.bin\
	-dhd_tag DHD_REL_1_201_9
43438a1 clone 43438a1td -dhd_tag TRUNK
43438a1 clone 43438a1_coex -wlinitcmds "wl btc_mode 1"
43438a1td clone 43438a1td_coex -wlinitcmds "wl btc_mode 1"

43438a1 clone 43438a1t\
	-tag BISON_REL_7_10_323_*\
	-type 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr-tdls-hs20sta-aibss-relmcast.bin
43438a1t clone 43438a1tx -dhd_tag TRUNK
43438a1t clone 43438a1t_coex -wlinitcmds "wl btc_mode 1"
43438a1tx clone 43438a1tx_coex -wlinitcmds "wl btc_mode 1"

43438a1 clone 43438a1_06T\
	-tag BISON06T_BRANCH_7_45
43438a1_06T clone 43438a1_06T_coex -wlinitcmds "wl btc_mode 1"

43438a1_06T clone 43438a1_06T_t -dhd_tag TRUNK
43438a1_06T_t clone 43438a1_06T_t_coex -wlinitcmds "wl btc_mode 1"

43438a1_06T clone 43438a1_local1 -image "/home/pkwan/BCM43430/rtecdc_536250.bin"\
	-wlinitcmds "wl btc_mode 1"

43438a1_local1 clone 43438a1_local2 -image "/home/pkwan/BCM43430/rtecdc_536626.bin"

mc63DUT2 clone 43435b0\
	-tag DINGO2_BRANCH_9_15\
	-brand hndrte-dongle-wl\
	-type 43435b0-roml/config_sdio_debug/rtecdc.bin\
	-nvram src/shared/nvram/bcm943430fsdnge_elna_Bx.txt\
	-clm_blob 43435b0.clm_blob\
	-dhd_tag DHD_BRANCH_1_359\
	-app_tag DHD_BRANCH_1_359\
    -dhd_brand linux-external-dongle-sdio\
	-slowassoc 5 -nobighammer 1 -nocal 1 -noaes 1 -notkip 1\
	-power {npc3 1}

# prior to 7/9/15
# 	-dhd_tag DHD_REL_1_363_17\
# 	-app_tag trunk\
# replaced as of 7/13/15 per request:
# 	-type 43435b0-roml/sdio-g-p2p-pno-nocis-keepalive-aoe-idsup-ve-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-wnm-wl11u-anqpo-noclminc-logtrace-srscan-mpf-pmmt-clm_min-srmem-srfast-err-assert/rtecdc.bin\
	
43435b0 clone 43435b0_coex -wlinitcmds {wl btc_mode 1}
# name change per development 10/13/15
43435b0 clone 43430b0 -type 43430b0-roml/config_sdio_debug/rtecdc.bin -clm_blob 43430b0.clm_blob
43430b0 clone 43430b0_coex -wlinitcmds {wl btc_mode 1}
# 43430b0 clone 43430b0_daytona -tag DINGO2_REL_9_15_182_1 -dhd_tag DHD_REL_1_359_91 -app_tag DHD_REL_1_359_91
# 43430b0 clone 43430b0_daytona -tag DINGO2_REL_9_15_196_* -dhd_tag DHD_REL_1_359_91 -app_tag DHD_REL_1_359_91
# 43430b0 clone 43430b0_daytona -tag DINGO2_REL_9_15_24* -dhd_tag DHD_REL_1_359_115 -app_tag DHD_REL_1_359_115
43430b0 clone 43430b0_daytona \
	-tag DINGO2_REL_9_15_245_1 \
	-type 43430b0-roml/config_sdio_debug/rtecdc.bin -clm_blob 43430b0.clm_blob \
	-dhd_tag DHD_REL_1_359_115 -app_tag DHD_REL_1_359_115

43430b0_daytona clone 43430b0_daytona_coex -wlinitcmds {wl btc_mode 1}
43430b0 clone 43430b0_915r -tag DINGO2_REL_9_15_* -dhd_tag DHD_REL_1_359_9* -app_tag DHD_REL_1_359_9*
43430b0_915r clone 43430b0_915r_coex -wlinitcmds {wl btc_mode 1}

mc63DUT2 clone 43430b0_mac\
	-tag DINGO2_REL_9_15_182_1 -dhd_tag DHD_REL_1_359_91 -app_tag DHD_REL_1_359_91\
	-brand linux-external-dongle-sdio\
	-customer olympic\
	-type ../C-43430__s-B0/maccabeelite.trx -clm_blob ../C-43430__s-B0/maccabeelite.S-2__F-1_2__E-1_2_3_4_5_6_7_high.clmb \
	-nvram ../C-43430__s-B0/P-maccabeelite_M-MCBL_V-u__m-1.0.txt\
    -dhd_brand linux-external-dongle-sdio\
	-slowassoc 5 -nobighammer 1 -nocal 1 -noaes 1 -notkip 1\
	-power {npc3 1}
	
43430b0_mac clone 43430b0_mac_coex -wlinitcmds {wl btc_mode 1}

### added BTCohost2 for 4350c0

UTF::WinBT BTCOHOST2\
	-lan_ip mc63tst1\
	-sta "BTCohost2"\
	-bt_comm "com5@3000000"\
	-bt_comm_startup_speed 115200\
    -device_reset "172.16.1.40 2"\
    -user user\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_ss_location 0x0021E000\
    -bt_power_adjust 40\
	-bt_xtal_freq 40\
    -type BCM4350/C0\
	-version BCM4350C0_*\
	-brand "Generic/UART/37_4MHz/wlbga_BU_eLNA"\
	-file *.cg*
	
# # # 	    -power "npc1 2"\

BTCohost2 clone BTCohost4350c0
BTCohost2 clone BTCohost43438a0\
	-bt_comm "com6@3000000"\
	-bt_ss_location 0x00210750\
	-bt_xtal_freq 37.4\
	-type BCM4343/A0\
	-version BCM4343A0_*\
	-brand "Generic/UART/26MHz/wlbga_PTH"\
	-file *.hcd

BTCohost43438a0 clone BTCohost43438a0_cfg51 -version BCM4343A0*0051*
BTCohost43438a0 clone BTCohost43438a1 -bt_ss_location 0x00211700 -bt_w_size 251
BTCohost43438a0 clone BTCohost43438a1x -bt_ss_location 0x00211700 -bt_w_size 251 -type BCM4343/A1 -version BCM4343A1_* -brand "Generic/UART/26MHz/wlbga_ref"
BTCohost43438a1x clone BTCohost43438a1x_local -project_dir /home/pkwan -brand "Generic/UART/26MHz/wlbga_BU"
BTCohost43438a1x clone BTCohost43438a1_bu -brand "Generic/UART/26MHz/wlbga_BU"

BTCohost2 clone BTCohost43430\
	-bt_comm "com6@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
	-type BCM4343/A0\
    -bt_ss_location 0x00210750\
    -brand Generic/UART/26MHz/wlbga_eLG\
	-version *\
    -file *.hcd

BTCohost43430 clone BTCohost43435B0 \
	-project_dir "/home/pkwan" -device_reset "" \
	-bt_ss_location 0x00211700 \
	-type BCM43435B0 \
	-brand Generic/UART/37_4MHz/fcbga_eLNA_ePA_bypassed\
	-file *.cgr
	
# 	-brand Generic/UART/37_4MHz/fcbga_iLNA_ePA\ ;# original entry: eLNA_ePA
# 	-brand Generic/UART/37_4MHz/fcbga_eLNA_ePA_bypassed_Olympic\

# BTCohost43435B0 clone BTCohost43430B0\
# 	-type BCM43430 -version * -brand ""
	
BTCohost43430 clone BTCohost43430B0 \
	-bt_ss_location 0x00211700 \
	-type BCM43430 -version B0 \
	-brand *13.*_eLNA_ePA_bypassed_*\
	-file *.cgr

BTCohost43430B0 clone BTCohost43430B0_daytona -brand *14.*_eLNA_ePA_bypassed_*
	
BTCohost43435B0 clone BTCohost_Maccabee\
	-type BCM43430\
	-version *TDK*\
	-brand ""	

UTF::WinBT BTCOHOST\
	-lan_ip mc63tst2\
	-sta "BTCohostXP"\
	-bt_comm "com4@3000000"\
	-bt_comm_startup_speed 115200\
        -device_reset "172.16.1.12 1"\
        -power "npc1 2"\
        -user user\
        -bt_xtal_freq 26\
        -bt_power_adjust 40\
        -type BCM4330/B1\
        -bt_ss_location 0x00210000\
        -brand ""\
        -version BCM4330B1_*_McLaren

BTCohostXP clone BTCohost43342\
	-bt_comm "com14@3000000"\
    -bt_ss_location 0x00210000\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
	-type BCM43342/A1\
	-brand ""\
	-version "*Chardonnay*"\
	-file *.cgs

BTCohost43342 clone BTCohost43342_local\
	-image "/home/pkwan/BCM43342/BCM43342A1_11.4.780.808_Southpaw_OS_USI_20131023.cgs"
	
BTCohost43342 clone BTCohost_southpaw -type BCM43342/A1_All_Builds -version "*/Olympic" -brand "Southpaw_OS/USI"
BTCohost43342 clone BTCohost_southpaw_elna -type BCM43342/A1 -brand "" -version "*Southpaw_OS_USI_eLNA*"
BTCohost43342 clone BTCohost_southpaw_ilna -type BCM43342/A1 -brand "" -version "*Southpaw_OS_USI_iLNA*"
# 	-bt_comm "com6@3000000"\


UTF::WinBT BTREFXP\
	-lan_ip mc63tst3\
        -sta "BTRefXP_reset"\
        -type "BCM2046"\
        -bt_comm "usb0"\
        -power "npc3 1"\
        -user user\
        -device_reset "172.16.1.13 1"
 
BTRefXP_reset clone BTRefXP -device_reset ""
        
UTF::DHD MC63DUT2 \
	-lan_ip mc63tst4 \
	-sta {STA-Linux-43362A0 eth1} \
        -device_reset "172.16.1.12 2" \
        -power "172.16.1.12 3"\
        -hostconsole mc63end1:40002 \
	-tag FALCON_BRANCH_5_90 \
	-nvram "src/shared/nvram/bcm943362sdg.txt" \
        -brand linux-internal-dongle \
        -type "43362a0-roml/sdio-p2p-idsup-idauth.bin"\
        -customer "bcm" \
        -wlinitcmds {wl mpc 0} 

#changed to FC15 on 10/3/13

UTF::DHD MC63DUT \
	-lan_ip mc63tst4 \
	-sta {STA-Linux-4330sdio eth0} \
        -device_reset "172.16.1.12 2" \
        -power "172.16.1.12 3"\
	-tag FALCON_BRANCH_5_90 \
        -nvram "src/shared/nvram/bcm94330OlympicUNO3.txt" \
        -brand linux-internal-dongle \
        -type "4330b0-roml/sdio-g*.bin"\
        -customer "bcm" \
        -wlinitcmds {wl mpc 0}  
        
# # #     -hostconsole mc63end1:40002 \;# disabled after moving over to FC15 10/4/13

STA-Linux-4330sdio clone 43342A1ToB\
    -tag PHO2203RC1_BRANCH_6_25\
    -brand linux-external-dongle-sdio\
    -dhd_tag NIGHTLY\
    -dhd_brand linux-external-dongle-sdio\
    -customer "olympic"\
	-type "{,*/}43342a*/southpaw.bin" -clm_blob "43342_southpaw.clm_blob"\
	-nvram "{,*/}43342a*/bcm943342SouthpawUSIK.txt"\
	-wlinitcmds ""\
	-console mc63tst4:40000\
	-power "npc1 1"\
	-tcpwindow 1152k -udp 400m -nokpps 1 -nobighammer 1 \
    -datarate {-i 0.5 -frameburst 1}

43342A1ToB clone 43342A1ToB_coex -wlinitcmds {wl btc_mode 1}
43342A1ToB clone 43342A1ToBx -nvram "{,*/}43342a*/southpaw-u-kk.txt" -clm_blob "43342_southpaw.clm_blob"
43342A1ToBx clone 43342A1ToBx_coex -wlinitcmds {wl btc_mode 1}
43342A1ToB clone 43342A1_t178 -tag PHO2203RC1_TWIG_6_25_178
43342A1_t178 clone 43342A1_t178_coex -wlinitcmds {wl btc_mode 1}
43342A1ToB clone 43342A1_r178 -tag PHO2203RC1_REL_6_25_178*
43342A1_r178 clone 43342A1_r178_coex -wlinitcmds {wl btc_mode 1}
43342A1_r178 clone 43342A1_r178d -dhd_date 2015.8.6.

#Cisco E4200 eth1 (intended for 2G) and eth2 (intended for 5G)
UTF::Router AP2 \
    -sta {4718 eth1 4718/4331 eth2} \
    -lan_ip 192.168.1.1 \
    -relay "mc63end1" \
    -lanpeer lan0 \
    -console "mc63end1:40003" \
    -power {npc2 1} \
    -brand linux-external-router \
    -nvram {
        et0macaddr=00:90:4c:32:00:00
        macaddr=00:90:4c:32:01:0a
	lan_ipaddr=192.168.1.1
	lan_gateway=192.168.1.1
	dhcp_start=192.168.1.100
  	dhcp_end=192.168.1.149
	lan1_ipaddr=192.168.2.1
	lan1_gateway=192.169.2.1
	dhcp1_start=192.168.2.100
        dhcp1_end=192.168.2.149
       	fw_disable=1
       	wl_msglevel=0x101
        wl0_ssid=mc63test
        wl0_channel=1
        wl0_radio=1
        antswitch=0
        wl0_obss_coex=0
}

UTF::Router AP1 \
    -sta {4717a eth1} \
    -lan_ip 192.168.1.1 \
    -relay "mc63end1" \
    -lanpeer lan0 \
    -console "mc63end1:40003" \
    -power {npc2 1} \
    -brand linux-external-router \
    -nvram {
        et0macaddr=00:90:4c:32:00:00
        macaddr=00:90:4c:32:01:0a
        lan_ipaddr=192.168.1.1
        lan_gateway=192.168.1.1
        dhcp_start=192.168.1.100
        dhcp_end=192.168.1.149
        lan1_ipaddr=192.168.2.1
        lan1_gateway=192.169.2.1
        dhcp1_start=192.168.2.100
        dhcp1_end=192.168.2.149
        fw_disable=1
        wl_msglevel=0x101
        wl0_ssid=mc63test
        wl0_channel=1
        wl0_radio=1
        antswitch=0
        wl0_obss_coex=0
}

# clone AP1s
4717a clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
4717a clone 4717MILLAU -tag "MILLAU_REL_5_70_48_27" -brand linux-internal-router
4717a clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router 

# clone AP2s
4718 clone 4718COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
4718 clone 4718Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
4718 clone 4718MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
4718 clone 4718MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
4718 clone 4718MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
4718 clone 4718MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
4718 clone 4718Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
4718 clone 4718AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

set ::UTF::SetupTestBed {
     # G1: wlan; G2: BT
	 G1 attn 10
	 G2 attn 20
     if {[catch {43342A1ToB wl down} ret]} {UTF::Message ERROR "" "Error shutting down 43342A1ToB radio..."}
     if {[catch {43438a0 wl down} ret]} {UTF::Message ERROR "" "Error shutting down 43438a0 radio..."}
     
     43342A1ToB deinit
     43438a0 deinit
     
     unset ::UTF::SetupTestBed
     return
}

unset UTF::TcpReadStats
