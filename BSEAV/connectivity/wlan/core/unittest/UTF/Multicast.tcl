#!/bin/env utf
#
#
# UTF library for Multicast
#
# Written by: Robert J. McMahon March 2010
#
# $Id$
# $Copyright Broadcom Corporation$
#
package require UTF
package provide UTF::Multicast 2.0

namespace eval UTF::Multicast {
    #
    #  Kill any outstanding iperf instances.
    #
    proc force_mcast_iperf_cleanup {device grpip} {
	set output [$device rexec "ps -ef | grep iperf"]
	foreach line [split $output "\n"] {
	    if {[regexp "^root\[\ \]+(\[0-9\]+).*-B\[ \]+$grpip" $line - pid]} {
		catch {$device rexec "/usr/bin/kill -s SIGKILL $pid"}
	    }
	}
    }

    proc force_igmpq_cleanup {device} {
	set output [[$device lan] rexec -x "pkill igmp_querier"]
    }

    #
    #  Add a net multicast route into the hosts routing table.  Delete
    #  any previous entries first making sure this route is the only one.
    #
    proc mcast_route_default {device} {
	while {![catch "$device rexec route del -net 224.0.0.0/4" result]} {}
	set sta [$device cget -sta]
	set ifdev [lindex $sta 1]
	$device rexec route add -net "224.0.0.0/4" dev $ifdev
    }

    #
    #  Add the host multicast route into the hosts routing table.  Delete
    #  any previous entries first making sure this route is the only one.
    #
    proc mcast_route_addhost {sta grpip} {
	if {[$sta hostis Cygwin WinDhd]} {
	    if {[catch {$sta exec route add $grpip mask 255.255.255.255 [$sta ipaddr]} err]} {
		UTF::Message WARN $sta "Error during multicast route add $err"
	    }
	} else {
	    set dev [$sta cget -device]
	    if {[catch "$sta rexec route add -host $grpip dev $dev" err]} {
		if {[regexp {SIOCADDRT: File exists} $err]} {
		    # Delete any stale routes and add then new one
		    UTF::Message WARN $sta "Delete of existing mcast route $grpip for $sta then retry add route"
		    mcast_route_delhost $sta $grpip
		    $sta rexec route add -host $grpip dev $dev
		}
	    }
	}
    }

    proc mcast_route_delhost {sta grpip} {
	#
	# RJM: Probably should do a better job at this.  Fix it later.
	#
	if {[$sta hostis Cygwin WinDhd]} {
	    catch "$sta rexec route delete $grpip mask 255.255.255.255"
	} else {
	    while {![catch "$sta rexec route del -host $grpip" result]} {}
	}
    }

    #
    # Send an enable/disable emf command to the AP
    #
    proc emf_exec_command {device state} {
	variable emf_initialized
	set bridgedev [$device brname]
	set wldev [$device wlname]
	set output [$device emf status $bridgedev]
	if {![regexp {^Multicast forwarding status: ([A-Z]+)} $output - fwstatus]} {
	    error "EMF status command failed in parsing"
	}
	set state [string toupper $state]
	switch  -exact $fwstatus {
	    "DISABLED" {
		if {$state == "ON" || $state == "ENABLE" || $state == "1"} {
		    set output [$device emf start $bridgedev]
		    set output [$device emf status $bridgedev]
		}
		set emf_initialized "FALSE"
	    }
	    "ENABLED" {
		if {$state == "OFF" || $state == "DISABLE" || $state == "0"} {
		    set output [$device emf stop $bridgedev]
		    set output [$device emf status $bridgedev]
		}
		set output [$device emf add rtport $bridgedev $wldev]
		set emf_initialized "TRUE"
		set output [$device emf list rtport $bridgedev]
		if {[regexp "RTPORT list of ${bridgedev}\: (\[a-z0-9\]+)" $output - dev]} {
		    return "$device emf rtport=$dev"
		} else {
		    error "$device emf rtport not set"
		}
	    }
	    default {
		error "EMF status of $fwstatus unknown"
	    }
	}
    }
    proc test_emf {dut} {
	if {[catch {$dut emf status [$dut brname]} res]} {
	    return 0
	}
	if {[regexp {^Multicast forwarding status: ENABLED} $res]} {
	    return 1
	}
	return 0
    }
    #
    # Test for the emf RTPORT as empty
    #
    proc test_rtport_empty {ap} {
	if {$ap == "" || ![$ap hostis Router]} {
	    error "Invalid device = $ap for rtport test"
	}
	set bridgedev [$ap brname]
	set output [$ap emf list rtport $bridgedev]
	if {[regexp "RTPORT list of ${bridgedev}:(.+)" $output - tmp]} {
	    set tmp [string trim $tmp]
	} else {
	    error "fail rtport parse"
	}
	if {[string length $tmp]} {
	    UTF::Message INFO $ap "nonempty rtport: $tmp"
	    return 0
	} else {
	    return 1
	}
    }

