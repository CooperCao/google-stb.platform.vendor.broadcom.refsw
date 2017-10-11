# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for MC41testbed
#

source utfconf/mc41shared.tcl

UTF::Power::WebSwitch webswitch2 -lan_ip 192.168.1.15 -relay lan
UTF::Power::Synaccess npc4 -lan_ip 192.168.1.18 -relay lan

package require UTF::Vaunix
UTF::Vaunix D -lan_ip mc41end1


# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc41d"

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    D attn default
    foreach S {4360d 43451b1o 43452 4339a0} {
	catch {$S wl down}
	$S deinit
    }
}

UTF::Linux mc41tst11 -sta {4360d enp1s0} \
    -console {mc41end1:40012} \
    -power {npc4 2} \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \
    -tcpwindow 4m
4360d configure -ipaddr 192.168.1.92 -attngrp D -hasdhcpd 1 -date 2017.1.4.0

###################################################

UTF::DHD mc41tst8 -sta {43451b1o eth0} \
    -app_tag BIS759T5RC2_BRANCH_7_63 \
    -app_brand linux-external-dongle-pcie \
    -tag BIS759T5RC2_BRANCH_7_63 \
    -dhd_tag DHD_BRANCH_1_579 \
    -power {npc4 1} \
    -console "mc41end1:40009" -console {} \
    -hostconsole "mc41end1:40007" \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94345fcpagb_epa.txt" \
    -type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-bssinfo/rtecdc.bin \
    -customer olympic \
    -wlinitcmds {wl down;wl vht_features 3;wl country XZ/35} \
    -slowassoc 5 -escan 1 -nobighammer 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -tcpwindow 1152k -udp 400m -nocal 1 -docpu 1 -nointerrupts 1 \
    -yart {-attn5g 7-63 -attn2g 17-63 -pad 44 -frameburst 1} \
    -msgactions {
	"DTOH msgbuf not available" FAIL
    }

43451b1o clone 43451b1o.1 -sta {_43451b1o eth0 43451b1o.1 wl0.1} \
    -apmode 1 -nochannels 1 -slowassoc 5 \
    -wlinitcmds {wl down;wl vht_features 3;wl apsta 1;wl ssid -C 1 43451b1AP} \
    -noaes 1 -notkip 1 -yart {}

43451b1o.1 configure -ipaddr 192.168.1.88

43451b1o clone 43451b1ox \
    -customer olympic \
    -type ../C-4345__s-B1/tempranillo.trx -clm_blob tempranillo.clmb \
    -wlinitcmds {wl down;wl vht_features 3;wl country XZ/202} \
    -perfonly 0

##

43451b1o clone 43451b1b61 \
    -tag BIS759T2RC1_BRANCH_7_61 \
    -app_tag BIS759T2RC1_BRANCH_7_61
43451b1ox clone 43451b1b61x \
    -tag BIS759T2RC1_BRANCH_7_61 \
    -app_tag BIS759T2RC1_BRANCH_7_61

# targets for private builds
#    -type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace-srscan-clm_min-txpwrcap/rtecdc.bin \
#    -clm_blob 43451_tempranillo.clm_blob \

UTF::Linux mc41tst8n -lan_ip mc41tst8 -sta {43451b1n eth0} \
    -power {npc4 1} \
    -console "mc41end1:40007" \
    -type debug-p2p-mchan \
    -wlinitcmds {wl vht_features 3} \
    -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -tcpwindow 1152k -udp 400m -docpu 1 \
    -nopm1 1 -nopm2 1 -nobighammer 1 -nointerrupts 1 \
    -yart {-attn5g 7-63 -attn2g 17-63 -pad 44 -frameburst 1} \
    -msgactions {
	{ai_core_reset: Failed to take core} {
	    $self worry $msg;
	    $self power cycle;
	    return 1
	}
    }

43451b1n clone 43451b1nb -tag BISON_BRANCH_7_10
43451b1n clone 43451b1no -tag BIS715T254_BRANCH_7_52

###################################

