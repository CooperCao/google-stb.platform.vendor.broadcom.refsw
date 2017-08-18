# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: cc5b057be1fd43f9d123f3c0fa929fd994de43e9 $}

package require UTF::Linux

set sta_type "Fc19_43342"
set ctrl_ip 10.19.102.24
set pwr_ip 10.19.102.131
set pwr_usb1_ip 10.19.102.131
source [file dirname [info script]]/Pod.tcl

# AP.
package require UTF::StaticAP
UTF::StaticAP %AUTO% \
    -sta {
       Ap_5 {
       -ssid smoke5
       -security open
       }
       Ap_2 {
       -ssid smoke
       -security open
       }
}

# STAs.
UTF::DHD STA_proto\
    -sta "$sta_type eth0"\
    -user root\
    -dhd_brand linux-external-dongle-sdio \
    -nvram "bcm943342SouthpawUSIK_ES5.1.txt" \
    -clm_blob 43342_southpaw1E1H_R-220.clm_blob \
    -brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
    -type 43342a0-roml/sdio-g-p2p-pno-nocis-keepalive-aoe-idsup-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-noclminc-logtrace-srscan-mpf-pmmt-frag_dur-clm_min-sr-dsprot-mfp-proxd/rtecdc.bin \
    -wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port power_usb usb_port mac
    1 10.19.102.98 40000 PowerUSB1 1 "00:90:4c:c5:12:39"
    2 10.19.102.99 40001 PowerUSB1 2 "00:90:4c:c5:12:40"
    3 10.19.102.100 40002 PowerUSB1 3 "00:90:4c:c5:12:41"
    4 10.19.102.101 40003 PowerUSB1 4 "00:90:4c:c5:12:42"
    5 10.19.102.102 40004 PowerUSB1 5 "00:90:4c:c5:12:43"
    6 10.19.102.103 40005 PowerUSB1 6 "00:90:4c:c5:12:44"
}

set_stas_dhd_nocons
