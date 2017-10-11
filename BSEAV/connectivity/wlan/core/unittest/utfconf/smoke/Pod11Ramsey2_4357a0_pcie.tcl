# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "4357a0_pcie"
set sta_type_aux "4357a0_pcie_aux"
set ctrl_ip 10.19.102.25
set pwr_ip 10.19.102.209
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
  -type 4357a0-ram/config_pcie_debug/rtecdc.bin \
  -nvram "bcm94357fcpagbe.txt"\
  -clm_blob 4347a0.clm_blob\
  -wlinitcmds {
     wl down;
     wl band a;
     wl interface_create sta -c 1;
     sleep 1;
     wl -i wl1.2 band b;
   }
set sta_table {
    # seq ip host_port   mac
    1 10.19.102.179 47000 00:90:4c:12:d0:01
    2 10.19.102.180 47001 00:90:4c:12:d0:02
    3 10.19.102.181 47002 00:90:4c:12:d0:03
    4 10.19.102.182 47003 00:90:4c:12:d0:04
    5 10.19.102.183 47004 00:90:4c:12:d0:05
    6 10.19.102.184 47005 00:90:4c:12:d0:06
}

set_stas_dhd_pcie_dual
