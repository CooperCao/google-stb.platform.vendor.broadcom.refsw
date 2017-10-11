#!/usr/local/bin/tclsh

# $Id$
# $Copyright Broadcom Corporation$

# Written by John Brearley 2009

# Used to collect current samples continuously from an Agilent E3644A
# power supply. This is done to compensate for the fact that the E3644A
# does not have a high speed capture buffer like the 66321D has. The
# E3644A can collect about 5 samples per second. 

# Normally this script is called by Power.tcl, Agilent object, method
# setup_current_trigger.

# Get 3 command line parameters
if {$argc < 3} {
   puts "$argv0 ERROR: Need 3 calling tokens: path_agshell ip:port sample_cnt"
   exit 1
}
set path_agshell [lindex $argv 0]
set ip_port [lindex $argv 1]
set sample_cnt [lindex $argv 2]
puts "$argv0 path_agshell=$path_agshell ip_port=$ip_port sample_cnt=$sample_cnt"

# Main loop to collect current samples.
set t1 [clock clicks];# start test timing
set high ""
set low ""
set data ""
for {set i 1} {$i <= $sample_cnt} {incr i} {
    set curr [exec $path_agshell/agshell $ip_port "meas:curr\?"]
    set data "${data}${curr}, " ;# collect samples in CSV string

    # Collect high & low values
    if {$high == "" || $curr > $high} {
       set high $curr
    }
    if {$low == "" || $curr < $low} {
       set low $curr
    }
}
set t2 [clock clicks];# end test timing

# Show data in the same format returned by the get_current_66321D.sh script.
# This ensures that method get_current_trigger_data will parse it correctly.
set result "$high, $low, $data"
puts "$argv0 Time=[expr ($t2 - $t1)/1000] ms High=$high, Low=$low, Data=$data"
return "$high, $low, $data"
