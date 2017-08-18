# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC59
# CoEx testbed configuration
#
# DUT: 43362 ;# replaces 4336B1 which is EoL

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut mc59_BTCohost2		      	;# BlueTooth device under test
set ::bt_ref mc59_BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 4335c0_epa_01T 		    ;# HND WLAN device under test
set ::wlan_rtr mc59_AP1   				;# HND WLAN router
set ::wlan_tg lan0              		;# HND WLAN traffic generator

# loaned ip addresses to MC54:
# 10.19.87.102 -- bcm94334 host
# 10.19.87.101 -- BTCohost
# 10.19.87.124 -- WebRelay DUT2

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
# moved to dedicated filer volume 10/30/13
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc59"

# Define power controllers on cart.
# package require UTF::Power
UTF::Power::Synaccess npc_DUT -lan_ip 172.16.59.20
UTF::Power::Synaccess npc_DUT2 -lan_ip 172.16.59.25
UTF::Power::Synaccess npc_Ref -lan_ip 172.16.59.30
UTF::Power::Synaccess npc_AP -lan_ip 172.16.59.40
UTF::Power::Synaccess npc_AP2 -lan_ip 172.16.59.60 -rev 1
UTF::Power::WebRelay 10.19.87.104 ;# WebRelay DUT
UTF::Power::WebRelay 10.19.87.103
UTF::Power::WebRelay 10.19.87.124 ;# WebRelay DUT2

# Attenuator
# G1: WLAN
# G2: BT
# G3: 11ac
UTF::Aeroflex af -lan_ip 172.16.59.10 -group {G1 {1 2 3} G2 {6} G3 {4 5} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
	G1 attn 20
    G2 attn 3
	G3 attn 103

   foreach A {mc59_AP1} {
	  catch {$A restart wl0_radio=0}
    }
    
    # silence DUT radios
    foreach S {43342h 4356} {
		if {[catch {$S wl down} ret]} {
	        error "SetupTestBed: $S wl down failed with $ret"
	    }
	    $S deinit
	}
    
	catch {mc59DUT rexec initctl start consoleloggerUSB}	
    
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# mc59end1  10.19.87.95   Linux UTF Host

UTF::Linux mc59end1 -sta "lan0 eth1" -lan_ip 10.19.87.95

### lan0 configure -ipaddr 192.168.1.144
# -lan_ip 10.19.87.95
#

# mc59DUT2: 4335c0 epa SDIO, mc60end5
#

### 4335c0 epa device; hostname mc60end5, lan_ip 10.19.87.109
### changed to mc59end5 ip addr 10.19.85.180 8/7/14

UTF::DHD mc59DUT2\
	-lan_ip 10.19.85.180\
 	-sta "4335c0 eth1"\
    -tag AARDVARK_BRANCH_6_30 \
    -customer "bcm"\
    -dhd_tag NIGHTLY -slowassoc 5 \
    -brand linux-external-dongle-sdio \
    -dhd_brand linux-internal-dongle \
    -nvram "bcm94339wlbgaFEM_AM.txt" \
    -modopts {sd_uhsimode=2 sd_txglom=1 sd_tuning_period=10} \
    -type 4339a*-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-ampduhostreorder-ve-mfp-okc-txbf-relmcast*-idsup-idauth.bin\
    -datarate {-b 250m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3} \
    -nocal 1 -noaes 1 -notkip 1\
    -tcpwindow 1152k -udp 200m\
	-console 10.19.85.180:40001\
	-power "npc_DUT2 2"\
	-device_reset "10.19.87.124 1"\
    -postinstall {dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 10} \
    -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl bw_cap 2g -1}
         

