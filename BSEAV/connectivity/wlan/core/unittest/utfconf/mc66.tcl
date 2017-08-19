# -*-tcl-*-
#
# Configuration file for Milosz Zielinski's MC66 testbed
#

# Load Packages
package require UTF::Linux
package require UTF::Sniffer


# UTFD support
set ::env(UTFDPORT) 9978
package require UTFD

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/mc66"

# Enable Windows TestSigning
set UTF::TestSigning 1

# Define power controllers on cart
#in the upper large Ramsey
UTF::Power::Synaccess npc1 -lan_ip 172.16.1.21 -relay mc66end1 -rev 1
#in the router(AP) small Ramsey
UTF::Power::Synaccess npc2 -lan_ip 172.16.1.22 -relay mc66end1 -rev 1
#in the lower large Ramsey
UTF::Power::Synaccess mc66npc10 -lan_ip 10.22.23.181 -relay mc66end1 -rev 1
#in the lower large Ramsey
UTF::Power::Synaccess mc66npc11 -lan_ip 10.22.23.182 -relay mc66end1 -rev 1

# switches on the cart
# 172.16.1.70 - on the middle shelf
# 172.16.1.71 - in the lower large Ramsey
# 172.16.1.72 - on the middle shelf
# 172.16.1.73 - in the upper large Ramsey
# 172.16.1.74 - in the router(AP) small Ramsey


set ::UTF::SetupTestBed {
    #AP1 restart wl0_radio=0
    unset ::UTF::SetupTestBed
    return
}

# UTF Endpoint - Traffic generators (no wireless cards)
UTF::Linux mc66end1 \
    -tcpwindow 512k \
    -sta {lan eth1}

# Linux host 1
UTF::Linux mc66tst1 \
        -sta {4358fc19 enp3s0} \
        -tcpwindow 512k \
        -power {npc1 1} \
        -power_button {auto} \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}\
        -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
        -slowassoc 5 -reloadoncrash 1 \
        -nvram bcm94358wlspie_p105.txt \
        -datarate {-i 0.5 -frameburst 1} -udp 800m \
        -wlinitcmds {wl down; wl bw_cap 2g -1; wl vht_features 3} \
        -tcpwindow 3M
    
4358fc19 clone 4358fc19-b735 -tag BISON05T_BRANCH_7_35 
4358fc19 clone 4358fc19b -tag BISON_BRANCH_7_10
    
#       -docpu 1
#   -extsup 1 
#   -datarate {-i 0.5 -frameburst 1}
#            -app_tag trunk
#            -dhd_tag trunk
UTF::DHD mc66tst1d -lan_ip mc66tst1 -sta {4358 eth0} \
    -power {npc1 1} \
    -hostconsole "mc66end1:40000" \
    -tag BISON05T_BRANCH_7_35 \
    -app_tag DHDNC39RC65_BRANCH_1_47 \
    -dhd_tag DHDNC39RC65_BRANCH_1_47 \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -brand linux-external-dongle-pcie \
    -nvram bcm94358wlspie_p105.txt \
    -type 4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-proxd-nan-shif-tbow-assert/rtecdc.bin \
	 -wlinitcmds {wl vht_features 3} \
    -slowassoc 5 \
    -notkip 1 -noaes 1 -nobighammer 1 -nocal 1 \
    -tcpwindow 3m -udp 800m \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -msgactions {"Pktid pool depleted." WARN}

    
#   -perfonly 1 -perfchans {36/80} \  
4358 clone 4358b35 \
	-tag BISON05T_BRANCH_7_35 \
	-type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-sstput/rtecdc.bin

#	-type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-sstput/rtecdc.bin
#	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert/rtecdc.bin

4358b35 clone 4358b35x \
	-type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-sstput/rtecdc.bin
#	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth/rtecdc.bin

4358b35 clone 4358b35ssx \
	-type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-sstput/rtecdc.bin

4358 clone 4358b35t \
	-tag BISON05T_TWIG_7_35_105 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin
#	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert/rtecdc.bin
	
