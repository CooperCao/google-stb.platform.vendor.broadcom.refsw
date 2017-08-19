#!/bin/env utf

#
# UTF Framework reusable test script utilities
# Based on snit
# $Copyright Broadcom Corporation$
#

namespace eval UTF::help {}
package provide UTF::help 2.0

# This package must be loaded first without loading any other UTF
# packages. The reason that this is necessary is that UTF/doc.tcl
# actually runs some code when it is loaded and does some parsing
# of command line arguments. The UTF/doc.tcl code is also responsible
# for responding to the -help option. 

# The higher level script needs a chance to setup ::script_help before
# UTF/doc.tcl is loaded. Consequently, UTF/help.tcl must not load 
# any other UTF packages. This ensures that ::script_help can be 
# setup before UTF/doc.tcl is loaded/run.

# A side effect is that this package cant use the UTF UTF/doc.tcl
# man page capability. However a pseudo man page is available at the
# end of this file. Type: UTF/help.tcl

proc UTF::setup_help {help_text getopts_list} {

    # Dont overwrite ::script_help if it is already defined.
    if {[info exists ::script_help]} {
        return
    }

    # help_text is used as is, as it is expected to contain new-lines
    # for the desired formatting. 
    set ::script_help "$help_text"

    # getopts_list is in list of lists format, and needs reformatting
    # for online help display.
    foreach line $getopts_list {

        # Skip blank lines, if any.
        set cnt [llength $line] 
        if {$cnt == 0} { 
            continue
        }

        # Option name is first token in line
        set opt_name [lindex $line 0]
        set opt_name [lindex [split $opt_name "."] 0] ;# drop .arg suffix

        # Middle token in line, if any, is default value
        if {$cnt == 3} {
            set opt_default "\<[lindex $line 1]\>"
            append opt_name " value" ;# emulate cmdline behavior
        } else {
            set opt_default ""
        }

        # Last token in line is description
        set opt_description [lindex $line end] 

        # Reorder data to match package cmdline behavior, add new-lines.
        set opt_name [format %-19s $opt_name] ;# right pad with spaces
        append ::script_help "\n -$opt_name $opt_description $opt_default"
    }
}

# Skip rest if being sourced
if {![info exists ::argv0] || ![string match [info script] $::argv0]} {
    return
}

# Pseudo man page to display when user types: UTF/help.tcl
puts "\nUTF::setup_help help_text getopts_list\n"
puts "Some scripts provide online help when the user specifies the -help"
puts "option. This routine reformats help_text & getopt_list strings and"
puts "saves them in global variable ::script_help BEFORE the other UTF"
puts "packages are loaded. Dont overwrite ::script_help if it is already"
puts "defined. This allows for nesting of scripts. The first script to"
puts "load defines the overall help info.\n"

puts "help_text is a text string with help information. It is"
puts "expected to contain new-line characters to achieve the desired"
puts "formatting. getopts_list is a TCL list of lists that can"
puts "be passed to package cmdline proc getKnownOptions. There are no"
puts "formatting characters allowed in getopts_list."

puts "\nExample usage:"
puts "package require UTF::help ;# must be first package loaded"
puts "set Sanity_help \"details \\nmore details \\nmore details\""
puts "set Sanity_getopts {{auth \"authorize devices\"} {perfloop.arg 1 \"performance loops\"}}"
puts "UTF::setup_help \$Sanity_help \$Sanity_getopts"
puts "package require UTF ;# load other UTF packages after setup_help is run"
puts "\nSee also Test/Sanity.test for more detailed example"
