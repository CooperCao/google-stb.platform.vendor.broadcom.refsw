# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 5a68a483490fcde0c7e88b8e4c0d49efc744f5c7 $}

package require UTF::Linux

set sta_type "Fc19_43602"
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
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -brand linux-internal-dongle-pcie \
    -app_tag trunk\
    -type 43602a1-ram/pcie-ag-mfgtest-seqcmds-splitrx-redux/rtecdc.bin\
    -wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port power_usb usb_port mac
    6 10.19.91.103 40005 PowerUSB1 6 "22:15:4e:d5:38:32"
}

set_stas_dhd_nocons
