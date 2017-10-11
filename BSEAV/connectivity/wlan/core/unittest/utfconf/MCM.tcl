# -*-tcl-*-

#
# MCM testbed configuration
#

package require UTF::Linux

# Define power controllers on cart.
package require UTF::Power
UTF::Power::Synaccess npc10 -lan_ip 172.16.1.10 -relay MCMUTF

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext19/$::env(LOGNAME)/2017/MCM"

set ::UTF::SetupTestBedReboot {

	set rc 0
	set rc_msg ""
	
    foreach STA $::UTF::RebootList {
    
	    UTF::Try "STA $STA Check" {

	    	set reboot_flag 0
	    	
			# ping test
			set lan_ip [$STA cget -lan_ip]
			set lan_ip [string trim $lan_ip]
			if {$lan_ip == ""} {
				set lan_ip [$STA cget -name]
				set lan_ip [string trim $lan_ip]
			}
			set lan_ip [string tolower $lan_ip]
			UTF::Message INFO "" "STA=$STA lan_ip=$lan_ip"

			UTF::Message INFO "" "*************** Ping Test ****************"
			
			if {[string match -nocase "*linux*" $::tcl_platform(os)]} {
				# Linux
				set ping_options "-c 2"
			} else {
				#Windows
				set ping_options "-n 2"
			}

			set catch_resp [catch {exec ping $lan_ip $ping_options} catch_msg]
			UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

			if {$catch_resp != 0} {
				UTF::Message INFO "" "Ping failed for $STA $lan_ip. Reboot it"
				set reboot_flag 1
			} else {			
				UTF::Message INFO "" "Ping passed for $STA $lan_ip"
				
				UTF::Message INFO "" "*************** SSH Test ****************"

				set user_name [$STA cget -user]
				UTF::Message INFO "" "ssh -S none -o ConnectTimeout=20 $user_name\@$lan_ip pwd"
				set catch_resp [catch {exec ssh -S none -o ConnectTimeout=20 $user_name\@$lan_ip rm -f mem_dump_*} catch_msg]
				set catch_resp [catch {exec ssh -S none -o ConnectTimeout=20 $user_name\@$lan_ip rm -f debug_dump_*} catch_msg]
				set catch_resp [catch {exec ssh -S none -o ConnectTimeout=20 $user_name\@$lan_ip pwd} catch_msg]
				UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

				if {$catch_resp != 0} {
					UTF::Message INFO "" "SSH failed for $STA $lan_ip. Reboot it"
					set reboot_flag 1
				}
			}
			
			if {$reboot_flag == 1} {
				UTF::Message INFO "" "*************** Reboot ****************"
				
				set catch_resp [catch {$STA power off} catch_msg]
				UTF::Sleep 15
				set catch_resp [catch {$STA power on} catch_msg]
				UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"
				UTF::Sleep 120

				set catch_resp [catch {exec ping $lan_ip $ping_options} catch_msg]
				UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

				if {$catch_resp != 0} {
					UTF::Message INFO "" "***Failed. Ping failed after reboot for $STA $lan_ip"
					set rc -1
					append rc_msg "$STA reboot failed "
					error "Reboot Failed"
				} else {
					UTF::Message INFO "" "Ping passed after reboot for $STA $lan_ip"

					set catch_resp [catch {exec ssh -S none -o ConnectTimeout=20 root@$lan_ip pwd} catch_msg]
					UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

					if {$catch_resp != 0} {
						UTF::Message INFO "" "***Failed. SSH failed after reboot for $STA $lan_ip"
						set rc -1
						append rc_msg "$STA reboot failed "
						error "Reboot Failed"
					} else {
						UTF::Message INFO "" "SSH passed after reboot for $STA $lan_ip"
						append rc_msg "$STA rebooted "
						return "Rebooted"
					}
				}
			}
		}
    }
    
    if {$rc == -1} {
    	error $rc_msg
    } else {
    	return $rc_msg
    }
}

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
	set ::UTF::STAList "4357"
	set ::UTF::RebootList "$::UTF::STAList"
	set ::UTF::DownList "$::UTF::STAList"

	UTF::Try "Reboot Testbed" {
		eval $::UTF::SetupTestBedReboot
	}

    # To prevent inteference.
    foreach S "$::UTF::DownList" {
	    UTF::Try "$S Down" {
		    catch {$S wl down}
		    catch {$S deinit}
			return
	    }
    }
    # unset S so it doesn't interfere
    unset S  
    
    # delete myself
    unset ::UTF::SetupTestBed
    unset ::UTF::SetupTestBedReboot
    
    return
}

