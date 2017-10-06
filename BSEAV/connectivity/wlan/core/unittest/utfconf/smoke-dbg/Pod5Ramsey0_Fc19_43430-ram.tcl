# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "Fc19_43430"
set ctrl_ip 10.19.80.25
set pwr_ip 10.19.80.195
set pwr_usb1_ip 10.19.80.196
set pwr_usb2_ip 10.19.80.197
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
    -app_tag trunk \
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43430b0-ram/sdio-g/rtecdc.bin \
	-nvram bcm943430fsdnge_elna_Bx.txt  \
    -nomaxmem 1 \
	-wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port power_usb usb_port mac 
    4 10.19.80.165 40103 PowerUSB1 4 "00:90:4c:c5:12:38"
}

set_stas_dhd_nocons
