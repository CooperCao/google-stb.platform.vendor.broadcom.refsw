# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set UTF::MacOSLoadPower 1
set sta_type "MacBkELC_4350"
set ctrl_ip 10.19.102.23
set pwr_ip 10.19.102.74
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
    -kextunload 2\
	-kextload 1\
    -nativetools 1 \
    -console {/var/log/system.log /var/log/wifi.log} \
	-wlinitcmds {wl msglevel +assoc }

set sta_table {
    # seq wlan_ip ip
    1 192.168.1.90 10.19.102.43
    2 192.168.1.91 10.19.102.44
    3 192.168.1.92 10.19.102.45
    4 192.168.1.93 10.19.102.46
    5 192.168.1.94 10.19.102.47
    6 192.168.1.95 10.19.102.48
}

set_stas_wlan_lap
