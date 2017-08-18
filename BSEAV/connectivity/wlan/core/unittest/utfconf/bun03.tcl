#
# Bunnik UTF Test Configuration
#
# UTF configuration for BUN03 testbed @ Broadcom Bunnik
#

# load packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Power
package require UTF::DHD

UTF::Logfile "bun03.log"
#set ::UTF::SummaryDir "/projects/hnd_software_nl/sig/$::env(LOGNAME)/bun02"
#set ::UTF::SummaryDir "/home/$::env(LOGNAME)/UTF/bun02"
set ::UTF::SummaryDir "/projects/hnd_software_ext3/$::env(LOGNAME)/UTF/bun03"

# this is needed for multiple STA to multiple AP tests
#set ::sta_list "43143sird 43242fmac 43143eds 43143fmac"
#set ::ap_list "4706/4360 4706/4331"

# Power controllers
UTF::Power::Synaccess apac_pwr \
    -lan_ip 192.168.2.50 \
    -relay aplan1 \
    -rev 1

UTF::Power::Blackbox bun207_pwr \
    -lan_ip 192.168.2.52 \
    -relay aplan1

UTF::Power::Synaccess bun208_pwr \
    -lan_ip 192.168.2.51 \
    -relay aplan1 \
    -rev 1

UTF::Power::Synaccess 4708_pwr \
    -lan_ip 192.168.2.54 \
    -relay aplan1 \
    -rev 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 10.176.8.90 \
        -group {G1 {1 2 3} G2 {4 5 6} G3 {7 8 9} ALL {1 2 3 4 5 6 7 8 9}}

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
    # Do the attenuator last, sometimes it get hosed with
    # utf error couldn't open socket.
    #
    G1 attn 30;
    G2 attn 10;
    G3 attn 100;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    # AP1 restart wl0_radio=0

    # Disable USB devices
    43242bmac power_sta cycle
#    43143bmac power_sta cycle
    43143sird power_sta cycle
    43143eds power_sta cycle

    # delete myself
    unset ::UTF::SetupTestBed

    return
}

# Set testrig specific attenuation ranges for use with RvRNightly1.test
set ::cycle5G40AttnRange "45-100 100-45"
set ::cycle5G20AttnRange "45-100 100-45"
set ::cycle2G40AttnRange "45-100 100-45"
set ::cycle2G20AttnRange "45-100 100-45"

##### Relay / Consolelogger
##### Running FC15
UTF::Linux lb-bun-87 \
    -sta {aplan1 p3p1} \
    -lan_ip 10.176.8.87 \
    -user root
aplan1 configure -ipaddr 192.168.2.99


##### DUT Host Machine
##### FC15 Linux 
UTF::Linux lb-bun-207 \
    -sta {43602 eth0} \
    -power {bun207_pwr 1} \
    -lan_ip "10.176.8.207" \
    -user root \
    -console "10.176.8.87:40001" \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -type obj-debug-apdef-stadef \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -preinstall_hook {{%S dmesg -n 7}} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M \
    -yart {-attn5g 45-100 -attn2g 45-100} \
    -wlinitcmds {
        echo default > /sys/module/pcie_aspm/parameters/policy;
#	ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; mimo_bw_cap 1; wl assert_type 1; wl vht_features 3;
    }
#    -slowassoc 5 -reloadoncrash 1 \
#    -nobighammer 1 \

43602 configure -attngrp G1

43602 clone 43602_rel \
    -tag BISON_REL_7_10_212

43602 clone 43602_wlan-go \
    -type obj-debug-p2p-mchan
 
43602_wlan-go clone 43602_p2p-go \
    -sta {43602_p2p-go wl0.1 }

43602 clone 43602_nodhcp
43602_nodhcp configure -ipaddr 192.168.1.207 -attngrp G2

43602_nodhcp clone 43602_softap \
    -wlinitcmds {
        wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl mimo_bw_cap 1; wl amsdu 0; wl txbf 0;
    }
43602_softap configure -ipaddr 192.168.1.207 -attngrp G2 -ap 1 -hasdhcpd 0

