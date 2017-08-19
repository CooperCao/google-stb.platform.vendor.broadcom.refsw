#!/bin/env utf

#
# Script utilities used by BTAMP tests
#
# $Copyright Broadcom Corporation$
#

package require UTF::doc
package require UTF::utils

namespace eval btamplib {}
package provide UTF::btamplib 2.0

UTF::doc {
    # [manpage_begin UTF::btamp n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {Script utilities used by BTAMP tests}]
    # [copyright {2009 Broadcom Corporation}]
    # [require UTF::btamplib]
    # [description]
    # [para]

    # UTF::btamp contains various reusable script utility routines
    # intended for use by BTAMP scripts. See sample usage of the 
    # of the utilities in Test/btampqual.test.

    # [list_begin definitions]
}


#======================== This code facilitates interactive debugging ============
if {![info exists ::tc_num]} {
    set ::tc_num "n/a"
}
if {![info exists ::tc_title]} {
    set ::tc_title "n/a"
}
if {![info exists ::errors]} {
    set ::errors 0
}
if {![info exists ::error_list]} {
    set ::error_list ""
}
if {![info exists ::warnings]} {
    set ::warnings 0
}
if {![info exists ::warning_list]} {
    set ::warning_list ""
}
if {![info exists ::tc_errors]} {
    set ::tc_errors 0
}
if {![info exists ::tc_fail]} {
    set ::tc_fail 0
}
if {![info exists ::tc_fail_list]} {
    set ::tc_fail_list ""
}
if {![info exists ::tc_pass]} {
    set ::tc_pass 0
}
if {![info exists ::tc_state]} {
    set ::tc_state ""
}
if {![info exists ::local_os]} {
    set ::local_os $tcl_platform(os)
}
if {![info exists ::log_ip]} {
    set ::log_ip "xyz"
}
if {![info exists ::FAST]} {
    set ::FAST 0
}
if {![info exists ::country]} {
    set ::country us
}
if {![info exists ::ch_low]} {
    set ::ch_low 1
}
if {![info exists ::ch_high]} {
    set ::ch_high 1
}
if {![info exists ::log_line_cnt]} {
    set ::log_line_cnt 0
}
if {![info exists ::log_line_max]} {
    set ::log_line_max 0
}
if {![info exists ::loc_wl]} {
    # For Cygwin, use Unix forward slash directory separator
    set ::loc_wl "c:/tools/win32/bin/wl"
}
if {![info exists ::linux_wl]} {
    set ::linux_wl "/usr/bin/wl"
}
if {![info exists ::loc_dhd]} {
    # dhd module is expected to be in same directory as wl.
    regsub "wl" $::loc_wl "dhd" ::loc_dhd
}
if {![info exists ::linux_dhd]} {
    # dhd module is expected to be in same directory as wl.
    regsub "wl" $::linux_wl "dhd" ::linux_dhd
}
if {![info exists ::loc_msgs]} {
    set ::loc_msgs "c:/tools/win32/var/log"
}
if {![info exists ::linux_msgs]} {
    set ::linux_msgs "/var/log"
}
if {![info exists ::localhost]} {
    set ::localhost xyz
}
if {![info exists ::btamp_dut]} {
    set ::btamp_dut ""
}
if {![info exists ::btamp_ref1]} {
    set ::btamp_ref1 ""
}
if {![info exists ::btamp_ref2]} {
    set ::btamp_ref2 ""
}
#============================== DebugView procs =================================
# Initially the approach was to shut down DebugView at the start of each test,
# delete the existing messages log, and then restart DebugView. Then after each
# test had completed, DebugView would be shut down again and the messages log 
# would be transferred to the control script host for analysis. The fatal flaw
# in this approach was that DebugView often got a file access / permission error
# when it started up, and would sit there with a popup window error message that
# requires the human to go click the OK button and take additional recovery action.
# This resulted in a very high testcase failure rate due to lack of DebugView
# message logs.

# The new approach is to leave DebugView running at all times. We basically get
# only the part of the ongoing running DebugView log file that is relevant to
# the test currently being run. This is achieved by using the unix utility wc
# to count the number of lines in the DebugView log file at the start of each
# test. At the end of the test, we use the unix utility tail to get all new 
# lines of data from the DebugView log file, starting at the line number we 
# saved at the beginning of the test. In addition to radically reducing the 
# testcase failure rate, this approach is also much, much faster.

UTF::doc {
    # [call [cmd btamplib::deleteDebugViewLog] [arg ip] [opt logFileName=messages]]

    # Used to delete existing DebugView log file, if any, on host [arg ip].
}

proc btamplib::deleteDebugViewLog {ip {logFileName messages}} {
    UTF::Message LOG $ip "btamplib::deleteDebugViewLog: ip=$ip logFileName=$logFileName"
    btamplib::stopDebugView $ip

    # This time delay may help eliminates file access errors
    UTF::Sleep 2 $ip "Wait for DebugView to release $::loc_msgs/${logFileName}"

    # Erase logFileName
    btamplib::run_cmd $ip "rm -f $::loc_msgs/${logFileName}" -noerror
}

UTF::doc {
    # [call [cmd btamplib::getMessagesLogLineCnt] [arg ip] [opt logFileName=messages]
    # [opt noerror=null]]

    # This routine gets the line count of the DebugView or Linux [arg logFileName] on host
    # [arg ip] and stores it in a global variable called ::[arg ip]_messages_line_cnt.
    # An error will be logged if the line count is less than the 
    # last time it was checked. When [arg noerror] is set to -noerror, all errors
    # will be logged as LOG messages.[para]

    # Returns the integer line count.
}

proc btamplib::getMessagesLogLineCnt {ip {logFileName "messages"} {noerror ""}} {

    # Clean up calling args
    set noerror [string trim $noerror]
    set noerror [string tolower $noerror]

    # If necessary, initialize a separate global variable to store
    # the line count for each host.
    set ip [string trim $ip]
    set var "::${ip}_messages_line_cnt"
    if {[info exists $var]} {
        set curr_cnt [set $var]
        puts "found $var=$curr_cnt"
    } else {
        set $var -1
        set curr_cnt [set $var]
        puts "initializing $var"
    }

    # Use wc to get number of lines in file on remote PC.
    set type [UTF::check_host_type $ip]
    if {$type == "Cygwin" || $type == "WinDHD"} {
        set dir $::loc_msgs
    } else {
        set dir $::linux_msgs
    }
    set resp [btamplib::run_cmd $ip "wc -l $dir/${logFileName}" -noerror]

    # Normally the line count will be the first token of the response.
    set new_cnt [lindex $resp 0]
    # puts "resp=$resp new_cnt=$new_cnt"
    if {[regexp {^\d+$} $new_cnt]} {
        # Got valid integer, save it.
        set $var $new_cnt
    } else {
        # We got an error. Log it if appropriate.
        if {$noerror != "-noerror"} {
            btamplib::logError $ip "btamplib::getMessagesLogLineCnt got resp=$resp"
        }
        set $var "-1"
        return "-1"
    }

    # Normally there are all manner of items being logged by DebugView or Linux.
    # So the line count should have increased since the last time we looked.
    # However 4329 can be very quiet. So we only complain if line count has
    # decreased. If the host crashes & reboots, the log file will be empty.
    if {$new_cnt < $curr_cnt && $noerror != "-noerror"} {
        btamplib::logError $ip "btamplib::getMessageLogLineCnt got new_cnt=$new_cnt\
            LE curr_cnt=$curr_cnt"
    }
    UTF::Message LOG $ip "btamplib::getMessagesLogLineCnt new_cnt=$new_cnt"

    # If any Windows log file GE 500,000 lines, set the flag to clean up ALL 
    # log files Windows at the end of the test run.
    if {($type == "Cygwin" || $type == "WinDHD") && $new_cnt >= 500000} { 
        set ::clean_dbgview_logs "yes"
    }
    return $new_cnt
}


UTF::doc {
    # [call [cmd btamplib::getMessagesLog] [arg ip]
    # [opt logFileName=messages]]

    # Uses ssh & tail to get the DebugView or Linux messages logfile on host [arg ip]. This routine
    # assumes that the logfile is in directory specified by ::loc_msgs[para]

    # The contents of the file is returned as a string.
}

proc btamplib::getMessagesLog {ip {logFileName messages}} {
    set type [UTF::check_host_type $ip]
    set ip [string trim $ip]
    UTF::Message LOG $ip "btamplib::getMessagesLog: $ip $logFileName $type"

    # Set directory for messages file based on OS
    if {$type == "Cygwin" || $type == "WinDHD"} {
        set dir $::loc_msgs
    } else {
        set dir $::linux_msgs
    }

    # Use the tail --line=+N to get all data in file starting at line N
    set var "::${ip}_messages_line_cnt"
    if {[info exists $var]} {
        set curr_line [set $var]
        # puts "found $var=$curr_line"
    } else {
        btamplib::logError $ip "btamplib::getMessagesLog var=$var not found!"
        return
    }
    set file_string [btamplib::run_cmd $ip "tail --line=+$curr_line\
        $dir/$logFileName" -s -warn]
    return $file_string
}

UTF::doc {
    # [call [cmd btamplib::stopDebugView] [arg ip]]

    # Stops all DebugView processes on host [arg ip].
    # Checks processes really are gone and tries again
    # if necessary.
}

proc btamplib::stopDebugView { ip } {
    UTF::Message LOG $ip "btamplib::stopDebugView ip=$ip"
    # Wait for last HCI events to finish up. If we dont wait, we may be 
    # missing messages in the log file. 
    UTF::Sleep 2 $ip

    # UTF::terminate_pid_by_name tries multiple times
    set catch_resp [catch "UTF::terminate_pid_by_name $ip Dbgview" catch_msg]
    if {$catch_resp != 0} {
        btamplib::logError $ip "btamplib::stopDebugView: $catch_resp"
    }

    # Did we really get rid of DebugView?
    set cnt [btamplib::findProcessByName $ip Dbgview -nowarn]
    if {$cnt == 0} {
        return
    }

    # We failed to terminate the DebugView processes after multiple tries => error
    btamplib::logError $ip "btamplib::stopDebugView tried\
        to terminate DebugView processes and failed, cnt=$cnt!"
    return
}

UTF::doc {
    # [call [cmd btamplib::waitDebugView] [arg ip]]

    # On host ip, waits for DebugView to be started by debugview_daemon.tcl that
    # runs locally on host [arg ip].[para]

    # Returns time waited, in seconds.
}

proc btamplib::waitDebugView { ip } {

    # Save starting time
    set start_sec [clock seconds]
    UTF::Message LOG $ip "btamplib::waitDebugView ip=$ip starting..."

    # Watch for DebugView processes in a loop.
    while { 1 } {
        set cnt [btamplib::findProcessByName $ip dbgview -nowarn]
        if {$cnt > 0} {
            # Found DebugView processes, log the info and return
            set now_sec [clock seconds]
            set delta_sec [expr $now_sec - $start_sec]
            UTF::Message LOG $ip "btamplib::waitDebugView found $cnt DebugView\
                process, delay $delta_sec seconds"
            return $delta_sec
        }

        # Have we timed out?
        set now_sec [clock seconds]
        set delta_sec [expr $now_sec - $start_sec]
        # UTF::Message LOG $ip "btamplib::waitDebugView delta_sec=$delta_sec"
        if {$delta_sec > 45} {
            break 
        }

        # Wait a bit and try again
        UTF::Sleep 2 $ip
    }

    # Issue timeout error
    btamplib::logError "$ip" "btamplib::waitDebugView $ip timed out, no DebugView\
        process found, delay $delta_sec seconds"
    return $delta_sec
}

#============================== Miscellaneous procs ============================

UTF::doc {
    # [call [cmd btamplib::check_embedded] [arg ip]]

    # Checks for 4325, 4329 & 4330 devices under test. Sets variable ::list_embedded
    # with list of hosts found.
}

proc btamplib::check_embedded {} {

    # We may have already run.
    if {[info exists ::list_embedded]} {
        return
    }

    # Check dut & ref1,2 for 4325 & 4329 devices.
    # In case of private builds, also look at the nvram & sta for clues.
    set ::list_embedded ""
    foreach host "$::btamp_dut $::btamp_ref1 $::btamp_ref2" {
        set dongle_image ""
        set catch_resp [catch "set dongle_image \[$host cget -dongleimage\]" catch_msg]
        set dongle_nvram ""
        set catch_resp [catch "set dongle_nvram \[$host cget -nvram\]" catch_msg]
        set dongle_sta ""
        set catch_resp [catch "set dongle_sta \[$host cget -sta\]" catch_msg]
        UTF::Message LOG $host "image=$dongle_image nvram=$dongle_nvram sta=$dongle_sta"
        if {[regexp {4325|4329|4330} $dongle_image] || [regexp {4325|4329|4330} $dongle_nvram] ||\
            [regexp {4325|4329|4330} $dongle_sta]} {
            lappend ::list_embedded $host
        }
    }
    UTF::Message LOG $::localhost "check_embedded list_embedded=$::list_embedded"
}

UTF::doc {
    # [call [cmd btamplib::testSetup] [arg ip]]

    # Does setup at start of each testcase for host [arg ip].
}

proc btamplib::testSetup { ip } {
    set ip [string trim $ip]
    set type [UTF::check_host_type $ip]
    UTF::Message LOG $ip "TC$::tc_num $::tc_title"
    UTF::Message LOG $ip "DUT=$::btamp_dut Ref1=$::btamp_ref1 Ref2=$::btamp_ref2"
    UTF::Message LOG $ip "================ btamplib::testSetup starting $ip $type"

    # If necessary, cleanup from last testcase that ran. If a PC crashed, or
    # UTF found an error message, the testcase may not have completed normally
    # by calling tc_result. When tc_result is called, the tc_state variable is
    # set back to null. If tc_state has any data at this point, it is the tc_num
    # of the testcase that just abended, and we now add it to the tc_fail_list.

    # NB: Testcases may not run in consecutive order, eg 1 3 8, so we had to save
    # the actual testcase number that was running in tc_state. We cant take the 
    # current testcase number and compute what was the previous testcase that ran.

    # NB: This setup routine can get called multiple times per testcase, so dont
    # add our own testcase number to the fail list.
    if {$::tc_state != "" && $::tc_state != $::tc_num} {
        lappend ::tc_fail_list $::tc_state
        incr ::tc_fail
        UTF::Message LOG $ip "btamplib::testSetup previous TC $::tc_state did not\
            complete normally, added to tc_fail_list"
    }

    # Save the testcase number that is about to start.
    set ::tc_state $::tc_num

    # We check for a flag that in the config file to indicate that we should not
    # send "wl down" commands to the driver. If not found, we also check the 
    # wl capability set for nodown.
    set var "::${ip}_nodown"
    if {[info exists $var]} {
        set nodown [set $var]
        UTF::Message LOG $ip "btamplib::testSetup found nodown=$nodown"
    } else {
        set capabilities [btamplib::run_cmd $ip "$::loc_wl cap"]
        set nodown 0
        if {[regexp "nodown" $capabilities]} {
            set nodown 1
            UTF::Message LOG $ip "btamplib::testSetup found nodown device capabilities=$capabilities"
        }
        set $var $nodown ;# save for future TC
    }

    # For FAST=1, dont reset the wl driver at start of each testcase.
    if {$::FAST == 0} {
        btamplib::driverControl $ip disable
        UTF::Sleep 2 "$ip" "be nice to 43225 (1)"
        btamplib::driverControl $ip enable
        UTF::Sleep 2 "$ip" "be nice to 43225 (2)"
    }
    UTF::Message LOG $ip "btamplib::testSetup done driverControl"

    # For Windows, we expect the remote host to be running DebugView.
    if {$type == "Cygwin" || $type == "WinDHD"} {
        btamplib::waitDebugView $ip
    }

    # Get current line number of DebugView or Linux messages logfile.
    btamplib::getMessagesLogLineCnt $ip

    # We check for a flag in the config file to indicate that some commands are
    # send via dhd instead of wl. It is the embedded devices that need this.
    set var "::${ip}_use_dhd"
    if {[info exists $var]} {
        set use_dhd [set $var]
        UTF::Message LOG $ip "btamplib::testSetup found dhd device use_dhd=$use_dhd"
    } else {
        set use_dhd 0
    }

    # Setup wl / dhd
    if {$nodown == 1} {
        UTF::Message LOG $ip "btamplib::testSetup skipping wl down"
    } else {
        btamplib::run_cmd $ip "$::loc_wl down"
    }
    btamplib::run_cmd $ip "$::loc_wl mpc 0"
    btamplib::run_cmd $ip "$::loc_wl btamp 1" -noerror ;# being discontinued
    if {$nodown == 1} {
        UTF::Message LOG $ip "btamplib::testSetup skipping wl up"
    } else {
        btamplib::run_cmd $ip "$::loc_wl up"
    }
    btamplib::run_cmd $ip "$::loc_wl btamp_flags 0"
    if {$use_dhd == 1} {
        # embedded devices do msglevel differently
        btamplib::run_cmd $ip "$::loc_wl msglevel +assoc"
        btamplib::run_cmd $ip "$::loc_dhd msglevel +bta"

    } else {
        # All other devices
        btamplib::run_cmd $ip "$::loc_wl msglevel +bta +assoc"
    }

    # Get and log the wl driver version being used.
    btamplib::run_cmd $ip "$::loc_wl ver"

    # Hack for 4329 issue PR79593. We need to reset the 4329 dbgview events after each of the other
    # devices is reset, so we check all test devices each time.
    btamplib::check_embedded
    foreach host "$::list_embedded" {
        UTF::Message LOG $host "PR79593 - ignore extra embedded events during setup"
        UTF::Sleep 5 ;# wait for slow 4329 events
        btamplib::getMessagesLogLineCnt $host "messages" "-noerror"
    }

    # btamplib::run_cmd $ip "$::loc_wl status" -noerror
    # btamplib::run_cmd $ip "$::loc_wl channel" -noerror
    # btamplib::run_cmd $ip "$::loc_wl disassoc" -noerror
    UTF::Message LOG $ip "================ btamplib::testSetup end $ip $type"
    return
}

UTF::doc {
    # [call [cmd btamplib::collect_info] [arg args]]

    # This proc is used to collect RSSI and chanspec info from the
    # hosts specified by [arg args]. Returns RSSI.
}

proc btamplib::collect_info { args } {
    UTF::Message LOG $::localhost "btamplib::collect_info ======== args=$args"
    set result ""
    foreach host $args {
        set rssi [btamplib::run_cmd $host "$::loc_wl rssi" -noerror]
        lappend result $rssi
        btamplib::run_cmd $host "$::loc_wl chanspec" -noerror
    }

    # Return comma separated list of RSSI.
    regsub -all " " $result "," result
    return $result
}

UTF::doc {
    # [call [cmd btamplib::driverControl] [arg ip] [arg cmd]]

    # This routine sends [arg cmd] to devcon.exe on host [arg ip].
}

proc btamplib::driverControl {ip cmd} {

    # Get UTF sta name, device & type
    set catch_resp [catch "set temp \[$ip cget -sta\]" catch_msg]
    if {$catch_resp != 0} {
        btamplib::logError $ip "btamplib::driverControl cget -sta got: $catch_msg"
        return
    }
    set sta [lindex $temp 0]
    set dev [lindex $temp 1]
    set type [UTF::check_host_type $ip]
    UTF::Message LOG $ip "btamplib::driverControl cmd=$cmd sta=$sta dev=$dev type=$type"

    # We try to get the pci_id for the specific host from the config file.
    set ip [string trim $ip]
    set var "::${ip}_pci_id"
    if {[info exists $var]} {
        set pci_id [set $var]
    } else {
        set pci_id ""
    }

    # mc13tst4 now has remote KVM, no real need for exception handling here.

    # If we got the pci_id from the config file, run devcon.
    set pci_id [string trim $pci_id]
    if {$pci_id != ""} {
        regsub -all {\\} $pci_id {\\\\\\\\} pci_id ;# need lots of backslashes
        UTF::Message LOG $ip "btamplib::driverControl ip=$ip from config file pci_id=$pci_id cmd=$cmd"
        btamplib::run_cmd $ip "c:/devcon.exe $cmd $pci_id"
        UTF::Sleep 1 $ip
        return
    }

    # Cygwin related objects are handled one way.
    if {$type == "Cygwin" || $type == "WinDHD"} {
        $sta devcon $cmd
        UTF::Sleep 1 $ip
        return
    }

    # Linux related objects.
    # NB: Initially unload/reload was done, but that causes all BTAMP settings to be lost,
    # and scanning wont occur, resulting in Physical Link not being set up.
    set var "::${ip}_nodown"
    if {[info exists $var]} {
        set nodown [set $var]
    } else {
        set nodown 0
    }
    if {$cmd == "disable"} {
        catch "$sta wl disassoc"
        if {$nodown == 0} {
            catch "$sta wl down"
        }
        return

    } elseif {$cmd == "enable"} {
        if {$nodown == 0} {
            catch "$sta wl up"
        }
        return

    } else {
        logError $ip "btamplib::driverControl type=$type unsupported cmd=$cmd"
        return
    }
}

UTF::doc {
    # [call [cmd btamplib::findProcessByName] [arg ip] [arg ps_name]
    # [opt args=null]]

    # Logs the [arg ps_name] process details found on host [arg ip].
    # [arg args] can be -nowarn and/or -noerror [para]

    # Returns the integer count of [arg ps_name] processes.
}

proc btamplib::findProcessByName {ip ps_name args} {

    # Clean up & parse calling args
    set args [string trim $args]
    set args [string tolower $args]
    set nowarn ""
    if {[string match *-nowarn* $args]} {
        set nowarn "-nowarn"
    }
    set noerror ""
    if {[string match *-noerror* $args]} {
        set noerror "-noerror"
    }
    # puts "nowarn=$nowarn noerror=$noerror"

    # Get process list on remote host.
    set ps_list [UTF::get_pid_list $ip]

    # Count the number of processes that match the specified ps_name.
    set ps_list [split $ps_list "\n"]
    set cnt 0
    set results ""
    foreach line $ps_list {
        # puts "$ip line=$line"
        if {[string match -nocase *$ps_name* $line]} {
            incr cnt
            if {$cnt == 1} {
                append results "$line" ;# dont add newline for first line
            } else {
                append results "\n$line" ;# add newline for subsequent lines
            }
        }
    }

    # Return the matching process count.
    if {$cnt > 0} {
        UTF::Message LOG $ip "btamplib::findProcessByName $ip $ps_name found $cnt process:\n$results"

    } else {
        if {$nowarn != "-nowarn"} {
            btamplib::logWarning $ip "btamplib::findProcessByName no $ps_name process exists!"
        }
    }
    return $cnt
}

UTF::doc {
    # [call [cmd btamplib::getMacAddr] [arg host]]

    # Returns the mac address for the wireless connection on [arg host]. 
    # [arg host] can be a STA object or a host object. Result is also stored
    # in a variable called ::[arg host]_macaddr which is cached for faster
    # access on subsequent calls.[para]

    # If you define the variable ::[arg host]_macaddr  in your config file, 
    # it will be used directly without checking the actual STA hardware. While
    # the scripts will run a bit faster there is the issue of remembering to
    # keep the variable up to date when you change your STA hardware.
}

proc btamplib::getMacAddr { host } {

    # Get hostname for host, as it may be a sta object.
    set host [UTF::get_name $host]

    # Get the first name in hosts sta list.
    set catch_resp [catch "set sta \[$host cget -sta\]" catch_msg]
    if {$catch_resp != 0} {
        error "btamplib::getMacAddr host=$host $catch_msg"
    }
    set sta [lindex $sta 0]
    # puts "host=$host sta=$sta"

    # We may already have a cached value. If so, use it.
    set var_name "::${host}_macaddr"
    if {[info exists $var_name]} {
        set temp [set $var_name]
        # puts "$host found cached MAC value $temp"
        regsub -all {:} $temp "" temp
        regsub -all {\-} $temp "" temp
        # puts "$host found cached MAC value $temp"
        set $var_name $temp
        return $temp
    }

    # Get the mac address of the sta and cache it.
    set macaddr [$sta macaddr]
    # puts "macaddr=$macaddr"
    regsub -all {:} $macaddr "" macaddr
    regsub -all {\-} $macaddr "" macaddr
    # puts "macaddr=$macaddr"
    set $var_name $macaddr
    return $macaddr
}

