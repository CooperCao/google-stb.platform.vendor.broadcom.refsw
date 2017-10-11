# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Cygwin

set sta_type "Win81_32_4352"
set ctrl_ip 10.19.80.24
set pwr_ip 10.19.80.131
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
    -brand win8x_internal_wl\
    -date "2016.6.15.0"\
    -wlinitcmds {wl assert_type 1}\
    -installer InstallDriver \
    -osver 81

set sta_table {
    # seq ip
    5 10.19.80.102
}

set_stas