4358b35t clone 4358b35tx \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth/rtecdc.bin
	
4358 clone 4358b \
	-tag BISON_BRANCH_7_10 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin
#	-type 4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert/rtecdc.bin

4358 clone 4358b-sarctrl \
	-tag BISON_BRANCH_7_10 \
        -nobighammer 1  -noaes 1 \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-amsdutx-wl11u-mfp-tdls-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-assert/rtecdc.bin

4358 clone 4358b-relmcast \
	-tag BISON_BRANCH_7_10 \
        -nobighammer 1  -noaes 1 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin
#	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin
	
4358 clone 4358b-112 \
        -tag BISNC105RC66_BRANCH_7_112 \
        -nobighammer 1  -noaes 1 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx5g-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-btcdyn-wepso-rcc-noccxaka-sarctrl-xorcsum-assert.bin


4358 clone 4358b35105RC \
        -tag BISNC105RC66_BRANCH_7_112 \
        -nobighammer 1  -noaes 1 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin


4358 clone 4358-andrey \
        -image /projects/hnd_sw_mobfw/work/andrey/BISON05T_TWIG_7_35_105/4358a1-roml-pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx-autosf-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-proxd-shif-btcdyn-idsup-chkd2hdma-ssoption.bin \
	-wlinitcmds {wl vht_features 3; wl amsdu_autosf 0}
#	-image /projects/hnd_sw_mobfw/work/andrey/BISON05T_TWIG_7_35_105/rtecdc-autosf-tput.bin
	
		
# 4358 clone 4358b
#	-tag BISON_BRANCH_7_10
#	-type 4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-proxd-nan-shif-tbow-assert/rtecdc.bin

# 4358b clone 4358bx
#	-type 4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-proxd-nan-shif-tbow/rtecdc.bin

        
UTF::Linux mc66tst2 \
-sta {43570fc19 enp3s0} \
        -tcpwindow 512k \
         -power {npc1 2} \
        -power_button {auto} \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}\
        -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
         -slowassoc 5 -reloadoncrash 1 \
    -nvram bcm943570pcie_p205.txt \
    -datarate {-i 0.5 -frameburst 1} -udp 800m \
    -wlinitcmds {wl down; wl bw_cap 2g -1; wl vht_features 3} \
    -tcpwindow 3M -yart {-pad 34 -attn5g 16-95 -attn2g 29-95 -frameburst 1}
       
43570fc19 clone 43570fc19-BISON -tag BISON_BRANCH_7_10       
        


# Dell E6400 with 43142 chip
# Windows 8.1 x64    
#  2015.3.31 update (updated on 2015.4.15)
#  2015.7.02 update installed on 2015.7.6
#
UTF::Cygwin mc66tst3 -sta {43142Win81x64} \
    -osver 8164 \
    -sign 1 \
    -brand win8x_internal_wl \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -tcpwindow 4M \
    -kdpath kd.exe \
    -usemodifyos 1 \
    -power {mc66npc10 2} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}}

43142Win81x64 clone 43142Win81x64-TOT
43142Win81x64 clone 43142Win81x64-TOT-8 -brand win8_internal_wl
43142Win81x64 clone 43142Win81x64_BISON_735 -tag BISON05T_BRANCH_7_35


# Dell E6400 with 4352 chip
# Windows 8 x64    
# 2015.3.31 update (updated on 2015.4.15)
#  2015.7.02 update installed on 2015.7.6
# 
UTF::Cygwin mc66tst4 -sta {4352Win8x64} \
    -osver 864 \
    -sign 1 \
    -brand win8x_internal_wl \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -tcpwindow 4M \
    -kdpath kd.exe \
    -usemodifyos 1 \
    -power {mc66npc10 1} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}}

4352Win8x64 clone 4352Win8x64-TOT
4352Win8x64 clone 4352Win8x64_BISON_735 -tag BISON05T_BRANCH_7_35


