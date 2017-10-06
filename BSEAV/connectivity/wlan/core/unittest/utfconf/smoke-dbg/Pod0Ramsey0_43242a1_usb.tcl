# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type 43242a1_usb
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
  -wlinitcmds {wl msglevel +assoc}\
  -nvram bcm943242usbref_p665.txt \
  -brand linux-internal-media \
  -nomaxmem 1 \
  -app_tag SMOKE_REL_FC19_43242_usb_bmac_BISON04T_BRANCH_7_14_wl_bcmdl_combo \
  -driver "debug-apdef-stadef-high-p2p-mchan-tdls-wowl-media" \
  -type 43242a1-bmac/ag-assert-p2p-mchan-media-srvsdb-vusb.bin.trx \
  -nvram bcm943242usbref_p665.txt

set sta_table {
   # seq ip port host_port power_usb usb_port
   5 10.19.90.38 41004 40004 PowerUSB1 5
}
set_stas_bmac