4335c0 clone 4335c0_epa
4335c0_epa clone 4335c0_epaF -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10}
4335c0_epa clone 4335c0_epa_coex -udp 400m -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl btc_mode 1} -datarate {-b 350m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3} 
4335c0_epa_coex clone 4335c0_epa_coexd -dhd_date 2013.10.4
4335c0_epa clone 4335c0_epa_dbg -dhd_date 2013.7.3 -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10}
4335c0_epa_dbg configure -ipaddr 192.168.1.153
4335c0_epa_dbg clone 4335c0_epa_dbg_coex -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl bw_cap 2g -1; wl btc_mode 1}
4335c0_epa clone 4335c0_epa_01T\
	-tag AARDVARK01T_REL_6_37_*\
	-type 4339a*-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-autoabn-ampduhostreorder-ve-mfp-okc-txbf-idsup-idauth.bin
4335c0_epa clone 4335c0_epadhd199 -dhd_tag DHD_REL_1_99 -dhd_brand ""
4335c0_epa_01T clone 4335c0_epa_01T_fix -dhd_image /home/pkwan/dbg_tmp/dhd_r409486/dhd.ko
4335c0_epa_01T clone 4335c0_epa_01T_dbg -dhd_date 2013.7.3 -tag AARDVARK01T_REL_6_37_*
4335c0_epa_01T clone 4335c0_epa_01T_coex -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl bw_cap 2g -1; wl btc_mode 1}
4335c0 clone 4335c0_mod2 -modopts {sd_uhsimode=2 sd_txglom=1 sd_tuning_period=10}

4335c0 clone 4335c0_ipa -nvram src/shared/nvram/bcm94339wlipa.txt

4335c0 clone 4335b0\
    -tag AARDVARK_BRANCH_6_30 \
    -dhd_tag NIGHTLY \
    -brand linux-external-dongle-sdio \
    -dhd_brand linux-internal-dongle \
    -nvram "bcm94335wlbgaFEM_AA.txt" \
    -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
    -type 4335b0-roml/sdio-ag-pool-p2p-idsup-idauth-pno-pktfilter-keepalive-aoe-ccx-sr-{vsdb,mchan}-proptxstatus-lpc-autoabn-txbf\
    -datarate {-b 350m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3} \
    -nocal 1 -nopm1 1 -nopm2 1 \
    -postinstall {dhd -i eth1 msglevel 0x20001; dhd -i eth1 txglomsize 10} \
    -wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1; wl ampdu_mpdu 24; wl vht_features 1} \
    -tcpwindow 1152k

4335b0 clone 4335b0x\
 -dhd_tag DHD_REL_1_61\
 -dhd_brand linux-external-dongle-sdio

### RF comparison only
4335c0 clone 4350b1\
   -tag AARDVARK_BRANCH_6_30\
   -brand linux-external-dongle-sdio\
   -dhd_tag NIGHTLY -slowassoc 5\
   -dhd_brand linux-internal-dongle\
   -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10}\
   -type "4350b1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth-assert-err.bin"\
   -wlinitcmds "wl ampdu_mdpu 16; wl vht_features 3; wl bw_cap 2g -1"\
   -tcpwindow 2m -udp 400m \
   -nobighammer 1 -nocal 1\
   -postinstall "dhd -i eth1 sd_blocksize 2 256; dhd -i eth1 txglomsize 40"\
   -nvram "bcm94350wlagbe_KA.txt"\
   -datarate {-b 350m -i 0.5 -frameburst 1}

4350b1 clone 4350b1_coex -wlinitcmds "wl ampdu_mdpu 16; wl vht_features 3; wl bw_cap 2g -1; wl btc_mode 1"
4350b1_coex clone 4350b1_Bison_coex -tag BISON_REL_7_10_26 -type 4350b1-ram/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-lpc-wl11u-txbf-pktctx-dmatxrc.bin
4350b1 clone 4350b1_Bison_ToB \
   -tag BISON_BRANCH_7_10 \
   -type 4350b1-ram/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth-assert
4350b1_Bison_ToB clone 4350b1_Bison_ToB_coex -wlinitcmds "wl ampdu_mdpu 16; wl vht_features 3; wl bw_cap 2g -1; wl btc_mode 1"
4350b1_Bison_ToB clone 4350b1_Bison_ram -tag BISON_{REL,TWIG}_7_10_* -type 4350b1-ram/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth-assert