UTF::doc {
    # [call [cmd btamplib::run_cmd] [arg ip] [arg cmd] [opt args=null]]

    # On host [arg ip], runs command [arg cmd] via UTF rexec and catches errors.
    # Used to keep the higher level scripts from dying on an error.[para]

    # Valid args:
    # -no -noerror means dont log the error as an error, log as info.
    # -q  -quiet means dont log the commandline.
    # -s  -silent means dont log the output.
    # -w  -warn means dont log the error as an error, log as a warning.[para]

    # Returns cmd output.
}

proc btamplib::run_cmd {ip cmd args} {

    # For Linux objects, swap the Windows loc_wl to linux_wl, same for dhd.
    set type [UTF::check_host_type $ip]
    if {$type != "Cygwin" && $type != "WinDHD"} {
        regsub $::loc_wl $cmd  $::linux_wl cmd
        regsub $::loc_dhd $cmd  $::linux_dhd cmd
    }

    # Parse the args, if any.
    set noerror 0
    set options ""
    set quiet 0
    set silent 0
    set warn 0
    if {[regexp -nocase {\-no} $args]} {
        set noerror 1
    }
    if {[regexp -nocase {\-q} $args]} {
        lappend options "-quiet"
        set quiet 1
    }
    if {[regexp -nocase {\-s} $args]} {
        lappend options "-silent"
        set silent 1
    }
    if {[regexp -nocase {\-w} $args]} {
        set warn 1
    }

    # To accomodate the embedded devices, selected wl commands get redirected to the
    # dhd.exe command. First see if we are dealing with one of these devices.
    set var "::${ip}_use_dhd"
    if {[info exists $var]} {
        set use_dhd [set $var]
    } else {
        set use_dhd 0
    }
    # UTF::Message LOG $ip "btamplib::run_cmd found ip=$ip use_dhd=$use_dhd"

    # When appropriate, "wl HCI_cmd", "wl HCI_ACL_data" need to use dhd instead.
    if {$use_dhd == 1 && ([regexp -nocase {wl.*HCI_cmd} $cmd] ||\
        [regexp -nocase {wl.*HCI_ACL_data} $cmd])} {
        regsub "wl" $cmd "dhd" cmd

    }

    # Linux dhd also requires the "-i ethN" option for all commands.
    if {$use_dhd == 1 && ($type != "Cygwin" && $type != "WinDHD")} {
        set sta [$ip cget -sta]
        set dev [lindex $sta 1]
        regsub "dhd" $cmd "dhd -i $dev" cmd
    }

    # puts "noerror=$noerror quiet=$quiet silent=$silent options=$options cmd=$cmd"

    # Now run the cmd
    set catch_resp [catch "$ip rexec $options \"$cmd\"" catch_msg]
    if {$catch_resp != 0 && $noerror == 0 && $warn == 0} {
        btamplib::logError "$ip" "btamplib::run_cmd args=$args cmd=$cmd\
            catch_msg=$catch_msg"

    } elseif {$catch_resp != 0 && $noerror == 0 && $warn == 1} {
        btamplib::logWarning "$ip" "\n\n\nbtamplib::run_cmd args=$args cmd=$cmd\
            catch_msg=$catch_msg\n\n\nEND WARN++\n\n\n"
    }
    return "$catch_msg"
}

UTF::doc {
    # [call [cmd btamplib::restore_qtp_build] [arg ip]]

    # On host [arg ip] restores the original build from Ray Hayes,
    # which was used for certification. Build is assumed to be in:
    # c:\\wl_backup. Build is valid only on XP PC.
}

proc btamplib::restore_qtp_build {ip} {
    UTF::Message LOG $ip "btamplib::restore_qtp_build ip=$ip"
    btamplib::run_cmd $ip "erase /F /Q c:\\tools\\win32\\bin\\wl.exe"
    btamplib::run_cmd $ip "copy /Y c:\\WL_Backup\\wl.exe c:\\tools\\win32\\bin"
    btamplib::run_cmd $ip "erase /F /Q c:\\Windows\\system32\\drivers\\bcmwl5.sys"
    btamplib::run_cmd $ip "copy /Y c:\\WL_Backup\\bcmwl5.sys c:\\Windows\\system32\\drivers"
    return
}

#============================== Testcase Logging procs ===========================

UTF::doc {
    # [call [cmd btamplib::tc_result]]

    # Shows overall testcase result, maintains testcase counters.
}

proc btamplib::tc_result { } {

    # Add log trace info.
    UTF::Message LOG "" "btamplib::tc_result TC$::tc_num $::tc_errors errors" 

    # Debug info for benefit of 4329.
    # foreach ip "$::btamp_dut $::btamp_ref1 $::btamp_ref2" {
    #    btamplib::run_cmd $ip "$::loc_wl status" -noerror
    #    btamplib::run_cmd $ip "$::loc_wl channel" -noerror
    # }

    # The testcase completed normally by calling tc_result.
    # Set the tc_state back to null.
    set ::tc_state ""

    # Check for testcase errors.
    if {$::tc_errors == 0} {
        incr ::tc_pass
        # UTF translates return to PASS.
        return

    } else {
        incr ::tc_fail
        lappend ::tc_fail_list $::tc_num ;# these TC will be retried
        # The errors thrown from here will go onto the main UTF summary web page.
        # In the case of many errors, it is not useful to display them all on the
        # web page as it just fills it with junk making it difficult to read. The
        # compromise is to display a few errors, which will ensure the user sees
        # there was a problem. The user can look at the detailed log to see all the
        # error in the context of the testcase.
        if {$::tc_errors == 1} {
            set msg "1 error: $::error_list"
        } elseif {$::tc_errors <= 3} {
            set msg "$::tc_errors errors: $::error_list"
        } else {
            set msg "$::tc_errors errors: [lrange $::error_list 0 2] ..." ;# limit to 3 errors
        }

        # Append known PR numbers to failed TC.
        if {$::tc_num == 42} {
            append msg " PR77204"
        }

        # Always reset tc_errors & error_list in preparation for the next testcase.
        set ::tc_errors 0
        set ::error_list ""
        error "$msg"
    }
}

UTF::doc {
    # [call [cmd btamplib::logError] [arg ip] [arg msg]]

    # Common routine for logging & counting errors.
}

proc btamplib::logError {ip msg} {
    incr ::errors ;# overall total error count
    incr ::tc_errors ;# testcase level error count
    UTF::Message ERROR $ip " "
    UTF::Message ERROR $ip "@@@@ $msg"
    UTF::Message ERROR $ip " "
    regsub -all {\n} $msg "" msg

    # There have been cases of some errors being many pages of
    # text long. These errors were getting pushed onto the main  
    # web summary page and the summary email, which made a real
    # mess. So we limit the error_list to 200 charactars per msg.
    # The tc_result routine then only shows the first 3 msgs max
    # on the web page & email, which limits the damage.
    lappend ::error_list [string range $msg 0 199]
    return
}

UTF::doc {
    # [call [cmd btamplib::logWarning] [arg ip] [arg msg]]

    # Common routine for logging & counting warnings.
}

proc btamplib::logWarning {ip msg} {
    incr ::warnings
    set msg "++WARN: $::tc_num $::tc_title $ip $msg"
    UTF::Message WARN $ip "$msg"
    append ::warning_list "$msg\n"
    return
}

#============================== Testcase Preamble Procs ==========================

UTF::doc {
    # [call [cmd btamplib::hciPreamble514Init] [arg host]]

    # Does HCI Reset and EventMaskPage2 on [arg host].
}

proc btamplib::hciPreamble514Init {host} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.4

    UTF::Message LOG $host "================ btamplib::hciPreamble514Init\
        starting host=$host"   

    # Send HCI Reset command. See HCI vol2 parts D/E section 7.3.2
    btamplib::run_cmd $host "$::loc_wl HCI_cmd Reset"

    # HCI Event Mask Page 2 command is now required to be here.
    btamplib::run_cmd $host "$::loc_wl HCI_cmd Set_Event_Mask_Page_2\
        0xff 0x1f 0x00 0x00 0x00 0x00 0x00 0x00"
    UTF::Message LOG $host "================ btamplib::hciPreamble514Init end host=$host"
    return
}

UTF::doc {
    # [call [cmd btamplib::hciPreamble5151PhyLink] [arg IUT] [arg LT1]
    # [opt physical_link_handle=0x10]]

    # Preamble to set up a physical link from host [arg IUT] to host [arg LT1].
}

proc btamplib::hciPreamble5151PhyLink {IUT LT1 {physical_link_handle 0x10}} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.5.1

    # Get mac_addr for both hosts
    set iutMac [btamplib::getMacAddr $IUT]
    set lt1Mac [btamplib::getMacAddr $LT1]
    UTF::Message LOG $IUT "======================== btamplib::hciPreamble5151PhyLink\
        starting IUT=$IUT iutMac=$iutMac LT1=$LT1 lt1Mac=$lt1Mac\
        physical_link_handle=$physical_link_handle"

    # Test spec doesnt explicity say to sent HCI Reset & Event Mask Page 2,
    # it just say IUT starts in disconnected state, which implies the above
    # items are done. Use preamble 514.
    btamplib::hciPreamble514Init $IUT
    btamplib::hciPreamble514Init $LT1

    # Read Local AMP Info & Assoc are covered by preamble 5153
    btamplib::hciPreamble5153Read $IUT 0x00
    btamplib::hciPreamble5153Read $LT1 0x00 

    # Continue setting up physical link
    # puts "::loc_wl=$::loc_wl"
    btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Create_Physical_Link\
        [btamplib::composeHciCreatePhysicalLinkHexCmd $physical_link_handle]"
    btamplib::run_cmd $LT1 "$::loc_wl HCI_cmd Accept_Physical_Link_Request\
        [btamplib::composeHciCreatePhysicalLinkHexCmd $physical_link_handle]"
    btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Write_Remote_AMP_ASSOC\
        [btamplib::composeHciWriteRemoteAmpAssocHexCmd $lt1Mac $physical_link_handle\
        $::country $::ch_low $::ch_high]"
    btamplib::run_cmd $LT1 "$::loc_wl HCI_cmd Write_Remote_AMP_ASSOC\
        [btamplib::composeHciWriteRemoteAmpAssocHexCmd $iutMac $physical_link_handle\
        $::country $::ch_low $::ch_high]"
    # Wait for write remote amp assoc to complete before doing read local amp assoc.
    UTF::Sleep 5 $IUT
    btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Read_Local_AMP_ASSOC\
        $physical_link_handle 0 0 0 0"
    UTF::Message LOG $IUT "======================== btamplib::hciPreamble5151PhyLink end"
    return
}

UTF::doc {
    # [call [cmd btamplib::hciPreamble5152LogLink] [arg IUT] [arg LT1]
    # [opt physical_link_handle=0x10] [opt QOS=BE] [opt args=null]]

    # Preamble to set up a physical link from host [arg IUT] to host [arg LT1]
    # and then a logical link from host [arg IUT] to host [arg LT1].[para]

    # If args=-symmetric, then both hosts will perform all the 
    # necessary protocol steps to setup the logical link.
    # If args=-noiutinit, then the IUT initialization is skipped.
    # If args=-nolt1init, then the LT1 initialization is skipped.
}

proc btamplib::hciPreamble5152LogLink {IUT LT1 {physical_link_handle 0x10} {QOS BE} args} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.5.2

    # Get mac_addr for both hosts
    set iutMac [btamplib::getMacAddr $IUT]
    set lt1Mac [btamplib::getMacAddr $LT1]

    # Clean up args
    set args [string trim $args]
    set args [string tolower $args]
    regsub -all {\{} $args "" args
    regsub -all {\}} $args "" args
    UTF::Message LOG $IUT "======================== btamplib::hciPreamble5152LogLink\
        starting IUT=$IUT iutMac=$iutMac LT1=$LT1 lt1Mac=$lt1Mac\
        physical_link_handle=$physical_link_handle QOS=$QOS args=$args"

    # There are some minor differences from 5151, eg, dont send ReadLocalAmpAssoc.
    # So we replicate a minor variation of 5151 to faithfully follow the test
    # spec.

    # Test spec doesnt explicity say to send HCI Reset & Event Mask Page 2,
    # it just say IUT starts in disconnected state, which implies the above
    # items are done. Use preamble 514. When this preamble is used to setup
    # multiple links, we need to be able to selectively skip the init sequences
    # for the second and subsequent links.
    if {![string match "*-noiutinit*" $args]} {
        btamplib::hciPreamble514Init $IUT
    }
    if {![string match "*-nolt1init*" $args]} {
        btamplib::hciPreamble514Init $LT1
    }

    # Read Local AMP Info only. 
    btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Read_Local_AMP_Info"
    btamplib::run_cmd $LT1 "$::loc_wl HCI_cmd Read_Local_AMP_Info"

    # Continue setting up physical link.
    btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Create_Physical_Link\
        [btamplib::composeHciCreatePhysicalLinkHexCmd $physical_link_handle]"
    btamplib::run_cmd $LT1 "$::loc_wl HCI_cmd Accept_Physical_Link_Request\
        [btamplib::composeHciCreatePhysicalLinkHexCmd $physical_link_handle]"
    btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Write_Remote_AMP_ASSOC\
        [btamplib::composeHciWriteRemoteAmpAssocHexCmd $lt1Mac $physical_link_handle\
        $::country $::ch_low $::ch_high]"
    btamplib::run_cmd $LT1 "$::loc_wl HCI_cmd Write_Remote_AMP_ASSOC\
        [btamplib::composeHciWriteRemoteAmpAssocHexCmd $iutMac $physical_link_handle\
         $::country $::ch_low $::ch_high]"
    # Wait for write remote amp assoc to complete before doing read local amp assoc.
    UTF::Sleep 5 $IUT
    if {[string match "*-symmetric*" $args]} {
        # ReadLocalAmpAssoc on both hosts
        btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Read_Local_AMP_ASSOC\
            $physical_link_handle 0 0 0 0"
        btamplib::run_cmd $LT1 "$::loc_wl HCI_cmd Read_Local_AMP_ASSOC\
            $physical_link_handle 0 0 0 0"
    } else {
        # ReadLocalAmpAssoc only on IUT
        btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Read_Local_AMP_ASSOC\
            $physical_link_handle 0 0 0 0"
    }

    # Setup a Logical Link based on QOS parameter.
    if {[string match "*-symmetric*" $args]} {
        # CreateLogicalLink on both hosts
        btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Create_Logical_Link\
            [btamplib::composeHciCreateLogicalLinkHexCmd $physical_link_handle $QOS]"
        btamplib::run_cmd $LT1 "$::loc_wl HCI_cmd Create_Logical_Link\
            [btamplib::composeHciCreateLogicalLinkHexCmd $physical_link_handle $QOS]"
    } else {
        # CreateLogicalLink only on IUT
        btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Create_Logical_Link\
            [btamplib::composeHciCreateLogicalLinkHexCmd $physical_link_handle $QOS]"
    }
    UTF::Message LOG $IUT "======================== btamplib::hciPreamble5152LogLink end"
    return
}

UTF::doc {
    # [call [cmd btamplib::hciPreamble5153Read] [arg host]
    # [arg physical_link_handle]]

    # On [arg host], does HCI Read Local Amp Info & Read Local AMP ASSOC
    # for [arg physical_link_handle]
}

proc btamplib::hciPreamble5153Read {host physical_link_handle} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.5.3

    UTF::Message LOG $host "================ btamplib::hciPreamble5153Read starting host=$host\
        physical_link_handle=$physical_link_handle"

    # Send HCI Read Local Amp Info
    btamplib::run_cmd $host "$::loc_wl HCI_cmd Read_Local_AMP_Info"

    # Send HCI Read Local Amp Assoc
    btamplib::run_cmd $host "$::loc_wl HCI_cmd Read_Local_AMP_ASSOC $physical_link_handle 0 0 0 0"
    UTF::Message LOG $host "================ btamplib::hciPreamble5153Read end"
    return
}

UTF::doc {
    # [call [cmd btamplib::hciPreamble5154PhyLink2LogLink] [arg IUT] [arg LT1]
    # [arg LT2] [opt physical_link_handle1=0x10] [opt physical_link_handle2=0x11] 
    # [opt QOS1=BE] [opt QOS2=BE] [opt args=null]]

    # Preamble to setup a physical link and logical link from host [arg IUT] to
    # host [arg LT1] using [opt physical_link_handle1] and [opt QOS1],
    # followed by a physical link and logical link from host [arg IUT] to
    # host [arg LT2] using [opt physical_link_handle2] and [opt QOS2].[para]

    # If args=-symmetric, then all hosts perform all the necessary protocol
    # steps to setup the logical links.
}

proc btamplib::hciPreamble5154PhyLink2LogLink {IUT LT1 LT2 {physical_link_handle1 0x10}\
    {physical_link_handle2 0x11} {QOS1 BE} {QOS2 BE} args} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.5.4, p22
    # There is a limitation within BlueTooth of only 1 PLH per connected pair.
    # So, to get 2 physical links, they have to go to different destinations.
    UTF::Message LOG "$::localhost" "================================\
        btamplib::hciPreamble5154PhyLink2LogLink starting IUT=$IUT LT1=$LT1\
        LT2=$LT2 physical_link_handles: $physical_link_handle1 $physical_link_handle2\
        QOS: $QOS1 $QOS2 args=$args"

    # Use existing building blocks
    btamplib::hciPreamble5152LogLink $IUT $LT1 $physical_link_handle1 $QOS1 $args
    btamplib::hciPreamble5152LogLink $IUT $LT2 $physical_link_handle2 $QOS2 $args -noiutinit

    UTF::Message LOG "$::localhost" "================================\
        btamplib::hciPreamble5154PhyLink2LogLink end"
    return
}

UTF::doc {
    # [call [cmd btamplib::hciPreamble5155LogLink2] [arg IUT] [arg LT1]
    # [opt physical_link_handle=0x11] [opt QOS=GU] [opt args=null]]

    # Preamble to set up a physical link from host [arg IUT] to host [arg LT1]
    # and then two logicals link from host [arg IUT] to host [arg LT1].
    # The first logical link has QOS=BE, while the second logical link
    # has QOS=[opt QOS], default=GU.[para]

    # If args=-symmetric, then all hosts perform all the necessary protocol
    # steps to setup the logical links.
}

proc btamplib::hciPreamble5155LogLink2 {IUT LT1 {physical_link_handle 0x10} {QOS GU} args} {
    # Preamble to set up a BE logical link and a second logical link, type=QOS
    # Reference: BT 802.11 PAL Test Specification section 5.1.5.5
    # If args=-symmetric, then both hosts will perform all the 
    # necessary protocol steps to setup the logical link.
    UTF::Message LOG "$::localhost" "========================\
        btamplib::hciPreamble5155LogLink2 start IUT=$IUT LT1=$LT1\
        physical_link_handle=$physical_link_handle QOS=$QOS args=$args"

    # Use existing building blocks. First logical link is always QOS=BE.
    btamplib::hciPreamble5152LogLink $IUT $LT1 $physical_link_handle BE $args
    btamplib::hciPreambleLogLink     $IUT $LT1 $physical_link_handle $QOS $args

    UTF::Message LOG "$::localhost" "========================\
        btamplib::hciPreamble5155LogLink2 end"
    return
}

UTF::doc {
    # [call [cmd btamplib::hciPreambleLogLink] [arg IUT] [arg LT1]
    # [opt physical_link_handle=0x10] [opt QOS=BE] [opt args]]

    # Preamble to set up a logical link from host [arg IUT] to host [arg LT1]
    # using an already existing physical link.[para]

    # If args=-symmetric, then both hosts perform all the necessary protocol
    # steps to setup the logical links.
}

proc btamplib::hciPreambleLogLink {IUT LT1 {physical_link_handle 0x10} {QOS BE} args} {

    # Clean up args
    set args [string trim $args]
    set args [string tolower $args]
    regsub -all {\{} $args "" args
    regsub -all {\}} $args "" args
    UTF::Message LOG $IUT "================ btamplib::hciPreambleLogLink\
        starting IUT=$IUT LT1=$LT1 physical_link_handle=$physical_link_handle\
        QOS=$QOS args=$args"

    # Setup logical link.
    if {[string match "*-symmetric*" $args]} {
        # Setup logical link on both hosts
        btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Create_Logical_Link\
            [btamplib::composeHciCreateLogicalLinkHexCmd $physical_link_handle $QOS]"
        btamplib::run_cmd $LT1 "$::loc_wl HCI_cmd Create_Logical_Link\
            [btamplib::composeHciCreateLogicalLinkHexCmd $physical_link_handle $QOS]"
    } else {
        # Setup logical link only on IUT
        btamplib::run_cmd $IUT "$::loc_wl HCI_cmd Create_Logical_Link\
            [btamplib::composeHciCreateLogicalLinkHexCmd $physical_link_handle $QOS]"
    }
    UTF::Message LOG $IUT "================ btamplib::hciPreambleLogLink end"
    return
}

#============================== Preamble Parsing procs ===========================

UTF::doc {
    # [call [cmd btamplib::parseHciPreamble514Init] [arg host]]

    # Parses HCI commands & events generated by proc hciPreamble514Init
}

proc btamplib::parseHciPreamble514Init {host} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.4

    UTF::Message LOG $::log_ip "================ btamplib::parseHciPreamble514Init\
        starting $::log_ip"
    btamplib::parseHciReset

    # Command Hci Set_Event_Mask_Page_2 is now required to be here.
    btamplib::parseHciSetEventMaskPage2
    UTF::Message LOG $::log_ip "================ btamplib::parseHciPreamble514Init\
        end $::log_ip"
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciPreamble5151PhyLink] [arg host] [arg mac_addr]
    # [opt expected_status=0x0] [opt physical_link_handle=0x10]]

    # Parses HCI commands & events generated by proc hciPreamble5151PhyLink
}
proc btamplib::parseHciPreamble5151PhyLink {host mac_addr {expected_status 0x0}\
    {expected_physical_link_handle 0x10}} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.5.1

    # Items common for all hosts
    UTF::Message LOG $::log_ip "======================== btamplib::parseHciPreamble5151PhyLink\
        starting $::log_ip host=$host mac_addr=$mac_addr expected_status=$expected_status\
        expected_physical_link_handle=$expected_physical_link_handle"
    btamplib::parseHciPreamble514Init $host
    btamplib::parseHciPreamble5153Read 0x$mac_addr 0x00 ;#PLH does not exist yet

    # NB: Setting connection accept timeout is not in test spec, so dont do
    # parseHciWriteConnectionAcceptTimeout

    # The DUT / IUT does CreatePhysicalLink, other host does AcceptPhysicalLinkRequest.
    set host [string trim $host]
    set host [string toupper $host]
    if {$host == "DUT" || $host == "IUT"} {
        btamplib::parseHciCreatePhysicalLink
    } else {
        btamplib::parseHciAcceptPhysicalLinkRequest
    }

    # Items common for all hosts
    btamplib::parseHciWriteRemoteAmpAssoc $expected_physical_link_handle

    # Only the DUT / IUT is expected to get a ChannelSelected event.
    set host [string trim $host]
    set host [string toupper $host]
    if {$host == "DUT" || $host == "IUT"} {
        btamplib::parseHciChannelSelected $expected_physical_link_handle
    }

    # Items common for all hosts
    btamplib::parseHciPhysicalLinkComplete $expected_status $expected_physical_link_handle

    # Only the DUT / IUT does a ReadLocalAmpAssoc event.
    if {$host == "DUT" || $host == "IUT"} {
        set subband 1 ;# must have 1 sub-band triplet
        btamplib::parseHciReadLocalAmpAssoc 0x$mac_addr $expected_physical_link_handle $subband
    }

    # NB: Test plan implies that PhysicalLinkComplete is last
    # item to show up, but its not.
    UTF::Message LOG $::log_ip "========================\
        btamplib::parseHciPreamble5151PhyLink end $::log_ip"
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciPreamble5152LogLink] [arg host] [arg mac_addr]
    # [opt expected_status=0x00] [opt expected_physical_link_handle=0x10]
    # [opt expected_logical_link_handle=0x0000] [opt expected_tx_flow_spec_id=0x01]
    # [opt args]]

    # Parses HCI commands & events generated by proc hciPreamble5152LogLink
}