##########################################################################################
# UTF Endpoint - Traffic generators (no wireless cards)
##########################################################################################
UTF::Linux MCMUTF \
	-lan_ip 10.19.13.152 \
    -sta "lan p4p1"

lan configure -ipaddr 192.168.2.220

#-----------------------------------------------------------------------------------------
#						                   STAs 
#-----------------------------------------------------------------------------------------

##########################################################################################
#						STA 4357 PCIe Chip - Brix
##########################################################################################
#    -hostconsole lab4hnd-mirabadi:40051 \

# Guinness MuRata ES4.1 with B0 chip shield modules and diplexer rework
# bcm94357GuinnessMurataMM.txt
# wl and dhd from /projects/hnd/swbuild/build_linux/trunk/linux-combined-apps/2016.10.26.2/internal/bcm/x86_64/

UTF::DHD MCM4357 \
	-lan_ip 10.19.13.191 \
	-iperf iperf208 \
	-sta {4357 eth0} \
    -name "4357-DUT" \
	-power "npc10 1" \
	-power_button "auto" \
    -nvram_add {macaddr=00:90:4C:12:D0:02} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "src/shared/nvram/bcm94357GuinnessMurataMM.txt" \
    -clm_blob 4347b0.clm_blob \
    -type 4357b0-roml/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl band a;
	wl vht_features 7;
	wl country US/0;
	wl -w 1 interface_create sta;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}} \
    -perfchans {36/80} -channelsweep {-band a} \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" WARN
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
            "PHYTX error" WARN
            "MQ_ERROR" FAIL
            "segfault" FAIL
    }

4357 configure -ipaddr 192.168.2.236

######## NAN Slave ########
4357 clone 4357i-Nan \
	-postcopy {
		$self touch .iguana
		if {![catch {$self rm .trunk}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
	} \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357i-NanS" \
	-sta {4357i-Nan eth0 4357i-Nan-Aux wl0.2} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -clm_blob 4357_apolloe.clm_blob \
    -type 4357b0-roml/config_pcie_olympic_min_nan/rtecdc.bin \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
    -app_brand linux-combined-apps \
	-tag IGUANA_BRANCH_13_10
4357i-Nan configure -ipaddr 192.168.2.236    
4357i-Nan-Aux configure -ipaddr 192.168.5.236 -attngrp G3
######################
4357i-Nan clone 4357t-Nan \
	-postcopy {
		$self touch .trunk
		if {![catch {$self rm .iguana}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
	} \
	-name "4357t-NanS" \
	-sta {4357t-Nan eth0 4357t-Nan-Aux wl0.2} \
    -brand hndrte-dongle-wl \
	-dhd_brand linux-external-dongle-pcie \
    -clm_blob 4357{a0,}.clm_blob \
    -type 4357b0-ram/config_pcie_nan/rtecdc.bin \
    -nvram "bcm94357GuinnessMurataMM.txt" \
	-tag trunk
4357t-Nan configure -ipaddr 192.168.2.236    
4357t-Nan-Aux configure -ipaddr 192.168.5.236 -attngrp G3
######################

##########################################################################################
# Sigma Endpoint
##########################################################################################
UTF::Linux MCMSigmaEndpoint \
	-lan_ip 10.19.13.229 \
    -sta "SigmaEndpoint enp0s20u2"

##########################################################################################
# Sigma STA
##########################################################################################
UTF::Linux MCMSigmaSTA \
	-lan_ip 192.168.2.7 \
    -sta "SigmaSTA enp2s0"

##########################################################################################
# Sigma DUT
##########################################################################################
UTF::Linux MCMSigmaDUT \
	-lan_ip 192.168.2.8 \
    -sta "SigmaDUT enp2s0"

#####################################################################################################
#
#	Sigma Parameters:
#
#####################################################################################################

set ::sigma_tool_loc "/home/mirabadi/Yati"
set ::sigma_endpoint_name "SigmaEndpoint"
set ::sigma_sta_name "brix7"
set ::sigma_dut_name "brix8"

set ::sigma_blob_name "4357a0.clm_blob"
set ::sigma_nvram_name "bcm94357fcpagbe_p402.txt"

#####################################################################################################
#
#	Sigma Tests to run ( 0 = disabled  ,  1 = enabled )
#
#####################################################################################################

set ::sigma_test1_enabled 		1
set ::sigma_test1_name 			"NAN-5.3.1"
set ::sigma_test1_max_time		600

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat lab4hnd-mirabadi
# To check the params:
# 	ssh lab4hnd-mirabadi ps -wo pid,args -p 14666,8802,10227,11576,17628
##########################################################################################
UTF::Q MCM
