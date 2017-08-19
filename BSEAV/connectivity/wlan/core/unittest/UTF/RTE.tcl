#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 299b24dee612a45ea81232d2d320936e5518d025 $
# $Copyright Broadcom Corporation$
#

package provide UTF::RTE 2.0

package require UTF
package require snit
package require UTF::doc

UTF::doc {
    # [manpage_begin UTF::DHD n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF RTE support}]
    # [copyright {2013 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::RTE provides common RTE support for ARM dongles.

    # [list_begin definitions]

}

namespace eval UTF::RTE {

    variable inited 0

    proc init {} {
	variable inited
	set gdb \
	    "$::UTF::projtools/linux/hndtools-armeabi-2011.09/bin/arm-none-eabi-gdb"
	if {!$inited} {
	    localhost rexec -x -s -t 180 $gdb -batch -ex quit
	    set inited 1
	}
    }


    UTF::doc {
	# [call [cmd UTF::RTE::findassert] [arg name] [arg file] \
	#	  [arg line] [arg hndrte_src]]

	# Report assert text from dongle assert messages.  This is
	# used for dongles which contain partial assert text, ie those
	# without BCMDBG_ASSERT_TRAP defined.
    }

    proc findassert {name file line hndrte_src} {
	#UTF::Message DBG "" [list UTF::RTE::findassert $name $file $line $hndrte_src]
	set ret {}
	if {![string match {*.?} $file]} {
	    append file ".c"
	}
	# Strip path, used by some dongle builds.
	regsub {.*/} $file {} file
	foreach file [exec find $hndrte_src ( ( -name builds -prune ) -o -type f ) -name $file] {
	    set f [open $file]
	    for {set i 0} {$i < $line && ![eof $f]} {incr i} {
		gets $f data
	    }
	    close $f
	    # Clean up file
	    set file [string map [list $hndrte_src "src"] $file]
	    # Clean up assert text
	    regsub {^\s+} $data {} data
	    regsub {\s*;\s*$} $data {} data
	    lappend ret "$data in file \"$file\" at line $line"
	}
	return [join $ret "\n"]
    }

    UTF::doc {
	# [call [cmd UTF::RTE::findtrap] [arg name] [arg traptype] \
	#	  [arg {stack list}] [arg hndrte_exe]]

	# Report stack trace from dongle trap messages.  These may
	# include ASSERTS if BCMDBG_ASSERT_TRAP is defined.
    }

    proc findtrap {name trap stack hndrte_exe} {
	set gdb \
	    "$::UTF::projtools/linux/hndtools-armeabi-2011.09/bin/arm-none-eabi-gdb"
	#UTF::Message DBG "" [list UTF::RTE::findtrap $name $trap $stack $hndrte_exe]
	set inram ""
	set inrom ""

	set exedir [file normalize [file dirname $hndrte_exe]]
	set trapcmd "cd $exedir\n"
	set roml_exe [file join $exedir roml.exe]

	set rtecdc [file tail [file rootname $hndrte_exe]]

	set start 0
	set end 0xffffffff
	if {[file exists $roml_exe]} {
	    if {[catch {open [file join $exedir ${rtecdc}.map]} fd]} {
		UTF::Message WARN $name $fd
	    } else {
		set map [read $fd]
		close $fd
		regexp -line {^([[:xdigit:]]+)\sT\stext_start$} $map - start
		set start "0x$start"
		regexp -line {^([[:xdigit:]]+)\sA\s_end$} $map - end
		set end "0x$end"
	    }
	    if {$start == 0} {
		UTF::Message INFO $name "RAM/ROM split $end"
	    } else {
		UTF::Message INFO $name "ROM/RAM split $start"
	    }
	}

	# CM3
	array set types \
	    {1 RST 2 NMI 3 FAULT 4 MM 5 BUS 6 USAGE
		b ASSERT c DMON e PENDSV f SYSTICK 10 ISR}
	if {[catch {set trap "Trap $types($trap)"}]} {
	    set trap "Trap $trap"
	}

	set symbols ""
	set depth 0
	set last -1
	foreach a $stack {
	    if {$a == 0} {
	 	UTF::Message INFO $name "$a is a NULL pointer"
		continue
	    } elseif {$a == $last} {
	 	UTF::Message INFO $name "$a is repeated"
		continue
	    } elseif {$start == 0 ? $a > $end : $a < $start} {
	 	if {$symbols ne "ROM"} {
		    append trapcmd "file roml.exe\n"
		    set symbols ROM
		}
	    } else {
	 	if {$symbols ne "RAM"} {
		    append trapcmd "file ${rtecdc}.exe\n"
		    set symbols RAM
		}
	    }
	    set last $a
	    if {$symbols eq "RAM" && $depth < 2} {
		# First two RAM symbols show source
		append trapcmd "l *$a\n"
		incr depth
	    } else {
		# Rest of stack just gets info
		append trapcmd "i li *$a\n"
	    }
	}
	append trapcmd "quit"

	set f [open "/tmp/trap.cmd" w]
	catch {file attributes "/tmp/trap.cmd" -permissions 00666}
	puts $f $trapcmd
	UTF::Message LOG $name $trapcmd
	close $f
	set cmd "$gdb -q -n"
	set code [catch {UTF::BuildFile::gdb $cmd "/tmp/trap.cmd"} ret]
	regsub -line -all {/projects/.*/\.\./} $ret {} ret
	regsub -line -all {Reading symbols from .*/} $ret {Reading symbols from } ret
	regsub -all {Load new symbol table [^\n]+\n} $ret {} ret
	UTF::Message LOG $name "$ret"
	if {![regexp -line {is (in \S*) \((.*):(\d+)\)\.$} $ret - func file line]} {
	    set func "at"
	    if {![regexp -line {is at (.*):(\d+)\.$} $ret - file line]} {
		set file "unknown"
		set line "unknown"
	    }
	}
	set file [file tail $file]
	if {[regexp -line "^${line}\\s+(.*ASSERT\\(.*)" $ret - ass]} {
	    regsub {[;\s]*$} $ass {} ass
	    return "$ass $func ($file:$line)"
	} else {
	    return "$trap $func ($file:$line)"
	}
    }

