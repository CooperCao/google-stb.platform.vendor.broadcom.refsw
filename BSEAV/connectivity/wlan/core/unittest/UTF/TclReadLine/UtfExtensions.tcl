package require UTF

package provide TclReadLine::UTF 2.0

namespace eval TclReadLine::UTF {
    variable color
    set color(procnorm) green
    set color(procbold) green
    set color(methods) Green
    set color(ns) blue
    set color(nsbold) Blue
    set color(inst) blue
    set color(objns) cyan
    set color(vars) cyan
    set color(procbody) yellow
    set color(options) cyan
}

#
# Set colors here, need to standardize on a file to read these
#

proc inspect {name args} {
    TclReadLine::UTF::inspect $name
    if {[TclReadLine::UTF::is_objectinst $name] && [$name info methods inspect] == "inspect" } {
	if {[llength $args]} {
	    $name inspect $args
	} else {
	    $name inspect
	}
    }
}

proc TclReadLine::UTF::inspect {name {paging 1} {showdelegation 1}} {
    set ns [TclReadLine::UTF::get_snit_inst $name]
    # Check if this is a snit instances
    if {$ns != ""} {
	set rowcount 2
	puts "UTF object $name"
	incr rowcount +2
	if {![catch {$name info type} type]} {
	    puts "  Type is $type"

	    # set tmp [namespace eval $type {array get options}]
	    # array set typeoptions $tmp
	} else {
	    puts "  Strange: UTF type is UNKNOWN!!"
	}
	puts "  Uses namespace $ns"
	incr rowcount +1
	if {$showdelegation} {
	    set delegationpath [TclReadLine::UTF::delegation_lookup $name]
	    if {[llength $delegationpath]} {
		puts "Current delegation path is:"
		incr rowcount +1
		foreach component $delegationpath {
		    foreach {type compCmd compns} $component {}
		    puts "  $compCmd is type $type and uses namespace $compns"
		    incr rowcount +1
		}
	    }
	}
	if {$paging} {
	    set currblock [fconfigure stdin -blocking]
	    set currbuff [fconfigure stdin -buffering]
	    fconfigure stdin -buffering none -blocking 1
	    exec /bin/stty raw -echo
	    set rows [TclReadLine::getRows]
	}

 	set lookuplist "$name"
	if {$showdelegation} {
	    foreach component $delegationpath {
		lappend lookuplist [lindex $component 1]
	    }
	}

	#
	#  RJM:  Need to lookup the except methods as well
	#  from ${type}::$Snit_info(exceptmethods)
	#
	foreach item $lookuplist {
	    puts -nonewline "\n$item instance methods available are:\n  "
	    set charcount 2
	    incr rowcount +2
	    set cols [TclReadLine::getColumns]
	    foreach objmethod [TclReadLine::UTF::getpublicinstmethods $item] {
		set tmp [string length $objmethod]
		set charcount [expr $tmp + $charcount + 1]
		if {$charcount > $cols} {
		    puts -nonewline "\n  "
		    incr rowcount
		    set charcount [expr $tmp + 3]
		    if {$paging && ($rowcount > $rows)} {
			puts -nonewline "--More--"
			flush stdout
			while {1} {
			    TclReadLine::rawInput
			    set kb [read stdin 1]
			    if {[string length $kb]} {
				TclReadLine::lineInput
				break
			    } else {
				update
			    }
			}
			if {$kb == "q" || $kb == "Q"} {
			    break
			}
			puts -nonewline "\r"
			set rowcount 2
		    }
		}
		puts -nonewline "$objmethod "
	    }
	}
	puts ""
	puts "\nCurrent option settings are:"
	incr rowcount +3
	set optionlist [lsort [$name info options]]
	foreach opt $optionlist {
	    set value [$name cget $opt]
	    if {$paging} {
		set numlines [llength [split $value "\n"]]
		if {$numlines} {
		    incr rowcount +$numlines
		} else {
		    incr rowcount +1
		    set numlines 1
		}
		if {$paging && ($rowcount > $rows)} {
		    puts -nonewline "--More--"
		    flush stdout
		    while {1} {
			TclReadLine::rawInput
			set kb [read stdin 1]
			if {[string length $kb]} {
			    TclReadLine::lineInput
			    break
			} else {
			    update
			}
		    }
		    if {$kb == "q" || $kb == "Q"} {
			break
		    }
		    puts -nonewline "\r"
		    set rowcount [expr $numlines + 2]
		}
	    }
	    puts [format "%-30s %s" "  options\($opt\)" "= $value"]
	}
	if {$paging} {
	    fconfigure stdin -buffering $currbuff -blocking $currblock
	}
    } else {
	set tmp [info proc $name]
	if {$tmp != $name  && $tmp != "\:\:$name"} {
	    puts "$name isn't a procedure in this namespace=[namespace current] , level=[info level]"
	    return
	}
	puts "proc $name [info args $name]"
	set bodyoutput [info body $name]
	puts $bodyoutput
	set vars [info vars $name]
	if {$vars != ""} {
	    puts "vars: [info vars $name]"
	}
	#
	# Search for internal procedures
	#
	set buf "private procs:\n"
	foreach line [split $bodyoutput "\n"] {
	    if {[regexp {[ \t]*proc[ \t]+([^ \t]+)} $line - match]} {
		append buf "\t$match \n"
	    }
	}
	puts $buf
    }
}


