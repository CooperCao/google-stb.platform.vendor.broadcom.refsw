# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: 93b570bd69fcde54050ee8709b981e2051db7769 $}

package require UTF::Linux

set sta_type "4364b1_pcie"
set ctrl_ip 10.19.102.25
set pwr_ip 10.19.102.195
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
  -brand inux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -app_tag DHD_BRANCH_1_359 \
  -type 4364b1-roml/config_pcie_release/rtecdc.bin \
  -nvram "bcm94364fcpagb_2.txt"\
  -clm_blob 4364b0.clm_blob\
  -wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port   mac
    1 10.19.102.162 45000 00:90:4c:1b:00:01
}

set_stas_dhd_pcie