    proc test_igs_rtport {ap ip} {
	if {$ap == "" || ![$ap hostis Router]} {
	    error "Invalid device = $ap for igs rtport test"
	}
	set bridgedev [$ap brname]
	set output [$ap igs list rtport $bridgedev]
	foreach line [split $output "\n"] {
	    if {[regexp "[ip2x $ip] " $line]} {
		return 1
	    }
	}
	return 0
    }

    proc test_sdb_entry {ap grpip ip} {
	if {$ap == "" || ![$ap hostis Router]} {
	    error "Invalid device = $ap for sdb test"
	}
	set bridgedev [$ap brname]
	set xgrpip [ip2x $grpip]
	UTF::Message TEST $ap "Check IGS entry for $grpip (${xgrpip}) ${ip} ([ip2x $ip])"
	set output [$ap igs list sdb $bridgedev]
	foreach line [split $output "\n"] {
	    if {[regexp "[ip2x $grpip]\\\s+[ip2x $ip]" $line]} {
		return 1
	    }
	}
	UTF::Message TEST $ap "No IGS entry for ${grpip}/$ip (${xgrpip}/[ip2x $ip])"
	return 0
    }

    #
    # Validate the emf multicast forwarding database has the entry installed
    #
    proc test_emfdb_add {device grpip} {
	#emf list mfdb br0
	#Group           Interface      Pkts
	#e0000016        eth1           0
	set xgrpip [ip2x $grpip]
	UTF::Message TEST $device "EMFDB check entry for $grpip (${xgrpip})"
	set output [$device emf list mfdb [$device brname]]
	if {[regexp "$xgrpip\[ \]+(\[a-z\]+\[0-9\]+)\[ \]+\[0-9\]+" $output - dev]} {
	    return $dev
	} else {
	    error "emfdb_add fail $device $grpip ($xgrpip)"
	}
    }
    #
    # Validate the emf multicast forwarding database has the entry removed
    #
    proc test_emfdb_delete {device grpip} {
	#emf list mfdb br0
	#Group           Interface      Pkts
	#e0000016        eth1           0
	set retrytest 2
	while {$retrytest} {
	    incr retrytest -1
	    set output [$device emf list mfdb [$device brname]]
	    set xgrpip [ip2x $grpip]
	    if {[regexp "$xgrpip\[ \]+(\[a-z\]+\[0-9\]+)\[ \]+\[0-9\]+" $output - dev]} {
		UTF::Sleep 1.0
	    } else {
		return 1
	    }
	}
	error "emfdb_del fail $device $grpip ($xgrpip)"
    }

    proc show_emfdb {aps} {
	foreach ap $aps {
	    set output [$ap emf list mfdb [$ap brname]]
	}
    }

