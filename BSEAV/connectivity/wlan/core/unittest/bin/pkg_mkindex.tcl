#!/bin/env utf
# -*-tcl-*-
#
# $Id: 6069518333d7421a0686e518f6e9a620c593eb42 $
# $Copyright Broadcom Corporation$

# [call [cmd pkg_mkindex.tcl] [arg args]]

# Generate UTF [file pkgIndex.tcl] file.  This is a wrapper around
# the built-in [cmd ::pkg_mkindex], but adds better error
# checking.

# If [arg args] are not specified, defaults to the regular UTF
# files.

# Needs to be standalone to avoid dependencies when bootstrapping
# pkgIndex

if {$argv eq {}} {
    set argv {. *.tcl UTF/*.tcl UTF/*/*.tcl Test/*.test Test/*/*.test}
}

set ::pkg_mkindex_failed ""
set ::pkgs 0
proc ::tclLog {msg} {
    upvar file file
    switch -re -matchvar m $msg {
	{^packages provided were (.*)} {
	    foreach pv [lindex $m 1] {
		set p [lindex $pv 0]
		if {[info exists ::pkg_dup($p)]} {
		    set msg \
			"warning: \"$file\" duplicates package $p from $::pkg_dup($p)"
		    puts stderr $msg
		    lappend ::pkg_mkindex_failed $msg
		} else {
		    lappend ::pkg_dup($p) $file
		}
	    }
	}
	{^successful sourcing} -
	{^commands} {}
	{^warning:} {
	    puts stderr $msg
	    lappend ::pkg_mkindex_failed $msg
	}
	{^processed} {
	    incr ::pkgs
	}
	default {
	    puts $msg
	}
    }
}
::pkg_mkIndex -verbose {*}$argv
puts "$::pkgs packages processed"
if {$::pkg_mkindex_failed ne ""} {
    #puts stderr [join $::pkg_mkindex_failed "\n"]
    #file delete pkgIndex.tcl
    exit 1
}
if {[file isdirectory .svn]} {
    if {[set diff [exec svn diff pkgIndex.tcl]] ne ""} {
	puts "$diff\nCheck in? \[n] "
	if {[gets stdin] eq "y"} {
	    puts [exec svn ci -m "versions" pkgIndex.tcl]
	}
    }
} else {
    if {[set diff [exec git diff pkgIndex.tcl]] ne ""} {
	puts "$diff\nCheck in? \[n] "
	if {[gets stdin] eq "y"} {
	    puts [exec echo git commit -m "versions" pkgIndex.tcl]
	}
    }
}
exit 0