proc btamplib::parseHciPreamble5152LogLink {host mac_addr {expected_status 0x00}\
    {expected_physical_link_handle 0x10} {expected_logical_link_handle 0x0000}\
    {expected_tx_flow_spec_id 0x01} args} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.5.2
    # Parses HCI commands & events generated by proc hciPreamble5152LogLink

    # Many items are similar to preamble 5.1.5.1, but there are some differences,
    # such as only doing the ReadLocalAmpInfo at the start.

    # Check arg values
    set args [string trim $args]
    set args [string tolower $args]
    regsub -all {\{} $args "" args
    regsub -all {\}} $args "" args
    UTF::Message LOG $::log_ip "======================== btamplib::parseHciPreamble5152LogLink\
        starting $::log_ip host=$host mac_addr=$mac_addr\
        expected_status=$expected_status\
        expected_physical_link_handle=$expected_physical_link_handle\
        expected_logical_link_handle=$expected_logical_link_handle\
        expected_tx_flow_spec_id=$expected_tx_flow_spec_id args=$args"

    # We may need to selectively skip the init sequence.
    if {![string match "*-noiutinit*" $args] &&\
        ![string match "*-nolt\[12\]init*" $args]} {
        btamplib::parseHciPreamble514Init $host
    }

    # Items common for all hosts
    btamplib::parseHciReadLocalAmpInfo

    # The DUT / IUT does CreatePhysicalLink, other host does AcceptPhysicalLinkRequest.
    set host [string trim $host]
    set host [string toupper $host]
    if {$host == "DUT" || $host == "IUT"} {
        btamplib::parseHciCreatePhysicalLink 
    } else {
        btamplib::parseHciAcceptPhysicalLinkRequest
    }

    # Items common for all hosts
    btamplib::parseHciWriteRemoteAmpAssoc $expected_physical_link_handle

    # Only the DUT / IUT is expected to get a ChannelSelected event.
    set host [string trim $host]
    set host [string toupper $host]
    if {$host == "DUT" || $host == "IUT"} {
        btamplib::parseHciChannelSelected $expected_physical_link_handle
    }

    # Items common for all hosts
    btamplib::parseHciPhysicalLinkComplete 0x0 $expected_physical_link_handle

    # Only the DUT / IUT does a ReadLocalAmpAssoc event and
    # LogicalLinkCreation. If args=-symmetric, then both hosts
    # did all items.
    if {$host == "DUT" || $host == "IUT" || [string match "*-symmetric*" $args]} {
        set subband 1 ;# must have 1 sub-band triplet
        btamplib::parseHciReadLocalAmpAssoc 0x$mac_addr $expected_physical_link_handle $subband 0x00 "01 02 05" $host
        btamplib::parseHciCreateLogicalLink
        btamplib::parseHciLogicalLinkComplete $expected_status $expected_physical_link_handle\
            $expected_logical_link_handle $expected_tx_flow_spec_id
    }
    UTF::Message LOG $::log_ip "======================== btamplib::parseHciPreamble5152LogLink end"
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciPreamble5153Read] [arg mac_addr]
    # [opt physical_link_handle=0x10]]

    # Parses HCI commands & events generated by proc hciPreamble5153Read
}

proc btamplib::parseHciPreamble5153Read {mac_addr {expected_physical_link_handle 0x10}} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.5.3
    UTF::Message LOG $::log_ip "================ btamplib::parseHciPreamble5153Read\
        starting $::log_ip mac_addr=$mac_addr\
        expected_physical_link_handle=$expected_physical_link_handle"
    btamplib::parseHciReadLocalAmpInfo
    btamplib::parseHciReadLocalAmpAssoc $mac_addr $expected_physical_link_handle
    UTF::Message LOG $::log_ip "================ btamplib::parseHciPreamble5153Read\
        end $::log_ip"
    return
}

# There is no proc: parseHciPreamble5154PhyLink2LogLink
# The issue is that the parser works on one message file at a time.
# So, just call parseHciPreamble5152LogLink for each file. 

UTF::doc {
    # [call [cmd btamplib::parseHciPreamble5155LogLink2] [arg host] [arg mac_addr]
    # [opt expected_physical_link_handle=0x10] [opt expected_logical_link_handle1=0x0000]
    # [opt expected_logical_link_handle2=0x0001] [opt expected_tx_flow_spec_id1=0x01]
    # [opt expected_tx_flow_spec_id2=0x03] [opt args]]

    # Parses HCI commands & events generated by proc hciPreamble5155LogLink2
}

proc btamplib::parseHciPreamble5155LogLink2 {host mac_addr {expected_status 0x00}\
    {expected_physical_link_handle 0x10} {expected_logical_link_handle1 0x0000}\
    {expected_logical_link_handle2 0x0001} {expected_tx_flow_spec_id1 0x01}\
    {expected_tx_flow_spec_id2 0x03} args} {
    # Reference: BT 802.11 PAL Test Specification section 5.1.5.5 p24
    # Parses HCI commands & events generated by proc hciPreamble5155LogLink2
    UTF::Message LOG $::log_ip "================================\
        btamplib::parseHciPreamble5155LogLink2 starting $::log_ip\
        host=$host mac_addr=$mac_addr\
        expected_status=$expected_status\
        expected_physical_link_handle=$expected_physical_link_handle\
        expected_logical_link_handles: $expected_logical_link_handle1\
        $expected_logical_link_handle2\
        expected_tx_flow_spec_ids: $expected_tx_flow_spec_id1\
        $expected_tx_flow_spec_id2 args=$args"

    # Use existing building blocks
    btamplib::parseHciPreamble5152LogLink $host $mac_addr $expected_status\
        $expected_physical_link_handle $expected_logical_link_handle1\
        $expected_tx_flow_spec_id1 $args
    btamplib::parseHciPreambleLogLink $expected_status $expected_physical_link_handle\
        $expected_logical_link_handle2 $expected_tx_flow_spec_id2

    UTF::Message LOG $::log_ip "================================\
        btamplib::parseHciPreamble5155LogLink2 end $::log_ip"
    return
}

UTF::doc {
    # [call [cmd btamplib::hciPreambleLogLink] [opt expected_status=0x00]
    # [opt expected_physical_link_handle=0x10] [opt expected_logical_link_handle=0x0000]
    # [opt expected_tx_flow_spec_id=0x01]]

    # Parses HCI commands & events generated by proc hciPreambleLogLink
}

proc btamplib::parseHciPreambleLogLink {{expected_status 0x00}\
    {expected_physical_link_handle 0x10} {expected_logical_link_handle 0x0000}\
    {expected_tx_flow_spec_id 0x01}} {
    # Parses HCI commands & events generated by proc hciPreambleLogLink
    UTF::Message LOG $::log_ip "================ btamplib::parseHciPreambleLogLink\
        starting $::log_ip\
        expected_status=$expected_status\
        expected_physical_link_handle=$expected_physical_link_handle\
        expected_logical_link_handle=$expected_logical_link_handle\
        expected_tx_flow_spec_id=$expected_tx_flow_spec_id"

    # Use existing building blocks
    btamplib::parseHciCreateLogicalLink
    btamplib::parseHciLogicalLinkComplete $expected_status $expected_physical_link_handle\
        $expected_logical_link_handle $expected_tx_flow_spec_id

    UTF::Message LOG $::log_ip "================ btamplib::parseHciPreambleLogLink\
        end $::log_ip"
    return
}


#============================== HCI Parsing procs ===============================

# General notes:
# The specs are in Big endian, BRCM code is Little endian, so lots
# of bytes to be flipped...

# Reference: CR AMP HCI section 5.4.1 p57
# Opcode = 2 bytes
# Param_length = 1 byte
# HCI Command packet: Opcode Param_Length Param1 ... ParamN

# Reference: CR AMP HCI section 5.4.4 p62
# Event code = 1 byte
# Param_length = 1 byte
# HCI Event packet: Event_code Param_length Event1 ... EventN

# OGF = Opcode Group Field
# Info is scattered throughout the  documents
# CR AMP HCI section 7.1 p79: Link Control commands, OGF is 0x01
# Core V3.0+HS section 7.2 p504: Link Policy Commands, OGF is 0x02
# CR AMP HCI section 7.3 p93: Control and Baseband Commands, OGF is 0x03
# Core V3.0+HS section 7.4 p615: Informational Parameters Commands, OGF is 0x04
# Core V3.0+HS section 7.5 p624: Status Parameters Commands, OGF is 0x05
# Core V3.0+HS section 7.6 p644: Testing Commands, OGF is 0x06

UTF::doc {
    # [call [cmd btamplib::checkEol] [arg index]]

    # Used to check we have hit the end of the ::hci_event_data
}

proc btamplib::checkEol {index} {

    # Check calling token
    set index [string trim $index]
    if {$index == ""} {
        btamplib::logError $::log_ip "btamplib::checkEol got null index!"
        return
    }

    # Check specified location onwards is null.
    set hex [lrange $::hci_event_data $index end]
    set hex [string trim $hex]
    if {$hex == ""} {
        UTF::Message LOG $::log_ip "btamplib::checkEol got expected EOL at index=$index"
    } else {
        btamplib::logError $::log_ip "btamplib::checkEol got hex=$hex, expected EOL at index=$index"
    }
}

UTF::doc {
    # [call [cmd btamplib::checkHciCommand] [arg expected_cmd]]

    # Checks HCI Command Text is what we expected. [arg expected_cmd]
    # can be a regular expression.
}

proc btamplib::checkHciCommand {expected_cmd} {

    # Check HCI Command Text is what we expected.
    if {[regexp -nocase $expected_cmd $::hci_cmd_text]} {
        UTF::Message LOG $::log_ip "btamplib::checkHciCommand $::hci_cmd_text matched\
            $expected_cmd"
    } else {
        btamplib::logError $::log_ip "btamplib::checkHciCommand $::hci_cmd_text\
            DID NOT match $expected_cmd"
    }
    return
}

UTF::doc {
    # [call [cmd btamplib::checkHciEventCode] [arg expected_code]]

    # Checks the HCI Event Code is what we expected.
}

proc btamplib::checkHciEventCode {expected_code} {

    # Check HCI Event Code is what we expected.
    set expected_code [string trim $expected_code]
    set expected_code [string tolower $expected_code]
    if {$::hci_event_code == $expected_code} {
        UTF::Message LOG $::log_ip "btamplib::checkHciEventCode got\
            expected_code=$expected_code"
    } else {
        btamplib::logError $::log_ip "btamplib::checkHciEventCode\
            hci_event_code=$::hci_event_code NE $expected_code"
    }
    return
}

UTF::doc {
    # [call [cmd btamplib::checkHciEventText] [arg expected_text]]

    # Check HCI Event Text is what we expected. [arg expected_text]
    # can be a regular expression.
}

proc btamplib::checkHciEventText {expected_text} {

    # Check HCI Event Text is what we expected.
    if {[regexp -nocase $expected_text $::hci_event_text]} {
        UTF::Message LOG $::log_ip "btamplib::checkHciEventText $::hci_event_text matched $expected_text"
    } else {
        logError $::log_ip "btamplib::checkHciEventText $::hci_event_text\
            DID NOT match $expected_text"
    }
    return
}

UTF::doc {
    # [call [cmd btamplib::checkHciOpcode] [arg expected_OGF]
    # [arg expected_OCF] [opt byte1_offset=1]]

    # Gets data from ::hci_event_data & check against expected values.
}

proc btamplib::checkHciOpcode {expected_OGF expected_OCF {byte1_offset 1}} {
    # Gets data from ::hci_event_data & check against expected values.

    # Reference: CR AMP HCI section 5.4.1 p57
    # Opcode = 2 bytes
    #     OGF = Opcode Group Field, upper 6 bits of Opcode
    #     OCF = Opcode Command Field, lower 10 bits of Opcode
    # NB: Opcode is not always in the same place for all packets.

    # Get actual opcode bytes.
    set byte2_offset [expr $byte1_offset + 1]
    set actual_opcode "0x[lindex $::hci_event_data $byte2_offset][lindex $::hci_event_data $byte1_offset]"
    if {$actual_opcode == "0x"} {
        # handles case where there is no event data.
        set actual_opcode 0x0000
    }

    # Check actual_opcode for hex syntax.
    if {![btamplib::ishex $actual_opcode]} {
        btamplib::logError $::log_ip "btamplib::checkHciOpcode invalid actual_opcode=$actual_opcode"
        return
    }

    # Extract OGF, OCF.
    set actual_OGF [expr $actual_opcode & 0xfc00] ;# get upper 6 bits
    set actual_OGF [expr $actual_OGF >> 10]
    set actual_OCF [expr $actual_opcode & 0x03ff] ;# get lower 10 bits
    # puts "actual_opcode=$actual_opcode actual_OGF=$actual_OGF actual_OCF=$actual_OCF"

    # Check actual against expected
    set expected_OGF [string trim $expected_OGF]
    set expected_OGF [string tolower $expected_OGF]
    set expected_OCF [string trim $expected_OCF]
    set expected_OCF [string tolower $expected_OCF]
    if {$actual_OGF == $expected_OGF && $actual_OCF == $expected_OCF} {
        UTF::Message LOG $::log_ip "btamplib::checkHciOpcode actual_opcode=$actual_opcode\
             got expected_OGF=$expected_OGF expected_OCF=$expected_OCF\
             byte1_offset=$byte1_offset"
        return

    } else {
        btamplib::logError $::log_ip "btamplib::checkHciOpcode actual_opcode=$actual_opcode\
            actual_OGF=$actual_OGF actual_OCF=$actual_OCF NE\
            expected_OGF=$expected_OGF expected_OCF=$expected_OCF\
            byte1_offset=$byte1_offset"
        return
    }
}

UTF::doc {
    # [call [cmd btamplib::checkHciStatus] [arg expected_status_list]
    # [opt offset=3] [opt 00_mapping=Success]]

    # Gets actual status from ::hci_event_data based on [opt offset].
    # Checks actual status against list of values in [arg expected_status_list].
    # Some events have a different interpretation of 0x00, such as Pending instead
    # of Success. The [opt 00_mapping] allows status 0x00 to be redefined as
    # needed. 
}

proc btamplib::checkHciStatus {expected_status_list {offset 3} {00_mapping Success}} {
    # Gets actual status from ::hci_event_data based on offset.
    # Checks actual status against list of values in expected_status_list.

    # Get actual status and error message string.
    set actual_status "[lindex $::hci_event_data $offset]"
    set actual_status [btamplib::cleanupHex $actual_status]
    set actual_message [btamplib::translateHciStatus $actual_status $00_mapping]

    # Cleanup hex in expected_status_list
    set temp ""
    foreach hex $expected_status_list {
        lappend temp [btamplib::cleanupHex $hex]
    }
    set expected_status_list $temp

    # Check status against expected_status
    if {[lsearch -exact $expected_status_list $actual_status] >= 0} {
        UTF::Message LOG $::log_ip "btamplib::checkHciStatus offset=$offset actual_status=$actual_status\
            $actual_message MATCHED oneof expected_status_list=$expected_status_list"
        return

    } else {
        btamplib::logError $::log_ip "btamplib::checkHciStatus offset=$offset\
            actual_status=$actual_status $actual_message NOT oneof\
            expected_status_list=$expected_status_list"
        return
    }
}

UTF::doc {
    # [call [cmd btamplib::checkNumHciCmd] [arg expected_num_hci_cmd]
    # [opt byte_offset=0]]

    # Gets data from ::hci_event_data & checks against [arg expected_num_hci_cmd].
    # [opt byte_offset] allows for the issue that the data is not always
    # in the same place for all events.
}

proc btamplib::checkNumHciCmd {expected_num_hci_cmd {byte_offset 0}} {
    # Gets data from ::hci_event_data & check against expected_num.

    # Get actual num_hci_cmd.
    # Not always in same place for all packets....
    set actual_num_hci_cmd "[lindex $::hci_event_data $byte_offset]"
    set actual_mum_hci_cmd [string trim $actual_num_hci_cmd]

    # Check actual against expected
    set expected_num_hci_cmd [string trim $expected_num_hci_cmd]
    if {$actual_num_hci_cmd == $expected_num_hci_cmd} {
        UTF::Message LOG $::log_ip "btamplib::checkNumHciCmd byte_offset=$byte_offset\
            got expected_num_hci_cmd=$expected_num_hci_cmd"
        return

    } else {
        btamplib::logError $::log_ip "btamplib::checkNumHciCmd byte_offset=$byte_offset\
            actual_num_hci_cmd=$actual_num_hci_cmd NE\
            expected_num_hci_cmd=$expected_num_hci_cmd"
        return
    }
}

UTF::doc {
    # [call [cmd btamplib::composeHciCreateLogicalLinkHexCmd]
    # [opt physical_link_handle=0x10] [opt service_type=BE]]

    # Composes the CreateLogicalLink hex string for use in wl cmd.
    # [opt service_type] can be: BE, GU, XL, 00
}

proc btamplib::composeHciCreateLogicalLinkHexCmd {{physical_link_handle 0x10}\
    {service_type BE}} {

    # Composes the CreateLogicalLink hex string for use in wl cmd.

    # Reference: CR AMP HCI section 7.1.40, p87
    # OCF: 0x0038
    # Physical Link Handle = 1 byte
    # TX Flow Spec = 16 bytes
    # RX Flow Spec = 16 bytes

    # Flow spec: See Core_V3 + HS, Vol 3, Part A (Logical Link Control and Adaptation
    # Protocol aka L2CAP), section 5.6, p85
    # Table 5.4, p86 shows QOS parameter names, sizes & defaults:
    # Identifier = 1 byte, unique within physical link, default = 0x1
    # Service Type = 1 byte, default = BestEffort = 0x1, see Table 5.5
    # Max SDU size = 2 bytes, default = 0xFFFF
    # SDU Interarrival time = 4 bytes, default = 0xFFFFFFFF
    # Access Latency = 4 bytes, default = 0xFFFFFFFF
    # Flush Timeout = 4 bytes, default = 0xFFFFFFFF

    # Original string used by Joe
    # 0x10 1 1 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
    # 2 1 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
   
    # Initialization
    set service_type [string trim $service_type]
    set service_type [string toupper $service_type]
    set service ""
    set id1 ""
    set id2 ""
    set tx_spec ""
    set rx_spec ""

    # The specific ids & timeouts are specified in QTP, section 5.4, p54.

    # Set up Best Effort QOS profile (default)
    if {$service_type == "BE"} {
        set service 0x1
        set id1 0x1
        set id2 0x2
        set other "0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff"
        set tx_spec "$id1 $service $other"
        set rx_spec "$id2 $service $other"
    }

    # Set up Guaranteed QOS profile
    # See Core_V3 + HS, Vol 3, Part A (Logical Link Control and Adaptation,
    # Section 5.6 p87
    if {$service_type == "GU"} {
        set service 0x2 ;# Table 5.5 0x2 = guaranteed
        set id1 0x3
        set id2 0x4
        set max_sdu "0xd4 0x05" ;# 1492
        set sdu_time "0xff 0xff 0xff 0xff"
        set latency "0x10 0x27 0x00 0x00" ;# 10,000 microseconds
        # set flush "0xd0 0x07 0x00 0x00" ;# 2,000 microseconds
        # set flush "0x88 0x13 0x00 0x00" ;# 5,000 microseconds
        # set flush "0x58 0x16 0x00 0x00" ;# 7,000 microseconds
        set flush "0x10 0x27 0x00 0x00" ;# 10,000 microseconds
        set tx_spec "$id1 $service $max_sdu  $sdu_time $latency $flush"
        set rx_spec "$id2 $service 0xff 0xff $sdu_time $latency $flush"
    }

    # Set up XL profile that asks for too much bandwidth, used by TC27.
    # Ray says to use TX|RX_GU_BW_FS. TX_RX_GU_LARGE_FS are not defined
    # anywhere in the BTAMP QTP.
    if {$service_type == "XL"} {
        set service 0x2 ;# Table 5.5 0x2 = guaranteed
        set id1 0x4
        set id2 0x4
        set max_sdu "0xd4 0x05" ;# 1492
        set sdu_time "0x64 0x00 0x00 0x00" ;# 100
        set latency "0xff 0xff 0xff 0xff"
        set flush "0xff 0xff 0xff 0xff"
        set tx_spec "$id1 $service $max_sdu $sdu_time $latency $flush"
        set rx_spec "$id2 $service $max_sdu $sdu_time $latency $flush"
    }

    # PR76168: profile sends mostly 0's - got an assert
    if {$service_type == "00"} {
        set service 0x2 ;# Table 5.5 0x2 = guaranteed
        set id1 0x3
        set id2 0x4
        set max_sdu "0xd4 0x05" ;# 1492
        set sdu_time "0x00 0x00 0x00 0x00"
        set latency "0x00 0x00 0x00 0x00"
        set flush "0x00 0x00 0x00 0x00"
        set tx_spec "$id1 $service $max_sdu $sdu_time $latency $flush"
        set rx_spec "$id2 $service $max_sdu $sdu_time $latency $flush"
    }

    # Display parms, return hex_cmd
    UTF::Message LOG "$::localhost" "btamplib::composeHciCreateLogicalLinkHexCmd\
        physical_link_handle=$physical_link_handle\
        \nservice_type=$service_type service=$service id1=$id1 id2=$id2\
        \ntx_spec=$tx_spec \nrx_spec=$rx_spec"
    set hex_cmd "$physical_link_handle $tx_spec $rx_spec"
    return "$hex_cmd"
}

UTF::doc {
    # [call [cmd btamplib::composeHciFlowSpecModifyHexCmd]
    # [opt handle1=0x00] [opt handle2=0x00] [opt service_type=BE_AGG]]

    # Composes the FlowSpecModify hex string for use in wl cmd.
    # [opt service_type] can be: BE_AGG
}

proc btamplib::composeHciFlowSpecModifyHexCmd {{handle1 0x00} {handle2 0x00}\
    {service_type BE_AGG}} {

    # Reference: CR AMP HCI section 7.1.44 p92
    # HCI FlowSpecModify
    # Command parameters:
    #    Handle = 2 bytes, 12 bits significant
    #    TXFlowSpec = 16 bytes
    #    RXFlowSpec = 16 bytes

    # Initialization                   
    set tx_spec ""
    set rx_spec ""
    set service_type [string trim $service_type]
    set service_type [string toupper $service_type]

    # See BTAMP QTP section 5.4 p54
    if {$service_type == "BE_AGG"} {
        set service 0x1
        set id1 0x1 ;# QTP says use 0x4, but it you get errors trying to change id
        set id2 0x2 ;# QTP says use 0x4, but it you get errors trying to change id
        set max_sdu "0xff 0xff"
        set sdu_time "0xff 0xff 0xff 0xff"
        set latency "0xff 0xff 0xff 0xff"
        set flush "0xff 0xff 0xff 0xff"
        set tx_spec "$id1 $service $max_sdu $sdu_time $latency $flush"
        set rx_spec "$id2 $service $max_sdu $sdu_time $latency $flush"
    }

    # Display parms, return hex_cmd
    UTF::Message LOG "$::localhost" "btamplib::composeHciFlowSpecModifyHexCmd\
        handle=$handle1 $handle2 service_type=$service_type"
    set hex_cmd "$handle2 $handle1 $tx_spec $rx_spec"
    return "$hex_cmd"
}

UTF::doc {
    # [call [cmd btamplib::composeHciCreatePhysicalLinkHexCmd]
    # [opt physical_link_handle=0x10] [opt key_type=0x3]]

    # Composes the CreatePhysicalLink hex string for use in wl cmd.
}

proc btamplib::composeHciCreatePhysicalLinkHexCmd {{physical_link_handle 0x10}\
    {key_type 0x3}} {
    # Composes the CreatePhysicalLink hex string for use in wl cmd.

    # Reference: CR AMP HCI section 7.1.37 p82
    # OCF = 0x0035
    # Physical Link Handle = 1 byte
    # Key Length = 1 byte
    # Key Type = 1 byte
    #   3 = debug
    #   4 = unauthenticated
    #   5 = authenticated
    # AMP Key = Key Length bytes; see also Core V3.0 + HS, section 7.7.5
    # returns nothing

    # Original key used by Joe.
    # set original "0x10 32 3 0x4f 0x12 0x87 0x31 0xee 0x62 0xc0 0x68 0x3f\
    #    0x28 0x3b 0xd9 0x9d 0x74 0x1a 0x3d 0x3c 0x6d 0xa9 0xd4\
    #    0x98 0xf3 0xe5 0xf1 0xec 0xdd 0xab 0xfc 0xdd 0x06 0x4a 0xf6"
    # set orig_str [hex2ascii $original]
    # puts "orig_str=$orig_str"

    # Debug AMP key is numbers in sequence 0 - 31, nothing fancy.
    set amp_key "0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7\
        0x8 0x9 0xa 0xb 0xc 0xd 0xe 0xf\
        0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17\
        0x18 0x19 0x1a 0x1b 0x1c 0x1d 0x1e 0x1f"
    set key_len [llength $amp_key]
    set key_len [decimal2hex $key_len]
    UTF::Message LOG "$::localhost" "btamplib::composeHciCreatePhysicalLinkHexCmd\
        physical_link_handle=$physical_link_handle key_type=$key_type \nkey_len=$key_len\
        amp_key=$amp_key"
    set hex_cmd "$physical_link_handle $key_len $key_type $amp_key"
    return $hex_cmd
}

