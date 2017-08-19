# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# pb6a Configuration
#
# Win7 STA test rig
#

set ::UTF::SummaryDir "/projects/hnd_sig/utf/pb6a"

UTF::Power::Synaccess pb6anpc1
UTF::Power::Synaccess pb6aips1
UTF::Power::Synaccess npc2 -relay SR2End05 -lan_ip 192.168.1.10
UTF::Power::WebRelay usbwr -relay SR2End05 -lan_ip 192.168.1.4

## Endpoints
UTF::Linux SR2End05 -sta {lan eth1}
lan configure -ipaddr 192.168.1.50

# Unload all the STA drivers before testing.  This is needed because
# we don't have completely reliable control over the multiple Windows
# drivers so it's safest to start with them all off.  Might as well
# disable the Mac too, while we're at it.
set ::UTF::SetupTestBed {
    UTF::Try "4322w7 unload" {
	if {[catch {4322w7 unload} ret]} {
	    if {[regexp {No route} $ret]} {
		append ret ": Attempting recovery..."
		4322w7 power cycle
		UTF::Sleep 60
	    }
	    error $ret
	}
	return $ret
    }
    4322w7 deinit

}


# Win7 4322MC_HP(DE) in WWAN slot
UTF::Cygwin pb6atst2 \
    -sta {
	4322w7 00:1A:73:F2:DA:C8
	vwifi7   VWIFI
    } \
    -node {DEV_432B} \
    -power {pb6anpc1 2} \
    -user user -osver 7 \
    -reg {Country "US"} \
    -allowdevconreboot 1 -udp 300m

4322w7 clone 4322w7k -tag KIRIN_BRANCH_5_100
4322w7 clone 4322w7k82 -tag KIRIN_REL_5_100_82_*
4322w7 clone 4322w7r -tag RUBY_BRANCH_6_20
4322w7 clone 4322w7v -tag AARDVARK_BRANCH_6_30
4322w7 clone 4322w7b -tag BISON_BRANCH_7_10

# 4313 in WLAN slot
4322w7 clone 4313w7 -sta {4313w7 00:90:4C:64:01:5B} -node {DEV_4727} -udp 100m
4313w7 clone 4313w7k -tag KIRIN_BRANCH_5_100
4313w7 clone 4313w7k82 -tag KIRIN_REL_5_100_82_*
4313w7 clone 4313w7r -tag RUBY_BRANCH_6_20
4313w7 clone 4313w7v -tag AARDVARK_BRANCH_6_30
4313w7 clone 4313w7b -tag BISON_BRANCH_7_10

# 4312 in WPAN slot
4322w7 clone 4312w7 -sta {4312w7 00:25:56:49:A5:86} -node {DEV_4315} -udp 56m
4312w7 clone 4312w7k -tag KIRIN_BRANCH_5_100
4312w7 clone 4312w7k82 -tag KIRIN_REL_5_100_82_*
4312w7 clone 4312w7r -tag RUBY_BRANCH_6_20
4312w7 clone 4312w7v -tag AARDVARK_BRANCH_6_30
4312w7 clone 4312w7b -tag BISON_BRANCH_7_10

# 43236 USB BMac Dongle on Win7 (same host as above)
UTF::WinDHD pb6atst2b -lan_ip pb6atst2 -sta {43236w7 00:90:4C:03:21:23} \
    -node {PID_BD17} -embeddedimage 43236 \
    -console "SR2End05:40003" -power_sta {usbwr 1} \
    -power {pb6anpc1 2} \
    -user user -osver 7 -udp 300m \
    -brand win_internal_wl \
    -type checked/DriverOnly_BMac \
    -nobighammer 1 \
    -msgactions {
	{dbus_usb_dl_writeimage: Bad Hdr or Bad CRC} FAIL
    }

# Disable shared wep on 43236w7 trunk.
43236w7 configure -nosharedwep 1

43236w7 clone 43236w7k -tag KIRIN_BRANCH_5_100
43236w7 clone 43236w7k68 -tag KIRIN_REL_5_100_68_*
43236w7 clone 43236w7r -tag RUBY_BRANCH_6_20
43236w7 clone 43236w7v -tag AARDVARK_BRANCH_6_30
43236w7 clone 43236w7t14 -tag AARDVARK01T_TWIG_6_37_14
43236w7 clone 43236w7b -tag BISON_BRANCH_7_10

43236w7 clone 43526w7 -sta {43526w7 00:90:4C:0E:60:7D} \
    -node {PID_BD1D} -embeddedimage 43526b -udp 300m \
    -console {SR2End05:40002} -power_sta {usbwr 4} -nocal 1 \
    -wlinitcmds {wl down; wl vht_features 3}

43526w7 clone 43526w7v -tag AARDVARK_BRANCH_6_30
43526w7 clone 43526w7v38 -tag AARDVARK_{TWIG,REL}_6_30_38{,_*}
43526w7 clone 43526w7t14 -tag AARDVARK01T_TWIG_6_37_14
43526w7 clone 43526w7b -tag BISON_BRANCH_7_10

# All enabled - special clone to allow all devices to be enabled at
# the same time.  BMAC devices are included so they can be accessed,
# but loading isn't going to work
4322w7 clone 4322all -sta {
    4322all  00:1A:73:F2:DA:C8
    4313all  00:90:4C:64:01:5B
    4312all  00:25:56:49:A5:86
    43236all 00:90:4C:03:21:23
} -node {PCI|USB|SD} -tag KIRIN_BRANCH_5_100

# Enable 4322 and 4313 at the same time
4322w7 clone 43xxw7 -node {DEV_4[37]2}


UTF::Linux pb6atst1 -sta {4360mcV eth0} \
    -tag BISON_BRANCH_7_10 \
    -console SR2End05:40001 -power {pb6anpc1 1} \
    -reloadoncrash 1 \
    -wlinitcmds {
	wl msglevel +assoc; wl dtim 3;
	wl vht_features 3;
	service dhcpd stop; sleep 1; wl -i eth0 down;:
    }

4360mcV configure -ipaddr 192.168.1.57 -hasdhcpd 1 -ap 1

####
UTF::Q pb6a
