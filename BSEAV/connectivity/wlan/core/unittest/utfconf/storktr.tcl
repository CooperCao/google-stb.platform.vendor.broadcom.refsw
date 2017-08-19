# -*-tcl-*-
#
# Stork UTF Test Configuration
#
# UTF configuration for Stork testbed @ Broadcom
#

# load packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Power
package require UTF::DHD


UTF::Logfile "storktr.log"

set ::UTF::SummaryDir "/projects/hnd_software_ext3/$::env(LOGNAME)/UTF/storktr"


# Power controllers

UTF::Power::Synaccess ps-storksta \
     -lan_ip 10.191.8.128 \
     -rev 1

UTF::Power::Synaccess ps-storkap \
     -lan_ip 10.191.8.130 \
     -rev 1 

# Attenuator - Aeroflex
UTF::Aeroflex storkaf -lan_ip 10.191.8.131 \
    -group {G1 {1 2 3} ALL {1 2 3 4 5 6 7 8 9}}


# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn 0;
    G1 attn 60;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    # AP1 restart wl0_radio=0


    # delete myself
    unset ::UTF::SetupTestBed

    return
}


# Set testrig specific attenuation ranges for use with RvRNightly1.test
set ::cycle2G20AttnRange "20-85 85-20"

##### Relay / Consolelogger
##### Running FC15
UTF::Linux stork \
    -sta {aplan1 p1p1} \
    -lan_ip 10.191.8.126 

aplan1 configure -ipaddr 192.168.1.50 


##### DUT Host Machine
##### STA BUPC DUT Intel DG41TY motherboard FC15 Linux

UTF::DHD storksta \
    -sta {4335f15 eth1} \
    -power {ps-storksta 1} \
    -lan_ip "10.191.8.127" \
    -user root \
    -tag AARDVARK_TWIG_6_30_207 \
    -nvram "bcm94335wlipa_DVT.txt" \
    -console "10.191.8.126:40000" \
    -dhd_tag NIGHTLY \
    -dhd_brand "linux-internal-dongle" \
    -brand "linux-mfgtest-dongle-sdio" \
    -type "4335b0-ram/sdio-ag-mfgtest-seqcmds-assert-err" \
    -noaes 1 \
    -notkip 1 \
    -perfchans { 3 } \
    -modopts {sd_uhsimode=3} \
    -datarate {-skiptx "9-2 0x2-8x3" -skiprx "9-23 0x2-8x3"} \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl mpc 0; wl mimo_bw_cap 1; wl frameburst 1 }


#    -dhd_image /projects/hnd_wlansys/work/hkushmar/4335/driver/dhdbom/src/dhd/linux/dhd-cdc-sdstd-2.6.29.4-167.fc11.i686.PAE/dhd.ko \
#    -image /projects/hnd_wlansys/work/hkushmar/4335/driver/AARDVARK_TWIG_6_30_207/src/dongle/rte/wl/builds/4335b0-ram/sdio-ag-mfgtest-dump/rtecdc.bin \   
#    -type obj-debug-apdef-stadef \
#    -post_perf_hook {{%S wl dump ampdu} {%S wl rssi} {%S wl nrate}}\
#    -pre_perf_hook {{%S wl ampdu_clear_dump}}
#    -brand "linux-internal-wl" \
#    -nvram "bcm94335wlipa_DVT.txt" \
#    -type obj-debug-apdef-stadef \
#    -console "10.191.8.126:40000" \
#    -modopts assert_type=1 \
#    -brand "linux-internal-wl" \

4335f15 configure -attngrp G1 -ipaddr 192.168.1.1


4335f15 clone 4335ipa \
	-tag AARDVARK_BRANCH_6_30 \
	-nvram "bcm94335wlipa_300.txt" \
	-type "4339a0-roml/sdio-ag-mfgtest-seqcmds-sr-autoabn-assert-err-phydbg-dbgam-dump-ipa"
	

4335f15 clone 4335FEM \
	-tag AARDVARK_BRANCH_6_30 \
	-type "4335b0-ram/sdio-ag-mfgtest-seqcmds-assert-err" \
	-nvram "bcm94335wlbgaFEM_AA.txt"


#windows 7  43224
 
UTF::Cygwin storksta1 \
        -osver 7 \
        -user user \
        -sta {43224Win7} \
        -power {ps-storksta1 1} \
	-lan_ip "10.191.8.132" \
	-tag "NIGHTLY" \
	-installer inf\
        -power_button {auto}
	
#       -console "10.191.8.126:40000" \
  
#43224Win7 clone 43224Win7-KIRIN  -tag "KIRIN_BRANCH_5_100"
#43224Win7 clone 43224Win7-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"

43224Win7 configure -attngrp G1 -ipaddr 192.168.1.3


##### Broadcom Router
##### BUPC SAP Intel DG41TY motherboard f15.
UTF::Linux storkap \
    -power {ps-storkap 1} \
    -lan_ip "10.191.8.129" \
    -sta {4360f15 eth1} \
    -console "10.191.8.126:40001" \
    -tag AARDVARK_REL_6_30_163_35 \
    -wlinitcmds {ifconfig eth1 down;ifconfig eth1 hw ether 00:10:18:AF:52:D3 up; wl up}
    
    
#   -wlinitcmds {ifconfig eth1 down;ifconfig eth1 hw ether 00:10:18:AF:52:D3 up; wl bssid} \
#   -relay {pelicanap} \
#   -image "/projects/hnd/swbuild/build_linux/AARDVARK_REL_6_30_118_30/linux-internal-wl/2012.10.19.0/release/obj-debug-apdef-stadef-2.6.38.6-26.rc1.fc15.i686.PAE/wl.ko"
#   -tag "AARDVARK_REL_6_30_118_30" \
#   -brand "linux-internal-wl" \
#   -console "10.176.8.85:40000" 
#   -lanpeer {aplan1} \


4360f15 configure -attngrp G1 -ap 1 -ipaddr 192.168.1.2


