# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: e39f7be2ad296beac5305ae235f84cd201a12b2d $}

package require UTF::Linux

set sta_type "Fc19_4350c5_pcie"
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
    -user root\
    -postinstall {dhd -i eth0 msglevel -event}\
    -dhd_brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94350RieslingBMurataMT.txt" \
    -brand linux-external-dongle-pcie \
    -clm_blob 4350_riesling_b.clm_blob \
    -type 4350c5-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv-ecounters-bssinfo/rtecdc.bin \

set sta_table {
    # seq ip host_port    mac
    2 10.19.91.35 40001 "00:90:4c:12:c0:01"
}

set_stas_dhd_pcie
