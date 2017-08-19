# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 00ffc4fa26406d1d41ae9329270006ff6925bdfc $}

package require UTF::Linux

set sta_type "4357b0_pcie"
set sta_type_aux "4357b0_pcie_aux"
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
  -sta "$sta_type eth0 $sta_type_aux wl1.2"\
  -dhd_brand linux-internal-dongle-pcie \
  -brand linux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -type 4357b0-ram/config_pcie_debug/rtecdc.bin \
  -nvram "bcm94357GuinnessUsiKK.txt" \
  -clm_blob 4357a0.clm_blob \
  -wlinitcmds {
     wl down;
     wl band a;
     wl interface_create sta -c 1;
     sleep 1;
     wl -i wl1.2 band b;
   }
set sta_table {
    # seq ip host_port   mac
    1 10.19.90.98 40000  00:90:4c:12:d0:01
}

set_stas_dhd_pcie_dual
