# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "MacBkProELC_43602"
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
UTF::MacOS STA_proto\
	-sta "$sta_type en0"\
	-brand macos-internal-wl-gala\
	-type Debug_10_11\
	-user root\
	-kextload 1\
    -nomaxmem 1 \
	-wlinitcmds {wl msglevel +assoc }

set sta_table {
    # seq wlan_ip ip
    4 192.168.1.90 10.19.91.101
}

set_stas_wlan
