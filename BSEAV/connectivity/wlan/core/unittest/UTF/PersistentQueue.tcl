#
# UTF Persistent Queues
#
# Queues that persist
#
# Written by: Robert J. McMahon June 2014
#
# $Id: 8459c4e6f3c6de2c7459fda2f2d2cec2bf264c32 $
# $Copyright Broadcom Corporation$
package require UTF
package require snit
package require md5

package provide UTF::PersistentQueue 2.0

snit::type UTF::PersistentQueue {
    typevariable EXCEPTATTRIBUTES ""
    typevariable DEFAULTDIRECTORY "/var/tmp/utf"
    option -name -default {} -readonly true
    option -directory -readonly true -default {}
    option -duplicatecheck -type boolean -default false
    option -priority -default {} -readonly true
    option -maxdepth -default 0 -type integer -readonly true
    variable filename
    constructor {args} {
	$self configurelist $args
	if {$options(-name) eq {}} {
	    set options(-name) [namespace tail $self]
	}
	if {$options(-directory) eq {}} {
	    set options(-directory) $DEFAULTDIRECTORY
	}
	if {![file exists $options(-directory)]} {
	    if {[catch {file mkdir $options(-directory)} res]} {
		UTF::Message ERROR $options(-name) "unable to make directory $options(-directory)"
		error $res
	    }
	}
	set filename [file join $options(-directory) "$options(-name).pqueue"]
	if {![file exists $filename]} {
	    if {[catch {open $filename w} fid]} {
		error $fid
	    }
	    puts -nonewline $fid {}
	    close $fid
	}
    }
    destructor {
	$self delete
    }
    method inspect {} {
	puts "\ncount=[$self count] values=[$self list]"
    }
    method id {args} {
	set key $args
	foreach n [lsort [array names options]] {
	    if {[lsearch -exact $EXCEPTATTRIBUTES $n] == -1} {
		lappend key $n $options($n)
	    }
	}
	regsub -all {[/<>]} $key "." key
	set myid [::md5::md5 -hex $key]
	return $myid
    }
    method mypriority {} {
	return $options(-priority)
    }
    method count {} {
	return [llength [$self list]]
    }
    method list {} {
	if {[catch {open $filename r} fid]} {
	    error $fid
	}
	set values [read $fid]
	close $fid
	return $values
    }
    method enqueue {args} {
	UTF::GetKnownopts {
	    {head "enqueue at head of line"}
	    {allowdups "override duplicate checking"}
	}
	if {$args eq ""} {
	    return
	}
	if {![catch {open $filename r} fid]} {
	    set update [read $fid]
	    close $fid
	} else {
	    set update {}
	}
	if {$options(-duplicatecheck) && !$(allowdups) && [lsearch -exact $update $args] != -1} {
	    UTF::_Message INFO $options(-name) "Duplicate enqueue for $args ignored"
	    return
	}
	if {!$(head)} {
	    lappend update $args
	} else {
	    set update [linsert $update 0 $args]
	}
	if {$options(-maxdepth) > 0 && [llength $update] > $options(-maxdepth)} {
	    UTF::_Message ERROR $options(-name) "queue overflow"
	    if {!$(head)} {
		set update [lrange $update 1 end]
	    } else {
		set update [lrange $update 0 end-1]
	    }
	}
	if {[catch {open $filename w} fid]} {
	    error $fid
	}
	puts -nonewline $fid $update
	close $fid
    }
    method dequeue {} {
	if {[catch {open $filename r} fid]} {
	    error $fid
	}
	set values [read $fid]
	close $fid
	if {[llength $values]} {
	    set value [lindex $values 0]
	    set update [lrange $values 1 end]
	    if {[catch {open $filename w} fid]} {
		error $fid
	    }
	    puts -nonewline $fid $update
	    close $fid
	} else {
	    set value {}
	}
	return $value
    }
    method push {args} {
	$self enqueue $args
    }
    method pop {} {
	if {[catch {open $filename r} fid]} {
	    error $fid
	}
	set values [read $fid]
	close $fid
	if {[llength $values]} {
	    set value [lindex $values end]
	    set update [lrange $values 0 end-1]
	    if {[catch {open $filename w} fid]} {
		error $fid
	    }
	    puts -nonewline $fid $update
	    close $fid
	} else {
	    set value {}
	}
	return $value
    }
    method peek {{ix 1}} {
	if {[catch {open $filename r} fid]} {
	    error $fid
	}
	set values [read $fid]
	close $fid
	set value [lindex $values [expr {$ix - 1}]]
	return $value
    }
    method rm {args} {
	set update [lsearch -all -not -inline [$self list] $args]
	if {[catch {open $filename w} fid]} {
	    error $fid
	}
	puts -nonewline $fid $update
	close $fid
    }
    method keep {{count 0}} {
	if {$count > 0} {
	    if {[catch {open $filename r} fid]} {
		error $fid
	    }
	    set update [read $fid]
	    close $fid
	    set update [lrange $update 0 [expr {$count - 1}]]
	    if {[catch {open $filename w} fid]} {
		error $fid
	    }
	    puts -nonewline $fid $update
	    close $fid
	} else {
	    $self clear
	}
    }
    method clear {} {
	if {[catch {open $filename w} fid]} {
	    error $fid
	}
	puts -nonewline $fid {}
	close $fid
    }
    method delete {} {
	if {[catch {rm $filename} err]} {
	    UTF::Message WARN $options(-name) "$err"
	}
    }
}