    proc show_emfdb_rates {args} {
	UTF::Getopts {
	    {ap.arg ""}
	    {dwell.arg "1.0"}
	    {v ""}
	}
	set duts $(ap)
	set sleeptime $(dwell)
	set bridgedev [$(ap) brname]
	foreach dut $duts {
	    set times($dut,timea) [clock clicks -milliseconds]
	    set output($dut,timea) [$dut emf list mfdb $bridgedev]
	}
	UTF::Sleep $sleeptime
	foreach dut $duts {
	    set times($dut,timeb) [clock clicks -milliseconds]
	    set output($dut,timeb) [$dut emf list mfdb $bridgedev]
	}
	foreach dut $duts {
	    foreach line [split $output($dut,timea) "\n"] {
		if {[regexp {(e[0-9a-fA-F]{7,7})[\s]+(eth[0-9]+|vlan[0-9]+)[\s]+([0-9]+)} $line - hexgrp dev pkts]} {
		    set a($dut,$hexgrp,$dev) $pkts
		}
	    }
	    foreach line [split $output($dut,timeb) "\n"] {
		if {[regexp {(e[0-9a-fA-F]{7,7})[\s]+(eth[0-9]+|vlan[0-9]+)[\s]+([0-9]+)} $line - hexgrp dev pkts]} {
		    set b($dut,$hexgrp,$dev) $pkts
		}
	    }
	}
	UTF::Message INFO "" "Router\tGroup IP\tGroup (hex)\tdev\tpps"
	foreach element [lsort [array names b]] {
	    if {[catch {expr {$b($element) - $a($element)}} results]} {
		continue
	    } else {
		set tmp [split $element ,]
		set dut [lindex $tmp 0]
		set hex [lindex $tmp 1]
		set ip [x2ip $hex]
		set dev [lindex $tmp 2]
		set pps [expr {int($results * 1000 /($times($dut,timeb) - $times($dut,timea)))}]
		if {$pps || $(v)} {
		    UTF::Message INFO "" "$dut\t$ip\t$hex\t$dev\t$pps"
		}
	    }
	}
    }

    proc stainfo {ap {sleeptime 1.0}} {
	set output [$ap wl assoclist]
	foreach line [split $output "\n"] {
	    if {[regexp {assoclist[\s]+([0-9A-Fa-f]{2,2}:[0-9A-Fa-f]{2,2}:[0-9A-Fa-f]{2,2}:[0-9A-Fa-f]{2,2}:[0-9A-Fa-f]{2,2}:[0-9A-Fa-f]{2,2})} $line - da]} {
		set stainfoa($da) [$ap wl sta_info $da]
		set times(a,$da) [clock clicks -milliseconds]
	    }
	}
	UTF::Sleep $sleeptime
	foreach da [array names stainfoa] {
	    set stainfob($da) [$ap wl sta_info $da]
	    set times(b,$da) [clock clicks -milliseconds]
	}
	UTF::Message INFO "" "STA\t\t\tpps"
	foreach da [array names stainfoa] {
	    if {![regexp {[\s]+tx pkts:[\s]+([0-9]+)} $stainfoa($da) - pktsa]} {
		continue
	    }
	    if {![regexp {[\s]+tx pkts:[\s]+([0-9]+)} $stainfob($da) - pktsb]} {
		continue
	    }
	    set pps [expr {int(($pktsb - $pktsa) * 1000 / ($times(b,$da) - $times(a,$da)))}]
	    UTF::Message INFO "" "$da\t$pps"
	}
    }

    proc show_igs {aps} {
	foreach ap $aps {
	    set output [$ap igs list sdb [$ap brname]]
	    set output [$ap igs list rtport [$ap brname]]
	    set output [$ap igs show stats [$ap brname]]
	}
    }

    proc test_wmf {sta} {
	return [$sta wl wmf_bss_enable]
    }
    proc enable_wmf {sta} {
	$sta wl wmf_bss_enable 1
    }
    proc disable_wmf {sta} {
	$sta wl wmf_bss_enable 0
    }
    proc txcounter {sta} {
	set output [$sta wl counters]
	if {[regexp {d11_txmulti ([\d]+) } $output - count]} {
	    return $count
	} else {
	    return -1
	}
    }
    proc rxcounter {sta} {
	set output [$sta wl counters]
	if {[regexp {rxdfrmmcast ([\d]+) } $output - count]} {
	    return $count
	} else {
	    return -1
	}
    }
    proc mcastrates {stas {sleeptime 1.0}} {
	set start [clock clicks -milliseconds]
	foreach sta $stas {
	    set first($sta) [$sta wl counters]
	}
	UTF::Sleep $sleeptime
	foreach sta $stas {
	    set second($sta) [$sta wl counters]
	}
	set finish [clock clicks -milliseconds]
	foreach sta $stas {
	    set parseok 1
	    if {![regexp {d11_txmulti ([\d]+) } $first($sta) - txcounta]} {
		set txcounta "parseerr"
		set parseok 0
	    }
	    if {![regexp {rxdfrmmcast ([\d]+) } $first($sta) - rxounta]} {
		set rxcounta "parseerr"
		set parseok 0
	    }
	    if {![regexp {d11_txmulti ([\d]+) } $second($sta) - txcounta]} {
		set txcountb "parseerr"
		set parseok 0
	    }
	    if {![regexp {rxdfrmmcast ([\d]+) } $second($sta) - rxounta]} {
		set rxcountb "parseerr"
		set parseok 0
	    }
	    if {$parseok} {
		set rxrate($sta) [expr {($rxcountb - $rxcounta) * 1000 / ($start - $finish)}]
		set txrate($sta) [expr {($txcountb - $txcounta) * 1000 / ($start - $finish)}]
	    }
	}
	return [list [array get rxrate] [array get txrate]]
    }