#4350c1 for RF comparison
4350b1 clone 4350c1\
  -tag BISON_BRANCH_7_10\
  -type 4350c1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin
4350c1 clone 4350c1_coex -wlinitcmds "wl ampdu_mdpu 16; wl vht_features 3; wl bw_cap 2g -1; wl btc_mode 1"  
4350c1 clone 4350c1_twig\
  -tag BISON_TWIG_7_10_82\
  -type 4350c1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-tdls-ccx-ve-mfp-idsup-idauth 
4350c1_twig clone 4350c1_twig_coex -wlinitcmds "wl ampdu_mdpu 16; wl vht_features 3; wl bw_cap 2g -1; wl btc_mode 1"

4350c1 clone 4350c1_full -datarate {-b 350m -i 0.5 -frameburst 1}
4350c1_twig clone 4350c1_twig_full -datarate {-b 350m -i 0.5 -frameburst 1}

### end RF comparison

# hostname: mc59end3

UTF::WinBT MC59_BTCOHOSTXP2 \
	-lan_ip 10.19.87.97 \
	-sta "mc59_BTCohost2"\
	-power "npc_DUT2 1"\
	-device_reset "10.19.87.124 2"\
	-bt_comm "com8@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -brand ""\
    -type BCM43362\
    -bt_ss_location 0x00226000\
	-type BCM2076/B1\
	-brand Nokia/UART/BringupBoard_3Wire\
    -version *\
    -file *.cgs
    
mc59_BTCohost2 clone BTCohost2
mc59_BTCohost2 clone BTCohost43362    
   
mc59_BTCohost2 clone BTCohost2_local\
    -project_dir "/home/pkwan"\
    -type BCM43362\
    -version *\
    -brand ""\
    -file "BCM2076B1_002.002.004.0032.0000_3wire.cgs"

mc59_BTCohost2 clone Cohost4335c0_ipa\
	-bt_comm "com15@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -bt_ss_location 0x00218000\
	-type BCM4339\
	-brand "Generic/UART/37_4MHz/wlbga_iPA"\
	-version *

Cohost4335c0_ipa clone Cohost4335c0_epa\
	-bt_comm "com15@3000000"\
	-brand "Generic/UART/37_4MHz/wlbga_ePA"\
	-type BCM4339\
	-version *	

Cohost4335c0_epa clone Cohost4335c0_epa_cfg60\
	-version *60.*
	
Cohost4335c0_epa clone Cohost4335c0_epa_cfg61\
	-version *61.*
	
Cohost4335c0_epa clone Cohost4335c0_epa_local\
	-project_dir "/home/pkwan"\
	-brand "*/UART/37_4MHz/wlbga_ePA"\
	-type BCM4339\
	-version *	
	
# #  hostname mc59end2; ip 10.19.87.96

UTF::DHD mc59DUT \
	-lan_ip 10.19.87.96\
 	-sta "43241a0sdio eth1"\
 	-power "npc_DUT 1"\
	-device_reset "10.19.87.104 1"\
 	-tag "PHOENIX2_BRANCH_6_10"\
    -brand "linux-internal-dongle"\
    -customer "bcm"\
 	-type "43241a0-roml/sdio-ag-p2p-idauth-idsup-pno.bin"\
    -nvram "src/shared/nvram/bcm94324ipaagb_p111.txt"\
    -pre_perf_hook {{mc59_AP1 wl ampdu_clear_dump}}\
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {mc59_AP1 wl dump ampdu} {mc59_AP1 wl nrate} {mc59_AP1 wl rate} {mc59_AP1 wl dump rssi} }

