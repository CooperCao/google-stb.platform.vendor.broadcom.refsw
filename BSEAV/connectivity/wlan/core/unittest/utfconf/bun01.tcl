#
# Bunnik UTF Test Configuration
#
# UTF configuration for BUN01 testbed @ Broadcom Bunnik
#

# load packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Power
package require UTF::DHD

UTF::Logfile "bun01.log"
#set ::UTF::SummaryDir "/projects/hnd_software_nl/sig/$::env(LOGNAME)/bun01"
#set ::UTF::SummaryDir "/home/$::env(LOGNAME)/UTF/bun01"
set ::UTF::SummaryDir "/projects/hnd_software_ext3/$::env(LOGNAME)/UTF/bun01"

# this is needed for multiple STA to multiple AP tests
#set ::sta_list "43143sdio 43242fmac 4334b1 43569 43570fd 43602fd"
#set ::ap_list "4708/4360 4708/4331"

# Power controllers
UTF::Power::Blackbox lb-bun-71 \
    -lan_ip 10.176.8.71

UTF::Power::Blackbox lb-bun-72 \
    -lan_ip 10.176.8.72

UTF::Power::Synaccess apac_pwr \
    -lan_ip 192.168.1.100 \
    -relay aplan1 \
    -rev 1


# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 10.176.8.67 \
        -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # First pause a few seconds so that back to back tests don't
    # cause failures
    #
    UTF::Sleep 10

    #
    # Make Sure Attenuators are set to 0 value
    #
    #
    # Do the attenuator last, sometimes it get hosed with
    # utf error couldn't open socket.
    #
    G1 attn 0;
    G2 attn 0;
    # Change G3 to 0 or 20 for P2P testing (43224 <-> 43242)
    G3 attn 100;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    # AP1 restart wl0_radio=0

    # Disable USB devices
    43242bmac power_sta cycle
    43569 power_sta cycle
    43143sdio power_sta cycle

    # delete myself
    unset ::UTF::SetupTestBed

    return
}

# Set testrig specific attenuation ranges for use with RvRNightly1.test
set ::cycle5G40AttnRange "0-65 65-0"
set ::cycle5G20AttnRange "0-65 65-0"
set ::cycle2G40AttnRange "0-65 65-0"
set ::cycle2G20AttnRange "0-65 65-0"

##### Relay / Consolelogger
##### Running FC15
UTF::Linux lb-bun-85 \
    -sta {aplan1 p5p1} \
    -lan_ip 10.176.8.85 \
    -user root
aplan1 configure -ipaddr 192.168.1.50


##### DUT Host Machine
##### FC11 Linux 
UTF::Linux lb-bun-231 \
    -sta {43602 enp1s0} \
    -power {lb-bun-72 1} \
    -lan_ip "10.176.8.231" \
    -user root \
    -console "10.176.8.54:10004" \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -type obj-debug-apdef-stadef \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -preinstall_hook {{%S dmesg -n 7}} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M \
    -yart {-attn5g 45-100 -attn2g 45-100}
#    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1 } \

43602 configure -attngrp G1

43602 clone 43602_wlan-go \
    -type obj-debug-apdef-stadef-p2p-mchan-tdls
 
43602_wlan-go clone 43602_p2p-go \
    -sta {43602_p2p-go wl0.1 }

##### DUT Host Machine
##### FC11 Linux 
UTF::DHD lb-bun-231-dhd \
    -sta {43602fd eth0} \
    -power {lb-bun-72 1} \
    -lan_ip "10.176.8.231" \
    -user root \
    -hostconsole "10.176.8.54:10004" \
    -tag BISON_BRANCH_7_10 \
    -dhd_tag NIGHTLY \
    -driver "dhd-msgbuf-pciefd-debug" \
    -brand linux-external-dongle-pcie \
    -type "43602a0-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1;} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -customer "bcm" \
    -nobighammer 1 \
    -slowassoc 5 -noaes 1 -notkip 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2g -nocal 1 \
    -yart {-attn5g 45-100 -attn2g 45-100}

43602fd configure -attngrp G1

43602fd clone 43602fd_perf \
    -brand linux-external-dongle-pcie \
    -type "43602a0-ram/pcie-ag-splitrx"

43602fd clone 43602fdap \
    -brand linux-external-dongle-pcie \
    -type "43602a0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-wnm-txbf-pktctx-amsdutx-ampduretry.bin"

43602fd clone 43602fdap \
    -brand linux-internal-dongle-pcie \
    -type "43602a0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-wnm-txbf-pktctx-amsdutx-ampduretry-err-assert.bin"

