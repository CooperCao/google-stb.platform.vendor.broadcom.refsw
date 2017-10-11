# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux
set UTF::UseFCP 1
set UTF::MacOSLoadPower 1
set sta_type "MacBkELC_4350"
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
    -kextunload 2\
    -nomaxmem 1 \
	-wlinitcmds {wl msglevel +assoc }

set sta_table {
    # seq wlan_ip ip
    5 192.168.1.90 10.19.91.102
}

set_stas_wlan_lap