# clones of mc59DUT
43241a0sdio clone 4324a0sdio -type "4324a0-roml/sdio-ag-p2p-idauth-idsup-pno.bin"
43241a0sdio clone 43241a0min -type "43241a0min-roml/sdio-ag-p2p-pno-idsup-sr-idauth-proptxstatus.bin"
43241a0sdio clone 4324a0bu -brand linux-external-dongle-sdio -type "4324a0-roml/sdio-ag-idsup.bin"
43241a0sdio clone 43241Ref -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"
43241a0sdio clone 43241b0Ref\
   -type "43241b0-roml/sdio-ag-p2p-pno-pktfilter-keepalive-aoe-ccx-extsup-sr-mchan-vsdb-proptxstatus.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -postinstall {dhd -i eth1 serial 1}
43241a0sdio clone 43241b0min -type "43241b0min-roml/sdio-ag-p2p-pno-idsup-sr-idauth-proptxstatus.bin"
43241a0sdio clone 43241b0 -tag PHOENIX2_REL_6_10_79 -brand linux-external-dongle-sdio -type "43241b0min-roml/sdio-ag-p2p-pno-pktfilter-keepalive-aoe-ccx-extsup-sr-mchan-vsdb-proptxstatus.bin"
43241a0sdio clone 43241b0Int\
   -tag PHOENIX2_REL_6_10_??\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-p2p-pno-pktfilter-keepalive-aoe-ccx-extsup-sr-mchan-vsdb-proptxstatus.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -postinstall {dhd -i eth1 serial 1}
   
#   -tag PHOENIX2_REL_6_10_79\
   
43241a0sdio clone 43241b0ToB\
   -tag PHOENIX2_BRANCH_6_10\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -postinstall {dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}
#     -type "43241b0min-roml/sdio-ag-p2p-idauth-idsup-pno.bin" 


43241a0sdio clone 43241b0ToB_opt2\
   -tag PHOENIX2_BRANCH_6_10\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr-opt2.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p100.txt"\
   -console mc59end2:40001\
   -postinstall {dhd -i eth1 serial 1; dhd -i eth1 sd_divisor 1}\
   -datarate {-skiptx 32}
#     -type "43241b0min-roml/sdio-ag-p2p-idauth-idsup-pno.bin" 

43241a0sdio clone 43241b0ToBF\
   -tag PHOENIX2_BRANCH_6_10\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -postinstall {dhd -i eth1 serial 1; dhd -i eth1 sd_divisor 1}\
   -wlinitcmds {wl btc_mode 0}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0ToBw\
   -tag PHOENIX2_BRANCH_6_10\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -postinstall {dhd -i eth1 serial 1}\
   -wlinitcmds {wl btc_mode 0}\
   -datarate {-skiptx 32}   

43241a0sdio clone 43241b0ToBRvR\
   -tag PHOENIX2_BRANCH_6_10\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}

43241a0sdio clone 43241b0ToBRvR_P100\
   -tag PHOENIX2_BRANCH_6_10\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p100.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0ToBRvR_P100_Exp\
   -tag PHOENIX2_BRANCH_6_10\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-g-idsup.bin"\
   -nvram "bcm943241ipaagb_p100.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}

43241a0sdio clone 43241b0RvR_SVT\
   -tag PHOENIX2_REL_6_10_116_*\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-p2p-pno-pktfilter-keepalive-aoe-idsup-sr-idauth-vsdb-dmatxrc-proptxstatus.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0RvRF_SVT\
   -tag PHOENIX2_REL_6_10_116_*\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-p2p-pno-pktfilter-keepalive-aoe-idsup-sr-idauth-vsdb-dmatxrc-proptxstatus.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 sd_divisor 1}\
   -datarate {-skiptx 32}   

43241a0sdio clone 43241b0RvRF_SVTx\
   -tag PHOENIX2_REL_6_10_116_*\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-p2p-pno-pktfilter-keepalive-aoe-idsup-sr-idauth-vsdb-dmatxrc-proptxstatus.bin"\
   -nvram "/home/yesun/myprojects/driver/TOT_SVN/src/shared/nvram/bcm943241_Manta_rev04_nvram.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 sd_divisor 1}\
   -datarate {-skiptx 32}      
   
43241a0sdio clone 43241b0RvRF_SVTopt2\
   -tag PHOENIX2_REL_6_10_116_*\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-opt2.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 sd_divisor 1}\
   -datarate {-skiptx 32}
      
