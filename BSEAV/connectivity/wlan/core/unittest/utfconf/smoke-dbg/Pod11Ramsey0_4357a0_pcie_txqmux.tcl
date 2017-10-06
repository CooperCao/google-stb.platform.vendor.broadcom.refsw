# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

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
  -sta "$sta_type eth0 $sta_type_aux wl1.2"\
  -dhd_brand linux-internal-dongle-pcie \
  -brand linux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -type 4357a0-ram/pcie-ag-err-assert-splitrx-txqmux-logtrace-redux/rtecdc.bin \
  -nvram "bcm94357fcpagbe.txt"\
  -clm_blob 4347a0.clm_blob\
  -app_tag trunk \
  -wlinitcmds {
     wl down;
     wl band a;
     wl interface_create sta -c 1;
     sleep 1;
     wl -i wl1.2 band b;
   }
set sta_table {
    # seq ip host_port   mac
    3 10.19.102.164 45002 00:90:4c:12:d0:01
}

set_stas_dhd_pcie_dual
