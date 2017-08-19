# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 843a95b50d215009b5905c8ed8375b1c32623068 $}

package require UTF::Linux

set sta_type 43569a2_usb
set ctrl_ip 10.19.90.23
set pwr_ip 10.19.90.67
set pwr_usb1_ip 10.19.90.67
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
  -dhd_brand linux-internal-media \
  -driver dhd-cdc-usb-gpl \
  -brand linux-internal-media \
  -type 43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx \
  -nvram bcm943569usbir_p156.txt

set sta_table {
   # seq ip port host_port power_usb usb_port mac
   4 10.19.90.37 41003 40003 PowerUSB1 4  "00:90:4c:1a:90:01"
}

set_stas_dhd
