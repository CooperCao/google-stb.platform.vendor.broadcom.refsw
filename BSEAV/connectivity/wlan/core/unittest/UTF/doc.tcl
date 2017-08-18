#!/bin/env utf
#
# $Copyright Broadcom Corporation$
# $Id: a3ff36ce66d7afe275b0df1b7ba995cfa4733db0 $
#

package require cmdline
package require snit

package provide UTF::doc 2.0

namespace eval UTF {

    # Keep pkg_mkIndex happy
    if {[info command __package_orig] != ""} {
	set ::argv0 ""
	set ::argv ""
	set args(man) "0"
    }

    variable options
    variable args
    variable manpage ""
    variable pagename $::argv0
    # Find UTF base directory based on current script
    variable unittest [file dirname \
			   [file dirname [file normalize [info script]]]]
    variable usrutf $unittest
    lappend options \
	{format.arg "manpage" \
	     "Manpage format: html nroff text latex doctools ..."}
    lappend options {man "display manpage"}

    # Look for usage / help info in ::script_help, otherwise use a simple
    # hard coded string.
    # Note: this use of a global variable is a temporary hack which
    # will go away with the new argument parsing/documentation system.
    if {[info exists ::script_help]} {
        set usage $::script_help
    } else {
        set usage "options:"
    }

    # Convert "man" into "-man".  Necessary for various doc-related
    # optimizations.
    if {[set ix [lsearch $::argv "man"]] >= 0} {
	set ::argv [lreplace $::argv $ix $ix "-man"]
    }
    unset ix

    # Save original commandline in case anyone needs it later.
    set ::__argv $::argv

    # Save shell-format version of original commandline
    set c ""
    foreach a [concat $::argv0 $::__argv] {
	if {[regexp {[\s;\$]} $a]} {
	    lappend c "'$a'"
	} else {
	    lappend c $a
	}
    }
    set ::__cmdline [join $c " "]
    unset c

    if {[catch {cmdline::getKnownOptions ::argv $options $usage} ret]} {
	puts stderr $ret
	exit 1
    } else {
	array set args $ret
    }

    snit::macro UTF::stamethod {name} {
	UTF::doc "
	# \[call \[arg staname] \[method {$name}] \[arg {args ...}]]

	# Pass \[method {$name}] on to the STA's host with the
	# interface name as the first argument.
    "
	method $name {args} "
	    \$host $name \$options(-device) {*}\$args
	"
    }

    snit::macro UTF::PassThroughMethod {name option} {
	UTF::doc "
	# \[call \[arg host] \[method {$name}] \[arg {cmd args ...}]]


	# Execute \[arg {cmd args ...}] on the host object named in
        # \[arg host]'s \[option $option] option.

    "
	method $name {args} "
	set o \[lindex \[\$self cget $option] 0]
	if {\$args eq {}} {
	    return \$o
	}
	if {\$o ne {}} {
	    \$o {*}\$args
	} else {
	    error \"$option not set\"
	}
    "
    }

    # Allow UTF.tcl to have no args for interactive mode
    if {([regexp {.*\.(tcl|exp)|shell$} $::argv0] &&
	 ![regexp {UTF\.tcl} $::argv0]) ? [llength $::argv] > 0 :
	!$UTF::args(man)} {
	# running a subcommand, disable manpage generation
	proc doc {args} {
	}
	snit::macro UTF::doc {args} {
	    typemethod man {} {return}
	}
	proc man {} {
	    puts stderr "Unrecognised command or option: $::argv"
	}
	return
    }

