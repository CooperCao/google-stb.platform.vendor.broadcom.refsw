#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 9b9738884ac6ebb9f072e846547cabea65701d45 $
# $Copyright Broadcom Corporation$
#

package provide UTF::Q 2.0

namespace eval UTF::Q {

    variable socket

    proc start_server {server} {
	variable yumdone 0
	if {[catch {$server rexec -n "$UTF::unittest/bin/utfq.tcl"} ret]} {
	    if {!$yumdone && ![regexp {can't find package Tclx} $ret]} {
		error $ret $::errorInfo
	    }
	    set yumdone 1
	    $server rexec -t 120 -n yum install -y tclx
	    $server rexec -n "$UTF::unittest/bin/utfq.tcl"
	}
    }

    proc stop_server {server} {
	$server rexec -n "pkill utfq.tcl"
    }

    proc connect {what who host} {
	variable socket
	if {[catch {socket $host 6700} socket($what)]} {
	    start_server $who
	    set socket($what) [socket $host 6700]
	}
	fconfigure $socket($what) -buffering line
    }

    proc request {what {who lan}} {
	variable socket
	if {[info exists socket($what)] && ![catch {fconfigure $socket($what) -peername}]} {
	    error "$what: already locked"
	}
	if {[$who info type] eq "::UTF::STA"} {
	    set who [$who cget -host]
	}
	if {[set host [$who cget -lan_ip]] eq ""} {
	    set host [$who cget -name]
	}
	connect $what $who $host
	UTF::Message LOG Q "Requesting $what from $host"
	set time [clock seconds]
	puts $socket($what) [list L $what [pid]]
	set pos 0
	while {1} {
	    while {[gets $socket($what) reply] < 0} {
		# retry
		UTF::Sleep $pos
		connect $what $who $host
		UTF::Message LOG Q "Re-requesting $what from $host"
		puts $socket($what) [list L $what [pid]]
	    }
	    if {[regexp {QUEUED.* (\d+)} $reply - pos]} {
		UTF::Message LOQ Q "QUEUED at posn $pos"
		continue
	    } elseif {$reply eq "LOCKED $what"} {
		break
	    } else {
		error "Error from lock server: $reply"
	    }
	}
	set time [expr {[clock seconds] - $time}]
	UTF::Message LOG Q "$reply after ${time}s"
	fileevent $socket($what) readable [list UTF::Q::_lost $what]
    }

    proc _lost {what} {
	variable socket
	if {[gets $socket($what) reply] >= 0} {
	    UTF::Message WARN Q "Error from lock server: $reply"
	}
	UTF::Message WARN Q "Lock lost - exiting"
	set ::UTF::exit "Lock lost - exiting"
	exit
    }

    proc release {what} {
	variable socket
	if {[info exists socket($what)]} {
	    close $socket($what)
	    unset socket($what)
	    UTF::Message LOG Q "Released $what"
	} else {
	    error "$what: not locked"
	}
    }
}

proc UTF::Q {args} {
    if {$UTF::args(nolock)} {
	UTF::Message WARN Q "UNSAFE - bypassing lock for $args"
	return
    }
    eval UTF::Q::request $args
}

