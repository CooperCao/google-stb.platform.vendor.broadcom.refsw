# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "Fc19_43452"
set ctrl_ip 10.19.91.24
set pwr_ip 10.19.91.131
set pwr_usb1_ip 10.19.91.131
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
    -user root\
    -dhd_brand linux-internal-dongle-pcie \
    -nvram "bcm94345Corona2TDKKK.txt" \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -clm_blob 43452_hans.clm_blob \
    -wlinitcmds {wl country XZ/248}\
    -type "43452a3-roml/pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-clm_min-ecounters-bssinfo-ltecx-txpwrcap-err-proxd-idauth-txcal-p5txgtbl-assert/rtecdc.bin"

set sta_table {
    # seq ip host_port     mac
    2 10.19.91.99 40001  "00:90:4c:c5:12:38"
}

set_stas_dhd_pcie
