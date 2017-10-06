# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "Fc19_4366c0_pcie"
set ctrl_ip 10.19.90.23
set pwr_ip 10.19.90.74
set pwr_usb1_ip 10.19.90.74
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
	-type 4366c0-ram/pcie-ag-splitrx-fdap-mbss-mfgtest-seqcmds-phydbg-phydump-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-11nprop-ringer-dmaindex16-dbgam-dbgams-bgdfs-murx-assert-vasip/rtecdc.bin \
	-wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port power_usb usb_port mac
    1 10.19.90.43 40300 PowerUSB1 1 "22:15:4e:d5:38:36"
    2 10.19.90.44 40301 PowerUSB1 2 "22:15:4e:d5:38:37"
    3 10.19.90.45 40302 PowerUSB1 3 "22:15:4e:d5:38:38"
    4 10.19.90.46 40303 PowerUSB1 4 "22:15:4e:d5:38:39"
    5 10.19.90.47 40304 PowerUSB1 5 "22:15:4e:d5:38:40"
    6 10.19.90.48 40305 PowerUSB1 6 "22:15:4e:d5:38:41"
}

set_stas_dhd_nocons
