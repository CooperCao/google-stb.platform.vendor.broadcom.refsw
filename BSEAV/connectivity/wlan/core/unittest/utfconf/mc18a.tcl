# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for mc16 testbed
#

# Common config
source utfconf/mc18.tcl

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc18a"

set ::UTF::SetupTestBed {
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {47186b14 47186b14/43236 4718b14 4718b14/43217} {
	catch {$S apshell wl -i [$S cget -device] down}
    }
    A attn default
}

#UTF::Linux sr1end12 -sta {wan eth1}
UTF::Linux sr1end25 -sta {uplan eth1}
UTF::Linux sr1end19 -sta {upwan eth1}

UTF::Linux SR1End15 -sta {lan2 eth1}
UTF::Linux SR1End02 -sta {wan2 eth1}

lan2 configure -ipaddr 192.168.1.60

#lan configure -ipaddr 192.168.11.50
uplan configure -ipaddr 192.168.1.60

UTF::Linux mc18tst1 -sta {sta2 enp1s0} -console "sr1end25:40002" \
    -power {mc18ips1 5} -slowassoc 5 \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -reloadoncrash 1 -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1}

UTF::Linux mc18tst2 -sta {sta1 enp1s0} -console "sr1end25:40003" \
    -power {mc18ips1 4} -slowassoc 5 \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -reloadoncrash 1 -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1}

#sta1 configure -date 2015.7.10.0
#sta2 configure -date 2015.7.10.0

UTF::Router _47186 -name 47186 -sta {dummy dummy} \
    -brand linux26-internal-usbap \
    -embeddedimage 43236b \
    -power {mc18ips1 8} \
    -lan_ip 192.168.1.3 \
    -relay uplan \
    -lanpeer uplan \
    -wanpeer upwan \
    -console "sr1end25:40000" \
    -rteconsole "sr1end25:40001" \
    -nvram {
	lan_gateway=192.168.1.3
	fw_disable=1
	wl0_ssid=47186
	wl0_chanspec=11
	wl0_radio=0
	wl0_frameburst=on
	wl1_ssid=47186/43236
	wl1_chanspec=36
	wl1_radio=0
	wl1_frameburst=on
	watchdog=6000

	# BISON extra features
	wl0_atf=1
	wl0_pspretend_retry_limit=5
	wl1_atf=1
	wl1_pspretend_retry_limit=5
    } \
    -yart {-attn5g 30-95 -attn2g 40-95 -pad 26}

dummy configure -attngrp A

dummy clone 47186b14 -tag BISON04T_BRANCH_7_14 \
    -sta {47186b14 eth1 47186b14.%15 wl0.%} -perfchans 3l
dummy clone 47186b14/43236 -tag BISON04T_BRANCH_7_14 \
    -sta {47186b14/43236 eth2 47186b14/43236.%3 wl1.%} -perfchans 36l


#############


UTF::Router _4718 -name 4718 -sta {dummy dummy} \
    -lan_ip 192.168.1.2 \
    -power {mc18ips1 2} \
    -relay lan2 \
    -lanpeer lan2 \
    -wanpeer wan2 \
    -console "sr1end25:40007" \
    -brand linux26-internal-router \
    -nvram {
	lan_gateway=192.168.1.2
	wl_msglevel2=0x08
	fw_disable=1
	watchdog=3000
	wl0_ssid=4718
	wl0_chanspec=1
	wl0_radio=0
	wl0_frameburst=on
	wl1_ssid=4718/43217
	wl1_chanspec=3
	wl1_radio=0
	wl1_frameburst=on

	# BISON extra features
	wl0_atf=1
	wl0_pspretend_retry_limit=5
	wl1_atf=1
	wl1_pspretend_retry_limit=5
    } \
    -udp 300m \
    -yart {-attn5g 25-95 -attn2g 35-95 -pad 26}

dummy configure -attngrp A

dummy clone 4718b14 -tag BISON04T_BRANCH_7_14 \
    -sta {4718b14 eth1 4718b14.%15 wl0.%} -perfchans 3l
dummy clone 4718b14/43217 -tag BISON04T_BRANCH_7_14 \
    -sta {4718b14/43217 eth2 4718b14/43217.%15 wl1.%} -perfchans 3l




#4718b14/43228	configure -upstream 47186b14/43236
#47186b14/43236	configure -upstream 4718b14/43228
#4718b14       	configure -upstream 47186b14
#47186b14	configure -upstream 4718b14



#############

set UTF::RouterNightlyCustom {
    UTF::Try "$Router: Vendor IE" {
	package require UTF::Test::vndr_ie
	UTF::Test::vndr_ie $Router $STA1
    }
    catch {
	package require UTF::Test::MiniUplinks
	UTF::Test::MiniUplinks $Router $STA1 -sta $STA2
    }
}

#####
UTF::Q mc18a uplan

