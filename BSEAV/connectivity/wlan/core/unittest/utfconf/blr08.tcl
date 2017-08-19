#Testbed configuration file for BLR08
#Edited by Rohit B on 13 May 2016 
#Last check-in 13 May 2015 

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

package require snit

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr08end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
		ALL {1 2 3}
               }

set ::UTF::SetupTestBed {
    G1 attn 0
    catch {G2 attn 15;}
    catch {G1 attn 15;}
	foreach S {4360ref 4361a0 43012 4355b3 4364b1} {
	catch {$S wl down}
	$S deinit
    }
    return
}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr08"


set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""

set ::cycle5G80AttnRange "0-95 95-0"
set ::cycle5G40AttnRange "0-95 95-0"
set ::cycle5G20AttnRange "0-95 95-0"
set ::cycle2G40AttnRange "0-95 95-0"
set ::cycle2G20AttnRange "0-95 95-0"

 UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
 set UTF::BuildFileServer repo
 set UTF::UseFCP nocheck

####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr08end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr08end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr08end1 -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.1.1.42 -relay blr08end1 -rev 1
UTF::Power::WebRelay  web43 -lan_ip 172.1.1.43 -invert 1
########################### Test Manager ################

UTF::Linux blr08end1 \
     -lan_ip 10.132.116.65 \
     -sta {lan eth0}


UTF::Linux blr08ref0 -sta {4360ref eth0} \
	-lan_ip 10.132.116.66 \
	-power "npc21 1" \
	-power_button "auto" \
	-reloadoncrash 1 \
	-tag EAGLE_BRANCH_10_10 \
	-brand "linux-internal-wl" \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
	-post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
	-wlinitcmds {wl msglevel +assoc; wl btc_mode 0; \
            service dhcpd stop; wl down; wl mimo_bw_cap 1; wl vht_features 3;
	}

4360ref configure -ipaddr 192.168.1.100 -ap 1  -attngrp G1  \

4360ref clone 4360sta \
	-ap 0 \
	-tcpwindow 2m -udp 800m \
	-wlinitcmds {wl msglevel +assoc;} \
	-yart {-attn5g 30-95 -attn2g 30-95} \
	-channelsweep {-usecsa -band b}  \

4360ref clone 4360b35 \
	 -tag BISON05T_BRANCH_7_35 \



UTF::Sniffer blr08ref1 -sta {snif eth0} \
    	-lan_ip 10.132.116.67 \
    	-power "npc31 1"\
    	-power_button "auto" \
    	-slowassoc 5 -reloadoncrash 1 \
    	-tag BISON_BRANCH_7_10 \
    	-brand "linux-internal-wl" \

    # -tcpwindow 4M \
    # -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \

# snif configure -ipaddr 192.168.1.101 -ap 1  -attngrp G2 \

########################################################################

UTF::DHD blr08tst1 \
	-lan_ip 10.132.116.68 \
	-sta {4355b3 eth0} \
        -power {npc41 1} \
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-brand linux-external-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-dhd_brand linux-internal-dongle-pcie \
	-nvram "../../olympic/C-4355__s-B3/P-simbab_M-SBLO_V-m__m-6.5.txt" \
	-dhd_tag DHD_BRANCH_1_359 \
	-app_tag DHD_BRANCH_1_359 \
	-clm_blob "4355_simba_b.clm_blob" \
	-type 4355b3-roml/config_pcie_debug/rtecdc.bin \
	-perfchans {36/80 36l 3} \
        -tcpwindow 3m \
	-udp 800m \
	-yart {-attn5g 30-95 -attn2g 30-95}\
	-wlinitcmds {wl down;wl country US;wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl ampdu_mpdu} \
	-pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl reset_cnts} } \
	-post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters} {%S wl ampdu_clear_dump}} \
	
	
4355b3 configure -ipaddr 192.168.1.109 \

4355b3 clone 4355b3-44 \
	-tag DIN2930R18_BRANCH_9_44 \


######################################################################
UTF::DHD blr08tst3 \
        -lan_ip 10.132.116.70 \
        -sta {4361a0 eth0} \
        -power {npc42 1} \
        -power_button "auto" \
	-tag JAGUAR_BRANCH_14_10 \
	-type 4361a0-ram/config_pcie_utf/rtecdc.bin \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand hndrte-dongle-wl \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
	-nvram "src/shared/nvram/bcm94361fcpagbss.txt" \
	-clm_blob 4347a0.clm_blob \
	-slowassoc 5 \
	-tcpwindow 4m \
	-udp 800m \
	-perfchans "36/80 3" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3} \
	-pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl band} {%S wl chanspec} {%S wl reset_cnts} } \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters} {%S wl ampdu_clear_dump} {%S wl band} {%S wl chanspec} {wl dump rsdb}} \

4361a0 configure -ipaddr 192.168.1.104 \



######################################################################

