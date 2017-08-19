
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer2
package require UTF::utils2

package require utfconf
package provide UTF::WebRelay 

package require UTF::Airport

set UTF::DataRatePlot 1
set ::UTF::Gnuplot "/usr/bin/gnuplot"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext9/$::env(LOGNAME)/mc97"

set ::UTF::SummaryDir "/projects/hnd_sig_ext15/$::env(LOGNAME)/mc97"

################Define Testbed Objects
# Define power controllers on cart

UTF::Power::Synaccess mc97npc22_1 -lan_ip 172.19.16.151 -relay "mc97end1" -rev 1
UTF::Power::Synaccess mc97npc22_2 -lan_ip 172.19.16.152 -relay "mc97end1" -rev 1
UTF::Power::Synaccess mc97npc22_3 -lan_ip 172.19.16.153 -relay "mc97end1" -rev 1
#4717_2ap
UTF::Power::Synaccess mc92npc22_4 -lan_ip 172.19.16.154 -relay "mc97end1" -rev 1

UTF::Power::Synaccess mc97npc22_5 -lan_ip 172.19.16.155 -relay "mc97end1" -rev 1

UTF::Power::Synaccess mc97npc22_6 -lan_ip 172.19.16.156 -relay "mc97end1" -rev 1

#4360AP
UTF::Power::Synaccess mc97npc22_7 -lan_ip 172.19.16.110 -relay "mc97end1" -rev 1



#real AP
UTF::Power::Synaccess mc97npc22_6 -lan_ip 172.19.16.156 -relay "mc97end1" -rev 1

UTF::Aeroflex af -lan_ip 172.19.16.11:20000/udp \
    -relay "mc97end1" -group { G1 {1 2 3 } G4 {10 11 12}  ALL {1 2 3 10 11 12} }


#UTF::Power::WebRelay hub -lan_ip 172.19.16.112 -relay "mc97end1"
#Attenuator - Aeroflex
#UTF::Aeroflex af -lan_ip 172.19.16.10 -relay "mc97end1" -group { G1 {1 2 3 } G2 {4 5 6} G3 {7 8 9} ALL {1 2 3 4 5 6 7 8 9} } 

set ::UTF::SetupTestBed {
    package require UTF::Test::RebootTestbed
     UTF::Try "Reboot Testbed" {
     UTF::Test::RebootTestbed -hostlist "4360FC15apv 4354sdio 4356pcie" 
    }
#     catch { snif1 load }
      catch { snif1 wl down }
#     catch { 4360FC15apv wl down }
       af setGrpAttn G1 0
##     af setGrpAttn G2 10
##     af setGrpAttn G3 0
   # unset ::UTF::SetupTestBed  
    return
}

#AP1 and AP2
set  ::src_apmac_index "wl0_hwaddr"
set  ::src_ap_ssid_index "wl0_ssid"
set  ::dst_apmac_index "wl1_hwaddr"
set  ::dst_ap_ssid_index "wl1_ssid"

#
###################

#RvRFastSweep mystepprofile
set UTF::StepProfile(myprofile) ""
set dwell 1
set max 65
set stepsize 1
set upramp ""
set downramp ""
for {set ix 0} {$ix < $max} {incr ix +$stepsize} { 
lappend upramp "$stepsize $dwell" 
}
for {set ix $max} {$ix > 0} {incr ix -$stepsize} {
  lappend downramp "-$stepsize $dwell"
}
set UTF::StepProfile(myprofile) [concat $upramp $downramp]


# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc97end1 \
        -sta {lan eth1 }
#        -lan_ip 192.168.1.99
#        -wlinitcmds { service iperf restart }\
#        -lan_ip 192.168.1.99
#
lan configure -ipaddr 192.168.1.99
########
#p2p
#########p2p
#set ::bt_dut ""                 ;# BlueTooth device under test
#set ::bt_ref ""         ;# BlueTooth reference board
#set ::wlan_dut 43228FC9-p2p             ;# HND WLAN device under test
#set ::wlan_rtr 4717-1                   ;# HND WLAN router
#set ::wlan_tg  mc97tst7

####


#Sniffer fc15 
# -power { mc97npc22_3 1} 
UTF::Sniffer mc97tst2 \
     -sta  {snif1 eth0}\
     -power { mc97npc22_3 3}\
     -power_button {auto}\
     -tag  AARDVARK_BRANCH_6_30\
     -header radiotap
#ruby_branch_6_20
#AARDVARK_BRANCH_6_30
#RUBY_BRANCH_6_20

###
UTF::Linux mc97tst7 \
        -sta "4360FC15ap0 eth0"\
         -tcpwindow 2m \
         -udp 1.2g\
        -power { mc97npc22_1 1} \
        -power_button {auto}\
        -wlinitcmds {wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 }\
        -brand linux-internal-wl\
        -perfchans {36/80 36l 36 6l 6}\
        -tag AARDVARK_BRANCH_6_30\
        -type debug-p2p-mchan

4360FC15ap0 clone 4360FC15v -perfchans { 6l 6} -tag AARDVARK_BRANCH_6_30
# -lanpeer mc97tst7 -host mc97tst7
4360FC15ap0 configure -ipaddr 10.10.10.7 -ap 1 -attngrp G1 -lanpeer mc97tst7 

4360FC15ap0 clone 4360FC15apv -tag AARDVARK_BRANCH_6_30 -type obj-debug-apdef-stadef
4360FC15apv configure -ipaddr 10.10.10.7 -ap 1 -attngrp G1 -wlinitcmds { wl down; wl -u txbf_bfe_cap 1 ; wl -u txbf_bfr_cap 1; wl -u txbf 1 ; wl up }

