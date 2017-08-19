#Testbed configuration file for BLR23
#Edited by Rohit B on 27 May 2016 
#Last check-in 27 May 2015 

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

package require snit

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
	-relay "blr23end1" \
	-group {
		G1 {1 2}
		G2 {3 4}
		ALL {1 2 3 4}
	       }

set ::UTF::SetupTestBed {
    catch {G2 attn 15;}
    catch {G1 attn 15;}
	foreach S {4360softap 4357-NanMaster 4357-NanSlave 4357-NanSpare} {
	catch {$S wl down}
	$S deinit
    }
    return
}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr23"

UTF::Linux blr23end1 \
	-lan_ip 10.132.116.186 \
	-sta {lan eth0} \


UTF::Linux blr23softap \
	-lan_ip 10.132.116.187 \
	-sta {4360softap enp1s0} \
	-power_button "auto" \
	-reloadoncrash 1 \
	-tag EAGLE_BRANCH_10_10 \
	-brand "linux-internal-wl" \
	-tcpwindow 4M \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
	-post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
	-wlinitcmds {wl msglevel +assoc; wl btc_mode 0; \
	    service dhcpd stop; wl down; wl mimo_bw_cap 1; wl vht_features 3;
	}

4360softap configure -ipaddr 192.168.1.100 -ap 1  -attngrp G2 \

##################################################################

UTF::DHD blr23tst1 \
	-lan_ip 10.132.116.188 \
	-sta {4357-NanMaster eth0} \
	-power_button "auto" \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
	-dhd_brand linux-internal-dongle-pcie \
	-dhd_tag DHD_BRANCH_1_579 \
	-driver dhd-msgbuf-pciefd-debug \
	-nvram "bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10 \
	-clm_blob 4347a0.clm_blob \
	-type 4357a0-ram/config_pcie_nan/rtecdc.bin \
	-wlinitcmds {dhd -i eth0 msglevel -msgtrace; wl down; wl country US/0; wl vht_features 7;} \
	-nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
	-yart {-attn5g 30-95 -attn2g 30-95 -pad 23} \
	-perfchans {36/80} -channelsweep {-band a} \

4357-NanMaster configure -ipaddr 192.168.1.234 \

####################################################################

UTF::DHD blr23tst2 \
	-lan_ip 10.132.116.189 \
	-sta {4357-NanSlave eth0} \
	-power_button "auto" \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
	-dhd_brand linux-internal-dongle-pcie \
	-dhd_tag DHD_BRANCH_1_579 \
	-driver dhd-msgbuf-pciefd-debug \
	-nvram "bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10 \
	-clm_blob 4347a0.clm_blob \
	-type 4357a0-ram/config_pcie_nan/rtecdc.bin \
	-wlinitcmds {dhd -i eth0 msglevel -msgtrace; wl down; wl country US/0; wl vht_features 7;} \
	-nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
	-yart {-attn5g 30-95 -attn2g 30-95 -pad 23} \
	-perfchans {36/80} -channelsweep {-band a} \

4357-NanSlave configure -ipaddr 192.168.1.236 \

####################################################################

UTF::DHD blr20tst1 \
	-lan_ip 10.132.116.140 \
	-sta {4357-NanSpare eth0} \
	-power_button "auto" \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
	-dhd_brand linux-internal-dongle-pcie \
	-dhd_tag DHD_BRANCH_1_579 \
	-driver dhd-msgbuf-pciefd-debug \
	-nvram "bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10 \
	-clm_blob 4347a0.clm_blob \
	-type 4357a0-ram/config_pcie_nan/rtecdc.bin \
	-wlinitcmds {dhd -i eth0 msglevel -msgtrace; wl down; wl country US/0; wl vht_features 7;} \
	-nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
	-yart {-attn5g 30-95 -attn2g 30-95 -pad 23} \
	-perfchans {36/80} -channelsweep {-band a} \

4357-NanSpare configure -ipaddr 192.168.1.235 \


UTF::Q blr23