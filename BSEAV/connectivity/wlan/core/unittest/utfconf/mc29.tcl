# -*-tcl-*-
#
# Configuration file for Milosz Zielinski's MC29 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::DHD

# UTFD support
set ::env(UTFDPORT) 9978
package require UTFD


UTF::Linux xlinux

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/mc29"

# Enable Windows TestSigning
set UTF::TestSigning 1

# Define power controllers on cart
UTF::Power::Synaccess npc21 -lan_ip 172.5.5.21 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.5.5.41 -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.5.5.42 -rev 1
UTF::Power::Synaccess npc71 -lan_ip 172.5.5.71 -rev 1
#UTF::Power::Synaccess npc72 -lan_ip 172.5.5.72 -rev 1

#./UTF.tcl npc100 power on 4   (utfobjectname {power on/power off} {port})
UTF::Power::Synaccess RamseyLargeFan -lan_ip 172.5.5.92


#UTF::Power::WebRelay  web190 -lan_ip 192.168.1.90 -invert 1

# Attenuator - Aeroflex
#                ALL {1 2 3 4 5 6 7 8 9 10 11 12}
#		G2 {4 5 6} 
#		G3 {7 8 9} 
#		G4 {10 11 12} 
UTF::Aeroflex af -lan_ip 172.5.5.91 \
	-relay "mc29end1" \
	-group {
		G1 {1 2 3} 
                ALL {1 2 3}
	       }

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to default (usually 0) value
    #
    catch {ALL attn 0;}
    catch {G1 attn  20;}
#    catch {G1 attn 40;}
#    catch {G2 attn 0;}
#    catch {G3 attn 0;}
#    catch {G4 attn 0;}

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
#    catch {4331-a restart wl0_radio=0}
#    catch {4331-a restart wl1_radio=0}
    catch {4706/4360 restart wl0_radio=0}
    catch {4706/4331 restart wl1_radio=0}
    #catch {47061/4360 restart wl0_radio=0}
    #catch {47061/4331 restart wl1_radio=0}
    # ensure sniffer is unloaded
    #catch {4331SNF1 unload}
    #catch {4360SNF1 unload}    

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S { fc15-29  W8X64-29 W8X64-29-3  fc15-294} {
	    UTF::Try "$S Down" {
		    catch {$S wl down}
		    catch {$S deinit}
	    }
    }
    # unset S so it doesn't interfere
    unset S
          
    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    # commented out the next line until the power controller is replaced
    #RamseyLargeFan power on 8

    return
}

set ::UTF::PostTestHook {
    # commented out the next line until the power controller is replaced
    #RamseyLargeFan power off 8
}

# Define Sniffer
UTF::Sniffer mc29snf1 -user root \
        -sta {4331SNF1 eth1} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc13 1} \
        -power_button {auto} \
        -console "mc29end1:40001"

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc29end1 -lan_ip 10.22.20.110  \
    -sta {lan eth1} 

# UTF Endpoint2 FC11 - Traffic generators (no wireless cards)
#UTF::Linux mc29end2 -lan_ip 10.22.20.111  
#    -sta {lan eth1} 


# mc29tst1: Laptop DUT Dell E4310 
# OS: Windows 8 build 9200
# BCMs: "43228 dual band"
# 2015.3.31 update installed on 2015.4.17
# power cycled on 2015.5.11 - hung - on Activate screen
# 2015.7.02 update installed on 2015.7.06
UTF::Cygwin mc29tst1 -user user -sta {W8X86-29} \
        -osver 8 \
        -sign true \
        -installer inf \
        -lan_ip mc29tst1 \
        -tcpwindow 4M \
        -brand win8x_internal_wl \
        -wlinitcmds {wl msglevel 0x101; wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 0} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -power "npc21 2" \
        -power_button {auto}


