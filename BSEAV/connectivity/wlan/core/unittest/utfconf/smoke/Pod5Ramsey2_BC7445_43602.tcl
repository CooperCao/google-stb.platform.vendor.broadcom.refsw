# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: c769e878f9c9282b5e1e4e55ee94cde905f5475c $}

package require UTF::Linux
package require UTF::STB

set sta_type "BCM7445_43602"
set ctrl_ip 10.19.80.25
set pwr_ip 10.19.80.209
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
UTF::STB STA_proto\
    -sta "$sta_type eth1"\
	-dhd_brand linux-internal-media \
	-brand linux-external-media \
    -type dhd-msgbuf-pciefd-mfp-armv7l-debug \
    -dongleimage "43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive.bin"

set sta_table {
    # seq ip host_port mac wlan_ip
    1 10.19.80.179 45100 "00:90:4c:11:22:33" 192.168.1.90
    2 10.19.80.180 45101 "00:90:4c:11:22:34" 192.168.1.91
    3 10.19.80.181 45102 "00:90:4c:11:22:35" 192.168.1.92
    4 10.19.80.182 45103 "00:90:4c:11:22:36" 192.168.1.93
    5 10.19.80.183 45104 "00:90:4c:11:22:37" 192.168.1.94
    6 10.19.80.184 45105 "00:90:4c:11:22:38" 192.168.1.95
}

set_stas_dhd_nocons_stb
