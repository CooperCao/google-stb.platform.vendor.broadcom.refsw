# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 0a0aea89d0b58fc885376cbef78671d7d58e841d $}

package require UTF::Linux

set sta_type "Fc19_43012"
set ctrl_ip 10.19.102.25
set pwr_ip 10.19.102.195
set pwr_usb1_ip 10.19.102.195
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
    -app_tag DHD_BRANCH_1_363 \
    -dhd_brand linux-external-dongle-sdio \
    -nvram "bcm943012fcbga.txt" \
    -brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
    -type -type 43012b0-roml/config_sdio_debug/rtecdc.bin \
    -wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port power_usb usb_port mac
    2 10.19.102.163 45001 PowerUSB1 2 "00:90:4c:c5:12:38"
}

set_stas_dhd_nocons