##### DUT Host Machine
##### FC15 Linux 
UTF::DHD lb-bun-207-dhdpcie \
    -sta {43602fd eth0} \
    -power {bun207_pwr 1} \
    -power_sta {bun207_pwr 2} \
    -lan_ip "10.176.8.207" \
    -user root \
    -hostconsole "10.176.8.87:40001" \
    -tag BISON_BRANCH_7_10 \
    -dhd_tag NIGHTLY \
    -driver "dhd-msgbuf-pciefd-debug" \
    -brand linux-internal-dongle-pcie \
    -type "43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx" \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -customer "bcm" \
    -nobighammer 1 \
    -slowassoc 5 -noaes 0 -notkip 0 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2g -nocal 1 \
    -yart {-attn5g 45-100 -attn2g 45-100}
#    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; echo default > /sys/module/pcie_aspm/parameters/policy; wl vht_features 3; } \
#    -type "43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-err-assert" \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl vht_features 3; wl ack_ratio 0} \
#    -noafterburner 1 \
#    -postinstall {dhd -i eth1 serial 1; dhd -i eth1 msglevel 0x20001; dhd -i eth1 sd_divisor 1; dhd -i eth1 proptx 0; } \
#    -nocal 1 \
#    -nvram "fake43602.txt" \

43602fd configure -attngrp G1

43602fd clone 43602fd-err \
    -type "43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx-err-assert"

43602fd clone 43602fd_wlan-go \
    -type "43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx"
 
43602fd_wlan-go clone 43602fd_p2p-go \
    -sta {43602fd_p2p-go wl0.1 }

43602fd clone 43602fdap \
    -brand linux-external-dongle-pcie \
    -type "43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-proptxstatus"
#    -wlinitcmds {wl ampdu_release 48; wl ampdu_mpdu 32; }

43602fdap clone 43602fdap_assert \
    -brand linux-internal-dongle-pcie \
    -type "43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-proptxstatus-err-assert"

43602fdap clone 43602fdap_softap
43602fdap_softap configure -ipaddr 192.168.1.207 -attngrp G2 -ap 1 -hasdhcpd 0
43602fdap_softap clone 43602fdap_softap_prop \
    -type "43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-wnm-txbf-pktctx-amsdutx-ampduretry-proptxstatus"

43602fdap clone 43602fdap_nodhcp
43602fdap_nodhcp configure -ipaddr 192.168.1.207 -attngrp G2

43602fd clone 43143sird \
    -tag PHOENIX2_BRANCH_6_10 \
    -dhd_tag NIGHTLY \
    -brand linux-external-dongle-sdio \
    -driver {} \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus" \
    -nvram "bcm943143sird_p112.txt" \
    -noafterburner 1 -noaes 1 -notkip 1 \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1; } \
    -modopts { sd_tuning_period=10 sd_divisor=1 } \
    -perfchans { 1l 1 }



43143sird clone 43143sird_trunk \
    -tag NIGHTLY \
    -type "43143-ram/sdio-g-p2p-mchan"

43143sird clone 43143sird_mfgtest \
    -brand "linux-mfgtest-dongle-sdio" \
    -type "43143b0-roml/sdio-g-mfgtest-seqcmds"

43143sird clone 43143sird_rel \
    -dhd_tag DHD_REL_1_131_* \
    -tag PHOENIX2_REL_6_10_198_*

43143sird_rel clone 43143sird_relnodhcp
43143sird_relnodhcp configure -ipaddr 192.168.1.207

43143sird_rel clone 43143sird_relmfgtest \
    -brand "linux-mfgtest-dongle-sdio" \
    -dhd_brand "linux-external-dongle-sdio" \
    -type "43143b0-roml/sdio-g-mfgtest-seqcmds"

43143sird clone 43143sird_wlan-go \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus"
 
43143sird_wlan-go clone 43143sird_p2p-go \
    -sta {43143sird_p2p-go wl0.1 }

43143sird_rel clone 43143sird_wlan-go_rel \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus"
 
43143sird_wlan-go_rel clone 43143sird_p2p-go_rel \
    -sta {43143sird_p2p-go_rel wl0.1 }


