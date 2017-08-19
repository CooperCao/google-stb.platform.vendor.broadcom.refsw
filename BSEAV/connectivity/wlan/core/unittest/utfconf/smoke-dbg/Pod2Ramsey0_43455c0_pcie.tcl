# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id: e6bab0344c0af5be444f1b7dd198cc3ea8a0af62 $}

package require UTF::Linux

set sta_type 43455c0_pcie
set ctrl_ip 10.19.90.25
set pwr_ip 10.19.90.195
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
  -brand linux-external-dongle-pcie \
  -driver dhd-msgbuf-pciefd-debug \
  -app_tag trunk \
  -type 43455c0-roml/43455_pcie-43455_ftrs-pno-aoe-pktfilter-sr-pktctx-lpc-pwropt-wapi-mfp-clm_4335_ss-txpwr-rcc-fmc-wepso-noccxaka-sarctrl-proxd-gscan-linkstat-pwrstats-idsup-ndoe-pwrofs-hs20sta-nan-xorcsum-amsdutx-swdiv/rtecdc.bin \
  -nvram bcm943457wlpagb.txt

set sta_table {
   # seq ip host_port   mac
   3 10.19.90.164 40002  "00:90:4c:c5:52:78"
}

set_stas_dhd_pcie