#AARDVARK_TWIG_6_30_163 --AARDVARK_TWIG_6_30_234
4360FC15ap0 clone 4360FC15ap_twig -tag AARDVARK_TWIG_6_30_* -type obj-debug-apdef-stadef
4360FC15ap_twig configure -ipaddr 10.10.10.7 -ap 1 -attngrp G1 -wlinitcmds { wl down; wl -u txbf_bfe_cap 1 ; wl -u txbf_bfr_cap 1; wl -u txbf 1 ; wl up }


4360FC15apv clone 4360FC15apv_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }
4360FC15apv_ifs configure -ipaddr 10.10.10.7 -ap 1 -attngrp G1


4360FC15ap0 clone  4360FC15apbi -tag BISON_BRANCH_7_10
4360FC15apbi  configure -ipaddr 10.10.10.7 -ap 1 -attngrp G1




###
#4360FC15ap0 clone 4360FC15ap0-P2P \
        -sta { 4360FC15ap0-p2p.1 wl0.1}
#4360FC15ap0 clone 4360FC15ap0-WLAN \
        -sta { 4360FC15ap0-p2p eth0}



###



#4335
#  -hostconsole "mc89end1:40005"
# -power {npcdut2a 1} 
# -nvram bcm94335wlcspMS_AM_P400.txt -> bcm94335wlbgaFEM_AM.txt
###########################################
if { 0 } {
UTF::DHD mc97tst5 \
        -lan_ip mc97tst5 \
        -sta {4335b0 eth0} \
        -power { mc97npc22_2 1}\
        -udp 1.2g\
        -tcpwindow 2m \
        -perfchans {36/80 36l 36 6l 6}\
        -power_button "auto" \
        -tag AARDVARK_BRANCH_6_30 \
        -dhd_tag NIGHTLY \
        -brand linux-external-dongle-sdio \
        -dhd_brand linux-internal-dongle \
        -nvram bcm94335wlbgaFEM_AM.txt \
        -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
        -type 4335b0-roml/sdio-ag-pool-p2p-idsup-idauth-pno-pktfilter-keepalive-aoe-ccx-sr-vsdb-proptxstatus-lpc-autoabn-assert-err-txbf.bin \
        -datarate {-b 350m -i 0.5 -frameburst 1} \
        -nocal 1 \
        -postinstall {dhd -i eth0 txglomsize 10} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl dump rssi}} \
        -wlinitcmds {
            wl down; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; wl mimo_bw_cap 1; wl ampdu_ -dhd_type "dhd-cdc-usb-gpl"mpdu 24; wl vht_features 1; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0 ; wl up
        }

4335b0 clone 4335b0_s
4335b0_s configure -ipaddr 10.10.10.11

# AARDVARK_TWIG_6_30_171
4335b0 clone 4335sdio -tag  AARDVARK_TWIG_6_30_*

#-type 4350b0-roml/sdio-ag-pool-p2p-idsup-idauth-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-assert-err-txbf.bin

#4335b0-roml/sdio-ag-pool-p2p-idsup-idauth-pno-pktfilter-keepalive-aoe-ccx-sr-vsdb-proptxstatus-lpc-autoabn-assert-err-txbf.bin

4335sdio configure -ipaddr 10.10.10.11


4335sdio clone 4335sdio_bf
4335sdio_bf configure \
        -wlinitcmds {
            wl down ; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; wl mimo_bw_cap 1; wl ampdu_mpdu 24; wl vht_features 1; wl txbf 1; wl txbf_bfe_cap 1; wl txbf_bfr_cap 1; wl up
        }  
4335sdio_bf  configure -ipaddr 10.10.10.11

#new
4335b0 clone 4335sdiov -tag AARDVARK_BRANCH_6_30 -type 4335b0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-assert-err-txbf-idsup-idauth-pktctx-dmatxrc.bin

#previous 4335b0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-assert-err-txbf-idsup-idauth.bin

4335sdiov clone 4335sdiov_ns

4335sdiov configure -ipaddr 10.10.10.11

4335sdiov clone 4335sdiov_bf -tag AARDVARK_BRANCH_6_30

4335sdiov_bf configure \
        -wlinitcmds {
            wl down ; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; wl mimo_bw_cap 1; wl ampdu_mpdu 24; wl vht_features 1; wl txbf 1; wl txbf_bfe_cap 1; wl txbf_bfr_cap 1; wl up
        }
4335sdiov_bf  configure -ipaddr 10.10.10.11


#c0 -type sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-autoabn-ampduhostreorder-ve-proxd-p2po-okc-txbf-err-assert.bin

4335sdio clone 4335sdio_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }

4335sdio_ifs configure -ipaddr 10.10.10.11


4335sdiov clone 4335sdiov_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }

4335sdiov_ifs configure -ipaddr 10.10.10.11


4335sdiov_ns clone 4335sdiov_ns_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }




##4335 p2p
#4335sdiov_ns clone 4335sdiov_ns-P2P \
        -sta { 4335sdiov_ns-p2p.1 wl0.1}
#4335sdiov_ns  clone 4335sdiov_ns-WLAN \
        -sta { 4335sdiov_ns-p2p eth0}


#static ip
#4335sdiov clone 4335sdiov-P2P \
        -sta { 4335sdiov-p2p.1 wl0.1}
#4335sdiov clone 4335sdiov-WLAN \
        -sta { 4335sdiov-p2p eth0}

}