# USB bmac dongle
43143sird clone 43242bmac \
    -power_sta {bun207_pwr 3} \
    -console "10.176.8.87:40002" \
    -tag AARDVARK_BRANCH_6_30 \
    -dhd_tag {} \
    -brand "linux-internal-wl-media" \
    -driver "debug-apdef-stadef-high-p2p-mchan-tdls-media" \
    -type "43242a1-bmac/ag-assert-p2p-mchan-srvsdb-media-vusb/rtecdc.bin.trx" \
    -nvram "bcm943242usbref_p461.txt" \
    -nvram_add "usbdesc_composite=0x010A" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1; } \
    -modopts {} \
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

43242bmac clone 43242bmac_wlan-go \
    -driver "obj-debug-p2p-mchan-high" \
    -type "43242a1-bmac/ag-assert-p2p-mchan/rtecdc.bin.trx"
 
43242bmac_wlan-go clone 43242bmac_p2p-go \
    -sta {43242bmac_p2p-go wl0.1 }

43242bmac clone 43242bmac_rel \
	-brand "linux-external-wl-media-full-src" \
    -driver "nodebug-apdef-stadef-high-p2p-mchan-tdls-media" \
    -tag "AARD14RC50_REL_6_37_114_*" \
    -type "43242a1-bmac/ag-p2p-mchan-srvsdb-media-vusb/rtecdc.bin.trx"

43242bmac_rel clone 43242bmac_relmfgtest \
    -brand "linux-mfgtest-wl"\
    -driver "debug-apdef-stadef-high-mfgtest" \
    -type "43242a1-bmac/ag-assert-mfgtest/rtecdc.bin.trx"

# USB full dongle
43242bmac clone 43242fmac \
    -tag PHOENIX2_BRANCH_6_10 \
    -brand linux-internal-dongle-usb \
    -driver dhd-cdc-usb-comp-gpl \
    -type "43242a1-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tdls-srvsdb-pclose-proptxstatus-vusb.bin.trx" \
    -perfchans { 36l 1 } -noaes 1 -notkip 1 \
    -customer "bcm" \
    -dhd_tag NIGHTLY \

#    -dhd_image /home/pieterpg/dhd-cdc-usb-gpl-debug-F15.ko
#    -perfchans {} -noaes 1 -notkip 1 -datarate {-skiptx 32} \

43242fmac clone 43242fmac_trunk \
    -brand linux-internal-dongle-usb \
    -tag NIGHTLY \
    -type "43242-ram/usb-ag-p2p-mchan.bin.trx"

43242fmac clone 43242fmac_mfgtest \
    -type "43242a1-roml/usb-ag-mfgtest-internal-phydbg.bin.trx"

43242fmac clone 43242fmac_rel \
    -brand linux-external-dongle-usb \
    -type "43242a1-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tdls-srvsdb-pclose-proptxstatus-vusb.bin.trx" \
    -dhd_tag DHD_REL_1_131_* \
    -tag PHOENIX2_REL_6_10_222_*

43242fmac_rel clone 43242fmac_relmfgtest \
    -brand linux-mfgtest-dongle-usb \
    -type "43242a1-roml/usb-ag-mfgtest-phydbg.bin.trx"

43242fmac clone 43242fmac_wlan-go \
    -type "43242a1-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tdls-srvsdb-pclose-proptxstatus-vusb.bin.trx"
 
43242fmac_wlan-go clone 43242fmac_p2p-go \
    -sta {43242fmac_p2p-go wl0.1 }

43242fmac clone 43242fmac_tdls \
    -type "43242a1-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tdls-srvsdb-pclose-proptxstatus-vusb.bin.trx"
43242fmac_tdls configure -tdlswith 43143fmac_tdls

43242fmac_tdls clone 43242fmac_tdlssdio
43242fmac_tdlssdio configure -tdlswith 43143sird_tdls
43242fmac_tdls clone 43242fmac_tdlssdiohp
43242fmac_tdlssdiohp configure -tdlswith 43143eds_tdls

