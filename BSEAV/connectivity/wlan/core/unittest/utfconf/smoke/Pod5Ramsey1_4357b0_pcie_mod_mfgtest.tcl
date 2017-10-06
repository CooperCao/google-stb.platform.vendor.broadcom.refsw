# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "4357b0_pcie_mod"
set sta_type_aux "4357b0_pcie_mod_aux"
set ctrl_ip 10.19.80.25
set pwr_ip 10.19.80.202
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
  -sta "$sta_type eth0 $sta_type_aux wl1.2"\
  -dhd_brand linux-internal-dongle-pcie \
  -brand linux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -type 4357b0-roml/config_pcie_mfgtest_debug/rtecdc.bin \
  -nvram "bcm94357GuinnessUsiKK.txt" \
  -clm_blob 4347b0.clm_blob \
  -wlinitcmds {
     wl down;
     wl band a;
     wl interface_create sta -c 1;
     sleep 1;
     wl -i wl1.2 band b;
   }
set sta_table {
    # seq ip host_port   mac
    1 10.19.80.171 41100  00:90:4c:12:d0:01
    2 10.19.80.172 41101  00:90:4c:12:d0:02
    3 10.19.80.173 41102  00:90:4c:12:d0:03
    4 10.19.80.174 41103  00:90:4c:12:d0:04
    5 10.19.80.175 41104  00:90:4c:12:d0:05
    6 10.19.80.176 41105  00:90:4c:12:d0:06
}

set_stas_dhd_pcie_dual
