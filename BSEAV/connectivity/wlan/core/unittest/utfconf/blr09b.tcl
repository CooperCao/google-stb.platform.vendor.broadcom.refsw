#Testbed configuration file for blr09end1 UTF  Teststation
#Created by Sumesh Nair on 19Nov2014 05:00 PM  
#Last checkin 19Nov2014 
####### Controller section:
# blr09end1: FC15
# IP ADDR 10.131.80.101
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP section:
#
#  
# 
#
####### STA section:
#
# blr09tst1: 
# blr09tst2: 
# blr09tst3:4359b0
# blr09tst4: 
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
        -relay "blr09end1.ban.broadcom.com" \
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
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr09"

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \


# Turn off most RvR initialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""


####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.11 -relay blr09end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.21 -relay blr09end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.31 -relay blr09end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.1.1.42 -relay blr09end1.ban.broadcom.com -rev 1

########################### Test Manager ################

UTF::Linux blr09end1.ban.broadcom.com \
     -lan_ip 10.131.80.101 \
     -sta {lan p16p2}

# Power          - npc21 port 1    172.1.1.21
################################################################################

##########################################################################################
#                                  STA 43570 Chip 
##########################################################################################

# PCIe NIC mode
UTF::Linux blr09tst1 \
        -lan_ip 10.131.80.104 \
		-sta {4360x enp1s0} \
        -power {npc41 1} \
        -power_button "auto" \
        -tag BISON_BRANCH_7_10 \
        -brand linux-internal-wl \
        -tcpwindow 3m -slowassoc 5 -reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3}

4360x configure -ipaddr 192.168.1.109 \

 #PCIe Full Dongle mode
UTF::DHD blr09tst1d \
    -lan_ip 10.131.80.104 \
    -sta {43570 eth0} \
    -power {npc41 1} \
    -power_button "auto" \
	-perfchans {36/80 36l 36 1l 1} \
	-dhd_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-external-media \
	-nvram bcm943570pcieir_p150.txt \
	-slowassoc 5 -extsup 1 -docpu 1 \
	-datarate {-i 0.5 -frameburst 1} \
	-tcpwindow 3m -udp 800m -nocal 1 \
	-wlinitcmds {wl msglevel +assoc; wl vht_features 3;} \
	-msgactions {
            "Pktid pool depleted." WARN
    	}

43570 configure -ipaddr 192.168.1.102 \
		

43570 clone 43570b \
	-tag BISON_BRANCH_7_10 \
	-brand hndrte-dongle-wl \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert/rtecdc.bin

43570b clone 43570bx \
	-perfonly 1 -perfchans {36/80} \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds/rtecdc.bin \
	
	#-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive/rtecdc.bin

43570 clone 43570b35 \
	-tag BISON05T_BRANCH_7_35 \
	-brand hndrte-dongle-wl \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert/rtecdc.bin

43570b35 clone 43570b35x \
	-perfonly 1 -perfchans {36/80} \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds/rtecdc.bin

43570 clone 43570b143 -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
	-brand linux-internal-media \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert.bin

43570 clone 43570b143x -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
	-brand linux-external-media \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds.bin
	
###########################################
# blr09tst3
# 4358
# 
#  
###########################################
UTF::DHD blr09tst2d \
        -lan_ip 10.131.80.105 \
        -sta {4358 eth0} \
        -power {npc41 1} \
        -power_button "auto" \
	-dhd_tag DHDNC39RC65_BRANCH_1_47 \
	 -dhd_brand linux-internal-dongle-pcie \
	-brand linux-external-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
        -nvram bcm94358wlpciebt.txt\
	-slowassoc 5 \
	-extsup 1 \
	-perfchans {36/80 36l 36 1l 3} \
        -datarate {-i 0.5 -frameburst 1} \
        -tcpwindow 2m -udp 1.2g \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3}

4358 configure -ipaddr 192.168.1.104 \
		
		
4358 clone 4358b112x \
	-tag  BISNC105RC66_BRANCH_7_112\
	-type 4358a3-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-xorcsum-proxd.bin \

