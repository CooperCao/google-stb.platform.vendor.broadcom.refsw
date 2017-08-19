#!/bin/env utf
#
# Video Channel in UTF
# Author Robert McMahon March 2011
#
# $Id: 516e2c91370f3185e5f4c93c0a54f174a4e3ca48 $
# $Copyright Broadcom Corporation$
#
package require snit
package require UTF
package require UTF::Streams

package provide UTF::VideoChannel 2.0

snit::type UTF::VideoChannel {
    typevariable LOSTMAX 2   ;# units of pkts
    typevariable JITTERMAX 1 ;# units of ms

    option -onfailure -default "tclerror"
    option -stas
    option -tx
    option -encoding -default HDVIDEO
    option -silent -type boolean -default 1
    option -events -type boolean -default 1
    option -multicast -type boolean -default 1
    option -igmpversion -default "v2"

    variable stream
    variable state
    variable utf_msgtag
    variable everyid
    variable channelerror

    typemethod allvideochannels {args} {
	set instances [$type info instances]
	set faillist {}
	set cmd [string tolower [lindex $args 0]]
	foreach instance $instances {
	    if {[catch {eval $instance $cmd [lrange $args 1 end]} err]} {
		lappend faillist "[namespace tail $instance]:$err"
	    }
	}
	if {[llength $faillist]} {
	    UTF::Message ERROR VideoChannels $faillist
	    error $faillist
	}
    }

    constructor {args} {
	set state ""
	set channelerror 0
	$self configurelist $args
	set utf_msgtag "[namespace tail $self]"
    }
    destructor {
	$self stop
	$stream  destroy
    }
    method inspect {} {
	puts "\nStream is:"
	putst "\t$stream"
    }
    method start {} {
	if {$state eq "RUNNING"} {
	    return
	}
	if {$state eq ""} {
	    $self __instantiate_stream
	}
	if {! [catch {$stream start}]} {
	    set state "RUNNING"
	}
    }
    method stop {} {
	if {$state ne "RUNNING"} {
	    return
	}
	if {$everyid ne ""} {
	    UTF::Every cancel $everyid
	}
	if {[catch {$stream stop}] err} {
	    set state "ERROR"
	    set msg $err
	    UTF::Message ERROR $utf_msgtag $err
	    error $msg
	} else {
	    set state "STOPPED"
	}
    }
    method reset {} {
	if {$options(-events)} {
	    set channelerror 0
	    $stream stats -clear
	    $stream configure -trigger_armed 1
	}
    }
    method clear {} {
	$stream stats -clear
    }
    method check {} {
	set lost [$stream stats -lost]
	set jitter [$stream stats -jitter]
	if {!$options(-silent)} {
	    UTF::Message INFO $utf_msgtag "lost: $lost"
	    UTF::Message INFO $utf_msgtag "jitter: $jitter"
	}
	if {$channelerror} {
	    # Reset error flag for next call to check
	    set channelerror 0
	    set msg "check indicates failed"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
    }
    method __instantiate_stream  {} {
	if {$options(-tx) eq ""} {
	    set msg "Video channel tx not configured"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	if {$options(-stas) eq ""} {
	    set msg "Video channel stas not configured"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	set triggercallback [mymethod __checkstream]
	set stream [UTF::stream %AUTO% -transmitsta $options(-tx) -receivesta $options(-stas)  -traffictype $options(-encoding) -trigger_callback $triggercallback -multicast $options(-multicast)]
	if {$options(-events)} {
	    $stream configure -trigger_armed 1
	}
	set state "INSTANTIATED"
    }
    method __checkstream {args} {
	# UTF::Message DEBUG  $utf_msgtag $args
	set lost [lindex [eval [concat [lindex $args 0] stats -lost]] end]
	if {$lost >= $LOSTMAX} {
	    set msg "channel failed due to $lost lost packets"
	    UTF::Message ERROR $utf_msgtag $msg
	    set channelerror 1
	    error $msg
	}
	set jitter [lindex [eval [concat [lindex $args 0] stats -jitter]] end]
	if {$jitter > $JITTERMAX} {
	    set msg "channel failed due to large jitter of $jitter ms"
	    UTF::Message ERROR $utf_msgtag $msg
	    set channelerror 1
	    error $msg
	}
    }
}
