# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 5f6dfeb1f157ff9fa58c83e336252fb41ee9bf85 $}

package require UTF::Linux

set sta_type 4358a1_pcie
set ctrl_ip 10.19.91.25
set pwr_ip 10.19.91.209
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
  -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert.bin \
  -nvram bcm94358wlpciebt.txt

set sta_table {
   # seq ip host_port   mac
   1 10.19.91.179 42000  "00:90:4c:19:90:01"
   2 10.19.91.180 42001  "00:90:4c:19:90:02"
   3 10.19.91.181 42002  "00:90:4c:19:90:03"
   4 10.19.91.182 42003  "00:90:4c:19:90:04"
   5 10.19.91.183 42004  "00:90:4c:19:90:05"
   6 10.19.91.184 42005  "00:90:4c:19:90:06"
}

set_stas_dhd_pcie
