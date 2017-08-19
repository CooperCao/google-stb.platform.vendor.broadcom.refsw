
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::utils

package require utfconf
package provide UTF::WebRelay 

package require UTF::Airport

set UTF::DataRatePlot 1
set ::UTF::Gnuplot "/usr/bin/gnuplot"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext9/$::env(LOGNAME)/mc97"

set ::UTF::SummaryDir "/projects/hnd_sig_ext15/$::env(LOGNAME)/mc97_2"

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
    -relay "mc97end1" -group {  G2 {4 5 6} G3 {7 8 9} ALL {4 5 6 7 8 9} }


#UTF::Power::WebRelay hub -lan_ip 172.19.16.112 -relay "mc97end1"
#Attenuator - Aeroflex
#UTF::Aeroflex af -lan_ip 172.19.16.10 -relay "mc97end1" -group { G1 {1 2 3 } G2 {4 5 6} G3 {7 8 9} ALL {1 2 3 4 5 6 7 8 9} } 

set ::UTF::SetupTestBed {
    package require UTF::Test::RebootTestbed
    UTF::Try "Reboot Testbed" {
    UTF::Test::RebootTestbed -hostlist "4360Win8_64bi  4360FC15bi snif1" 
    }
     catch { snif1 load }
     catch { snif1 wl down }
     catch { 4717_2 wl down }
     catch { 4709/4360g wl down }
     catch { 4709/4360a wl down }
    # af setGrpAttn G1 0
     af setGrpAttn G2 0
      af setGrpAttn G3 20
   # unset ::UTF::SetupTestBed  
    return
}

#AP1 and AP2
set  ::src_apmac_index "wl0_hwaddr"
set  ::src_ap_ssid_index "wl0_ssid"
set  ::dst_apmac_index "wl0_hwaddr"
set  ::dst_ap_ssid_index "wl0_ssid"

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
UTF::Sniffer mc97tst9 \
     -sta  {snif1 eth0} \
     -power { mc97npc22_3 1} \
     -power_button {auto}\
     -tag  AARDVARK_BRANCH_6_30\
     -date 2013.9.10.0


#   KIRIN_REL_5_100_139
#     -header radiotap
#RUBY_BRANCH_6_20
#AARDVARK_BRANCH_6_30
#RUBY_BRANCH_6_20

###
if { 0 } {
UTF::Linux mc97tst7 \
        -sta "4360FC15ap0 eth0"\
        -power { mc97npc22_1 1} \
        -power_button {auto}\
        -wlinitcmds {wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 }\
        -brand linux-internal-wl\
        -perfchans {36/80 36l 36 6l 6}\
        -tag AARDVARK_BRANCH_6_30\
        -type obj-debug-apdef-stadef-high

#debug-p2p-mchan
4360FC15ap0 clone 4360FC15v -perfchans { 6l 6} -tag AARDVARK_BRANCH_6_30
# -lanpeer mc97tst7 -host mc97tst7
4360FC15ap0 configure -ipaddr 10.10.10.7 -ap 1 -attngrp G1 -lanpeer mc97tst7 

4360FC15ap0 clone 4360FC15apv -tag AARDVARK_BRANCH_6_30 -type obj-debug-apdef-stadef
4360FC15apv configure -ipaddr 10.10.10.7 -ap 1 -attngrp G1 -wlinitcmds { wl down; wl -u txbf_bfe_cap 1 ; wl -u txbf_bfr_cap 1; wl -u txbf 1 ; wl up }

#AARDVARK_TWIG_6_30_163 --AARDVARK_TWIG_6_30_234
4360FC15ap0 clone 4360FC15ap_twig -tag AARDVARK_TWIG_6_30_* -type obj-debug-apdef-stadef
4360FC15ap_twig configure -ipaddr 10.10.10.7 -ap 1 -attngrp G1 -wlinitcmds { wl down; wl -u txbf_bfe_cap 1 ; wl -u txbf_bfr_cap 1; wl -u txbf 1 ; wl up }


4360FC15ap_twig clone 4360FC15ap_twig_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }


}

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
        -tcpwindow 1152K \
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
            wl down; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; wl mimo_bw_cap 1; wl ampdu_mpdu 24; wl vht_features 1; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0 ; wl up
        }

4335b0 clone 4335b0_s
4335b0_s configure -ipaddr 10.10.10.11

# AARDVARK_TWIG_6_30_171
4335b0 clone 4335sdio -tag  AARDVARK_TWIG_6_30_*

#-type 4350b0-roml/sdio-ag-pool-p2p-idsup-idauth-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-assert-err-txbf.bin

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






}

#


UTF::Linux mc97tst1 \
        -sta "3213 eth0"\
        -power { mc97npc22_2 2}\