#
# Based from:  snit::RT.CacheMethodCommand {type selfns win self method}
#
proc TclReadLine::UTF::delegation_lookup {name} {
    if {![TclReadLine::UTF::is_objectinst $name]} {
	return
    }
    set results ""
    while {1} {
	set type [$name info type]
	set selfns [TclReadLine::UTF::get_snit_inst $name]
	variable ${type}::Snit_methodInfo
	variable ${type}::Snit_typecomponents
	variable ${selfns}::Snit_components
	if {[catch {foreach {flag pattern compName} $Snit_methodInfo(*) {}}]} {
	    break
	}
	if {$flag == 1} {
	    break
	}
	if {"" != $compName} {
	    if {[info exists Snit_components($compName)]} {
		set compCmd $Snit_components($compName)
	    } elseif {[info exists Snit_typecomponents($compName)]} {
		set compCmd $Snit_typecomponents($compName)
	    } else {
		error "$type $self delegates method \"$method\" to undefined component \"$compName\""
	    }
	    #        lappend subList %c [list $compCmd]
	    lappend results [list [$compCmd info type] $compCmd [TclReadLine::UTF::get_snit_inst $compCmd]]
	    set name $compCmd
	} else {
	    break
	}
    }
    return $results
}

proc TclReadLine::UTF::getpublicinstmethods {name} {
    if {![TclReadLine::UTF::is_objectinst $name]} {
	return ""
    }
    set publicmethods ""
    set allmethods [lsort [$name info methods]]
    foreach meth $allmethods {
	if {[string equal [string range $meth 0 1] "__"]} {
	    continue
	}
	if {[llength $meth] > 1} {
	    set meth \{$meth\}
	}
	lappend publicmethods $meth
    }
    return $publicmethods
}

proc TclReadLine::UTF::getpublicmethods {name} {
    if {![TclReadLine::UTF::is_objectinst $name]} {
	return ""
    }
    set allmethods [TclReadLine::UTF::getpublicinstmethods $name]
    set delegations [TclReadLine::UTF::delegation_lookup $name]
    foreach delegation $delegations {
	set allmethods [concat $allmethods [TclReadLine::UTF::getpublicinstmethods [lindex $delegation 1]]]
    }
    set allmethods [lsort -unique $allmethods]
    return $allmethods
}

proc TclReadLine::UTF::parse_firstword {word offset} {
    upvar $word firstword
    upvar $offset firstwordix

    set firstwordix [string first " " $TclReadLine::CMDLINE]
    if {$firstwordix > 0} {
	set firstword [string range $TclReadLine::CMDLINE 0 $firstwordix]
	set fwcomplete 1
    } else {
	set firstword [string range $TclReadLine::CMDLINE 0 end]
	set fwcomplete 0
    }
    set firstword [string trim $firstword " "]
    return $fwcomplete
}

