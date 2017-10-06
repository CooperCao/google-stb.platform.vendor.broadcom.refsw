
#-*-sc33.tcl-*-

package require UTF::Sniffer
package require UTF::Linux
package require UTF::Aeroflex
package require UTF::Multiperf
package require UTF::utils

set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

#set UTF::controlchart_iperf "-b 300M -i .5 -t 2.5"

# sc33 configuration

set ::UTF::SetupTestBed {ALL attn 0}
set ::UTF::SummaryDir "/projects/hnd_sig_ext10/sunnyc/mc33"
set ::UTF::SummaryLoc "/projects/hnd_sig_ext10/sunnyc/Sniffer33"

#set ::UTF::PostTestAnalysis "/home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl"

#set ::UTF::PostTestHook {
#    package require UTF::utils
#    UTF::do_post_test_analysis [info script] ""
#}

#set ::aux_lsf_queue sj-hnd

# The conf file to test

#Endpoints
UTF::Linux mc33end1 -sta {lan eth2} 
#UTF::Linux mc33end2 -sta {lan1 eth2} -tcpslowstart 4


UTF::Sniffer SNIF -lan_ip 10.19.85.167 -user root -tag BASS_BRANCH_5_60 -sta {sta4321 eth1}


# Specify BTAMP QTP tests
#set ::btamp_qtp_tests {{mc11tst1 mc11tst3 mc11tst4 "Vista 4313combo"}\
#    {mc11tst3 mc11tst1 mc11tst4 "Win7 4313combo"}\
#    {mc11tst4 mc11tst1 mc11tst3 "Win7 43225combo"}}

UTF::Aeroflex Aflex -lan_ip 172.16.1.210 -group {G1 {1 2} G2 {3 4} ALL {1 2 3 4}}

# Power
UTF::Power::Synaccess mc33npc1 -lan_ip 172.16.1.5
UTF::Power::Synaccess mc33npc2 -lan_ip 172.16.1.10 -rev 1
UTF::Power::Synaccess mc33npc3 -lan_ip 172.16.1.6
UTF::Power::Synaccess mc33npc4 -lan_ip 172.16.1.11
UTF::Power::Synaccess mc33npc5 -lan_ip 172.16.1.101
UTF::Power::Synaccess mc33npc6 -lan_ip 172.16.1.104

#Lava 172.16.1.99 for consolelogger1

# STAs


# 4322Win7
UTF::Cygwin mc38tst1 \
    -user user -sta "4322Win7_TOB" \
    -lan_ip 10.19.86.63 \
    -tag BISON_BRANCH_7_10 \
    -brand win_internal_wl \
    -noafterburner 1 \
    -osver 7 \
    -tcpwindow 512k \
    -power {mc38npc3 1} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -wlinitcmds {wl btc_mode 0}

4322Win7_TOB clone 4322Win7_REL \
    -tag BISON_REL_7_10_*


# Full dongle 43239 FC9
UTF::DHD mc33tst8 \
    -sta {43239FC9 eth1} \
    -console "mc33end1:40004" \
    -type "43239a0-roml/sdio-ag-idauth-pno" \
    -nvram "src/shared/nvram/bcm943239elhsdbsdio_p204.txt" \
    -power {mc33npc6 1} -power_button {auto} \
    -power_sta {mc33npc6 2} \
    -tag FALCON_REL_5_90_195_* \
    -noafterburner 1 \
    -nocal 1 -notkip 0 \
    -brand linux-external-dongle-sdio \
    -customer "bcm" \
    -postinstall {dhd -i eth1 sd_divisor 1} \
    -wlinitcmds {wl mpc 0; wl nphy_percal 0;:} 

43239FC9 configure -ipaddr 192.168.1.23

43239FC9 clone 43239P2P \
    -type "43239a0-roml/sdio-ag-p2p-idauth-pno"

# 4352HMB Win8 
UTF::Cygwin mc33tst1 \
    -user user -sta "4352Win8" \
    -noafterburner 1 \
    -tag BISON_BRANCH_7_10 \
    -node {DEV_43B1} \
    -brand win8x_internal_wl \
    -osver 8 \
    -tcpwindow 512k \
    -power {mc33npc3 2} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -wlinitcmds {wl btc_mode 0}

4352Win8 clone 4352Win8_nightly \
    -tag NIGHTLY