43241b0RvRF_SVTopt2 clone 43241b0RvRFb_SVTopt2 -wlinitcmds {} 
43241b0RvRF_SVTopt2 clone 43241b0RvRFb_SVTopt2_26 -tag PHOENIX2_REL_6_10_116_26 -wlinitcmds {} 
43241b0RvRF_SVTopt2 clone 43241b0_SVTopt2 -wlinitcmds {} -postinstall {}
43241b0RvRF_SVTopt2 clone 43241b0_SVTopt2L -wlinitcmds {} -postinstall {} -nvram /home/pkwan/BCM43241/bcm943241ipaagb_p110.txt
43241b0_SVTopt2L clone 43241b0_SVTopt2L5Gx -type "43241b0min-roml/sdio-ag-idsup-opt2/rtecdc.bin"
43241b0_SVTopt2L5Gx clone 43241b0_SVTopt2LF5Gx -postinstall {dhd -i eth1 sd_divisor 1}
43241b0_SVTopt2LF5Gx clone 43241b0_SVTopt2roml -type "43241b0-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-autoabn.bin"
43241b0RvRF_SVTopt2 clone 43241b0RvRF_SVTopt2d -dhd_image "/projects/hnd_software_ext7/work/maheshd/code/tot/trunk/src/dhd/linux/dhd-cdc-sdstd-2.6.29.4-167.fc11.i686.PAE/dhd.ko"
43241b0RvRFb_SVTopt2 clone 43241b0RvRFb_SVTopt2d -dhd_image "/projects/hnd_software_ext7/work/maheshd/code/tot/trunk/src/dhd/linux/dhd-cdc-sdstd-2.6.29.4-167.fc11.i686.PAE/dhd.ko"

43241a0sdio clone 43241b0RvRF_SVTopt2Int\
   -tag PHOENIX2_REL_6_10_116_*\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr-opt2.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 sd_divisor 1; dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}   

43241a0sdio clone 43241b0RvRF_SVTopt2Intx\
   -tag PHOENIX2_REL_6_10_116_*\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr-opt2.bin"\
   -nvram "/home/yesun/myprojects/driver/TOT_SVN/src/shared/nvram/bcm943241_Manta_rev04_nvram.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 sd_divisor 1; dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}   
   
#  hardware P110 since 07/12
  
43241a0sdio clone 43241b0RvR_Req\
   -tag PHOENIX2_REL_6_10_116\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-p2p-pno-pktfilter-keepalive-aoe-idsup-sr-idauth-vsdb-dmatxrc-proptxstatus.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0_Req\
   -tag PHOENIX2_REL_6_10_116\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-p2p-pno-pktfilter-keepalive-aoe-idsup-sr-idauth-vsdb-dmatxrc-proptxstatus.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {}\
   -postinstall {}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0RvR_SVT_exp\
   -tag PHOENIX2_REL_6_10_116\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-p2p-pno-idsup-sr-idauth-proptxstatus.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p100.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0_SVT\
   -tag PHOENIX2_REL_6_10_116_*\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-p2p-pno-pktfilter-keepalive-aoe-idsup-sr-idauth-vsdb-dmatxrc-proptxstatus.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -postinstall {}\
   -datarate {-skiptx 32}
   
#    sdio-ag-p2p-pno-pktfilter-keepalive-aoe-idsup-sr-idauth-vsdb-dmatxrc-proptxstatus.bin
#    sdio-ag-pno-p2p-idauth-idsup-proptxstatus-dmatxrc-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr.bin ;# this is for Panda board only
   
# hardware P110 since 07/12
   
43241a0sdio clone 43241b0_SVT_exp\
   -tag PHOENIX2_REL_6_10_116_7\
   -brand linux-external-dongle-sdio\
   -type "43241b0min-roml/sdio-ag-p2p-pno-idsup-sr-idauth-proptxstatus.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -postinstall {}\
   -datarate {-skiptx 32}