UTF::doc {
    # [call [cmd btamplib::composeHciWriteRemoteAmpAssocHexCmd]
    # [opt mac_addr=010203040506] [opt physical_link_handle=0x10]
    # [opt country=XXX] [opt ch_low=null] [opt ch_high=null]
    # [opt invalid_tlv=null]]

    # Composes the WriteRemoteAmpAssoc hex string for use with wl command.
}

proc btamplib::composeHciWriteRemoteAmpAssocHexCmd {{mac_addr 010203040506}\
    {physical_link_handle 0x10} {country "XXX"} {ch_low ""} {ch_high ""}\
    {invalid_tlv ""}} {
    # Composes the WriteRemoteAmpAssoc hex string for use with wl command.

    # Reference: CR AMP HCI section 7.5.9, p135
    # OCF = 0x000b
    # Physical Link Handle = 1 byte
    # Length So Far = 2 bytes
    # Amp Assoc Remaining Length = 2 bytes
    # Amp Assoc Fragment = 1 - 248 bytes

    # See CR 802.11 PAL section 2.14.1 p10 for list of TLV
    # defined in AMP Assoc message.

    # set original "0x10 0x0 0x0 0x1d 0x0 1 6 0 0x00 0x1a 0x92 0xb1 0xf2\
    #    0xeb 2 9 0 0x58 0x58 0x58 201 254 0 14 1 20 5 5 0 1 72 00 16 09"
      
    # TLV 01 mac_addr
    set mac_hex [btamplib::mac2hex $mac_addr]
    set len [btamplib::decimal2hex [llength $mac_hex]]
    set len "$len 0x0" ;# pad to 2 bytes, swapping byte order
    set tlv(01) "0x01 $len $mac_hex"

    # TLV 02 preferred channel list. 
    # See also CR 802.11 PAL section 2.14.4 & 2.14.5
    # See also Core System Package, Vol 5, section 3.2.3, p35
    set country [string toupper $country]
    if {$country == "US"} {
        set reg_ext_id [btamplib::decimal2hex 201]
        set reg_class [btamplib::decimal2hex 12]
        set cov_class 0x0
    } else {
        # Debug country = XXX
        set reg_ext_id [btamplib::decimal2hex 201]
        set reg_class [btamplib::decimal2hex 254]
        set cov_class 0x0
    }

    # Country must be 3 characters
    set country [string trim $country]
    set country [string toupper $country]
    set len [string length $country]
    if {$len == 2} {
        set country "$country " 
    } elseif {$len <= 1 || $len > 3} {
        btamplib::logError $::log_ip "composeHciWriteRemoteAmpAssocHexCmd\
            Invalid country code: $country, must be 2 or 3 characters!"
        return
    }
    set country_hex [btamplib::ascii2hex $country]

    # User may have specified specific channel list to use.
    set ch_low [string trim $ch_low]
    set ch_high [string trim $ch_high]
    set ch_triplet ""
    set max_power ""
    set num_ch ""
    if {$ch_low != "" && $ch_high != ""} {
        set ch_low [btamplib::decimal2hex $ch_low]
        set ch_high [btamplib::decimal2hex $ch_high]
        set num_ch [expr $ch_high - $ch_low + 1]
        set num_ch [btamplib::decimal2hex $num_ch]
        set max_power 0x20
        set ch_triplet "$ch_low $num_ch $max_power"
    }

    # Compose final channel tlv.
    set channel_str "$country_hex $reg_ext_id $reg_class $cov_class $ch_triplet"
    set len [btamplib::decimal2hex [llength $channel_str]]
    set len "$len 0x0" ;# pad to 2 bytes, swapping byte order
    set tlv(02) "0x02 $len $channel_str"

    # TLV 05 PAL version
    set pal_ver 0x1
    set sig_company "0x0 0xf"
    set pal_subver "0x0 0x0"
    set tlv(05) "0x05 0x05 0x0 $pal_ver $sig_company $pal_subver"

    # Compose composite AMP assoc msg.
    # Invalid TLV allows for error testing
    set physical_link_handle [btamplib::cleanupHex $physical_link_handle]
    set length_so_far "0x0 0x0" ;# set to 0 for first fragment
    set amp_assoc_msg "$invalid_tlv $tlv(01) $tlv(02) $tlv(05)"
    set len [btamplib::decimal2hex [llength $amp_assoc_msg]]
    set len "$len 0x0" ;# pad to 2 bytes, swapping byte order
    set hex_cmd "$physical_link_handle $length_so_far $len $amp_assoc_msg"
    UTF::Message LOG "$::localhost" "btamplib::composeHciWriteRemoteAmpAssocHexCmd\
        physical_link_handle=$physical_link_handle \nlength_so_far=$length_so_far\
        amp_assoc_msg_len=$len\
        \ninvalid_tlv=$invalid_tlv\
        \ntlv(01)=$tlv(01) mac_addr=$mac_addr\
        \ntlv(02)=$tlv(02) country=$country $country_hex reg_ext_id=$reg_ext_id\
        \nreg_class=$reg_class cov_class=$cov_class ch_low=$ch_low num_ch=$num_ch\
        ch_high=$ch_high max_power=$max_power\
        \ntlv(05)=$tlv(05) pal_ver=$pal_ver sig_company=$sig_company\
        pal_subver=$pal_subver"
    return $hex_cmd
}

UTF::doc {
    # [call [cmd btamplib::parseHciAcceptLogicalLinkRequest]
    # [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the HCI AcceptLogicalLinkRequest
    # command & event data.
}

proc btamplib::parseHciAcceptLogicalLinkRequest {{expected_status 0x0}} {

    # Reference: CR AMP HCI section 7.1.41 p88
    # OCF = 0x0039
    # Physical Link Handle = 1 byte
    # TX flow spec = 16 bytes
    # RX flow spec = 16 bytes
    # see proc composeHciCreateLogicalLinkHexCmd for details
    # of TX / RX flow specs
    # expect status event - see p89 on events generated

    # Get next event 
    UTF::Message LOG $::log_ip "======== btamplib::parseHciAcceptLogicalLinkRequest\
        $::log_ip expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = accept logical link
    btamplib::checkHciCommand "accept logical link"

    # Check command status
    btamplib::parseHciCommandStatus 0x1 0x39 $expected_status
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciAcceptPhysicalLinkRequest]
    # [opt expected_status=0x0]]

     # Reads the next HCI item, expecting it to be the HCI AcceptPhysicalLinkRequest
     # command & event data.
}

proc btamplib::parseHciAcceptPhysicalLinkRequest {{expected_status 0x0}} {

    # Reference CR AMP HCI section 7.1.38, p84
    # OCF = 0x0036
    # physical link handle = 1 byte
    # AMP key length = 1 byte
    # AMP key type = 1 byte
    # AMP key =  # bytes per key length
    # expect status event

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciAcceptPhysicalLinkRequest\
        $::log_ip expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = accept physical link
    btamplib::checkHciCommand "accept physical link"

    # Check command status
    btamplib::parseHciCommandStatus 0x1 0x36 $expected_status
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciAclData] [opt expected_link_handle=0x0010]
    # [opt expected_flags=0x03] [opt expected_data_pattern=null]]

    # Reads the next HCI item, expecting it to be ACL data. Optionally checks
    # data against [opt expected_data_pattern]
}

proc btamplib::parseHciAclData {{expected_link_handle 0x0010} {expected_flags 0x03}\
    {expected_data_pattern ""}} {

    # Reference: CR AMP HCI, section 5.4.2, p58
    # Handle = 12 bits NB: Fine print says for RX its the physical link handle!
    # Packet_Boundary_Flag = 2 bits
    # Broadcast_Flag = 2 bits
    # Data_Total_Length = 2 bytes
    # Data = based on above length
    # 2009/7/8 - sent email to Sherman & Ray asking how to decode flags
    # and pesumably map them to the 2 2-bit fields above.

    # Get next event 
    UTF::Message LOG $::log_ip "======== btamplib::parseHciAclData $::log_ip\
        expected_link_handle=$expected_link_handle\
        expected_flags=$expected_flags expected_data_pattern=$expected_data_pattern"
    btamplib::getNextHciItem ACLDATA

    # Check physcical link handle
    btamplib::checkHexString acl_data_plh $::hci_acl_handle oneof $expected_link_handle
    
    # Check flags.
    btamplib::checkHexString acl_data_flags $::hci_acl_flags oneof $expected_flags

    # Ray's email 2009/7/13 says: lower 2 bits are Packet Boundary flags,
    # next 2 bits are BroadCast flags.
    # set ::hci_acl_flags 4 ;# test code
    if {$::hci_acl_flags != ""} {
        set PB [expr $::hci_acl_flags & 0x03]
        set BC [expr $::hci_acl_flags & 0x0c]
        set BC [expr $BC >> 2]
        UTF::Message LOG $::log_ip "btamplib::parseHciAclData flags: PB=$PB BC=$BC"
    }

    # Do optional wildcard pattern matching on ACL data.
    set expected_data_pattern [string trim $expected_data_pattern]
    if {$expected_data_pattern == ""} {
        return
    }
    if {[string match -nocase "*$expected_data_pattern*" $::hci_acl_data]} {
        UTF::Message LOG $::log_ip "btamplib::parseHciAclData MATCHED\
            expected_data_pattern=$expected_data_pattern"
    } else {
        btamplib::logError "$::log_ip" "btamplib::parseHciAclData data $::hci_acl_data\
            did NOT match $expected_data_pattern"
    }
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciAmpAssocMsg] [arg start] [arg expected_length]
    # [arg mac_addr] [opt subband=null] [opt "expected_tlv_list=01 02 05"]
    # [opt host=DUT]]

    # Parses the AmpAssoc portion of an event, which has already been read.
}

proc btamplib::parseHciAmpAssocMsg {start expected_length mac_addr {subband ""}\
    {expected_tlv_list "01 02 05"} {host "DUT"}} {
    # See CR 802.11 PAL section 2.14.1 p10 for list of TLV
    # defined in this message.

    # Clean up calling tokens
    set start [string trim $start]
    set expected_length [string trim $expected_length]
    set host [string trim $host]
    set host [string toupper $host]
    if {$start == "" || $expected_length == ""} {
        btamplib::logError $::log_ip "btamplib::parseAmpAssocMsg got null calling data\
            start=$start expected_length=$expected_length"
        return
    }

    # Extract the AMP Assoc Msg data
    set end [expr $start + $expected_length -1]
    set msg_data [lrange $::hci_event_data $start $end]

    # Did we get the correct number of bytes?
    set len [llength $msg_data]
    if {$len == $expected_length} {
        UTF::Message LOG $::log_ip "btamplib::parseAmpAssocMsg got expected\
            $expected_length d.[format %d $expected_length] bytes"
    } else {
        btamplib::logError $::log_ip "btamplib::parseAmpAssocMsg got msg_data=$msg_data\
            len=$len bytes, was expecting $expected_length\
            d.[format %d $expected_length] bytes"
    }

    # Load TLV data into array.
    btamplib::parseTlvData $msg_data tlv_array

    # Parse items & expected values.
    set names [lsort [array names tlv_array]]
    # puts "names=$names"
    set found_list ""
    foreach type $names {
        set value $tlv_array($type)
        set value [string tolower $value]

        # Look for 01 mac address
        if {$type == "01"} {
            if {[lsearch -exact $expected_tlv_list $type] >= 0} {
                lappend found_list $type
            }
            regsub -all {\s} $value "" value
            btamplib::checkHexString "$type mac_addr" $value oneof $mac_addr
            continue
        }

        # Look for 02 preferred channel list
        if {$type == "02"} {
            if {[lsearch -exact $expected_tlv_list $type] >= 0} {
                lappend found_list $type
            }
            btamplib::parseHciChannelList $type $value 1 13 $subband
            continue
        }

        # Look for 03 connected channel list
        if {$type == "03"} {
            if {[lsearch -exact $expected_tlv_list $type] >= 0} {
                lappend found_list $type
            }
            btamplib::parseHciChannelList $type $value 1 13 $subband
            continue
        }

        # Look for 04 PAL capabilities
        if {$type == "04"} {
            if {[lsearch -exact $expected_tlv_list $type] >= 0} {
                lappend found_list $type
            }
            set value [swapHexOrder $value]
            btamplib::checkHexString "$type pal_capabilities" $value range 0 3
            continue
       }

        # Look for 05 PAL version
        # See CR 802.11 PAL section 2.14.6
        if {$type == "05"} {
            if {[lsearch -exact $expected_tlv_list $type] >= 0} {
                lappend found_list $type
            }
            set pal_version [lindex $value 0] 
            btamplib::checkHexString "$type pal_version" $pal_version oneof 0x01
            set company_id [btamplib::swapHexOrder [lrange $value 1 2]]
            btamplib::checkHexString "$type company_id" $company_id oneof 15
            set pal_subversion [btamplib::swapHexOrder [lrange $value 3 4]]
            btamplib::checkHexString "$type pal_subversion" $pal_subversion oneof 2320
            continue
        }

        # Any other TLV are currently unknown ==> error
        btamplib::logError "$::log_ip" "btamplib::parseAmpAssocMsg unknown\
            type=$type value=$value"
    }

    # Compare found mandatory types to expected list
    set expected_tlv_list [lsort $expected_tlv_list]
    set found_list [lsort $found_list]
    if {$found_list == $expected_tlv_list} {
        UTF::Message LOG $::log_ip "btamplib::parseAmpAssocMsg found all mandatory TLV items:\
            $expected_tlv_list" 
    } else {
        if {$host == "DUT" || $host == "IUT"} {
            # For DUT, TLV must be present
            btamplib::logError $::log_ip "btamplib::parseAmpAssocMsg found_list=$found_list NE\
                expected_tlv_list=$expected_tlv_list"
        } else {
            # For Ref1/2, see PR76129, most TLV not present.
            UTF::Message LOG $::log_ip "btamplib::parseAmpAssocMsg found_list=$found_list NE\
                expected_tlv_list=$expected_tlv_list"
        }
    }
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciChannelList] [arg type] [arg channel_string]
    # [arg ch_low] [arg ch_high] [opt subband=null]]

    # Decodes the TLV ChannelList [arg channel_string] data.
}

proc btamplib::parseHciChannelList {type channel_string ch_low ch_high {subband ""}} {
    # Reference: CR 802.11 PAL section 2.14.4 & 2.14.5
    # TypeId = 1 byte, 02=preferred channel list, 03=connected channel list

    # Reference: Core System package AMP controller Vol 5 section 3.2.3 p35

    # Decode country (3 bytes) info to ASCII.
    # See IEEE 802.11 2007 section 7.3.2.9 p105
    set country [btamplib::hex2ascii [lrange $channel_string 0 2]]
    btamplib::checkTextString "$type country" $country "US " "XXX" "DE " "AL "

    # Remainder of ChannelList taken from: Core System Package, Vol 5,
    # section 3.2.3, p35

    # We expect at least one regulatory triplet consisting of:
    # Regulatory Extension ID, Regulatory Class & Check Coverage Class.
    set found_reg 0
    set found_subband 0
    set i 3 ;# start after 3 character country code
    set reg_channels_found ""
    set subband_channels_found ""
    while { 1 } {

        # Get regulatory triplet.
        set j [expr $i + 2]
        set triplet [lrange $channel_string $i $j]
        set len  [llength $triplet]
        # puts "parseHciChannelList $type i=$i j=$j found triplet=$triplet"
        if {$len == 0} {
            # There are no more optional triplets, we are done.
            break
        }

        # This is a regulatory triplet only if the regulatory extension ID = 201 0xc9
        set reg_ext_id [cleanupHex [lindex $triplet 0]]
        if {$reg_ext_id != "0xc9"} {
            UTF::Message LOG $::log_ip "btamplib::parseHciChannelList $type triplet=$triplet\
                is not a regulatory triplet, first byte is not 0xc9"
            break
        }

        # Count the regulatory triplets found, move the pointer ahead.
        incr found_reg
        set i [expr $j + 1]
        UTF::Message LOG $::log_ip "btamplib::parseHciChannelList $type triplet=$triplet\
            is a regulatory triplet, first byte is 0xc9"

        # Get channel list for the specific country & regulatory class
        set reg_class [lindex $triplet 1]
        set ch_list [btamplib::regulatory2channels $country $reg_class]
        set reg_channels_found "$reg_channels_found $ch_list"

        # Check Coverage Class = 0
        btamplib::checkHexString "$type coverage_class" [lindex $triplet 2] oneof 0
    }

    # Did we get at least one of the regulatory triplets?
    if {$found_reg == 0} {
        btamplib::logError "$::log_ip" "btamplib::parseHciChannelList $type did\
            NOT find any regulatory triplets!"
    }

    # There may be optional sub-band triplets.
    while { 1 } {

        # Get optional triplet.
        set j [expr $i + 2]
        set triplet [lrange $channel_string $i $j]
        set len  [llength $triplet]
        # puts "btamplib::parseHciChannelList $type i=$i j=$j subband triplet=$triplet"
        set i [expr $j + 1]
        if {$len == 0} {
            # There are no more optional triplets, we are done.
            break
        }

        # Decode triplet in format: first_ch num_ch max_power
        incr found_subband
        set first_ch [lindex $triplet 0]
        set first_ch [btamplib::hex2decimal $first_ch]
        set num_ch [lindex $triplet 1]
        set num_ch [btamplib::hex2decimal $num_ch]
        set max_power [lindex $triplet 2]
        UTF::Message LOG $::log_ip "btamplib::parseHciChannelList $type subband\
            triplet=$triplet first_ch=$first_ch num_ch=$num_ch max_power=$max_power"

        # Add channels based on triplet data.
        for {set k $first_ch} {$k <= [expr $first_ch + $num_ch - 1]} {incr k} {
            # puts "adding ch $k"
            lappend subband_channels_found $k
        }

        # Check max_power is in range 0 - 20
        btamplib::checkHexString "$type max_power" $max_power range 0 20
    }

    # There are places where we must have sub-band triplets.
    set subband [string trim $subband]
    if {$subband != ""} {
        if {$found_subband == $subband} {
            UTF::Message LOG $::log_ip "btamplib::parseHciChannelList $type got\
                the expected $subband sub-band triplet(s)"
        } else {
            btamplib::logError "$::log_ip" "btamplib::parseHciChannelList $type\
                found $found_subband subband triplet(s), was expecting $subband\
                sub-band triplets!"
        }
    }

    # When sub-band triplets are present, we use the more detailed subband triplet
    # information and ignore the channels found from the regulatory triplets.
    if {$subband_channels_found != ""} {
        set channels_found $subband_channels_found
        UTF::Message LOG $::log_ip "btamplib::parseHciChannelList $type ignoring\
            regulatory triplet channels: $reg_channels_found"
        UTF::Message LOG $::log_ip "btamplib::parseHciChannelList $type using\
            sub-band triplet channels: $channels_found" 

    } else {
        set channels_found $reg_channels_found
        UTF::Message LOG $::log_ip "btamplib::parseHciChannelList $type using\
            regulatory triplet channels: $channels_found" 
    }

    # We expect at least one channel in the range ch_low - ch_high
    set matched_channels ""
    if {[regexp {^\d+$} $ch_low] && [regexp {^\d+$} $ch_high]} {
        for {set i $ch_low} {$i <= $ch_high} {incr i} {
            if {[lsearch -exact $channels_found $i] >=0} {
                lappend matched_channels $i
            }
        }
    }
    if {$matched_channels != ""} {
        UTF::Message LOG $::log_ip "btamplib::parseHciChannelList $type\
            matched_channels=$matched_channels MATCHED range: $ch_low - $ch_high"
    } else {
        logError "$::log_ip" "btamplib::parseHciChannelList\
            $type channels_found=$channels_found had NO matches in range:\
            $ch_low - $ch_high"
    }
    return $matched_channels
}

UTF::doc {
    # [call [cmd btamplib::regulatory2channels] [arg country]
    # [arg regulatory_class]]

    # Returns allowed channel list for specified [arg country] string
    # & hex [arg regulatory_class].
}

proc btamplib::regulatory2channels {country regulatory_class} {
    # Returns allowed channel list for specified country & hex regulatory class.

    # Reference: 802.11K 2008 Annex J - has partial list
    # Reference: 802.11Y 2008 Annex J - has complete list

    # Only load channel info the first time.
    if {![info exists ::reg_cl(US,01)]} {

        # US regulatory classes
        UTF::Message LOG $::log_ip "btamplib::regulatory2channels loading channels"
        set ::reg_cl(US,01) "36 40 44 48"
        set ::reg_cl(US,02) "52 56 60 64"
        set ::reg_cl(US,03) "149 153 157 161"
        set ::reg_cl(US,04) "100 104 108 112 116 120 124 128 132 136 140"
        set ::reg_cl(US,05) "149 153 157 161 165"
        set ::reg_cl(US,06) "1 2 3 4 5 6 7 8 9 10"
        set ::reg_cl(US,07) "1 2 3 4 5 6 7 8 9 10"
        set ::reg_cl(US,08) "11 13 15 17 19"
        set ::reg_cl(US,09) "11 13 15 17 19"
        set ::reg_cl(US,0a) "21 25"
        set ::reg_cl(US,0b) "21 25"
        set ::reg_cl(US,0c) "1 2 3 4 5 6 7 8 9 10 11"
        set ::reg_cl(US,0d) "133 137"
        set ::reg_cl(US,0e) "132 134 136 138"
        set ::reg_cl(US,0f) "131 132 133 134 135 136 137 138" 

        # European regulatory classes
        set ::reg_cl(EUR,04) "1 2 3 4 5 6 7 8 9 10 11 12 13"

        # SWAG for DE - need real data here...
        set ::reg_cl(DE,01) "1 2 3 4 5 6 7 8 9 10 11 12 13"
        set ::reg_cl(DE,02) "1 2 3 4 5 6 7 8 9 10 11 12 13"
        set ::reg_cl(DE,03) "1 2 3 4 5 6 7 8 9 10 11 12 13"
        set ::reg_cl(DE,04) "1 2 3 4 5 6 7 8 9 10 11 12 13"

        # Japan regulatory classes
        set ::reg_cl(JAP,1e) "1 2 3 4 5 6 7 8 9 10 11 12 13"
        set ::reg_cl(JAP,1f) "14"
        set ::reg_cl(JAP,20) "52 56 60 64"

        # Development & Debug
        set ::reg_cl(XXX,fe) "1 2 3 4 5 6 7 8 9 10 11"
        set ::reg_cl(XV,fe) "1 2 3 4 5 6 7 8 9 10 11"

        # SWAG for AL - need real data here...
        set ::reg_cl(AL,01) "1 2 3 4 5 6 7 8 9 10 11 12 13"
        set ::reg_cl(AL,02) "1 2 3 4 5 6 7 8 9 10 11 12 13"
        set ::reg_cl(AL,03) "1 2 3 4 5 6 7 8 9 10 11 12 13"
        set ::reg_cl(AL,04) "1 2 3 4 5 6 7 8 9 10 11 12 13"
    }

    # Clean up country.
    set country [string trim $country]
    set country [string toupper $country]

    # Massage regulatory class into 2 hex digits
    set regulatory_class [string trim $regulatory_class]
    set regulatory_class [string tolower $regulatory_class]
    regsub {^0x} $regulatory_class "" regulatory_class ;# ignore leading 0x
    if {[string length $regulatory_class] == 1} {
        set regulatory_class "0${regulatory_class}" ;# add leading 0
    }

    # Look up the specified channel list and return string.
    if {[info exists ::reg_cl($country,$regulatory_class)]} {
        set result "$::reg_cl($country,$regulatory_class)"
        UTF::Message LOG $::log_ip "btamplib::regulatory2channels country=$country\
            regulatory_class=$regulatory_class result=$result"

    } else {
        set result "Not Defined"
        logError "$::log_ip" "btamplib::regulatory2channels country=$country\
            regulatory_class=$regulatory_class result=$result"
    }
    return $result
}