####4354
###########################################
if { 1 } {
UTF::DHD mc97tst5 \
        -lan_ip mc97tst5 \
        -sta {4354 eth0 } \
        -power { mc97npc22_2 1}\
        -tcpwindow 2m \
        -udp 1.2g \
        -perfchans {36/80 6l}\
        -power_button "auto" \
        -tag  BISON_REL_7_10_82_*\
        -dhd_tag DHD_REL_1_137\
        -brand linux-external-dongle-sdio \
        -dhd_brand linux-external-dongle-sdio\
        -nvram bcm94354wlsagbl.txt \
        -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
        -type 4354a0-ram/sdio-ag-pktctx-dmatxrc-txbf-dbgtput-dbgam-phydbg.bin\
        -datarate {-b 1.2g -i 0.5 -frameburst 1} \
        -nocal 1 \
        -postinstall { dhd -i eth0 txglomsize 40 ; dhd -i eth0 sd_blocksize 2 256  } \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl dump rssi}} \
        -wlinitcmds {
            wl down; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; wl mimo_bw_cap 1; wl frameburst 1;  wl ampdu_mpdu 16; wl vht_features 3; wl ampdu_mpdu 24 ; wl ampdu_release 12 ; wl up
        }

# remove above  wl pool 100
# dhd -i eth0 download 4350c0-ram/sdio-ag-proptxstatus-pktctx-dmatxrc-txbf-dbgtput/rtecdc.bin bcm94350wlagbe_KA.txt   ?
# 1. 4354a0-ram/sdio-ag-pktctx-dmatxrc-txbf-dbgtput-dbgam-phydbg.bin
# 2. 4354a0-ram/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin
# 3 4354a0-ram/sdio-ag-pktctx-dmatxrc-txbf-dbgtput-dbgam-phydbg.bin 
    

4354 clone 4354_s
4354_s configure -ipaddr 10.10.10.11

# AARDVARK_TWIG_6_30_171
#4354 clone 4354sdio -tag  AARDVARK_TWIG_6_30_*

4354 clone 4354sdio_ns
#4354 clone 4354sdio -tag  BISON_REL_7_10_82_* -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-assert-err.bin    -dhd_tag NIGHTLY -dhd_brand linux-internal-dongle


4354 clone 4354sdio -tag  BISON_REL_7_10_82_* -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err.bin  -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio          

#old sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-assert-err.bin

#add authen
#  sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err.bin


4354sdio configure -ipaddr 10.10.10.11



#4354 clone 4354sdiotwig_ns -tag BISON_TWIG_7_10_* -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-assert-err.bin    -dhd_tag NIGHTLY -dhd_brand linux-internal-dongle


#4354 clone 4354sdiotwig_ns -tag BISON_TWIG_7_10_* -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-assert-err.bin -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio 

#comment above and up date the firmware
4354 clone 4354sdiotwig_ns -tag BISON_TWIG_7_10_* -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err.bin -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio

#SDIo-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err.bin



#update to wsec enabled type
#4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err

#4354 clone 4354sdiotwig_ns -tag BISON_TWIG_7_10_* -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err.bin -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio


4354sdiotwig_ns clone 4354sdiotwig 
4354sdiotwig configure -ipaddr 10.10.10.11


#
# 4354sdiotwig clone 4354sdiotwigmfg -brand linux-mfgtest-dongle-sdio -type 4354a1-roml/sdio-ag-mfgtest-seqcmds-txbf-sr-srfast-assert-err.bin -dhd_brand linux-internal-dongle 

4354sdiotwig clone 4354sdiotwigmfg -brand linux-mfgtest-dongle-sdio -type 4354a1-roml/sdio-ag-mfgtest-seqcmds-txbf-sr-srfast-assert-err.bin -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio
4354sdiotwigmfg configure -ipaddr 10.10.10.11
 
#4354 clone 4354bi -tag  BISON_BRANCH_7_10 -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-assert-err.bin -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio
#/projects/hnd/swbuild/build_linux/BISON_BRANCH_7_10/linux-external-dongle-sdio/2013.12.13.0/release/bcm/firmware/4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-assert-err.bin

#comment out above and update firewire  to test 
#4354 clone 4354bi -tag  BISON_BRANCH_7_10 -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err.bin -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio


#new 04/09
4354 clone 4354bi -tag  BISON_BRANCH_7_10 -type  4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-pwropt-idsup-idauth-assert-err.bin -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio

 
#-dhd_tag NIGHTLY -dhd_brand linux-internal-dongle

4354bi clone 4354bi_s
4354bi_s configure -ipaddr 10.10.10.11

#clone object for p2p test
4354bi_s clone 4354_sdio_p2p -sta { 4354bi_s_go eth0 4354bi_s_pgo  wl0.1}  
4354bi_s_go configure -ipaddr 10.10.10.11


#light weight just assert-err removed
#
4354bi_s clone 4354bis -tag  BISON_BRANCH_7_10 -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-okc-tdls-ccx-ve-mfp-ltecxgpio-pwropt-idsup-idauth.bin  -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio

#clone object for p2p test
4354bis clone 4354_sdio2_p2p -sta { 4354bis_go eth0 4354bis_pgo  wl0.1}
4354bis_go configure -ipaddr 10.10.10.11


4354bi clone 4354bis82rc22 -tag  BIS82RC22_BRANCH_7_21
4354bis82rc22 clone 4354bis82rc22_s
4354bis82rc22_s configure -ipaddr 10.10.10.11

#Caribou test branch