##### DUT Host Machine
##### FC15 Linux 
UTF::Linux lb-bun-208 \
    -sta {4360 eth0} \
    -power {bun208_pwr 1} \
    -lan_ip "10.176.8.208" \
    -user root \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -type obj-debug-apdef-stadef \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -preinstall_hook {{%S dmesg -n 7}} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M \
    -yart {-attn5g 10-100 -attn2g 10-100} \
    -wlinitcmds {wl mimo_bw_cap 1 ; wl vht_features 1; }
#    -console "10.176.8.87:40004" \
#    -wlinitcmds {
#        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; mimo_bw_cap 1; wl assert_type 1; wl vht_features 3;
#    }
#    -slowassoc 5 -reloadoncrash 1 \
#    -nobighammer 1 \

4360 configure -attngrp G2

4360 clone 4360_wlan-go \
    -type obj-debug-p2p-mchan
 
4360_wlan-go clone 4360_p2p-go \
    -sta {4360_p2p-go wl0.1 }

4360 clone 4360_nodhcp
4360_nodhcp configure -ipaddr 192.168.1.208 -attngrp G2

4360_nodhcp clone 4360_softap
4360_softap configure -ap 1 -hasdhcpd 0

##### DUT Host Machine
##### FC15 Linux 
UTF::DHD lb-bun-208-dhd \
    -sta {43143eds eth0} \
    -power {bun208_pwr 1} \
    -power_sta {bun208_pwr 2} \
    -hostconsole "10.176.8.87:40004" \
    -lan_ip "10.176.8.208" \
    -user root \
    -tag PHOENIX2_BRANCH_6_10 \
    -dhd_tag NIGHTLY \
    -brand linux-external-dongle-sdio \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus" \
    -nvram "bcm943143eds_p202.txt" \
    -noafterburner 1 \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1; } \
    -postinstall { dhd -i eth0 sdiod_drive 12; dhd -i eth0 sdiod_drive; dhd -i eth0 sd_divisor 1; dhd -i eth0 sd_divisor; } \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -customer "bcm" \
    -noaes 1 -notkip 1 \
    -perfchans { 1l 1 } -nocal 1 -slowassoc 5
#    -nocal 1 \
#    -modopts {sd_uhsimode=3} \

43143eds configure -attngrp G2

43143eds clone 43143eds_trunk \
    -tag NIGHTLY \
    -type "43143-ram/sdio-g-p2p-mchan"

43143eds clone 43143eds_mfgtest \
    -brand "linux-mfgtest-dongle-sdio" \
    -type "43143b0-roml/sdio-g-mfgtest-seqcmds"

43143eds clone 43143eds_rel \
    -dhd_tag DHD_REL_1_131_* \
    -tag PHOENIX2_REL_6_10_198_*

43143eds_rel clone 43143eds_relnodhcp
43143eds_relnodhcp configure -ipaddr 192.168.1.208

43143eds_rel clone 43143eds_relmfgtest \
    -brand "linux-mfgtest-dongle-sdio" \
    -type "43143b0-roml/sdio-g-mfgtest-seqcmds"

43143eds clone 43143eds_wlan-go \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus"
 
43143eds_wlan-go clone 43143eds_p2p-go \
    -sta {43143eds_p2p-go wl0.1 }

43143eds_rel clone 43143eds_wlan-go_rel \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus"
 
43143eds_wlan-go_rel clone 43143eds_p2p-go_rel \
    -sta {43143eds_p2p-go_rel wl0.1 }

43143eds_rel clone 43143eds_tdls \
    -brand linux-external-dongle-sdio \
    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-tdls-proptxstatus.bin"
43143eds_tdls configure -tdlswith 43242fmac_tdlssdiohp
#    -dhd_tag DHD_REL_1_93 \
#    -type "43143b0-roml/sdio-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-tdls-proptxstatus.bin"

# USB bmac dongle
43143eds clone 43143bmac \
    -console "10.176.8.87:40003" \
    -tag AARDVARK_BRANCH_6_30 \
    -dhd_tag AARDVARK_BRANCH_6_30 \
    -brand "linux-internal-wl" \
    -driver "debug-apdef-stadef-high" \
    -type "43143b0-bmac/g-assert/rtecdc.bin.trx" \
    -nvram "bcm943143usbirdsw_p450.txt" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1 } \
    -slowassoc 5 -nocal 1 \
    -perfchans {} -postinstall {} -modopts {} \
    -customer ""
