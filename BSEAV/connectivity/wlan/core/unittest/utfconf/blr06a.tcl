 #Testbed configuration file for blr04end1
#Edited Battu kaushik Date 27june2014
#Last checkin 03SEP2013 12AM
####### Controller section:
# blr06end1: FC15
#
#
####### SOFTAP section:

# AP1:4360a
# AP2:4360b
####### STA section:
#
# blr06tst1: 4349 eth0 (10.131.80.32)
# blr06ref0: 4360a eth0 (10.132.30.30)
# blr06ref1: 4360b eth0 (10.132.30.31)

######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

UTF::Aeroflex af -lan_ip 172.1.1.22 \
        -relay "blr06end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
                G3 {7 8 9}
                ALL {1 2 3 4 5 6 7 8 9}
                }
#G1 configure default 0
#G2 configure default 0
#G3 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 0
    catch {G3 attn 0;}
    catch {G2 attn 0;}
    catch {G1 attn 0;}
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr06"

set UTF::SetupTestBed {
    G1 attn default
    foreach S {4360a 4360b 4359a2} {
	catch {$S wl down}
	$S deinit
    }
    return
}


#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

set ::UTF::FBDefault 1 \


UTF::Linux blr06end1 \
     -lan_ip 10.131.80.29 \
     -sta {lan em1} \

UTF::Linux blr06ref0 \
     -lan_ip 10.131.80.30 \
     -sta {4360a eth0} \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3} \

4360a configure -ipaddr 192.168.1.90 -attngrp G1 -hasdhcpd 1 \

UTF::Linux blr06ref1 \
     -lan_ip 10.131.80.31 \
     -sta {4360b eth0} \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -wlinitcmds {
       wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }


4360b configure -ipaddr 192.168.2.100 -attngrp G1 -hasdhcpd 1 \


UTF::DHD blr06tst1 \
     -lan_ip 10.131.80.32 \
     -sta {4359a2 eth0} \
     -dhd_brand linux-internal-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO_BRANCH_9_10 \
     -nocal 1 -slowassoc 5 \
     -nvram "bcm94359fcpagbss_2.txt" \
     -type 4359a2-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-sr-idsup-idauth-proptxstatus-ampduhostreorder-sstput-die5-rsdbsw/rtecdc.bin \
    -udp 800m  \
    -tcpwindow 2m \
    -wlinitcmds {wl vht_features 3} \
    -yart {-attn5g 05-95 -attn2g 30-95}

#4359a2 configure -ipaddr 192.168.1.91 -hasdhcpd 1 \


#4359a2 clone 4359a2sr \
 #       -type 4359a2-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-wl11u-pktctx-okc-tdls-ccx-mfp-splitrx-wnm-ve-idsup-idauth-sr-ltecx-proptxstatus-ampduhostreorder/rtecdc.bin \
 #       -wlinitcmds {wl vht_features 3;wl rsdb_mode 0} \

#4359a2 clone 4359a2.1 -sta {4359a2.1 eth0 _4359a2.2 wl1.2} \
#    -type 4359a2-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-wl11u-pktctx-okc-tdls-ccx-mfp-splitrx-wnm-ve-idsup-idauth-sr-ampduhostreorder/rtecdc.bin \
#    -wlinitcmds {wl mpc 0;wl vht_features 3;wl rsdb_mode 1;wl -w 1 bss -C 2 sta } \
 #   -perfchans {36/80 36l}

#_4359a2.2 clone 4359a2.2 -sta {_4359a2.1 eth0 4359a2.2 wl1.2} \
#    -perfonly 1 
#4359a2.2 configure -dualband {4360b 4359a2.1 -c1 36/80 -c2 3l -b1 800m -b2 800m}

4359a2 clone 4359a2 \
	-tag DINGO_BRANCH_9_10 \
	-type 4359a2-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-sr-idsup-idauth-proptxstatus-ampduhostreorder-sstput-die5-rsdbsw/rtecdc.bin \
	-nvram "bcm94359fcpagbss_2.txt" \

4359a2 clone 4359a2.1 -sta {4359a2.1 eth0 _4359a2.2 wl1.2} \
    -wlinitcmds {wl mpc 0;wl vht_features 3;wl rsdb_mode 1;wl -w 1 bss -C 2 sta;wl -i wl1.2 chanspec 1} \
    -perfchans {36/80 36l} -nocustom 1 \

_4359a2.2 clone 4359a2.2 -sta {_4359a2.1 eth0 4359a2.2 wl1.2} \
    -perfonly 1 -nocustom 1 \

4359a2.2 configure -dualband {4360b 4359a2.1 -c1 36/80 -c2 3l -b1 800m -b2 800m} \

4359a2 clone 4359a2err \
    -type 4359a2-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die5-rsdbsw/rtecdc.bin \
    -wlinitcmds {wl vht_features 3;wl rsdb_mode 1} \


4359a2err clone 4359a2err.1 -sta {4359a2err.1 eth0 _4359a2err.2 wl1.2} \
    -wlinitcmds {wl down;wl mpc 0;wl vht_features 3;wl rsdb_mode 1;wl -w 1 bss -C 2 sta;wl -i wl1.2 down;wl -i wl1.2 chanspec 3} \
    -nopm1 1 -nopm2 1 -perfchans {36/80} -nocustom 1

_4359a2err.2 clone 4359a2err.2 -sta {_4359a2err.1 eth0 4359a2err.2 wl1.2} \
    -perfonly 1 -perfchans 3l
4359a2err.2 configure -dualband {4360b 4359a2err.1 -c1 36/80 -c2 3l -b1 800m -b2 800m}

 


UTF::Linux blr04softap -sta {4360d eth1} \
    -lan_ip 10.131.80.46 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }


set UTF::StaNightlyCustom {
    if {$(ap2) ne ""} {
        package require UTF::Test::MultiSTANightly
        MultiSTANightly -ap1 $Router -ap2 $(ap2) -sta $STA \
            -noap -nosetuptestbed -nostaload -nostareload -nosetup \
            -noapload -norestore -nounload
    }
}