4354 clone 4354sdio_cari -tag CARIBOU_BRANCH_8_10 -type 4354a1-ram/sdio-ag-proptxstatus-ampduhostreorder-p2p-mchan-pktctx-assert-idsup-idauth-nopromisc.bin -dhd_tag DHD_BRANCH_1_141 -dhd_brand linux-external-dongle-sdio



4354sdio_cari configure -ipaddr 10.10.10.11

#  
4354sdio clone 4354sdio_nbf
4354sdio_nbf configure \
        -wlinitcmds {
            wl down ; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; wl mimo_bw_cap 1; wl frameburst 1;  wl ampdu_mpdu 16; wl vht_features 3;   wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl up
        }  -ipaddr 10.10.10.11


4354sdiotwig clone 4354sdiotwig_nbf  
4354sdiotwig_nbf configure \
        -wlinitcmds {
            wl down ; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; wl mimo_bw_cap 1; wl frameburst 1;  wl ampdu_mpdu 16; wl vht_features 3;  wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl up
        }  -ipaddr 10.10.10.11






4354sdio clone 4354sdio_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ;  wl frameburst 0;  wl ampdu_mpdu 16; wl vht_features 3;  wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }
4354sdio_ifs configure -ipaddr 10.10.10.11


4354sdiotwig clone 4354sdiotwig_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }
4354sdiotwig_ifs configure -ipaddr 10.10.10.11



}





##################pcie

####4354_2 (4356, pcie)
###########################################
if { 1 } {
UTF::DHD mc97tst3 \
        -lan_ip mc97tst3 \
        -sta {4356p eth0 } \
        -power { mc97npc22_5 2 }\
        -tcpwindow 2m \
        -udp 1.2g \
        -perfchans {36/80 6l}\
        -power_button "auto" \
        -tag  BISON_BRANCH_7_10\
        -dhd_tag NIGHTLY\
        -brand linux-external-dongle-pcie \
        -dhd_brand linux-external-dongle-pcie\
        -driver dhd-msgbuf-pciefd-debug\
        -nvram bcm94356wlpagbl.txt \
        -type 4356a2-roml/pcie-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx.bin\
        -datarate {-b 1.2g -i 0.5 -frameburst 1} \
        -nocal 1 \
        -wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }

#remove  wl mpc 0
4356p configure -ipaddr 10.10.10.18

4356p clone 4356p_735 -tag BISON05T_BRANCH_7_35 -brand linux-external-dongle-pcie -type 4356a2-roml/pcie-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth.bin\
-wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }




4356p clone 4356pmfg -tag BISON_BRANCH_7_10 -brand linux-mfgtest-dongle-pcie  -type 4356a2-roml/pcie-ag-msgbuf-pktctx-splitrx-txbf-splitbuf-mfgtest-seqcmds-sr-assert-err.bin\
-wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }
4356pmfg configure -ipaddr 10.10.10.18


4356p clone 4356pmfg_735 -tag BISON05T_BRANCH_7_35 -brand linux-mfgtest-dongle-pcie  -type 4356a2-roml/pcie-ag-msgbuf-pktctx-splitrx-txbf-splitbuf-mfgtest-seqcmds-sr-assert-err.bin\
-wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }
4356pmfg_735 configure -ipaddr 10.10.10.18





4356p clone 4356prel -tag BISON_REL_7_10_*  -brand linux-external-dongle-pcie
4356prel configure -ipaddr 10.10.10.18

#new target
#4354p clone 4354pcie  -type 4354a1-roml/pcie-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-mfp-idsup-idauth-assert.bin

#4354p clone 4354pcie  -type 4354a1-roml/pcie-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-mfp-amsdutx-idsup-idauth-assert.bin\
#-wlinitcmds {
#            wl down; wl msglevel +scan; wl btc_mode 0; wl vht_features 3 ; wl ampdu_release 12;  wl up
#        }


4356p clone 4356pcie  -type 4356a2-roml/pcie-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth-assert.bin\
-wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }
4356pcie configure -ipaddr 10.10.10.18


4356p clone 4356pcie_735 -tag  BISON05T_BRANCH_7_35 -type 4356a2-roml/pcie-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth-assert.bin\
-wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }
4356pcie_735 configure -ipaddr 10.10.10.18







#clone for p2p test 
4356pcie clone 4356_pcie_p2p -sta { 4356pcie_gc eth0 4356pcie_pgc  wl0.1}
4356pcie_gc configure -ipaddr 10.10.10.18



4356pcie clone 4356pcie2  -type 4356a2-roml/pcie-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth-assert.bin\
 -wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }



4356pcie_735 clone 4356pcie2_735  -type 4356a2-roml/pcie-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth-assert.bin\
 -wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }



#clone for p2p test
4356pcie2 clone 4356_pcie2_p2p -sta { 4356pcie2_gc eth0 4356pcie2_pgc  wl0.1}
4356pcie2_gc configure -ipaddr 10.10.10.18
#
#


4356pcie clone 4356pcie_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }


}



#####################4356 sdio
 
if { 1 } {
UTF::DHD mc97tst5 \
        -lan_ip mc97tst5 \
        -sta {4356s eth0 } \
        -power { mc97npc22_2 1 }\
        -tcpwindow 2m \
        -udp 1.2g \
        -perfchans {36/80 6l}\
        -power_button "auto" \
        -tag BISON_BRANCH_7_10\
        -dhd_tag NIGHTLY\
        -brand linux-external-dongle-sdio \
        -dhd_brand linux-external-dongle-sdio\
        -driver dhd-msgbuf-pciefd-debug\
        -nvram bcm94356wlsagbl.txt\
        -type 4356a2-roml/sdio-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx.bin\
        -datarate {-b 1.2g -i 0.5 -frameburst 1} \
        -nocal 1 \
        -wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }

