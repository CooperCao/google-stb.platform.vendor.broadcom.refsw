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

package provide UTF::tcpdump 2.0

snit::type UTF::tcpdump {
    option -sta -readonly true
    option -name -default "" -readonly true
    option -snarf -type integer -default 96
    option -debug -type boolean -default 0
    option -usestdbuf -type boolean -default 0
    option -monitor -type boolean -default 0

    variable fid
    variable events -array {}
    variable utfmsgtag
    variable message_ratelimiter

    constructor {args} {
	$self configurelist $args
	if {[catch {$options(-sta) cget -device} dev] || $dev eq ""} {
	    error "device for $options(-sta) not found"
	}
	if {![$options(-sta) hostis Linux MacOS]} {
	    error "tcpdump not supported for OS on $options(-sta)"
	}
	if {$options(-name) eq ""} {
	    set utfmsgtag "[namespace tail $self]"
	    set options(-name) $utfmsgtag
	} else {
	    set utfmsgtag $options(-name)
	}
	set events(state) "INIT"
    }
    destructor {
	$self stop
    }
    method start {args} {
	UTF::Getopts {
	    {igmp "enable igmp filter"}
	    {mac "enable mac addresses in display"}
	    {ack "enable tcp ack filter"}
	    {arp "sniff arp traffic"}
	    {dhcp "enable dhcp filter"}
	    {wget "enable wget"}
	}
	if {[info exists fid]} {
	    return
	}
	set tcpdumpcmd "/usr/sbin/tcpdump"
	if {$options(-monitor)} {
	    append tcpdumpcmd " -I"
	}
	if {$(mac)} {
	   append tcpdumpcmd  " -e"
	}
	append tcpdumpcmd " -i [$options(-sta) cget -device] -p -s $options(-snarf) -tt -nn -l"
	if {$(arp)} {
	    append tcpdumpcmd { arp}
	}
	if {$(dhcp)} {
	    append tcpdumpcmd { port 67 or port 68}
	}
	if {$(wget)} {
#	    append tcpdumpcmd { arp or \\\(ip and \\\(port 67 or port 68 or port 80 or port 3128\\\)\\\)}
#	    append tcpdumpcmd { arp or ip}
	}
	if {$options(-usestdbuf)} {
	    set tcpdumpcmd "/usr/local/bin/gstdbuf -oL $tcpdumpcmd"
	}
	if {$options(-debug)} {
	    UTF::Message DEBUG "$options(-sta)"  $tcpdumpcmd
	}
	if {[catch {eval [concat $options(-sta) rpopen -noinit $tcpdumpcmd]} fid]} {
	    set msg "tcpdump start failed: $fid"
	    UTF::Message ERROR $utfmsgtag $msg
	    error $msg
	} else {
	    set state "INIT"
	}
	fconfigure $fid -blocking 1 -buffering line
	fileevent $fid readable [mymethod __tcpdump_handler [$options(-sta) cget -device]]
	set events(watchdog) "PENDING"
	set watchdog [after 2000 [list set [myvar events(watchdog)] "TIMEOUT"]]
	while {$events(state) ne "LISTENING"} {
	    vwait [myvar events]
	    if {$events(watchdog) eq "TIMEOUT"} {
		break
	    }
	}
	if {![catch {after info $watchdog}]} {
	    after cancel $watchdog
	    UTF::_Message SNIFF $options(-sta) "Started per $utfmsgtag"
	} else {
	    UTF::_Message SNIFF $options(-sta) "Start ERROR per $utfmsgtag"
	}
    }

    method stop {} {
	if {![info exists fid]} {
	    return
	}
	if {[catch {exec kill -s INT [pid $fid]} err]} {
	    UTF::Message ERROR $utfmsgtag $err
	} else {
	    set events(watchdog) "PENDING"
	    set watchdog [after 2000 [list set [myvar events(watchdog)] "TIMEOUT"]]
	    while {$events(state) ne "CLOSED"} {
		vwait [myvar events]
		if {$events(watchdog) eq "TIMEOUT"} {
		    break
		}
	    }
	}
	if {![catch {after info $watchdog}]} {
	    after cancel $watchdog
	    UTF::_Message SNIFF $options(-sta) "Stopped per $utfmsgtag"
	} else {
	    UTF::_Message SNIFF $options(-sta) "Stop ERROR per $utfmsgtag"
	    if {$events(state) ne "CLOSED"} {
		catch {exec kill -s HUP [pid $fid]}
	    }
	}
    }
    method status {} {
	return [info exists fid]
    }
    method __tcpdump_handler {dev} {
	if {[eof $fid]} {
	    if {![catch {close $fid} err]} {
		set events(state) "CLOSED"
		unset fid
	    } else  {
		UTF::_Message ERROR $utfmsgtag $err
		set events(state) "ERROR"
	    }
	} else {
	    set buf [gets $fid]
	    if {$options(-debug)} {
		UTF::_Message HNDLR $utfmsgtag $buf
	    }
	    if {[regexp "^listening on $dev" $buf]} {
		set events(state) "LISTENING"
	    } elseif {[regexp {([0-9]+) packets dropped by kernel} $buf]} {
		UTF::Message SNIFF $utfmsgtag $buf
	    } elseif {[regexp {^([0-9]+\.[0-9]{6,6})(.*)} $buf - tt line]} {
		foreach {m f} [split $tt .] {}
		set timestamp "[clock format $m -format %H:%M:%S].$f"
		UTF::_Message SNIFF  $options(-sta) "[string trim $line] at $timestamp per $utfmsgtag"
	    }
	}
    }
}
