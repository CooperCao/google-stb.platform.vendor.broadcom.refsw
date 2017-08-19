# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: e7646994f814e588a5c88f1470bb0126a652041b $}

package require UTF::Linux

set sta_type "Fc19_43435"
set ctrl_ip 10.19.91.24
set pwr_ip 10.19.91.131
set pwr_usb1_ip 10.19.91.131
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
    -clm_blob 43430b0.clm_blob \
    -brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
    -type 43430b0-roml/config_sdio_release/rtecdc.bin \
    -wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port power_usb usb_port mac
    3 10.19.91.100 40002 PowerUSB1 3 "00:90:4c:c5:12:38"
}

set_stas_dhd_nocons
