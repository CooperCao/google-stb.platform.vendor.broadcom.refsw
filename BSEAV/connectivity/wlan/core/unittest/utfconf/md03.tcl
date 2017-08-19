# -*-tcl-*-
#
# Testbed configuration file for Milosz Zielinski's md03 
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::utils
package require UTF::DHD

# Set default to use wl from trunk; Use -app_tag to modify.
set UTF::TrunkApps 1

#package require UTF::Test::ConnectAPSTA

set ::env(UTFDPORT) 9978
package require UTFD


UTF::Linux xlinux

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/md03"

# Enable Windows TestSigning
set UTF::TestSigning 1

# Define power controllers on cart
# npc42 is on the shelf with the STAs
UTF::Power::Synaccess npc42 -lan_ip 172.5.5.42 -rev 1
# npc22 and npc33 are on the top shelf with the routers
UTF::Power::Synaccess npc22 -lan_ip 172.5.5.22 -rev 1
UTF::Power::Synaccess npc33 -lan_ip 172.5.5.33 -rev 1

# web190 not populated
#UTF::Power::WebRelay  web190 -lan_ip 192.168.1.90 -invert 1

# Attenuator - Aeroflex
#	-relay "md03end1"
#                ALL {1 2 3 4 5 6 7 8 9 10 11 12}
#		G4 {10 11 12} 
UTF::Aeroflex af -lan_ip 172.5.5.91 \
	-group {
		G1 {1 2 3} 
		G2 {4 5 6} 
		G3 {7 8 9} 
	       }

set K3 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Set attenuators to useful default value
    #
#    catch {ALL attn 0;}
    catch {G1 attn 0;}
    catch {G2 attn 0;}
    catch {G3 attn 0;}
#    catch {G4 attn 0;}

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
#    catch {4331-AP2 restart wl0_radio=0}
#    catch {4331-AP2 restart wl1_radio=0}
    catch {4321MP-AP2 restart wl0_radio=0}
    catch {4321MP-AP2 restart wl1_radio=0}
    # ensure sniffer is unloaded
    #catch {4331SNF1 unload}
    #catch {4360SNF1 unload}    

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4358A1  fc15-032 md03-43430b0} {
	    UTF::Try "$S Down" {
		    catch {$S wl down}
#		    catch {$S rexec iptables -F}
		    # deinit needs to be the last command in setup
                    catch {$S deinit}
	    }
    }
    # unset S so it doesn't interfere
    unset S
          
    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}

if { 0 } {
# Define Sniffer
UTF::Sniffer md03snf1 -user root \
        -sta {4331SNF1 eth1} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc13 1} \
        -power_button {auto} \
        -console "md03end1:40001"
}

# UTF Endpoint1 FC11 - Traffic generator (no wireless cards)
UTF::Linux md03end1 -lan_ip 10.22.20.134  \
    -sta {lan eth1} 

# UTF Endpoint2 FC11 - Traffic generator (no wireless cards)
#UTF::Linux md03end2 -lan_ip 10.22.20.135  \
#    -sta {lan eth1} 




###########################################
# 4358wlspie_P105 A1 PCIe 11ac 2x2 (a variation of 43569)
# (Obtained from Maneesh Mishra
###########################################
#	-app_tag trunk
#        -dhd_tag DHD_BRANCH_1_201

UTF::DHD md03tst1 \
        -lan_ip md03tst1 \
        -sta {4358A1 eth0} \
        -power {npc22 1} \
	-power_button "auto" \
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
        -wlinitcmds {wl down; wl vht_features 3;} \
        -wlinitcmds {wl down; wl bw_cap 2g -1}


4358A1 configure -attngrp G1

4358A1 clone 4358A1b35 \
	-tag BISON05T_BRANCH_7_35 \
        -notkip 1 -noaes 1 -nobighammer 1 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin
#        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx5g-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-btcdyn-wepso-rcc-noccxaka-sarctrl-xorcsum-assert/rtecdc.bin
#	-type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-sstput/rtecdc.bin
#	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert/rtecdc.bin
	#-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-proxd-nan-shif-tbow-assert/rtecdc.bin


4358A1b35 clone 4358A1b35x \
	-perfonly 1 -perfchans {36/80} \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth/rtecdc.bin

4358A1b35 clone 4358A1b35ssx \
	-perfonly 1 -perfchans {36/80} \
	-type 4358a1-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-sstput/rtecdc.bin

4358A1 clone 4358A1b35t \
	-tag BISON05T_TWIG_7_35_105 \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert/rtecdc.bin
	#-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-proxd-nan-shif-tbow-assert/rtecdc.bin

4358A1b35t clone 4358A1b35tx \
	-perfonly 1 -perfchans {36/80} \
	-type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth/rtecdc.bin

