#!/usr/bin/tclsh
# This file is stored in CVS in unittest/bin

# Give help if needed.
set x [lindex $argv 0]
set x [string tolower $x]
set x [string range $x 0 1]
# puts "x=$x"
if {$x == "-h" || $x == "/?" || $x == ""} {
    puts "Basic usage: $argv0 \[standard iperf options\]"
    puts " "
    puts "This is a simple wrapper script around iperf to add local timestamps to"
    puts "the STDOUT messages from iperf. Newer versions of iperf support the"
    puts "'-y C' option which formats the output in CSV format including a"
    puts "timestamp. However UTF uses an older version of iperf that does not"
    puts "support that option, so this wrapper script implements the timestamps."
    exit 1
}

# Start iperf as command pipeline channel.
set args "iperf $argv"
set fd [open "|$args" r]
puts "$argv0 fd=$fd args=$args"

# Setup channel for reading.
fconfigure $fd -blocking 0
fileevent $fd readable {set reading READY}

# Watch for msgs from iperf & add timestamps.
while { 1 } {

    # Wait for data
    vwait reading

    # Try to get data from fd.
    set msg ""
    set catch_resp [catch "set msg \[gets $fd\]" catch_msg]
    # puts "catch_resp=$catch_resp catch_msg=$catch_msg"
    if {$catch_resp != 0} {
        # This fd may have been valid earlier, but it is now expired.
        error "$argv0 ERROR: $catch_msg"
    }

    # Process data from fd.
    if {[eof $fd]} {
        puts "$argv0 got normal EOF, closing fd=$fd"
        # Put file descriptor back to blocking so that close can
        # get a valid return status
        fconfigure $fd -blocking 1
        if {[catch {close $fd} ret]} {
           error "$argv0 ERROR closing fd=$fd $ret"
        }
        break

    } elseif {![fblocked $fd]} {
        # Add timestamps to the response data.
        set hhmmss [clock format [clock seconds] -format "%T"]
        puts "$hhmmss $msg"
        flush stdout ;# may or may not help
    }
}