UTF::DHD mc41tst7 -sta {4339a0 eth0} \
    -power {webswitch1 2} \
    -hostconsole "mc41end1:40005" \
    -tag AARDVARK_BRANCH_6_30 \
    -slowassoc 5 \
    -brand linux-external-dongle-sdio \
    -dhd_brand linux-internal-dongle \
    -nvram "bcm94339wlbgaFEM_AM.txt" \
    -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
    -type 4339a0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-ampduhostreorder-ve-mfp-okc-txbf-pktctx-dmatxrc-ltecx-idsup-idauth-err-assert \
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 \
    -postinstall {dhd -i eth0 sd_blocksize 2 256; dhd -i eth0 txglomsize 10} \
    -wlinitcmds {wl pool 60; wl ampdu_mpdu 24; wl vht_features 3} \
    -tcpwindow 1152k -udp 400m -nofragmentation 1 \
    -yart {-attn5g 7-63 -attn2g 17-63 -pad 44 -frameburst 1}

4339a0 configure -ipaddr 192.168.1.93

4339a0 clone 4339a0v37 -tag AARDVARK01T_BRANCH_6_37 \
    -type 4339a0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls{,-autoabn}-ampduhostreorder-ve-mfp-okc-txbf-idsup-idauth-err-assert

4339a0 clone 4339a0.1 -sta {4339a0.0 eth0 4339a0.1 wl0.1} \
    -apmode 1 -nochannels 1 -yart {} -datarate 0 \
    -wlinitcmds {wl pool 60; wl ampdu_mpdu 16; wl vht_features 3; wl bw_cap 2g -1; wl apsta 1; wl ssid -C 1 4350b1AP}
4339a0.1 configure -ipaddr 192.168.1.94

4339a0v37 clone 4339a0s32 -tag AARDVARK01S_{TWIG,REL}_6_37_32{,_*}


# note pool size reduced from 50 to recover some memory
4339a0 clone 4339a0t -tag trunk \
    -type 4339a0-ram/sdio-ag-assert-clm_4335c0_min/rtecdc.bin \
    -extsup 1 -nobighammer 1 \
    -yart {-attn5g 10-63 -attn2g 20-63 -pad 41} \
    -wlinitcmds {wl pool 48; wl ampdu_mpdu 24; wl vht_features 3}


4339a0 clone 4339a0x \
    -type 4339a0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-ampduhostreorder-ve-mfp-okc-txbf-pktctx-dmatxrc-ltecx-idsup-idauth \
    -noaes 1 -perfonly 1 -perfchans {36/80} \
    -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3}


4339a0 clone 4339a0v38 -tag AARDVARK02T_BRANCH_6_38
4339a0x clone 4339a0v38x -tag AARDVARK02T_BRANCH_6_38

4339a0.1 clone 4339a0v38.1 -sta {4339a0v38.0 eth0 4339a0v38.1 wl0.1} \
    -tag AARDVARK02T_BRANCH_6_38

###################################################

UTF::DHD mc41tst9n -lan_ip mc41tst9 -sta {43452 eth0} \
    -power {nb1 5} \
    -hostconsole "mc41end1:40010" \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -tag BIS747T144RC2_BRANCH_7_64 \
    -app_tag BIS747T144RC2_BRANCH_7_64 \
    -app_brand linux-external-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -nocal 1 -slowassoc 5 \
    -nvram "src/shared/nvram/bcm94345Corona2TDKKK.txt" \
    -type "43452a3-roml/pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-clm_min-ecounters-bssinfo-ltecx-txpwrcap-err-txcal-p5txgtbl-assert/rtecdc.bin" \
    -clm_blob "43452_hans.clm_blob" \
    -txcb "43452_hans.txcap_blob" \
    -customer olympic \
    -wlinitcmds {wl down;wl vht_features 3} \
    -escan 1 -nobighammer 1 -datarate {-i 0.5} \
    -tcpwindow 2m -udp 400m -nointerrupts 1 \
    -yart {-attn5g 8-95 -attn2g 4-95 -pad 72} \
    -perfchans {36/80 3}

43452 clone 43452.1 -sta {_43452 eth0 43452.1 wl0.1} \
    -apmode 1 -nochannels 1 -slowassoc 5 \
    -wlinitcmds {wl down;wl vht_features 3;wl apsta 1;wl ssid -C 1 43452AP} \
    -noaes 1 -notkip 1 -yart {}

43452.1 configure -ipaddr 192.168.1.88

43452 clone 43452x \
    -type "../C-43452__s-A3/hans.trx" \
    -clm_blob "hans.clmb" \
    -txcb "hans.txcb" \
    -perfonly 1 -perfchans {36/80}

#####
UTF::Q mc41d
