# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type "4364b1_pcie"
set ctrl_ip 10.19.102.25
set pwr_ip 10.19.102.202
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
  -brand linux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -type 4364b1-roml/config_pcie_perf_sdb_udm/rtecdc.bin \
  -nvram "bcm94364fcpagb_2.txt"\
  -clm_blob 4364b0.clm_blob\
  -wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port   mac
    1 10.19.102.171 46000 00:90:4c:1b:00:01
    2 10.19.102.172 46001 00:90:4c:1b:00:02
    3 10.19.102.173 46002 00:90:4c:1b:00:03
    4 10.19.102.174 46003 00:90:4c:1b:00:04
    5 10.19.102.175 46004 00:90:4c:1b:00:05
    6 10.19.102.176 46005 00:90:4c:1b:00:06
}

set_stas_dhd_pcie
