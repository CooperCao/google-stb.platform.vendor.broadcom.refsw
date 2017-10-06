# -*-tcl-*-
#
# Testbed configuration file for mc56 (Wiced)
#

# Load Packages
#package provide UTF::Wiced 2.0

#package require snit
#package require UTF::doc
#package require UTF::Test::APRetrieveSecurity

package require UTF::Aeroflex
package require UTF::Wiced


# SummaryDir sets the location for test results

set ::UTF::SummaryDir "/projects/hnd_sig_ext10/sunnyc/mc56"

# Define power controllers on cart
UTF::Power::Synaccess mc56npc2 -lan_ip 172.16.1.10

# Attenuator - Aeroflex

UTF::Aeroflex af -lan_ip 172.16.1.210 -relay lan \
    -group {G1 {1} ALL {1 2 3 4 5 6}}

G1 configure -default 20


#set ::UTF::relay localhost
#set ::UTF::sdk1 "/cygdrive/c/WICED-SDK"

set ::UTF::SetupTestBed {

#component base -inherit yes
# option -relay localhost
# option -sdk1 "/cygdrive/c/WICED-SDK"

    G1 attn default
    # git pull
#    localhost -n -t 60 \
#    "cd sdk1 &&\
#git reset --hard"
    return
}

# UTF Controller
UTF::Linux mc56end1 -sta {lan eth3} 

#SoftAP
UTF::Linux mc84tst1 -sta {4360ap enp1s0} \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -wlinitcmds {wl down; wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \
    -tcpwindow 4m 


#-power {mc11npc6 1}

4360ap configure -ipaddr 192.168.1.21 -hasdhcpd 1 -attngrp G1 -ap 1 -ssid 4360SoftAP

# WICED SDK relay
UTF::Linux mc56tst2 
UTF::Linux mc56tst3
#-user $::tcl_platform(user)

# Remove -e to avoid running out of arg space on Wiced
catch {unset UTF::MultiperfE}


# Note consolelogger set up on relay via:
# ./UTF.tcl mc56tst1 InstallConsolelogger -dev /dev/ttyS3
# on linux, ttyUSB1

# WICED DUT
UTF::Wiced _43907 -sta {43907} \
    -sdk "/root/Wiced-SDK" \
    -platform "BCM943909WCD1_3.B1" \
    -sdkfw "43909/43909B0.bin" \
    -type 43909b0-roml/m2m-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth-nouart-dfsradar-mfp-swdiv-ve.bin \
    -relay mc56tst2 \
    -console mc56tst2:40001 \
    -power {mc56npc2 2} \
    -power_button "auto" \
    -tag BIS120RC4_TWIG_7_15_168 \
    -udp 250m -perfchans {36/80 3} \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g {24-63} -attn2g {45-77} -pad 29 -frameburst 1} \
    -wlinitcmds {wl down; wl bw_cap 2g -1; wl vht_features 3}

43907 configure -ipaddr 192.168.1.22
#P43907 configure -ipaddr 192.168.5.22


UTF::Wiced _43907_2 -sta {43907_2} \
    -sdk "/root/Wiced-SDK" \
    -platform "BCM943909WCD1_3.B1" \
    -sdkfw "43909/43909B0.bin" \
    -type 43909b0-roml/m2m-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth-nouart-dfsradar-mfp-swdiv-ve.bin \
    -relay mc56tst3 \
    -console mc56tst3:40002 \
    -power {mc56npc2 1} \
    -power_button "auto" \
    -tag BIS120RC4_TWIG_7_15_168 \
    -udp 250m -perfchans {36/80 3} \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g {24-63} -attn2g {45-77} -pad 29 -frameburst 1} \
    -wlinitcmds {wl down; wl bw_cap 2g -1; wl vht_features 3}


UTF::Wiced _43909 -sta {43909} \
    -sdk "/root/Wiced-SDK" \
    -platform "BCM943909WCD1_3.B1" \
    -sdkfw "43909/43909B0.bin" \
    -type 43909b0-roml/m2m-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth-nouart-dfsradar-mfp-swdiv-ve.bin \
    -relay mc56tst3 \
    -console mc56tst3:40001 \
    -power {mc56npc3 1} \
    -power_button "auto" \
    -tag BIS120RC4_TWIG_7_15_168 \
    -udp 250m -perfchans {36/80 3} \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g {24-63} -attn2g {45-77} -pad 29 -frameburst 1} \
    -wlinitcmds {wl down; wl bw_cap 2g -1; wl vht_features 3}

43909 configure -ipaddr 192.168.1.23
#P43909 configure -ipaddr 192.168.5.23


# memory is low
UTF::Wiced _4343W -sta {4343W} \
    -sdk "/root/Wiced-SDK" \
    -platform "BCM94343WWCD2" \
    -relay mc56tst2 \
    -console mc56tst2:40001 \
    -power {mc56npc2 1} \
    -power_button "auto" \
    -tag BIS120RC4_TWIG_7_15_168 \
    -udp 250m -perfchans {36/80 3} \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g {24-63} -attn2g {45-77} -pad 29 -frameburst 1} \
    -wlinitcmds {wl down; wl bw_cap 2g -1; wl vht_features 3}

4343W configure -ipaddr 192.168.1.24
#P4343W configure -ipaddr 192.168.5.24


UTF::Q mc56 lan
# -console mc56tst1:40000 \
