# -*-tcl-*-
#
# Pelican UTF Test Configuration
#
# UTF configuration for Pelican testbed @ Broadcom
#

# load packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Power
package require UTF::DHD

UTF::Logfile "pelicantr.log"
#set ::UTF::SummaryDir "/projects/hnd_software_nl/sig/$::env(LOGNAME)/bun01"
#set ::UTF::SummaryDir "/home/$::env(LOGNAME)/UTF/bun01"
set ::UTF::SummaryDir "/projects/hnd_software_ext3/$::env(LOGNAME)/UTF/pelicantr"
#set ::UTF::SummaryDir "/home/olgac/UTFlogs"

# this is needed for multiple STA to multiple AP tests
#set ::sta_list "43241b0 43242fmac 4334b1 43143fmac"
#set ::ap_list "4717_1 4717_3"

# Power controllers
UTF::Power::Synaccess ps-pelicansta \
     -lan_ip 10.191.8.122 \
     -rev 1

UTF::Power::Synaccess ps-pelicanap \
     -lan_ip 10.191.8.125 \
     -rev 1 

# Attenuator - Aeroflex
UTF::Aeroflex pelicanaf -lan_ip 10.191.8.123 \
     -group {G1 {1 2 3} ALL {1 2 3}}
#    -group {G1 {1 2 3} ALL {1 2 3 4 5 6 7 8 9}}

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn 0;
    G1 attn 60;
    #G1 attn 40;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    # AP1 restart wl0_radio=0

    # Disable USB devices
    # 43242bmac power_sta cycle
    # 43143bmac power_sta cycle

    # delete myself
    unset ::UTF::SetupTestBed

    return
}

# Set testrig specific attenuation ranges for use with RvRNightly1.test
set ::cycle2G20AttnRange "46-110 110-46"
set ::cycle5G20AttnRange "25-90 90-25"
set ::cycle5G80AttnRange "25-90 90-25"

set ::rvr_overall_timeout_min -1

##### Relay / Consolelogger
##### Running FC11
UTF::Linux pelican \
    -sta {aplan1 p1p1} \
    -lan_ip 10.191.8.120 
#    -user root
aplan1 configure -ipaddr 192.168.1.50 

# aplan1 clone aplan2 \
#    -sta {aplan2 p6p1}
# aplan2 configure -ipaddr 192.168.2.50


##### DUT Host Machine

#######################

UTF::DHD pelicansta \
    -sta {4335f15 eth1} \
    -power {ps-pelicansta 1} \
    -lan_ip "10.191.8.121" \
    -noaes 1 \
    -notkip 1 \
    -user root \
    -tag AARDVARK_BRANCH_6_30 \
    -nvram "bcm94339wlipa.txt" \
    -brand "linux-mfgtest-dongle-sdio" \
    -dhd_brand "linux-internal-dongle" \
    -type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn" \
    -dhd_tag NIGHTLY \
    -console "10.191.8.120:40000" \
    -modopts {sd_uhsimode=3} \
    -postinstall {dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 10} \
    -nomaxmem 1 \
    -tcpwindow 1152k \
    -datarate {-i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x1-9x3}
#    -udp 400m

#    -noaes 1 \
#    -notkip 1 \

#    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1 }

#    -perfchans { 3 5 } \
#    -modopts {sd_uhsimode=3} \
#    -modopts { sd_uhsimode=2 sd_tuning_period=10 } \
#    -type "4335b0-ram/sdio-ag-mfgtest-seqcmds-assert-err" \   
#    -datarate {-skiptx "9-23 0x2-8x3" -skiprx "9-23 0x2-8x3"}\
#    -post_perf_hook {{%S wl dump ampdu} {%S wl rssi} {%S wl nrate}}\
#    -pre_perf_hook {{%S wl ampdu_clear_dump}} 
#    -udp 450m \
#    -dhd_brand "linux-internal-dongle" \
#    -modopts { sd_uhsimode=2 sd_tuning_period=10 } \
#    -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl bw_cap 2g -1; wl btc_mode 1} \

