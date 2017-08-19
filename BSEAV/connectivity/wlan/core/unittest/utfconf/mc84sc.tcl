


#-*-mc84sc.tcl-*-

package require UTF::Sniffer
package require UTF::Linux
package require UTF::Aeroflex
package require UTF::Multiperf

# mc84sc configuration

# Optional items for controlchart.test to run each iteration
#set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}
set UTF::IPerfBeta 1
set UTF::TcpReadStats 1

set ::UTF::SummaryDir "/projects/hnd_sig_ext10/sunnyc/mc84sc"
set ::UTF::SummaryLoc "/projects/hnd_sig_ext10/sunnyc/Sniffer"

# The conf file to test

#Endpoints
UTF::Linux mc84end1 -lan_ip 10.19.85.71 -sta {lan em1}
#UTF::Linux mc52end2 -lan_ip 10.19.86.220 -sta {lan1 eth1}

# mc11snf1
UTF::Sniffer SNIF -lan_ip 10.19.85.81 -user root -tag BASS_BRANCH_5_60 -sta {sta4360 eth1}

UTF::Aeroflex Aflex -lan_ip 172.16.1.210 -group {G1 {1 2 3} G2 {4 5 6} G3 {7 8 9} ALL {1 2 3 4 5 6 7 8 9}}

# Power
UTF::Power::Synaccess mc84npc1 -lan_ip 172.16.1.150
UTF::Power::Synaccess mc84npc2 -lan_ip 172.16.1.10
#UTF::Power::Synaccess mc52npc3 -lan_ip 172.16.1.12
#UTF::Power::Synaccess mc52npc4 -lan_ip 172.16.1.13


