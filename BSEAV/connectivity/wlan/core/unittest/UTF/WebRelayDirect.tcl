#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::WebRelayDirect 2.0

#package requrie snit 1.0
package require UTF
package require http

snit::type UTF::WebRelayDirect {
    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip -default {} -readonly true
    option -relaynum -type integer -default 1 -readonly true
    option -holdtime -type integer -default 1
    option -longholdtime -type integer -default 5
    option -blocking -type boolean -default 0

    variable state -array {}
    variable utfmsg_tag {}

    constructor {args} {
	$self configurelist $args
	set state($options(-relaynum)) "OFF"
	set state(powercycle) "OFF"
	set utmsg_tag [namespace tail $self]
    }
    method press {args} {
	UTF::Getopts {
	    {blocking ""}
	    {hold ""}
	    {time.arg "" "time to hold button"}
	}
	if {[$self is_pressed] || $state(powercycle) eq "ON"} {
	    $self __block
	}
	if {$(hold)} {
	    set pulsetime $options(-longholdtime)
	} elseif {$(time) ne {}} {
	    set pulsetime $(time)
	} else {
	    set pulsetime $options(-holdtime)
	}
	set urlstring "http://${options(-lan_ip)}/state.xml?relay${options(-relaynum)}State=2&pulseTime${options(-relaynum)}=$pulsetime"
	UTF::Message INFO $utfmsg_tag $urlstring
	if {[catch {::http::geturl $urlstring} token]} {
	    error $token
	}
	set state($options(-relaynum)) "PULSE"
	after [expr {1000 * $pulsetime}] [list set [myvar state($options(-relaynum))] "OFF"]
	set xmldata [::http::data $token]
	::http::cleanup $token
	if {$options(-blocking) || $(blocking)} {
	    $self __block
	}
    }
    method __block {} {
	while {$state($options(-relaynum)) ne "OFF"} {
	    vwait [myvar state]
	}
    }
    method is_pressed {} {
	if {$state($options(-relaynum)) ne "OFF"} {
	    return 1
	} else {
	    return 0
	}
    }
    method {power cycle} {} {
	if {[catch {$self press -hold} err]} {
	    UTF::Message ERROR $utfmsg_tag $err
	    error $err
	}
	set state(powercycle) "ON"
	after [expr {1000 * $options(-longholdtime)}] "[list set [myvar state(powercycle)] OFF]; $self press"
	return
    }
}
