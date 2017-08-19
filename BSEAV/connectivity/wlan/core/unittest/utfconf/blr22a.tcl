# Testbed Configuration for blr22end1 UTF StaNightly TestStation
# Created By Vishnu G Warrier on 01.09.2015

# ------------ Controller Section ---------------- #
# blr22end1 : FC_19
# IP Addr   : 10.132.116.144
# NETMASK   : 255.255.254.0
# GATEWAY   : 10.131.80.1
# ------------------------------------------------ #

# --------------- SoftAP Section ----------------- #
# blr22ref0 : FC_19 4360mc_1 (99)  (10.132.116.145)  
# blr22ref1 : FC_19                (10.131.80.132)                       
# ------------------------------------------------ #

# ------------------Sta Section ------------------ #
# blr22tst1 : FC_19 43012     eth0 (10.132.116.147)
# blr22tst2 : FC_19 4355b2    eth0 (10.131.80.142)
# blr22tst3 :                 eth0 (10.131.80.145)
# blr22tst4 :                 eth0 (10.131.80.146)
# ------------------------------------------------ #

# --------------------------------****---------------------------------- #

# ----- Load Packages ----- #

package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Vaunix
package require UTF::Airport

# ---------------------- AeroFlex Attenuator --------------- #

UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr22end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
				G3 {7 8 9}
		        ALL {1 2 3 4 5 6 7 8 9}
                }
ALL configure -default 20
# G1 configure -default 90
# G2 configure -default 90
# # Default Testbed Configuration options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 0
    catch {G2 attn 15;}
    catch {G1 attn 15;}
	foreach S {4360 43012a0} {
	catch {$S wl down}
	$S deinit
    }
	ALL attn default
    return
}
# # 'SummaryDir' sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr22"


# # Pointing Apps to Trunk 
set ::UTF::TrunkApps 1 \

set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \


	  
# ----------------------- Power Controllers ---------------- #
# package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr22end1 -rev 1

# ------------------------ Test Manager -------------------- #
UTF::Linux blr22end1 \
     -lan_ip 10.132.116.144 \
	 -sta {lan em1} \

# -------------------------- blr22ref0 --------------------- #
# 
# ---------------------------------------------------------- #

UTF::Linux blr22ref0 -sta {4360 enp1s0} \
     -lan_ip 10.132.116.145 \
	 -power "npc11 1" \
     -power_button "auto" \
     -reloadoncrash 1 \
     -tag BISON_BRANCH_7_10 \
	 -brand "linux-internal-wl" \
	 -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
     -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
     -wlinitcmds {wl msglevel +assoc; wl dtim 3;wl down; wl vht_features 3;} \

	
4360 configure -ipaddr 192.168.1.100 -ap 1  -attngrp G1 \

 

# -------------------------- blr22tst1 --------------------- # 
# 
# ---------------------------------------------------------- #

UTF::DHD blr22tst1 \
        -lan_ip 10.132.116.147 \
        -sta {43012a0 eth0} \
        -power {npc11 8} \
        -power_button auto \
		-dhd_tag DHD_BRANCH_1_363 \
		-dhd_brand linux-external-dongle-sdio \
		-app_tag DHD_BRANCH_1_363 \
        -nvram bcm943012fcbga.txt \
		-brand hndrte-dongle-wl \
		-tag FOSSA_BRANCH_11_10 \
		-type 43012a0-roml/threadx-sdio-ag-p2p-pool-pno-aoe-pktfilter-keepalive-proptxstatus-idsup-idauth-ulp-wowl-romuc-assert/rtecdc.bin \
	    -wlinitcmds {wl msglevel +assoc; wl PM 0} \
		-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
		-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate}} \
		-perfchans {36 3} \
		-yart {-attn5g 30-95 -attn2g 30-95} \
		-channelsweep {-usecsa}  \
		
		#
		#-app_brand linux-external-dongle-sdio \
		
43012a0 configure -ipaddr 192.168.1.120 \

43012a0 clone 43012mfg \
        -type 43012a0-roml/threadx-sdio-ag-p2p-proptxstatus-mfgtest/rtecdc.bin \
		
43012a0 clone 43012a0h \
        -tag HORNET_BRANCH_12_10 \
		-type 43012a0-ram/config_sdio/rtecdc.bin \
		-perfchans {3} \
		-channelsweep {-usecsa -band b} \
		
# -------------------------- blr22tst1 --------------------- # 
# 
# ---------------------------------------------------------- #	

UTF::DHD blr22tst2 \
        -lan_ip 10.132.116.148 \
        -sta {4355b2 eth0} \
        -power {npc11 7} \
        -power_button auto \
		-dhd_brand linux-external-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag DIN2915T165R6_BRANCH_9_41 \
        -nvram "bcm94355fcpagb.txt" \
        -nocal 1 -slowassoc 5 -reloadoncrash 1 \
        -udp 800m  \
	    -tcpwindow 8m \
	    -pre_perf_hook {{%S wl rssi 00:10:18:E2:F6:E2}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi} {%S wl nrate} {%S wl chanspec} {%S wl dump rsdb}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl chanspec} {%S wl dump rsdb}} \
        -yart {-attn5g 05-95 -attn2g 30-95} \
		-brand hndrte-dongle-wl \
	    -dhd_tag trunk \
	    -app_tag trunk \
	    -type 4355b2-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-sr-awdl-ndoe-pf2-mpf-pwrstats-wl11u-noclminc-ampduhostreorder-die0-err-assert-norsdb/rtecdc.bin \
        -wlinitcmds {wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl rsdb_mode 0;wl ampdu_mpdu} \
        -perfchans {36/80 36l 36 3} \
		
		4355b2 configure -ipaddr 192.168.1.130 \
	          
		
set ::43012a0_pathloss 10 \
		

UTF::Q blr22a
