# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Cygwin

set sta_type "Win81_32_4352"
set ctrl_ip 10.19.80.23
set pwr_ip 10.19.80.81
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
UTF::Cygwin STA_proto\
    -sta $sta_type\
    -user smoketest\
    -date "2016.6.15.0"\
    -brand win8x_internal_wl\
    -wlinitcmds {wl assert_type 1}\
    -installer InstallDriver \
    -osver 81

set sta_table {
    # seq ip
    1 10.19.80.52
    2 10.19.80.53
    3 10.19.80.54
    4 10.19.80.55
    5 10.19.80.56
    6 10.19.80.57
}

set_stas
