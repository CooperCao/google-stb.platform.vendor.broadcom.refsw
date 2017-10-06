# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type 43242a1_usb
set ctrl_ip 10.19.80.23
set pwr_ip 10.19.80.74
set pwr_usb1_ip 10.19.80.74
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
  -wlinitcmds {wl msglevel +assoc}\
  -brand linux-internal-media \
  -driver "debug-apdef-stadef-high-p2p-mchan-tdls-wowl-media" \
  -type 43242a1-bmac/ag-assert-p2p-mchan-media-srvsdb-vusb.bin.trx \
  -nomaxmem 1 \
  -app_tag SMOKE_REL_FC19_43242_usb_bmac_BISON04T_BRANCH_7_14_wl_bcmdl_combo \
  -nvram bcm943242usbref_p665.txt

set sta_table {
   # seq ip port host_port power_usb usb_port mac
   1 10.19.80.43 44000 41000 PowerUSB1 1  "00:90:4c:c5:12:38"
   2 10.19.80.44 44001 41001 PowerUSB1 2  "00:90:4c:c5:12:39"
   3 10.19.80.45 44002 41002 PowerUSB1 3  "00:90:4c:c5:12:40"
   4 10.19.80.46 44003 41003 PowerUSB1 4  "00:90:4c:c5:12:41"
   5 10.19.80.47 44004 41004 PowerUSB1 5  "00:90:4c:c5:12:42"
   6 10.19.80.48 44005 41005 PowerUSB1 6  "00:90:4c:c5:12:43"
}
set_stas_dhd