43602fd clone 43602fd_wlan-go \
    -type "43602a0-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx"
 
43602fd_wlan-go clone 43602fd_p2p-go \
    -sta {43602fd_p2p-go wl0.1 }

43602fd clone 43143sdio \
    -power_sta {lb-bun-72 3} \
    -tag PHOENIX2_{TWIG,REL}_6_10_198{,_*} \
    -dhd_tag NIGHTLY \
    -brand linux-external-dongle-sdio \
    -driver {} \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus" \
    -nvram "bcm943143eds_p202.txt" \
    -noafterburner 1 \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1; } \
    -postinstall { dhd -i eth0 sdiod_drive 12; dhd -i eth0 sdiod_drive; sleep 1; dhd -i eth0 sd_divisor 1; dhd -i eth0 sd_divisor; sleep 1;} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters}} \
    -customer "bcm" \
    -noaes 1 -notkip 1 \
    -perfchans { 1l 1 } -nocal 1 -slowassoc 5
#    -postinstall {dhd -i eth1 serial 1; dhd -i eth1 msglevel 0x20001; } \
#    -nocal 1 \
#    -modopts {sd_uhsimode=3} \

43143sdio clone 43143sdio_trunk \
    -tag NIGHTLY \
    -type "43143-ram/sdio-g-p2p-mchan"

43143sdio clone 43143sdio_mfgtest \
    -brand "linux-mfgtest-dongle-sdio" \
    -type "43143b0-roml/sdio-g-mfgtest-seqcmds"

43143sdio clone 43143sdio_rel \
    -dhd_tag DHD_REL_1_131_* \
    -tag PHOENIX2_REL_6_10_198_*

43143sdio_rel clone 43143sdio_relmfgtest \
    -brand "linux-mfgtest-dongle-sdio" \
    -type "43143b0-roml/sdio-g-mfgtest-seqcmds"

43143sdio clone 43143sdio_wlan-go \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus"
 
43143sdio_wlan-go clone 43143sdio_p2p-go \
    -sta {43143sdio_p2p-go wl0.1 }

43143sdio_rel clone 43143sdio_wlan-go_rel \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus"
 
43143sdio_wlan-go_rel clone 43143sdio_p2p-go_rel \
    -sta {43143sdio_p2p-go_rel wl0.1 }

# USB bmac dongle
43143sdio clone 43242bmac \
    -power_sta {lb-bun-72 4} \
    -console "10.176.8.85:40002" \
    -tag BISON_BRANCH_7_10 \
    -dhd_tag {} \
    -brand "linux-internal-wl-media" \
    -driver "debug-apdef-stadef-high-p2p-mchan-tdls-wowl-media" \
    -type "43242a1-bmac/ag-assert-p2p-mchan-srvsdb-media-vusb/rtecdc.bin.trx" \
    -nvram "bcm943242usbref_p461.txt" \
    -nvram_add "usbdesc_composite=0x010A" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1; } \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -slowassoc 5 -nocal 1 -noaes 0 -notkip 0 \
    -perfchans {} -postinstall {} \
    -customer ""

43242bmac clone 43242bmac_bis \
    -tag BISON_BRANCH_7_10 \
    -brand linux-internal-media \
    -driver "debug-apdef-stadef-high-p2p-mchan-tdls-wowl-media" \
    -type "43242a1-bmac/ag-assert-p2p-mchan-media-srvsdb-vusb.bin.trx" \
    -nvram_add "usbdesc_composite=0x010A" \
    -customer "bcm"

43242bmac_bis clone 43242bmac_trunk -tag trunk

43242bmac clone 43242bmac_wlan-go \
    -type "43242a1-bmac/ag-assert-p2p-mchan/rtecdc.bin.trx"
 
43242bmac_wlan-go clone 43242bmac_p2p-go \
    -sta {43242bmac_p2p-go wl0.1 }

43242bmac clone 43242bmac_rel \
    -brand "linux-external-wl-media-full-src" \
    -driver "nodebug-apdef-stadef-high-p2p-mchan-tdls-media" \
    -tag "AARD14RC50_REL_6_37_114_*" \
    -type "43242a1-bmac/ag-p2p-mchan-srvsdb-media-vusb/rtecdc.bin.trx"

43242bmac_rel clone 43242bmac_wlan-go_rel \
    -type "43242a1-bmac/ag-p2p-mchan-srvsdb-media-vusb/rtecdc.bin.trx"
 
43242bmac_wlan-go_rel clone 43242bmac_p2p-go_rel \
    -sta {43242bmac_p2p-go_rel wl0.1 }

