#!/bin/sh
# -*-tcl-*-
# The next line restarts using tcl on Linux \
test -x /usr/bin/tclsh && exec /usr/bin/tclsh "$0" ${1+"$@"}
# The next line restarts using tcl on Solaris \
exec /tools/tcl/8.4a4/SunOS/bin/tclsh "$0" ${1+"$@"}

# $Copyright Broadcom Corporation$
# $Id: 128a09b6534c4a94596a5e19ae622cd16de9d4d7 $

# This wrapper is a hack to get around the fact we don't have a
# consistent and up to date tcl installation on the Web server.
puts "Content-Type: text/html; charset=ISO-8859-1\n"
lappend ::env(TCLLIBPATH) .. /projects/hnd/tools/linux/share

catch {

    lappend auto_path /projects/hnd/tools/linux/share
    package require html

    if {$argv != ""} {
	set argv [string map {:: / / {}} $argv]
    }
    if {$argv eq "" || ![file executable ../$argv] ||
	[catch {
	    exec [info nameofexecutable] ../$argv -man -format html 2>@stdout
	} ret]} {
	if {![info exists ret]} {
	    set ret ""
	} else {
	    set ret "<pre>$ret</pre>"
	}

	foreach f [lsort [split \
			      [eval exec grep titledesc \
				   [glob -type x {../{UTF/{{*/,}*/,},}*}]] \
			      "\n"]] {
	    if {![regexp {~:} $f] && [regexp {(.*):.*\{(.*)\}} $f - f d]} {
		regsub {^../} $f {} f
		regsub -all {/} $f "::" f
		regsub -all { } $f "%20" f
		lappend tools $f $d
	    }
	}

	puts [subst {
	    <!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 3.2//EN">
	    [html::head "Unit Test Framework Tools"]
	    [html::bodyTag]
	    [html::h1 "Unit Test Framework Tools"]
	    $ret
	    [html::openTag dl]
	    [html::foreach {f d} $tools {
		[html::openTag dt]
		[html::openTag a "href=\"APdoc.cgi?$f\""]
		$f
		[html::closeTag]
		[html::closeTag]
		[html::openTag dd]
		$d
		[html::closeTag]
	    }]
	    [html::closeTag]
	    [html::closeTag]
	    [html::end]
	}]
    } else {
	puts $ret
    }
} ret
puts $ret