#remove  wl mpc 0
4356s configure -ipaddr 10.10.10.18

4356s clone 4356s_735 -tag BISON05T_BRANCH_7_35 -brand linux-external-dongle-sdio -type 4356a2-roml/sdio-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth.bin\
-wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }




4356s clone 4356smfg -tag BISON_BRANCH_7_10 -brand linux-mfgtest-dongle-sdio  -type 4356a2-roml/sdio-ag-msgbuf-pktctx-splitrx-txbf-splitbuf-mfgtest-seqcmds-sr-assert-err.bin -wlinitcmds { wl down; wl msglevel +scan; wl btc_mode 0;  wl up }
4356smfg configure -ipaddr 10.10.10.18


4356s clone 4356smfg_735 -tag BISON05T_BRANCH_7_35 -brand linux-mfgtest-dongle-sdio  -type 4356a2-roml/sdio-ag-msgbuf-pktctx-splitrx-txbf-splitbuf-mfgtest-seqcmds-sr-assert-err.bin -wlinitcmds { wl down; wl msglevel +scan; wl btc_mode 0; wl up }
4356smfg_735 configure -ipaddr 10.10.10.18



4356s clone 4356srel -tag BISON_REL_7_10_*  -brand linux-external-dongle-sdio
4356srel configure -ipaddr 10.10.10.18

4356s clone 4356sdio  -type 4356a2-roml/sdio-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth-assert.bin\
-wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }
4356sdio configure -ipaddr 10.10.10.18


4356s clone 4356sdio_735 -tag  BISON05T_BRANCH_7_35 -type 4356a2-roml/sdio-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth-assert.bin\
-wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }
4356sdio_735 configure -ipaddr 10.10.10.18



#clone for p2p test 
4356sdio clone 4356_sdio_p2p -sta { 4356sdio_gc eth0 4356sdio_pgc  wl0.1}
4356sdio_gc configure -ipaddr 10.10.10.18


4356sdio clone 4356sdio2  -type 4356a2-roml/sdio-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth-assert.bin\
 -wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }


4356sdio_735 clone 4356sdio2_735  -type 4356a2-roml/pcie-ag-msgbuf-splitrx-pktctx-aoe-p2p-pno-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-okc-tdls-ccx-ve-amsdutx-idsup-idauth-assert.bin\
 -wlinitcmds {
            wl down; wl msglevel +scan; wl btc_mode 0;  wl up
        }



#clone for p2p test
4356sdio2 clone 4356_sdio2_p2p -sta { 4356sdio2_gc eth0 4356sdio2_pgc  wl0.1}
4356sdio2_gc configure -ipaddr 10.10.10.18
#
#


4356sdio clone 4356sdio_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }


}




#########endof 4354



if { 0 } {
# -power {npctst1 1} 
# move 4313 to mc97tst1
# from mc97tst5usb
#         -tcpwindow 1152K 
UTF::DHD mc97tst1 \
        -lan_ip mc97tst1 \
        -sta {43143b0 eth0} \
          -udp 1.2g\
                -tcpwindow 2m \
        -console "mc97end1:40004" \
        -power { mc97npc22_2 2}\
        -power_button "auto" \
        -brand linux-internal-dongle-usb \
        -dhd_brand linux-internal-dongle-usb \
        -driver dhd-cdc-usb-gpl \
        -tag PHOENIX2_BRANCH_6_10\
        -dhd_tag  NIGHTLY\
        -nvram "src/shared/nvram/bcm943143usbirdsw_p700.txt" \
         -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-pclose-proptxstatus-assert.bin.trx" \
        -wlinitcmds {wl msglevel +assoc; wl down; wl mimo_bw_cap 1} \
        -perfchans {6l 6} -nocal 1 -slowassoc 5 -noframeburst 1 \
        -datarate {-skiptx 0x3-9x3 -skiprx 0x3-9x3} \
        -nobighammer 1 \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}} \
        -msgactions {{dbus_usb_dl_writeimage: Bad Hdr or Bad CRC} FAIL} \
        -nvram_add {
            # Disable noise cal (PR#103469 PR#103468)
            noise_cal_enable_2g=0
            noise_cal_enable_5g=0
        }

#






43143b0 clone  43143b0_s 
43143b0_s configure -ipaddr 10.10.10.12

43143b0_s clone 43143b0s_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }


#


#43143 nightly
43143b0 clone 43143b0n_s -tag NIGHTLY
43143b0n_s configure -ipaddr 10.10.10.12

#twig

#43143b0 clone 43143b0twig_s -tag  AARDVARK01T_TWIG_6_37_14 -dhd_tag aARDVARK01T_TWIG_6_37_14 -brand linux-internal-wl-media -dhd_brand linux-internal-wl-media
#43143b0twig_s configure -ipaddr 10.10.10.12


 #-subdir "release/firmware" 

}

