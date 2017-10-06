# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type 43451b1_pcie
set ctrl_ip 10.19.91.23
set pwr_ip 10.19.91.81
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
  -brand linux-external-dongle-pcie \
  -dhd_brand linux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -nvram "bcm94345fcpagb_epa.txt" \
  -postinstall {dhd -i eth0 msglevel -event}\
  -type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-bssinfo/rtecdc.bin\
  -msgactions {"DTOH msgbuf not available" FAIL }

set sta_table {
   # seq ip host_port     mac
   1 10.19.91.52 43000   00:28:c7:0a:42:a2
   2 10.19.91.53 43001   00:28:c7:0a:42:a3
   3 10.19.91.54 43002   00:28:c7:0a:42:a4
   4 10.19.91.55 43003   00:28:c7:0a:42:a5
   5 10.19.91.56 43004   00:28:c7:0a:42:a6
   6 10.19.91.57 43005   00:28:c7:0a:42:a7
}

set_stas_dhd_pcie