43242bmac_rel clone 43242bmac_relmfgtest \
    -brand "linux-mfgtest-wl"\
    -driver "debug-apdef-stadef-high-mfgtest" \
    -type "43242a1-bmac/ag-assert-mfgtest/rtecdc.bin.trx"

# USB full dongle
43242bmac clone 43242fmac \
    -tag PHOENIX2_{TWIG,REL}_6_10_224{,_*} \
    -app_tag PHOENIX2_REL_6_10_198_* \
    -brand linux-internal-dongle-usb \
    -driver dhd-cdc-usb-comp-gpl \
    -type "43242a1-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tdls-srvsdb-pclose-proptxstatus-vusb.bin.trx" \
    -perfchans { 36l 1 } -noaes 1 -notkip 1 \
    -customer "bcm" \
    -dhd_tag NIGHTLY
#    -wlinitcmds {} \
#    -dhd_image /home/pieterpg/dhd-cdc-usb-gpl-debug-F11.ko
#    -perfchans {} -noaes 1 -notkip 1 -datarate {-skiptx 32} \

43242fmac clone 43242fmac_trunk \
    -brand linux-internal-dongle-usb \
    -tag NIGHTLY \
    -type "43242-ram/usb-ag-p2p-mchan.bin.trx"

43242fmac clone 43242fmac_mfgtest \
    -type "43242a1-roml/usb-ag-mfgtest-internal-phydbg.bin.trx"

43242fmac clone 43242fmac_rel \
    -brand linux-external-dongle-usb \
    -type "43242a1-roml/usb-ag-p2p-mchan-idauth-idsup-pktfilter-wowlpf-tdls-srvsdb-pclose-proptxstatus-vusb.bin.trx" \
    -dhd_tag DHD_REL_1_141_5_* \
    -tag PHOENIX2_REL_6_10_224_*

43242fmac_rel clone 43242fmac_relmfgtest \
    -brand linux-mfgtest-dongle-usb \
    -type "43242a1-roml/usb-ag-mfgtest-phydbg.bin.trx"

43242fmac clone 43242fmac_wlan-go \
    -type "43242a1-roml/usb-ag-p2p-mchan-idauth-idsup-pktfilter-wowlpf-tdls-srvsdb-pclose-proptxstatus-vusb.bin.trx"
 
43242fmac_wlan-go clone 43242fmac_p2p-go \
    -sta {43242fmac_p2p-go wl0.1 }

43242fmac_rel clone 43242fmac_wlan-go_rel \
    -type "43242a1-roml/usb-ag-p2p-mchan-idauth-idsup-pktfilter-wowlpf-tdls-srvsdb-pclose-proptxstatus-vusb.bin.trx"
 
43242fmac_wlan-go_rel clone 43242fmac_p2p-go_rel \
    -sta {43242fmac_p2p-go_rel wl0.1 }

##### DUT Host Machine
##### FC11 Linux 
UTF::Linux lb-bun-232 \
    -sta {43570 enp1s0} \
    -power {lb-bun-71 1} \
    -lan_ip "10.176.8.232" \
    -user root \
    -console "10.176.8.54:10003" \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -type obj-debug-apdef-stadef \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}

43570 configure -attngrp G2

43570 clone 43570_wlan-go \
    -type obj-debug-p2p-mchan
 
43570_wlan-go clone 43570_p2p-go \
    -sta {43570_p2p-go wl0.1 }

43570 clone 43570_rel \
    -tag BISON05T_REL_7_35_* \
    -brand "linux-internal-media" \
    -type "debug-apdef-stadef-high-p2p-mchan-tdls-wowl-media"

43570_rel clone 43570_rel_wlan-go
 
43570_rel_wlan-go clone 43570_rel_p2p-go \
    -sta {43570_rel_p2p-go wl0.1}

##### DUT Host Machine
##### FC11 Linux 
UTF::DHD lb-bun-232-dhd \
    -sta {43570fd eth0} \
    -power {lb-bun-71 1} \
    -lan_ip "10.176.8.232" \
    -user root \
    -hostconsole "10.176.8.54:10003" \
    -tag BISON_BRANCH_7_10 \
    -dhd_tag NIGHTLY \
    -driver "dhd-msgbuf-pciefd-debug" \
    -brand linux-external-media \
    -type "43570a0-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-mfp-sr-proptxstatus-ampduhostreorder-keepalive" \
    -nvram "src/shared/nvram/bcm943570pcieir_p110.txt" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1;} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -customer "bcm" \
    -nobighammer 1 \
    -slowassoc 5 -noaes 1 -notkip 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2g -nocal 1 \
    -yart {-attn5g 45-100 -attn2g 45-100}

