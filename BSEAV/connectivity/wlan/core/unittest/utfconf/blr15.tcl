# -*-tcl-*-
#
# MuMIMO Testbed configuration file for blr15end1
#Edited Gopikrishnan R  28 Feb 2016 

####### Controller section:
# blr15end1: FC19 (10.132.116.106)
#
####### Router AP section:
#
# AP1: 4360softap 
#      
####### STA section:
#
# blr15tst1: 4366sb0FC19 enp1s0 (10.132.11660)
# blr15tst2: 4366sb0FC19 eth0 (10.132.11661)
#
######################################################## #



# Load Packages
package require UTF::Aeroflex
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
package require UTF::doc

set UTF::ChannelPerf 1
set UTF::Use11 1

# Define power controllers on cart.
package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr15end1 -rev 1

UTF::Power::Agilent ag1 -lan_ip "192.168.1.100 5024" -model "N6700B" -voltage 3.6 -channel 1

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

#Summary Directory
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr15"

set UTF::TrunkApps 1
# Define Attenuator
# G1  Channel 1 & 2 Attenuation between SoftAP and blr15ts1
# G2  Channel 3 & 4 Attenuation between blor02tst1 and blr15tst2   G2 not present in the path between softap and blr15tst1

# Define Attenuator
# G1  Channel 1 & 2 Attenuation between SoftAP and blr15ts1
# G2  Channel 3 & 4 Attenuation between blor02tst1 and blr15tst2   G2 not present in the path between softap and blr15tst1

UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr15end1" \
        -group {
                G1 {1 5}
                G2 {2 6}
				G3 {3 7}
                G4 {4 8}
				G5 {1 2 3 4 5 6 7 8}
				G6 {1 5 3 7}
				ALL {1 2 3 4 5 6 7 8}
				
               }
G1 configure -default 0
G2 configure -default 0
G5 configure -default 0
ALL configure -default 0 

set UTF::ChannelPerf 1
set UTF::Use11 1

####################################

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
				UTF::Message INFO "" "ssh $user_name\@$lan_ip ls"
				set catch_resp [catch {exec ssh $user_name\@$lan_ip ls} catch_msg]
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
				UTF::Sleep 60

				set catch_resp [catch {exec ping $lan_ip $ping_options} catch_msg]
				UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

				if {$catch_resp != 0} {
					UTF::Message INFO "" "***Failed. Ping failed after reboot for $STA $lan_ip"
					set rc -1
					append rc_msg "$STA reboot failed "
					error "Reboot Failed"
				} else {
					UTF::Message INFO "" "Ping passed after reboot for $STA $lan_ip"

					set catch_resp [catch {exec ssh root@$lan_ip ls} catch_msg]
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
    #
    # Make Sure Attenuators are set to 0 value
    #
   ALL attn default

      set ::UTF::APList "4366softap"
       set ::UTF::STAList "4366sa 4366sb 4366sc 4366sd"
       set ::UTF::DownList "4366softap $::UTF::APList $::UTF::STAList"

    # Make sure APs are on before testing
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    foreach AP "$::UTF::APList" {
            UTF::Try "$AP Radio Down" {
                        catch {$AP power on}
                UTF::Sleep 5
                        catch {$AP restart wl0_radio=0}
                        catch {$AP restart wl1_radio=0}
                        catch {$AP deinit}
                }
    }
    # unset S so it doesn't interfere
    unset AP  

    # To prevent inteference.
    foreach S "$::UTF::DownList" {
            UTF::Try "$S Down" {
                    catch {$S wl down}
                    catch {$S deinit}
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
UTF::Linux blr15end1 \
     -lan_ip 10.132.116.106 \
     -sta {lan em2} \
	 -power "npc21 2" \
     -power_button "auto" \



##########################################################################################
#                                  STA 4366sa0 Chip 
##########################################################################################

UTF::Linux blr15tst1 \
        -lan_ip 10.132.116.108 \
        -sta {4366sa enp1s0} \
        -power {npc11 2} \
        -power_button "auto" \
-brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
	-reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -tcpwindow 2m -udp 800m \
        -nocal 1 -slowassoc 15 \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl rsdb_mode 0;} \
		    	-msgactions {
            {ai_core_reset: Failed to take core} {
            	$self worry $msg;
            	$self power cycle;
            	return 1
            }
    	}

4366sa configure -attngrp G1

#rsdb mode
4366sa clone 4366sa1x1 \
         -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 1; wl rxchain 1} \

# 4366sa clone 4366sa1x1t \
	# -tag DIN07T48RC50_REL_9_75_145 \
    # -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode rsdb;}

# 4366sa clone 4366sa2x2t \
	# -tag DIN07T48RC50_BRNACH_9_75 \
	# -type 43596a0-roml/config_pcie_features2/rtecdc.bin \
    # -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode mimo;}

	#mimo mode
4366sa clone 4366sa2x2 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 3; wl rxchain 3} \

		#for Single User MIMO
