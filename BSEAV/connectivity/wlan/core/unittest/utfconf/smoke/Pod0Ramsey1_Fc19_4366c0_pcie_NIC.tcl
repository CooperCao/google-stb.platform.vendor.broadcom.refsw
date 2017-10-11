# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "Fc19_4366c0_pcie"
set ctrl_ip 10.19.90.23
set pwr_ip 10.19.90.74
set pwr_usb1_ip 10.19.90.74
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
	-type debug-apdef-stadef \
	-brand linux-internal-wl \
	-wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip port
    1 10.19.90.43 40300
    2 10.19.90.44 40301
    3 10.19.90.45 40302
    4 10.19.90.46 40303
    5 10.19.90.47 40304
    6 10.19.90.48 40305
}

set_stas_wcons
