# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: d77ea39cee492f08a96bedc0e13c57509c022f4c $}

package require UTF::Linux

set sta_type "4355c0_pcie"
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
  -dhd_brand linux-internal-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -brand linux-internal-dongle-pcie \
  -app_tag DHD_BRANCH_1_359 \
  -type 4355c0-roml/config_pcie_mfgtest/rtecdc.bin \
  -nvram "bcm94355KristoffMurataMM.txt" \
  -clm_blob "4355_kristoff.clm_blob" \
  -wlinitcmds {wl msglevel +assoc}

set sta_table {
    # seq ip host_port   mac
    1 10.19.91.34 40000  00:90:4c:12:d0:01
}

set_stas_dhd_pcie