4366sa clone 4366sa1x1su \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 1; wl rxchain 1; wl txbf_bfe_cap 1;}



UTF::Linux blr15tst2 \
        -lan_ip 10.132.116.109 \
        -sta {4366sb enp1s0} \
		-power {npc11 3} \
        -power_button "auto" \
        -brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
        -tcpwindow 2m -udp 800m \
        -nocal 1 -slowassoc 15 \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl rsdb_mode 0;} \

4366sb configure -attngrp G2

4366sb clone 4366sb1x1 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 1; wl rxchain 1} \

4366sb clone 4366sb-ht \
		-wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl vhtmode 0; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl txchain 1;wl rxchain 1}

4366sb clone 4366sb-lg \
		-wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl vhtmode 0; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl nmode 0; wl txchain 1;wl rxchain 1}

# 4366sb clone 4366sb1x1t \
	# -tag DIN07T48RC50_REL_9_75_145 \
    # -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode rsdb;}

# 4366sb clone 4366sb2x2t \
	# -tag DIN07T48RC50_BRNACH_9_75 \
	# -type 43596a0-roml/config_pcie_features2/rtecdc.bin \
    # -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode mimo;}

4366sb clone 4366sb2x2 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 3; wl rxchain 3} \

		#for Single User MIMO
4366sb clone 4366sb1x1su \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 1; wl rxchain 1; wl txbf_bfe_cap 1;}


UTF::Linux blr15tst3 \
        -lan_ip 10.132.116.179 \
        -sta {4366sc enp1s0} \
        -power {npc11 4} \
        -power_button "auto" \
-brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
        -tcpwindow 2m -udp 800m \
        -nocal 1 -slowassoc 15 \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl rsdb_mode 0;} \


4366sc configure -attngrp G3

4366sc clone 4366sc1x1 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 1; wl rxchain 1} \

4366sc clone 4366sc-ht \
		-wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl vhtmode 0; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl txchain 1;wl rxchain 1}
4366sc clone 4366sc-lg \
		-wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl vhtmode 0; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl nmode 0; wl txchain 1;wl rxchain 1}

# 4366sc clone 4366sc1x1t \
	# -tag DIN07T48RC50_REL_9_75_145 \
    # -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode rsdb;}

# 4366sc clone 4366sc2x2t \
	# -tag DIN07T48RC50_BRNACH_9_75 \
	# -type 43596a0-roml/config_pcie_features2/rtecdc.bin \
    # -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode mimo;}

4366sc clone 4366sc2x2 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 3; wl rxchain 3} \

		#for Single User MIMO
4366sc clone 4366sc1x1su \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 1; wl rxchain 1; wl txbf_bfe_cap 1;}



UTF::Linux blr15tst4 \
        -lan_ip 10.132.116.180 \
        -sta {4366sd enp1s0} \
        -power {npc11 5} \
        -power_button "auto" \
-brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
        -tcpwindow 2m -udp 800m \
        -nocal 1 -slowassoc 15 \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl rsdb_mode 0;wl country '#a/0'} \

4366sd configure -attngrp G4

4366sd clone 4366sd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 1; wl rxchain 1} \

