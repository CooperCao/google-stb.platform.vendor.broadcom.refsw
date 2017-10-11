# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type 43451b1_pcie
set ctrl_ip 10.19.91.23
set pwr_ip 10.19.91.67
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
  -driver dhd-msgbuf-pciefd-debug \
  -nvram "bcm94345fcpagb_epa.txt" \
  -postinstall {dhd -i eth0 msglevel -event}\
  -type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap/rtecdc.bin\
  -msgactions {"DTOH msgbuf not available" FAIL }

set sta_table {
   # seq ip host_port   mac
   4 10.19.91.37 40003  "00:90:4c:c5:03:03"
}

set_stas_dhd_pcie