# -power {npctst1 1} 
# move 4313 to mc97tst1
# from mc97tst5usb
#         -tcpwindow 1152K 
UTF::DHD mc97tst1usb \
        -lan_ip mc97tst1 \
        -sta {43143b0 eth0} \
                -tcpwindow 3600K \
        -console "mc97end1:40004" \
        -power { mc97npc22_2 2}\
        -power_button "auto" \
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
if { 0 } {
UTF::DHD mc97tst1usb22 \
        -lan_ip mc97tst1 \
        -sta { 43143p2pbmac eth0 43143p2pbmac.0 wl0.1 } \
        -tcpwindow 1152K \
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
UTF::DHDUSB mc97tst1dhdusb \
    -lan_ip mc97tst1 \
    -tcpwindow 2m \
    -udp 1.2g\
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
}

43143p2pbmac clone  43143p2pbmac_s
43143p2pbmac_s configure -ipaddr 10.10.10.12


43143p2pbmac clone 43143p2pbmac_vs -tag AARDVARK_BRANCH_6_30 -dhd_tag AARDVARK_BRANCH_6_30
43143p2pbmac_vs configure -ipaddr 10.10.10.12



#
UTF::Linux mc97tst3 \
        -sta "none eth0"\
        -power { mc97npc22_5 2}\

##
# on E4200 , small labtop
# -power {npctst1 1} 
# move 4313 to mc97tst1
# from mc97tst5usb mc97npc22_5
#older one
 #  -power_sta "hub 1"
UTF::DHD mc97tst3 \
        -lan_ip mc97tst3 \
        -sta {43143b0_2 eth0} \
        -tcpwindow 3600K \
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



############################
#  -nvram "src/shared/nvram/bcm943143usbirdsw_p700.txt" 

# either -nvram_image "/projects/hnd/software/gallery/src/shared/nvram/bcm943143usbirdsw_p700.txt"\

# or    -nvram_subdir src/shared/nvram \
    -nvram_file  bcm943143usbirdsw_p700.txt\
#

if { 1 } {
#-console "mc97end1:40005" 
UTF::DHDUSB mc97tst3dhdusb \
    -lan_ip mc97tst3 \
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
}

43143p2pbmac_2 clone  43143p2pbmac_2s
43143p2pbmac_2s configure -ipaddr 10.10.10.13




43143p2pbmac_2 clone 43143p2pbmac_2vs -tag AARDVARK_BRANCH_6_30 -dhd_tag AARDVARK_BRANCH_6_30
43143p2pbmac_2vs configure -ipaddr 10.10.10.13




####



# -console "mc97end1:40004" 
if { 0 } {
UTF::DHDUSB mc97tst1dhdusb2 \
    -lan_ip mc97tst1 \
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
UTF::Power::WebRelay webrelay1 -lan_ip 172.19.16.112 -invert 1 -relay mc97end1 
UTF::Power::Laptop X52_power -button {webrelay1 4} 
UTF::MacOS mc97tst4 \
           -sta { X52 en0 X52.1 p2p0 } \
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

X52 clone X52v -tag "AARDVARK_BRANCH_6_30"
X52v clone X52v_s -tag "AARDVARK_BRANCH_6_30"
X52v_s configure -ipaddr 10.10.10.14


X52v clone X52v_rel -tag "AARDVARK_REL_6_30_223_*"
X52v clone X52v_rels -tag "AARDVARK_REL_6_30_223_*"
X52v_rels configure -ipaddr 10.10.10.14


X52v clone X52_rels -tag "BU4360B1_REL_6_30_227_*"

X52_rels configure -ipaddr 10.10.10.14



X52v_s clone X52v_s_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }
X52v_s_ifs configure -ipaddr 10.10.10.14



##########
#        -osver 764 \
#

UTF::Cygwin mc97tst6 \
        -osver 7 \
        -user user \
        -sta { 4360Win7  00:10:18:A9:83:D1 } \
        -node {DEV_43A0}\
        -tag "NIGHTLY"\
        -installer inf\
        -power_button {auto}
#        -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0 }
#       -console "mc57end1:40004"

4360Win7  clone  4360Win7bi -tag BISON_BRANCH_7_10
####
###win8_64
#  -node {DEV_43A0}\
# -sta {4360Win8_64 00:10:18:A9:83:F1 }\
#         -brand win8_internal_wl\
#
UTF::Cygwin mc97tst8 \
        -osver 864 \
        -sta {4360Win8_64 00:10:18:A9:83:F1 }\
        -node {DEV_43A0}\
        -tcpwindow 2m \
        -udp 1.2g\
        -brand win8x_internal_wl\
        -perfchans {36/80 6l }\
        -user user \
        -installer inf\
        -power  {mc97npc22_6 1}\
        -power_button {auto}

4360Win8_64 clone 4360Win8_64v -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl
4360Win8_64 clone 4360Win8_64bi -tag BISON_BRANCH_7_10 -brand win8x_internal_wl

