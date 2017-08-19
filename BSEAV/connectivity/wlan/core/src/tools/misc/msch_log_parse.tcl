#! /tools/bin/tclsh
#
# Multichannel scheduler is capable of storing logs and dumping them
# to the host when required. This script parses the logs and extracts meaningful
# data that is easier to visualize
#
# msch_log_parse.tcl,v 2.0 jingyao
#
# Usage: Input to the parser is the log file that is dumped by MSCH module.
#        Alternatively, this script can also be sued to run a basic scan/sta
#        test on the linux brix and generate the log file. If this path is chosen,
#        FW path, DHD path etc., need to be entered.
#        Example
#	 Branch:
#		DINGO_TWIG_9_10_127
#
#	 working binaries:
#		/projects/hnd_sw_mobhost_ext1/work/4359b0_Android/logs/working
#
#	 Compile options:
#		4359b0-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-pktctx-die3-dfsradar-msch-norsdb-biglog
#
#
# Once we have the log file either way, the script parses it to generate data such as:
# Duration:			The duration that the msch scheduler spends on each channel
# Start time:			Each channel's start time
# End time:			Each channel's end time
# Onchan_start_duration:	From onchan to chan_start
# Adopt_onchan_duration:	From adtop time to on chan time
#				The time it takes for the scheduler to issue a callback
#				from the moment it decides to change the channel
#
# These are few examples. Can be modified to generate necessary data as we see fit.
# The parsed data is stored as lists in a text file. These can be easily imported to
# gnuplot() or R to visualize as graphs
# An example of R commands to generate plots are mentioned at the end of this script


# ###################################################################################

# Clear all existing files

set lsdata [exec ls -a]

if [string match "channel_list.txt" $lsdata] \
{
    exec rm channel_list.txt
}
if [string match "duration_list.txt" $lsdata] \
{
    exec rm duration_list.txt
}
if [string match "starttime_list.txt" $lsdata] \
{
    exec rm starttime_list.txt
}
if [string match "endtime_list.txt" $lsdata] \
{
    exec rm endtime_list.txt
}
if [string match "onchan_start_duration_list.txt" $lsdata] \
{
    exec rm onchan_start_duration_list.txt
}
if [string match "adopt_onchan_duration_list.txt" $lsdata] \
{
    exec rm adopt_onchan_duration_list.txt
}

puts "Want to parse an existing log or get a new log? (E or N)"
flush stdout
set option [gets stdin]
if [string match "E" $option] {
    puts "Chosen existing"
    puts "==============="
    puts "Enter exact filename:  "
    flush stdout
    set filename [gets stdin]
    if { [file exists $filename] == 0} {
        puts "File not found!"
        exit
    }
} elseif [string match "N" $option] {
    puts "Chosen new"
    puts "Enter a file name to save log"
    flush stdout
    set filename [gets stdin]
    puts "Enter the FW path"
    flush stdout
    set fw_path [gets stdin]
    puts "Enter DHD path"
    flush stdout
    set dhd_path [gets stdin]
    puts "Enter the nvram file path"
    flush stdout
    set nvram_path [gets stdin]
    puts "Enter the SSID to join"
    flush stdout
    set ssid [gets stdin]

    if {[catch {exec ./wl disassoc} errmsg]} {
      puts ".... - $errmsg.....Continuing"
    } else {
      puts "continuing"
    }
    if {[catch {exec ./wl down} errmsg]} {
      puts ".... - $errmsg.....Continuing"
    } else {
      puts "continuing"
    }
    if {[catch {exec rmmod dhd} errmsg]} {
      puts ".... - $errmsg.....Continuing"
    } else {
      puts "continuing"
    }

    exec insmod $dhd_path
    after 1000
    exec ifconfig -a >@stdout
    exec dhd -i eth1 download $fw_path $nvram_path
    after 1000
    exec dhd -i eth1 dconpoll 50
    after 1000
    exec ifconfig eth1 192.168.1.99 up
    after 1000
    exec dhd -i eth1 version
    after 1000
    exec ./wl up
    after 1000
    exec ./wl msch_collect 1 -s 40 -p
    puts "Security enabled? {Y or N}"
    flush stdout
    set security [gets stdin]
    if [string match "Y" $security] \
    {
        puts "Chosen Security, enter password"
        flush stdout
        set passwd [gets stdin]
        exec ./wl wsec 4
        exec ./wl set_pmk $passwd
        exec ./wl sup_wpa 1
        exec ./wl wpa_auth 0x80
    }
    exec ./wl join $ssid
    exec ./wl status
    after 5000
    set lsdata [exec ls -a]
    if [string match $filename $lsdata] \
    {
        exec exec rm $filename
    }
    
    exec ./wl msch_dump $filename

} else {
    puts "Wrong Input"
    exit
}

