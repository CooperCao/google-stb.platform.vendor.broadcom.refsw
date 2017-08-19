# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: c28a8256937d729a1ec7cd5360f3a380e3bdb6ea $}

package require UTF::Linux

set sta_type FC19_p2p_4359c0_gc
set ctrl_ip 10.19.90.25
set pwr_ip 10.19.90.195
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
   2 10.19.90.163 40001  "00:90:4c:12:d0:08"
}

set_stas_dhd_pcie