    proc log_mcastrates {rx tx} {
	array set rxrates $rx
	array set txrates $tx
	foreach sta [array names rxrates] {
	    UTF::Message RATES $sta "RXRate = $rxrates($sta) TXRate = $txrates($sta)"
	}
    }

    proc IPv4? {ip} {
	set octets [split $ip .]
	if {[llength $octets] != 4} {
	    return -code error "invalid ip address \"$ip\""
	}
	for {set ix 1} {$ix < 4} {incr ix} {
	    set oct [lindex $octets $ix]
	    if {$oct < 0 || $oct > 255} {
		return -code error "invalid ip address \"$ip\""
	    }
	}
	set oct [lindex $octets 0]
	if {$oct < 224 || $oct > 239} {
	    return 0
	}
	return 1
    }

    #
    # Adjust kernel settings
    #
    proc force_igmp_version {sta {version "2"}} {
	if {![$sta hostis Linux]} {
	    error "IGMP force version not implemented yet"
	}
	set dev [$sta cget -device]
	$sta rexec "echo $version > /proc/sys/net/ipv4/conf/${dev}/force_igmp_version"
	set dev [join [split $dev .] /]
	set sysctlstring "net.ipv4.conf.${dev}.force_igmp_version"
	set output [$sta rexec sysctl $sysctlstring]
	if {![regexp "$sysctlstring = (\[0-3\])" $output - version]} {
	    error "couldn't set igmp version"
	} else {
	    return $version
	}
    }

    # RJM - need to do a capabilities check
    proc ConfigEMF {args} {
	UTF::Getopts {
	    {ap.arg "" "AP list to configure EMF"}
	    {noapply "Only return the nvram settings"}
	}
	if {$(ap) == {}} {
	    error "must pass -ap"
	}
	set nvrams ""
	foreach ap $(ap) {
	    if {[test_wmf $ap]} {
		set wmfsetting 1
		set lazywds 0
	    } else {
		set wmfsetting 0
		set lazywds 1
	    }
	    set wlname [$ap wlname]
	    set nvramstring "emf_enable=1 wl_wmf_bss_enable=$wmfsetting wl_lazywds=$lazywds ${wlname}_wmf_bss_enable=$wmfsetting"
	    if {!$(noapply)} {
		eval [concat $ap reboot $nvramstring]
	    } else {
		lappend nvrams $nvramstring
	    }
	}
	# return in the same order of the ap list
	return $nvrams
    }
    proc ip2x {ip {validate 0}} {
	set octets [split $ip .]
	if {[llength $octets] != 4} {
	    set octets [lrange [concat $octets 0 0 0] 0 3]set
	}
	if {$validate} {
	    foreach oct $octets {
		if {$oct < 0 || $oct > 255} {
		    return -code error "invalid ip address"
		}
	    }
	}
	binary scan [binary format c4 $octets] H8 x
	return $x
    }
    proc x2ip {hex} {
	set r {}
	set hex "0x$hex"
	set bin [binary format I [expr {$hex}]]
	binary scan $bin c4 octets
	foreach octet $octets {
	    lappend r [expr {$octet & 0xFF}]
	}
	return [join $r .]
    }
    proc grpip2MAC {ipaddr} {
	if {[regexp {2(2[4-9])|(3[0-9])\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})} $ipaddr - ip2 ip3 ip4]} {
	    set mac2 [expr {$ip2 & 127}]
	} else {
	    UTF::Message ERROR $utf_msgtag "$ipaddr is not a valid multicast group ip addr"
	}
	return [format "01.00.5e.%02x.%02x.%02x" $mac2 $ip3 $ip4]
    }
}