UTF::doc {
    # [call [cmd btamplib::parseHciChannelSelected]
    # [opt expected_physical_link_handle=0x10]]

    # Reads the next HCI item, expecting it to be the ChannelSelected event
    # data, returns actual_handle.
}

proc btamplib::parseHciChannelSelected {{expected_physical_link_handle 0x10}} {

    # Reference: CR AMP HCI section 7.7.52, p147
    # OCF 0x41
    # Physical Link Handle = 1 byte

    # This event arrives later on, so dont look for a command
    UTF::Message LOG $::log_ip "======== btamplib::parseHciChannelSelected $::log_ip\
        expected_physical_link_handle=$expected_physical_link_handle"
    btamplib::getNextHciItem EVENT

    # Check event code = 0x41
    btamplib::checkHciEventCode 0x41

    # Check event text = channel select
    btamplib::checkHciEventText "channel select"

    # Check physical link handle
    set actual_handle [btamplib::checkOtherData physical_link_handle 0 0 oneof\
        $expected_physical_link_handle]
    btamplib::checkEol 1

    # Return actual handle
    return $actual_handle
}

UTF::doc {
    # [call [cmd btamplib::parseHciCommandComplete] [arg expected_OGF]
    # [arg expected_OCF] [arg expected_status_list]]

    # Used to parse the common data portion of an event that has already been read.
}

proc btamplib::parseHciCommandComplete {expected_OGF expected_OCF expected_status_list} {

    # Reference: CR AMP HCI section 7.7.14 p138
    # HCI command complete event = 0xe, 
    # Num_HCI_Comm = 1 byte, usually 0x1
    # Opcode = 2 bytes
    # remainder depends on command
    # sample: len=4 data=01 03 0c 00
    #      Num_HCI = 1
    #      Opcode = 0c03 NB: flip the bytes before decoding!
    #         OGF = 3 
    #         OCF = 3 = Reset
    #      status = 0 = success

    # This routine assumes that the HCI event has already been
    # read and the data is available in ::hci_event_data.

    # Most command complete events have same format,
    # but different opcode & status values.

    # Check event = command complete
    btamplib::checkHciEventText "command complete"

    # Check event code = 0xe
    btamplib::checkHciEventCode 0xe

    # Check NumHciCmd
    btamplib::checkNumHciCmd 01

    # Check Opcode
    btamplib::checkHciOpcode $expected_OGF $expected_OCF 

    # Check Status (byte 3)
    btamplib::checkHciStatus $expected_status_list

    # Other routines check the event specific data and also
    # check for EOL.
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciCommandStatus] [arg expected_OGF]
    # [arg expected_OCF] [arg expected_status]]

    # Used to parse the CommandStatus event common data, which has already
    # been read.
}

proc btamplib::parseHciCommandStatus {expected_OGF expected_OCF expected_status} {

    # Reference: CR AMP HCI section 7.7.15 p139
    # HCI command complete status = 0xf
    # Status = 1 byte, 0 = command pending
    # Num_HCI_Comm = 1 byte, usually 0x1
    # Opcode = 2 bytes

    # Check event = command status
    btamplib::checkHciEventText "command status"

    # Check event code = 0xf
    btamplib::checkHciEventCode 0xf

    # Check Status, offset = 0
    btamplib::checkHciStatus $expected_status 0 Pending

    # Check NumHciCmd, offset = 1
    btamplib::checkNumHciCmd 1 1

    # Check Opcode, offset = 2
    btamplib::checkHciOpcode $expected_OGF $expected_OCF 2

    # Check EOL, byte 4 should be null.
    btamplib::checkEol 4
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciCompletedDataBlocks] [opt expected_num_handles=1]
    # [opt args=0x0000...]]

    # Reads the next HCI item, expecting it to be the CompletedDataBlocks event
    # data. [opt args] will contain the expected values for the handles, one
    # per triplet in the event, defaults to 0x0000.
}

proc btamplib::parseHciCompletedDataBlocks {{expected_num_handles 1} args } {
    # args will contain the expected values for the handles, one
    # per triplet in the event, defaults to 0x0000.

    # Reference: CR AMP HCI, section 7.7.59, p152
    # Event code: 0x48
    # Total_num_Data_Blocks = 2 bytes, buffer pool size
    # Num_of_Handles = 1 byte, number of triplets in this event
    # Triplet description:
    #   Handle = 2 bytes (12 bits meaningful)
    #   Num_of_Completed_Packets = 2 bytes
    #   Num_of_Completed_Blocks = 2 bytes

    # Get the next event. There is no HCI command.
    UTF::Message LOG $::log_ip "======== btamplib::parseHciCompletedDataBlocks $::log_ip\
        expected_num_handles=$expected_num_handles args=$args"
    btamplib::getNextHciItem EVENT

    # Check event = completed data blocks
    btamplib::checkHciEventText "completed data blocks"

    # Check event code = 0x48
    btamplib::checkHciEventCode 0x48

    # Check Total_num_Data_Blocks
    btamplib::checkOtherData Total_num_Data_Blocks 0 1 range 0 0xffff

    # Check Total_num_Data_Blocks
    set num_of_handles [btamplib::checkOtherData num_of_handles 2 2 oneof $expected_num_handles]

    # Parse remaining data as triplets. To minimize the errors generate from
    # invalid packets, we exit the loop when the first handle is null.
    set start 3
    set found_triplets 0
    for {set i 0} {$i < $num_of_handles} {incr i} {

        # Check handle, 2 bytes. Expected value is taken from args.
        set expected_handle [lindex $args $i]
        if {$expected_handle == ""} {
            set expected_handle 0x0000
        }
        # puts "i=$i expected_handle=$expected_handle"
        set end [expr $start + 1]
        set cur_handle [btamplib::checkOtherData handle $start $end oneof $expected_handle]
        if {$cur_handle == ""} {
            break
        }

        # Check Num_of_Completed_Packets, 2 bytes, usually = 1
        set start [expr $end + 1]
        set end [expr $start + 1]
        btamplib::checkOtherData Num_of_Completed_Packets $start $end oneof 1

        # Check Num_of_Completed_Blocks, 2 bytes, usually = 1
        set start [expr $end + 1]
        set end [expr $start + 1]
        btamplib::checkOtherData Num_of_Completed_Blocks $start $end oneof 1

        # Move pointer ahead for next triplet, if any.
        set start [expr $end + 1]
        incr found_triplets
    }

    # Did we get the expected handles?
    if {$found_triplets != $num_of_handles} {
        btamplib::logError "$::log_ip" "btamplib::parseHciCompletedDataBlocks found_triplets=$found_triplets\
            NE num_of_handles=$num_of_handles"
    }

    # Check for EOL. Should be no more data left.
    btamplib::checkEol $start
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciCreateLogicalLink] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the CreateLogicalLink event
    # data.
}

proc btamplib::parseHciCreateLogicalLink {{expected_status 0x0}} {

    # Reference: CR AMP HCI section 7.1.40 p87
    # OCF = 0x0038
    # Physical Link Handle = 1 byte
    # TX flow spec = 16 bytes
    # RX flow spec = 16 bytes
    # see proc composeHciCreateLogicalLinkHexCmd for details
    # of TX / RX flow specs
    # expect status event - see p88 on events generated

    # Get next event 
    UTF::Message LOG $::log_ip "======== btamplib::parseHciCreateLogicalLink\
        $::log_ip expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = create logical link
    btamplib::checkHciCommand "create logical link"

    # Check command status
    btamplib::parseHciCommandStatus 0x1 0x38 $expected_status
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciCreatePhysicalLink] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the CreatePhysicalLink event
    # data.
}

proc btamplib::parseHciCreatePhysicalLink {{expected_status 0x0}} {

    # Reference: CR AMP HCI section 7.1.37 p82
    # OCF = 0x0035
    # Physical Link Handle = 1 byte
    # Key Length = 1 byte
    # Key Type = 1 byte
    #   3 = debug
    #   4 = unauthenticated
    #   5 = authenticated
    # AMP Key = Key Length bytes
    # expect status event - see p83 on events generated

    # Get next event 
    UTF::Message LOG $::log_ip "======== btamplib::parseHciCreatePhysicalLink $::log_ip expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = create physical link
    btamplib::checkHciCommand "create physical link"

    # Check command status
    btamplib::parseHciCommandStatus 0x1 0x35 $expected_status
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciDisconnectLogicalLink] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the DisconnectLogicalLink event
    # data.
}

proc btamplib::parseHciDisconnectLogicalLink {{expected_status 0x0}} {

    # Reference: CR AMP HCI section 7.1.42, p90
    # OCF 0x3A
    # Logical Link Handle = 2 bytes

    # Returns: get Status Event, p90

    # Get next event 
    UTF::Message LOG $::log_ip "======== btamplib::parseHciDisconnectLogicalLink $::log_ip expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = disconnect logical link
    btamplib::checkHciCommand "disconnect logical link"

    # Check command status
    btamplib::parseHciCommandStatus 0x1 0x3A $expected_status
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciDisconnectPhysicalLink] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the DisconnectPhysicalLink event
    # data.
}

proc btamplib::parseHciDisconnectPhysicalLink {{expected_status 0x0}} {

    # Reference: CR AMP HCI section 7.1.39, p86
    # OCF 0x37
    # Physical Link Handle = 1 byte
    # Reason Code = 1 byte, same values as Status code

    # Returns: get Status Event

    # Get next event 
    UTF::Message LOG $::log_ip "======== btamplib::parseHciDisconnectPhysicalLink $::log_ip expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = disconnect physical link
    btamplib::checkHciCommand "disconnect physical link"

    # Check command status
    parseHciCommandStatus 0x1 0x37 $expected_status
    btamplib::checkEol 4
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciDisconnectLogicalLinkComplete]
    # [opt expected_status=0x0] [opt expected_logical_link_handle=0x0000]
    # [opt reason=0x16]]

    # Reads the next HCI item, expecting it to be the DisconnectLogicalLinkComplete
    # event data.
}

proc btamplib::parseHciDisconnectLogicalLinkComplete {{expected_status 0x0}\
    {expected_logical_link_handle 0x0000} {reason 0x16}} {

    # Reference: CR AMP HCI section 7.7.57, p151
    # OCF 0x46
    # Status = 1 byte
    # Logical Link Handle = 2 bytes
    # Reason = 1 byte, same values as Status code.

    # This event arrives later on, so dont look for a command
    UTF::Message LOG $::log_ip "======== btamplib::parseHciDisconnectLogicalLinkComplete\
        $::log_ip expected_status=$expected_status\
        expected_logical_link_handle=$expected_logical_link_handle\
        reason=$reason"
    btamplib::getNextHciItem EVENT

    # Check event code = 0x46
    btamplib::checkHciEventCode 0x46

    # Check event text = disconnect logical link complete
    btamplib::checkHciEventText "disconnect logical link complete"

    # Check Status, offset = 0
    btamplib::checkHciStatus $expected_status 0 "Disconnection has occurred"

    # Check physical link handle
    btamplib::checkOtherData logical_link_handle 1 2 oneof $expected_logical_link_handle

    # Check Reason using the Status code routine, offset = 3
    UTF::Message LOG $::log_ip "btamplib::parseHciDisconnectLogicalLinkComplete Reason\
        code, offset=3 interpreted by Status code routine"
    btamplib::checkHciStatus $reason 3

    # Check for EOL
    btamplib::checkEol 4
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciDisconnectPhysicalLinkComplete]
    # [opt expected_status=0x0] [opt expected_physical_link_handle=0x10]
    # [opt reason=0x16]]

    # Reads the next HCI item, expecting it to be the DisconnectPhysicalLinkComplete
    # event data.
}

proc btamplib::parseHciDisconnectPhysicalLinkComplete {{expected_status 0x0}\
    {expected_physical_link_handle 0x10} {reason 0x16}} {

    # Reference: CR AMP HCI section 7.7.53, p148
    # OCF 0x42
    # Status = 1 byte
    # Physical Link Handle = 1 byte
    # Reason = 1 byte, same values as Status code.

    # This event arrives later on, so dont look for a command
    UTF::Message LOG $::log_ip "======== btamplib::parseHciDisconnectPhysicalLinkComplete\
        $::log_ip expected_status=$expected_status\
        expected_physical_link_handle=$expected_physical_link_handle\
        reason=$reason"
    btamplib::getNextHciItem EVENT

    # Check event code = 0x42
    btamplib::checkHciEventCode 0x42

    # Check event text = disconnect physical link complete
    btamplib::checkHciEventText "disconnect physical link complete"

    # Check Status, offset = 0
    checkHciStatus $expected_status 0 "Disconnection has occurred"

    # Check physical link handle
    btamplib::checkOtherData physical_link_handle 1 1 oneof $expected_physical_link_handle

    # Check Reason using the Status code routine, offset = 2
    UTF::Message LOG $::log_ip "btamplib::parseHciDisconnectPhysicalLinkComplete\
        Reason code, offset=2 interpreted by Status code routine"
    btamplib::checkHciStatus $reason 2

    btamplib::checkEol 3
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciEnhancedFlush] [opt expected_status=0x0]
    # [opt expected_handle=0x0000]]

    # Reads the next HCI item, expecting it to be the EnhancedFlush event data.
}

proc btamplib::parseHciEnhancedFlush {{expected_status 0x0} {expected_handle 0x0000}} {

    # Parses HCI EnhancedFlush command and response event.
    # Reference: CR AMP HCI section 7.3.66 p108
    # HCI EnhancedFlush command
    # OCF = 0x005F
    # Command parameters:
    #    Handle = 2 bytes, 12 bits significant
    #    Packet type = 1 byte, 0x00 only value defined
    # Returns command status event
    #
    # Later on, get EnhancedFlushComplete event

    # NB: Flush is intended to purge data from the transmitting
    # side to make way for newer packets

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciEnhancedFlush\
        expected_status=$expected_status expected_handle=$expected_handle"
    btamplib::getNextHciItem

    # Check HCI command = Enhanced Flush
    btamplib::checkHciCommand "Enhanced Flush"

    # Check HCI command status event
    btamplib::parseHciCommandStatus 0x03 0x5F $expected_status
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciEnhancedFlushComplete] [opt expected_handle=0x0000]]

    # Reads the next HCI item, expecting it to be the EnhancedFlushComplete event data.
}

proc btamplib::parseHciEnhancedFlushComplete {{expected_handle 0x0000}} {

    # Parses HCI EnhancedFlushComplete event.
    # Reference: CR AMP HCI section 7.7.47 p146
    # HCI EnhancedFlushComplete
    # Event code = 0x39
    # Parameters:
    #    Handle = 2 bytes, 12 bits significant

    # Get next event only
    UTF::Message LOG $::log_ip "======== btamplib::parseHciEnhancedFlushComplete\
        expected_handle=$expected_handle"
    btamplib::getNextHciItem EVENT

    # Check event code = 0x39
    btamplib::checkHciEventCode 0x39

    # Check event text = Enhanced Flush Complete
    btamplib::checkHciEventText "Enhanced Flush Complete"

    # Check link handle
    btamplib::checkOtherData link_handle 0 1 oneof $expected_handle
    btamplib::checkEol 2
}

UTF::doc {
    # [call [cmd btamplib::parseHciFlushOccurred] [opt expected_handle=0x0000]]

    # Reads the next HCI item, expecting it to be the FlushOccurred event data.
}

proc btamplib::parseHciFlushOccurred {{expected_handle 0x0000}} {

    # Parses HCI FlushOccurred event.
    # Reference: CR AMP HCI section 7.7.17 p141
    # HCI FlushOccurred
    # Event code = 0x11
    # Parameters:
    #    Handle = 2 bytes, 12 bits significant

    # Get next event only
    UTF::Message LOG $::log_ip "======== btamplib::parseHciFlushOccurred\
        expected_handle=$expected_handle"
    btamplib::getNextHciItem EVENT

    # Check event code = 0x11
    btamplib::checkHciEventCode 0x11

    # Check event text = Flush Occurred
    btamplib::checkHciEventText "Flush Occurred"

    # Check link handle
    btamplib::checkOtherData link_handle 0 1 oneof $expected_handle
    btamplib::checkEol 2
}

UTF::doc {
    # [call [cmd btamplib::parseHciFlowSpecModify] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the FlowSpecModify event data.
}

proc btamplib::parseHciFlowSpecModify {{expected_status 0x0}} {

    # Parses HCI FlowSpecModify command and response event.
    # Reference: CR AMP HCI section 7.1.44 p92
    # HCI FlowSpecModify command
    # OCF = 0x003C
    # Command parameters:
    #    Handle = 2 bytes, 12 bits significant
    #    TXFlowSpec = 16 bytes
    #    RXFlowSpec = 16 bytes
    # Returns command status event
    #    Status = 1 byte
    #
    # Later on, get FlowSpecModifyComplete event

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciFlowSpecModify\
        expected_status=$expected_status"
    btamplib::getNextHciItem
    # UTF::Message LOG $::log_ip "parseHciFlowSpecModify event data: $::hci_event_data"

    # Check HCI command = Flow Spec Modify
    btamplib::checkHciCommand "Flow Spec Modify"

    # Check HCI command status event
    btamplib::parseHciCommandStatus 0x1 0x3C $expected_status
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciFlowSpecModifyComplete] [opt expected_status=0x0]
    # [opt expected_handle=0x0000]]

    # Reads the next HCI item, expecting it to be the FlowSpecModifyComplete event data.
}

proc btamplib::parseHciFlowSpecModifyComplete {{expected_status 0x0}\
    {expected_handle 0x0000}} {

    # Parses HCI FlowSpecModify command and response event.
    # Reference: CR AMP HCI section 7.7.58 p152
    # HCI FlowSpecModifyComplete command
    # Event code = 0x47
    # Return parameters:
    #    Status = 1 byte
    #    Handle = 2 bytes, 12 bits significant

    # Get next event only
    UTF::Message LOG $::log_ip "======== btamplib::parseHciFlowSpecModifyComplete\
        expected_status=$expected_status expected_handle=$expected_handle"
    btamplib::getNextHciItem EVENT
    # UTF::Message LOG $::log_ip "parseHciFlowSpecModifyComplete event data: $::hci_event_data"

    # Check event code = 0x47
    btamplib::checkHciEventCode 0x47

    # Check event text = Flow Spec Modify Complete
    btamplib::checkHciEventText "Flow Spec Modify Complete"

    # Check Status, offset = 0
    btamplib::checkHciStatus $expected_status 0

    # Check logical link handle
    btamplib::checkOtherData logical_link_handle 1 2 oneof $expected_handle

    btamplib::checkEol 3
}

UTF::doc {
    # [call [cmd btamplib::parseHciLogicalLinkCancel] [opt expected_status=0x0]
    # [opt physical_link_handle=0x10"] [opt flow_spec_id=0x01]]

    # Reads the next HCI item, expecting it to be the LogicalLinkCancel event data.
}

proc btamplib::parseHciLogicalLinkCancel {{expected_status 0x0}\
     {physical_link_handle 0x10} {flow_spec_id 0x01}} {

    # Parses HCI LogicalLinkCancel command and response event.
    # Reference: CR AMP HCI section 7.1.43 p90
    # HCI LogicalLinkCancel command
    # OCF = 0x003B
    # Command parameters:
    #    PhysicalLinkHandle = 1 byte
    #    FlowSpecId = 1 byte
    # returns:
    #    Status = 1 byte
    #    PhysicalLinkHandle = 1 byte
    #    FlowSpecId = 1 byte

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciLogicalLinkCancel\
        expected_status=$expected_status physical_link_handle=$physical_link_handle\
        flow_spec_id=$flow_spec_id"
    btamplib::getNextHciItem

    # Check HCI command = Logical Link Cancel
    btamplib::checkHciCommand "Logical Link Cancel"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x1 0x3b $expected_status

    # Check physical_link_handle
    btamplib::checkOtherData physical_link_handle 4 4 oneof $physical_link_handle

    # Check flow_spec_id
    btamplib::checkOtherData flow_spec_id 5 5 oneof $flow_spec_id

    # Should be EOL
    btamplib::checkEol 6
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciLogicalLinkComplete] [opt expected_status=0x0]
    # [opt expected_physical_link_handle=0x10] [opt expected_logical_link_handle=0x0000]
    # [opt expected_tx_flow_spec_id=0x01]]

    # Reads the next HCI item, expecting it to be the LogicalLinkComplete event
    # data.
}

proc btamplib::parseHciLogicalLinkComplete {{expected_status 0x0}\
    {expected_physical_link_handle 0x10} {expected_logical_link_handle 0x0000}\
    {expected_tx_flow_spec_id 0x01}} {

    # Reference: CR AMP HCI section 7.7.56, p150
    # Event code 0x45
    # Status = 1 byte
    # Logical Link Handle = 2 bytes, 12 bits used
    # Physical Link Handle = 1 byte
    # Reference: Core_v3.0+HS, Vol 2, section 7.7.56, p710
    # now defines a 5th byte in the event, not shown in previous
    # older reference.
    # TX_flow_spec_id = 1 byte

    # This event arrives later on, so dont look for a command
    UTF::Message LOG $::log_ip "======== btamplib::parseHciLogicalLinkComplete $::log_ip\
         expected_status=$expected_status\
         expected_physical_link_handle=$expected_physical_link_handle\
         expected_logical_link_handle=$expected_logical_link_handle\
         expected_tx_flow_spec_id=$expected_tx_flow_spec_id"
    btamplib::getNextHciItem EVENT

    # Check event code = 0x45
    btamplib::checkHciEventCode 0x45

    # Check event text = logical link complete
    btamplib::checkHciEventText "logical link complete"

    # Check Status, offset = 0
    btamplib::checkHciStatus $expected_status 0

    # Check logical link handle
    btamplib::checkOtherData logical_link_handle 1 2 oneof $expected_logical_link_handle

    # Check physical link handle
    btamplib::checkOtherData physical_link_handle 3 3 oneof $expected_physical_link_handle

    # Check TX flow spec ID.
    btamplib::checkOtherData TX_flow_spec_ID 4 4 oneof $expected_tx_flow_spec_id
    btamplib::checkEol 5
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciPhysicalLinkComplete] [opt expected_status=0x0]
    # [opt expected_physical_link_handle=0x10]]

    # Reads the next HCI item, expecting it to be the PhysicalLinkComplete event
    # data.
}

proc btamplib::parseHciPhysicalLinkComplete {{expected_status 0x0}\
    {expected_physical_link_handle 0x10}} {

    # Reference: CR AMP HCI section 7.7.51, p146
    # Event code 0x40
    # Status = 1 byte
    # Physical Link Handle = 1 byte

    # This event arrives later on, so dont look for a command
    UTF::Message LOG $::log_ip "======== btamplib::parseHciPhysicalLinkComplete\
        $::log_ip expected_status=$expected_status\
        expected_physical_link_handle=$expected_physical_link_handle"
    btamplib::getNextHciItem EVENT

    # Check event code = 0x40
    btamplib::checkHciEventCode 0x40

    # Check event text = physical link complete
    btamplib::checkHciEventText "physical link complete"

    # Check Status, offset = 0
    checkHciStatus $expected_status 0

    # Check physical link handle
    btamplib::checkOtherData physical_link_handle 1 1 oneof $expected_physical_link_handle
    btamplib::checkEol 2
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciReadDataBlockSize] [opt expected_status=0x0]
    # [opt expected_max_acl_data_len=0x05d4] [opt expected_data_block_len=0x05d4]
    # [opt expected_minimum_data_blocks=0x30]]

    # Reads the next HCI item, expecting it to be the ReadDataBlockSize event data.
    # NB: The defaults used here are based on our product, not anything found in
    # the specs.
}

proc btamplib::parseHciReadDataBlockSize {{expected_status 0x0} \
    {expected_max_acl_data_len 0x05d4} {expected_data_block_len 0x05d4}\
    {expected_minimum_data_blocks 0x0c}} {

    # Parses HCI read data block size command & event response
    # Reference CR AMP HCI section 7.4.7 p121
    # OCF = 0x000A
    # command parameters: none
    # 
    # returns:
    #    status = 1 byte
    #    Max_ACL_pkt_data_len = 2 bytes  (Max PDU Size)
    #    Data_block_len = 2 bytes (portion of overall pkt size one buffer can hold)
    #    Total_num_data_blocks = 2 bytes (buffers available)

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciReadDataBlockSize $::log_ip\
        expected_status=$expected_status\
        expected_max_acl_data_len=$expected_max_acl_data_len\
        expected_data_block_len=$expected_data_block_len\
        expected_minimum_data_blocks=$expected_minimum_data_blocks"
    btamplib::getNextHciItem

    # Check HCI command = read data block size
    btamplib::checkHciCommand "read data block size"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x4 0x0A $expected_status

    # Check Max_ACL_pkt_data_len 
    btamplib::checkOtherData Max_ACL_pkt_data_len 4 5 oneof $expected_max_acl_data_len

    # Check Data_block_len, expected value
    btamplib::checkOtherData Data_block_len 6 7 oneof $expected_data_block_len

    # Check Total_num_data_blocks
    btamplib::checkOtherData Total_num_data_blocks 8 9 range $expected_minimum_data_blocks 4000000000

    btamplib::checkEol 10
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciReadFailedContactCounter]
    # [opt expected_link_handle=0x0000] [opt expected_count=0]]
    
    # Reads the next HCI item, expecting it to be the ReadFailedContactCounter
    # event data.
}