4335f15 configure -attngrp G1
# -ipaddr 192.168.1.1

4335f15 clone 4335f15PHYDbg \
	 -type "4335c0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn-assert-err-phydbg-dbgam-dump-ipa"

4335f15 clone 4335f15FEM \
	-dhd_tag DHD_REL_1_101 \
	-type "4335b0-ram/sdio-ag-mfgtest-seqcmds-assert-err" \
	-nvram "bcm94335wlbgaFEM_AA.txt"
	
4335f15 clone 4335f15Cal \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn-assert-err-phydbg-dbgam-dump-ipa"
	
4335f15 clone 4335f15ipa \
	-dhd_tag DHD_REL_1_99 \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn-assert-err-phydbg-dbgam-dump-ipa"	 

4335f15 clone 4335f15SDIO \
	-dhd_tag DHD_REL_1_100
	
4335f15 clone 4335iPA \
	-dhd_tag DHD_REL_1_102 \
	-date 2013.5.5 \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn-assert-err-phydbg-dbgam-dump-ipa" \
	-udp 450m
	
4335f15 clone 4335iPA_NIGHTLY \
	-date 2013.5.5
	
4335f15 clone 4335FEM_NIGHTLY \
	-nvram "bcm94335wlbgaFEM_AA.txt" \
	-type "4335b0-ram/sdio-ag-mfgtest-seqcmds-assert-err" \
	-date 2013.5.5
	
4335f15 clone 4335_NIGHTLY \
	-tag AARDVARK_REL_6_30_263 \
	-nvram "bcm94335wlbgaFEM_AA.txt" \
	-type "4335b0-ram/sdio-ag-mfgtest-seqcmds-assert-err"
	
4335f15 clone 4339 \
	-tag AARDVARK01T_BRANCH_6_37 \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn-assert-err-phydbg-dbgam-dump-ipa"
	
# -date 2013.6.8
	

4335f15 clone 4339db \
	-image "/home/olgac/UTF/drv/rtecdc.bin"


4335f15 clone 4339p202 \
	-tag AARDVARK01T_BRANCH_6_37 \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn-assert-err-phydbg-dbgam-dump-ipa"
	
	
4335f15 clone 4339sdio3 \
	-tag AARDVARK01T_BRANCH_6_37 \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn-assert-err-phydbg-dbgam-dump-ipa" \
	-modopts {sd_uhsimode=3}
	
	
4335f15 clone 4339p203ipa \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-assert-err-phydbg-dbgam-dump-ipa"

4335f15 clone 4339p203ipaTest \
	-brand "linux-external-dongle-sdio" \
	-type "4339a0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-ampduhostreorder-ve-mfp-okc-txbf-relmcast-pktctx-dmatxrc-aibss-ltecx-idsup-idauth"

4335f15 clone 4339p203ipa906 \
	-image "/home/olgac/UTF/drv/rtecdc_423906.bin"
	
#	-date 2013.7.21.0
#	-tag AARDVARK01T_BRANCH_6_37 \

4339p203ipa clone 4339_HW_PAPD-Mode0

4339_HW_PAPD-Mode0 clone 4339_Analytic_PAPD-Mode2_wYrefInit \
	-image "/home/olgac/UTF/drv/rtecdc.bin"

4335f15 clone 4339ePA \
	-nvram "bcm94335wlbgaFEM_AA.txt" \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr"
	
#	-tag AARDVARK01T_BRANCH_6_37 \
#	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn" \
	
# test fo Daniel #
	
4335f15 clone 4335b0fem \
	-nvram "bcm94335wlbgaFEM_AA.txt" \
	-image "/projects/hnd_swbuild/build_linux/USERS/arishoni_trunk_/NIGHTLY/linux-external-dongle-sdio/2013.8.1.0/release/bcm/firmware/4335b0-ram/sdio-ag-p2p-proptxstatus-dmatxrc-pktctx-assert-err.bin" \
	-dhd_brand "linux-internal-dongle"

