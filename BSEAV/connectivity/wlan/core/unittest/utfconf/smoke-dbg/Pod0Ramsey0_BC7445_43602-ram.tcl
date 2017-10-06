# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux
package require UTF::STB

set sta_type "BCM7445_43602"
set ctrl_ip 10.19.90.23
set pwr_ip 10.19.90.67
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
    -app_tag trunk \
	-dhd_brand linux-internal-media \
	-brand linux-external-media \
    -type dhd-msgbuf-pciefd-mfp-armv7l-debug \
    -dongleimage "43602a1-ram/pcie-ag-err-splitrx.bin"

set sta_table {
    # seq ip host_port mac wlan_ip
    6 10.19.90.39 40005 "00:90:4c:11:22:33" 192.168.1.91
}

set_stas_dhd_nocons_stb
