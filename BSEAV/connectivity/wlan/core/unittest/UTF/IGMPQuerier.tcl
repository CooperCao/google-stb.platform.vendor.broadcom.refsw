#!/bin/env utf
#
# IGMP querier control for UTF
# Author Robert McMahon July, 22 2010
#
# Requires igmpquerier written as a separate C program
# Source for that is in src/tools/misc/igmpquerier
#
# $Id: 58e010a0c751537abe8f50180d9d5cc65670ca6e $
# $Copyright Broadcom Corporation$
#
package require snit
package require UTF
package require UTF::Multicast

package provide UTF::IGMPQuerier 2.0

snit::type UTF::IGMPQuerier {
    typevariable FASTTIMEOUT 60000

    option -ap
    option -reportinterval -default 30 -configuremethod __configureoptionwithapply
    option -command ""
    option -bincmd {/usr/local/bin/igmp_querier}
    option -silent -type boolean -default 0
    option -querygroup -default "224.0.0.1" -validatemethod __validate_mcastaddr

    variable igmpq -array {}
    variable tcpdumpfid
    variable tcpdump -array {}
    variable sniffbuffer {}
    variable everyid
    variable utf_msgtag

    typemethod alligmpqueriers {args} {
	set instances [$type info instances]
	foreach instance $instances {
	    eval $instance [lindex $args 0] [lrange $args 1 end]
	}
    }

    typemethod IGMPQuerierExit {} {
	eval [mytypemethod alligmpqueriers "destroy"]
    }

    constructor {args} {
	$self configurelist $args
	set utf_msgtag "[namespace tail $self]"
	set igmpq(state) "CLOSED"
	set igmpq(pendingcount) 0
    }

    destructor {
	$self stop
	if {[info exists tcpdump(state)] && $tcpdump(state) == "LISTENING"} {
	    $self sniff off
	}
    }

    method __validate_mcastaddr {option value} {
	if {![UTF::Multicast::IPv4? $value]} {
	    set msg "Invalid multicast address of $value"
	    UTF::Message ERROR $utf_msgtag
	    error $msg
	}
    }

    #
    # Start the igmp querier
    #
    method start {} {
	if {[info exists igmpq(fid)]} {
	    return
	}
	if {$options(-ap) == {}} {
	    error "IGMP querier ($utf_msgtag) configure error: no -ap"
	}
	set transmitter [$options(-ap) lan]
	if {$transmitter == {}} {
	    error "IGMP querier ($utf_msgtag) configure error: no -lanpeer"
	}
	if {![$transmitter hostis Linux]} {
	    error "IGMP querier ($utf_msgtag) not supported on $transmitter"
	}
	set options(-command) "$options(-bincmd) -d -g $options(-querygroup)"
	set igmpq(state) "STARTING"
	if {[catch  {$transmitter rpopen "$options(-command)"} igmpq(fid)]} {
	    error "IGMP querier($utf_msgtag) start err=$igmpq(fid)"
	}
	fconfigure $igmpq(fid) -blocking 1 -buffering line
	fileevent $igmpq(fid) readable [mymethod __igmpqhandler $igmpq(fid)]
	#
	# Wait for the igmpq handler to start
	#
	set igmpq(wd) "PENDING"
	set watchdog [after $FASTTIMEOUT [list set [myvar igmpq(wd)] "TIMEOUT"]]
	while {$igmpq(state) != "RUNNING"} {
	    vwait [myvar igmpq]
	    if {$igmpq(wd) != "TIMEOUT"} {
		continue
	    } else {
		break
	    }
	}
	if {$igmpq(wd) == "TIMEOUT"} {
	    error "$utf_msgtag Start timeout"
	} else {
	    after cancel $watchdog
	}
	UTF::Message INFO [$options(-ap) cget -name]  "$utf_msgtag $options(-querygroup) started with interval = $options(-reportinterval)"
	if {$options(-reportinterval) > 0} {
	    set everyid [UTF::Every $options(-reportinterval) [mymethod send]]
	}
	set sniffbuffer {}
    }

    method stop {} {
	if {[info exists igmpq(fid)]} {
	    UTF::Message INFO [$options(-ap) cget -name] "Stopping $utf_msgtag $options(-querygroup)"
	    if {[catch {exec kill -s SIGINT [pid $igmpq(fid)]} result]} {
		UTF::Message ERROR $utf_msgtag $result
	    }
	    set igmpq(wd) "PENDING"
	    set watchdog [after $FASTTIMEOUT [list set [myvar igmpq(wd)] "TIMEOUT"]]
	    while {$igmpq(state) != "CLOSED"} {
		vwait [myvar igmpq]
		if {$igmpq(wd) != "TIMEOUT"} {
		    continue
		} else {
		    break
		}
	    }
	    if {$igmpq(wd) == "TIMEOUT"} {
		error "$utf_msgtag Stop timeout"
	    } else {
		after cancel $watchdog
	    }
	    unset igmpq(fid)
	}
    }

    method send {args} {
	UTF::Getopts {
	    {noblock "block on iqmp report being sent"}
	}
	if {![info exists igmpq(fid)]} {
	    UTF::Message WARN $utf_msgtag "IGMP Querier not running and send requested. Starting querier w/report interval 0."
	    set $options(-reportinterval) 0
	    $self start
	}
	if {[info exists igmpq(fid)]} {
	    incr igmpq(pendingcount) +1
	    if {!$options(-silent) && !$(noblock)} {
		    UTF:::Message INFO [$options(-ap) cget -name] "IGMP Querier post for $options(-querygroup) ($igmpq(pendingcount))"
	    }
	    if {[catch {exec kill -s SIGUSR1 [pid $igmpq(fid)]} err]} {
		incr igmpq(pendingcount) -1
		UTF:::Message ERROR $utf_msgtag "IGMP Querier error: $err"
	    } else {
		catch {after cancel $igmpq(aid)}
		set igmpq(aid) [after 500 [list set [myvar igmpq(pendingcount)] 0]]
		if {!$(noblock)} {
		    while {$igmpq(pendingcount) > 0} {
			vwait [myvar igmpq(pendingcount)]
		    }
		}
	    }
	} else {
	    set msg "IGMP Querier send failed for AP=$options(-ap)"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
    }

    method sniff {option} {
     	if {$option == "options"} {
	    return [list off on]
	}
	if {$options(-ap) == {}} {
	    error "IGMP querier ($utf_msgtag) configure error: no -ap"
	}
	set transmitter [$options(-ap) cget -lanpeer]
	if {$transmitter == {}} {
	    error "IGMP querier ($utf_msgtag) configure error: no -lanpeer"
	}
	if {![$transmitter hostis Linux]} {
	    error "IGMP querier ($utf_msgtag) not supported on $transmitter"
	}
	set ifdev [$transmitter cget -device]
	switch -exact $option {
	    "on" {
		if {![info exists tcpdump(state)] || $tcpdump(state) != "LISTENING"} {
		    set tcpdumpfid [$transmitter rpopen "/usr/sbin/tcpdump -tt -i $ifdev -s 96 -n -l ip proto 2"]
		    #
		    # Setup read handler for the tcpdump
		    #
		    set tcpdump(state) "INIT"
		    fconfigure $tcpdumpfid -blocking 1 -buffering line
		    fileevent $tcpdumpfid readable [mymethod __tcpdumphandler $tcpdumpfid]
		    #
		    # Wait for the tcpdump handler to start
		    #
		    set tcpdump(wd) "PENDING"
		    set watchdog [after $FASTTIMEOUT [list set [myvar tcpdump(wd)] "TIMEOUT"]]
		    while {$tcpdump(state) != "LISTENING"} {
			vwait [myvar tcpdump]
			if {$tcpdump(wd) != "TIMEOUT"} {
			    continue
			} else {
			    break
			}
		    }
		    if {$tcpdump(wd) == "TIMEOUT"} {
			UTF::Message INFO $utf_msgtag "Time out waiting for tcpdump"
		    } else {
			after cancel $watchdog
		    }
		}
	    }
	    "off" {
		if {$tcpdump(state) == "LISTENING"} {
		    if {[catch {exec kill -s SIGINT [pid $tcpdumpfid]} result]} {
			UTF::Message ERROR $utf_msgtag $result
		    }
		}
	    }
	    default {
		error "$utf_msgtag invalid sniff option $option"
	    }
	}
    }

    method status {} {
	if {[info exists igmpq(fid)]} {
	    UTF::Message $utf_msgtag INFO "IGMPQuerier state = $igmpq(state)"
	    UTF::Message $utf_msgtag LOG $sniffbuffer
	    set sniffbuffer {}
	} else {
	    UTF::Message $utf_msgtag INFO "IGMPQuerier stopped"
	}
	switch -exact $igmpq(state) {
	    "CLOSED" {
		return 0
	    }
	    "RUNNING" {
		return 1
	    }
	    default {
	       return -1
	    }
	}
    }

    method __igmpqhandler {fid} {
	if {[eof $fid]} {
	    if {[catch {close $fid} err]} {
		UTF::Message WARN $utf_msgtag "close error: $err"
	    }
	    if {[info exists everyid]} {
		UTF::Every cancel $everyid
	    }
	    set igmpq(state) "CLOSED"
	} else {
	    set line [gets $fid]
	    if {[regexp {^IGMP All Hosts Querier (\(pid=[0-9]+\))} $line - rpid]} {
		set igmpq(state) "RUNNING"
		set igmpq(pid) $rpid
	    } elseif {[regexp {^Sent IGMP all hosts querier to} $line]} {
	        incr igmpq(pendingcount) -1
		if {[info exists igmpq(aid)]} {
		    after cancel $igmpq(aid)
		    if {$igmpq(pendingcount)} {
			set igmpq(aid) [after 500 [list set [myvar igmpq(pendingcount)] 0]]
		    }
		}
	    }
	    if {!$options(-silent) && $line ne ""} {
		UTF::Message LOG [$options(-ap) cget -name] "$line ($igmpq(pendingcount))"
	    }
	}
    }

    method __tcpdumphandler {fid} {
	if {[eof $fid]} {
	    if {[catch {close $fid} err]} {
		UTF::Message WARN $utf_msgtag "tcpdump close error: $err"
	    }
	    set tcpdump(state) "CLOSED"
	    unset fid
	} else {
	    set line [gets $fid]
	    append sniffbuffer $line
	    UTF::Message LOG $utf_msgtag $line
	    if {[regexp {^listening on } $line]} {
		set tcpdump(state) "LISTENING"
	    }
	}
    }

    method __configureoptionwithapply {option value} {
	if {$options($option) == $value} {
	    return
	}
	set options($option) $value
	if {[info exists igmpq(fid)]} {
	    $self stop; $self start
	}
    }


}

if {$::tcl_interactive} {
    if {![catch {package present TclReadLine}]} {
	TclReadLine::addExitHandler ::UTF::IGMPQuerier IGMPQuerierExit
    } else {
	lappend ::__install_interactive_exit_callbacks [list ::UTF::IGMPQuerier IGMPQuerierExit]
    }
}