4360Win8_64v clone 4360Win8_64vs
4360Win8_64vs configure -ipaddr 192.168.1.223


4360Win8_64v clone 4360Win8_64v_s
4360Win8_64v_s configure -ipaddr 192.168.1.223


4360Win8_64bi clone 4360Win8_64bis
4360Win8_64bis configure -ipaddr 192.168.1.223



4360Win8_64 clone 4360Win8_64s
4360Win8_64s configure -ipaddr  192.168.1.223
if { 0 } {
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

}


#4313on XP sp3
if { 1 } {
UTF::Cygwin mc71tst2 \
        -user user \
        -sta {4313XP} \
        -installer inf\
             -udp 1.2g\
                -tcpwindow 2m \
        -power_button {auto}\
         -datarate {-b 1.2g -i 0.5 -frameburst 1} \
         -nocal 1

4313XP clone 4313XPbi -tag "BISON_BRANCH_7_10"

4313XPbi clone 4313XPbis
4313XPbis configure -ipaddr 192.168.1.205

}





#X29
UTF::Linux mc97sta1 \
        -sta "4360FC15v eth0"\
         -tcpwindow 2m \
         -udp 1.2g\
        -power { mc97npc22_6 2} \
        -power_button {auto}\
        -wlinitcmds {wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 }\
        -brand linux-internal-wl\
        -perfchans {36/80 6}\
        -tag AARDVARK_BRANCH_6_30\
        -type debug-p2p-mchan


4360FC15v clone 4360FC15bi -tag BISON_BRANCH_7_10 
4360FC15v clone 4360FC15n -tag NIGHTLY
4360FC15bi clone 4360FC15bis
4360FC15bis configure -ipaddr 192.168.1.224 


4360FC15bi clone 4360FC15bi_ifs -datarate {-b 350m -i 0.5 -frameburst 0 -skiptx (1-48)  -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }

#

#AP2 4717_2 -> AP1 4709/4360a
#roam time test preconfig , as a regular roamming

set  ::src_apmac_index "macaddr"
set  ::src_ap_ssid_index "wl0_ssid"

set  ::dst_apmac_index "wl1_hwaddr"
set  ::dst_ap_ssid_index "wl1_ssid"




if { 1 } {
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
                macaddr=00:90:4C:07:00:2a
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
                wl0_radio=0
               # Used foR RSSI -35 to -45 TP Variance
                antswitch=0
                wl0_obss_coex=0
                lan_proto=static
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
        -perfchans "36/80 36l 36"\
        -brand linux26-internal-router\
        -tag "AARDVARK_BRANCH_6_30" \
        -nvram {
                et0macaddr=00:90:4c:01:22:6f
                wl0_hwaddr=00:90:4C:0F:50:9A 
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
                wl0_ssid=test4717_2
                wl0_channel=36
                 wl0_radio=0
                 wl1_radio=0
               # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                wl0_obss_coex=0
                lan_proto=static
                lan_dhcp=0
        }

4706/4360 configure -attngrp G3
}



if { 1 } {
UTF::Router AP1 \
        -sta "4709/4360a eth2 4709/4360g eth1 " \
        -lan_ip 192.168.1.3 \
        -lanpeer lan \
        -relay "mc97end1" \
        -console "mc97end1:40006" \
        -power { mc97npc22_7 1 } \
        -brand linux-2.6.36-arm-internal-router\
        -tag "BISON_BRANCH_7_10" \
        -nvram {
              #  et0macaddr=00:90:4C:11:20:28
              #  wl0_hwaddr=00:90:4C:0F:50:9A 
                 lan_ipaddr=192.168.1.3
                 lan_gateway=192.168.1.3
             #   dhcp_start=192.168.1.120
             #   dhcp_end=192.168.1.130
             #   lan1_ipaddr=192.168.2.3
             #   lan1_gateway=192.169.2.3
             #   dhcp1_start=192.168.2.120
             #   dhcp1_end=192.168.2.130
             #   fw_disable=1
             #   #router_disable=1
             #   wl_msglevel=0x101
              # g band 
                wl0_ssid=4709_4360g
                wl0_channel=36
                 wl0_radio=0
              #a band  
               wl1_radio=0
               wl1_ssid=4709_4360a
               # Used for RSSI -35 to -45 TP Variance
              #  antswitch=0
                wl0_obss_coex=0
                wl1_obss_coex=0
                lan_proto=static
                lan_dhcp=0
                lan1_proto=static
                lan1_dhcp=0
                qos_enable=1
              
        }

4709/4360a configure -attngrp G3 
#4709/4360a configure -perfchans "36/80 36l 36"

4709/4360g configure -attngrp G3 
#4709/4360g configure -perfchans "6l 6"

}