#	-type 4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-proxd-nan-shif-tbow-assert/rtecdc.bin
#4358A1 clone 4358A1BISON \
#	-tag BISON_BRANCH_7_10 \
#        -type 4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-assert/rtecdc.bin


4358A1 clone 4358A1BISON2 \
        -tag BISON_BRANCH_7_10 \
        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-aibss-relmcast/rtecdc.bin
#        -type 4358a1-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-amsdutx-wl11u-mfp-tdls-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-assert


#4358A1b clone 4358A1bx \
#	-perfonly 1 -perfchans {36/80} \
#	-type 4358a1-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth-proxd-nan-shif-tbow/rtecdc.bin



# md03tst2: Desktop DUT
# OS:  Linux FC15
# BCM: Linux 4359 A404
#
UTF::Linux md03tst2 \
        -sta "fc15-032 eth0" \
        -console "md03end1:40006" \
        -wlinitcmds {wl mpc 0; service dhcpd stop;:} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} } \
        -tcpwindow 4M \
        -power "npc42 2"


fc15-032  configure -attngrp G2

# Clones of STA fc15-032 with Different Options for Test
fc15-032 clone fc15-032-TOT
fc15-032 clone fc15-032-BISON    -tag BISON_BRANCH_7_10
fc15-032 clone fc15-032-EAGLE    -tag EAGLE_BRANCH_10_10





# md03tst3: Desktop DUT
# OS:  Linux FC15
# BCM: 43430 B0
#
#http://www.sj.broadcom.com/projects/hnd_swbuild/build_linux/DINGO07T_BRANCH_9_35/hndrte-dongle-wl/2015.5.14.1/build/dongle/43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-hs20sta/rtecdc.bin
#http://www.sj.broadcom.com/projects/hnd_swbuild/build_linux/DINGO2_BRANCH_9_15/hndrte-dongle-wl/2015.5.14.1/src/shared/nvram/bcm943430fsdng_Bx.txt
#         -nvram bcm943430wlselgs_26MHz.txt \

UTF::DHD md03tst3 \
        -lan_ip md03tst3 \
        -sta {md03-43430b0 eth0} \
        -hostconsole "md03end1:40003" \
        -power {npc42 1} \
        -power_button "auto" \
        -app_tag DHDNC39RC65_BRANCH_1_47 \
        -dhd_tag DHDNC39RC65_BRANCH_1_47 \
        -brand linux-external-dongle-sdio \
        -nvram bcm943430fsdng_Bx.txt \
        -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -tcpwindow 2m -udp 150m \
        -modopts {sd_uhsimode=1} \
        -wlinitcmds {wl down; wl bw_cap 2g -1}

md03-43430b0  configure -attngrp G1

#        -type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-hs20sta/rtecdc.bin
md03-43430b0 clone 43430b0-DINGO \
        -tag DINGO07T_BRANCH_9_35 \
        -type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls/rtecdc.bin

# this clone doesn't work
md03-43430b0 clone 43430b0-TOT \
	-tag trunk \
        -type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-hs20sta/rtecdc.bin


# md03tst4: Desktop DUT
# OS:  Linux FC15
# BCM: Linux 4360
#
UTF::Linux md03tst4 \
        -lan_ip md03tst4 \
        -sta "fc15-034-ap eth0" \
        -console "md03end1:40002" \
        -tcpwindow 4M \
        -power "npc42 3" \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
        -pre_perf_hook {{%S wl ampdu_clear_dump} } \
        -post_perf_hook {{%S wl dump rssi}} \
        -wlinitcmds {wl down; wl bw_cap 2 -1;}
#        -wlinitcmds {wl msglevel +assoc; wl country US/0; wl down; wl bw_cap 2 -1; wl vht_features 7;}

#fc15-034  configure -attngrp G2
fc15-034-ap clone fc15-034-ap-egl -app_date 2015.3.1.0
fc15-034-ap-egl configure -ipaddr 192.168.1.135 -attngrp G2 -ap 1 -hasdhcpd 1


