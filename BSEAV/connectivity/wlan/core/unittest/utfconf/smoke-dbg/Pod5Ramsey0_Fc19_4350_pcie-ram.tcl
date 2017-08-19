# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 6970e1d1fff2ba80b7d4cbaa971e749c4454e996 $}

package require UTF::Linux

set sta_type "Fc19_4350_pcie"
set ctrl_ip 10.19.80.25
set pwr_ip 10.19.80.195
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
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94350RieslingBMurataMT.txt" \
    -brand linux-internal-dongle-pcie \
    -nomaxmem 1 \
    -type 4350c1-ram/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-assert-err.bin

set sta_table {
    # seq ip host_port    mac                 code   rev
    3 10.19.80.164 40102 "00:90:4c:12:c0:01"   US     0
}

set_stas_dhd_pcie_cust
