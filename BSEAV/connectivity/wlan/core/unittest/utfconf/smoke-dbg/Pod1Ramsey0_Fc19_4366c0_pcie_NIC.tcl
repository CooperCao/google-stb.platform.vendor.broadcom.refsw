# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 930fc17a33c64dc07777719189e2db53e608361b $}

package require UTF::Linux

set sta_type "Fc19_4366c0_pcie"
set ctrl_ip 10.19.90.24
set pwr_ip 10.19.90.131
set pwr_usb1_ip 10.19.90.131
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
UTF::Linux STA_proto\
    -sta "$sta_type enp2s0"\
    -date "2016.12.14.1"\
	-type debug-apdef-stadef \
	-brand linux-internal-wl \
	-wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip port
    2 10.19.90.99 40001
}

set_stas_wcons