proc btamplib::parseHciReadFailedContactCounter {{expected_link_handle 0x0000}\
    {expected_count 0}} {
    # Parses HCI ReadFailedContactCounter command & event response
    # Reference CR AMP HCI section 7.5.1 p123
    # OCF = 0x0001
    # command parameters:
    #    link_handle = 2 bytes, 12 bits significant
    # returns:
    #    status = 1 byte
    #    link_handle = 2 bytes, 12 bits significant
    #    failed contact counter = 2 bytes

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciReadFailedContactCounter\
        $::log_ip expected_link_handle=$expected_link_handle expected_count=$expected_count"
    btamplib::getNextHciItem

    # Check HCI command = read Failed Contact Counter
    btamplib::checkHciCommand "read Failed Contact Counter"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x5 0x1 0x00

    # Check link handle
    btamplib::checkOtherData link_handle 4 5 oneof $expected_link_handle

    # Check failed contact counter
    btamplib::checkOtherData failed_contact_counter 6 7 oneof $expected_count
 
    btamplib::checkEol 8
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciResetFailedContactCounter]
    # [opt expected_link_handle=0x0000]]
    
    # Reads the next HCI item, expecting it to be the ResetFailedContactCounter
    # event data.
}

proc btamplib::parseHciResetFailedContactCounter {{expected_link_handle 0x0000}} {
    # Parses HCI ResetFailedContactCounter command & event response
    # Reference CR AMP HCI section 7.5.1 p123
    # OCF = 0x0002
    # command parameters:
    #    link_handle = 2 bytes, 12 bits significant
    # returns:
    #    status = 1 byte
    #    link_handle = 2 bytes, 12 bits significant

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciResetFailedContactCounter\
        $::log_ip expected_link_handle=$expected_link_handle"
    btamplib::getNextHciItem

    # Check HCI command = reset Failed Contact Counter
    btamplib::checkHciCommand "reset Failed Contact Counter"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x5 0x2 0x00

    # Check link handle
    btamplib::checkOtherData link_handle 4 5 oneof $expected_link_handle

    btamplib::checkEol 6
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciReadLinkQuality] [opt expected_link_handle=0x0010]
    # [opt expected_link_quality_low=100] [opt expected_link_quality_high=255]]

    # Reads the next HCI item, expecting it to be the ReadLinkQuality event data.
}

proc btamplib::parseHciReadLinkQuality {{expected_link_handle 0x0010}\
    {expected_link_quality_low 100} {expected_link_quality_high 255}} {
    # Parses HCI read link quality command & event response
    # Reference CR AMP HCI section 7.5.3 p125
    # OCF = 0x0003
    # command parameters:
    #    link_handle = 2 bytes
    # returns:
    #    status = 1 byte
    #    link_handle = 2 bytes
    #    link_quality = 1 byte

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciReadLinkQuality $::log_ip\
        expected_link_handle=$expected_link_handle\
        expected_link_quality_low=$expected_link_quality_low\
        expected_link_quality_high=$expected_link_quality_high"
    btamplib::getNextHciItem

    # Check HCI command = read link quality
    btamplib::checkHciCommand "read link quality"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x5 0x3 0x00

    # Check link handle
    btamplib::checkOtherData link_handle 4 5 oneof $expected_link_handle

    # Check link quality
    btamplib::checkOtherData link_quality 6 6 range $expected_link_quality_low $expected_link_quality_high
 
    btamplib::checkEol 7
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciReadLocalAmpAssoc] [arg mac_addr]
    # [opt expected_physical_link_handle=0x00] [opt subband=null]
    # [opt expected_status_list=0x00] [opt "expected_tlv_list=01 02 05"]
    # [opt host=DUT]]

    # Reads the next HCI item, expecting it to be the ReadLocalAmpAssoc event
    # data.
}

proc btamplib::parseHciReadLocalAmpAssoc {mac_addr {expected_physical_link_handle 0x00}\
    {subband ""} {expected_status_list 0x00} {expected_tlv_list "01 02 05"} {host "DUT"}} {

    # Parses HCI read local amp assoc command and response event.
    # Reference: CR AMP HCI section 7.5.8 p133
    # OCF = 0x000a
    # command parameters:
    #    physical_link_handle = 1 byte
    #    length_so_far = 2 bytes
    #    max_amp_assoc_length = 2 bytes
    # returns:
    #    status = 1 byte
    #    physical_link_handle = 1 byte
    #    NB: response does NOT have length_so_far field!
    #    amp_assoc_remaining_length = 2 bytes
    #    amp_assoc_fragment = 1 to 248 bytes

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciReadLocalAmpAssoc\
        $::log_ip mac_addr=$mac_addr\
        expected_physical_link_handle=$expected_physical_link_handle\
        subband=$subband expected_status_list=$expected_status_list\
        expected_tlv_list=$expected_tlv_list host=$host"
    btamplib::getNextHciItem

    # Check HCI command = read local amp assoc
    btamplib::checkHciCommand "read local amp assoc"

    # Check HCI command complete data. See PR76129 re expected_status.
    set host [string trim $host]
    set host [string toupper $host]
    if {$host == "DUT" || $host == "IUT"} {
        # DUT is required to have the expected status
        btamplib::parseHciCommandComplete 0x5 0xa $expected_status_list
    } else {
        # Ref1/2 usually get 0x30
        btamplib::parseHciCommandComplete 0x5 0xa "$expected_status_list 0x30"
    }

    # Check physical link handle
    btamplib::checkOtherData physical_link_handle 4 4 oneof $expected_physical_link_handle

    # Check amp_assoc_remaining_length is in range 0 248
    set data_length [btamplib::checkOtherData amp_assoc_remaining_length 5 6 range 0 248]

    # Parse the contents of the assoc message
    btamplib::parseHciAmpAssocMsg 7 $data_length $mac_addr $subband $expected_tlv_list $host

    # Should be EOL.
    if {$data_length == ""} {
        set data_length 0
    }
    btamplib::checkEol [expr 7 + $data_length]
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciReadLocalAmpInfo] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the ReadLocalAmpInfo event
    # data.
}

proc btamplib::parseHciReadLocalAmpInfo {{expected_status 0x0}} {

    # Parses HCI read local amp info command and response event.
    # Reference: CR AMP HCI section 7.5.7 p129
    # HCI Read Local Amp Info
    # OCF = 0x0009
    # has no command parameters
    # returns:
    # Num_HCI_Comm = 1 byte
    # Opcode = 2 bytes
    # status = 1 byte
    # amp_status = 1 byte
    # total_bandwidth = 4 bytes
    # max_guaranteed_bandwidth = 4 bytes
    # min_latency = 4 bytes
    # max_pdu_size = 4 bytes
    # controller_type = 1 byte
    # pal_capabilities = 2 bytes
    # max_amp_assoc_length = 2 bytes
    # max_flush_timeout = 4 bytes
    # best_effort_flush_timeout = 4 bytes

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciReadLocalAmpInfo $::log_ip\
         expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = read local amp info
    btamplib::checkHciCommand "read local amp info"

    # Check HCI command complete data (bytes 0 - 3)
    btamplib::parseHciCommandComplete 0x5 0x9 $expected_status

    # See 802.11 PAL Test Spec section 5.2.1.1 p26 for most of
    # the expected values used below.

    # Check AMP status = 0, 1. Apr 2009 test spec no longer shows
    # expected value for AMP status, so 1 is quite reasonable.
    # Email 2010/4/8 from Sherman says 3 & 6 are also valid.
    btamplib::checkOtherData amp_status 4 4 oneof 0 1 3 6

    # Check total_bandwidth in range 0 - 30000
    set tot_bw [btamplib::checkOtherData total_bw 5 8 range 0 30000]
 
    # Check max_guaranteed_bandwidth <= total_bandwidth
    btamplib::checkOtherData max_guar_bw 9 12 range 0 $tot_bw

    # Check min_latency - see IEEE 802.11 2007
    # CWmin values 15/31/63 shown on p536/547/589
    # DIFS - need reference... 
    # 0x1c is what Joe was getting...
    btamplib::checkOtherData min_latency 13 16 oneof 0x0000001c

    # Check max_pdu_size, see BT_Core_V3.0 Vol 5 section 6 p51
    # gives max=1492, not sure if there is a minimum or not.
    btamplib::checkOtherData max_pdu_size 17 20 oneof 1492

    # Check controller_type = 1
    btamplib::checkOtherData controller_type 21 21 oneof 1

    # Check pal_capabilities = 0,1
    btamplib::checkOtherData pal_capabilities 22 23 oneof 0 1

    # Check max_amp_assoc_length see BT_Core_V3.0 Vol 5 section 6 p51
    # gives max=672, not sure if there is a minimum or not.
    btamplib::checkOtherData max_amp_assoc_length 24 25 oneof 672

    # Check max_flush_timeout in range 0 - 0xffffffff, see TSE3348
    btamplib::checkOtherData max_flush_timeout 26 29 range 0 0xffffffff

    # Check best_effort_flush_timeout in range 0 - 0xffffffff, see TSE3348
    btamplib::checkOtherData be_flush_timeout 30 33 range 0 0xffffffff

    # Should be EOL.
    btamplib::checkEol 34
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciReadLocalSupportedCommands] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the ReadReadLocalSupportedCommands
    # event data.
}

proc btamplib::parseHciReadLocalSupportedCommands {{expected_status 0x0}} {

    # Parses HCI ReadLocalSupportedCommands command and response event.
    # Reference: CR AMP HCI section 7.4.2 p120
    # HCI ReadLocalSupportedCommands
    # OCF = 0x0002
    # Command parameters: none
    # 
    # returns:
    #   status = 1 byte
    #   supported commands = 64 bytes as a bit mask
    #   see routine translateHciCommands for details on decoding the bit mask

    # See Sherman's email 2009/10/1 for supported list used below.

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciReadLocalSupportedCommands $::log_ip\
        expected_status=$expected_status"
    btamplib::getNextHciItem
    
    # Check HCI command = read Local Supported Commands
    btamplib::checkHciCommand "read Local Supported Commands"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x4 0x2 $expected_status

    # Decode all 64 bytes of the bit mask and compare with expected values.
    set all_cmds ""
    for {set i 0} {$i <= 63} {incr i} {

        # Get commands represented by the i'th byte in the bit mask
        set j [expr $i + 4] ;# first 4 bytes are event header
        set bit_mask [lindex $::hci_event_data $j]
        set actual_cmds [btamplib::translateHciCommands $i $bit_mask]
        append all_cmds " $actual_cmds"

        # See Sherman's email 2009/10/1 for supported command lists.
        if {$i == 5} {
            set expected_cmds "Reset"
        } elseif {$i == 7} {
            set expected_cmds "ReadConnectionAcceptTimeout WriteConnectionAcceptTimeout"
        } elseif {$i == 11} {
            set expected_cmds "ReadLinkSupervisionTimeout WriteLinkSupervisionTimeout"
        } elseif {$i == 14} {
            set expected_cmds "ReadLocalVersionInformation ReadBufferSize"
        } elseif {$i == 15} {
            set expected_cmds "ReadFailedContactCount ResetFailedContactCount ReadLinkQuality"
        } elseif {$i == 19} {
            set expected_cmds "EnhancedFlush"
        } elseif {$i == 21} {
            set expected_cmds "CreatePhysicalLink AcceptPhysicalLinkRequest\
                DisconnectPhysicalLink CreateLogicalLink AcceptLogicalLink\
                DisconnectLogicalLink LogicalLinkCancel FlowSpecModify"
        } elseif {$i == 22} {
            set expected_cmds "ReadLogicalLinkAcceptTimeout WriteLogicalLinkAcceptTimeout\
                SetEventMaskPage2 ReadLocationData WriteLocationData\
                ReadLocalAMPInfo ReadLocalAMPASSOC WriteRemoteAMPASSOC"
        } elseif {$i == 23} {
            set expected_cmds "ReadDataBlockSize"
        } elseif {$i == 24} {
            set expected_cmds "ReadBestEffortFlushTimeout WriteBestEffortFlushTimeout\
                ShortRangeMode"
        } else {
            set expected_cmds ""
        }

        # Check if we have missing or extra commands.
        set missing_cmds [UTF::list_delta $actual_cmds $expected_cmds]
        set extra_cmds [UTF::list_delta $expected_cmds $actual_cmds]
        if {$missing_cmds == "" && $extra_cmds == ""} { 
            UTF::Message LOG "$::log_ip" "Byte $i got expected_cmds=$expected_cmds"
        } else {
            btamplib::logError "$::log_ip" "Byte $i actual_cmds=$actual_cmds NE\
                expected_cmds=$expected_cmds"
            if {$missing_cmds != ""} {
                btamplib::logError "$::log_ip" "Byte $i missing_cmds=$missing_cmds"
            }
            if {$extra_cmds != ""} {
                btamplib::logError "$::log_ip" "Byte $i extra_cmds=$extra_cmds"
            }
        }
    }

    # Log sorted list of all_cmds
    set all_cmds [lsort $all_cmds]
    set cnt [llength $all_cmds]
    UTF::Message LOG "$::log_ip" "cnt=$cnt all_cmds=$all_cmds"

    # Should be EOL
    btamplib::checkEol 68
}

UTF::doc {
    # [call [cmd btamplib::parseHciReadLocalVersionInfo] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the ReadReadLocalVersionInfo
    # event data.
}

proc btamplib::parseHciReadLocalVersionInfo {{expected_status 0x0}} {

    # Parses HCI ReadLocalVersionInfo command and response event.
    # Reference: CR AMP HCI section 7.4.1 p118
    # HCI ReadLocalVersionInfo
    # OCF = 0x0001
    # Command parameters: none
    # 
    # returns:
    #   status = 1 byte
    #   HCI Version = 1 byte
    #   HCI Revision = 2 bytes
    #   LMP/PAL Version = 1 byte
    #   Manufacturer Name = 2 bytes
    #   LMP/PAL Subversion = 2 bytes

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciReadLocalVersionInfo $::log_ip\
        expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = read Local Version Info
    btamplib::checkHciCommand "read Local Version Info"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x4 0x1 $expected_status

    # See Sherman's email 2009/10/1 for expected values used below.

    # Check HCI Version
    btamplib::checkOtherData HCI_Version 4 4 oneof 5

    # Check HCI Revision
    btamplib::checkOtherData HCI_Revision 5 6 oneof 0x7dc0 0x7db0 0x061e 0x0614 0x0564 0x055a 0x053c 0x04dc 0x04d9 0x04db 0x0001 0x0000

    # Check LMP/PAL Version
    btamplib::checkOtherData LMP/PAL_Version 7 7 oneof 1

    # Check Manufacturer Name
    btamplib::checkOtherData Manufacturer_Name 8 9 oneof 0x000f

    # Check LMP/PAL Subversion. For TOB, will show 0. For tagged builds, will be 
    # different value every build...
    btamplib::checkOtherData LMP/PAL_Subversion 10 11 range 0 0xffff

    # Should be EOL
    btamplib::checkEol 12
}

UTF::doc {
    # [call [cmd btamplib::parseHciReadRSSI] [opt expected_status=0x0]
    # [opt expected_link_handle=0x0000]]

    # Reads the next HCI item, expecting it to be the ReadRSSI event data.
    # Returns the RSSI value. NB: Currently not supported by WLAN BTAMP.
}

proc btamplib::parseHciReadRSSI {{expected_status 0x0} {expected_link_handle 0x0000}} {

    # Parses HCI RSSI command and response event.
    # Reference: CR AMP HCI section 7.5.4 p127
    # HCI read RSSI command
    # OCF = 0x0005
    # Command parameters:
    #   handle = 2 bytes, 12 bits significant
    # 
    # returns:
    #   status = 1 byte
    #   handle = 2 bytes, 12 bits significant
    #   RSSI = 1 byte

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciReadRSSI $::log_ip\
        expected_status=$expected_status expected_link_handle=$expected_link_handle"
    btamplib::getNextHciItem

    # Check HCI command = read RSSI
    btamplib::checkHciCommand "read RSSI"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x3 0x5 $expected_status

    # Check link handle
    btamplib::checkOtherData link_handle 4 5 oneof $expected_link_handle

    # Check RSSI
    set RSSI [btamplib::checkOtherData RSSI 6 6 range 0x0 0xff]

    # Should be EOL
    btamplib::checkEol 7
    return $RSSI
}

UTF::doc {
    # [call [cmd btamplib::parseHciReset] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the Reset event data.
}

proc btamplib::parseHciReset {{expected_status 0x0}} {

    # Parses HCI reset command and response event.
    # Reference: CR AMP HCI section 7.3.2 p97
    # HCI reset command
    # OCF = 0x0003
    # has no command parameters
    # returns 1 status byte

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciReset $::log_ip\
        expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = reset
    btamplib::checkHciCommand reset

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x3 0x3 $expected_status

    # Should be EOL
    btamplib::checkEol 4
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciSetEventMaskPage2] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the SetEventMaskPage2 event
    # data.
}

proc btamplib::parseHciSetEventMaskPage2 {{expected_status 0x0}} {

    # Parses HCI set event mask 2 command and response event.
    # Reference: CR AMP HCI section 7.3.69 p111
    # HCI Set Event Mask Page 2
    # OCF = 0x0063
    # has no command parameters
    # returns 1 status byte

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciSetEventMaskPage2\
        $::log_ip expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = event mask page 2
    btamplib::checkHciCommand "event mask page 2"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x3 0x63 $expected_status

    # Should be EOL
    btamplib::checkEol 4
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciShortRangeMode] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the ShortRangeMode event
    # data.
}

proc btamplib::parseHciShortRangeMode {{expected_status 0x0}} {

    # Reference: CR AMP HCI section 7.3.76 p117
    # OCF 0x006B
    # Command parameters:
    #     physical link handle = 1 byte
    #     short range mode = 1 byte, 0=off, 1=on
    # returns:
    # status = 1 byte, 0 = success

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciShortRangeMode\
        $::log_ip expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = Short Range Mode
    btamplib::checkHciCommand "Short Range Mode"

    # Check HCI command status
    btamplib::parseHciCommandStatus 0x1 0x6B $expected_status
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciShortRangeModeChangeComplete]
    # [opt expected_status=0x0] [opt expected_physical_link_handle=0x10]
    # [opt expected_mode=0x01]]

    # Reads the next HCI item, expecting it to be the ShortRangeModeChangeComplete
    # event data.
}

proc btamplib::parseHciShortRangeModeChangeComplete {{expected_status 0x0}\
    {expected_physical_link_handle 0x10} {expected_mode 0x01}} {

    # Reference: CR AMP HCI section 7.7.60 p154
    # OCF 0x004C
    #
    # returns:
    #     status = 1 byte, 0 = success
    #     physical link handle = 1 byte
    #     short range mode = 1 byte, 0=off, 1=on

    # Get next event only
    UTF::Message LOG $::log_ip "======== btamplib::parseHciShortRangeModeChangeComplete\
        $::log_ip expected_status=$expected_status\
        expected_physical_link_handle=$expected_physical_link_handle\
        expected_mode=$expected_mode"
    btamplib::getNextHciItem EVENT

    # Check event code = 0x4C
    btamplib::checkHciEventCode 0x4C

    # Check event text = Short Range Mode Change Complete
    btamplib::checkHciEventText "Short Range Mode Change Complete"

    # Check Status, offset = 0
    checkHciStatus $expected_status 0

    # Check physical link handle
    btamplib::checkOtherData physical_link_handle 1 1 oneof $expected_physical_link_handle

    # Check mode
    btamplib::checkOtherData mode 2 2 oneof $expected_mode

    btamplib::checkEol 3
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciVendorSpecificEvent]]

    # Reads the next HCI item, expecting it to be the VendorSpecificEvent event
    # data. Issues a Warning if it is a VendorSpecificEvent event, otherwise
    # backs the parser variables back so the event will be reparsed by the
    # next routine.
}

proc btamplib::parseHciVendorSpecificEvent { } {
    # Sometimes the drivers put out a Vendor Specific Event, usually for debugging
    # purposes.

    # Get Vendor Specific Event.
    btamplib::getNextHciItem EVENT

    # Check event = vendor specific
    if {![regexp -nocase {vendor\s*specific} $::hci_event_text]} {
        # This is not a VSE, so we roll the parser pointer back.
        # This allows whatever routine comes next to parse the
        # event we just looked at.
        if {[info exists ::hci_start_item_index]} {
            set ::log_line_cnt $::hci_start_item_index
        } else {
            set ::log_line_cnt 0
        }
        UTF::Message LOG $::log_ip "======== btamplib::parseHciVendorSpecificEvent\
            setting ::log_line_cnt=$::log_line_cnt"
        return
    }

    # So we really did get VSE.
    # Check event code = 0xff
    btamplib::checkHciEventCode 0xff

    # Log a Warning with the event data. No idea how to decode the data.
    btamplib::logWarning $::log_ip "btamplib::parseHciVendorSpecificEvent\
        hci_event_data: $::hci_event_data"
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciWriteConnectionAcceptTimeout]
    # [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the WriteConnectionAcceptTimeout
    # event data.
}

proc btamplib::parseHciWriteConnectionAcceptTimeout {{expected_status 0x0}} {

    # Reference: CR AMP HCI section 7.3.14 p100
    # OCF 0x0016
    # timeout = 2 bytes
    # returns:
    # status = 1 byte, 0 = success

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciWriteConnectionAcceptTimeout $::log_ip expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = Write Connection Accept Timeout
    btamplib::checkHciCommand "Write Connection Accept Timeout"

    # Check HCI command complete data
    btamplib::parseHciCommandComplete 0x3 0x16 $expected_status

    # Should be EOL
    btamplib::checkEol 4
    return
}

UTF::doc {
    # [call [cmd btamplib::parseHciWriteRemoteAmpAssoc]
    # [opt expected_physical_link_handle=0x10] [opt expected_status=0x0]]

    # Reads the next HCI item, expecting it to be the WriteRemoteAmpAssoc event
    # data.
}

proc btamplib::parseHciWriteRemoteAmpAssoc {{expected_physical_link_handle 0x10}\
     {expected_status 0x0}} {

    # Reference: CR AMP HCI section 7.5.9, p135
    # OCF = 0x000b
    # Physical Link Handle = 1 byte
    # Length So Far = 2 bytes
    # Amp Assoc Remaining Length = 2 bytes
    # Amp Assoc Fragment = 1 - 248 bytes

    # returns:
    # Status = 1 byte 0x0 = Success
    # Physical Link Handle = 1 byte

    # Get next event
    UTF::Message LOG $::log_ip "======== btamplib::parseHciWriteRemoteAmpAssoc\
        $::log_ip expected_physical_link_handle=$expected_physical_link_handle\
        expected_status=$expected_status"
    btamplib::getNextHciItem

    # Check HCI command = write reamote amp assoc
    btamplib::checkHciCommand "write remote amp assoc"

    # Parse common items
    btamplib::parseHciCommandComplete 0x5 0xb $expected_status

    # Physical Link Handle is byte 4
    btamplib::checkOtherData physical_link_handle 4 4 oneof $expected_physical_link_handle

    # Should be EOL
    btamplib::checkEol 5
    return
}

UTF::doc {
    # [call [cmd btamplib::translateHciCommands] [arg byte_num] [arg bit_mask]]

    # Gets the command text strings for [arg byte_num] and extracts the strings
    # corresponding to the [arg bit_mask]. Returns a string.
}