set fp [open $filename r]
set file_data [read $fp]
close $fp

set register_index [lsearch -all $file_data "wlc_msch_timeslot_register:"]
set onchan_index [lsearch -all $file_data "ON_CHAN"]
set slotstart_index [lsearch -all $file_data "SLOT_START"]
set slotend_index [lsearch -all $file_data "SLOT_END:"]
set adopttoon_index [lsearch -all $file_data "_msch_chan_adopt"]
set total_channels [llength $slotend_index]

set total_channel_changes 0
foreach elem $onchan_index {
    incr total_channel_changes
}

puts "Total channel changes : "
puts $total_channel_changes

# Capture channel list
for {set i 0} {$i < [expr $total_channels]} {incr i} {

	set slotend_elem [lindex $slotend_index [expr $i]]
	set check_slotreq_end [lindex $file_data [expr $slotend_elem - 1]]
	set comparision_reqend [string match REQ_END $check_slotreq_end]
	#if SLOT_END does not appear, drop the package
	if {$comparision_reqend == 1} {
		set channel_value [lindex $file_data [expr $slotend_elem - 3]]
		lappend channels_array [lindex $channel_value]
	} else {
		set channel_value [lindex $file_data [expr $slotend_elem - 2]]
		lappend channels_array [lindex $channel_value]
	}
}

# Clean data
set channels_array [string map {/80 "" /40 ""} $channels_array]
puts $channels_array

set total_channel_changes [llength $channels_array]
set total_onchan_entries [llength $onchan_index]
set total_slotstart_entries [llength $slotstart_index]
set total_slotend_entries [llength $slotend_index]
set total_channeladopt_entries [llength $adopttoon_index]


# format: REGISTER:
#  <chanspec_list>:  xxx
#  099096.292572 CALLBACK: channel 1 -- ON_CHAN REQ_START ID 0 pre-start: 0.000000 end: 0.000000 duration 0
#  099096.325653 CALLBACK: channel 1 -- SLOT_START
#  099108.523498 CALLBACK: channel 1 -- SLOT_END: duration 39978
#  099108.523498 MESSAGE: MSCH_DEBUG: _msch_chan_adopt Change Chanspec 0x1002
#
#  If No SLOT_END shopws up, drop the packet
#  099096.287841 MESSAGE: MSCH_DEBUG: wlc_msch_timeslot_register: req_entity_create, chanspec_cnt 1
#  099096.287872 MESSAGE: MSCH_DEBUG: _msch_chan_adopt Change Chanspec 0x1001
#
#  099096.292572 CALLBACK: channel 1 -- ON_CHAN REQ_START ID 0 pre-start: 0.000000 end: 0.000000 duration 0
#  099096.325653 CALLBACK: channel 1 -- SLOT_START


# Duration from slot start to slot end
for {set i 0} {$i <[expr $total_channels]} {incr i} {

	set slotend_elem [lindex $slotend_index [expr $i]]
	set check_slotreq_end [lindex $file_data [expr $slotend_elem - 1]]
	set comparision_reqend [string match REQ_END $check_slotreq_end]

	#if SLOT_END does not appear, drop the package
	if {$comparision_reqend == 1} {
		set slotend_time [lindex $file_data [expr $slotend_elem - 6]]
		set slotstart_time [lindex $file_data [expr $slotend_elem - 12]]
		set duration [expr $slotend_time - $slotstart_time]
	} else {
		set slotend_time [lindex $file_data [expr $slotend_elem - 5]]
		set slotstart_time [lindex $file_data [expr $slotend_elem - 11]]
		set duration [expr $slotend_time - $slotstart_time]
	}

	lappend duration_array [lindex $duration]

	#store slot start time and slot end time
	if {$i == 0} {
		set starttime_offset [expr floor($slotstart_time)]
		set endtime_offset [expr floor($slotend_time)]
	}

	set slotstart_adjusted_time [expr $slotstart_time - $starttime_offset]
	set slotend_adjusted_time [expr $slotend_time - $endtime_offset]
	lappend starttime_array_adj [lindex $slotstart_adjusted_time]
	lappend endtime_array_adj [lindex $slotend_adjusted_time]

	lappend starttime_array [lindex $slotstart_time]
	lappend endtime_array [lindex $slotend_time]
}


