#!/bin/env utf
#
# Voice call in UTF
# Author Robert McMahon March 2011
#
# $Id$
# $Copyright Broadcom Corporation$
#
package require snit
package require UTF
package require UTF::Streams

package provide UTF::VoiceCall 2.0

snit::type UTF::VoiceCall {
    typevariable LOSTMAXPER 0.05;
    typevariable LOSTMAX 1;
    typevariable JITTERMAX 0.10;
    typevariable LATENCYMAX 150;# units of ms

    option -monitor -type boolean -default 0
    option -interval -type integer -default 1 ;# in seconds
    option -onfailure -default "tclerror"
    option -stas
    option -encoding -default G711
    option -silent -type boolean -default 1
    option -events -type boolean -default 0

    variable call_leg -array {}
    variable state
    variable utf_msgtag
    variable everyid
    variable callerror

    typemethod allcalls {args} {
	set instances [$type info instances]
	set faillist {}
	set cmd [string tolower [lindex $args 0]]
	foreach instance $instances {
	    if {[catch {eval $instance $cmd [lrange $args 1 end]} err]} {
		lappend faillist "[namespace tail $instance]:$err"
	    }
	}
	if {[llength $faillist]} {
	    UTF::Message ERROR VoiceCalls $faillist
	    error $faillist
	}
    }

    constructor {args} {
	set state ""
	set everyid ""
	set callerror 0
	$self configurelist $args
	set utf_msgtag "[namespace tail $self]"
    }
    destructor {
	$self stop
	foreach leg [array names call_leg] {
	    $call_leg($leg) destroy
	}
    }
    method inspect {} {
	puts "\nCall legs are:"
	parray call_leg
	puts "\nMonitor: $everyid"
    }
    method start {} {
	if {$state eq "RUNNING"} {
	    return
	}
	if {$state eq ""} {
	    $self __instantiate_call_legs
	}
	foreach leg [array names call_leg] {
	    $call_leg($leg) start
	}
	if {$options(-monitor)} {
	    set everyid [UTF::Every $options(-interval) [mymethod call_check]]
	}
	set state "RUNNING"
    }
    method stop {} {
	if {$state ne "RUNNING"} {
	    return
	}
	if {$everyid ne ""} {
	    UTF::Every cancel $everyid
	}
	foreach leg [array names call_leg] {
	    $call_leg($leg) stop
	}
	set state "STOPPED"
    }
    method reset {} {
	if {$options(-events)} {
	    set callerror 0
	    $call_leg(tx) stats -clear
	    $call_leg(rx) stats -clear
	    $call_leg(tx) configure -trigger_armed 1
	    $call_leg(rx) configure -trigger_armed 1
	}
    }
    method clear {} {
	$call_leg(tx) stats -clear
	$call_leg(rx) stats -clear
    }
    method call_check {} {
	set txlost [$call_leg(tx) stats -lost]
	set txjitter [$call_leg(tx) stats -jitter]
	set rxlost [$call_leg(rx) stats -lost]
	set rxjitter [$call_leg(rx) stats -jitter]
	if {!$options(-silent)} {
	    UTF::Message INFO $utf_msgtag "a->b lost: $txlost"
	    UTF::Message INFO $utf_msgtag "a->b jitter: $txjitter"
	    UTF::Message INFO $utf_msgtag "b->a lost: $rxlost"
	    UTF::Message INFO $utf_msgtag "b->a jitter: $rxjitter"
	}
	if {$callerror} {
	    # Reset call error flag for next call_check
	    set callerror 0
	    set msg "Callcheck indicates failed"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
    }
    method __instantiate_call_legs {} {
	if {[llength $options(-stas)] != 2} {
	    set msg "voice call must have exactly two endpoints"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	set triggercallback [mymethod __checkstream]
	set call_leg(tx) [UTF::stream %AUTO% -tx [lindex $options(-stas) 0] -rx [lindex $options(-stas) 1] -traffictype $options(-encoding) -reportinterval 0.25]
	set call_leg(rx) [UTF::stream %AUTO% -tx [lindex $options(-stas) 1] -rx [lindex $options(-stas) 0] -traffictype $options(-encoding) -reportinterval 0.25]
	# Streams won't invoke trigger callbacks unless they're armed
	if {$options(-events)} {
	    $call_leg(tx) configure -trigger_armed 1
	    $call_leg(rx) configure -trigger_armed 1
	}
	set state "INSTANTIATED"
    }
    method __checkstream {txrx stream} {
	# UTF::Message DEBUG  $utf_msgtag $args
	if {$txrx eq "rx"} {
	    set lost [lindex [eval $stream stats -lost] end]
	    if {$lost >= $LOSTMAX} {
		set msg "call leg failed due to $lost lost packets"
		UTF::Message ERROR $utf_msgtag $msg
		set callerror 1
		error $msg
	    }
	    set jitter [lindex [eval $stream stats -jitter] end]
	    if {$jitter > $JITTERMAX} {
		set msg "call leg failed due to large jitter of $jitter ms"
		UTF::Message ERROR $utf_msgtag $msg
		set callerror 1
		error $msg
	    }
	}
    }
}