4358b112x clone 4358b112 \
	-type 4358a3-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx5g-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-btcdyn-wepso-rcc-noccxaka-sarctrl-xorcsum-idsup-idauth-proxd-assert-err.bin \

4358 clone 4358b35 \
	-tag BISON05T_BRANCH_7_35 \
	-brand hndrte-dongle-wl \
	-type 4358a2-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-proxd-nan-hs20sta-assert/rtecdc.bin

4358b35 clone 4358b35x \
	-type 4358a2-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds/rtecdc.bin

# PCIe NIC mode
UTF::Linux blr09tst2 \
    -lan_ip 10.131.80.105\
    -sta {4358l eth0} \
    -power {npc41 1} \
    -power_button "auto" \
	-tag BISON05T_BRANCH_7_35 \
	-type debug-apdef-stadef-extnvm \
	-nvram "bcm94358wlpagbl.txt" \
	-tcpwindow 3m -udp 800m \
	-docpu 1 -reloadoncrash 1 \
	-slowassoc 5 -datarate {-i 0.5 -frameburst 1} \
	-wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1; wl vht_features 3}

4358l configure -ipaddr 192.168.1.105 \
	


##########################################################################################
#                                  STA 4359b0 Chip 
##########################################################################################

UTF::DHD blr09tst3 \
        -lan_ip 10.131.80.106 \
        -sta {4359b0x eth0} \
        -power "npc41 1" \
        -power_button "auto" \
        -perfchans {36/80 36l 36 1l 3} \
        -dhd_tag trunk \
        -app_tag trunk \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag DINGO_BRANCH_9_10 \
		-slowassoc 5  \
        -nvram "bcm94359fcpagbss_2.txt" \
        -type 4359b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-sr-idsup-idauth-proptxstatus-ampduhostreorder-sstput-die5-rsdbsw/rtecdc.bin \
        -tcpwindow 2m \
        -extsup 1 -nocustom 1 \
        -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1}  \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
        -yart {-attn5g 16-95 -attn2g 48-95 -pad 30}

4359b0x configure -ipaddr 192.168.1.234 \


4359b0x clone  4359b0 \
	 -type 4359b0-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die5-rsdbsw/rtecdc.bin \
	 


############################## 4359b0 die3 #######################
	 
4359b0 clone 4359d3 \
	-tag DINGO_BRANCH_9_10 \
	-type 4359b0-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die3-rsdbsw/rtecdc.bin \
	-nvram "bcm94359fcpagbss.txt" \
	-wlinitcmds {wl rsdb_mode 0;wl vht_features 3} \

4359d3 clone 4359b1 \
   -type 4359b1-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die3-rsdbsw/rtecdc.bin \
   -nvram "bcm943593fcpagbss.txt" \
	
#card 4345, Full dongle mode

#card 43455 NIC mode

UTF::Linux blr09tst4l \
        -lan_ip 10.131.80.107 \
        -sta {43455l eth0} \
        -power "npc42 2"\
        -power_button "auto" \
		-tag BISON05T_BRANCH_7_35 \
        -brand "linux-internal-wl" \
		-nvram "bcm943457wlsagb.txt" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
		-type debug-apdef-stadef-p2p-mchan-tdls \
        -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

43455l configure -ipaddr 192.168.1.91 \

#card 4345, Full dongle mode

UTF::DHD blr09tst4 \
	-lan_ip 10.131.80.107 \
	-sta {43455 eth0} \
    -power {npc41 2} \
	-console "blr09end1:40000" \
    -tag BISON06T_BRANCH_7_45	\
    -dhd_brand linux-external-dongle-sdio \
    -brand linux-external-dongle-sdio \
    -nvram "bcm943457wlsagb.txt" \
	-modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
    -type 43455c0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-ltecx-wfds.bin \
    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; dhd -i eth0 sd_divisor 1; } \
    -slowassoc 5 -escan 1 \
	-extsup 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -tcpwindow 1152k -udp 400m -nocal 1 -docpu 1 -nointerrupts 1 \
    -yart {-attn5g 7-63 -attn2g 17-63 -pad 44 -frameburst 1} \
    -msgactions {
	"DTOH msgbuf not available" FAIL
    }