#    -power_sta {} \

43143bmac clone 43143bmac_wlan-go \
    -driver "obj-debug-p2p-mchan-high" \
    -type "43143b0-bmac/g-assert-p2p-mchan/rtecdc.bin.trx"
 
43143bmac_wlan-go clone 43143bmac_p2p-go \
    -sta {43143bmac_p2p-go wl0.1 }

43143bmac clone 43143bmac_rel \
    -tag "AARDVARK_REL_6_30_256" \
    -dhd_tag "AARDVARK_REL_6_30_256"

43143bmac_rel clone 43143bmac_relmfgtest \
    -brand "linux-mfgtest-wl"\
    -driver "debug-apdef-stadef-high-mfgtest" \
    -type "43143b0-bmac/g-assert-mfgtest/rtecdc.bin.trx"

# USB full dongle
43143bmac clone 43143fmac \
    -wlinitcmds {} \
    -tag PHOENIX2_BRANCH_6_10 \
    -brand linux-internal-dongle-usb \
    -driver dhd-cdc-usb-gpl \
    -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus.bin.trx" \
    -perfchans {} -noaes 1 -notkip 1 \
    -customer "bcm" \
    -dhd_tag NIGHTLY \
    -yart {-attn2g 45-95 }
#    -brand linux-internal-dongle-usb \
#    -perfchans {} -noaes 1 -notkip 1 \

43143fmac clone 43143fmac_trunk \
    -brand linux-internal-dongle-usb \
    -tag NIGHTLY \
    -type "43143-ram/usb-g-assert.bin.trx"

43143fmac clone 43143fmac_mfgtest \
    -type "43143b0-roml/usb-g-mfgtest-seqcmds.bin.trx"

43143fmac clone 43143fmac_rel \
    -brand linux-external-dongle-usb \
    -dhd_tag DHD_REL_1_131_* \
    -tag PHOENIX2_REL_6_10_198_*

43143fmac_rel clone 43143fmac_relhp \
    -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-vsdb-pclose-swdiv-wfds-proptxstatus.bin.trx"

43143fmac_rel clone 43143fmac_relnodhcp
43143fmac_relnodhcp configure -ipaddr 192.168.1.208

43143fmac_rel clone 43143fmac_reltob \
    -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-tdls-proptxstatus.bin.trx" \
    -tag PHOENIX2_BRANCH_6_10

43143fmac_rel clone 43143fmac_relmfgtest \
    -dhd_brand linux-external-dongle-usb \
    -brand linux-mfgtest-dongle-usb \
    -type "43143b0-roml/usb-g-mfgtest-seqcmds.bin.trx"

43143fmac_relmfgtest clone 43143fmac_relmfgtestnodhcp
43143fmac_relnodhcp configure -ipaddr 192.168.1.76

43143fmac clone 43143fmac_wlan-go \
    -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-vsdb-pclose-swdiv-ccx-wfds-proptxstatus.bin.trx"
 
43143fmac_wlan-go clone 43143fmac_p2p-go \
    -sta {43143fmac_p2p-go wl0.1 }

43143fmac_rel clone 43143fmac_tdls \
    -brand linux-external-dongle-usb \
    -dhd_tag DHD_REL_1_94 \
    -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-tdls-proptxstatus.bin.trx"
43143fmac_tdls configure -tdlswith 43242fmac_tdls