proc TclReadLine::UTF::parse_cmdline {words cmd2complete} {
    upvar $words wordarray
    upvar $cmd2complete last

    set ix 1
    set tmp [string map {\" \0 \{ \1 \[ \2}  $TclReadLine::CMDLINE]
    foreach word $tmp {
	set wordarray($ix) [string map {\0 \" \1 \{ \2 \[} $word]
	incr ix
    }
    if {[string index $TclReadLine::CMDLINE end] != " "} {
	set last [string map {\0 \" \1 \{ \2 \[} [lindex $tmp end]]
    } else {
	set last ""
    }
    return [llength $tmp]
}

#
#  Check for snit instance, look only 10 lines into the body
#  self [set ::UTF::Linux::Snit_inst3::Snit_instance]
#
proc TclReadLine::UTF::is_objectinst {name} {
    if {[get_snit_inst $name] != ""} {
	return 1
    } else {
	return 0
    }
}

proc TclReadLine::UTF::get_snit_inst {name} {
    if {[catch {uplevel #0 info procs $name} result] || $result != $name} {
	return
    }
    if {[catch {uplevel #0 info body $name} named_body]} {
	return
    }
    set ix 0
    foreach line [split $named_body "\n"] {
	incr ix
	if {$ix > 10} {
	    break
	}
        if {[regexp {self \[set ([A-Za-z0-9_(::)]+::[A-Za-z0-9_]+)::Snit_instance\]} $line - snitinst]} {
		return $snitinst
	}
    }
    return
}


proc TclReadLine::UTF::is_namespace {word} {
    if {[string range $word 0 1] == "::"} {
	return 1
    } else {
	return 0
    }
}

proc TclReadLine::UTF::print_namespaces {namespaces} {
    variable color
    TclReadLine::print "\nNamespaces: \n"
    foreach namesp $namespaces {
	if {[namespace eval $namesp namespace children] != ""} {
	    set colorcode [UTF::color2ansicode $color(nsbold)]
	} else {
	    set colorcode [UTF::color2ansicode $color(ns)]
	}
	puts -nonewline "[TclReadLine::ESC]\[${colorcode}m${namesp}[TclReadLine::ESC]\[0m "
    }
    puts -nonewline "\n"
}

proc TclReadLine::UTF::NamespaceCompletion {} {
    variable color
    set buf ""
    set firstword ""
    set fwcomplete [parse_firstword firstword firstwordix]
    if {$firstword == ""} {
	set namespaces [uplevel #0 namespace children]
	print_namespaces $namespaces
	return 0
    }
    if {![is_namespace $firstword]} {
	return 0
    }
    set parentnamespace [string range $firstword 0 [expr [string last "::" $firstword] - 1]]
    if {$parentnamespace == ""} {
	set parentnamespace "::"
    }
    if {!$fwcomplete} {
	set maybe ""
	foreach x [namespace eval $parentnamespace namespace children] {
	    if {[string match $firstword* $x]} {
		lappend maybe $x
	    }
	}
	set shortest [TclReadLine::shortMatch $maybe]
	if {$shortest != ""} {
	    set ix [string length $firstword]
	    set TclReadLine::CMDLINE [string replace $TclReadLine::CMDLINE 0 $ix $shortest]
	    set TclReadLine::CMDLINE_CURSOR [string length $shortest]
	    if {[llength $maybe] > 1} {
		print_namespaces $maybe
	    } elseif {$shortest == $firstword} {
		if {[namespace exists $firstword]} {
		    if {[namespace eval $firstword namespace children] == ""} {
			set procs [namespace eval $firstword info procs]
			set colorcode [UTF::color2ansicode $color(procnorm)]
			set buf [lsort $procs]
			puts "\n[TclReadLine::ESC]\[${colorcode}m${buf}[TclReadLine::ESC]\[0m"
		    } else {
			set TclReadLine::CMDLINE [string replace $TclReadLine::CMDLINE 0 [expr $ix + 2] "$shortest\:\:"]
			set TclReadLine::CMDLINE_CURSOR [expr [string length $shortest] +2]
		    }
		}
	    }
	} else {
	    set TclReadLine::COMPLETION_MATCH " not found "
	}
	return 1
    } elseif {[namespace exists $firstword]} {
	set procs [namespace eval $firstword info procs]
	if {$procs == ""} {
	    set colorcode [UTF::color2ansicode $color(vars)]
	    set buf [lsort [namespace eval $firstword info vars]]
	    puts "\n[TclReadLine::ESC]\[${colorcode}m${buf}[TclReadLine::ESC]\[0m"
	} else {
	    set colorcode [UTF::color2ansicode $color(procnorm)]
	    set buf [lsort $procs]
	    puts "\n[TclReadLine::ESC]\[${colorcode}m${buf}[TclReadLine::ESC]\[0m"
	}
    }
    return 0
}

proc TclReadLine::UTF::ObjectCompletionMaybe {maybelist currstr colorcode} {
    set maybe ""
    foreach x $maybelist {
	if {[string match $currstr* $x]} {
	    lappend maybe $x
	}
    }
    set shortest [TclReadLine::shortMatch $maybe]
    if {$shortest != ""} {
	set TclReadLine::CMDLINE [lreplace $TclReadLine::CMDLINE end end $shortest]
	set TclReadLine::CMDLINE_CURSOR [string length $TclReadLine::CMDLINE]
	if {[llength $maybe] > 1} {
	    TclReadLine::print "\n[TclReadLine::ESC]\[${colorcode}m${maybe}[TclReadLine::ESC]\[0m\n"
	}
    } else {
	set TclReadLine::COMPLETION_MATCH " not found "
    }
}


proc TclReadLine::UTF::ObjectCompletion { } {
    variable color
    set numwords [TclReadLine::UTF::parse_cmdline words last]
    if {$numwords && $last != ""} {
	incr numwords -1
    }
    #
    #  Check for null first word,
    #  if so display objects/instances
    #
    if {!$numwords} {
	set levelprocs [lsort [uplevel #0 info procs]]
	if {$last == ""} {
	    set cmds ""
	    foreach objinstance $levelprocs {
		if {[is_objectinst $objinstance]} {
		    if {![catch {$objinstance info type} type]} {
			set snitinst [namespace tail [get_snit_inst $objinstance]]
			set colorcodeobj [UTF::color2ansicode $color(objns)]
			set colorcodeinst [UTF::color2ansicode $color(inst)]
			lappend cmds "[TclReadLine::ESC]\[${colorcodeinst}m${objinstance}[TclReadLine::ESC]\[0m [TclReadLine::ESC]\[${colorcodeobj}m${type} ($snitinst)[TclReadLine::ESC]\[0m"
		    } else {
			error "Completion with $objinstance"
		    }
		}
	    }
	    puts -nonewline "\n"
	    foreach cmd $cmds {
		TclReadLine::print "  $cmd\n"
	    }
	} else {
	    set colorcode [UTF::color2ansicode $color(inst)]
	    TclReadLine::UTF::ObjectCompletionMaybe $levelprocs $last $colorcode
	}
	return 0
    }

    if {![is_objectinst $words(1)]} {
	return [TclReadLine::UTF::CheckTypeCompletion]
    }

    set instmethods [TclReadLine::UTF::getpublicmethods $words(1)]
    set colorcode [UTF::color2ansicode $color(methods)]
    if {$numwords == 1} {
	if {$last == ""} {
	    TclReadLine::print "\n[TclReadLine::ESC]\[${colorcode}m[lsort ${instmethods}][TclReadLine::ESC]\[0m\n"
	} else {
	    TclReadLine::UTF::ObjectCompletionMaybe $instmethods $last $colorcode
	}
    } elseif {$numwords == 2} {
	if {$words(2) == "configure" || $words(2) == "cget"} {
	    set ns [TclReadLine::UTF::get_snit_inst $words(1)]
	    if {$ns != ""} {
		set colorcode [UTF::color2ansicode $color(options)]
		set optionvalues [$words(1) info options]
		if {$last == ""} {
		    TclReadLine::print "\n[TclReadLine::ESC]\[${colorcode}m[lsort ${optionvalues}][TclReadLine::ESC]\[0m\n"
		} else {
		    TclReadLine::UTF::ObjectCompletionMaybe $optionvalues $last $colorcode
		}
	    }
	} elseif {[$words(1) info methods $words(2)] == $words(2)} {
	    if {![catch {eval [list $words(1) $words(2) "options"]} optionvalues]} {
		set colorcode [UTF::color2ansicode $color(options)]
		if {$last == ""} {
		    TclReadLine::print "\n[TclReadLine::ESC]\[${colorcode}m[lsort ${optionvalues}][TclReadLine::ESC]\[0m\n"
		} else {
		    TclReadLine::UTF::ObjectCompletionMaybe $optionvalues $last $colorcode
		}
	    }
	}
    }
    return 1
}


proc TclReadLine::UTF::CheckTypeCompletion {} {
    variable color
    set numwords [TclReadLine::UTF::parse_cmdline words last]
    if {$numwords && $last != ""} {
	incr numwords -1
    }
    #
    #  Check for null first word,
    #
    if {!$numwords} {
	return 0
    }

    if {[catch {$words(1) info typemethods} typemethods]} {
	return 0
    }

    if {![llength $typemethods]} {
	return 0
    }

    set colorcode [UTF::color2ansicode $color(methods)]
    if {$numwords == 1} {
	if {$last == ""} {
	    TclReadLine::print "\n[TclReadLine::ESC]\[${colorcode}m[lsort ${typemethods}][TclReadLine::ESC]\[0m\n"
	} else {
	    TclReadLine::UTF::ObjectCompletionMaybe $typemethods $last $colorcode
	}
    }
    return 1
}


#
# Do inspection of UTF (objects, instances and namespaces)
#
proc TclReadLine::UTF::InspectCompletion { } {
    variable color

    set numwords [TclReadLine::UTF::parse_cmdline words last]
    if {($numwords == 2 && $last != "") || ($numwords == 1 && $last == "")} {
	set numwords 1
    } else {
	return 0
    }
    #
    #  Check for "inspect" as the first word,
    #  if so display objects/instances
    #
    if {![string equal $words(1) "inspect"]} {
	return 0
    }
    set candidatelist ""
    set levelprocs [lsort [uplevel #0 info procs]]
    foreach objinstance $levelprocs {
	if {[is_objectinst $objinstance]} {
	    lappend candidatelist $objinstance
	}
    }
    set colorcode [UTF::color2ansicode $color(inst)]
    if {$last == ""} {
	TclReadLine::print "\n[TclReadLine::ESC]\[${colorcode}m[lsort ${candidatelist}][TclReadLine::ESC]\[0m\n"
    } else {
	TclReadLine::UTF::ObjectCompletionMaybe $candidatelist $last $colorcode
    }
    return 1
}


proc TclReadLine::UTF::ProcCompletion {} {
    variable color
    set buf ""
    set firstword ""
    set fwcomplete [parse_firstword firstword firstwordix]
    if {$firstword == ""} {
	return 0
    }
    set exclude [info commands]
    foreach x $exclude {
	if {[string match $firstword* $x]} {
	    return 0
	}
    }
    set firstwordns "\:\:$firstword"
    set parentnamespace "::[namespace qualifier $firstword]"
    if {!$fwcomplete} {
	set maybe ""
	set nslist [namespace eval $parentnamespace namespace children]
	foreach x $nslist {
	    if {[string match $firstwordns* $x]} {
		lappend maybe [string range $x 2 end]
	    }
	}
	if {[namespace exists  $parentnamespace]} {
	    set proclist [lsort [namespace eval $parentnamespace info procs]]
	    foreach x  $proclist {
		set tmp [string range $parentnamespace 2 end]
		if {[string match [namespace tail $firstword]* $x]} {
		    lappend maybe "$tmp\:\:$x"
		}
	    }
	}
	set shortest [TclReadLine::shortMatch $maybe]
	if {$shortest != ""} {
	    set ix [string length $shortest]
	    set TclReadLine::CMDLINE [string replace $TclReadLine::CMDLINE 0 $ix $shortest]
	    set TclReadLine::CMDLINE_CURSOR [string length $shortest]
	    if {[llength $maybe] > 1} {
		set colorcode [UTF::color2ansicode $color(procnorm)]
		puts "\n[TclReadLine::ESC]\[${colorcode}m${maybe}[TclReadLine::ESC]\[0m "
	    }
	} else {
	    set TclReadLine::COMPLETION_MATCH " not found "
	}
	return 1
    }
    return 0
}


set completionhandlers "TclReadLine::UTF::ProcCompletion \
		TclReadLine::UTF::NamespaceCompletion\
		TclReadLine::UTF::InspectCompletion\
		TclReadLine::UTF::ObjectCompletion"


foreach completionhandler $completionhandlers {
    TclReadLine::delCompletionHandler $completionhandler
    TclReadLine::addCompletionHandler $completionhandler
}

if {[info exists ::__install_interactive_exit_callbacks]} {
    foreach callback $::__install_interactive_exit_callbacks {
	eval [concat TclReadLine::addExitHandler $callback]
    }
    unset ::__install_interactive_exit_callbacks
}
