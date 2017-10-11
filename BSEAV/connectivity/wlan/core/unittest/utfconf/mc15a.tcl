# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for mc15a testbed
#

# Common config
source utfconf/mc15.tcl

set ::UTF::usrutf /usr/UTF

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc15a"

set ::UTF::SetupTestBed {
    A attn default
}

# Endpoints
UTF::Linux sr1end20 -sta {lan2 em2} -onall false
lan2 configure -ipaddr 192.168.1.50

UTF::Linux sr1end21 -sta {lan3 eth1} -onall false
lan3 configure -ipaddr 192.168.1.51

# sr1end17 is dead
UTF::Linux sr1end18 -sta {wan eth1}

# STAs

set stacommon {
    -type debug-apdef-stadef-p2p-mchan-tdls
    -wlinitcmds {
	wl msglevel +assoc;
	wl down;
	wl country '#a/0';
	wl bw_cap 2g -1;
	wl vht_features 3;
    }
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1
}

UTF::Linux mc15tst2 -sta {sta1 enp1s0} \
    -power {mc15ips1 6} \
    -console {sr1end20:40000} \
    {*}$stacommon


#-modopts {assert_type=1} \
#    -msgactions {
#	PHYTX {
#	    if {![info exists ::__action_fired]} {
#		set ::__action_fired 1
#		sta1 wl pmac shm 0x748 -n 128
#		sta1 wl pmac shm 0x8c8 -n 182
#	    }
#	    return 0
#	}
#   }

sta1 configure -attngrp A

UTF::Linux mc15tst1 -sta {sta2 enp1s0} \
    -power {mc15ips1 4} \
    -console {sr1end11:40003} \
    {*}$stacommon

sta2 configure -attngrp A

# Router

UTF::Router 4709 -sta {
    4709/43602a eth1 4709/43602a.%15 wl0.%
    _4709/43602g eth2 _4709/43602g.%15 wl1.%
} \
    -relay lan2 \
    -lanpeer {lan2 lan3} \
    -wanpeer wan \
    -brand linux-2.6.36-arm-internal-router \
    -console {sr1end20:40001} \
    -power {mc15ips1 5} \
    -nvram {
	watchdog=2000
	lan_stp=0
	lan1_stp=0
	wl1_ssid=4709/43602g
	wl1_chanspec=1
	wl1_radio=0
	wl0_ssid=4709/43602a
	wl0_chanspec=36
	wl0_radio=0
	samba_mode=2
	wl0_country_code=#a
	wl0_country_rev=0
	wl1_country_code=#a
	wl1_country_rev=0

	# Allow vht_features to use defaults - 2g will be
	# using prop11n instead of VHT
    } \
    -datarate {-i 0.5 -auto} -udp 1.5g \
    -yart {-frameburst 1 -attn5g 20-93 -attn2g 35-93 -pad 26} \
    -noradio_pwrsave 1 -perfchans {36/80}

4709/43602a clone 4709e/43602a -tag EAGLE_BRANCH_10_10

# Hybrid DHD+NIC with 43602 in NIC mode:
4709/43602a clone 4709b25/43602a -tag BISON04T_TWIG_7_14_131_25 \
    -brand linux-2.6.36-arm-internal-router-dhdap

# Clone for external 43602 on 5g
4709/43602a clone 4709x/43602a -sta {4709x/43602a eth1 4709x/43602g eth2} \
    -brand linux-2.6.36-arm-external-vista-router-full-src \
    -perfonly 1 -datarate 0 -docpu 1 -nocustom 1

4709x/43602a configure -dualband {4709x/43602g -c1 36/80}

4709x/43602a clone 4709ex/43602a \
    -tag EAGLE_BRANCH_10_10 -sta {4709ex/43602a eth1 4709ex/43602g eth2}

4709ex/43602a configure -dualband {4709ex/43602g -c1 36/80}


# Clone for 43602 on 2G
_4709/43602g clone 4709/43602g -perfchans {3} -nosamba 1

4709/43602g clone 4709e/43602g -tag EAGLE_BRANCH_10_10

# Hybrid DHD+NIC with 43602 in NIC mode:
4709/43602g clone 4709b25/43602g -tag BISON04T_TWIG_7_14_131_25 \
    -brand linux-2.6.36-arm-internal-router-dhdap

# WAR for SWWLAN-143231, SWWLAN-143520
4709/43602a configure -noaes 1 -notkip 1 -nopm1 1
4709/43602g configure -noaes 1 -notkip 1 -nopm1 1


set UTF::RouterNightlyCustom {
    UTF::Try "$Router: Vendor IE" {
	package require UTF::Test::vndr_ie
	UTF::Test::vndr_ie $Router $STA1
    }

#    package require UTF::Test::MiniUplinks
#    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 \
#	-pstareboot -dodwdsr 1 -dowetr 1

    package require UTF::Test::Repeaters
    UTF::Test::Repeaters $Router $STA1 -sta $STA2

}

#####
UTF::Q mc15a

