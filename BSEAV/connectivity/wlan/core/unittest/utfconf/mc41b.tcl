# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: fe862a0dd4d02d3d200dae055374804f22c568cd $
#
# Testbed configuration file for MC41testbed
#

source utfconf/mc41shared.tcl

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc41b"

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    B attn default
}


UTF::Linux mc41tst3 -sta {4360b enp1s0} \
    -console {mc41end1:40013} \
    -power {webswitch2 2} \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON05T_BRANCH_7_35 \
    -tag EAGLE_BRANCH_10_10 \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \
    -tcpwindow 4m

4360b configure -ipaddr 192.168.1.95 -attngrp B -hasdhcpd 1 -ap 1

###################################################

#set UTF::PerfCacheMigrate {
#    BIS735T255RC1 BIS747T144RC2
#}

UTF::DHD mc41tst9n -lan_ip mc41tst9 -sta {43452 eth0} \
    -power {nb1 5} \
    -hostconsole "mc41end1:40010" \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -tag BIS747T144RC2_BRANCH_7_64 \
    -app_tag BIS747T144RC2_BRANCH_7_64 \
    -dhd_tag DHD_BRANCH_1_359 \
    -nocal 1 -slowassoc 5 \
    -nvram "src/shared/nvram/bcm94345Corona2TDKKK.txt" \
    -type "43452a3-roml/pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-clm_min-ecounters-bssinfo-ltecx-txpwrcap-err-txcal-p5txgtbl-assert/rtecdc.bin" \
    -clm_blob "43452_hans.clm_blob" \
    -txcb "43452_hans.txcap_blob" \
    -customer olympic \
    -wlinitcmds {wl down;wl vht_features 3} \
    -escan 1 -nobighammer 1 -datarate {-i 0.5} \
    -tcpwindow 2m -udp 400m -nointerrupts 1 \
    -yart {-attn5g 0-95 -attn2g 0-95 -pad 36} \
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
UTF::Q mc41b
