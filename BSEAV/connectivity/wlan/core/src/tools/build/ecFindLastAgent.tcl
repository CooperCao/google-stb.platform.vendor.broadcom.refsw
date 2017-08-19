#-------------------------------------------------------------
# ecFineLastAgent.tcl - provided by electric cloud
#
# This script contains Tcl code that accesses the E.C annolib 
# API to output the agent that caused a emake build to fail.
#
# The node and agent descriptot is output to stdout 
#
# Arguments:
#    --anno=<xml-annotation-file>
#
# In our hnd s/w build env  xml-file is created at <build>/misc/,emake*.xml
#
# $Id: ecFindLastAgent.tcl,v 12.1 2006-09-12 02:19:07 $
#
#-------------------------------------------------------------

load annolib annolib
package require annolib

# current annolib reference
set anno {}


#----------------------------------------------------------
# get_failed_node
#    Finds the node where this build failed.  If no jobs failed,
#    then returns empty string.
#
# Arguments:
#   None
# 
# Returns:
#    the node or empty string
# -----------------------------------------------------------
proc get_failed_node {} {
    $::anno jobiterbegin
    
    while { [$::anno jobitermore] } {
	set job_id [$::anno jobiternext]
	if { [$::anno job exitcode $job_id] } {
	    # just stop after the first one is found ... 
	    # ASSUMPTION : if an error is ignored, then it won't show
	    # up in this list
	    return [$::anno job agent $job_id]
	}
    }
    # if we made it here, then there are no failed jobs in this build
    return "";
}

#------------------------------------------------------------
# main
#      main subroutine
# Opens the annotation file and starts processing
#
# Arguments:
#    argv  command line arguments
#------------------------------------------------------------
proc main {argv} {

    # process command line arguments
    set usage \
	{$argv0 --anno=file

Switches:
    --anno=file      path and filename to annotation.

	}

    set numArgs [llength $argv]

    for {set i 0} {$i < $numArgs} {incr i} {
        set flag [lindex $argv $i]
        if { [string equal -length 7 "--anno=" $flag] } {
	    set annofile [string range $flag 7 end]
	} else {
	    puts $usage
	    exit
	}
    }
    
    if { [info exists annofile] } {
	# TODO: check to see if the anno file exists?
	set anno_fh [open $annofile "r"]
	fconfigure $anno_fh -translation binary
	set ::anno [anno create]
	$::anno load $anno_fh

	puts [get_failed_node]

	close $anno_fh
    } else {
	puts $usage
	exit
    }
}


main $argv