#W8X64-29 clone W8X64-29-TOT
#W8X64-29 clone W8X64-29-AARD-637  -tag AARDVARK01T_BRANCH_6_37_*
#W8X64-29 clone W8X64-29-AARD-TOB  -tag AARDVARK_BRANCH_6_30
#W8X64-29 clone W8X64-29-BISON     -tag BISON_BRANCH_7_10     -brand win8x_internal_wl
#W8X64-29 clone W8X64-29-BISON735  -tag BISON05T_BRANCH_7_35
#W8X64-29 clone W8X64-29-BISON735_trace  -tag BISON05T_BRANCH_7_35 \
#               -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl msglevel +wsec +prpkt +trace}} \
#               -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl msglevel -wsec -prpkt -trace}}
#W8X64-29 clone W8X64-29-TOT_trace  \
#               -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl msglevel +wsec +prpkt +trace}} \
#               -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl msglevel -wsec -prpkt -trace}}
               
               
#               -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl msglevel +wsec +prpkt +trace}}
#               -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl msglevel -wsec -prpkt -trace}}



#               -nocache -bin "-altsys /projects/hnd_sw_ndis/work/rraina/osldelay/bcmwl63.sys"

#W8X64-29-TOT   clone W8X64-29-RAHUL  \
#               -post_perf_hook {{%S wl dump perf_stats}}




# mc29tst2: Desktop DUT
# OS: Linux FC 15
# BCMs: "Linux 4331hm P152"
#
UTF::Linux mc29tst2 \
        -sta "fc15-29 eth0" \
        -console "mc29end1:40004" \
        -wlinitcmds {wl mpc 0; service dhcpd stop;:} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} } \
        -tcpwindow 4M \
        -power "npc21 1" 
#	-perfchans {36l 3}


# Clones of STA 4331FC15 with Different Options for Test
#-type obj-debug-p2p-mchan
#fc15-29 clone fc15-29-AARDVARK-P2P -tag AARDVARK_BRANCH_6_30 -type obj-debug-p2p-mchan 
#fc15-29 clone fc15-29-AARDVARK -tag AARDVARK_BRANCH_6_30 
#fc15-29 clone fc15-29-637      -tag AARDVARK01T_BRANCH_6_37_*
fc15-29 clone fc15-29-BISON    -tag BISON_BRANCH_7_10 
fc15-29 clone fc15-29-EAGLE    -tag EAGLE_BRANCH_10_10 
fc15-29 clone fc15-29-CARIBOU  -tag CARIBOU_BRANCH_8_10 -wlinitcmds {wl msglevel 0x100} 
fc15-29 clone fc15-29-BISON_phycal    -tag BISON_BRANCH_7_10 \
                                -pre_perf_hook {{%S wl ampdu_clear_dump} \
                                                {%S wl phy_cal_disable 1}   {%S wl dump phycal}} \
                                -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} \
                                                 {%S wl phy_cal_disable 0}  {%S wl dump phycal}}
fc15-29 clone fc15-29-TOT

#fc15-29 clone fc15-29-BISONp2p -tag BISON_BRANCH_7_10  -type debug-p2p-mchan



if {0} {
# mc29tst3: Laptop DUT Dell Vostro 1500
# OS: Windows 8.1
# BCMs: "43142 dual band"
#
UTF::Cygwin mc29tst3 \
        -osver 8164 \
        -sign true \
        -user user \
        -installer inf \
        -lan_ip mc29tst3 \
        -sta {W8X64-29-3} \
        -perfchans { 36l 36/80 } \
        -tcpwindow 4M \
        -brand win8x_internal_wl \
        -wlinitcmds {wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 0} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -power "npc41 1" \
        -power_button {auto}

W8X64-29-3 clone W8X64-293-TOT
W8X64-29-3 clone W8X64-293-BISON    -tag BISON_BRANCH_7_10    -brand win8x_internal_wl
#W8X64-29-3 clone W8X64-293-AARD-TOB -tag AARDVARK_BRANCH_6_30
}



###########################################
# 4358wlspie_P105 A1 PCIe 11ac 2x2 (a variation of 43569)
# (Obtained from Maneesh Mishra
###########################################
#	-app_tag trunk

