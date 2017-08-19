# -*-tcl-*-
#
# Testbed MC86 sub-rig configuration file
# Filename: mc86b.tcl
# Charles Chai 01/11/2013
#
# Hardware
#   MC86END1  : FC11 controller   
#   MC86TST8  : FC19 softap
#   MC86TST2  : FC15 STA
#   MC86TST4  : FC19 STA
#   mc86tst5  : FC15 STA
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#
source "utfconf/mc86.tcl"

package require UTF::Linux
package require UTF::utils

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc86b"

# Set default to use wl from trunk; Use -app_tag to modify.
#set UTF::TrunkApps 1

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G2 attn 0

    # Make sure all systems are deinit and down 
    # just in case debugging left them loaded
    foreach S {4360ap 4330 43342 4358 4345b1} {
        UTF::Try "$S Down" {
       	    catch {$S wl down}
            catch {$S deinit}
        }
    }
    # unset S so it doesn't interfere
    unset S

    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}


###########################################
# MC86B
# 4330wlsdagb_3.2 SDIO - 11n 1x1 
###########################################
UTF::DHD mc86tst2 \
        -lan_ip mc86tst2 \
        -sta {4330 eth0} \
	-console mc86end1:40009 \
        -hostconsole mc86end1:40004 \
        -power {npcdut2a 1} \
        -power_button auto \
        -tag FALCON_{TWIG,REL}_5_90_195{,_*} \
        -dhd_tag trunk \
        -brand linux-internal-dongle \
        -dhd_brand linux-internal-dongle \
        -nvram "bcm94330wlsdagb_P300.txt" \
        -type "4330b2-roml/sdio-ag-ccx-btamp-p2p-idsup-idauth-pno" \
        -postinstall {dhd -i eth0 serial 1; dhd -i eth0 sd_divisor 1} \
        -perfchans {36 3} \
        -nocal 1 -nomaxmem 1 -nobighammer 1 -noframeburst 1 \
        -udp 60m -slowassoc 5

4330 clone 4330htagc0 -nvram_add {htagc=0}
4330 clone 4330htagc1 -nvram_add {htagc=1}

4330 clone 4330test -brand linux-mfgtest-dongle-sdio -type "4330b2-roml/sdio-ag-mfgtest.bin" 


###########################################
# MC86B
# 4358wlspie_P105 A1 PCIe 11ac 2x2 (a variation of 43569)
# (Obtained from Maneesh Mishra, 6/2/2014)
###########################################
UTF::DHD mc86tst4 \
        -lan_ip mc86tst4 \
        -sta {4358 eth0} \
        -hostconsole "mc86end1:40008" \
        -power {npcdut2b 1} \
	-power_button "auto" \
	-app_tag trunk \
	-dhd_tag DHD_BRANCH_1_201 \
        -dhd_brand linux-internal-dongle-pcie \
        -brand linux-external-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -nvram bcm94358wlspie_p105.txt \
        -slowassoc 5 \
        -datarate {-i 0.5 -frameburst 1} \
        -tcpwindow 2m -udp 800m \
        -nocal 1 -nopm1 1 -nopm2 1 \
	-post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3;}

4358 clone 4358b35 \
	-tag BISON05T_BRANCH_7_35 \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert/rtecdc.bin

4358b35 clone 4358b35x \
	-perfonly 1 -perfchans {36/80} \
	-notkip 1 -noaes 1 -nobighammer 1 \
	-type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-sstput/rtecdc.bin

4358 clone 4358b35t \
	-tag BISON05T_TWIG_7_35_105 \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx5g-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert/rtecdc.bin

4358b35t clone 4358b35tx \
	-perfonly 1 -perfchans {36/80} \
	-notkip 1 -noaes 1 -nobighammer 1 \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin

4358 clone 4358b.1 \
	-tag BISON_BRANCH_7_10 \
	-notkip 1 -noaes 1 -nobighammer 1 \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-amsdutx-wl11u-mfp-tdls-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-assert/rtecdc.bin

