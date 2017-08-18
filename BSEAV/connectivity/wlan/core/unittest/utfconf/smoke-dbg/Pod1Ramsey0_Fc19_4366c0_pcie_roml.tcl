# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 4f5cbc949fd8920e62ae26d0f399a5c299560de1 $}

package require UTF::Linux

set sta_type "Fc19_4366c0_pcie"
set ctrl_ip 10.19.90.24
set pwr_ip 10.19.90.131
set pwr_usb1_ip 10.19.90.131
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
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-dump-murx \
	-wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port power_usb usb_port mac
    2 10.19.90.99 40001 PowerUSB1 2 "00:90:4c:1d:20:01"
}

set_stas_dhd_nocons