43241a0sdio clone 43241b0RelRvR\
   -tag PHOENIX2_REL_6_10_11?\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}

43241a0sdio clone 43241b0ToBRvRF\
   -tag PHOENIX2_BRANCH_6_10\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 serial 1; dhd -i eth1 sd_divisor 1}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0RelRvRF\
   -tag PHOENIX2_REL_6_10_11?\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 serial 1; dhd -i eth1 sd_divisor 1}\
   -datarate {-skiptx 32}   

43241a0sdio clone 43241b0ToBRvR_b\
   -tag PHOENIX2_{BRANCH,REL}_6_10_*\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-g-idsup.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0RC91\
   -tag PHOENIX2_REL_6_10_91\
   -brand linux-external-dongle-sdio\
   -type "43241b0-roml/sdio-g-idsup.bin"\
   -nvram "bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0_roml\
   -tag PHOENIX2_REL_6_10_*\
   -brand linux-external-dongle-sdio\
   -type "43241b0-roml/sdio-g-idsup.bin"\
   -nvram "bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {}\
   -postinstall {}\
   -datarate {-skiptx 32}
   
43241b0_roml clone 43241b0roml_opt2\
   -tag PHOENIX2_REL_6_10_116_*\
   -type "43241b0-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-opt2-phydbg-autoabn.bin"\
   -postinstall {dhd -i eth1 sd_divisor 1}\
   -noaes 1 -notkip 1

   
43241a0sdio clone 43241b0_romlRvR\
   -tag PHOENIX2_{BRANCH,REL}_6_10_*\
   -brand linux-external-dongle-sdio\
   -type "43241b0-roml/sdio-g-idsup.bin"\
   -nvram "bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0_romlInt\
   -tag PHOENIX2_{BRANCH,REL}_6_10_*\
   -brand linux-internal-dongle\
   -type "43241b0-roml/sdio-g-idsup.bin"\
   -nvram "bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {}\
   -postinstall {}\
   -datarate {-skiptx 32}
   
43241a0sdio clone 43241b0_romlIntRvR\
   -tag PHOENIX2_{BRANCH,REL}_6_10_*\
   -brand linux-internal-dongle\
   -type "43241b0-roml/sdio-g-idsup.bin"\
   -nvram "bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}
      
43241a0sdio clone 43241b0ToBRvR_bP110\
   -tag PHOENIX2_{BRANCH,REL}_6_10_*\
   -brand linux-internal-dongle\
   -type "43241b0min-roml/sdio-g-idsup.bin"\
   -nvram "src/shared/nvram/bcm943241ipaagb_p110.txt"\
   -console mc59end2:40001\
   -wlinitcmds {wl btc_mode 0}\
   -postinstall {dhd -i eth1 serial 1}\
   -datarate {-skiptx 32}
   
package require UTF::HSIC

# 43342A1: code name Chardonnay
# UART: on mc59end2 USB0
	
UTF::HSIC mc59HSIC\
	-sta {43342h eth1}\
	-lan_ip 192.168.1.1\
    -relay 43241a0sdio\
    -lanpeer 43241a0sdio\
     -console "mc59end2:40010"\
     -power {npc_DUT 2}\
    -tag "PHO2203RC1_REL_6_25_35"\
    -type "*/43342a*/chardonnay.trx" \
	-nvram "*/43342a*/chardonnay-t-kk.txt"\
    -brand "linux-external-dongle-usb"\
    -customer "olympic"\
    -host_tag "RTRFALCON_REL_5_130_54"\
    -host_brand linux26-internal-hsic\
    -host_nvram {
		lan_ipaddr=192.168.1.1
		lan_mask=255.255.255.0
		wandevs=dummy
		clkfreq=530,176,88
		watchdog=6000
		lan_stp=0
		lan1_stp=0
    }\
    -postinstall {dhd hsicautosleep 1} \
    -wlinitcmds {wl mimo_bw_cap 1} \
    -nocal 1 -noframeburst 1 -slowassoc 5 \
    -datarate {-skiptx 32}

