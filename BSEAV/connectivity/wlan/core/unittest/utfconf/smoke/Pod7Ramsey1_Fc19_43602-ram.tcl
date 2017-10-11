# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "Fc19_43602"
set ctrl_ip 10.19.91.24
set pwr_ip 10.19.91.138
set pwr_usb1_ip 10.19.91.138
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
	-type 43602a1-ram/pcie-ag-err-assert-splitrx-logtrace-redux.bin \
	-wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port power_usb usb_port mac
    1 10.19.91.107 43000 PowerUSB1 1 "22:15:4e:d5:38:33"
    2 10.19.91.108 43001 PowerUSB1 2 "22:15:4e:d5:38:34"
    3 10.19.91.109 43002 PowerUSB1 3 "22:15:4e:d5:38:35"
    4 10.19.91.110 43003 PowerUSB1 4 "22:15:4e:d5:38:36"
    5 10.19.91.111 43004 PowerUSB1 5 "22:15:4e:d5:38:37"
    6 10.19.91.112 43005 PowerUSB1 6 "22:15:4e:d5:38:38"
}

set_stas_dhd_nocons
