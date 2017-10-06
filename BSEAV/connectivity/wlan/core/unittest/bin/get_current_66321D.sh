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
# Modified by John Brearley 2009
#
#\
exec /usr/local/bin/epidiag -f "$0" -- "$*"
set self [file tail $argv0]
puts "$self Setting up the GPIB configuration"

set SampleTime [lindex $argv 1]
set SamplePoints [lindex $argv 2]
set OffsetPoints [lindex $argv 3]
set CurrentRange [lindex $argv 4]
set CurrentTrigger [lindex $argv 5]
puts "$self SampleTime=$SampleTime SamplePoints=$SamplePoints\
    OffsetPoints=$OffsetPoints CurrentRange=$CurrentRange CurrentTrigger=$CurrentTrigger"

source /usr/local/lib/epigram/gpib.tcl

# this sets up the trigger for catpure
setgpib 2 *CLS
setgpib 2 "SENS:FUNC 'CURR'"
setgpib 2 "TRIG:SEQ2:SOUR INT"
setgpib 2 "TRIG:SEQ2:LEV:CURR $CurrentTrigger"
setgpib 2 "TRIG:SEQ2:SLOP:CURR POS"
setgpib 2 "TRIG:SEQ2:HYST:CURR 0.001"

# this configs the sample length and range
setgpib 2 "SENS:SWE:TINT $SampleTime"
setgpib 2 "SENS:SWE:POIN $SamplePoints"
setgpib 2 "SENS:SWE:OFFS:POIN $OffsetPoints"

# start
setgpib 2 "INIT:SEQ2"
setgpib 2 "SENS:CURR:RANG $CurrentRange"

# this gets the current samples
set t1 [clock clicks]
set data [getgpib 2 "FETC:ARR:CURR?"]
set low [getgpib 2 "FETC:CURR:LOW?"]
set high [getgpib 2 "FETC:CURR:HIGH?"]
set t2 [clock clicks]

# Display results.
puts "$self Time=[expr ($t2 - $t1)/1000] ms High=$high, Low=$low, Data=$data"
return "$high, $low, $data"
