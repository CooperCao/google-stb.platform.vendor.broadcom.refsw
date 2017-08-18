#
# UTF object for tcpdump
#
#
# Written by: Robert J. McMahon May 2015
#
# $Copyright Broadcom Corporation$
#

package require UTF
package require snit
package require UTF::Streams

package provide UTF::KPI::portmirror 2.0

snit::type UTF::KPI::portmirror {
    option -sta -readonly true
    option -dev -readonly true
    option -ap -readonly true
    option -name -default "" -readonly true
    option -snarf -type integer -default 96
    option -filters
    option -stream
    option -period -type integer -default 1
    option -debug -type boolean -default 0
    option -verbose -type boolean -default 0
    option -displayseqno -type boolean -default 0
    option -triggercallback -default {}
    option -clockref -default "tcpdump"
    option -holdtime -default 0.5

    variable myfid
    variable mystate
    variable watchdog
    variable utfmsgtag
    variable first -array {}
    variable lasttime
    variable stats -array {}
    variable seqno -array {}
    variable lasttcpdumpts

    constructor {args} {
	$self configurelist $args
	if {$options(-ap) ne ""} {
	    set sta [$options(-ap) cget -portmirror]
	    set dev [$sta cget -device]
	    if {$sta eq "" || $dev eq ""} {
		error "$options(-ap) -portmirror not configured properly"
	    } else {
		set options(-sta) $sta
		set options(-dev) $dev
	    }
	    if {$options(-name) eq ""} {
		set options(-name) PM_$options(-ap)
	    }
	}
	if {$options(-name) eq ""} {
	    set utfmsgtag "[namespace tail $self]"
	    set options(-name) $utfmsgtag
	} else {
	    set utfmsgtag $options(-name)
	}
	set mystate "CLOSED"
	set stats(pktcount) 0
    }
    destructor {
	$self stop
    }
    method start {args} {
	UTF::Getopts {
	    {arp "enable arp filter"}
	    {igmp "enable igmp filter"}
	    {mac "enable mac addresses in display"}
	    {dhcp "enable dhcp filter"}
	    {period.arg "1" "rate limiter period"}
	    {threshold.arg "6" "duplicate threshold"}
	    {f "force a pkill before the start"}
	}
	if {[info exists myfid]} {
	    return
	}
	if {$options(-stream) eq ""} {
	    error "port mirror needs a UTF::stream configured"
	}
	set triggered 0
	catch {unset lastttraffic}
	array unset seqno *
	array unset first *
	set stats(pktcount) 0
	set tcpdumpcmd "/usr/sbin/tcpdump"
	if {$(mac)} {
	    lappend tcpdumpcmd "-e"
	}
	lappend tcpdumpcmd -i $options(-dev) -l
	lappend tcpdumpcmd [string tolower [$options(-stream) cget -protocol]] and src [[$options(-stream) cget -transmitsta] ipaddr] and dst [[$options(-stream) cget -receivesta] ipaddr] and dst port [$options(-stream) cget -dstport]
	lappend tcpdumpcmd -s $options(-snarf) -tt -nn -x
	if {$(arp)} {
	    lappend tcpdumpcmd arp
	}
	if {$(dhcp)} {
	    lappend tcpdumpcmd port 67 or port 68
	}
	if {$(f)} {
	    catch {$options(-sta) rexec -x pkill -fx '$tcpdumpcmd'}
	}
	if {[catch {$options(-sta) rpopen {*}$tcpdumpcmd} myfid]} {
	    set msg "tcpdump start failed: $myfid"
	    UTF::Message ERROR $utfmsgtag $msg
	    catch {unset myfid}
	    error $msg
	}
	fconfigure $myfid -blocking 0 -buffering line
	fileevent $myfid readable [mymethod __tcpdump_handler $options(-dev)]
	set mystate "STARTPENDING"
	set watchdog [after 2000 [list set [myvar mystate] "TIMEOUT"]]
	while {$mystate ne "RUNNING"} {
	    vwait [myvar mystate]
	    if {$mystate eq "TIMEOUT"} {
		UTF::Message ERROR $utfmsgtag "Port mirror start timeout"
		if {[catch {exec kill -s HUP [pid $myfid]} err]} {
		    UTF::Message ERROR $utfmsgtag $err
		}
		if {[catch {close $myfid} err]} {
		    UTF::Message ERROR $utfmsgtag $err
	        }
		error "port mirror start failed per tcpdump timeout"
	    }
	}
	if {$mystate eq "RUNNING"} {
	    UTF::Message INFO $utfmsgtag "Port mirror enabled"
	    catch {after cancel $watchdog}
	} else {
	    $self stop
	    error "Port mirror start failed"
	}
    }
    method isActive? {} {
	set stats(active) "1"
	set aid [after [expr {int($options(-holdtime) * 2 * 1000)}] [list set [myvar stats(active)] "0"]]
	vwait [myvar stats]
	catch {after cancel $aid}
	return $stats(active)
    }
    method stop {} {
	if {$mystate eq "RUNNING"} {
	    set mypid [pid $myfid]
	    if {![catch {exec kill -s HUP $mypid} err]} {
		UTF::Message INFO $utfmsgtag "Kill signal -HUP sent to tcpdump $mypid"
		set aid [after [expr {2 * 1000}] [list set [myvar mystate] "TIMEOUT"]]
		while {$mystate eq "RUNNING"} {
		    vwait [myvar mystate]
		}
		catch {after cancel $aid}
	    } else {
		UTF::Message ERROR $utfmsgtag "Kill -s HUP fail : $err $myfid $mypid)"
	    }
	}
	if {$mystate ne "CLOSED"} {
	    UTF::Message ERROR $utfmsgtag "Stop failed: state = $mystate"
	}
    }
    method count {} {
	return $stats(pktcount)
    }
    method first {args} {
	UTF::Getopts {
	    {timestamp "Return timestamp in log format"}
	}
	if {[info exists first(ms)]} {
	    if {$(timestamp)} {
		return $first(timestamp)
	    } else {
		return $first(ms)
	    }
	} else {
	    return -1
	}
    }
    method last {} {
	if {[info exists lasttime]} {
	    return $lasttime
	} else {
	    return -1
	}
    }
    method pkill {} {
	catch {$options(-sta) rexec -x pkill -f '/usr/sbin/tcpdump'}
    }
    method status {} {
	return [info exists myfid]
    }
    method seqno {args} {
	UTF::Getopts {
	    {first "Return the first seq no"}
	    {last "Return the last seq no"}
	}
	if {$(first)} {
	    if {[info exists seqno(first)]} {
		return "[expr $seqno(first)] ($seqno(first))"
	    } else  {
		return -1
	    }
	} else {
	    if {[info exists seqno(last)]} {
		return "[expr $seqno(last)] ($seqno(last))"
	    } else  {
		return -1
	    }
	}
    }
    method __tcpdump_handler {dev} {
	set len [gets $myfid buf]
	if {[eof $myfid]} {
	    set mystate CLOSED
	    catch {close $myfid}
	    UTF::_Message HNDLR $utfmsgtag "Port mirror closed $myfid"
	    unset myfid
	}
	if {$len > 0} {
	    if {$mystate ne "RUNNING"} {
		UTF::_Message HNDLR $utfmsgtag "$buf $dev ($myfid)"
	    } elseif {$options(-debug)} {
		UTF::_Message DEBUG $utfmsgtag $buf
	    }
	    #1442971346.482437 IP 192.168.1.100.61001 > 192.168.1.42.61001: UDP, length 8
	    # 	  0x0010:  c0a8 012a ee4a ee4a 0208 43c1 193a 4212
	    if {[regexp {^([0-9]+\.[0-9]{6,6})(.*)} $buf - lasttcpdumpts line]} {
		set mystate "RUNNING"
		incr stats(pktcount)
		if {$options(-clockref) eq "tcpdump"} {
		    set lasttime [expr {round($lasttcpdumpts * 1000)}]
		} else {
		    set lasttime [clock milliseconds]
		}
		if {![info exists first(ms)]} {
		    set first(ms) $lasttime
		    set first(timestamp) [UTF::stream clock]
		    foreach {seconds r} [split $lasttcpdumpts .] {}
		    set logformat "[clock format $seconds -format %T]"
		    UTF::_Message INFO $utfmsgtag "First packet (${logformat}): $buf"
		    if {$options(-triggercallback) ne ""} {
			eval $options(-triggercallback)
			set options(-triggercallback) {}
		    }
		}
	    } elseif {[regexp {\s+0x0010:\s+[0-9a-f]{4,4} [0-9a-f]{4,4} [0-9a-f]{4,4} [0-9a-f]{4,4} [0-9a-f]{4,4} [0-9a-f]{4,4} ([0-9a-f]{4,4}) ([0-9a-f]{4,4})} $buf - up low]} {
		if {![info exists seqno(first)]} {
		    set seqno(first) "0x${up}$low"
		    UTF::_Message INFO $utfmsgtag "First SeqNo: $seqno(first)"
		}
		set seqno(last) "0x${up}$low"
		if {$options(-displayseqno)} {
		    if {[info exists lasttcpdumpts]} {
			foreach {seconds r} [split $lasttcpdumpts .] {}
			set logformat "[clock format $seconds -format %T]"
			if {[expr {abs([clock seconds] - $seconds) > 2}]} {
			    set msgtxt "TimeSyncErr SeqNo=$seqno(last) at ${logformat}\($lasttcpdumpts\)  "
			} else {
			    set msgtxt "SeqNo=$seqno(last) at ${logformat}\($lasttcpdumpts\)  "
			}
			UTF::Message HNDLR $utfmsgtag $msgtxt
			unset lasttcpdumpts
		    } else {
			UTF::Message HNDLR $utfmsgtag "SeqNo=$seqno(last) at *"
		    }
		}
	    } elseif {[regexp "^listening on $dev" $buf]} {
		set mystate "RUNNING"
	    } elseif {[regexp {([0-9]+) packets dropped by kernel} $buf - pktlosscnt]} {
		if {$pktlosscnt > 0} {
		    UTF::_Message HNDLR $utfmsgtag "Port mirror lost packets"
		}
	    }
	}
    }
}
