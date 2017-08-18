#Testbed configuration file for blr01and1
#Edited Battu kaushik Date 30Dec2015

package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix


set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blrsw"

#pointing Apps to trunk

set ::UTF::TrunkApps 1 \

set ::UTF::FBDefault 1 \

#set ::UTF::ChannelPerf 1 \

UTF::Linux repo -lan_ip xl-sj1-24.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck


UTF::Linux Adbcontrl \
     -lan_ip 10.132.116.157 \
     -sta {lan em1} \

UTF::Linux brix-ap \
    -sta {4360sw enp1s0} \
    -lan_ip 10.132.116.158 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10 \
    -brand "linux-internal-wl" \
    -perfchans {36/80} \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} } \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
    -tcpwindow 3M \
    -udp 1300M \
    -wlinitcmds {
                 wl msglevel 0x101;wl dtim 3; wl mimo_bw_cap 1; wl vht_features 3;wl mpc 0 
                }
4360sw configure -ipaddr 192.168.1.90  -ap 1 -hasdhcpd 1  \

package require UTF::Android 

UTF::Android SamsungGN5 \
     -relay Adbcontrl \
	 -sta {4359b1x wlan0} \
	 -dhd_brand android-external-dongle \
     -driver ZImage \
     -tag DIN07T48RC50_BRANCH_9_75 \
     -nocal 1 -slowassoc 5 \
     -nvram "bcm943596fcpagbss.txt"\
	 -brand hndrte-dongle-wl \
     -type 4364a0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-logtrace-assert/rtecdc.bin \
     -udp 1300M  \
	 -tcpwindow 3M \
	 -extsup 1 \
	 -wlinitcmds {wl down; wl vht_features 3} \
	 -pre_perf_hook {{%S wl nrate} {%S wl dump rsdb}} \
	 -post_perf_hook {{%S wl nrate } {%S wl dump rsdb}} \
	 -channelsweep {-usecsa} \
	 -perfchans {36/80 36l 36 3} \


4359b1x configure -ipaddr 192.168.1.101  \

UTF::Android Nexus6 \
     -relay Adbcontrl \
	 -adbdevice ZX1G42C2WL \
	 -sta {4359c0x wlan0} \
	 -dhd_tag DHD_REL_1_363_59_140 \
	 -dhd_brand android-external-dongle \
     -driver bootimage-5.1.1-nexus6-interposer  \
     -tag DIN07T48RC50_REL_9_75_155_11 \
     -nocal 1 -slowassoc 5 \
     -nvram "bcm943596fcpagbss.txt"\
	 -brand hndrte-dongle-wl \
     -type 43596a0-roml/config_pcie_release/rtecdc.bin \
	 -udp 1300M  \
	 -extsup 1 \
	 -tcpwindow 3M \
	 -wlinitcmds {wl down; wl vht_features 3} \
	 -pre_perf_hook {{%S wl nrate} {%S wl dump rsdb}} \
	 -post_perf_hook {{%S wl nrate } {%S wl dump rsdb}} \
	 -channelsweep {-usecsa} \
	 -perfchans {36/80 36l 3} \

4359c0x configure -ipaddr 192.168.1.91 \
  