4358 clone 4358b.2 \
        -tag BISON_BRANCH_7_10 \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-amsdutx-wl11u-mfp-tdls-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert/rtecdc.bin

# short of memory
4358 clone 4358b.3 \
        -tag BISON_BRANCH_7_10 \
	-perfchans {36/80} \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-amsdutx-wl11u-mfp-tdls-ltecx-wfds-okc-ccx-ve-idsup-idauth-proxd-nan-shif-tbow-assert/rtecdc.bin

4358 clone 4358bx \
        -tag BISON_BRANCH_7_10 \
	-perfonly 1 -perfchans {36/80} \
	-notkip 1 -noaes 1 -nobighammer 1 \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin
	#-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-amsdutx-wl11u-mfp-tdls-ltecx-wfds-mchandump/rtecdc.bin

4358 clone 4358r112 \
	-tag BISNC105RC66_BRANCH_7_112 \
	-app_tag DHDNC39RC65_BRANCH_1_47 \
	-dhd_tag DHDNC39RC65_BRANCH_1_47 \
	-notkip 1 -noaes 1 \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx5g-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-btcdyn-wepso-rcc-noccxaka-sarctrl-xorcsum-assert.bin

4358r112 clone 4358r112x \
	-perfonly 1 -perfchans {36/80} \
	-type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-xorcsum.bin


###########################################
# MC86B
# 43342fcagb_P224 SDIO 1x1 11n (43342 is very similar to 4334)
###########################################
UTF::DHD mc86tst5 \
        -lan_ip mc86tst5 \
        -sta {43342 eth0} \
        -console "mc86end1:40011" \
        -power {npcdut2b 2} \
        -power_button auto \
        -tag PHOENIX2_BRANCH_6_10 \
        -dhd_tag trunk \
        -brand linux-internal-dongle \
        -dhd_brand linux-internal-dongle \
        -type "43342a0-roml/sdio-ag-idsup-p2p-err-assert-sr-dmatxrc{,-autoabn}" \
        -nvram "src/shared/nvram/bcm943342fcagb.txt" \
	-modopts {sd_uhsimode=3 dhd_sdiod_uhsi_ds_override=A} \
        -postinstall {dhd -i eth0 serial 1; dhd -i eth0 sd_divisor 1} \
        -nocal 1 -noframeburst 1 \
	-post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}}


###########################################
# MC86b
# 4345fcpagb B1 PCIe 1x1 Dongle ePA (from Paul Filanowski, 3/26/2014, dev manager: Mike Hogan)
# Switched from 7_16 to 7_15, 4/8/2015
###########################################
# PCIe dongle mode
UTF::DHD mc86tst13 \
        -lan_ip mc86tst13 \
        -sta {4345b1 eth0} \
        -hostconsole "mc86end1:40014" \
        -power {npcdut2a 2} \
        -power_button auto \
	-tag BIS120RC4_BRANCH_7_15 \
        -brand linux-external-dongle-pcie \
	-dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -nvram bcm94345fcpagb_epa.txt \
	-customer olympic \
        -slowassoc 5 -escan 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -tcpwindow 1152k -udp 400m \
        -nocal 1 -docpu 1 -nointerrupts 1 \
	-post_perf_hook {{%S wl dump rssi}} \
        -wlinitcmds {wl down; wl vht_features 3; wl country XZ/35;} \
        -msgactions {
            "DTOH msgbuf not available" FAIL
        }

4345b1 clone 4345px \
	-perfonly 1 -perfchans {36/80} \
	-type ../C-4345__s-B1/tempranillo.trx -clm_blob tempranillo.clmb \
	-app_tag BIS120RC4_BRANCH_7_15 \
	-wlinitcmds {wl down; wl vht_features 3; wl country XZ/202;}

4345b1 clone 4345p \
	-type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap.bin

4345b1 clone 4345pbf \
	-type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-clm_min-txpwrcap.bin
	#-type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-bssinfo-logstrs.bin
	#-type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-clm_min-txpwrcap-bssinfo.bin
	#-type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-clm_min-txpwrcap.bin