# 4708 wireless router.
UTF::Router AP1 \
	 -sta {
    4708/4360 eth2 4708/4360.%15 wl1.%
    4708/4331 eth1 4708/4331.%15 wl0.%
    } \
    -power {npc2 2} \
    -relay "mc66end1" \
    -lan_ip 192.168.1.1 \
    -brand linux-2.6.36-arm-internal-router \
    -lanpeer lan \
    -console "mc66end1:40004" \
	-nvram {
	watchdog=2000; # PR#90439
	wl_msglevel=0x101
	wl0_ssid=4708/4331
	wl0_chanspec=3
	wl0_radio=0
	wl1_ssid=4708/4360
	wl1_chanspec=36
	wl1_radio=0
	wl1_vht_features=2
	wl0_vht_features=3
	samba_mode=2
	}


4708/4360 clone 4708/4360-gooddate -date 2015.5.18.0
4708/4331 clone 4708/4331-gooddate -date 2015.5.18.0


4708/4360 clone 4708b/4360 -tag BISON_BRANCH_7_10 \
    -nvram [concat {
	wl0_pspretend_retry_limit=5
	wl1_pspretend_retry_limit=5
	wl0_atf=1
	wl1_atf=1
    } [4708/4360 cget -nvram]]

4708/4331 clone 4708b/4331 -tag BISON_BRANCH_7_10 \
    -nvram [concat {
	wl0_pspretend_retry_limit=5
	wl1_pspretend_retry_limit=5
	wl0_atf=1
	wl1_atf=1
    } [4708/4331 cget -nvram]]

#4708b/4360 clone 4708a/4360   -tag AARDVARK01T_TWIG_6_37_14
#4708b/4331 clone 4708a/4331   -tag AARDVARK01T_TWIG_6_37_14
4708b/4360 clone 4708b714/4360   -tag BISON04T_BRANCH_7_14
4708b/4331 clone 4708b714/4331   -tag BISON04T_BRANCH_7_14
    
# 	-txt_override { 	watchdog=6000    }
###

#device is present, but powered off
if {0} {
UTF::Router AP2 \
    -sta "4717A eth1" \
    -relay "mc66end1" \
    -lan_ip 192.168.1.66 \
    -console "mc66end1:40001" \
    -lanpeer lan \
     -power {npc2 1} \
    -tag BISON_BRANCH_7_10 \
    -brand linux26-external-vista-router-combo \
    -trx "linux-gzip" \
    -nvram {
	    et0macaddr=00:90:4c:07:00:8b
		macaddr=00:90:4c:07:01:9b
        lan_ipaddr=192.168.1.66
        lan_gateway=192.168.1.66
        dhcp_start=192.168.1.100
        dhcp_end=192.168.1.150
        lan1_ipaddr=192.168.2.66
        lan1_gateway=192.169.2.66
        dhcp1_start=192.168.2.100
        dhcp1_end=192.168.2.150
        fw_disable=1
        wl_msglevel=0x101
        wl0_ssid=mc66testAP2
        wl0_channel=1
        wl0_radio=0
        antswitch=0
        wl0_obss_coex=0
}
}

       # et0macaddr=00:21:29:01:00:02
       # macaddr=00:21:29:01:00:03

# AP3 don't exist
if {0} {
# -console "mc66end1:40002"
# Broadcom 5358 PLC WiFi router
UTF::Router AP3 \
    -sta "53582 eth1" \
    -relay "mc66tst2" \
    -lan_ip 192.168.1.3 \
    -lanpeer lan \
    -power {npc3 1} \
    -brand linux26-external-plc-router-full-src \
    -tag "AKASHI_REL_5_110_58_*" \
    -nvram {
        lan_ipaddr=192.168.1.3
        lan_gateway=192.168.1.1
        dhcp_start=192.168.1.161
        dhcp_end=192.168.1.250
        fw_disable=1
        wl_msglevel=0x101
        wl0_ssid=mc66testPLCAPURE
        wl0_channel=1
        wl0_radio=0
        antswitch=0
        wl0_obss_coex=0
        wl_plc=1
        wl0_plc=1
}

}