##### Broadcom Router
# AP Section
# Netgear R6300
# 4706 AP with 4360 and 4331 cards
UTF::Router 4706 -sta {
    4706/4360 eth2
    4706/4331 eth1
} \
    -lan_ip 192.168.2.1 \
    -relay {lb-bun-87} \
    -lanpeer {aplan1} \
    -brand linux26-internal-router \
    -console "10.176.8.87:40000" \
    -power {apac_pwr 1} \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -nvram {
	# watchdog=3000 (default)
	lan_ipaddr=192.168.2.1
	dhcp_start=192.168.2.110
	dhcp_end=192.168.2.150
	wl_msglevel=0x101
	wl0_ssid=4706/4331
	wl0_channel=3
	wl0_bw_cap=-1
	wl0_radio=0
	wl0_obss_coex=0
	wl1_ssid=4706/4360
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
#4706/4360 configure -attngrp G1
#4706/4331 configure -attngrp G1
4706/4331 clone 4706/chanperf \
    -datarate {}
4706/4331 clone 4706/rel \
    -tag "AARDVARK01T_REL_6_37_14_44"

##### Broadcom Router
# AP Section
# BU router (BCM94708R8)
# 4706 AP with 43602 card
UTF::Router 4708 -sta {
    4708/43602mch5fd eth2
    4708/43602mch2fd eth1
} \
    -lan_ip 192.168.2.20 \
    -relay {lb-bun-87} \
    -lanpeer {aplan1} \
    -brand "linux-2.6.36-arm-up-external-vista-router-dhdap-full-src" \
    -console "10.176.8.87:40005" \
    -power {4708_pwr 1} \
    -tag "BISON04T_BRANCH_7_14" \
    -nvram {
	# watchdog=3000 (default)
	lan_ipaddr=192.168.2.20
	dhcp_start=192.168.2.151
	dhcp_end=192.168.2.170
	wl_msglevel=0x101
	wl0_ssid=bun3_43602mch2
	wl0_channel=3
	wl0_bw_cap=-1
	wl0_radio=0
	wl0_obss_coex=0
	wl1_ssid=bun3_43602mch5
	wl1_channel=36
	wl1_bw_cap=-1
	wl1_radio=0
	wl1_obss_coex=0
	#Only 1 AP can serve DHCP Addresses
	#router_disable=1
	et_msglevel=0; # WAR for PR#107305
	wl0_vht_features=3
#	wl0_frameburst=on
#	wl1_frameburst=on
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -yart {-frameburst 1 -attn5g "10-20+5 21-64+2 65-100+5" -attn2g "25-40+5 41-79+2 80-100+5" -pad 26} \
    -noradio_pwrsave 1
#	-perfchans {36/80}
#    -brand "linux-2.6.36-arm-internal-router-dhdap" \
#    -tag "BISON_REL_7_10_*" \
#    -power {apac_pwr 1} \
#	wl1_ssid=4706/4360
#	wl1_channel=36
#	wl1_bw_cap=-1
#	wl1_radio=0
#	wl1_obss_coex=0

4708/43602mch2fd clone 4708/43602mch2fd_mfg \
    -brand "linux-2.6.36-arm-mfgtest-router-dhdap-noramdisk"
4708/43602mch2fd_mfg clone  4708/43602mch2fd_mfg_dvt \
    -tag "BISON_REL_7_10_300_26"

4708/43602mch2fd_mfg clone  4708/43602mch2fd_mfg_netgear \
    -tag "BISON04T_REL_7_14_43_*"

4708/43602mch2fd_mfg clone  4708/43602mch2fd_mfg_relnew \
    -tag "BISON04T_REL_7_14_84"

4708/43602mch2fd clone 4708/43602mch2nic \
    -brand "linux-2.6.36-arm-internal-router"

4708/43602mch2nic clone 4708/43602mch2nic_rel \
    -tag "BISON_REL_7_10_212"

4708/43602mch2nic clone 4708/43602mch2nic_mfg \
    -brand "linux-2.6.36-arm-mfgtest-router-noramdisk"

4708/43602mch5fd clone 4708/43602mch5fd_mfg \
    -brand "linux-2.6.36-arm-mfgtest-router-dhdap-noramdisk"

4708/43602mch5fd clone 4708/43602mch5nic \
    -brand "linux-2.6.36-arm-internal-router"

4708/43602mch5nic clone 4708/43602mch5nic_mfg \
    -brand "linux-2.6.36-arm-mfgtest-router-noramdisk"

#Scheduler for rack using controller
UTF::Q bun03 lb-bun-87
