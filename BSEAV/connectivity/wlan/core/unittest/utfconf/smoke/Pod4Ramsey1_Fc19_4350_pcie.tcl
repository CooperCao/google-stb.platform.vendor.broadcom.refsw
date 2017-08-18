# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: de6160ac1fd4c0b3c381579834166f612870c7eb $}

package require UTF::Linux

set sta_type "Fc19_4350_pcie"
set ctrl_ip 10.19.80.24
set pwr_ip 10.19.80.138
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
    -brand linux-external-dongle-pcie \
    -type 4350c2-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv/rtecdc.bin \

set sta_table {
    # seq ip host_port    mac                  code   rev
    1 10.19.80.107 41000  "00:90:4c:12:c0:01"  US     0
    2 10.19.80.108 41001  "00:90:4c:12:c0:02"  US     0
    3 10.19.80.109 41002  "00:90:4c:12:c0:03"  US     0
    4 10.19.80.110 41003  "00:90:4c:12:c0:04"  US     0
    5 10.19.80.111 41004  "00:90:4c:12:c0:05"  US     0
    6 10.19.80.112 41005  "00:90:4c:12:c0:06"  US     0
}

set_stas_dhd_pcie_cust