# 43227Win7
UTF::Cygwin mc52tst6 \
    -user user -sta "43227Win7" \
    -tag BISON_BRANCH_7_10 \
    -noafterburner 1 \
    -osver 7 \
    -tcpwindow 512k \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl stbc_rx} {%S wl counters}} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl counters}} \
    -power {mc33npc3 1} \
    -power_button {auto}

43227Win7 clone 43227Win7_nightly \
    -tag NIGHTLY


# 43236XP
UTF::WinDHD mc33tst4 \
    -user user -sta "43236XP" \
    -power_sta "mc33npc1 1" \
    -console "mc33end1:40002" \
    -tag BISON_BRANCH_7_10 \
    -brand win_internal_wl \
    -type checked/DriverOnly_BMac \
    -alwayspowercycledongle 0 \
    -noafterburner 1 \
    -hack 0 \
    -osver 5 \
    -tcpwindow 512k \
    -post_perf_hook {{%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu} {%S wl curpower} {%S wl scansuppress 0} {%S mu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl curpower} {%S wl scansuppress 1} {%S mu}} \
    -power {mc33npc1 2} \
    -power_button {auto} 

43236XP clone 43236XP_nightly \
    -tag NIGHTLY


# Make the sniffer a linux STA
UTF::Linux 43224nic -lan_ip 10.19.85.167 \
    -sta "43224NIC eth1 43224NIC.1 wl0.1" \
    -tag KIRIN_BRANCH_5_100 \
    -brand "linux-internal-wl" \
    -type obj-debug-p2p-mchan \
    -post_perf_hook {{%S wl dump ampdu} {%S wl rssi} {%S wl nrate}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}

# Desktop with FC11 & 43236USB bmac dongle
UTF::DHDUSB mc33tst7 \
    -sta "43236p2pbmac eth1 43236p2pbmac.0 wl0.1" \
    -power "mc33npc5 1" \
    -power_button "auto" \
    -power_sta "mc33npc5 2" \
    -console "mc33end1:40001" \
    -hostconsole "mc33end1:40003" \
    -brand "linux-internal-wl-media" \
    -tag "BISON_BRANCH_7_10" \
    -subdir "release/firmware" \
    -type "43236b-bmac/ag-assert-p2p-mchan-media"\
    -file "rtecdc.bin.trx" \
    -dhd_brand "linux-internal-wl-media"\
    -dhd_subdir "release"\
    -dhd_type "obj-debug-apdef-stadef-high-p2p-mchan-tdls-media-2.6.29.4-167"\
    -dhd_file "wl.ko" \
    -wl_brand "linux-internal-wl-media"\
    -wl_subdir release/exe \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}\
    -auto_download ""\
    -wlinitcmds {wl msglevel +assoc}


# 4322USB - removed, keep it for 43236linux clone
UTF::DHDUSB mc33tst10 \
    -sta "mc33_4322USB_bmac eth1" \
    -power "mc33npc5 1" \
    -power_button "auto"\
    -power_sta "mc33npc5 2"\
    -console "mc33end1:40001"\
    -hostconsole "mc33end1:40003"\
    -user root \
    -noafterburner 1\
    -brand "linux-internal-wl" \
    -subdir "release/firmware" \
    -type "4322-bmac/ag"\
    -dhd_brand "linux-internal-wl"\
    -dhd_subdir "release"\
    -dhd_type "obj-debug-{,native-}apdef-stadef-high"\
    -dhd_file "wl.ko" \
    -wl_brand "linux-internal-wl"\
    -wl_subdir release/exe \
    -tag "RUBY_BRANCH_6_20" \
    -post_perf_hook {{%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}


# 43236linux clone
mc33_4322USB_bmac clone 43236linux \
    -sta "43236linux eth1" \
    -brand "linux-internal-dongle-usb" \
    -tag FALCON_TWIG_5_90_195 \
    -subdir "release/firmware" \
    -type "43236b{,?}-bmac/ag"\
    -type "43236b{,?}-bmac/ag-assert-p2p"\
    -file "rtecdc.bin.trx" \
    -dhd_brand "linux-internal-wl"\
    -dhd_subdir "release"\
    -dhd_type "obj-debug-{,native-}apdef-stadef-high"\
    -dhd_file "wl.ko" \
    -wl_brand "linux-internal-wl"\
    -wl_subdir release/exe \
    -nvram_subdir src/shared/nvram \
    -nvram_file fake43236usb_p523.txt \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}\
    -auto_download ""


