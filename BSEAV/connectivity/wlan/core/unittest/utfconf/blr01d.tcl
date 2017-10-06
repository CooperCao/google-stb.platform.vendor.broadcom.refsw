#Testbed configuration file for blr01end1 UTF ACT Teststation
#Created by Anuj Gupta on 23SEPT2014 10:00 PM  
#Last checkin 27 Oct 2014 
####### Controller section:
# blr01end1: FC15
# IP ADDR 10.131.80.47
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP section:
#
# blr05ref0 : FC 15 4360mc_1(99)  10.131.80.12 
# blr05ref1 : FC 15 4360mc_1(99)  10.131.80.13
#
####### STA section:
#
# blr05tst1: FC 15 43224 eth0 (10.131.80.14)
# blr05tst2: FC 15 43228 eth0 (10.131.80.15)
# blr05tst3: FC 15 43217 eth0 (10.131.80.17)
# blr05tst4: FC 15 4331  eth0 (10.131.80.18)
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr01end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
		ALL {1 2 3}
                }
#G1 configure default 0
#G2 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 0
    catch {G2 attn 0;}
    catch {G1 attn 0;}
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr01"

#pointing Apps to trunk
#set ::UTF::TrunkApps 1 \


# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""


####################### Power Controllers ################

package require UTF::Power

UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr01end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr01end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr01end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr01end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.1.1.42 -relay blr01end1.ban.broadcom.com -rev 1

########################### Test Manager ################

UTF::Linux blr01end1 \
     -lan_ip 10.131.80.47 \
     -sta {lan eth0}

############################ blr01softap ##########################################
# blr01softap      - 4360mc_1(99)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3 
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 1 - STE 4450
# Power          - npc 31 port 1    172.1.1.31
################################################################################ 
UTF::Linux blr01softap -sta {4360 eth1}  \
    -lan_ip 10.131.80.48 \
    -power "npc11 1" \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
    -wlinitcmds {
                 wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
                 wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
                }
4360 configure -ipaddr 192.168.1.100 -ap 1  -attngrp G1 \

################################ blr01tst2 ######################################
# blr01tst2      - 4345
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power          - npc21 port 2     172.1.1.11
################################################################################

UTF::Linux blr01tst2 \
        -lan_ip 10.131.80.50 \
        -sta {4345l eth0} \
        -power "npc21 1"\
        -power_button "auto" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
		-type debug-apdef-stadef-p2p-mchan-tdls \
		-tag BIS120RC4PHY_BRANCH_7_16 \
        -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

4345l configure -ipaddr 192.168.1.91 \

UTF::DHD blr01tst4 \
	-lan_ip 10.131.80.50 \
	-sta {4345d eth0} \
    -power {npc21 1} \
    -tag BISON_BRANCH_7_10 \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94345fcpagb_epa.txt" \
    -type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-cca-pwrstats-wnm-wl11u-anqpo-noclminc-pktctx-err-assert/rtecdc.bin \
    -wlinitcmds {wl down;wl vht_features 3} \
    -slowassoc 5 -escan 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -tcpwindow 1152k -udp 400m -nocal 1 -docpu 1 -nointerrupts 1 \
    -yart {-attn5g 7-63 -attn2g 17-63 -pad 44 -frameburst 1} \
    -msgactions {
	"DTOH msgbuf not available" FAIL
    }
4345d configure -ipaddr 192.168.1.92 \

# Olympic uses the internal supplicant in production
4345d clone 4345b -tag BIS120RC4PHY_BRANCH_7_16 \
    -type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap/rtecdc.bin \
    -customer olympic \
    -wlinitcmds {wl down;wl vht_features 3;wl country XZ/35}

4345b clone 4345b.1 -sta {_43451b1o eth0 4345b.1 wl0.1} \
    -apmode 1 -nochannels 1 -slowassoc 5 \
    -wlinitcmds {wl down;wl vht_features 3;wl apsta 1;wl ssid -C 1 43451b1AP} \
    -noaes 1 -notkip 1 -yart {}

4345b.1 configure -ipaddr 192.168.1.93

4345b clone 4345bx \
    -customer olympic \
    -type ../C-4345__s-B1/tempranillo.trx -clm_blob tempranillo.clmb \
    -wlinitcmds {wl down;wl vht_features 3;wl country XZ/202} \
    -perfonly 1 -perfchans {36/80} -app_tag BIS120RC4PHY_BRANCH_7_16


UTF::Q blr01d
