# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 3e5c18c19bf9f1b024f9d11ae99070c84d9b765d $}

package require UTF::Linux

set sta_type 4358a1_pcie
set ctrl_ip 10.19.90.23
set pwr_ip 10.19.90.67
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
  -dhd_brand linux-internal-dongle-pcie \
  -brand linux-external-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -app_tag trunk \
  -nomaxmem 1 \
  -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert.bin \
  -nvram bcm94358wlpciebt.txt

set sta_table {
   # seq ip host_port   mac
   1 10.19.90.34 40000  "00:90:4c:19:90:01"
}

set_stas_dhd_pcie