43570fd configure -attngrp G2

43570fd clone 43570fd_05T \
    -tag BISON05T_BRANCH_7_35 \
    -dhd_tag DHD_REL_1_201_*

43570fd_05T clone 43570fd_05T_wlan-go
43570fd_05T_wlan-go clone 43570fd_05T_p2p-go \
    -sta {43570fd_05T_p2p-go wl0.1}

43570fd clone 43570fd_rel \
    -tag BISON05T_REL_7_35_63 \
    -dhd_tag DHD_REL_1_201_9

43570fd_rel clone 43570fd_rel_wlan-go
43570fd_rel_wlan-go clone 43570fd_rel_p2p-go \
    -sta {43570fd_rel_p2p-go wl0.1}

43570fd clone 4334b1 \
    -tag "PHOENIX2_BRANCH_6_10" \
    -dhd_tag "NIGHTLY" \
    -brand linux-internal-dongle \
    -type "4334b1min-roml/sdio-ag-idsup-p2p-*-dmatxrc.bin" \
    -nvram "src/shared/nvram/bcm94334fcagb.txt" \
    -noafterburner 1 \
    -wlinitcmds {wl mpc 0; } \
    -perfchans { 1 } -nocal 1 \

4334b1 clone 4334b1_wlan-go \
    -type "4334b1min-roml/sdio-ag-idsup-p2p-*-dmatxrc.bin" \
 
4334b1_wlan-go clone 4334b1_p2p-go \
    -sta {4334b1_p2p-go wl0.1 }

43570fd clone 43569 \
    -power_sta {lb-bun-71 4} \
    -console "10.176.8.85:40003" \
    -tag BISON_BRANCH_7_10 \
    -dhd_tag NIGHTLY \
    -brand "linux-internal-media" \
    -driver dhd-cdc-usb-comp-gpl \
    -type "43569a0-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb.bin.trx" \
    -nvram "bcm943569usb_p309.txt" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1 } \
    -postinstall {}

43569 clone 43569_05T \
    -tag BISON05T_BRANCH_7_35 \
    -dhd_tag DHD_REL_1_201_*

43569_05T clone 43569_05T_wlan-go
43569_05T_wlan-go clone 43569_05T_p2p-go \
    -sta {43569_05T_p2p-go wl0.1}

43569 clone 43569_rel \
    -brand "linux-external-media" \
    -tag BISON05T_REL_7_35_63 \
    -dhd_tag DHD_REL_1_201_9

43569_rel clone 43569_rel_wlan-go
43569_rel_wlan-go clone 43569_rel_p2p-go \
    -sta {43569_rel_p2p-go wl0.1}


##### Broadcom Router
# AP Section
# Netgear R6300v2
# 4708 AP with 4360 and 4331 on MOBO
UTF::Router 4708 -sta {
    4708/4360 eth2
    4708/4331 eth1
} \
    -lan_ip 192.168.1.1 \
    -relay {lb-bun-85} \
    -lanpeer {aplan1} \
    -brand linux-2.6.36-arm-internal-router \
    -console "10.176.8.85:40001" \
    -power {apac_pwr 1} \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -nvram {
	# watchdog=3000 (default)
	lan_ipaddr=192.168.1.1
	lan1_ipaddr=192.168.2.1
	dhcp_start=192.168.1.110
	dhcp_end=192.168.1.150
	wl_msglevel=0x101
	wl0_ssid=BUN01_4708/4331
	wl0_channel=3
	wl0_bw_cap=-1
	wl0_radio=0
	wl0_obss_coex=0
	wl1_ssid=BUN01_4708/4360
	wl1_channel=36
	wl1_bw_cap=-1
	wl1_radio=0
	wl1_obss_coex=0
	#Only 1 AP can serve DHCP Addresses
	#router_disable=1
	et_msglevel=0; # WAR for PR#107305
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -noradio_pwrsave 1

# "AARDVARK_REL_6_30_70"
#    -tag "AARDVARK_REL_6_30_39_2001" \
# Used for RvRFastSweep.test
#4708/4360 configure -attngrp G1
#4708/4331 configure -attngrp G1
4708/4331 clone 4708/chanperf \
    -datarate {}
4708/4331 clone 4708/rel \
    -tag "AARDVARK01T_REL_6_37_14_44"

#Scheduler for rack using controller
UTF::Q bun01 lb-bun-85