# for Mor #
	
4335f15 clone 4339epa \
	-nvram "bcm94339wlbgaFEM_AM.txt" \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn" \
	-dhd_brand "linux-external-dongle-sdio"
	
4335f15 clone 4339ipaTab \
	-image "/home/olgac/UTF/drv/sdio-ag-mfgtest-seqcmds-sr-assert-err-phydbg-dbgam-dump-idsup-idauth-ipa.bin"	
				
		
#	-tag AARDVARK01T_BRANCH_6_37 \	
#	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn-assert-err-phydbg-dbgam-dump-ipa"
#       -type "4335b0-ram/sdio-ag-p2p-proptxstatus-dmatxrc-pktctx-assert-err"\	
#	-brand "linux-external-dongle-sdio" \
#	-tag AARDVARK_REL_6_30_263
#	-nvram "bcm94335wlbgaFEM_AA.txt" \
#	-type "4335b0-ram/sdio-ag-mfgtest-seqcmds-assert-err" \
#	-image "/home/olgac/UTF/drv/rtecdc.bin" \
#	-date 2013.7.16.3 \
#	-dhd_date 2013.7.17.2		 
#-type "4335b0-ram/sdio-ag-mfgtest-seqcmds-assert-err" \
#-type "4335c0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn" \
#-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn" \	 

################################################

