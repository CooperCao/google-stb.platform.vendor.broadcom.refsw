# Simple script to keep restarting DbgView every few seconds.
# Leaves DbgView down for a guaranteed short period so that BTAMP test
# scripts can rename the exiting logfile and get the desired results.

# Updated by John Brearley 2009/7/10

#================== global variables ==================================================
set dbgview_exe "c:\\\\program files\\\\dbgview\\\\dbgview.exe" ;# dgbview program location
set dbgview_log "c:\\\\tools\\\\win32\\\\var\\\\log\\\\messages" ;# dbgview logfile location
set self [file tail $argv0]
set self [file rootname $self]
set daemon_log "[pwd]/$self.html"
# puts "self=$self daemon_log=$daemon_log" ;# logfile for this scripts output
set poll_sec 3 ;# how often to check on dbgview, in seconds
set heartbeat_min 15 ;# how often to put heartbeat message in logfile, in minutes

#================== Common procs ======================================================
proc check_dbgview { } {
    # Returns: 0=dbgview is running OK, 1=dbgview is NOT running

    # Get list of dbgview processes, if any.
    set ps_list ""
    set catch_resp [catch "set ps_list \[exec tasklist /fi \"imagename eq dbgview*\"\]" catch_msg]
    # log_info "<pre>check_dbgview catch_resp=$catch_resp catch_msg=$catch_msg</pre>"

    # Look for dbgview in ps_list.
    # puts "ps_list=$ps_list"
    if {[string match -nocase "*dbgview*" $ps_list ]} {
        return 0 ;# dbgview is running OK
    } else {
        return 1 ;# dbgview is NOT running
    }
}

proc start_dbgview { } {
    # Starts a dbgview process with log output directed to a specific location.

    # We deliberately wait a few seconds to give the BTAMP script a guaranteed
    # window of opportunity to rename the existing dbgview logfile. If we agressively
    # restart dbgview ASAP, dbgview locks the existing logfile again, and the
    # BTAMP script will end up with no logfile and lots of failures.
    after 5000

    # If log file is still there, rename it and log the event
    if {[file exists "$::dbgview_log "]} {
        set catch_resp [catch "file rename -force \"$::dbgview_log\" \"$::dbgview_log.old\"" catch_msg]
        log_info "renaming $::dbgview_log to .old, $catch_msg"
    }

    # If supported, start dbgview with /v option to turn on verbose kernel option.
    # This is required for Vista PC to capture / display BTAMP HCI events.
    if {$::verbose_option == "yes"} {
        set catch_resp [catch "exec \"$::dbgview_exe\" /v /l \"$::dbgview_log\" &" catch_msg] 
        log_info "start_dbgview catch_resp=$catch_resp catch_msg=$catch_msg"

        # Since the above command is launched as a parallel process, we dont get any
        # direct feedback if it worked or not. So we go look for the new logfile that
        # should have been created if the process worked OK. If the log file is not 
        # we assume that the /v option was a problem and turn it off. 
        after 2000
        if {[file exists "$::dbgview_log"]} {
            # File got created OK, /v option is allowed.
            return

        } else {
            # File did NOT get created, /v option is not supported.
            set ::verbose_option no
		log_info "Turning off verbose_option, terminating PID=$catch_msg"
            # We also have to terminate the failed dbgview process, 
            # because the error message on the screen is listed as
            # a dbgview process. This will prevent more dbgview process
            # from being started up, as it looks like there is already
            # a valid dbgview process, except its not, its the error
            # msg.
            catch "exec taskkill /pid $catch_msg"
		# We continue onwards and start dbgview without /v option.
        }
    }

    # This version of DbgView doesnt support /v option.
    set catch_resp [catch "exec \"$::dbgview_exe\" /l \"$::dbgview_log\" &" catch_msg] 
    log_info "start_dbgview catch_resp=$catch_resp catch_msg=$catch_msg"
    return
}
 
proc stop_dbgview { } {
    # Stops all existing dbgview processes, if any.
    set catch_resp [catch "exec taskkill /F /IM dbgview.exe" catch_msg]
    log_info "stop_dbgview catch_resp=$catch_resp catch_msg=$catch_msg"
    return
}

proc log_info {msg} {
    # Opens logfile, adds msg with timestamp, closes log file.
    # The logfile is opened & closed every time to allow deleting the logfile
    # without impacting the operation of the running script.

    # Now open the log file for appends.
    set catch_resp [catch "set out \[open \"$::daemon_log\" a+\]" catch_msg]
    if {$catch_resp != 0} {
        puts "\nlog_info ERROR: could not open $::daemon_log catch_msg=$catch_msg!"
        return
    }
    # puts "daemon_log=$::daemon_log out=$out"

    # Create timestamp
    set ts ""
    catch "set ts [clock format [clock seconds] -format %Y%m%d.%H%M%S]" catch_msg

    # Display msg on stdout & write to logfile with html trailer.
    puts "$ts $msg"
    catch "puts $out \"$ts $msg<br>\""

    # Flush & close log file
    catch "flush $out"
    catch "close $out"
    return
}

#================== Main program ====================================================
# Give online help
if {$argc != 0} {
    puts "Basic usage: tclsh $argv0\n"
    puts "This daemon script keeps restarting DebugView with a logfile for the benefit"
    puts "of the BTAMP tests. For Vista, it needs to run with admin privileges."
    exit 1
}

# Add startup log entry
log_info " "
log_info " "
log_info "<b>$::self starting...</b>"

# Get Windows version number. Ver is a DOS shell command,
# so we execute it inside cmd.exe. /c option ensures the
# shell will exit, or we just hang there forever.
set ver ""
set catch_resp [catch "set ver \[exec cmd.exe /c ver\]" catch_msg]
regexp -nocase {Version\s+(\d+)} $ver - ver

# Set the DebugView verbose option default based on Windows version.
if {$ver == 5 || $ver == ""} {
    set verbose_option no
} else {
    set verbose_option yes
}
regsub -all -nocase {[^\w,.]} $catch_msg " " catch_msg
log_info "ver=$ver verbose_option=$verbose_option catch_msg=$catch_msg"

# Check values used in main loop. 
set poll_sec [string trim $poll_sec]
if {![regexp {^\d+$} $poll_sec] || $poll_sec == 0} {
    set poll_sec 3
}
set heartbeat_min [string trim $heartbeat_min]
if {![regexp {^\d+$} $heartbeat_min] || $heartbeat_min == 0} {
    set heartbeat_min 15
}
set hb_sec [expr int($heartbeat_min * 60)]
log_info "poll_sec=$poll_sec heartbeat_min=$heartbeat_min hb_sec=$hb_sec"

# Rename existing dbgview logfile, if any.
if {[file exists "$dbgview_log "]} {
    stop_dbgview
    set catch_resp [catch "file rename -force \"$dbgview_log\" \"$dbgview_log.old\"" catch_msg]
    log_info "renaming $dbgview_log to .old, $catch_msg"
}

# Main loop to monitor dbgview, runs forever...
set start_sec [clock seconds]
while { 1 } {

    # Restart dbgview if its not running.
    if {[check_dbgview] == 1} {
        start_dbgview
    }

    # Wait poll_sec
    after [expr int($poll_sec) * 1000]

    # Put heartbeat msg in log as needed.
    set cur_sec [clock seconds]
    set delta_sec [expr $cur_sec - $start_sec]
    if {$delta_sec >= $hb_sec} {
        log_info "heartbeat: $delta_sec sec"
        set start_sec $cur_sec
    }
}

