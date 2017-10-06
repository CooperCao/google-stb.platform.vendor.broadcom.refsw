# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "Fc19_43602"
set ctrl_ip 10.19.91.24
set pwr_ip 10.19.91.131
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
    -sta "$sta_type enp1s0"\
	-type debug-apdef-stadef \
	-brand linux-internal-wl \
	-wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip port
    6 10.19.91.103 40005
}

set_stas_wcons