UTF::DHD pelicansta2 \
    -sta {4354f15 eth1} \
    -power {ps-pelicansta 1} \
    -lan_ip "10.191.8.121" \
    -noaes 1 \
    -notkip 1 \
    -user root \
    -nvram "bcm94354wlsagbi.txt" \
    -tag BISON_REL_7_10_50_8 \
    -dhd_brand "linux-mfgtest-dongle-sdio" \
    -dhd_tag DHD_REL_1_137 \
    -console "10.191.8.120:40000" \
    -nomaxmem 1 \
    -tcpwindow 2m \
    -datarate {-i 0.5 -frameburst 1} \
    -pre_perf_hook {{%S wl counters} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -post_perf_hook {{%S wl counters} {%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl dump rssi}}
    
    


#    -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
#    -udp 1.2g
#    -tag BISON_TWIG_7_10_50 \
#    -brand "linux-mfgtest-dongle-sdio" \
#    -type "4354a0-ram/sdio-ag-mfgtest-assert-err" \
#    -datarate {-b 1.2g -i 0.5 -frameburst 1} \

4354f15 configure -attngrp G1


4354f15 clone 4354_TWIG \
    -postinstall {dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 40} \
    -image "/home/olgac/UTF/drv/4354a0_8_10_13/rtecdc.bin" \
    -wlinitcmds {
    		 wl down; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
		 wl mimo_bw_cap 1; wl frameburst 1; wl ampdu_mpdu 16; wl pool 100; wl vht_features 3; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl up}
#	-perfchans { 2 }

4354f15 clone 4350c0 \
     -postinstall {dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 40} \
     -nvram "bcm94350wlagbe_KA.txt" \
     -tag  BISON_BRANCH_7_10 \
     -brand "linux-mfgtest-dongle-sdio" \
     -type "4350c0-ram/sdio-ag-mfgtest-seqcmds-assert-err" \
     -wlinitcmds {
    		 wl down; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
		 wl mimo_bw_cap 1; wl frameburst 1; wl ampdu_mpdu 16; wl pool 100; wl vht_features 3; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl up}
	
4350c0 clone 4350c0_udp \
	-udp 1.2g
	
4350c0 clone 4350c0_roml \
	-image "/home/olgac/UTF/drv/4350c0epa_8_10_13/rtecdc.bin" \
	-perfchans { 36l }
	
4350c0 clone 4350c0_twig \
	-tag BISON_TWIG_7_10_82
	
#	-image "/home/olgac/UTF/drv/4350c0epa_8_10_13/rtecdc.bin"

4354f15 clone 4354_ext \
    -postinstall {dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 40} \
	-tag  BISON_REL_7_10_50_11 \
	-brand "linux-external-dongle-sdio" \
        -type "4354a0-ram/sdio-ag-pktctx-dmatxrc-txbf-dbgtput-dbgam-phydbg" \
	-wlinitcmds {
    		 wl down; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
		 wl mimo_bw_cap 1; wl frameburst 1; wl ampdu_mpdu 16; wl pool 100; wl vht_features 3; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl up}
	
#	-image "/home/olgac/UTF/drv/BISON_BRANCH_7_10/rtecdc.bin"

4354_ext clone 4354_p2p \
	-tag BISON_TWIG_7_10_82 \
	-type "4354a0-ram/sdio-ag-p2p-pno-aoe-sr-mchan-proptxstatus-ampduhostreorder"
	
4354f15 clone 4354_tob \
    -postinstall {dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 40} \
	-tag  BISON_BRANCH_7_10 \
	-brand "linux-mfgtest-dongle-sdio" \
        -type "4354a0-ram/sdio-ag-mfgtest-seqcmds-sr-assert-err" \
	-wlinitcmds {
    		 wl down; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
		 wl mimo_bw_cap 1; wl frameburst 1; wl ampdu_mpdu 16; wl pool 100; wl vht_features 3; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl up}

#	-image "/home/olgac/UTF/drv/TOB_7_10__r433352/rtecdc.bin"
#	-perfchans { 36/80 } \

4354_tob clone 4354_tob_udp \
	-udp 1.2g
	
4354_tob clone 4354_tob_l \
	-nvram "bcm94354wlsagbl.txt"
	
4354_tob_l clone 4354_tob_udp_l \
	-udp 1.2g

4354_tob clone 4354_twig \
	-tag BISON_TWIG_7_10_82 \
	-brand "linux-mfgtest-dongle-sdio" \
	-type "4354a0-ram/sdio-ag-mfgtest-seqcmds-assert-err"
	
4354_twig clone 4354_twig_udp \
	-udp 1.2g
	
4354_twig clone 4354_twig_rev \
	-image "/home/olgac/UTF/drv/BISON_TWIG_7_10_82__r438472/rtecdc.bin" \
	-udp 1.2g
	
4354_twig clone 4354_twig_l \
	-nvram "bcm94354wlsagbl.txt"
#	-perfchans { 2 3 4 5 6 13 }
	
4354_twig_l clone 4354_twig_udp_l \
	-udp 1.2g
	
4354_twig clone 4354_twig_cr \
	-perfchans { 36l } \
	-image "/home/olgac/UTF/drv/BISON_TWIG_7_10_82__r436966/rtecdc.bin"
#	-udp 1.2g

4354_twig_l clone 4354_twig_l_rev \
	-image "/home/olgac/UTF/drv/BISON_TWIG_7_10_82__r438472_n/rtecdc.bin" \
	-perfchans { 36l }

########################

#    4354a1  

4354f15 clone 4354a1 \
    -postinstall {dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 40} \
	-nvram "bcm94354wlsagbl.txt" \
	-dhd_brand "linux-mfgtest-dongle-sdio" \
	-dhd_tag DHD_REL_1_141_11 \
	-brand "linux-mfgtest-dongle-sdio" \
	-type "4354a1-roml/sdio-ag-mfgtest-seqcmds-sr-assert" \
	-wlinitcmds {
    		 wl down; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
		 wl mimo_bw_cap 1; wl frameburst 1; wl ampdu_mpdu 16; wl pool 100; wl vht_features 3; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl up}
	
4354a1 clone 4354a1_tob \
	-tag  BISON_BRANCH_7_10

#### Tamar #####
	
4354a1 clone 4354a1_ltob \
	-tag  BISON_BRANCH_7_10 \
	-modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10}
	
		
