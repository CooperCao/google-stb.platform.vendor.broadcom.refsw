# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type FC19_p2p_4359c0_gc
set ctrl_ip 10.19.102.24
set pwr_ip 10.19.102.145
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
  -brand linux-external-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -type 43596a0-roml/config_pcie_preco_st/rtecdc.bin \
  -nvram bcm943593fcpagbss.txt

set sta_table {
   # seq ip host_port   mac
   2 10.19.102.117 43001  "00:90:4c:12:d0:02"
   3 10.19.102.118 43002  "00:90:4c:12:d0:03"
   4 10.19.102.119 43003  "00:90:4c:12:d0:04"
   5 10.19.102.120 43004  "00:90:4c:12:d0:05"
   6 10.19.102.121 43005  "00:90:4c:12:d0:06"
}

set_stas_dhd_pcie