4366sd clone 4366sd-ht \
		-wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl vhtmode 0; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl txchain 1;wl rxchain 1}
		
4366sd clone 4366sd-lg \
		-wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl vhtmode 0; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl nmode 0; wl txchain 1;wl rxchain 1}

# 4366sd clone 4366sd1x1t \
	# -tag DIN07T48RC50_REL_9_75_145 \
    # -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode rsdb;}

4366sd clone 4366sd2x2t \
	-tag DIN07T48RC50_BRNACH_9_75 \
	-type 43596a0-roml/config_pcie_features2/rtecdc.bin \
    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode mimo;}

4366sd clone 4366sd2x2 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 3; wl rxchain 3} 

		# for Single User MIMO
4366sd clone 4366sd1x1su \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;wl txchain 1; wl rxchain 1; wl txbf_bfe_cap 1;}



##########################################################################################
#                                 SoftAP1 4366
##########################################################################################
UTF::Linux AP \
        -lan_ip 10.132.116.107 \
        -sta {4366ap enp1s0} \
        -power_button "auto" \
		-power {npc11 1} \
        -brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
	-reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
	-modopts {assert_type=1 nompc=1} \
    	-msgactions {
            {ai_core_reset: Failed to take core} {
            	$self worry $msg;
            	$self power cycle;
            	return 1
            }
    	}

4366ap clone 4366softap-egl -tag EAGLE_BRANCH_10_10
4366softap-egl configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -hasdhcpd 1

4366ap clone 4366softap-mutx0 -tag EAGLE_BRANCH_10_10
4366softap-mutx0 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode su} \
	-modopts {assert_type=1 nompc=1 ctdma=0} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl country '#a/0'; wl vht_features 7;}

4366ap clone 4366atf-mutx0 
4366atf-mutx0 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode su} \
	-modopts {assert_type=1 nompc=1 ctdma=0} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl country '#a/0'; wl vht_features 7; wl atf 1} \
	
4366ap clone 4366softap-mutx1 -tag EAGLE_BRANCH_10_10
4366softap-mutx1 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
	-modopts {assert_type=1 nompc=1 ctdma=1} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl country '#a/0'; wl vht_features 7; wl atf 0} \
	-post_perf_hook {{%S wl txchain}} \

4366ap clone 4366twig122 -tag EAGLE_TWIG_10_10_122

4366softap-mutx1 clone 4366softap-mutx1-twig122 -tag EAGLE_TWIG_10_10_122

4366ap clone 4366atf-mutx1
4366atf-mutx1 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
	-modopts {assert_type=1 nompc=1 ctdma=1} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl country '#a/0'; wl vht_features 7; wl atf 1} \
	-post_perf_hook {{%S wl txchain}} 

4366softap-mutx0 clone 4366softap-mutx0-twig122 -tag EAGLE_TWIG_10_10_122
4366softap-mutx0-twig122 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode su} \
	-modopts {assert_type=1 nompc=1 ctdma=0} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl country '#a/0'; wl vht_features 7; wl atf 0} \

4366softap-mutx1 clone 4366softap-mutx1-twig122 -tag EAGLE_TWIG_10_10_122
4366softap-mutx1-twig122 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
	-modopts {assert_type=1 nompc=1 ctdma=1} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl country '#a/0'; wl vht_features 7; wl atf 1} \



# DONGLE AP

UTF::DHD DAP \
        -lan_ip 10.132.116.107 \
        -sta {4366dap eth0} \
        -power {npc11 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
		       -reloadoncrash 1 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -post_perf_hook {{%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin



4366dap clone 4366dap-su
4366dap-su configure -ipaddr 192.168.1.115 -attngrp G5 -ap 1 -hasdhcpd 1 \
        -nvram_add ctdma=0
4366dap clone 4366dap-mu
4366dap-mu configure -ipaddr 192.168.1.115 -attngrp G5 -ap 1 -hasdhcpd 1 \
		-nvram_add ctdma=1 \
        -rvrnightly {-mumode mu} \



##########################################################################################
# Cron job test locking queue
##########################################################################################
UTF::Q blr15

