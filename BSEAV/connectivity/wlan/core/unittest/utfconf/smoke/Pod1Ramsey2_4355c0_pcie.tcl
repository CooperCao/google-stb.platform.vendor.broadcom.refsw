# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "4355c0_pcie"
set ctrl_ip 10.19.90.24
set pwr_ip 10.19.90.145
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
  -dhd_brand linux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -brand linux-internal-dongle-pcie \
  -type 4355c0-roml/config_pcie_release/rtecdc.bin \
  -nvram "bcm94355KristoffMurataMM.txt" \
  -clm_blob "4355_kristoff.clm_blob" \
  -wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port   mac
    1 10.19.90.116 40200  00:90:4c:12:d0:02
    2 10.19.90.117 40201  00:90:4c:12:d0:03
    3 10.19.90.118 40202  00:90:4c:12:d0:04
    4 10.19.90.119 40203  00:90:4c:12:d0:05
    5 10.19.90.120 40204  00:90:4c:12:d0:06
    6 10.19.90.121 40205  00:90:4c:12:d0:07
}

set_stas_dhd_pcie
