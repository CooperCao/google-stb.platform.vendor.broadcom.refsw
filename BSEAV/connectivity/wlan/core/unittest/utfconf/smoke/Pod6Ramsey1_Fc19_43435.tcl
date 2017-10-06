# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "Fc19_43435"
set ctrl_ip 10.19.91.23
set pwr_ip 10.19.91.74
set pwr_usb1_ip 10.19.91.74
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
    -nvram "bcm943430fsdnge_elna_Bx.txt" \
    -clm_blob 43430b0.clm_blob  \
    -brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
    -type 43430b0-roml/config_sdio_release/rtecdc.bin \
    -wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port power_usb usb_port mac
    1 10.19.91.43 42000 PowerUSB1 1 "00:90:4c:c5:12:38"
    2 10.19.91.44 42001 PowerUSB1 2 "00:90:4c:c5:12:39"
    3 10.19.91.45 42002 PowerUSB1 3 "00:90:4c:c5:12:40"
    4 10.19.91.46 42003 PowerUSB1 4 "00:90:4c:c5:12:41"
    5 10.19.91.47 42004 PowerUSB1 5 "00:90:4c:c5:12:42"
    6 10.19.91.48 42005 PowerUSB1 6 "00:90:4c:c5:12:43"
}

set_stas_dhd_nocons