proc btamplib::translateHciCommands {byte_num bit_mask} {

    # Check calling data
    set byte_num [string trim $byte_num]
    set bit_mask [string trim $bit_mask]
    if {$byte_num == "" || $bit_mask == ""} {
        btamplib::logError "$::log_ip" "btamplib::translateHciCommands got null calling\
            parameters, byte_num=$byte_num bit_mask=$bit_mask"
        return
    }

    # If necessary, load command strings
    if {![info exists ::cmd_string(0)]} {
        puts "loading ::cmd_string array"
        # See CR AMP HCI Supported Commands, section 6.26 p67
        # The command names are coded as one string for each byte, Bit 0 ... Bit 7.
        set ::cmd_string(0) "Inquiry InquiryCancel PeriodicInquiryMode\
            ExitPeriodicInquiryMode CreateConnection Disconnect AddSCOConnection\
            CancelCreateConnection"
        set ::cmd_string(1) "Accept_Connection_Request RejectConnectionRequest\
            LinkKeyRequestReply LinkKeyRequestNegativeReply PINCodeRequestReply\
            PINCodeRequestNegativeReply ChangeConnectionPacketType AuthenticationRequest"
        set ::cmd_string(2) "SetConnectionEncryption ChangeConnectionLinkKey\
            MasterLinkKey RemoteNameRequest CancelRemoteNameRequest\
            ReadRemoteSupportedFeatures ReadRemoteExtendedFeatures\
            ReadRemoteVersionInformation"
        set ::cmd_string(3) "ReadClockOffset ReadLMPHandle Reserved Reserved Reserved\
            Reserved Reserved Reserved"
        set ::cmd_string(4) "Reserved HoldMode SniffMode ExitSniffMode\
            ParkState ExitParkState QoSSetup RoleDiscovery"
        set ::cmd_string(5) "SwitchRole ReadLinkPolicySettings WriteLinkPolicySettings\
            ReadDefaultLinkPolicySettings WriteDefaultLinkPolicySettings\
            FlowSpecification SetEventMark Reset"
        set ::cmd_string(6) "SetEventFilter Flush ReadPINType WritePINType CreateNewUnitKey\
            ReadStoredLinkKey WriteStoredLinkKey DeleteStoredLinkKey"
        set ::cmd_string(7) "WriteLocalName ReadLocalName ReadConnectionAcceptTimeout\
            WriteConnectionAcceptTimeout ReadPageTimeout WritePageTimeout\
            ReadScanEnable WriteScanEnable"
        set ::cmd_string(8) "ReadPageScanActivity WritePageScanActivity\
            ReadInquiryScanActivity WriteInquiryScanActivity Reserved Reserved\
            Reserved Reserved"
        set ::cmd_string(9) "ReadClassOfDevice WriteClassOfDevice ReadVoiceSetting\
            WriteVoiceSetting ReadAutomaticFlushTimeout WriteAutomaticFlushTimeout\
            ReadNumBroadcastRetransmissions WriteNumBroadcastRetransmissions"
        set ::cmd_string(10) "ReadHoldModeActivity WriteHoldModeActivity\
            ReadTransmitPowerLevel ReadSynchronousFlowControlEnable\
            WriteSynchronousFlowControlEnable SetHostControllerToHostFlowControl\
            HostBufferSize HostNumberOfCompletedPackets"
        set ::cmd_string(11) "ReadLinkSupervisionTimeout WriteLinkSupervisionTimeout\
            ReadNumberofSupportedIAC ReadCurrentIACLAP WriteCurrentIACLAP\
            Reserved Reserved ReadPageScanMode"
        set ::cmd_string(12) "WritePageScanMode SetAFHChannelClassification Reserved\
            Reserved ReadInquiryScanType WriteInquiryScanType ReadInquiryMode\
            WriteInquiryMode"
        set ::cmd_string(13) "ReadPageScanType WritePageScanType\
            ReadAFHChannelAssessmentMode WriteAFHChannelAssessmentMode Reserved\
            Reserved Reserved Reserved"
        set ::cmd_string(14) "Reserved Reserved Reserved ReadLocalVersionInformation\
            Reserved ReadLocalSupportedFeatures ReadLocalExtendedFeatures ReadBufferSize"
        set ::cmd_string(15) "ReadCountryCode ReadBDADDR ReadFailedContactCount\
            ResetFailedContactCount ReadLinkQuality ReadRSSI ReadAFHChannelMap ReadBDClock"
        set ::cmd_string(16) "ReadLoopbackMode WriteLoopbackMode EnableDeviceUnderTestMode\
            SetupSynchronousConnection AcceptSynchronousConnection\
            RejectSynchronousConnection Reserved Reserved"
        set ::cmd_string(17) "ReadExtendedInquiryResponse WriteExtendedInquiryResponse\
            RefreshEncryptionKey Reserved SniffSubrating ReadSimplePairingMode\
            WriteSimplePairingMode ReadLocalOOBData"
        set ::cmd_string(18) "ReadInquiryResponseTransmitPower\
            WriteInquiryTransmitPowerLevel ReadDefaultErroneousDataReporting\
            WriteDefaultErroneousDataReporting Reserved Reserved Reserved\
            IOCapabilityRequestReply"
        set ::cmd_string(19) "UserConfirmationRequestReply\
            UserConfirmationRequestNegativeReply UserPasskeyRequestReply\
            UserPasskeyRequestNegativeReply RemoteOOBDataRequestReply\
            WriteSimplePairingDebugMode EnhancedFlush RemoteOOBDataRequestNegativeReply"
        set ::cmd_string(20) "Reserved Reserved SendKeypressNotification\
            IOCapabilitiesResponseNegativeReply Reserved Reserved Reserved Reserved"
        set ::cmd_string(21) "CreatePhysicalLink AcceptPhysicalLinkRequest\
            DisconnectPhysicalLink CreateLogicalLink AcceptLogicalLink\
            DisconnectLogicalLink LogicalLinkCancel FlowSpecModify"
        set ::cmd_string(22) "ReadLogicalLinkAcceptTimeout WriteLogicalLinkAcceptTimeout\
            SetEventMaskPage2 ReadLocationData WriteLocationData ReadLocalAMPInfo\
            ReadLocalAMPASSOC WriteRemoteAMPASSOC"
        set ::cmd_string(23) "ReadFlowControlMode WriteFlowControlMode ReadDataBlockSize\
            Reserved Reserved EnableAMPReceiverReports AMPTestEnd AMPTestCommand"
        set ::cmd_string(24) "Reserved Reserved ReadBestEffortFlushTimeout\
            WriteBestEffortFlushTimeout ShortRangeMode Reserved Reserved Reserved"
    }

    # Look up the command string for the byte_num
    if {[info exists ::cmd_string($byte_num)]} {
        set cmd_string $::cmd_string($byte_num)
    } else {
        # For currently undefined bytes, use Reserved
        set cmd_string "Reserved Reserved Reserved Reserved Reserved Reserved Reserved Reserved"
    }

    # Sanity check on above data strings
    set len [llength $cmd_string]
    if {$len != 8} {
        btamplib::logError "$::log_ip" "btamplib::translateHciCommands len=$len NE 8,\
            byte_num=$byte_num cmd_string=$cmd_string"
    }

    # Check each bit in the bit_mask and save the corresponding command from cmds.
    set result ""
    set bit_mask [btamplib::cleanupHex $bit_mask]
    for {set i 0} {$i <= 7} {incr i} {
        # Set up a mask to match the i'th bit in bit_mask
        set bit_position_mask 0x01
        set bit_position_mask [expr $bit_position_mask << $i]
        set bit_position_mask [format "0x%02x" $bit_position_mask]
        set bit_value [expr $bit_mask & $bit_position_mask]

        # If bit was set, save the corresponding cmd string item
        set cmd ""
        if {$bit_value > 0} {
           set cmd [lindex $cmd_string $i]
           lappend result $cmd
        }
        # puts "i=$i bit_mask=$bit_mask bit_position_mask=$bit_position_mask\
        #    bit_value=$bit_value cmd=$cmd"
    }
    UTF::Message LOG "$::log_ip" "btamplib::translateHciCommands byte_num=$byte_num\
        cmd_string=$cmd_string bit_mask=$bit_mask result=$result"
    return $result
}

UTF::doc {
    # [call [cmd btamplib::testTranslateHciCommands]]

    # Test routine for above translateHciCommands routine.
}

proc btamplib::testTranslateHciCommands { } {
    # Test routine for the translateHciCommands routine
    for {set i 0} {$i <= 63} {incr i} {
        foreach j {0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80} {
            btamplib::translateHciCommands $i $j
        }
    }
    return
}

UTF::doc {
    # [call [cmd btamplib::translateHciStatus] [arg status]
    # [opt 00_mapping=Success]]

    # Translate the [arg status] hex code to a text status / error message
    # string. [opt 00_mapping] allows remapping status=0x00 to Pending or 
    # whatever is appropriate instead of Success. Returns the text status /
    # error string.
}

proc btamplib::translateHciStatus {status {00_mapping "Success"}} {
    # Translate hex code to text error message string.
    # Reference: CR AMP HCI section 1.3 p173
    # Also see: BT Core V3, Part D, p 337

    # NB: The routine below testTranslateHciStatus will exercise
    # all the error message strings.

    # If needed, load the known codes & error message strings into a global array.
    if {![info exists ::status_msgs(00)]} {
        # puts "loading ::status_msgs array"
        set ::status_msgs(00) "Success"
        set ::status_msgs(01) "Unknown HCI Command"
        set ::status_msgs(02) "Unknown Connection Identifier"
        set ::status_msgs(03) "Hardware Failure"
        set ::status_msgs(04) "Page Timeout"
        set ::status_msgs(05) "Authentication Failure"
        set ::status_msgs(06) "PIN or Key Missing"
        set ::status_msgs(07) "Memory Capacity Exceeded"
        set ::status_msgs(08) "Connection Timeout"
        set ::status_msgs(09) "Connection Limit Exceeded"
        set ::status_msgs(0a) "Synchronous Connection Limit To A Device Exceeded"
        set ::status_msgs(0b) "ACL Connection Already Exists"
        set ::status_msgs(0c) "Command Disallowed"
        set ::status_msgs(0d) "Connection Rejected due to Limited Resources"
        set ::status_msgs(0e) "Connection Rejected Due To Security Reasons"
        set ::status_msgs(0f) "Connection Rejected due to Unacceptable BD_ADDR"
        set ::status_msgs(10) "Connection Accept Timeout Exceeded"
        set ::status_msgs(11) "Unsupported Feature or Parameter Value"
        set ::status_msgs(12) "Invalid HCI Command Parameters"
        set ::status_msgs(13) "Remote User Terminated Connection"
        set ::status_msgs(14) "Remote Device Terminated Connection due to Low Resources"
        set ::status_msgs(15) "Remote Device Terminated Connection due to Power Off"
        set ::status_msgs(16) "Connection Terminated By Local Host"
        set ::status_msgs(17) "Repeated Attempts"
        set ::status_msgs(18) "Pairing Not Allowed"
        set ::status_msgs(19) "Unknown LMP PDU"
        set ::status_msgs(1a) "Unsupported Remote Feature / Unsupported LMP Feature"
        set ::status_msgs(1b) "SCO Offset Rejected"
        set ::status_msgs(1c) "SCO Interval Rejected"
        set ::status_msgs(1d) "SCO Air Mode Rejected"
        set ::status_msgs(1e) "Invalid LMP Parameters"
        set ::status_msgs(1f) "Unspecified Error"
        set ::status_msgs(20) "Unsupported LMP Parameter Value"
        set ::status_msgs(21) "Role Change Not Allowed"
        set ::status_msgs(22) "LMP Response Timeout"
        set ::status_msgs(23) "LMP Error Transaction Collision"
        set ::status_msgs(24) "LMP PDU Not Allowed"
        set ::status_msgs(25) "Encryption Mode Not Acceptable"
        set ::status_msgs(26) "Link Key Can Not be Changed"
        set ::status_msgs(27) "Requested QoS Not Supported"
        set ::status_msgs(28) "Instant Passed"
        set ::status_msgs(29) "Pairing With Unit Key Not Supported"
        set ::status_msgs(2a) "Different Transaction Collision"
        set ::status_msgs(2b) "Reserved"
        set ::status_msgs(2c) "QoS Unacceptable Parameter"
        set ::status_msgs(2d) "QoS Rejected"
        set ::status_msgs(2e) "Channel Classification Not Supported"
        set ::status_msgs(2f) "Insufficient Security"
        set ::status_msgs(30) "Parameter Out Of Mandatory Range"
        set ::status_msgs(31) "Reserved"
        set ::status_msgs(32) "Role Switch Pending"
        set ::status_msgs(33) "Reserved"
        set ::status_msgs(34) "Reserved Slot Violation"
        set ::status_msgs(35) "Role Switch Failed"
        set ::status_msgs(36) "Extended Inquiry Response Too Large"
        set ::status_msgs(37) "Secure Simple Pairing Not Supported By Host"
        set ::status_msgs(38) "Host Busy - Pairing"
        set ::status_msgs(39) "Connection Rejected due to No Suitable Channel Found"
        set ::status_msgs(3a) "Controller Busy"
    }

    # Massage status into 2 hex digits
    set status [string trim $status]
    set status [string tolower $status]
    regsub {^0x} $status "" status ;# ignore leading 0x
    if {[string length $status] == 1} {
        set status "0${status}" ;# add leading 0
    }
    # puts "btamplib::translateHciStatus status=$status"

    # Sometimes status=0x00 maps to another meaning besides Success.
    # We allow 00 to be remapped as needed. Always set 00 mapping so that
    # the value will revert to Success by default. 
    set ::status_msgs(00) $00_mapping

    # Look up the specified status hex code and return string.
    if {[info exists ::status_msgs($status)]} {
        return "$::status_msgs($status)"
    } else {
        btamplib::logError "$::log_ip" "btamplib::translateHciStatus status=$status\
            Not Defined!"
        return "Not Defined"
    }
}

UTF::doc {
    # [call [cmd btamplib::testTranslateHciStatus]]

    # Test routine for above translateHciStatus routine.
}

proc btamplib::testTranslateHciStatus { } {
    # Test routine for the translateHciStatus routine
    for {set i 0} {$i <= 255} {incr i} {
        set status [format "%02x" $i]
        set message [btamplib::translateHciStatus $status]
        UTF::Message LOG $::log_ip "i=$i status=$status message=$message"
    }
    return
}

#============================== Middle Level Parsing procs =======================

UTF::doc {
    # [call [cmd btamplib::readLogFile] [arg log] [opt ip=xyz]]

    # If [arg log] looks like pathname.txt|log, reads the file in one shot. Otherwise
    # takes [arg log] as the actual file contents. Always sanitizes contents, sets
    # pointers used by other parsing routine, stores file contents in $::log_string.
    # [opt ip=xyz] is used solely for log message consistency.
}

proc btamplib::readLogFile {log {ip xyz}} {
 
    # Normally the dbbview messages come back as a text string from the remote PC.
    # However the testGetNextHciItem routine needs to be able to read in a file.
    # So we allow both forms of data to be processed here. There may be a future need
    # to reprocess other log files from whereever, so being able to read in a file is
    # may have addtional future uses.

    # Determine if we have a logfile path or logfile contents
    set type contents ;# default
    set file ""
    regexp {\s*(\S+)\s*} $log - file
    if {([regexp {\.txt$} $file] || [regexp {\.log$} $file]) && [file exists $file]} {
        set type file
    }
    # UTF::Message LOG $ip "btamplib::readLogFile type=$type file=$file"

    # Initialize counters, etc
    set ::log_line_cnt 0
    set ::log_line_max 0
    set ::log_ip $ip
    set ::log_string ""

    # If necessary, read in the logfile path
    UTF::Message LOG $ip "==================================================================================================="
    if {$type == "contents"} {
        UTF::Message LOG $ip "= btamplib::readLogFile messages"
        set results $log

    } else {
        UTF::Message LOG $ip "= btamplib::readLogFile reading $file"

        # Open the file for reading.
        set catch_resp [catch "set in \[open \"$file\" r\]" catch_msg]
        if {$catch_resp != 0} {
            btamplib::logError $ip "btamplib::readLogFile file $file not opened: $catch_msg"
            UTF::Message LOG $ip "==================================================================================================="
            return
        }
        # UTF::Message LOG $ip "file=$file ip=$ip in=$in"

        # Read the whole file in one shot & close file
        set catch_resp [catch "set results \[read $in\]" catch_msg]
        catch "close $in"
        if {$catch_resp != 0} {
            btamplib::logError $ip "btamplib::readLogFile file $file not read: $catch_msg"
            UTF::Message LOG $ip "==================================================================================================="
            return
        }
    }

    # Remove square & curly brackets to simplify parsing later on.
    regsub -all {\[} $results "" results
    regsub -all {\]} $results "" results
    regsub -all {\{} $results "" results
    regsub -all {\}} $results "" results

    # Split results by newline & save results in ::log_string.
    set results [split $results "\n"]
    set ::log_string $results
    set ::log_line_max [llength $results]
    # puts "results=$results"
    UTF::Message LOG $ip "= log_line_max=$::log_line_max"
    UTF::Message LOG $ip "==================================================================================================="

    # Put a copy of the file into the UTF log file.
    foreach line $results {
        UTF::Message LOG "$ip" "$line"
    }
    return
}

UTF::doc {
    # [call [cmd btamplib::getNextHciItem] [opt args=null]]

    # Parses HCI data stored in $::log_string.[para]

    # When args=null, we expect an HCI Command and an HCI Event.[para]

    # If args=EOF, then we expect there are no more HCI commands or 
    # events or data left in the log. This allows us to verify that there
    # are no unexpected items in the logfile without our knowlege.[para]

    # If args=EVENT, then we look only for an event response, and no HCI 
    # Command. For some commands, there is an immediate status response,
    # followed much later on by the final command result.[para]

    # If args=ACLDATA, we look only for ACL data **FROM** the driver. Data
    # sent **TO** the driver is of no interest.
}

proc btamplib::getNextHciItem {args} {

    # NB: When you change this routine, be sure to run: testgetNextHciItem
    # which runs it thru some tough parsing tests.

    # Check args values
    set args [string trim $args]
    set args [string toupper $args]
    regsub -all {\{} $args "" args
    regsub -all {\}} $args "" args
    # puts "args=$args"
    if {$args != "" && $args != "ACLDATA" && $args != "EOF" && $args != "EVENT"} {
        btamplib::logError "$::log_ip" "btamplib::getNextHciItem invalid args=$args"
    }

    # Parsing initialization
    # UTF::Message LOG $::log_ip "btamplib::getNextHciItem log_ip=$::log_ip\
    # log_line_cnt=$::log_line_cnt log_line_max=$::log_line_max"
    set ::hci_acl_data ""
    set ::hci_acl_flags ""
    set ::hci_acl_handle ""
    set ::hci_acl_line_start ""
    set ::hci_acl_line_end ""
    set ::hci_cmd_line_num ""
    set ::hci_cmd_text ""
    set ::hci_event_line_start ""
    set ::hci_event_line_end ""
    set ::hci_event_text ""
    set ::hci_event_data ""
    set ::hci_event_code ""
    set plen 0

    # Start parsing string at position indicated by ::line_cnt
    for {set i $::log_line_cnt} {$i < $::log_line_max} {incr i} {

        # Get next line in string
        set line [lindex $::log_string $i]
        set line [string trim $line]
        # puts "btamplib::getNextHciItem i=$i line=$line"

        # Skip blank lines
        if {$line == ""} {
            # puts "skipping blank line i=$i"
            continue
        }

        # Windows: remove leading timestamp
        # puts "btamplib::getNextHciItem i=$i line=$line"
        regsub {^\d\d:\d\d:\d\d} $line "" line
        # Linux: remove leading date host kernel:
        regsub {^\S+\s+\d+\s+\d\d:\d\d:\d\d\s+\S+\s+kernel:} $line "" line
        set line [string trim $line]
        # puts "btamplib::getNextHciItem i=$i line=$line"
        
        # All Vendor Specific Events are logged as a Warning
        if {[regexp -nocase {HCI\s+Event:\s+Vendor\s+Specific} $line]} {
            btamplib::logWarning "$::log_ip" "btamplib::getNextHciItem got\
                Vendor Specific Event line [expr $i + 1]"
            continue
        }

        # Look for an HCI command. This can be used by other
        # parsing routines to help validate the response event data.
        if {[regexp -nocase {(HCI\s+Command:.*)plen} $line - temp]} {
            # If we already have an event or acl_handle, we are done.
            if {$::hci_event_text != "" || $::hci_acl_handle != "" } {
                btamplib::logWarning "$::log_ip" "btamplib::getNextHciItem did\
                    not get the expected blank line terminating the data"
                btamplib::getNextExitRtn $i $plen $args
                return
            }

            # Check for consecutive HCI commands.
            if {$::hci_cmd_text != ""} {
                btamplib::logError "$::log_ip" "btamplib::getNextHciItem found\
                    another HCI command: $temp before an Event, prior HCI\
                    command: $::hci_cmd_text"
            }

            # Save current HCI command
            set ::hci_cmd_text $temp
            set ::hci_cmd_line_num [expr $i + 1]
            set ::hci_start_item_index $i
            # puts "btamplib::getNextHciItem i=$i hci_cmd_text=$::hci_cmd_text"
            continue
        }

        # Look for an HCI event text string & packet length
        if {[regexp -nocase {(HCI\s+Event:.*)plen\s+(\d+)} $line - temp1 temp2]} {
            if {$::hci_event_text != "" || $::hci_acl_handle != "" } {
                # This handles the case of missing the normal
                # blank terminating line.
                btamplib::logWarning "$::log_ip" "btamplib::getNextHciItem did not\
                    get the expected blank line terminating the data"
                btamplib::getNextExitRtn $i $plen $args
                return
            }

            # Save current event info
            # puts "getNextHciItem i=$i hci_event_text=$::hci_event_text plen=$plen"
            set ::hci_event_text $temp1
            set plen $temp2
            set ::hci_event_line_start [expr $i + 1]
            set ::hci_event_line_end $::hci_event_line_start
            set ::hci_start_item_index $i

            # Extract event code from end of event text.
            set temp [split $::hci_event_text "("]
            set temp [lindex $temp 1]
            set temp [string trim $temp]
            set temp [string tolower $temp]
            regsub -all {\)} $temp "" ::hci_event_code
            # puts "btamplib::getNextHciItem hci_event_code=$::hci_event_code"
            continue
        }

        # Look for ACL data **FROM** the driver. The ">" indicates FROM.
        if {[regexp -nocase {>\s+ACL\s+data:\s+handle\s+([0-9,a-f,x]+)\s+flags\s+([0-9,a-f,x]+)\s+dlen\s+(\d+)}\
            $line - temp1 temp2 temp3]} {
            if {$::hci_event_text != "" || $::hci_cmd_text != "" } {
                # This handles the case of missing the normal
                # blank terminating line.
                btamplib::logWarning "$::log_ip" "btamplib::getNextHciItem did not\
                    get the expected blank line terminating the data"
                btamplib::getNextExitRtn $i $plen $args
                return
            }

            # Save current ACL data info
            set ::hci_acl_handle $temp1
            set ::hci_acl_flags $temp2
            set plen $temp3
            set ::hci_acl_line_start [expr $i + 1]
            set ::hci_acl_line_end $::hci_acl_line_start
            set ::hci_start_item_index $i
            # puts "btamplib::getNextHciItem i=$i acl_data\
            #     hci_acl_handle=$::hci_acl_handle hci_acl_flags=$::hci_acl_flags\
            #     plen=$plen"
            continue
        }

        # We can get a timer recovered msg in the middle of an event. These
        # come from the wltray.exe. Even if the wltray service is turned off,
        # there may be a registry entry that start up the wltray.exe.
        # So search the registry and rename the key to get rid of it, then
        # stop the process via task manager.
        if {[regexp -nocase {\d\d:\d\d:\d\d.*timer} $line]} {
            btamplib::logWarning "$::log_ip" "btamplib::getNextHciItem\
                i=$i ignoring: $line"
            continue
        }

        # If we have started to collect Event data or ACL data and we
        # find a wlN: message, then we are done collecting data.
        if {($::hci_event_text != "" || $::hci_acl_handle != "") &&
            [regexp -nocase {\swl\d:\s} $line]} {
            btamplib::logWarning "$::log_ip" "btamplib::getNextHciItem found wlN:\
                message while collecting data"
            btamplib::getNextExitRtn $i $plen $args
            return
        }

        # If we found an Event, then start collecting the Event data.
        if {$::hci_event_text != ""} {
            # Now we get the associated hex data string(s) for this HCI event.
            # We expect one or more lines of associated hex data, terminated by
            # a line with no hex data.
            set line [split $line ":"]
            set data [lindex $line 1]
            set data [string trim $data]
            # puts "btamplib::getNextHciItem i=$i data=$data"
    
            # Save event data, stop parsing when data is null.
            if {$data != ""} {
                append ::hci_event_data "$data "
                set ::hci_event_line_end [expr $i + 1]
                continue
            }
        
            # The blank line is the end of the event data.
            btamplib::getNextExitRtn $i $plen $args
            return
        }

        # If we found an acl_handle, then start collecting the Event data.
        if {$::hci_acl_handle != ""} {
            # Now we get the associated hex data string(s) for this ACL data.
            # We expect one or more lines of associated hex data, terminated by
            # a line with no hex data.
            set line [split $line ":"]
            set data [lindex $line 1]
            set data [string trim $data]
            # puts "btamplib::getNextHciItem i=$i data=$data"
    
            # Save ACL data, stop parsing when data is null.
            if {$data != ""} {
                append ::hci_acl_data "$data "
                set ::hci_acl_line_end [expr $i + 1]
                continue
            }
        
            # The blank line is the end of the ACL data.
            btamplib::getNextExitRtn $i $plen $args
            return
        }
    }

    # We hit EOF.
    if {$::hci_cmd_text != "" || $::hci_event_text != "" || $::hci_event_data != "" ||\
        $::hci_acl_handle != "" || $::hci_acl_data != ""} {
        btamplib::logWarning "$::log_ip" "btamplib::getNextHciItem hit EOF\
            while collecting data!"

    } else {
        if {$args == "EOF"} {
            UTF::Message LOG "$::log_ip" "btamplib::getNextHciItem hit EOF as expected"
        } else {
            btamplib::logError "$::log_ip" "btamplib::getNextHciItem hit EOF unexpectedly!"
        }
    }
    btamplib::getNextExitRtn $i $plen $args
    return
}

