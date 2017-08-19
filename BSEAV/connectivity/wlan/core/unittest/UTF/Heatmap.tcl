#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 7f05134f431ffeceb9c9e709460e486cef09df03 $
# $Copyright Broadcom Corporation$
#

package provide UTF::Heatmap 2.0

package require snit
package require UTF::doc

snit::type UTF::Heatmap {

    # config files.
    pragma -canreplace no

    option -file
    option -title
    option -palette {0 "black", 5 "blue", 100 "red", 500 "yellow", 900 "white"}
    option -pad 0
    option -yoff 1
    # dir: 0 left to right, 1 right to left
    option -dir -readonly 1 -type snit::boolean -default 0
    option -showmean -readonly 1 -type snit::boolean -default 0

    variable names {}
    variable ixlist {}
    variable max 0
    variable line -array  {}

    method addelement {LIST e} {
	upvar $LIST list
	if {![info exists list]} {
	    set list $e
	} elseif {$options(-dir)} {
	    set list [lreplace $list 0 -1 $e]
	} else {
	    lappend list $e
	}
    }

    method add {ix hist} {

	set file [open $options(-file) w]
	$self addelement ixlist $ix

	set i 0

	# Track max seen so far so that we keep the data in matrix form

	foreach {r p} $hist {
	    dict set names $i $r
	    if {![info exists line($i)]} {
		# initialize new row
		for {set x 1} {$x < [llength $ixlist]} {incr x} {
		    lappend line($i) $options(-pad)
		}
	    }
	    $self addelement line($i) $p
	    puts $file [join $line($i) ,]
	    incr i
	}
	if {$i > $max} {
	    set max $i
	}
	for {} {$i < $max} {incr i} {
	    $self addelement line($i) $options(-pad)
	    puts $file [join $line($i) ,]
	}
	close $file
    }

    method names {} {
	return $names
    }

    method plotscript {args} {
	UTF::Getopts {
	    {ytics.arg auto "ytics"}
	    {notitle "Hide title"}
	    {noxtics "Hide xtic labels"}
	}

	if {$max <= 1} {
	    # No data
	    return
	}

	if {$options(-showmean)} {
	    # compute means (do this late since we may have had to pad out
	    # the matrix earlier)
	    for {set x 0} {$x < [llength $ixlist]} {incr x} {
		set mean 0
		for {set i 0} {$i < $max} {incr i} {
		    set mean \
			[expr {$mean + ($i * [lindex $line($i) $x] / 100.0)}]
		}
		lappend means [expr {$mean+$options(-yoff)}]
	    }
	    set file [open "$options(-file)" a]
	    puts $file "\n\n"
	    puts $file [join $means "\n"]
	    close $file
	}

	set xtics ""
	set i 0
	set interval [expr {[llength $ixlist] / 10}]
	foreach a $ixlist {
	    if {$(noxtics)} {
		set label ""
	    } else {
		set label $a
	    }
	    if {$interval == 0 || $i % $interval == 0} {
		lappend xtics "'$label' $i"
	    }
	    incr i
	}
	lappend xtics "'$label' [expr {$i-1}]"

	set s "# Heatmap plotscript\n"
	append s "set datafile separator ','\n"
	append s "set palette defined ($options(-palette))\n"
	append s "unset y2tics\n"
	append s "set grid front lc 'grey'\n"
	if {$(notitle)} {
	    append s "unset title\n"
	} else {
	    append s "set title \"$options(-title)\"\n"
	}
	append s "set xtics ([join $xtics ,])\n"
	append s "set ytics $(ytics)\n"
	append s "plot '$options(-file)' index 0 matrix using 1:(\$2+$options(-yoff)):3 with image notitle"
	if {$options(-showmean)} {
	    append s ", '' index 1 using 0:1 with l lc 'white' lw 2 notitle"
	}
	append s "\n";
	append s "set ytics auto\n"

	return $s
    }
}