    proc parse_traplog_file {who file} {
	set fd [open $file]
	set txt [read $fd]
	close $fd
	regsub -line -all {^.*>\s+} $txt {} txt
	parse_traplog $who $txt
    }

    proc parse_traplog {who traplog} {
	set name [$who cget -name]
	set msg ""
	set offset 0
	if {[regexp -line {ASSERT in file (.*) line (\d+) \(ra ([[:xdigit:]]+), fa ([[:xdigit:]]+)\)} $traplog - file line ra fa]} {
	    if {[catch {$who findassert $file $line} ret]} {
		UTF::Message WARN "$name>" $ret
	    } else {
		UTF::Message FAIL "$name>" $ret
		set msg $ret
	    }
	    set tt "b"
	    # Offset return address to find caller.
	    set stack [format "0x%x" [expr {"0x$ra" -3}]]
	    set offset 3; # skip hndrte_assert
	} elseif {[regexp -line {(?:Dongle t|T)rap type 0x([[:xdigit:]]).*,\s*lp 0x([[:xdigit:]]+),\s+rpc 0x([[:xdigit:]]+)} $traplog - tt lr pc] ||
		  [regexp -line {TRAP type 0x([[:xdigit:]]) @ epc 0x([[:xdigit:]]+), .*, lp 0x([[:xdigit:]]+)} $traplog - tt pc lr] ||
		  [regexp -line {(?:TRAP ([[:xdigit:]]+)\(([[:xdigit:]]+))\): pc ([[:xdigit:]]+), lr ([[:xdigit:]]+)} $traplog - tt fp pc lr]} {

	    # ARM backtrace addresses appear to be always off by 3 due to
	    # pipelining.
	    set stack [format "0x%x" [expr {"0x$pc" -3}]]
	    if {$pc != $lr && "0x$pc" != "0x$lr"+1} {
		lappend stack 0x$lr
	    }
	} else {
	    UTF::Message WARN "$name>" "No trap detected in traplog"
	    set tt 4
	}
	foreach {- p} [regexp -inline -line -all \
			   {^sp\+[[:xdigit:]]+\s+0*([[:xdigit:]]+)$} $traplog] {
	    if {$offset > 0} {
		incr offset -1
	    } else {
		# ARM backtrace addresses appear to be always off by 3 due to
		# pipelining.
		lappend stack [format "0x%x" [expr {"0x$p" -3}]]
	    }
	}

	if {[info exists stack]} {
	    if {[catch {$who findtrap $tt $stack} ret]} {
		UTF::Message WARN "$name>" $ret
	    } elseif {$ret ne ""} {
		UTF::Message FAIL "$name>" $ret
		set msg $ret
	    }
	}
	set msg
    }

    proc unwind_circular_buffer {msg} {
	# Join up lines broken just after timestamp
	regsub -all {(\d{6}\.\d{3} )\n} $msg {\1} msg

	# Discard lines with no timestamp (they aren't log messages)
	# Log messages are in a circular buffer, so cut and swap at
	# the point where timestamps wraparound.
	set log1 ""
	set log2 ""
	set max ""
	set top 1
	foreach l [split $msg \n] {
	    if {[regexp {^(\d{6}\.\d{3}) } $l - s]} {
		if {$top && ($max eq "" || $s >= $max)} {
		    set max $s
		    lappend log1 $l
		} else {
		    set top 0
		    lappend log2 $l
		}
	    }
	}
	join [concat $log2 $log1] \n
    }

}

UTF::doc {
    # [list_end]

}

UTF::doc {
    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also wl]
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [see_also [uri APdoc.cgi?UTF::Base.tcl UTF::Base]]
    # [see_also [uri APdoc.cgi?UTF::Cygwin.tcl UTF::Cygwin]]
    # [see_also [uri APdoc.cgi?UTF::Router.tcl UTF::Router]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
