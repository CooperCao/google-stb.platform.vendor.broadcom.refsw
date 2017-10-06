# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "4357b1_pcie_mod"
set sta_type_aux "4357b1_pcie_mod_aux"
set ctrl_ip 10.19.90.23
set pwr_ip 10.19.90.81
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
  -sta "$sta_type eth0 $sta_type_aux eth0"\
  -dhd_brand linux-internal-dongle-pcie \
  -brand linux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
  -nvram "bcm94357GuinnessUsiKK.txt" \
  -clm_blob 4357.clm_blob \
  -wlinitcmds {
     wl down;
   }
set sta_table {
    # seq ip host_port   mac
    1 10.19.90.52  42000  00:90:4c:12:d0:01
    2 10.19.90.53  42001  00:90:4c:12:d0:02
    3 10.19.90.54  42002  00:90:4c:12:d0:03
    4 10.19.90.55  42003  00:90:4c:12:d0:04
    5 10.19.90.56  42004  00:90:4c:12:d0:05
    6 10.19.90.57  42005  00:90:4c:12:d0:06
}

set_stas_dhd_pcie_dual