43342h clone 43342A1
43342h clone 43342A1_ToB -tag "PHO2203RC1_BRANCH_6_25" -host_tag RTRFALCON_REL_5_130_* -host_brand linux26-internal-hsic-phoenix2 -postinstall "bt_enable.sh"
43342A1_ToB clone 43342A1_ToB_coex -wlinitcmds {wl mimo_bw_cap 1; wl btc_mode 1}
### added bt_enable.sh for switching over to P407 HSIC host router

43342A1_ToB clone 43342A1_105 -tag "PHO2203RC1_REL_6_25_105_*" -type "*/43342a*/chardonnay.trx" -nvram "*/43342a*/chardonnay-t-kk.txt"
43342A1_105 clone 43342A1_105_coex -wlinitcmds {wl btc_mode 1}
43342A1_ToB clone 43342A1_646 -tag "PHO2625105RC255_REL_6_46_*" -type "*/43342a*/chardonnay.trx" -nvram "*/43342a*/chardonnay-t-kk.txt"
43342A1_646 clone 43342A1_646_coex -wlinitcmds {wl btc_mode 1}

# # # \ -channelsweep {-bw 20} -perfchans {64 3}
43342A1_ToB clone 43342A0 -nvram "Innsbruck/43342a0/chardonnay-m-mt.txt"

# # # 43342A1_ToB configure -ipaddr 192.168.1.145
43342A1_ToB configure -ipaddr 192.168.1.154

# BTCohost for 4324a0; hostname mc61end15
# temporarily using local cgs copy from IRV BroadcomInternal BCM4334 folder 1/13/12

UTF::WinBT MC59_BTCOHOSTXP \
    -lan_ip 10.19.85.153\
	-sta "mc59_BTCohost"\
	-bt_comm "com6@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -type BCM4324\
    -brand ""\
    -bt_ss_location 0x00210000\
    -file *.cgs \
    -version *

# # #     	-device_reset "10.19.87.104 2"\

# # #  as of 9/20/12 mc59_BTCohost is a WinXP desktop
# # #     -lan_ip 10.19.85.153 ;# replaced with 10.19.87.97 on 9/17/12 for debugging
    
mc59_BTCohost clone BTCohost43241\
	-type BCM43241/B0\
	-version *\
	-brand "Generic/37_4MHzClass1"\
	-file *.cgr
	
mc59_BTCohost clone BTCohost43241_local\
	-project_dir "/home/pkwan"\
	-type BCM43241\
	-brand ""\
	-version *\
	-file *.cg*
	
BTCohost43241 clone BTCohost43241_cfg53 -version BCM43241B0_002.001.013.0053.0000
BTCohost43241_local clone BTCohost43241_cfg55 -version BCM43241B0_002.001.013.0055.0000
BTCohost43241_local clone BTCohost43241_cfg32 -version BCM43241B0_002.001.013.0032.0000
BTCohost43241_local clone BTCohost43241_cfg31 -version BCM43241B0_002.001.013.0031.0000
BTCohost43241_local clone BTCohost43241_cfg30 -version BCM43241B0_002.001.013.0030.0000
BTCohost43241_local clone BTCohost43241_cfg29 -version BCM43241B0_002.001.013.0029.0000
BTCohost43241_local clone BTCohost43241_cfg24BigBuff -version BCM43241B0_002.001.013.0024.0000
BTCohost43241 clone BTCohost43241_cfg24 -version BCM43241B0_002.001.013.0024.0000
mc59_BTCohost clone BTCohost_pwr -power_sta "10.19.87.104 4"


BTCohost43241 clone BTCohost43342A1\
	-bt_comm "com9@3000000"\
    -bt_ss_location 0x00210000\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
	-type BCM43342/A1\
	-version BCM43342A1*Chardonnay_OS_TDK*\
	-brand ""\
	-file *.cg*

