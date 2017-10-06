#! /usr/bin/tclsh
# $Id$

package require Tclx

# You lock a resource by sending "L resource_name info\n". You unlock
# a resource with "U resource_name\n", or but disconnecting from the
# server.  Once you acquire the lock, you get back a message:
# "LOCKED". If you do not immediately acquire the lock you are placed
# in a queue until your turn (no message is sent until then).  Each
# connection holds only one lock/request.

# Disconnecting from the server immediately releases the lock or
# pending lock.

# lock server based on code by Todd Coram http://wiki.tcl.tk/14601
# daemon based on code by Tom Poindexter  http://wiki.tcl.tk/2224

variable port 6700
variable qport 6701

# lock(resource) -> chan ..
# First chan int the list has the lock, the rest are queued.
array set lock [list]

# client(chan) -> resource
array set client [list]
array set clinfo [list]

#
proc unlock {chan res} {
    global lock client clinfo
    if {[lindex $lock($res) 0] == $chan} {
        set lock($res) [lrange $lock($res) 1 end]
        set idx [lsearch -exact $client($chan) $res]
        set client($chan) [lreplace $client($chan) $idx $idx]
        set clinfo($chan) [lreplace $clinfo($chan) $idx $idx]
        # Notify the next in line
        if {[llength $lock($res)] != 0}  {
	    puts [lindex $lock($res) 0] "LOCKED $res"
        }
        return 1
    }
    set idx [lsearch -exact $lock($res) $chan]
    if {$idx != -1} {
        set lock($res) [lreplace $lock($res) $idx $idx]
    }
    return 0
}

# You will either aquire the lock (return 1) or be queued (return 0).
#
proc lock {chan res info} {
    global lock client clinfo
    lappend lock($res) $chan
    lappend client($chan) $res
    lappend clinfo($chan) $info
    return [locked? $chan $res]
}

proc locked? {chan res} {
    global lock
    if {[info exists lock($res)] && [lindex $lock($res) 0 ] == $chan} {
        return 1
    }
    return 0
}

proc query {chan addr port} {
    global lock clinfo client
    fconfigure $chan -buffering none
    foreach res [lsort [array names lock]] {
	foreach c $lock($res) {
	    puts $chan "$res $clinfo($c)"
	}
    }
#    puts $chan "lock: [array get lock]  "
#    puts $chan "clinfo: [array get clinfo]  "
#    puts $chan "client: [array get client]  "
#    puts $chan "channels: [file channels]  "
    close $chan
}

proc accept {chan addr port} {
    global client
    fconfigure $chan -buffering none
    fileevent $chan readable [list handle_req $chan]
    set client($chan) [list]
    set clinfo($chan) [list]
}

proc handle_req chan {
    global client lock clinfo dups
    if {[eof $chan]} {
        # Unlock resources
        foreach res $client($chan) {
	    puts stderr "($chan) Disconnected $res"
            unlock $chan $res
	    if {[info exists dups($chan)]} {
		unset dups($chan)
	    }
        }
        unset client($chan)
        unset clinfo($chan)
        close $chan
        return
    }
    set str [gets $chan]
    foreach {req res cli} $str {
        switch -- $req {
            L {
                puts stderr "($chan) Locking $res for $cli"
		if {[info exists dups($chan)]} {
		    puts $chan "DUPLICATE $res"
		    break
		}
		if {[catch {
		    append cli @[lindex [fconfigure $chan -peername] 1]
		    if {[lock $chan $res $cli]}  {
			puts $chan "LOCKED $res"
		    } else {
			puts $chan "QUEUED $res [llength $lock($res)]"
		    }
		    set dups($chan) 1
		} ret]} {
		    puts stderr $::errorInfo
		    unlock $chan $res
		}
            }
            U {
                if {![info exists lock($res)]} {
		    puts stderr "($chan) Not locked $res"
                    puts $chan "NOLOCKS $res"
                    break
                }
                if {[unlock $chan $res]} {
		    puts stderr "($chan) Unlocking $res"
                    puts $chan "UNLOCK $res"
                } else {
		    puts stderr "($chan) Dequeing $res"
                    puts $chan "DEQUEUED $res"
                }
		unset dups($chan)
            }
            default {
                puts $chan {HUH? Usage: L:resource or U:resource}
            }
	}
    }
}


proc shutdown {} {
    # whatever cleanup you need to do
    exit
}

proc dispatcher {} {
    variable port
    variable qport
    socket -server [list accept] $port
    socket -server [list query] $qport
    vwait forever
}


proc daemonize {} {
    close stdin
    close stdout
    close stderr
    if {[fork]} {exit 0}
    id process group set
    if {[fork]} {exit 0}
    set fd [open /dev/null r]
    set fd [open /dev/null w]
    set fd [open /dev/null w]
    cd /
    umask 022
    return [id process]
}

if {$argv eq ""} {
    daemonize
    signal ignore  SIGHUP
    signal unblock {QUIT TERM}
    signal trap    {QUIT TERM} shutdown
}
dispatcher
