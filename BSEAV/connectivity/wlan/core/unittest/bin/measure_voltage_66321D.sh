# Copyright 2009, Broadcom Corporation.
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
# Written by John Brearley 2009
#\
exec /usr/local/bin/epidiag-gpibfix -f "$0" -- "$*"
set self [file tail $argv0]
puts "$self Setting up the GPIB configuration"

source /usr/local/lib/epigram/gpib.tcl

puts "$self Measuring instantaneous voltage"
set data [getgpib 2 "MEAS:VOLT:ACDC?"]
puts "$self Instantaneous voltage=$data V"
return $data