# # # 		-bt_comm "com8@3000000"\ ;# P220 HSIC router board, replaced by P407 com9 7/18/14
# # # 	-version BCM43342A1*Chardonnay_OS_USI*\ ;# current Imperial reworked board as of 7/22/14: TDK, not USI
BTCohost43342A1 clone BTCohost43342A1_cfg1055 -version BCM43342A1*1055*Chardonnay_OS_TDK*
		
BTCohost43342A1 clone BTCohost43342A1_local\
	-image "/home/pkwan/BCM43342/BCM43342A1_11.1.702.720_Chardonnay_OS_USI_20130830.cgs"

# BT Ref - WinXP laptop BlueTooth 2046 Reference
# mc59end3  10.19.87.98   mc59_BTRefXP
#	-power_sta "10.19.85.199 1"\

UTF::WinBT MC59_BTREFXP\
    -lan_ip 10.19.87.98\
    -sta "mc59_BTRefXP"\
    -power "172.16.59.30 2"\
    -power_sta "172.16.59.30 1"\
    -device_reset "10.19.87.103 1"\
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user

#     -power "npc_Ref 1"\
#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router MC59AP2 \
    -lan_ip 192.168.1.3 \
    -sta "mc59_AP2 eth1" \
    -power "npc_AP 1"\
    -relay "mc59end1" \
    -lanpeer lan0 \
    -console "mc59end1:40000" \
    -tag "AKASHI_BRANCH_5_110"\
	-brand "linux-internal-router"\
   -nvram {
	   	lan_ipaddr=192.168.1.3
       	fw_disable=1
       	wl_msglevel=0x101
		dhcp_start=192.168.1.144
	    dhcp_end=192.168.1.149
       	wl0_ssid=TestMC59
      	wl0_radio=0
		wl0_channel=1
		w10_nmode=1
		wl0_antdiv=0
		antswitch=0
		wl0_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }

# 4706 AP with 4360 and 4331 cards ;# debug 12/16/14: backdate build to 2014.10.2.0

UTF::Router MC59AP1 -sta {
    4706/4360 eth1 4706/4360.%15 wl0.% 
    4706/4331 eth2
    } \
    -lan_ip 192.168.1.6 \
    -power "npc_AP2 1"\
    -relay "mc59end1" \
    -lanpeer lan0 \
    -console "mc59end1:40036" \
    -nvram {
	   	lan_ipaddr=192.168.1.6
		lan_gateway=192.168.1.6
		dhcp_start=192.168.1.150
	    dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.6
		lan1_gateway=192.168.2.6
		dhcp1_start=192.168.2.150
	    dhcp1_end=192.168.2.199
       	fw_disable=1
       	wl_msglevel=0x101
        boardtype=0x05b2; # 4706nr
        console_loglevel=7
       	wl0_ssid=TestMC59_4360
		wl0_channel=36
		w10_bw_cap=-1
		wl0_radio=0
		wl0_obss_coex=0
		wl1_ssid=TestMC59_4331
		wl1_channel=1
		wl1_bw_cap=-1
		wl1_radio=0
		wl1_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }\
    -datarate {-b 1.2G -i 0.5 -frameburst 1}\
    -noradio_pwrsave 1\
    -date 2014.10.2.

4706/4360 clone 4360_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
4706/4331 clone 4331_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
# 4360_AARDVARK clone mc59_AP1
4360_AARDVARK clone mc59_AP1_BISON -tag "BISON_BRANCH_7_10" 
4331_AARDVARK clone mc59_AP1_bg_BISON -tag "BISON_BRANCH_7_10"
4360_AARDVARK clone mc59_AP1_twig -tag "AARDVARK_REL_6_30_???_*"
4331_AARDVARK clone mc59_AP1_bg_twig -tag "AARDVARK_REL_6_30_???_*"
4360_AARDVARK clone mc59_AP1_trunk -tag TRUNK
4331_AARDVARK clone mc59_AP1_bg_trunk -tag TRUNK
mc59_AP1_BISON clone mc59_AP1
mc59_AP1_bg_BISON clone mc59_AP1_bg

unset UTF::TcpReadStats