#set ::UTF::PostTestAnalysis "/home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl"
# Set default to use wl from trunk; Use -app_tag to modify.
set UTF::TrunkApps 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G1 attn 0
    G2 attn 0
    G3 attn 0

    # Make sure all systems are deinit and down
    foreach S {4360ap 4359b0b} {
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


set ::UTF::PostTestHook {
    package require UTF::utils
    UTF::do_post_test_analysis [info script] ""
}

set ::UTF::recovery_max 0

#set ::aux_lsf_queue sj-hnd

#set ::rvr_sta_init {{%S wl stbc_rx 1} {%S wl stbc_tx -1}}
#set ::rvr_softap_init {{%S wl stbc_rx 1} {%S wl stbc_tx -1}}

# Try John's new setup
# For STA->AP
#set ::rvr_sta_to_ap_init
#set ::rvr_sta_4319st_init 
#set ::rvr_sta_4319st_to_ap_init

# For STA->SoftAP
#set ::rvr_sta_init
#set ::rvr_sta_to_softap_init {{%S wl stbc_rx 1} {%S wl stbc_tx -1}}
#set ::rvr_sta_4319st_init
# only for recent build
set ::rvr_sta_4319st_to_softap_init {{%S wl stbc_rx 1} {%S wl stbc_tx 1}}

# For AP
#set ::rvr_ap_init

# Fro SoftAP
#set ::rvr_softap_init {{%S wl stbc_rx 1} {%S wl stbc_tx -1}}
#set ::rvr_sta_43237ap_init {{%S wl stbc_rx 1} {%S wl stbc_tx -1}}
# only for recent mail
#set ::rvr_sta_43237ap_am_softap_init {{%S wl stbc_rx 1} {%S wl stbc_tx 1}}




# SoftAP/STA
########################

# 4360NIC Linux 



UTF::Linux mc84tst3 \
    -sta "4360ap enp3s0"\
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \
    -tcpwindow 4m \
    -power {mc84npc1 2} -power_button {auto} \
    -brand linux-internal-wl 

4360ap configure -ipaddr 192.168.1.21 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid 4360SoftAP -tcpwindow 2M -perfchans {6 6l 36 36l 36/80}

4360ap clone 4360ap_TOB \
    -tag BISON_BRANCH_7_10

4360ap_TOB configure -ipaddr 192.168.1.21 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid 4360SoftAP -tcpwindow 2M

4360ap clone 4360ap_TWIG \
    -tag BISON_REL_7_10_*

4360ap_TWIG configure -ipaddr 192.168.1.21 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid 4360SoftAP -tcpwindow 2M

4360ap clone 4360apx \
    -tcpwindow 0

4360ap clone 4360apy \
    -tcpwindow 2m

# Objects for Explicit TxBF
4360ap clone 4360ap-bf1
4360ap-bf1 configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1

4360ap clone 4360ap-bf0
4360ap-bf0 configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 \
    -wlinitcmds {wl msglevel +assoc; wl btc_mode 0; wl dtim 3; \
		     service dhcpd stop; wl down; wl mimo_bw_cap 1; wl vht_features 3; wl txbf 0; wl txbf_imp 0;
    }


UTF::DHD mc84tst1 \
    -sta {43434b0 eth0} \
    -tag DINGO07T_BRANCH_9_35 \
    -power {mc84npc2 2} -power_button {auto} \
    -brand hndrte-dongle-wl\
    -nvram "src/shared/nvram/bcm943430fsdng_Bx.txt"\
    -type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-hs20sta/rtecdc.bin\
    -dhd_tag DHD_REL_1_363_17 \
    -dhd_brand linux-external-dongle-sdio \
    -app_tag trunk \
    -slowassoc 5 -noaes 1 -notkip 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -tcpwindow 1152k -udp 400m -nocal 1\
    -nopm1 1 -nopm2 1

43434b0 configure -ipaddr 192.168.1.20 


###########################################
## MC84sc
## 4359fcpagbss_2_P201 B0 - 11ac 2x2 (From Tan Dadurian, 11/12/2014)
############################################

UTF::DHD mc84tst4 \
    -sta "4360ap2 eth0"\
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \
    -tcpwindow 4m \
    -power {mc84npc1 2} -power_button {auto} \
    -brand linux-internal-wl

4360ap2 configure -ipaddr 192.168.1.22 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid 4360SoftAP -tcpwindow 2M -perfchans {6 6l 36 36l 36/80}



UTF::DHD mc84tst5 \
        -sta {4359b0b eth0 P4359b0b wl0.1} \
        -power {mc84npc2 2} \
        -power_button "auto" \
        -console "mc84end1:40002" \
        -hostconsole "mc84end1:40001" \
        -tag DINGO_BRANCH_9_10 \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -nvram "bcm94359fcpagbss_2.txt" \
        -tcpwindow 2m -udp 800m \
        -nocal 1 -slowassoc 5 \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -type 4359b0-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-idsup-assert-err-die3-rsdbsw-sstput/rtecdc.bin \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl rsdb_mode 0;}

#MIMO

4359b0b clone 4359b0bx \
    -sta {4359b0bx eth0 P4359b0bx wl0.1} \
    -perfonly 1 -perfchans {36/80} \
    -type threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-sr-pktctx-die3-rsdbsw/rtecdc.bin 
 
4359b0b clone 4359b0b1 \
    -sta {4359b0b1 eth0 P4359b0b1 wl0.1} \
    -perfchans {36/80} \
    -wlinitcmds {wl down; wl vht_features 3; wl bss -C 2 sta;} \
    -type 4359b0-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-idsup-assert-err-die3-rsdbsw-sstput/rtecdc.bin

4359b0b1 clone 4359b0b.2 \
    -sta {4359b0b.2 eth0 P4359b0b.2 wl0.1} \
    -perfonly 1 -perfchans 3l
4359b0b1 configure -dualband {43602softap-bis 4359b0b -c1 36/80 -c2 3l -b1 800m -b2 800m}

##

4359b0bx clone 4359b0bx \
    -sta {4359b0bx eth0 _4359b0bx.2 wl0.1} \
    -perfchans {36/80} \
    -wlinitcmds {wl down; wl vht_features 3; wl bss -C 2 sta}

_4359b0bx.2 clone 4359b0bx.2 \
    -sta {_4359b0bx eth0 4359b0bx.2 wl0.1} \
    -perfonly 1 -perfchans 3l
4359b0bx.2 configure -dualband {43602softap-bis 4359b0bx -c1 36/80 -c2 3l -b1 800m -b2 800m}


############################

#####
UTF::Q mc84sc