4354a1 clone 4354a1_lrel \
	-tag BISON_REL_7_10_82_35 \
	-modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10}
	

4354a1_tob clone 4354a1_tob25 \
	-image "/home/olgac/UTF/drv/TOB_7_10__r459364/rtecdc.bin" \
	-modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10}

##################
	
4354a1_tob clone 4354a1_tob_003 \
	-image "/home/olgac/UTF/drv/TOB_7_10__r454003/rtecdc.bin"
	
4354a1_tob clone 4354a1_tob_999 \
	-image "/home/olgac/UTF/drv/TOB_7_10__r453999/rtecdc.bin"
	
###################

4354a1_tob clone 4354a1_tob_papd \
        -image "/home/olgac/UTF/drv/BISON_BRANCH_7_10_Hezi/rtecdc.bin"


4354a1_tob clone 4354a1_tob_udp \
	-udp 1.2g


4354a1 clone 4354a1_twig \
	-tag BISON_TWIG_7_10_82

4354a1_twig clone 4354a1_twig_test \
	-image "/home/olgac/UTF/drv/TWIG_7_10_82__r456255/rtecdc.bin"

# jira
	
4354a1_twig clone 4354a1_twig_jira \
	-tag BISON_REL_7_10_82_37 \
	-dhd_brand "linux-external-dongle-sdio" \
	-dhd_tag DHD_REL_1_141_37 \
	-perfchans { 6u } \
	-brand "linux-external-dongle-sdio" \
	-type "4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-assert-err"
	
4354a1_twig clone 4354a1_twig_udp \
	-udp 1.2g
	
	
4354a1 clone 4354a1_twig_cal \
	-tag BISON_TWIG_7_10_82 \
	-pre_perf_hook {{%S wl counters} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_forcecal 1}}
	
	
4354a1 clone 4354a1_tob_r_udp \
	-tag  BISON_BRANCH_7_10 \
	-image "/home/olgac/UTF/drv/TOB_7_10__r445197/rtecdc.bin" \
	-udp 1.2g
	
4354a1 clone 4354a1_test \
	-tag  BIS82RC22_BRANCH_7_21
	
		
#	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_watchdog 0} {%S wl phy_percal 0}} \
#       -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl dump rssi} {%S wl phy_watchdog 1} {%S wl phy_percal 2}}	
#	-nvram "/home/olgac/UTF/drv/BISON_BRANCH_7_10/bcm94354wlsagbi.txt" \
#	-image "/home/olgac/UTF/drv/BISON_TWIG_7_10_82__27_10_13/rtecdc.bin"



##########################
###    4354 PCIe


4354f15 clone 4354PCIe \
	-nvram "bcm94354wlpagbl.txt" \
	-tag BISON_BRANCH_7_10 \
	-brand "linux-mfgtest-dongle-pcie" \
	-type "4354a1-roml/pcie-ag-msgbuf-pktctx-splitrx-txbf-mfgtest-seqcmds-sr-assert" \
	-dhd_tag NIGHTLY \
	-dhd_brand "linux-external-dongle-pcie" \
	-driver "dhd-msgbuf-pciefd-debug" \
	-wlinitcmds {wl down; down +scan; wl mpc 0; wl btc_mode 0; wl vht_features 3; wl up} \
	-perfchans { 1 2 5 6 11 }

#	-slowassoc 5 \
#	-dhd_tag DHD_REL_1_150 \
#	-brand "linux-external-dongle-pcie" \
#	-type "4354a1-roml/pcie-ag-msgbuf-pktctx-splitrx-sr-txbf-assert-err" \
# fyang      -type "4354a1-roml/pcie-ag-msgbuf-splitrx-txbf-splitbuf-mfgtest-seqcmds-sr-assert-err" \

4354PCIe clone 4354pext \
	-brand "linux-external-dongle-pcie"
	
 