if { 0 } {
UTF::DHD mc97tst1usb22 \
        -lan_ip mc97tst1 \
        -sta { 43143p2pbmac eth0 43143p2pbmac.0 wl0.1 } \
        -udp 1.2g\
        -tcpwindow 2m \
        -console "mc97end1:40004" \
        -power { mc97npc22_2 2}\
        -power_button "auto" \
        -brand linux-internal-wl-media \
        -dhd_brand linux-internal-wl-media \
        -driver dhd-cdc-usb-gpl \
        -tag AARDVARK01T_TWIG_6_37_14\
        -dhd_tag AARDVARK01T_TWIG_6_37_14\
        -nvram "src/shared/nvram/bcm943143usbirdsw_p700.txt" \
         -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-pclose-proptxstatus-assert.bin.trx" \
        -wlinitcmds {wl msglevel +assoc; wl down; wl mimo_bw_cap 1} \
        -perfchans {6l 6} -nocal 1 -slowassoc 5 -noframeburst 1 \
        -datarate {-skiptx 0x3-9x3 -skiprx 0x3-9x3} \
        -nobighammer 1 \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}} \
        -msgactions {{dbus_usb_dl_writeimage: Bad Hdr or Bad CRC} FAIL} \
        -nvram_add {
            # Disable noise cal (PR#103469 PR#103468)
            noise_cal_enable_2g=0
            noise_cal_enable_5g=0
        }

}

if { 1 } {
#-console "mc97end1:40004" 
UTF::DHDUSB mc97tst1 \
    -lan_ip mc97tst1 \
    -udp 1.2g\
    -tcpwindow 2m \
    -sta "43143p2pbmac eth0 43143p2pbmac.0 wl0.1" \
    -power { mc97npc22_2 2}\
    -power_button auto 	\
    -perfchans {6l 6}\
    -brand linux-internal-wl-media \
    -subdir "release/firmware" \
    -type "43143b0-bmac/g-assert-p2p-mchan-media"\
    -file "rtecdc.bin.trx" \
    -wlinitcmds {wl down; wl mpc 0; wl up} \
    -dhd_tag  "AARDVARK01T_TWIG_6_37_14" \
    -dhd_brand "linux-internal-wl-media"\
    -dhd_subdir "release"\
    -dhd_type obj-debug-apdef-stadef-high-p2p-mchan-tdls-media-2.6.38.6-26.rc1 \
    -dhd_file "wl.ko" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -wl_brand "linux-internal-wl-media"\
    -nvram_image "/projects/hnd/software/gallery/src/shared/nvram/bcm943143usbirdsw_p700.txt"\
    -wl_subdir release/exe \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S wl dump txbf} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}\
    -nocal 1


43143p2pbmac clone  43143p2pbmac_s
43143p2pbmac_s configure -ipaddr 10.10.10.12


43143p2pbmac clone 43143p2pbmac_vs -tag AARDVARK_BRANCH_6_30 
43143p2pbmac_vs configure -ipaddr 10.10.10.12

43143p2pbmac_s clone 43143p2pbmac_s_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }
43143p2pbmac_s_ifs configure -ipaddr 10.10.10.12


#clone bison
43143p2pbmac clone 43143bmac_s -tag BISON_BRANCH_7_10 -dhd_tag AARDVARK01T_TWIG_6_37_14 -wl_brand "linux-internal-wl"  -type "43143b0-bmac/g-assert-p2p-mchan" -brand "linux-internal-wl"\
    -dhd_tag  "BISON_BRANCH_7_10" \
    -dhd_brand "linux-internal-wl"\
    -dhd_subdir "release"\
    -dhd_type obj-debug-apdef-stadef-high-p2p-mchan-tdls-2.6.38.6-26.rc1 \
    -dhd_file "wl.ko" 


43143bmac_s configure -ipaddr 10.10.10.12


43143bmac_s clone 43143bmac_s_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }
43143bmac_s_ifs configure -ipaddr 10.10.10.12


}

#

##
# on E4200 , small labtop
# -power {npctst1 1} 
# move 4313 to mc97tst1
# from mc97tst5usb mc97npc22_5
#older one
 #  -power_sta "hub 1"
if { 0 } {
UTF::DHD mc97tst3 \
        -lan_ip mc97tst3 \
        -sta {43143b0_2 eth0} \
        -udp 1.2g\
        -tcpwindow 2m \
        -console "mc97end1:40005" \
        -power_button "auto" \
        -power { mc97npc22_5 2} \
        -brand linux-internal-dongle-usb \
        -dhd_brand linux-internal-dongle-usb \
        -driver dhd-cdc-usb-gpl \
        -tag PHOENIX2_BRANCH_6_10 \
        -dhd_tag NIGHTLY \
        -nvram "src/shared/nvram/bcm943143usbirdsw_p700.txt" \
        -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-pclose-proptxstatus-assert.bin.trx" \
        -wlinitcmds {wl msglevel +assoc; wl down; wl mimo_bw_cap 1} \
        -perfchans {6l 6} -nocal 1 -slowassoc 5 -noframeburst 1 \
        -datarate {-skiptx 0x3-9x3 -skiprx 0x3-9x3} \
        -nobighammer 1 \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}} \
        -msgactions {{dbus_usb_dl_writeimage: Bad Hdr or Bad CRC} FAIL} \
        -nvram_add {
            # Disable noise cal (PR#103469 PR#103468)
            noise_cal_enable_2g=0
            noise_cal_enable_5g=0
        }

#    previous    -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-proptxstatus-assert.bin.trx" 

43143b0_2 clone  43143b0_2s 
43143b0_2s configure -ipaddr 10.10.10.13

43143b0_2s clone 43143b0_2s_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }


#43143b0_2 clone 43143b0_2-P2P \
#        -sta { 43143b0_2-p2p.1 wl0.1}
#43143b0_2 clone 43143b0_2-WLAN \
#        -sta { 43143b0_2-p2p eth0}



############################
#  -nvram "src/shared/nvram/bcm943143usbirdsw_p700.txt" 