UTF::doc {
    # [call [cmd btamplib::getNextExitRtn] [arg i] [arg plen] [opt args=null]]

    # Handles final checks on HCI data found by proc btamplib::getNextHciItem.
    # Normally only called by proc btamplib::getNextHciItem.
}

proc btamplib::getNextExitRtn {i plen args} {

    # Clean up args
    regsub -all {\}} $args "" args
    regsub -all {\{} $args "" args
    set args [string trim $args]
    set args [string toupper $args]
    # puts "btamplib::getNextExitRtn i=$i plen=$plen args=$args"

    # Display summary info.
    set data_len 0
    if {$::hci_event_text != ""} {
        UTF::Message LOG $::log_ip "btamplib::getNextExitRtn Cmd: line\
            $::hci_cmd_line_num $::hci_cmd_text Event: lines\
            $::hci_event_line_start-$::hci_event_line_end\
            $::hci_event_text"
        set data_len [llength $::hci_event_data]

    } elseif {$::hci_acl_handle != ""} {
        UTF::Message LOG $::log_ip "btamplib::getNextExitRtn ACL Data:\
            lines $::hci_acl_line_start-$::hci_acl_line_end\
            handle=$::hci_acl_handle flags=$::hci_acl_flags plen=$plen"
        set data_len [llength $::hci_acl_data]
    }

    # Check actual data length matches plen.
    if {$data_len != $plen} {
        btamplib::logError $::log_ip "btamplib::getNextExitRtn data_len=$data_len NE\
            plen=$plen data=$::hci_event_data $::hci_acl_data"
    }

    # Update log_line_cnt for next run.
    set ::log_line_cnt $i

    # Check if we were expecting an HCI command or not.
    if {$args == "" && $::hci_cmd_text == ""} {
        btamplib::logError "$::log_ip" "btamplib::getNextExitRtn did NOT find\
            HCI Command as expected, args=$args"
    }
    if {($args == "EVENT" || $args == "ACLDATA") && $::hci_cmd_text != ""} {
        btamplib::logError "$::log_ip" "btamplib::getNextExitRtn found HCI\
            Command line $::hci_cmd_line_num $::hci_cmd_text, was NOT\
            expecting an HCI Command, args=$args"
    }

    # If args=null|EVENT and we dont have an new event, data or code, this is an error.
    if {($args == "" || $args == "EVENT") &&\
        ($::hci_event_text == "" || $::hci_event_data == "" || $::hci_event_code == "")} {
        btamplib::logError $::log_ip "btamplib::getNextExitRtn did NOT find\
            HCI Event as expected, $::hci_cmd_text $::hci_event_data\
            $::hci_event_code args=$args"
    }

    # If args=ACLDATA and we have an new command or event, data or code, this is an error.
    if {$args == "ACLDATA" && ($::hci_cmd_text != "" || $::hci_event_text != "" ||\
        $::hci_event_data != "" || $::hci_event_code != "")} {
        btamplib::logError $::log_ip "btamplib::getNextExitRtn got expected\
            HCI Command / Event, $::hci_cmd_text $::hci_event_text $::hci_event_data\
            $::hci_event_code args=$args"
    }

    # Did we get ACL data when expected?
    if {$args == "ACLDATA" && ($::hci_acl_handle == "" ||\
        $::hci_acl_flags == ""  || $::hci_acl_data == "")} {
        btamplib::logError $::log_ip "btamplib::getNextExitRtn did NOT find ACL\
            Data as expected, $::hci_acl_handle $::hci_acl_flags $::hci_acl_data\
            args=$args"
    }

    # Did we hit EOF by as expected or by accident? 
    # This issue is handled at end of proc btamplib::getNextHciItem.

    # If args=EOF and we have any new HCI info, this is an error.
    if {$args == "EOF" && ($::hci_cmd_text != "" || $::hci_event_text != "" ||\
        $::hci_event_data != "" || $::hci_event_code != "" ||\
        $::hci_acl_handle != "" || $::hci_acl_flags != "" || $::hci_acl_data != "")} {
        btamplib::logError $::log_ip "btamplib::getNextExitRtn $::hci_cmd_text\
            $::hci_event_text $::hci_event_data  $::hci_event_code\
            $::hci_acl_handle $::hci_acl_flags $::hci_acl_data\
            is NOT expected, EOF was expected!"
    }
    return
}

UTF::doc {
    # [call [cmd btamplib::testGetNextHciItem]]

    # Stress test for proc btamplib::getNextHciItem.[para]

    # Returns: failure count.
}

proc btamplib::testGetNextHciItem { } {

    # Load the parsing stress test file
    UTF::Message LOG $::log_ip "\n\n\n\nbtamplib::testgetNextHciItem"
    set ::errors 0
    readLogFile bin/hci_parser_stress.txt abc
    if {$::errors != 0} { 
        return $::errors
    }

    # Define test hex groups: cmd args expected_errors_cnt expected_warnings_cnt
    # event_data_len acl_data_len
    set test_list ""
    lappend test_list btamplib::getNextHciItem "" 0 0 4 0       ;# Test  1: command & event 
    lappend test_list btamplib::getNextHciItem "" 1 0 6 0       ;# Test  2: Two commands then event
    lappend test_list btamplib::getNextHciItem event 1 1 0 0    ;# Test  3: event with no data
    lappend test_list btamplib::getNextHciItem event 0 0 4 0    ;# Test  4: event only
    lappend test_list btamplib::getNextHciItem "" 1 1 4 0       ;# Test  5: command missing for event, warning for Vendor Specific Event
    lappend test_list btamplib::getNextHciItem event 0 0 34 0   ;# Test  6: event only
    lappend test_list btamplib::getNextHciItem event 0 1 40 0   ;# Test  7: event missing terminator line
    lappend test_list btamplib::getNextHciItem "" 0 1 4 0       ;# Test  8: event missing the terminator line
    lappend test_list btamplib::getNextHciItem eof 1 0 5 0      ;# Test  9: event after missing the terminator
    lappend test_list btamplib::getNextHciItem event 1 0 11 0   ;# Test 10: event with wrong plen
    lappend test_list btamplib::getNextHciItem acldata 0 0 0 44 ;# Test 11: data
    lappend test_list btamplib::getNextHciItem acldata 1 0 0 10 ;# Test 12: data with wrong plen
    lappend test_list btamplib::getNextHciItem acldata 0 1 0 10 ;# Test 13: data terminated by wlN: output
    lappend test_list btamplib::getNextHciItem event 0 1 16 0   ;# Test 14: event missing terminator line, hits EOF
    lappend test_list btamplib::getNextHciItem eof 0 0 0 0      ;# Test 15: EOF was expected
    lappend test_list btamplib::getNextHciItem "" 3 0 0 0       ;# Test 16: EOF not expected
    lappend test_list btamplib::getNextHciItem event 2 0 0 0    ;# Test 17: EOF not expected
    lappend test_list btamplib::getNextHciItem acldata 2 0 0 0  ;# Test 18: EOF not expected

    # Run parser tests
    set fail 0
    set pass 0
    set i 0
    # puts "test_list=$test_list"
    foreach {cmd args err warn edl adl} $test_list {

        # Setup for next test
        set ::errors 0
        set ::warnings 0
        incr i 
        UTF::Message LOG $::log_ip "Test $i: cmd=$cmd args=$args expected_errors=$err\
            expected_warnings=$warn event_data_len=$edl acl_data_len=$adl"

        # Run test.
        $cmd $args
        set event_len [llength $::hci_event_data]
        set acl_len [llength $::hci_acl_data]
        UTF::Message LOG $::log_ip "HCI event code: $::hci_event_code len: $event_len data: $::hci_event_data"
        UTF::Message LOG $::log_ip "HCI acl handle: $::hci_acl_handle flags: $::hci_acl_flags len: $acl_len data: $::hci_acl_data"

        # Did we get expected results?
        if {$::errors == $err && $::warnings == $warn && $event_len == $edl &&\
            $acl_len == $adl} {
            UTF::Message LOG $::log_ip "Test $i: Pass\n"
            incr pass
        } else {
            UTF::Message LOG $::log_ip "Test $i: Fail\n"
            incr fail
        }
    }

    # Cleanup
    set ::errors 0
    set ::error_list ""
    set ::warnings 0
    set ::warning_list ""
    UTF::Message LOG $::log_ip "\n\nbtamplib::testgetNextHciItem Pass=$pass Fail=$fail"
    return $fail
}

UTF::doc {
    # [call [cmd btamplib::parseTlvData] [arg tlv_string] [arg tlv_array]]

    # Parses the Type Length Value data in the space separated hex string 
    # [arg tlv_string] and stores it in the array named [arg tlv_array].
    # This makes it easy for the calling routine to get the parsed data.
}

proc btamplib::parseTlvData {tlv_string tlv_array} {

    # We save data in array specified by calling routine. 
    upvar $tlv_array local_array

    # Parse tlv_string
    set cnt 0
    set i 0
    set tlv_string_len [llength $tlv_string]
    while {$i < $tlv_string_len} {

        # Get next type, 1 byte field
        set type [lindex $tlv_string $i] 

        # Get next length, 2 byte field
        set j [expr $i + 1]
        set k [expr $j + 1]
        set length [lrange $tlv_string $j $k]
        set length [swapHexOrder $length]
        set length [string tolower $length]
        if {$length == "0x"} {
            set length "0x00"
        }
        # puts "length=$length"
        
        # Get next value
        set l [expr $k + 1]
        set m [expr $l + $length - 1]
        set value [lrange $tlv_string $l $m]
        # puts "i=$i type=$type j=$j k=$k length=$length d.[hex2decimal $length]\
        #    l=$l m=$m value=$value"
        
        # Move pointer ahead to start of next TLV
        set i [expr $m + 1]

        # Did we get the correct amount of data?
        set len [llength $value]
        if {$len != $length} { 
            btamplib::logError $::log_ip "btamplib::parseTlvData i=$i\
                type=$type value=$value len=$len NE length=$length"
        }
        
        # Have we already saved this type before?
        if {[info exists local_array($type)]} {
            btamplib::logError $::log_ip "btamplib::parseTlvData i=$i\
                type=$type duplicate TLV discarding type=$type\
                length=$length value=$value"
            continue
        }

        # Save the type & value
        UTF::Message LOG $::log_ip "btamplib::parseTlvData saving type=$type\
            length=$length d.[hex2decimal $length] value=$value"
        set local_array($type) $value
        incr cnt
    }

    # Return count of unique TLV saved.
    return $cnt
}

#============================== Low Level Parsing procs ==========================

UTF::doc {
    # [call [cmd btamplib::ascii2hex] [arg ascii_str]]

    # Converts ascii string to hex string.[para]

    # Returns: hex_string
}

proc btamplib::ascii2hex {ascii_str} {

    # NB: format only seems to convert one ascii byte at a time
    # to hex. So we process multiple items in list in a loop.
    set hex_list ""
    set i 0
    while { 1 } {

        # Get next character
        set char [string range $ascii_str $i $i]
        if {$char == ""} {
            break
        }
        # puts "i=$i char=$char"

        # Convert ascii character to hex.
        set hex "??"
        set catch_resp [catch "set j \[scan \"$char\" \"%c\"\]" catch_msg]
        # puts "ascii2hex i=$i char=$char j=$j"
        set catch_resp [catch "set hex \[format \"%x\" $j\]" catch_msg]
        set hex [btamplib::cleanupHex $hex]
        # puts "ascii2hex i=$i char=$char hex=$hex"
        append hex_list "$hex "
        incr i
    }
    return $hex_list
}

UTF::doc {
    # [call [cmd btamplib::checkHexString] [arg name] [arg hex_string]
    # [arg check] [arg args]]

    # Checks the specified [arg hex_string] is in the specified range and / or
    # one of the list of specified values. [arg name] parameter is used only 
    # for logging purposes.[para] 

    # For check=range, args=expected_low expected_high [para]

    # For check=oneof, args=value1 ... valueN [para]

    # For check=combo, args=expected_low expected_high value1 ... valueN [para]

    # Returns: hex_string
}

proc btamplib::checkHexString {name hex_string check args} {
    # puts "btamplib::checkHexString $name $hex_string $check $args"

    # Validate the check that we will do.
    set check [string trim $check]
    set check [string tolower $check]
    if {$check != "range" && $check != "oneof" && $check != "combo"} {
        btamplib::logError $::log_ip "btamplib::checkString $name invalid\
           check=$check, must be: range, oneof, combo"
        return
    }
    
    # Clean up hex string, convert to decimal.
    set hex_string [btamplib::cleanupHex $hex_string]
    set dec_data [btamplib::hex2decimal $hex_string]

    # If appropriate, do range test.
    set args [string tolower $args]
    if {$check == "range" || $check == "combo"} {

        # Parse calling args for range limits.
        set expected_low [lindex $args 0]
        set expected_low [string trim $expected_low]
        set expected_high [lindex $args 1]
        set expected_high [string trim $expected_high]
        if {$expected_low == "" || $expected_high == ""} {
            btamplib::logError $::log_ip "btamplib::checkString $name null range value\
               expected_low=$expected_low expected_high=$expected_high"
            return $hex_string
        }

        # Do range test.
        if {$hex_string >= $expected_low && $hex_string <= $expected_high} {
            UTF::Message LOG $::log_ip "btamplib::checkString $name hex_string=$hex_string d.$dec_data\
                in range: $expected_low - $expected_high"
            return $hex_string
        }

        # NB: At this point, we didnt match the specified range. If combo was
        # specified, perhaps we will match one of the specific values in the
        # oneof list. So we keep going...
    }

    # If appropriate, do oneof test.
    if {$check == "oneof" || $check == "combo"} {

        # Parse calling args for oneof value(s)
        if {$check == "oneof"} {
            set oneof_list $args ;# no range values, all oneof values
        } else {
            set oneof_list [lrange $args 2 end] ;# skip the range values
        }
        set oneof_list [string trim $oneof_list]
        if {$oneof_list == ""} {
            btamplib::logError  $::log_ip "btamplib::checkString $name no\
                oneof values specified!"
            return $hex_string
        }

        # Do oneof test.
        if {[lsearch -exact $oneof_list $hex_string] >= 0 ||\
            [lsearch -exact $oneof_list $dec_data] >= 0} {
            UTF::Message LOG $::log_ip "btamplib::checkString $name hex_string=$hex_string\
                d.$dec_data got oneof expected values: $oneof_list"
            return $hex_string
        }
    }

    # No match occurred ==> error
    btamplib::logError $::log_ip "btamplib::checkString $name hex_string=$hex_string\
        d.$dec_data NO MATCH $check $args"
    return $hex_string
}

UTF::doc {
    # [call [cmd btamplib::checkTextString] [arg name] [arg text_string] [arg args]]

    # Checks if the specified [arg text_string] is an exact match for any one
    # of the [arg args]. [arg name] parameter is used only for logging purposes.
    # Returns null.
}

proc btamplib::checkTextString {name text_string args} {

    # Check we got one of the expected string(s) in args
    foreach str $args {
        if {$text_string == $str} {
            UTF::Message LOG $::log_ip "btamplib::checkString $name text_string=$text_string got oneof: $args"
            return
        }
    }

    # No match ==> error
    btamplib::logError $::log_ip "btamplib::checkString $name text_string=$text_string\
        NOT matched oneof: $args"
    return
}


UTF::doc {
    # [call [cmd btamplib::checkOtherData] [arg name] [arg start]
    # [arg end] [arg check] [arg args]]

    # Looks at the data stored in $::hci_event_data, and extracts
    # data starting at index [arg start] up to and including index [arg end].
    # Checks the extracted data is in the specified range and / or
    # one of the list of specified values. [arg name] parameter is used only 
    # for logging purposes.[para] 

    # For check=range, args=expected_low expected_high [para]

    # For check=oneof, args=value1 ... valueN [para]

    # For check=combo, args=expected_low expected_high value1 ... valueN [para]

    # Returns: hex_string
}

proc btamplib::checkOtherData {name start end check args} {

    # Extract data range
    set raw_hex [lrange $::hci_event_data $start $end]
    set raw_hex [string trim $raw_hex]
    if {$raw_hex == ""} {
        btamplib::logError $::log_ip "btamplib::checkOtherData $name (${start}-${end})\
           No data found!"
        return
    }

    # Did we get correct number of bytes?
    set expected_len [expr $end - $start + 1]
    set actual_len [llength $raw_hex]
    if {$expected_len != $actual_len} {
        btamplib::logError "$::log_ip" "btamplib::checkOtherData $name (${start}-${end})\
            got $actual_len bytes, expected $expected_len bytes"
    }

    # Swap byte order, 
    set swapped_hex [btamplib::swapHexOrder $raw_hex]

    # Handoff to checkString
    UTF::Message LOG $::log_ip "btamplib::checkOtherData $name bytes(${start}-${end})\
        raw_hex=$raw_hex  swapped_hex=$swapped_hex"
    set resp [eval btamplib::checkHexString $name $swapped_hex $check $args]

    # Return the response.
    return $resp
}

UTF::doc {
    # [call [cmd btamplib::cleanupHex] [arg string]]

    # Cleans up a hex [arg string] by removing whitespace, converting
    # to lower case and adding a leading 0x if necessary. Returns hex string.
}

proc btamplib::cleanupHex {string} {

    # Remove whitespace
    regsub -all {\s} $string "" string

    # Convert to lower case
    set string [string tolower $string]

    # Temporarily remove leading 0x, if any.
    regsub -nocase {^0x} $string "" string

    # Ensure at least 2 hex digits.
    if {[string length $string] == 1} {
        set string "0${string}"
    } elseif {[string length $string] == 0} {
        set string "00"
    }

    # Add back leading 0x
    set string "0x${string}"

    # Return string
    return $string
}

UTF::doc {
    # [call [cmd btamplib::decimal2hex] [arg decimal_list]]

    # Convert list of decimal or hex numbers to list of hex values.
    # Returns hex string.
}

proc btamplib::decimal2hex {decimal_list} {
    set hex_data ""
    foreach decimal $decimal_list {
        set hex "??"
        if {[regexp {^[0-9\-]+$} $decimal]} {
            # Handle integer numbers
            # puts "integer decimal=$decimal"
            set catch_resp [catch "set hex \[format \"0x%x\" \"$decimal\"\]" catch_msg]

        } elseif {[regexp -nocase {^[0-9a-fx]+$} $decimal]} {
            # Handle numbers that are already hex.
            # puts "already hexidecimal decimal=$decimal"
            set hex [btamplib::cleanupHex $decimal]
        }
        lappend hex_data $hex
    }
    return $hex_data
}

UTF::doc {
    # [call [cmd btamplib::hex2ascii] [arg hex_list]]

    # Converts [arg hex_list] to ascii string. 
}

proc btamplib::hex2ascii {hex_list} {

    # NB: format only seems to convert one hex byte at a time
    # to ascii. So we process multiple items in list in a loop.
    set ascii_str ""
    foreach hex $hex_list {
        set hex [btamplib::cleanupHex $hex]

        # Convert hex to ascii character if possible.
        set ascii_char "?"
        set catch_resp [catch "set ascii_char \[format \"%c\" $hex\]" catch_msg]
        # puts "hex2ascii hex=$hex ascii_char=$ascii_char"
        set ascii_str "${ascii_str}${ascii_char}"
    }
    return $ascii_str
}

UTF::doc {
    # [call [cmd btamplib::hex2decimal] [arg hex]]

    # Converts a single [arg hex] number to decimal value.
}

proc btamplib::hex2decimal {hex} {
    set hex [btamplib::cleanupHex $hex]
    set dec_data "??"
    set catch_resp [catch "set dec_data \[format \"%d\" \"$hex\"\]" catch_msg]
    return $dec_data
}

UTF::doc {
    # [call [cmd btamplib::ishex] [arg hex]]

    # Checks single [arg hex] number for 0xhhhh format.
}

proc btamplib::ishex {hex} {
    if {[regexp -nocase {^0x[\da-f]{4}$} $hex]} {
        return 1
    } else {
        return 0
    }
}

UTF::doc {
    # [call [cmd btamplib::mac2hex] [arg mac]]

    # Converts 12 mac address digits to a space seperated hex string.
}

proc btamplib::mac2hex {mac} {
    regsub -all {:} $mac "" mac ;# ignore emebeded colons
    regsub -all {\-} $mac "" mac ;# ignore emebeded hyphens

    set length [string length $mac]
    if {$length != 12} {
        btamplib::logError "$::log_ip" "btamplib::mac2hex mac=$mac length=$length NE 12"
        return
    }
    set mac [string tolower $mac]
    if {![regexp {^[0-9,a-f]+$} $mac]} {
        btamplib::logError "$::log_ip" "btamplib::mac2hex invalid character in mac=$mac"
        return
    }
    set b1 [string range $mac 0 1]
    set b2 [string range $mac 2 3]
    set b3 [string range $mac 4 5]
    set b4 [string range $mac 6 7]
    set b5 [string range $mac 8 9]
    set b6 [string range $mac 10 11]
    set result "0x$b1 0x$b2 0x$b3 0x$b4 0x$b5 0x$b6"
    return $result
}

UTF::doc {
    # [call [cmd btamplib::swapHexOrder] [arg string]]

    # Swap byte order of hex data [arg string] to be big endian.
    # Basically we read backwards in groups of 4 bytes.
    # Eg: 0xB1 0xB2 0xB3 0xB4 becomes: 0xB4B3B2B1
}

proc btamplib::swapHexOrder {string} {

    # Clean up input string, remove all 0x, convert to lower case.
    regsub -all -nocase {0x} $string "" string
    set string [string tolower $string]

    # If tokens in string are space separated, make sure each one
    # has 2 digits. We may get single digit hex data.
    set result1 ""
    foreach num $string {
        if {[string length $num] == 1} {
            set num "0$num"
        }
        append result1 $num
    }

    # Remove all whitespace.
    regsub -all {\s} $result1 "" result1
    set cnt [string length $result1]
    # puts "result1=$result1 cnt=$cnt"

    # Swap byte order within string of hex digits.
    set result2 "0x"
    for {set i 0} {$i < $cnt} {incr i 8} {
        set j [expr $i + 7]
        set 4Bytes [string range $result1 $i $j]
        set b1 [string range $4Bytes 0 1]
        set b2 [string range $4Bytes 2 3]
        set b3 [string range $4Bytes 4 5]
        set b4 [string range $4Bytes 6 7]
        # puts "i=$i j=$j 4Bytes=$4Bytes b1=$b1 b2=$b2 b3=$b3 b4=$b4"
        append result2 "${b4}${b3}${b2}${b1}"
    }

    # Return result2
    # puts "result2=$result2"
    return $result2
}

# ================================================================================
UTF::doc {
    # [list_end]
    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# Output manpage
UTF::man