4354PCIe clone 4354p_rel_th \
	-tag BISON_REL_7_10_221_1 \
	-type "4354a1-roml/pcie-ag-msgbuf-pktctx-splitrx-txbf-mfgtest-seqcmds-sr-assert-err" \
	-tcpwindow 8m \
	-wlinitcmds {wl down; wl msglevel +scan; wl frameburst 1; wl ampdu 1; wl mpc 0; wl btc_mode 0; wl vht_features 3; wl up}
	
4354PCIe clone 4354p_tob_th \
	-tag BISON_BRANCH_7_10 \
	-type "4354a1-roml/pcie-ag-msgbuf-pktctx-splitrx-txbf-mfgtest-seqcmds-sr" \
	-tcpwindow 8m \
	-wlinitcmds {wl down; wl msglevel +scan; wl frameburst 1; wl ampdu 1; wl mpc 0; wl btc_mode 0; wl vht_features 3; wl up}


##########################
###    43430


4354f15 clone 43430 \
	-nvram "bcm943430wlselgs_26MHz.txt" \
	-brand "linux-mfgtest-dongle-sdio" \
	-type "43430a0-roml/sdio-g-mfgtest-seqcmds-err-assert" \
	-dhd_tag DHD_REL_1_141_64 \
	-dhd_brand "linux-mfgtest-dongle-sdio" \
	-driver "dhd-cdc-sdstd-debug" \
	-wlinitcmds {wl down; wl msglevel +scan; wl up}
	
43430 clone 43430rel \
	-tag BISON_REL_7_10_226_23 \
	-perfchans { 1 2 5 6 11 }
	
43430rel clone 43430_dif \
	-dhd_tag DHD_REL_1_174
	
43430 clone 43430e \
	-tag BISON_REL_7_10_226_51 \
	-brand "linux-external-dongle-sdio" \
	-type "43430a0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo.bin" \
	-dhd_brand "linux-external-dongle-sdio" \
	-perfchans { 1 2 3 5 6 11 }

43430e clone 43430b \
	-tag BISON_BRANCH_7_10
	
43430e clone 43430test \
	-tag BISON_TWIG_7_10_226 \
	-image "/home/olgac/UTF/drv/TWIG_7_10_226__r483588/rtecdc.bin"
	
43430 clone 43430t \
	-tag BISON_TWIG_7_10_226 \
	-brand "linux-external-dongle-sdio" \
	-type "43430a0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo.bin" \
	-dhd_brand "linux-external-dongle-sdio"	

43430 clone 43430tw \
	-tag BISON_TWIG_7_10_226 \
	-brand "linux-mfgtest-dongle-sdio" \
	-type "43430a0-roml/sdio-g-mfgtest-seqcmds.bin" \
	-dhd_brand "linux-external-dongle-sdio"
	
43430e clone 43430ext_22 \
	-tag BISON_REL_7_10_226_22
	
43430e clone 43430ext_23 \
	-tag BISON_REL_7_10_226_23
	
43430e clone 43430e_tr_B \
	-dhd_tag trunk \
	-dhd_image "/home/olgac/UTF/drv/BorisGCC482_test/dhd-cdc-sdstd-debug-2.6.38.6-26.rc1.fc15.i686.PAE"
	
43430 clone 43430_ext \
	-image "/home/olgac/UTF/drv/PropTxTest/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-sr-vsdb-lpc-wl11u-autoabn-txbf-rcc-fmc-wepso-ccx-okc/rtecdc.bin"
	
43430 clone 43430_mfg \
	-image "/home/olgac/UTF/drv/PropTxTest/sdio-g-mfgtest-seqcmds-proptxstatus/rtecdc.bin"
	
43430 clone 43430_m \
	-image "/home/olgac/UTF/drv/PropTxTest/sdio-g-mfgtest-seqcmds/rtecdc.bin"
	
43430rel clone 43430j255 \
	-perfchans { 11 }
	
# wl vht_features 3;