UTF::DHD mc29tst4 \
        -lan_ip mc29tst4 \
        -sta {4358 eth0} \
        -power {npc41 2} \
        -app_tag DHDNC39RC65_BRANCH_1_47 \
        -dhd_tag DHDNC39RC65_BRANCH_1_47 \
        -dhd_brand linux-internal-dongle-pcie \
        -brand linux-external-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -nvram bcm94358wlspie_p105.txt \
        -slowassoc 5 \
        -datarate {-i 0.5 -frameburst 1} \
        -tcpwindow 2m -udp 800m \
        -nocal 1 -nopm1 1 -nopm2 1 \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -wlinitcmds {wl down; wl vht_features 3;}



4358 clone 4358b35 \
	-tag BISON05T_BRANCH_7_35 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin
#	-type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-sstput/rtecdc.bin

4358 clone 4358b-112 \
        -tag BISNC105RC66_BRANCH_7_112 \
        -nobighammer 1  -noaes 1 \
        -type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-xorcsum.bin
#        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin
#        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx5g-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-btcdyn-wepso-rcc-noccxaka-sarctrl-xorcsum-assert.bin


4358 clone 4358b35105RC \
        -tag BISNC105RC66_BRANCH_7_112 \
        -dhd_tag DHDNC39RC65_BRANCH_1_47 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin

4358 clone 4358b35105 \
	-tag BISON05T_TWIG_7_35_105 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin

4358 clone 4358BISON2 \
        -tag BISON_BRANCH_7_10 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin

4358b35 clone 4358b35x \
    -perfonly 1 -perfchans {36/80} \
    -type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-sstput/rtecdc.bin \
	-extsup 1

4358 clone 4358andrey \
        -image /projects/hnd_sw_mobfw/work/andrey/BISON05T_TWIG_7_35_105/4358a1-roml-pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx-autosf-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-proxd-shif-btcdyn-idsup-chkd2hdma-ssoption.bin \
	-wlinitcmds {wl vht_features 3; wl amsdu_autosf 0}



# Router (AP) section

# AP2: Netgear R6300/4706/4360 11ac 3x3 wireless router
UTF::Router 4360 -sta {
    4706/4360 eth2 4706/4360.%15 wl0.%
    4706/4331 eth1
    } \
    -lan_ip 192.168.1.201 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "mc29end1:40003" \
    -power {npc71 1} \
    -tag "BISON04T_BRANCH_7_14" \
    -nvram {
        # watchdog=3000 (default)
        lan_ipaddr=192.168.1.201
        lan1_ipaddr=192.168.2.1
        wl_msglevel=0x101
        wl0_ssid=4706/4360
        wl0_channel=1
        wl0_bw_cap=-1
        wl0_radio=0
        wl0_obss_coex=0
        wl1_ssid=4706/4331-mc29
        wl1_channel=36
        wl1_bw_cap=-1
        wl1_radio=0
        wl1_obss_coex=0
        #Only 1 AP can serve DHCP Addresses
        #router_disable=1
        et_msglevel=0; # WAR for PR#107305
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -noradio_pwrsave 1

# Clone for external 4360
4706/4360 clone 4706x/4360 \
    -sta {4706x/4360 eth1} \
    -brand linux26-external-vista-router-full-src \
    -perfonly 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1}

4706/4360 clone 4706/4360BISON    -tag "BISON_BRANCH_7_10"
4706/4360 clone 4706/4360BISON04T -tag "BISON04T_BRANCH_7_14"
4706/4360 clone 4706/4360TOT
4706/4360 clone 4706/4360_01T  -tag "AARDVARK01T_TWIG_6_37_14"

4706/4360 clone 4706/4360ARV   -tag "AARDVARK_BRANCH_6_30"
4706/4331 clone 4706/4331ARV   -tag "AARDVARK_BRANCH_6_30"

4706/4360 clone 4706/4360_714_89  -tag BISON04T_7_14_89



set ::UTFD::intermediate_sta_OS(0)    "FC15"
set ::UTFD::intermediate_sta_OS(1)    "FC15"
set ::UTFD::intermediate_sta_OS(2)    Win8x
set ::UTFD::intermediate_sta_OS(3)    Win8x
set ::UTFD::intermediate_last_svn(0)  0
set ::UTFD::intermediate_last_svn(1)  0
set ::UTFD::intermediate_last_svn(2)  0
set ::UTFD::intermediate_last_svn(3)  0

