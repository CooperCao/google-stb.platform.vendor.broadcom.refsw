# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: be0702f5b52db82b61b8f9e971ad00373910261e $
#
# Testbed configuration file for mc74b testbed
#

#source utfconf/mc41shared.tcl

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc74b"

package require UTF::Aeroflex

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
#    foreach S {4352 43142w10 43228w10} {
#	catch {$S wl down}
#	$S deinit
    #    }
    B attn default
}

# UTF Controller
UTF::Linux mc51end2 -sta {lan eth1}
lan configure -ipaddr 192.168.1.50

UTF::Power::Synaccess npc1 -relay lan -lan_ip 192.168.1.20

UTF::Aeroflex 192.168.1.5 -group {B {1 2 3}} -relay lan
B configure -default 20

##############

UTF::Linux mc74tst4 -sta {4360 enp1s0} \
    -slowassoc 5 -reloadoncrash 1 \
    -power {npc1 1} -console {mc51end2:40000} \
    -tag EAGLE_BRANCH_10_10 \
    -wlinitcmds {wl down;wl country '#a/0';wl vht_features 3;wl dtim 3;wl bw_cap 2g -1}

4360 configure -ipaddr 192.168.1.95 -attngrp B -hasdhcpd 1


#######################################
set UTF::TestSigning 1


# UTF::Cygwin _mc74tst6 -sta {43228w10} -osver 1064 \
#     -power {npc1 2} -kdpath kd.exe -sign 1 -installer InstallDriver \
#     -brand win8x_internal_wl -usemodifyos 1 \
#     -node DEV_4359 -udp 300m -reg {BandwidthCap 1} \
#     -yart {-frameburst 1 -attn5g 30-103 -attn2g 47-103 -pad 13} -iperfdaemon 0

# 43228w10 configure -ipaddr 192.168.1.91

# 43228w10 clone 43228w10b -tag BISON_BRANCH_7_10
# 43228w10 clone 43228w10b35 -tag BISON05T_BRANCH_7_35 \
#     -brand win10_internal_wl -usecsa 0 -use11h 0 -noibss 1 \
#     -wlinitcmds {wl down;wl spect 0}

# 43228w10 clone 43142w10 -node DEV_4365 -nobighammer 1

# #    -msgactions {{wl_sendup error} FAIL}

# 43142w10 clone 43142w10b -tag BISON_BRANCH_7_10
# 43142w10 clone 43142w10b35 -tag BISON05T_BRANCH_7_35 \
#     -brand win10_internal_wl -noibss 1

# # Win8.1
# 43228w10 clone 43228w81 -osver 8164
# 43142w10 clone 43142w81 -osver 8164

# 43228w10b35 clone 43228w81b35 -osver 8164 -brand win8x_internal_wl
# 43142w10b35 clone 43142w81b35 -osver 8164 -brand win8x_internal_wl

# BindIperf being used as a WAR for SWWLAN-125912
set UTF::BindIperf 1

UTF::WinDHD mc74tst6 -sta {43602w10} \
    -app_tag trunk \
    -osver 1064 -installer InstallDriver \
    -power {npc1 2} -kdpath kd.exe -sign 1 \
    -brand win10_dhdpcie_internal_wl \
    -type checked/DriverOnly/x64 \
    -embeddedimage 43602a1 \
    -usemodifyos 1 -tcpwindow 4m -noibss 1 -udp 700m \
    -wlinitcmds {wl down;wl vht_features 3;wl bw_cap 2 -1} \
    -allowdevconreboot 1 -perfchans {36/80 3} -nocal 1 \
    -yart {-pad 20 -attn5g 30-103 -attn2g 46-103} \

43602w10 configure \
    -yart {-pad 20 -attn5g 30-103 -attn2g 46-103 -b 0} \
    -channelsweep {-b 0} \
    -datarate {-b 0}

43602w10 clone 43602w8x \
    -brand win8x_dhdpcie_internal_wl

if {0} {
    # Example hack for capturing wdi_trace on ConnectAPSTA failures
    package require UTF::Test::ConnectAPSTA
    rename UTF::Test::ConnectAPSTA UTF::Test::ConnectAPSTA_orig
    proc UTF::Test::ConnectAPSTA {AP STAS args} {
	foreach S $STAS {
	    catch {$S wdi_trace start}
	}
	if {[catch {UTF::Test::ConnectAPSTA_orig $AP $STAS {*}$args} ret]} {
	    foreach S $STAS {
		set f [$S wdi_trace stop]
		set ret "html:$ret <a href=\"$f\">$f</a>"
	    }
	    error $ret
	}
    }
}



####
UTF::Q mc74b