43455 configure -ipaddr 192.168.1.92 \

43455 clone 43455b45x \
	-tag BISON06T_BRANCH_7_45 \
	-type 43455c0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wnm-wapi-okc-ccx-ltecx-wfds-wl11u-mfp-tdls-ve-sr-amsdutx-11nprop-dmatxrc-idsup-idauth.bin \

43455b45x clone 43455b45 \
	-type 43455c0-roml/43455_sdio-pno-aoe-pktfilter-pktctx-lpc-pwropt-wapi-43455_ftrs-wfds-mfp-sr-amsdutx-dmatxrc-err-assert.bin  \
	
43455 clone 43455bx \
	-tag BISON_BRANCH_7_10 \
	-brand hndrte-dongle-wl \
	-type 43455c0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-ltecx-wfds/rtecdc.bin \

43455bx clone 43455b \
	-brand hndrte-dongle-wl \
	-type 43455c0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wnm-okc-ccx-ltecx-wfds-wl11u-mfp-tdls-ve-amsdutx-11nprop-err-assert.bin

# Olympic uses the internal supplicant in production
43455 clone 43455bis120 -tag BIS120RC4PHY_BRANCH_7_16 \
    -type 43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap/rtecdc.bin \
    -customer olympic \
    -wlinitcmds {wl down;wl vht_features 3;wl country XZ/35} \

43455b clone 43455b.1 -sta {_43451b1o eth0 43455b.1 wl0.1} \
    -apmode 1 -nochannels 1 -slowassoc 5 \
    -wlinitcmds {wl down;wl vht_features 3;wl apsta 1;wl ssid -C 1 43451b1AP} \
    -noaes 1 -notkip 1 -yart {}
	


43455b.1 configure -ipaddr 192.168.1.93 \

#43455b clone 43455bx \
#   -customer olympic \
#    -type ../C-4345__s-B1/tempranillo.trx -clm_blob tempranillo.clmb \
#    -wlinitcmds {wl down;wl vht_features 3;wl country XZ/202} \
#   -perfonly 1 -perfchans {36/80} -app_tag BIS120RC4PHY_BRANCH_7_16


################ blr09ref0 (4360) ###########

UTF::Linux blr09ref0 -sta {4360 enp1s0} \
    -lan_ip 10.131.80.102 \
    -power "npc11 1"\
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \

4360 configure -ipaddr 192.168.1.101 -ap 1 -attngrp G1 \

4360 clone 4360b35 \
	-tag BISON05T_BRANCH_7_35 \

4360b35 clone 4360-bf1 -tag BISON05T_BRANCH_7_35 
4360-bf1 configure -ipaddr 192.168.1.129 -attngrp G1 -ap 1 

4360b35 clone 4360-bf0
4360-bf0 configure -ipaddr 192.168.1.129 -attngrp G1 -ap 1  \
        -wlinitcmds { wl txbf 0; wl txbf_imp 0; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0;}

# for testing 11n explicit TxBF, 4/9/2014
4360 clone 4360-bf1n
4360-bf1n configure -ipaddr 192.168.1.129 -attngrp G1 -ap 1  \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl country US/0; wl vhtmode 0;}

4360 clone 4360-bf0n
4360-bf0n configure -ipaddr 192.168.1.129 -attngrp G1 -ap 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl country US/0; wl vhtmode 0; wl txbf 0; wl txbf_imp 0;}

UTF::Linux blr09ref1 -sta {4360aci eth0} \
    -lan_ip 10.131.80.103 \
    -power "npc21 1"\
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \

4360aci configure -ipaddr 192.168.1.103 -ap 1 -attngrp G2 \


UTF::Sniffer sniffer \
        -lan_ip 10.131.80.57 \
        -sta {snif eth1} \
        -power_button "auto" \
		-tag "BISON05T_BRANCH_7_35" \

UTF::Q blr09b

