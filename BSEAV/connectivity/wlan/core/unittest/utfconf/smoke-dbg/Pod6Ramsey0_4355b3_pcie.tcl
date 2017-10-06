# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "4355b3_pcie"
set ctrl_ip 10.19.91.23
set pwr_ip 10.19.91.67
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
  -type 4355b3-roml/config_pcie_release/rtecdc.bin \
  -clm_blob "4355_simba_a.clm_blob" \
  -nvram "bcm94355StabelloCidreMurataKK.txt"

set sta_table {
    # seq ip host_port   mac
    3 10.19.91.36 40002  00:90:4c:12:d0:01
}

set_stas_dhd_pcie