# either -nvram_image "/projects/hnd/software/gallery/src/shared/nvram/bcm943143usbirdsw_p700.txt"\

# or    -nvram_subdir src/shared/nvram \
    -nvram_file  bcm943143usbirdsw_p700.txt\
#
}

if { 0 } {
#-console "mc97end1:40005" 
UTF::DHDUSB mc97tst3dhdusb \
    -lan_ip mc97tst3 \
        -udp 1.2g\
        -tcpwindow 2m \
    -sta "43143p2pbmac_2 eth0 43143p2pbmac_2.0 wl0.1" \
    -power { mc97npc22_5 2}\
    -power_button auto \
     -perfchans {6l 6}\
    -brand linux-internal-wl-media \
    -subdir "release/firmware" \
    -type "43143b0-bmac/g-assert-p2p-mchan-media"\
    -file "rtecdc.bin.trx" \
    -wlinitcmds {wl down; wl mpc 0; wl up} \
    -dhd_tag  "AARDVARK01T_TWIG_6_37_14" \
    -dhd_brand "linux-internal-wl-media"\
    -dhd_subdir "release"\
    -dhd_type obj-debug-apdef-stadef-high-p2p-mchan-tdls-media-2.6.38.6-26.rc1 \
    -dhd_file "wl.ko" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -wl_brand "linux-internal-wl-media"\
    -nvram_image "/projects/hnd/software/gallery/src/shared/nvram/bcm943143usbirdsw_p700.txt"\
    -wl_subdir release/exe \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S wl dump txbf} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}\
    -nocal 1


43143p2pbmac_2 clone  43143p2pbmac_2s
43143p2pbmac_2s configure -ipaddr 10.10.10.13




43143p2pbmac_2 clone 43143p2pbmac_2vs -tag AARDVARK_BRANCH_6_30 -dhd_tag AARDVARK_BRANCH_6_30
43143p2pbmac_2vs configure -ipaddr 10.10.10.13


#clone bison
43143p2pbmac_2 clone 43143bmac_2s -tag BISON_BRANCH_7_10 -dhd_tag AARDVARK01T_TWIG_6_37_14 -wl_brand "linux-internal-wl"  -type "43143b0-bmac/g-assert-p2p-mchan" -brand "linux-internal-wl"\
    -dhd_tag  "BISON_BRANCH_7_10" \
    -dhd_brand "linux-internal-wl"\
    -dhd_subdir "release"\
    -dhd_type obj-debug-apdef-stadef-high-p2p-mchan-tdls-2.6.38.6-26.rc1 \
    -dhd_file "wl.ko"
43143bmac_2s configure -ipaddr 10.10.10.13


#43143p2pbmac_2 clone 43143p2pbma_2-P2P \
#        -sta {43143p2pbmac_2-p2p.1 wl0.1}
#43143p2pbmac_2 clone 43143p2pbmac_2-WLAN \
#        -sta {43143p2pbmac_2-p2p eth0}

}
####



# -console "mc97end1:40004" 
if { 0 } {
UTF::DHDUSB mc97tst1dhdusb2 \
    -lan_ip mc97tst1 \
        -udp 1.2g\
      -tcpwindow 2m \
    -sta "43236p2pbmac eth0 43236p2pbmac.0 wl0.1" \
    -power { mc97npc22_2 2}\
    -power_button auto \
    -brand linux-internal-wl-media \
    -subdir "release/firmware" \
    -type "43236b-bmac/ag-assert-p2p-mchan-media"\
    -file "rtecdc.bin.trx" \
    -wlinitcmds {wl down; wl mpc 0; wl up} \
    -dhd_tag  "AARDVARK01T_TWIG_6_37_14" \
    -dhd_brand "linux-internal-wl-media"\
    -dhd_subdir "release"\
    -dhd_type "obj-debug-apdef-stadef-high-p2p-mchan-tdls-media-2.6.38.6-26.rc1"\
    -dhd_file "wl.ko" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -wl_brand "linux-internal-wl-media"\
    -wl_subdir release/exe\
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S wl dump txbf} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}\
    -nocal 1



43236p2pbmac clone  43236p2pbmac_s
43236p2pbmac_s configure -ipaddr 10.10.10.12

43236p2pbmac clone 43236p2pbmac_vs -tag AARDVARK_BRANCH_6_30 -dhd_tag AARDVARK_BRANCH_6_30
43236p2pbmac_vs configure -ipaddr 10.10.10.12

}



#############
if { 1  } {
UTF::Power::WebRelay webrelay1 -lan_ip 172.19.16.112 -invert 1 -relay mc97end1 
UTF::Power::Laptop X52_power -button {webrelay1 4} 
UTF::MacOS mc97tst4 \
           -sta { X52 en0 } \
           -udp 1.2g\
           -tcpwindow 2m \
           -power {X52_power}\
           -brand  "macos-internal-wl-ml" \
           -wlinitcmds {wl msglevel 0x101 wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 1 } \
           -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
           -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
           -type Debug_10_8 \
           -coreserver AppleCore \
           -kextload true \
           -perfchans "36/80 36l 36 6"

#TOT
X52 clone X52_s 
X52_s configure -ipaddr 10.10.10.14

#BISON_BRANCH_7_10
X52 clone X52bi -tag BISON_BRANCH_7_10
X52 clone X52bi_s -tag BISON_BRANCH_7_10
X52bi_s configure -ipaddr 10.10.10.14


X52bi_s clone X52bis_p2p -sta { X52bis_go en0 X52bis_pgo  p2p0}
X52bis_go configure -ipaddr 10.10.10.14



X52 clone X52v -tag "AARDVARK_BRANCH_6_30"
X52v clone X52v_s -tag "AARDVARK_BRANCH_6_30"
X52v_s configure -ipaddr 10.10.10.14


X52v clone X52v_rel -tag "AARDVARK_REL_6_30_223_*"
X52v clone X52v_rels -tag "AARDVARK_REL_6_30_223_*"
X52v_rels configure -ipaddr 10.10.10.14


X52v clone X52_rels -tag "BU4360B1_REL_6_30_227_*"

X52_rels configure -ipaddr 10.10.10.14




X52bi_s clone X52bi_s_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }
X52bi_s_ifs configure -ipaddr 10.10.10.14