    proc doc {args} {
	variable pagename
	variable manpage

	if {[info script] != $pagename} {
	    snit::macro UTF::doc {args} {
		typemethod man {} {return}
	    }
	    return
	} else {
	    snit::macro UTF::doc {args} {
		foreach txt $args {
		    # If wrapped in braces, add more protection
		    if {[regsub {^{\n?(.*?)\n?}$} $txt {\1} txt]} {
			set txt [string map \
				     {"\[" "\[lb\]" "\]" "\[rb\]" "\#" "\\#"} $txt]
		    }
		    append ::UTF::manpage $txt
		}
		typemethod man {} [list return $::UTF::manpage]
	    }
	    foreach txt $args {
		# If wrapped in braces, add more protection
		if {[regsub {^{\n?(.*?)\n?}$} $txt {\1} txt]} {
		    set txt [string map \
				 {"\[" "\[lb\]" "\]" "\[rb\]" "\#" "\\#"} $txt]
		}
		# Strip leading comment
		# Note: nl are preserved between empty or non-comment lines
		regsub -all {\n[\t ]*\# ?} $txt { } txt
		# Strip off protecting backslash
		regsub -all {\\(.)} $txt {\1} txt
		append manpage $txt
	    }
	    return
	}
    }

    proc man {args} {
	variable manpage
	if {$UTF::args(format) == "doctools"} {
	    puts $manpage
	    exit
	}
	set file "/tmp/[file tail [info script]]"
	set data [open $file w+]
	puts $data $manpage
	close $data
	if {$UTF::args(format) == "manpage"} {
	    set UTF::args(format) nroff
	    set doman 1
	} else {
	    set doman 0
	}

	# this exec is messy because it has to cope with scripts run
	# under the usual "utf" link (which gets the tcl lib paths
	# right), but also with scripts run under "expect" (which has
	# older tcllib, if any) and also a tclsh on Solaris for the
	# web-based man pages.
	if {[catch {
	    exec env TCLLIBPATH="/projects/hnd/tools/linux/share" \
		[info nameofexecutable] \
		"/projects/hnd/tools/linux/bin/dtplite" \
		-o - $UTF::args(format) $file} ret]} {
	    puts stderr $ret
	    puts stderr $manpage
	} else {
	    if {$UTF::args(format) == "nroff"} {
		regsub -line {^\.so man\.macros.?$} $ret {} ret
	    }
	    if {$doman} {
		set cmd [list | groff -mtty-char -Tascii -mandoc]
		if {![string match "CYGWIN*" $::tcl_platform(os)]} {
		    lappend cmd | less -irs
		}
		set fd [open $cmd w]
		puts $fd $ret
		if {[catch {close $fd} ret]} {
		    puts $ret
		}
	    } else {
		puts $ret
	    }
	}
	file delete $file
	exit
    }
}
# skip rest if being sourced
if {![info exists ::argv0] || ![string match [info script] $::argv0]} {
    return
}

UTF::doc {
    # [manpage_begin UTF::doc n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {documentation module for the HND Wireless Test Framework}]
    # [copyright {2005 Broadcom Corporation}]
    # [require APconsole]
    # [description]
    # [para]
    # UTF::doc provides commands for containing and displaying embedded source
    # code documentation.
    #

    # [list_begin definitions]
    #
    # [call [cmd UTF::doc] [arg {text ...}]]

    # Append [arg text] to the documentation.  [arg text] should be in
    # [emph doctools] format optionally indented with " # " for
    # readability.  Newlines between commented line are replaced by
    # spaces.  Newlines can be preserved by leaving blank lines, or by
    # omitting the comment '#' character.  '#' and '\\' can be
    # preserved in the output by protecting with '\\'.  Text can be
    # preserved verbatim by enclosing in an extra set of {} braces.
    # Multiple [arg text] arguments will be concatenated after
    # processing.  This is most useful for switching between protected
    # {} and non-protected modes.

    # [para]

    # [cmd UTF::doc] can also be used within the [cmd snit::type]
    # commands defining UTF objects.  In this case the document
    # created is retained within the snit interpreter.  To retrieve
    # the document and make it available to the outer script, follow
    # the last object definition with a call to the last new object's
    # [method man] method.

    # [call [cmd UTF::man] [lb][option -format] [arg format][rb]]

    # Output formatted manpage to stdout.
    # The default [arg format] is manpage.  Other available formats
    # include html, doctools, latex, nroff, text, tmml and wiki.

    # [call [cmd UTF::stamethod] [arg name]]

    # snit macro to be used in UTF STA object definitions as a shortcut for
    # methods which simply prepend the STA's device name on to their
    # argument list before calling a [arg host] method of the same
    # name.

    # [list_end]

    # [section EXAMPLES]
    # See any of the UTF scripts for examples.
    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also [uri APdoc.cgi?APconsole.exp APconsole]]
    # [see_also [uri APdoc.cgi?APweb.tcl APweb]]
    # [see_also doctools]
    # [keywords doctools nroff html man]
    # [manpage_end]

}
UTF::man