# Duration from on chan to start
for {set i 0} {$i <[expr $total_slotend_entries]} {incr i} {
	set slotend_elem [lindex $slotend_index [expr $i]]
	set slotonchan_elem [lindex $onchan_index [expr $i]]

	set index_end_onchain [expr $slotend_elem - $slotonchan_elem]

	#only calculate duration that has "SLOT_END" keyword
	#delete the element in on_chan, start list and adopt list that do not contain "SLOT-END" keyword
	while { $index_end_onchain != 21 && $index_end_onchain != 20} {
		set onchan_index [lreplace $onchan_index $i $i]
		set slotstart_index [lreplace $slotstart_index $i $i]
		set adopttoon_index [lreplace $adopttoon_index $i $i]

		set slotonchan_elem [lindex $onchan_index [expr $i]]
		set index_end_onchain [expr $slotend_elem - $slotonchan_elem]
		if {$i >= [expr $total_slotend_entries]} {
			break;
		}
	}

	set slotstart_elem [lindex $slotstart_index [expr $i]]
	set slotstart_time [lindex $file_data [expr $slotstart_elem - 5]]

	set slotonchan_elem [lindex $onchan_index [expr $i]]
	set slotonchan_time [lindex $file_data [expr $slotonchan_elem - 5]]

	set onchan_start_duration [expr $slotstart_time - $slotonchan_time]
	lappend onchan_start_duration_list [lindex $onchan_start_duration]
}

set total_channeladopt_entries [llength $adopttoon_index]

# Duration from adopt channel to on channel
for {set i 0} {$i <[expr $total_slotend_entries]} {incr i} {
	set slotadopt_elem [lindex $adopttoon_index [expr $i]]
	set slotonchan_elem [lindex $onchan_index [expr $i]]
	set index_adopt_onchain [expr $slotonchan_elem - $slotadopt_elem]
	if {$index_adopt_onchain == 9} {
		set slotonchan_time [lindex $file_data [expr $slotonchan_elem - 5]]
		set slotadopt_time [lindex $file_data [expr $slotadopt_elem - 3]]
		set adopt_onchan_duration [expr $slotonchan_time - $slotadopt_time]
		lappend adopt_onchan_duration_list [lindex $adopt_onchan_duration]
	}
}

# Save the data for plotting

# channels list
set filename "channel_list.txt"
set filid [open $filename "w"]
puts $filid $channels_array
close $filid

# duration from start chan to end chan
set filename "duration_list.txt"
set filid [open $filename "w"]
puts $filid $duration_array
close $filid


# start time
set filename "starttime_list.txt"
set filid [open $filename "w"]
puts $filid $starttime_array
close $filid

# end time
set filename "endtime_list.txt"
set filid [open $filename "w"]
puts $filid $endtime_array
close $filid

# duration from on chan to chan_start
set filename "onchan_start_duration_list.txt"
set filid [open $filename "w"]
puts $filid $onchan_start_duration_list
close $filid

# duration from adopt to on chan
set filename "adopt_onchan_duration_list.txt"
set filid [open $filename "w"]
puts $filid $adopt_onchan_duration_list
close $filid

# Plot the saved arrays using GNUplot or R

exit

# Example in R
#duration <- scan("C:/Users/jingyao/Desktop/MSCH/logs/duration_list.txt")
#channels <- scan("C:/Users/jingyao/Desktop/MSCH/logs/channel_list.txt")
#endtime <- scan("C:/Users/jingyao/Desktop/MSCH/logs/endtime_list.txt")
#starttime <- scan("C:/Users/jingyao/Desktop/MSCH/logs/starttime_list.txt")
#plot(duration, pch=16, main='Duration: Time spent on each channel during a scan + sta', xlab='Channel Index', ylab='Duration (ms)')
#for (i in 1:83) {
#k <- duration[i]
#text(i,k, channels[i], 2, 3)
#}

# Check http://people.duke.edu/~hpgavin/gnuplot.html for a tutorial on gnuplot()