# PCIe NIC mode
UTF::Linux mc86tst13n \
        -lan_ip mc86tst13 \
        -sta {4345pn eth0} \
        -console "mc86end1:40014" \
        -power {npcdut2a 2} \
        -power_button auto \
        -tag BIS120RC4_BRANCH_7_15 \
    	-type debug-apdef-stadef-p2p-mchan-tdls \
    	-slowassoc 5 -reloadoncrash 1 \
    	-datarate {-i 0.5 -frameburst 1} \
    	-tcpwindow 1152k -udp 400m \
    	-nopm1 1 -nopm2 1 -nobighammer 1 -nointerrupts 1 \
	-post_perf_hook {{%S wl dump rssi}} \
    	-wlinitcmds {wl vht_features 3;}

    	#-wlinitcmds {wl vht_features 3; wl assert_type 1}


###########################################
# AP Section
###########################################
# softAP
# 4360mc_P198 - 11ac 3x3
UTF::Linux mc86tst8 \
        -lan_ip mc86tst8 \
        -sta {4360ap enp1s0} \
        -console "mc86end1:40007" \
        -power {softap1 2} \
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -tcpwindow 4m \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3;}


# ----------------------------------------------
# Trial: use this AP as STA, 1/10/2014
4360ap clone 4360bx1 -tag BISON_BRANCH_7_10 \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl txchain 1; wl rxchain 1;}
# ----------------------------------------------

4360ap clone 4360softap
4360softap configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1

4360ap clone 4360softap-egl
4360softap-egl configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1

4360ap clone 4360softap-tot -tag trunk
4360softap-tot configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1

4360ap clone 4360softap-bis -tag BISON_BRANCH_7_10 
4360softap-bis configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1

# temp fix for 43342 (tput drop on downstream)
4360softap clone 4360softap-bis-x1 -tag BISON_BRANCH_7_10
4360softap-bis-x1 configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; \
            service dhcpd stop; wl country US/0; wl txchain 1;
        }

4360ap clone 4360softap-egl-bf1
4360softap-egl-bf1 configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

4360ap clone 4360softap-egl-bf0
4360softap-egl-bf0 configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}

4360ap clone 4360softap-tot-bf1 -tag trunk
4360softap-tot-bf1 configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

4360ap clone 4360softap-tot-bf0 -tag trunk
4360softap-tot-bf0 configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}

4360ap clone 4360softap-bis-bf1 -tag BISON_BRANCH_7_10
4360softap-bis-bf1 configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

4360ap clone 4360softap-bis-bf0 -tag BISON_BRANCH_7_10
4360softap-bis-bf0 configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}


# -----------------------------------------
# For other tests
# -----------------------------------------
#
# Only use one chain (so, AP transmit in SISO mode) 8/7/2013
4360ap clone 4360softap-x1-impbf1 -tag AARDVARK_BRANCH_6_30
4360softap-x1-impbf1 configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl rxchain 1; wl txchain 1;}

4360ap clone 4360softap-x1-impbf0 -tag AARDVARK_BRANCH_6_30
4360softap-x1-impbf0 configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl txbf 0; wl txbf_imp 0; wl rxchain 1; wl txchain 1;}

# For testing builds before 2013.5.9.1
4360ap clone 4360softap-impbf1-t
4360softap-impbf1-t configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl stbc_tx 0; wl stbc_rx 0; wl txbf_imp 2;}

4360ap clone 4360softap-impbf0-t
4360softap-impbf0-t configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl stbc_tx 0; wl stbc_rx 0;}

# 6/11/2014
4360softap clone 4360softap-bis-x -tag BISON_BRANCH_7_10
4360softap-bis-x configure -ipaddr 192.168.1.128 -attngrp G2 -ap 1 -hasdhcpd 1 \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl reset_cnts}} \
        -post_perf_hook {{%S wl rx_amsdu_in_ampdu} {%S wl dump ampdu} {%S wl dump rssi}}


# Turn on post log processing ('parse_wl_logs' link on report html page)
#set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl
#set ::aux_lsf_queue sj-hnd

###
UTF::Q mc86b

