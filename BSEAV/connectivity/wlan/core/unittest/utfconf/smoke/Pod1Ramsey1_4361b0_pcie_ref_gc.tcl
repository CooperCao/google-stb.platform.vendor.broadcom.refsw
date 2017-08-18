# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 33c0687157b2a4ebd7449b697301c6883f6450d5 $}

package require UTF::Linux

set sta_type "FC19_p2p_4361b0_pcie_gc"
set sta_type_aux "FC19_p2p_4361b0_pcie_gc_aux"
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
  -wlinitcmds {
     wl down;
   }


set sta_table {
    # seq ip host_port   mac
    2 10.19.90.108 40101 00:90:4c:12:d0:02
    3 10.19.90.109 40102 00:90:4c:12:d0:03
    4 10.19.90.110 40103 00:90:4c:12:d0:04
    5 10.19.90.111 40104 00:90:4c:12:d0:05
    6 10.19.90.112 40105 00:90:4c:12:d0:06
}

set_stas_dhd_pcie_dual
