#!/bin/env utf
#
# Rate limiter
# Author Robert McMahon June 2012
#
#
# $Id: c5bb8b50687cec51259603766a777b070e8d758a $
# $Copyright Broadcom Corporation$
#
package require snit
package require UTF

package provide UTF::RateLimiter 2.0

snit::type UTF::RateLimiter {
    option -threshold -type integer -default 1 -readonly true
    option -period -type integer -default 1 -readonly true
    option -fid

    variable prevmsg
    variable prevtime
    variable dupcount
    variable thresholdcntr
    variable prevcode
    variable prevwhere
    variable afterid
    variable timer
    variable fidstring

    constructor {args} {
	$self configurelist $args
	set timer [expr {$options(-period) * 1000}]
	if {$options(-fid) ne ""} {
	    set fidstring "($options(-fid))"
	} else {
	    set fidstring ""
	}
	set thresholdcntr [expr {$options(-threshold) - 1}]
	set dupcount 0
	set prevmsg {}
	set prevcode {}
	set prevwhere {}
    }
    destructor {
	catch {after cancel $afterid}
	$self flush
    }
    method message {msg {code ""} {where ""}} {
	set currtime [clock clicks -milliseconds]
	# subsequent line different or null msg
	# a null msg is used an event to kick out a "queued" dup msg
	if {[list $msg $code $where] ne [list $prevmsg $prevcode $prevwhere] || $msg eq {}} {
	    if {$dupcount > 0} {
		UTF::_Message ${prevcode}-RL $prevwhere "$prevmsg (filtered $dupcount duplicates) $fidstring"
	    }
	    set dupcount 0
	    set prevtime $currtime
	    set prevmsg $msg
	    set prevcode $code
	    set prevwhere $where
	    catch {after cancel $afterid}
	    if {$msg ne {}} {
		set thresholdcntr [expr {$options(-threshold) - 1}]
		UTF::_Message "$code   " $where $msg
		set afterid [after $timer [mymethod flush]]
	    }
	    return
	}
	# subsequent line a duplicate, do rate limiting
	# allow up to thresholdcntr dups before filtering
	incr dupcount
	if {!$thresholdcntr && [expr {$currtime - $prevtime > $timer}]} {
	    set prevtime $currtime
	    UTF::_Message ${code}-RL $where "$msg (filtered $dupcount duplicates) $fidstring"
	    set dupcount 0
	} elseif {$thresholdcntr} {
	    set prevtime $currtime
	    set dupcount 0
	    UTF::_Message "$code   " $where $msg
	    incr thresholdcntr -1
	}
	return
    }
    method flush {} {
	catch {$self message {} $prevcode $prevwhere}
	return
    }
}
