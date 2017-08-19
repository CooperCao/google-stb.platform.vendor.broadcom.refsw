# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: dd0923e1e9bfa16c10f5527b4c4ab47c69c8f337 $}

package require UTF::Linux

set sta_type "4357a0_pcie"
set sta_type_aux "4357a0_pcie_aux"
set ctrl_ip 10.19.102.25
set pwr_ip 10.19.102.195
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
  -sta "$sta_type eth0 $sta_type_aux wl0.2"\
  -dhd_brand linux-internal-dongle-pcie \
  -brand linux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -type 4361a0-roml/config_pcie_release/rtecdc.bin \
  -nvram "bcm94357fcpagbe.txt"\
  -clm_blob ss_mimo.clm_blob\
  -app_tag DHD_BRANCH_1_579\
  -wlinitcmds {
     wl down;
     wl interface_create sta;
     sleep 1;
   }


set sta_table {
    # seq ip host_port   mac
    3 10.19.102.164 45002 00:90:4c:12:d0:01
}

set_stas_dhd_pcie_dual