UTF::DHD blr08tst4 \
        -lan_ip 10.132.116.71 \
        -sta {4364b1 eth0} \
        -power {npc42 2} \
        -power_button "auto" \
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-app_tag DHD_BRANCH_1_359 \
	-dhd_tag DHD_BRANCH_1_579 \
	-nocal 1 -slowassoc 5 \
	-nvram "src/shared/nvram/bcm94364fcpagb_2.txt" \
	-brand hndrte-dongle-wl \
	-driver dhd-msgbuf-pciefd-debug \
	-dhd_brand linux-internal-dongle-pcie \
        -tcpwindow 4M \
	-udp 1600m \
        -reloadoncrash 1 \
	-clm_blob 4364b0.clm_blob \
	-channelsweep {-no2g40}  \
	-type 4364b1-roml/config_pcie_release_sdb_udm/rtecdc.bin \
        -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \
	-yart {-attn5g 30-95 -attn2g 30-95} \

4364b1 configure -ipaddr 192.168.1.91 \

4364b1 clone 4364b1-perf \
	-type 4364b1-roml/config_pcie_perf_sdb_udm/rtecdc.bin \

4364b1 clone 4364b1-trunk \
	-tag trunk \
	-app_tag trunk \
	-type 4364a0-ram/config_pcie_release_norsdb_row/rtecdc.bin \
	-perfchans {36/80 3} \
	
4364b1 clone 4364b2 \
	-type 4364b2-roml/config_pcie_release_sdb_udm/rtecdc.bin \

4364b2 clone 4364b2-perf \
	-type 4364b2-roml/config_pcie_perf_sdb_udm/rtecdc.bin \

########################################################################


UTF::DHD blr08tst2 \
        -lan_ip 10.132.116.69 \
        -sta {43012 eth0} \
        -power "npc41 2" \
        -power_button auto \
	-dhd_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-external-dongle-sdio \
        -nvram bcm943012fcbga.txt \
	-brand hndrte-dongle-wl \
	-tag FOSSA_BRANCH_11_10 \
	-app_tag DHD_BRANCH_1_579 \
	-type 43012a0-roml/threadx-sdio-ag-p2p-pool-pno-aoe-pktfilter-keepalive-proptxstatus-idsup-idauth-ulp-wowl-romuc-assert/rtecdc.bin \
        -tcpwindow 2m -udp 800m \
	-pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate}} \
	-wlinitcmds {wl msglevel +assoc; wl PM 0;} \
	-perfchans {3} \
	-yart {-attn5g 30-95 -attn2g 30-95} \
	-channelsweep {-usecsa}  \

43012 configure -ipaddr 192.168.1.122  -attngrp G1 \

43012 clone 43012h \
	-tag HORNET_BRANCH_12_10 \
	-type 43012a0-ram/config_sdio_idsup/rtecdc.bin \

43012h clone 43012ap \
	-ap 1 \
	-sta {43012ap eth0} \
	-type 43012a0-ram/config_sdio_p2p/rtecdc.bin \

43012 clone 43012t \
	-tag trunk \
	-type 43012a0-ram/config_sdio_noap/rtecdc.bin \

43012t clone 43012i \
	-tag IGUANA_BRANCH_13_10 \
	-type 43012a0-roml/config_sdio_p2p/rtecdc.bin \
	-clm_blob 43012a0.clm_blob \

43012i clone 43012api \
	-ap 1 \
	-sta {43012api eth0} \

43012i clone 43012i-pm \
	-type 43012a0-roml/config_sdio_release/rtecdc.bin \
	-clm_blob 43012a0.clm_blob \

43012t clone 43012apt \
	-ap 1 \
	-sta {43012apt eth0} \

43012h clone 43012pm \
	-type 43012a0-ram/config_sdio_pm/rtecdc.bin \

43012 clone 43012mfg \
	-type 43012a0-roml/threadx-sdio-ag-p2p-proptxstatus-mfgtest/rtecdc.bin \

43012i clone 43012B0 \
	-type 43012b0-roml/config_sdio_p2p/rtecdc.bin \
	-clm_blob 43012b0.clm_blob \
	-nvram "src/shared/nvram/bcm943012wlref_04.txt" \
	-extsup 1 \

43012B0 clone 43012B0-ap \
	-ap 1 \
	-sta {43012B0-ap eth0} \

43012B0 clone 43012B0-SStwig \
	-tag IGUANA_TWIG_13_10_69 \
	-dhd_tag DHD_TWIG_1_579_91 \
	-type 43012b0-roml/config_sdio_samsung/rtecdc.bin \

43012B0-SStwig clone 43012B0-SStwig-ap \
	-ap 1 \
	-sta {43012B0-SStwig-ap eth0} \

set ::43012_pathloss 20 \



set UTF::StaNightlyCustom {
	package require UTF::Test::DS1
	DS1 $Router $STA 
}



UTF::Q blr08

