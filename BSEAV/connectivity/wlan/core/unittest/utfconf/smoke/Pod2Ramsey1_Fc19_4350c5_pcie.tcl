# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "Fc19_4350c5_pcie"
set ctrl_ip 10.19.90.25
set pwr_ip 10.19.90.202
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
    1 10.19.90.171 40100 "00:90:4c:12:c0:01"
    2 10.19.90.172 40101 "00:90:4c:12:c0:02"
    3 10.19.90.173 40102 "00:90:4c:12:c0:03"
    4 10.19.90.174 40103 "00:90:4c:12:c0:04"
    5 10.19.90.175 40104 "00:90:4c:12:c0:05"
    6 10.19.90.176 40105 "00:90:4c:12:c0:06"
}

set_stas_dhd_pcie
