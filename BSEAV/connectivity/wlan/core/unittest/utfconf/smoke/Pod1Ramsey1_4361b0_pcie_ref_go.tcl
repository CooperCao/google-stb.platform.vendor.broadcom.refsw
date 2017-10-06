# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "FC19_p2p_4361b0_pcie_go"
set sta_type_aux "FC19_p2p_4361b0_pcie_go_aux"
set ctrl_ip 10.19.90.24
set pwr_ip 10.19.90.138
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
  -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
  -nvram "bcm94361fcpagbi_B0_p301.txt"\
  -clm_blob ss_mimo.clm_blob\
  -app_tag trunk\
  -wlinitcmds {
     wl down;
   }


set sta_table {
    # seq ip host_port   mac
    1 10.19.90.107 40100 00:90:4c:12:d0:01
}

set_stas_dhd_pcie_dual
