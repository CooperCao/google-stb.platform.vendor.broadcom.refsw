#!/bin/env utf
#
# $Copyright Broadcom Corporation$
#
package require snit
package require TclReadLine
package require UTF

package provide TclReadLine::Journal 1.1

snit::type TclReadLine::Journal {
    option -filename -default "utfjournal.cap"
    variable state "off"
    variable prevtime ""
    variable fid
    method status { } {
	TclReadLine::print "Journaling state=$state  file=$options(-filename)"
    }
    method append { } {
       $self __openjournal "a"

    }
    method on { } {
       $self __openjournal "w"
    }
    method off { } {
	if {$state == "capturing"} {
	    TclReadLine::delCmdCallback "[mymethod logentry]"
	    close $fid
	    set state "off"
	    TclReadLine::print "journaling off"
	    set prevtime ""
	}
    }
    method replay {{timedelay 0}} {
	if {![catch {open $options(-filename) r} tmp]} {
	    set adj 0
	    while {![eof $tmp]} {
		set cmds [gets $tmp]
		set waittime [lindex $cmds 0]
		set script [lrange $cmds 1 end]
		if {$script == ""} {
		    continue
		}
		if {$timedelay} {
		    set wt [$waitime - $adj]
		    if {$wt > 0} {
			set wt [expr $wt / 1000.0]
			UTF::Sleep $wt
		    }
		}
		set starttime [clock clicks -milliseconds]
		TclReadLine::print "CMD: $script\n"
		catch {uplevel #0 eval $script} output
		TclReadLine::print "$output"
		set finishtime [clock clicks -milliseconds]
		set adj [expr $finishtime - $starttime]
		puts -nonewline "\n"
	    }
	    close $tmp
	} else {
	    error "$tmp"
	}
    }
    method show { } {
	if {![catch {open $options(-filename) r} tmp]} {
	    while {![eof $tmp]} {
		set buf [gets $tmp]
		TclReadLine::print "$buf\n"
	    }
	    close $tmp
	} else {
	    error "$tmp"
	}
    }
    method logentry {args} {
	# Don't log journal commands
	if {[lindex $args 0] == [namespace tail $self]} {
	    return
	}
	set currtime [clock clicks -milliseconds]
	if {$prevtime != ""} {
	    set timedelta [expr $currtime - $prevtime]
	} else {
	    set timedelta 0
	}
	set prevtime $currtime
	puts $fid "${timedelta} $args"
	flush $fid
    }
    method __openjournal {openoption} {
	if {$state == "off"} {
	    TclReadLine::addCmdCallback "[mymethod logentry]"
	    if {[catch {open $options(-filename) $openoption} fid]} {
		error "opening journal file $options(-filename) : $fid"
	    } else {
		TclReadLine::print "Journaling to file $options(-filename)"
		set state "capturing"
	    }
	} else {
	    TclReadLine::print "Journaling already enabled."
	}
	return
    }
}

# Create the default journal if it doesn't already exist
if {[::TclReadLine::Journal info instances ::journal] == ""} {
    TclReadLine::Journal journal
}
