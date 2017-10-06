# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "4361b0_pcie"
set sta_type_aux "4361b0_pcie_aux"
set ctrl_ip 10.19.90.24 
set pwr_ip 10.19.90.131 
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
    3 10.19.90.100 40002 00:90:4c:12:d0:01
}

set_stas_dhd_pcie_dual