set ::UTFD::intermediate_sta_list(0)  fc15-29-TOT
set ::UTFD::intermediate_sta_list(1)  fc15-29-EAGLE
#set ::UTFD::intermediate_sta_list(2)  W8X64-29-TOT
#set ::UTFD::intermediate_sta_list(3)  W8X64-29-BISON735
set ::UTFD::intermediate_ap           "4706/4360"
set ::UTFD::intermediate_ap_name      "4360 AP"
set ::UTFD::max_STAindex_count        4
set ::UTFD::rigname                   "mc29"
set ::UTFD::driver_build_path         "/projects/hnd_sig_ext13/$::env(LOGNAME)/testbuildlist"
set ::UTFD::max_testbuild_age         48
set ::UTFD::svn_path_base             http://svn.sj.broadcom.com/svn/wlansvn/proj

#set ::UTFD::norestore 1


#set ::UTFD::testscripts #

#        UTFD::metascript %AUTO% -watch stanightly-TOT -script "./UTF.tcl /home/jpalte/src/tools/unittest/Test/StaNightly.test -utfconf mc29 -sta fc15-29--TOT -ap 4706/4360 -title '[mc29:StaNightly] 4360 AP to 4360 STA Linux Top of Tree' -nocache -perfonly" -type nightly -period 0.30 -watchinterval 10

# 00 01   *  *  *  /bin/bash /home/jpalte/md04-scripts/iptv-10-voice-test.script
#00   01   *  *  *  src/tools/unittest/cron/launch IPTV-10loop-voice Test/FTTRBasic.test -utfconf md04 -ap 4708psta2 -sta fc15-041-BISON -tos '0xC0' -w 4M -rcrestart -email 'hnd-utf-list' -dwell 1 -count 10 -load -chanspec 36/80 -tracelevel 'ampdu' -testdefaults -frameburst 1 -title 'MD04 IPTV 10 loop voice test BISON'
        
#        UTFD::metascript %AUTO% -watch 4708ap_twign -script "./UTF.tcl 4331lx1 load; /home/rmcmahon/UTF/iptv3x3/src/tools/unittest/Test/ConnectAPSTA.test -web 4708ap 4331lx1" -type nightly -period 0.30 -watchinterval 10

#        UTFD::metascript %AUTO% -watch IPTV-10loop-voice -script "./UTF.tcl fc15-041-BISON load;  /home/jpalte/src/tools/unittest/Test/FTTRBasic.test -web 4708psta2 fc15-041-BISON -tos '0xC0' -w 4M -rcrestart -email 'hnd-utf-list' -dwell 1 -count 10 -load -chanspec 36/80 -tracelevel 'ampdu' -testdefaults -frameburst 1 -title 'MD04 IPTV 10 loop voice test BISON'



# 00 04   *  *  *  /bin/bash /home/jpalte/md04-scripts/stanightly.script
#00 04     *  *  *  src/tools/unittest/cron/launch stanightly-TOT Test/StaNightly.test -utfconf md04  -sta fc15-041-TOT -ap 4708psta2 -title '[md04:StaNightly] PSTA2 STA1 TOT'


# 30 21   *  *  *  /bin/bash /home/jpalte/md04-scripts/stanightly-03T.script
#30 21     *  *  *  src/tools/unittest/cron/launch stanightly-03T Test/StaNightly.test -utfconf md04  -sta fc15-041-BISON03T -ap 4708psta2 -title '[md04:StaNightly] PSTA2 STA1 BISON 03T'



# 30 22   *  *  *  /bin/bash /home/jpalte/md04-scripts/iptv-full-test-03T.script
#30   22   *  *  *  src/tools/unittest/cron/launch IPTV-full-test-03T Test/FTTRBasic.test -utfconf md04 -ap 4708psta2 -sta fc15-041-BISON03T -dwell 1 -tos '0x0 0x80 0xc0' -w 4M -load  -tracelevel 'ampdu'  -chanspec 36/80 -rcrestart -testdefaults -frameburst 1 -email 'hnd-utf-list ericg ygong jihuac rmcmahon' -count 10 -title 'MD04:IPTV full test -03T '

#