X52bi_s clone X52bi_snbf
X52bi_snbf configure \
        -wlinitcmds {
            wl down ; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl vht_features 1 ; wl frameburst 1; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl assert_type 1 ; wl up
        }  -ipaddr 10.10.10.14



}

##########
###win8_64
#  -node {DEV_43A0}\
# -sta {4360Win8_64 00:10:18:A9:83:F1 }\
#         -brand win8_internal_wl\
#
UTF::Cygwin mc97tst8 \
        -osver 864 \
       -sta {4360Win8_64 00:10:18:A9:83:F1 }\
  -node {DEV_43A0}\
      -brand win8_internal_wl\
          -perfchans {36/80 36l 6l 6}\
        -user user \
        -installer inf\
        -power  {mc97npc22_6 1}\
        -power_button {auto}




4360Win8_64 clone 4360Win8_64v -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl
4360Win8_64 clone 4360Win8_64bi -tag BISON_BRANCH_7_10 -brand win8_internal_wl

4360Win8_64 clone 4360Win8_64bi -tag BISON_BRANCH_7_10 -brand win8_internal_wl

4360Win8_64v clone 4360Win8_64vs
4360Win8_64vs configure -ipaddr 192.168.1.223


4360Win8_64v clone 4360Win8_64v_s
4360Win8_64v_s configure -ipaddr 192.168.1.223


4360Win8_64bi clone 4360Win8_64bis
4360Win8_64bis configure -ipaddr 192.168.1.223



4360Win8_64 clone 4360Win8_64s
4360Win8_64s configure -ipaddr  192.168.1.223

UTF::Cygwin mc97tst6_1 \
        -lan_ip mc97tst6\
        -osver 864 \
        -user user \
        -sta {43142Win8_64 E4:D5:3D:DF:29:4B}\
        -node {DEV_4365}\
        -installer inf\
        -brand win8_internal_wl\
        -tag AARDVARK_REL_6_30_59_44 \
        -power_button {auto}

43142Win8_64 clone 43142Win8_64v -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl
43142Win8_64v clone 43142Win8_64vs
43142Win8_64vs configure -ipaddr 192.168.1.92

43142Win8_64 clone 43142Win8_64n -tag NIGHTLY -brand win8_internal_wl

43142Win8_64 clone 43142Win8_64r -tag RUBY_BRANCH_6_20 -brand win8_internal_wl







#
if { 0 } {
UTF::Router AP2 \
        -sta "4717_2 eth1" \
        -lan_ip 192.168.1.2 \
        -lanpeer lan \
        -relay "mc97end1" \
        -console "mc97end1:40003" \
        -power { mc92npc22_4 1 } \
        -brand linux-external-router \
        -tag "AKASHI_REL_5_110_65" \
        -nvram {
               # et0macaddr=00:01:36:22:7f:2d
               # macaddr=00:90:4C:07:00:3a
                lan_ipaddr=192.168.1.2
                lan_gateway=192.168.1.2
                dhcp_start=192.168.1.120
                dhcp_end=192.168.1.130
                lan1_ipaddr=192.168.2.2
                lan1_gateway=192.169.2.2
                dhcp1_start=192.168.2.120
                dhcp1_end=192.168.2.130
                fw_disable=1
                #router_disable=1
                wl_msglevel=0x101
                wl0_ssid=test4717_2
                wl0_channel=1
                wl0_radio=1
               # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                wl0_obss_coex=0
                lan_proto=dhcp
                lan_dhcp=0
        }

4717_2 configure -attngrp G2

}


#
if { 0 } {
UTF::Router AP1 \
        -sta "4706/4360 eth1" \
        -lan_ip 192.168.1.3 \
        -lanpeer lan \
        -perfchans {36/80 36l 36}\
        -relay "mc97end1" \
        -console "mc97end1:40006" \
        -power { mc97npc22_7 1 } \
        -brand linux26-internal-router\
        -tag "AARDVARK_BRANCH_6_30" \
        -nvram {
               # et0macaddr=00:01:36:22:7f:2d
               # macaddr=00:90:4C:07:00:3a
                lan_ipaddr=192.168.1.3
                lan_gateway=192.168.1.3
                dhcp_start=192.168.1.120
                dhcp_end=192.168.1.130
                lan1_ipaddr=192.168.2.3
                lan1_gateway=192.169.2.3
                dhcp1_start=192.168.2.120
                dhcp1_end=192.168.2.130
                fw_disable=1
                #router_disable=1
                wl_msglevel=0x101
                wl0_ssid=4706and4360
                wl0_channel=1
                 wl0_radio=1
                 wl1_radio=0
               # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                wl0_obss_coex=0
                lan_proto=dhcp
                lan_dhcp=0
        }

4706/4360 configure -attngrp G3
}
