# -*-tcl-*-
set __doc__ "Ramsey configuration."
set __version__  {$Id$}

package require UTF::Linux

set sta_type 43455c0_pcie
set ctrl_ip 10.19.102.24
set pwr_ip 10.19.102.138
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
  -type 43455c0-roml/43455_pcie-43455_ftrs-pno-aoe-pktfilter-sr-pktctx-lpc-pwropt-wapi-mfp-clm_4335_ss-txpwr-rcc-fmc-wepso-noccxaka-sarctrl-proxd-gscan-linkstat-pwrstats-idsup-ndoe-pwrofs-hs20sta-nan-xorcsum-amsdutx-swdiv/rtecdc.bin \
  -nvram bcm943457wlpagb.txt

set sta_table {
   # seq ip host_port   mac
   1 10.19.102.107 42000  "00:90:4c:c5:12:38"
   2 10.19.102.108 42001  "00:90:4c:c5:12:39"
   3 10.19.102.109 42002  "00:90:4c:c5:12:40"
   4 10.19.102.110 42003  "00:90:4c:c5:12:41"
   5 10.19.102.111 42004  "00:90:4c:c5:12:42"
   6 10.19.102.112 42005  "00:90:4c:c5:12:43"
}

set_stas_dhd_pcie