UTF::Linux pelicansta3 \
    -power {ps-pelicansta 1} \
    -lan_ip "10.191.8.121" \
    -sta {43228 eth1} \
    -console "10.191.8.120:40000" \
    -tag trunk \
    -image "/projects/hnd_swbuild_ext11_scratch/build_linux_ext11_scratch/NIGHTLY/linux-external-wl/2014.4.27.0/release/obj-nodebug-apdef-stadef-2.6.38.6-26.rc1.fc15.i686.PAE" \
    -nomaxmem 1 \
    -tcpwindow 2m
    
43228 configure -attngrp G1
    
43228 clone 43228tr_gcc482 \
	-image "/home/olgac/UTF/drv/BorisGCC482_test/wl-debug-apdef-stadef-2.6.38.6-26.rc1.fc15.i686.PAE"  

#    -nvram "bcm94354wlpagbl.txt" \

43228 clone 43228ext \
	-image "/home/olgac/UTF/drv/Boris_extended_warnings_test/debug-apdef-stadef-2.6.38.6-26.rc1.fc15.i686.PAE"  

	
##########################	
###   4360sta

if 0 {
UTF::Linux pelicansta \
    -power {ps-pelicansta 1} \
    -lan_ip "10.191.8.121" \
    -sta {4360sta eth1} \
    -console "10.191.8.120:40001" \4354PCIe load4354PCIe load


    -tag AARDVARK_BRANCH_6_30 \
    -nomaxmem 1

4360sta configure -attngrp G1 -ipaddr 192.168.1.1
}

#   -brand "linux-internal-wl" \
#    -type "obj-debug-apdef-stadef-2.6.38.6-26.rc1.fc15.i686.PAE" \


##### Broadcom Router
##### BUPC SAP Intel DG41TY motherboard f15.

UTF::Linux pelicanap \
    -power {ps-pelicanap 1} \
    -lan_ip "10.191.8.124" \
    -sta {4360f15 eth1} \
    -console "10.191.8.120:40001" \
    -tag AARDVARK_BRANCH_6_30 \
    -tcpwindow 2m \
    -nomaxmem 1
#    -wlinitcmds {
#        ifdown eth1; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; wl nmode 0\
#       service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;
#        }
#    -udp 1.2g
#-image "/projects/hnd/swbuild/build_linux/PRESERVED/AARDVARK_BRANCH_6_30/linux-internal-wl/2013.8.29.0/release/obj-debug-apdef-stadef-2.6.38.6-26.rc1.fc15.i686.PAE/wl.ko.gz" \ 
#          
#   -tag AARDVARK_BRANCH_6_30 \ 
#   -tag AARDVARK_REL_6_30_223_203 \ 
#   -wlinitcmds {ifconfig eth1 down; wl vht_features 3; ifconfig eth1 hw ether 00:10:18:AF:52:D3 up; wl up} \    
#   -wlinitcmds {ifconfig eth1 down;ifconfig eth1 hw ether 00:10:18:AF:52:D3 up; wl bssid} \
#   -relay {pelicanap} \
#   -image "/projects/hnd/swbuild/build_linux/AARDVARK_REL_6_30_118_30/linux-internal-wl/2012.10.19.0/release/obj-debug-apdef-stadef-2.6.38.6-26.rc1.fc15.i686.PAE/wl.ko"
#   -tag "AARDVARK_REL_6_30_118_30" \
#   -brand "linux-internal-wl" \
#   -console "10.176.8.85:40000" 
#   -lanpeer {aplan1} \
#   -post_perf_hook {%S wl pktq_stats} \

4360f15 configure -attngrp G1 -ap 1 -hasdhcpd 1 -ipaddr 192.168.1.30 -wlinitcmds { wl down; wl -u txbf_bfe_cap 1 ; wl -u txbf_bfr_cap 1; wl -u txbf 1 ; wl up }

# wl nmode 0;
#   4360f15 configure -attngrp G1 -ap 1 -hasdhcpd 1 -ipaddr 192.168.1.30