# DUT4 E4200 43236USB BMAC dongle
# On dhd_type, trailing -2 ensures we dont match on -p2p-mchan-tdls!
UTF::DHDUSB mc33tst9 \
    -sta "43236bmac eth1" \
    -power "mc33npc5 1" \
    -power_button "auto"\
    -power_sta "mc33npc5 2"\
    -console "mc33end1:40001"\
    -hostconsole "mc33end1:40003"\
    -user root \
    -brand "linux-internal-wl" \
    -tag BISON_BRANCH_7_10 \
    -subdir "release/firmware" \
    -type "43236b{,?}-bmac/ag-assert"\
    -file "rtecdc.bin.trx" \
    -dhd_brand "linux-internal-wl"\
    -dhd_subdir "release"\
    -dhd_type "obj-debug-{,native-}apdef-stadef-high-2"\
    -dhd_file "wl.ko" \
    -wl_brand "linux-internal-wl"\
    -wl_subdir release/exe \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}\
    -auto_download ""\
    


# DUT4 using 43236USB FULL dongle
43236bmac clone 43236full \
    -tag FALCON_BRANCH_5_90 \
    -brand "linux-internal-dongle-usb" \
    -subdir "release/bcm/firmware" \
    -type "43236b{,?}-roml/"\
    -file "ag.bin.trx" \
    -dhd_brand "linux-internal-dongle-usb"\
    -dhd_subdir "release/bcm/host"\
    -dhd_type "dhd-cdc-usb-gpl"\
    -dhd_file "dhd.ko" \
    -wl_brand "linux-internal-dongle-usb"\
    -wl_subdir release/bcm/apps \
    -nvram_subdir src/shared/nvram \
    -nvram_file fake43236usb_p532.txt \
    -post_perf_hook {{%S rte mu} {%S wl rssi} {%S wl nrate} {4717_1 wl dump ampdu}}\
    -pre_perf_hook {{4717_1 wl ampdu_clear_dump}}

# FULL TWIG Dongle
# NB: for -reqfw dhd, must have -nodis firmware!
43236full clone 43236fulltwig \
    -tag FALCON_TWIG_5_90_188\
    -req_fw 1 \
    -chip bcm43236\
    -file ag-nodis-fdaggr/rtecdc.bin.trx \
    -dhd_type "dhd-cdc-usb-reqfw-fdaggr-gpl"



# Items for p2p_coex.test
set ::wlan_rtr 4717_3
set ::wlan_tg lan


#APs
# 4717_3
UTF::Router mc33rt1 \
    -sta {4717_3 eth1} \
    -lan_ip 192.168.1.3 \
    -relay "mc33end1" \
    -power {mc33npc2 1} \
    -lanpeer lan \
    -console "10.19.85.249:40000" \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65" \
    -nvram {
	fw_disable=1
	watchdog=6000
	wl_msglevel=0x101
	wl0_ssid=mc33ap1
	wl0_channel=6
	et0macaddr=00:90:4c:07:00:6b
        macaddr=00:90:4c:07:00:7b
	antswitch=0
	wl0_obss_coex=0
    }


# 4717_4
#UTF::Router mc33rt2 \
#    -sta {4717_4 eth2} \
#    -lan_ip 192.168.1.4 \
#    -relay "mc33end2" \
#    -power {mc33npc4 1} \
#    -lanpeer lan1 \
#    -console "mc33end2:40000" \
#    -tag AKASHI_BRANCH_5_110 \
#    -brand linux-external-router \
#    -nvram {
#        fw_disable=1
#	watchdog=3000
#        wl_msglevel=0x101
#        wl0_ssid=mc33ap2
#	wl0_channel=1
#	et0macaddr=00:90:4c:07:00:4b
#	macaddr=00:90:4c:07:00:5b
#	antswitch=0
#	wl0_obss_coex=0
#    }

#Branches
#4717_3 configure -tag COMANCHE2_REL_5_22_88
4717_3 configure -tag AKASHI_REL_5_110_65

#4717_4 configure -tag AKASHI_BRANCH_5_110

# Set testrig specific path loss for rvr1.test
set ::43236XP_pathloss_2G 29
set ::43236XP_pathloss_5G 40
set ::43235Win7_pathloss_2G 28
set ::43227XP_pathloss_2G 27
#For 43236linux: -tag FALCON_TWIG_5_90_188 \
#    -type "43239a0-roml/sdio-ag-p2p-idauth-pno" \
