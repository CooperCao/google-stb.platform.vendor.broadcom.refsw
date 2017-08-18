#Copyright 2009, Broadcom Corporation.
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied or
# duplicated in any form, in whole or in part, without the prior written
# permission of Broadcom Corporation.
#
# $Id:
#
# this sets up the GPIB configurations
#
# Modified by John Brearley 2009/10/14
# Script now uses calling line tokens for desired voltage & current settings.
#\
exec /usr/local/bin/epidiag-gpibfix -f "$0" -- "$*"
set self [file tail $argv0]
puts "$self Setting up the GPIB configuration"

# Set desired voltage & current from command line arguments
# First arg in argv has been set to --
# puts "argv=$argv"
set out_voltage [lindex $argv 1]
if {![regexp {^[\d\.]+$} $out_voltage]} {
    set out_voltage 3.3
}
set out_current [lindex $argv 2]
if {![regexp {^[\d\.]+$} $out_current]} {
    set out_current 2
}
puts "$self out_voltage=$out_voltage out_current=$out_current"

source /usr/local/lib/epigram/gpib.tcl
puts "$self Now clearing and resetting the Power Supplies"
# This clears the Power supplies and resets them.
setgpib 2 *RST
setgpib 2 *CLS
setgpib 2 "VOLT $out_voltage"
setgpib 2 "CURR $out_current"
setgpib 2 "OUTP:COMP:MODE LLOCAL"
setgpib 2 "SENS:CURR:DET ACDC"
after 5000

# Activate output voltage as needed.
if {$out_voltage <= 0} {
   # NB: Dont turn on output with the output voltage set to 0!
   # Agilent goes into overload and starts making a loud whining noise!
   # Use the OUTP command to turn it off.
   setgpib 2 "OUTP OFF"
   puts "$self Output should be OFF / Disabled"
   exit

} else {

   setgpib 2 "OUTP ON"
   puts "$self Output should now be enabled for $out_voltage V"
}