if {0} {
## AP1
## Linksys E4200 4718/4331 Router AP1
UTF::Router AP1 \
    -sta "4331-AP1 eth2" \
    -lan_ip 192.168.1.1 \
    -relay "md03end1" \
    -lanpeer lan \
    -console "md03end1:40002" \
    -power "npc33 1" \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65"   \
        -nvram {
                boot_hw_model=E4200
                wandevs=et0
                {lan_ifnames=vlan1 eth1 eth2}
                et0macaddr=00:90:4c:07:00:8c
                macaddr=00:90:4c:07:00:9d
                sb/1/macaddr=00:90:4c:07:10:00
                pci/1/1/macaddr=00:90:4c:07:11:00
                lan_ipaddr=192.168.1.1
                lan_gateway=192.168.1.1
                dhcp_start=192.168.1.100
                dhcp_end=192.168.1.149
                lan1_ipaddr=192.168.2.1
                lan1_gateway=192.169.2.1
                dhcp1_start=192.168.2.100
                dhcp1_end=192.168.2.149
                fw_disable=1
                #Only 1 AP can serve DHCP Addresses
                #router_disable=1
                #simultaneous dual-band router with 2 radios
                wl0_radio=0
                wl1_radio=0
                wl1_nbw_cap=0
                wl_msglevel=0x101
                wl0_ssid=test4331-AP1-ant0
                wl1_ssid=test4331-AP1-ant1
                wl0_channel=1
                wl1_channel=1
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                # Used to WAR PR#86385
                wl0_obss_coex=0
                wl1_obss_coex=0
        }
}


# AP2
# Linksys E4200 4321MP Router
UTF::Router AP2 \
    -sta "4321MP-AP2 eth1" \
    -lan_ip 192.168.1.11 \
    -relay "md03end1" \
    -lanpeer lan \
    -console "md03end1:40005" \
    -power "npc22 2" \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65"   \
        -nvram {
                boot_hw_model=E4200
                wandevs=et0
                {lan_ifnames=vlan1 eth1 eth2}
                et0macaddr=00:90:4c:07:00:8f
                macaddr=00:90:4c:07:00:9f
                sb/1/macaddr=00:90:4c:07:10:00
                pci/1/1/macaddr=00:90:4c:07:11:00
                lan_ipaddr=192.168.1.11
                lan_gateway=192.168.1.11
                dhcp_start=192.168.1.100
                dhcp_end=192.168.1.149
                lan1_ipaddr=192.168.2.1
                lan1_gateway=192.169.2.1
                dhcp1_start=192.168.2.100
                dhcp1_end=192.168.2.149
                fw_disable=1
                #Only 1 AP can serve DHCP Addresses
                #router_disable=1
                #simultaneous dual-band router with 2 radios
                wl0_radio=0
                wl1_radio=0
                wl1_nbw_cap=0
                wl_msglevel=0x101
                wl0_ssid=test4321-AP2-ant0
                wl1_ssid=test4321-AP2-ant1
                wl0_channel=1
                wl1_channel=1
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                # Used to WAR PR#86385
                wl0_obss_coex=0
                wl1_obss_coex=0
        }


if {0} {

    -nvram {
            # Switch WRT600n LAN to vlan1, since vlan0 doesn't work
            "vlan1ports=1 2 3 4 8*"
            vlan1hwname=et0
            "landevs=vlan1 wl0 wl1"
            "lan_ifnames=vlan1 eth1 eth2"
            # Enable untagging on vlan2 else WAN doesn't work
            "vlan2ports=0 8u"
            et0macaddr=00:90:4c:11:00:8f
            macaddr=00:90:4c:07:00:9d
            lan_ipaddr=192.168.1.11
            lan_gateway=192.168.1.11
            dhcp_start=192.168.1.100
            dhcp_end=192.168.1.149
            lan1_ipaddr=192.168.2.1
            lan1_gateway=192.169.2.1
            dhcp1_start=192.168.2.100
            dhcp1_end=192.168.2.149
            fw_disable=1
            router_disable=1
            wl0_ssid=test4321-AP2-ant0
            wl0_channel=1
            wl0_radio=0
        }
}


set ::UTFD::max_STAindex_count        1


set ::UTFD::intermediate_sta_list(0)  fc15-032-TOT
set ::UTFD::intermediate_sta_OS(0)    "FC15"
set ::UTFD::intermediate_last_svn(0)  0
set ::UTFD::intermediate_sta_list(1)  fc15-032-EAGLE
set ::UTFD::intermediate_sta_OS(1)    "FC15"

set ::UTFD::intermediate_sta_list(2)  fc15-031-BISON
set ::UTFD::intermediate_sta_OS(2)    "FC15"
set ::UTFD::intermediate_last_svn(2)  0
set ::UTFD::intermediate_sta_list(3)  fc15-031-TOT
set ::UTFD::intermediate_sta_OS(3)    "FC15"
set ::UTFD::intermediate_last_svn(3)  0


set ::UTFD::intermediate_ap           "4321MP-AP2"
set ::UTFD::intermediate_ap_name      "4321 AP"
set ::UTFD::rigname                   "md03"
set ::UTFD::driver_build_path         "/projects/hnd_sig_ext13/milosz/testbuildlist"
set ::UTFD::max_testbuild_age         48
set ::UTFD::svn_path_base             http://svn.sj.broadcom.com/svn/wlansvn/proj

