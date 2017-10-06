#testbed configuration file for blr16end1
#Created on 22-7-2015
#Last checkin 
####### Controller section:
# blr16end1: FC19
#
#
####### SOFTAP section:

# AP1:4360c
# AP2:4360d
####### STA section:
# blr16tst1:43458 eth0(10.131.80.194)
# blr16tst4: 43458c0 eth0 (10.131.80.197) ---kept in DUT2
# blr03tst2:  eth0 (10.)
# blr03tst3: eth0 (10.)
# blr16ref0: 4360c (10.132.30.191)
# blr16ref1: 4360d (10.131.80.192)  
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix


UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr16end1" \
        -group {
                G1 {1 2 3 4}
                G2 {5 6 7 8}
                G3 { 9}
		ALL {1 2 3 4 5 6 7 8 9}
                }
ALL configure -default 30
#G2 configure default 0
#G3 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn default
    
}


# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr16"

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

UTF::Linux blr16end1 \
     -lan_ip 10.132.116.110 \
    -sta {lan eth0}

UTF::Linux blr16ref0 -sta {4360c eth1} \
    -lan_ip 10.132.116.111 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag "BISON_BRANCH_7_10" \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}  {%S wl scansuppress 1}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} } \
    -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth1; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
         wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
    }

4360c configure -ipaddr 192.168.1.91 -ap 1 -attngrp G1 \

UTF::Linux blr16ref1 -sta {4360d enp1s0} \
    -lan_ip 10.132.116.112 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag "BISON_BRANCH_7_10" \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}  {%S wl scansuppress 1}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} } \
    -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth1; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
         wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
    }

4360d configure -ipaddr 192.168.1.92 -ap 1 -attngrp G2 \



UTF::DHD blr16tst1 \
        -lan_ip 10.132.116.113 \
        -sta {43458b0 eth0} \
        -tcpwindow 4M \
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 0 \
		-app_tag trunk \
		-dhd_tag DHD_REL_1_363_42 \
		-nvram bcm943458fcpagb.txt \
		-dhd_brand linux-external-dongle-pcie \
		-driver dhd-msgbuf-pciefd-debug \
		-brand linux-external-dongle-pcie \
	    -type 43458c0-roml/43455_pcie-43455_ftrs-pno-aoe-pktfilter-sr-pktctx-lpc-pwropt-wapi-mfp-clm_4335_ss-txpwr-rcc-fmc-wepso-noccxaka-sarctrl-proxd-gscan-linkstat-pwrstats-idsup-ndoe-pwrofs-hs20sta-nan-xorcsum-amsdutx-swdiv.bin  \
        -tag "BISON06T_BRANCH_7_45" \
	    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
		-wlinitcmds {wl down; wl chanspec 36} \

43458b0 configure -ipaddr 192.168.1.93 \



	
UTF::DHD blr16tst4 \
         -lan_ip 10.132.116.116 \
         -sta {43458c0 eth0} \
         -tcpwindow 4M \
         -power_button "auto" \
         -slowassoc 5 -reloadoncrash 1 \
         -nobighammer 0 \
		 -app_tag trunk \
		 -nvram bcm943458fcpagb.txt \
		 -dhd_tag DHD_REL_1_363_42 \
		 -dhd_brand linux-external-dongle-sdio \
		 -brand linux-external-dongle-sdio \
		 -modopts {sd_uhsimode=3} \
		 -type 43458c0-roml/43455_sdio-43455_ftrs-pno-aoe-pktfilter-sr-pktctx-lpc-pwropt-wapi-mfp-clm_4335_ss-txpwr-rcc-fmc-wepso-noccxaka-sarctrl-proxd-gscan-linkstat-pwrstats-idsup-ndoe-pwrofs-hs20sta-nan-amsdutx.bin \
         -tag "BISON06T_BRANCH_7_45" \
         -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan; wl chanspec 36} \
         -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
         -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
		 -udp 800m \


43458c0 configure -ipaddr 192.168.1.94 \

43458c0 clone 43458d0 -brand linux-mfgtest-dongle-sdio \
        -type 43458c0-roml/sdio-ag-mfgtest-seqcmds-swdiv.bin \

UTF::DHD blr16tst2 \
         -lan_ip 10.132.116.114 \
         -sta {43430b0 eth0} \
         -tcpwindow 4M \
         -power_button "auto" \
         -slowassoc 5 -reloadoncrash 1 \
         -nobighammer 1 \
		 -app_tag trunk \
		 -nvram bcm943430fsdng_Bx.txt \
		 -dhd_tag DHD_REL_1_363_42 \
		 -dhd_brand linux-external-dongle-sdio \
		 -brand hndrte-dongle-wl \
		 -modopts {sd_divisor=1 ; sd_uhsimode=1} \
		 -type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-assert/rtecdc.bin \
         -tag "DINGO07T_BRANCH_9_35" \
         -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan} \
         -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
         -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
		 -udp 800m \
		 


43430b0 configure -ipaddr 192.168.1.95 \

43430b0 clone 43430d0 -brand hndrte-dongle-wl \
        -tag "DINGO_TWIG_9_10_160" \
		-type 43436b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-assert/rtecdc.bin \

		
UTF::Linux blr16tst3 \
        -lan_ip 10.132.116.115 \
        -sta {4331 eth1} \
        -tcpwindow 4M \
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "BISON05T_BRANCH_7_35" \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
		-udp 800m \

4331 configure -ipaddr 192.168.1.96 \

4331 clone 4331t  -tag trunk \

4331 clone 4331b -tag BISON05T_BRANCH_7_35 \

4331 clone 4331BIS -tag BISON04T_BRANCH_7_14 \
	-brand linux-mfgtest-wl \

	



UTF::Q blr01n
