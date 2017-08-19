#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: fd79489567e36ab7220f5c6654a40bdcb711f1d7 $
# $Copyright Broadcom Corporation$
#

package provide UTF::STBAP 2.0

package require snit
package require UTF::doc
package require UTF::Base

snit::type UTF::STBAP {

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip 192.168.1.2
    option -image
    option -tag "trunk"
    option -date "%date%"
    option -sta
    option -name -configuremethod CopyOption
    option -device
    option -nvram
    option -postboot
    option -assertrecovery 1
    option -wlinitcmds
    option -console
    option -console2; # Secondary console
    option -reloadoncrash -type snit::boolean -default false
    option -restartwait -type snit::double -default 0
    option -defer_restart -type snit::boolean -default false

    # base handles any other options and methods
    component base -inherit yes

    # Default nvram settings
    variable nvram_defaults {
	wl_msglevel=0x80101
	console_loglevel=7
	wl0_obss_coex=0
	wl1_obss_coex=0
#	wl0_bw_cap=-1
#	wl1_bw_cap=-1
	wl0_reg_mode=off
	wl1_reg_mode=off
    }

    variable Bridge
    variable stas {}

    variable reclaim -array {}
    variable pre_reclaim -array {}

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using \
	    UTF::Base %AUTO% -ssh ush \
	    -init [mymethod init] \
	    -brand "linux-internal-wl-media" \
	    -wlconf_by_nvram true
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	foreach {sta dev} [UTF::Staexpand $options(-sta)] {
	    if {$dev eq ""} {
		error "$sta has no device name"
	    } else {
		UTF::STA ::$sta -host $self -device $dev
		lappend stas ::$sta
	    }
	    if {$options(-device) eq ""} {
		# Full dongle routers need at least one device
		# name.
		set options(-device) $dev
	    }
	}
	$base configure -lan_ip $options(-lan_ip)
	if {[$self cget -relay] eq ""} {
	    error "No -relay specified for $options(-name)"
	}
	array set Bridge "stale 1"
    }

    destructor {
	catch {$base destroy}
	foreach {sta dev} $options(-sta) {
	    catch {$sta destroy}
	}
    }

    # Internal method for copying options to delegates
    method CopyOption {key val} {
	set options($key) $val
	$base configure $key $val
    }

    method findimages {args} {
	return "not implemented"
    }

    method load {args} {
	UTF::GetKnownopts {
	    {erase "Erase nvram"}
	}
	if {$(erase)} {
	    $self restore_defaults -nosetup
	}
	set ver [$self -t 5 wl ver]
	regexp {version (.*)} $ver - ver
	return "$ver (skipped)"
    }

    method cmeload {dir} {
	$self relay copyto \
	    $dir/3384b0-nicap_kernel $dir/3384b0-nicap_apps_nand_ubifs_bs128k_ps2k \
	    /var/lib/tftpboot
	$self relay $UTF::usrutf/cmeshell $options(-console2) load \
	    3384b0-nicap_kernel 3384b0-nicap_apps_nand_ubifs_bs128k_ps2k
	$self wait_for_boot 90
    }

    typevariable nvdirty -array {}
    typevariable nvram_cache -array {}
    method nvram {cmd args} {
	if {![regexp {^33} [$self boardname]]} {
	    return [$base rexec nvram $cmd {*}$args]
	}

	if {$cmd eq "commit"} {
	    # skip commit on CM
	    return
	}
	if {$cmd eq "set"} {
	    set a [lindex $args 0]
	    if {![regexp {^([^=]*)=(.*)$} $a - k v]} {
		error "Bad nvram setting \"$a\": should be key=value"
	    } else {
		# disable variables known not to work
		switch -glob $k {
		    #*_chanspec -
		    #*_reg_mode -
		    *_phytype {
			UTF::Message INFO $options(-name) "nvram $cmd $args: skipped"
			return
		    }
		}
		# Strip extra quotes
		if {![regsub {^\"(.*)\"$} $v {\1} v]} {
		    regsub {^'(.*)'$} $v {\1} v
		}
		set nvram_cache($k) $v
		set args [list "$k='$v'"]
	    }
	    if {[regexp {wl(\d)} "$args" - u]} {
		set nvdirty($u) 1
	    }
	}
	# Strip temporary debug logs
	regsub -all {^\#.*\n} [$base rexec -n nvram $cmd {*}$args] {}
    }

    UTF::doc {
	# [call [arg host] [method restart] [lb][arg {key=value ...}][rb]]

	# Set and commit nvram variables and run rc restart.  This is
	# similar to [arg host] [method reboot] but closer to the UI
	# behaviour.  Note that some nvram settings require a full
	# reboot and will not take effect during a restart.
    }

    method restart {args} {
	set qargs {}
	array set unit {}
	foreach a $args {
	    if {![regexp {^([^=]*)=(.*)$} $a - k v]} {
		error "Bad nvram setting \"$a\": should be key=value"
	    } else {
		# Strip extra quotes
		regsub {^\"(.*)\"$} $v {\1} v
		regsub {^'(.*)'$} $v {\1} v
		if {$k eq "lan_ipaddr"} {
		    UTF::Message INFO $options(-name) "Address change: $v"
		    set lan_ip $v
		}
		lappend qargs "$k=\"$v\""
	    }
	}
	foreach a $qargs {
	    $self nvram set $a
	}
	if {[llength $qargs]} {
	    $self nvram commit
	}
	if {$options(-defer_restart)} {
	    UTF::Message LOG $options(-name) "Deferred restart"
	    return
	}
	if {[regexp {^72} [$self boardname]]} {
	    $self rexec {rc restart >/dev/console 2>&1}
	} else {
	    if {[llength [set u [array names nvdirty]]] > 1} {
		set u ""
	    }
	    $self rexec rc restart {*}$u
	    array unset nvdirty
	    $self wait_for_restart
	    set changed {}
	    foreach k [array names nvram_cache] {
		set r [$self nvram get $k]
		if {$r ne $nvram_cache($k)} {
		    UTF::Message WARN $options(-name) "$k: $r ne $nvram_cache($k)"
		    lappend changed "$k=$r"
		}
	    }
	    array unset nvram_cache
	    if {$changed ne {}} {
		$self warn [join $changed]
	    }
	}

	set Bridge(stale) 1
	$base configure -initialized 0
	UTF::Sleep $options(-restartwait)

	if {[info exists lan_ip]} {
	    $self configure -lan_ip $lan_ip
	    # Don't try to reconnect yet since the caller may have to
	    # fix the relay's IP first.
	} elseif {[catch {$self rexec :} ret]} {
	    error "connection problem after rc restart" $::errorInfo
	}
	if {$options(-wlinitcmds) ne ""} {
	    $self rexec [string trim $options(-wlinitcmds)]
	}
	return
    }

    UTF::doc {
	# [call [arg host] [method reboot]]

	# Reboot DSL Router.

    }

    method reboot {args} {
	set qargs {}
	foreach a $args {
	    if {![regexp {^([^=]*)=(.*)$} $a - k v]} {
		error "Bad nvram setting \"$a\": should be key=value"
	    } else {
		# Strip extra quotes
		regsub {^\"(.*)\"$} $v {\1} v
		lappend qargs "$k=\"$v\""
	    }
	}
	foreach a $qargs {
	    $self rexec nvram set $a
	}
	if {[llength $qargs]} {
	    $self nvram commit
	    $self sync
	}
	$self rexec -x reboot
	$self wait_for_boot
	set Bridge(stale) 1
	UTF::Message DBG $options(-name) "reboot: nvram_cache [array get nvram_cache]"
	set changed {}
	foreach k [array names nvram_cache] {
	    set r [$self nvram get $k]
	    if {$r ne $nvram_cache($k)} {
		UTF::Message WARN $options(-name) "$k: $r ne $nvram_cache($k)"
		lappend changed "$k=$r"
	    }
	}
	array unset nvram_cache
	if {$changed ne {}} {
	    $self warn [join $changed]
	}

	if {$options(-postboot) ne ""} {
	    $self rexec [string trim $options(-postboot)]
	}
	if {$options(-wlinitcmds) ne ""} {
	    $self rexec [string trim $options(-wlinitcmds)]
	}
	return
    }

    UTF::doc {
	# [call [arg host] [method restore_defaults]
	#      [lb][option -noerase][rb] [lb][option -nosetup][rb]]

	# Restore defaults

    }
    method restore_defaults {args} {
	UTF::Getopts {
	    {noerase "Don't erase nvram first"}
	    {nosetup "Don't apply local setup"}
	    {n "Don't apply - just return settings"}
	}
	# Clear cached data
	foreach S $stas {
	    $S configure -ssid {} -security {} -wepkey {} -wepidx {} -wpakey {}
	}
	set nvram ""
	if {!$(nosetup)} {
	    # Merge config nvram with built-in defaults
	    foreach a [concat \
			   [UTF::decomment $nvram_defaults] \
			   [UTF::decomment $options(-nvram)]] {
		if {![regexp {^([^=]*)=(.*)$} $a - k v]} {
		    error "Bad nvram setting \"$a\": should be key=value"
		} else {
		    set nv($k) $v
		}
	    }
	    foreach k [lsort [array names nv]] {
		lappend nvram "$k=$nv($k)"
	    }
	}

	if {$(n)} {
	    return $nvram
	}
	if {[regexp {^72} [$self boardname]]} {
	    if {!$(noerase)} {
		$self rm -f /etc/nvrams_ap_current.txt
		$self sync
		$self reboot
	    }
	    if {$nvram ne ""} {
		# set our defaults again, in case the os overwrote them
		$self reboot {*}$nvram
		$self lanwanreset
	    }
	} else {
	    if {!$(noerase)} {
		$self nvram godefault
		array unset nvram_cache
		$self wait_for_restart
	    }
	    if {$nvram ne ""} {
		# set our defaults again, in case the os overwrote them
		$self restart {*}$nvram
	    }
	}
	return
    }

    UTF::doc {
	# [call [arg host] [method reload]]

	# Reload the wl driver on the router.  Used for debugging and
	# resetting counters.  Only works with monolithic driver for
	# now.
    }

    method reload {} {
	$self reboot
    }

    method lanwanreset {} {
	# Reset IP on lanpeer in case we're using DHCP, or if we
	# had to use a temporary IP for the erase.
	foreach l [$self cget -lanpeer] {
	    $l ifconfig [$l cget -ipaddr]
	    $l add_networks $self
	}
    }


    method wait_for_boot {{t 30}} {
	UTF::Sleep $t
	$self configure -initialized 0
	$self lan ping $options(-lan_ip) -c 30
	for {set i 0} {[catch {$self -n :} ret] && $i < 20} {incr i} {
	    if {[regexp {Connection refused} $ret]} {
		UTF::Sleep 20
	    }
	}
	UTF::Sleep 20
    }

    UTF::doc {
	# [call [arg host] [method copyto] [arg src] [arg dest]]

	# Copy local file [arg src] to [arg dest] on [arg host].  Auto
	# uncompression is supported.
    }
    method copyto {src dst} {
	if {$src eq ""} {
	    set src /dev/null
	}
	if {[file extension $src] eq ".gz" &&
	    [file extension $dst] ne ".gz"} {
	    set cmd "zcat"
	} else {
	    set cmd "cat"
	}
	append cmd ">$dst"
	if {[file executable $src]} {
	    append cmd ";chmod +x $dst"
	}
	$self rexec -n -t 120 $cmd <$src
    }

    UTF::doc {
	# [call [arg host] [method copyfrom] [arg src] [arg dest]]

	# Copy file [arg src] on [arg host] to local [arg dest].  Auto
	# compression is not supported.
    }
    method copyfrom {src dst} {
	set in [$self rpopen -2 cat $src]
	set out [open $dst w]
	fconfigure $in -translation binary
	fconfigure $out -translation binary
	try {
	    while {![eof $in]} {
		puts -nonewline $out [read $in 10240]
	    }
	    close $in
	} finally {
	    close $out
	}
    }

    UTF::doc {
	# [call [arg host] [method ifconfig] [arg dev] [option dhcp]]

	# Enable DHCP on device [arg dev].

	# [call [arg host] [method ifconfig] [arg {args ...}]]

	# Run ifconfig on the host, disabling DHCP if necessary.
    }

    # IP address cache
    variable ipaddr -array {}
    variable dhcpver ""

    method ifconfig {dev args} {
	if {[llength $args]} {
	    # invalidate cache
	    if {[info exists ipaddr($dev)]} {
		unset ipaddr($dev)
	    }

	    # Need to use kill -9 otherwise dhcpcd will bring down
	    # the interface and disassociate
	    set PF "/var/run/dhcpcd-${dev}.pid"
	    catch {$self rexec -n "test -f $PF && kill -9 `cat $PF`; rm $PF"}
	}
	if {$args eq "dhcp"} {
	    # Start up dhcpcd without updating any system files.
	    $self udhcpc -i $dev -n -q -p $PF
	} else {
	    $self rexec ifconfig $dev $args
	}
    }

    UTF::doc {
	# [call [arg host] [method macaddr] [arg dev]]

	# Returns the MAC address for [arg dev]
    }
    method macaddr {dev} {
	if {[regexp {HWaddr\s+(\S+)} [$self rexec -n ifconfig $dev] - mac]} {
	    return $mac
	} else {
	    error "No MAC address found"
	}
    }

    UTF::doc {
	# [call [arg host] [method ipaddr] [arg dev]]

	# Return IP address of device [arg dev] or Error if device is
	# down.  If device is a member of a bridge, the bridge
	# interface is queried instead.
    }

    method ipaddr {dev} {
	if {$Bridge(stale)} {
	    $self probe
	}
	set nvname $dev

	if {[info exists Bridge($nvname)]} {
	    set dev $Bridge($nvname)
	}
	# Support both linux and BSD ifconfig
	if {[regexp {inet (?:addr:)?([0-9.]+)} [$self -s ifconfig $dev] \
		 - addr]} {
	    return $addr
	} else {
	    error "No IP address available"
	}
    }

    UTF::doc {
	# [call [arg host] [method probe]]

	# Attempt to auto-discover properties of [arg host].
	# Records bridge/interface herarchy for later use.
    }

    method probe {} {
	# Build bridge array based on nvram since brctl is less
	# portable and may depend on STP.
	foreach {lan br} {lan br0 lan1 br1} {
	    if {[catch {$self nvram get ${lan}_ifnames} iflist($lan)]} {
		UTF::Message ERROR [$self cget -name] "probe: $iflist($lan)"
		return
	    }
	}
	foreach {lan br} {lan br0 lan1 br1} {
	    foreach if $iflist($lan) {
		set Bridge($if) $br
		set Bridge(${if},lan) $lan
	    }
	}
	set Bridge(stale) 0
    }

    UTF::doc {
	# [call [arg host] [method ping] [arg target]
	#	[lb][opt -c] [arg count][rb]
	#       [lb][opt -s] [arg size][rb]]

	# Ping from [arg host] to [arg target] with packet size [arg
	# size].  Target may be a STA or a Hostname/IP address.
	# Returns success if a response packet is received before [arg
	# count] (default 5) packets have been sent and error if not.
    }

    method ping {target args} {
	set msg "ping $target $args"
	UTF::Getopts {
	    {c.arg "5" "Count"}
	    {s.arg "56" "Size (ignored)"}
	} args
	if {[info commands $target] == $target} {
	    set target [$target ipaddr]
	}

	# ping -W makes negative tests faster, but isn't supported on
	# some versions so impose a timeout instead
	set ping [list $self rexec -n -t 1 ping -c 1 -s $(s) $target]

	# Loop, since Router ping doesn't have a short-circuit "count"
	for {set c 0} {$c < $(c)} {incr c} {
	    catch $ping ret
	    if {[regexp {(\d+) packets received} $ret - r] && $r > 0} {
		return
	    }
	}
	error "ping failed"
    }

    typevariable msgfile
    variable oops ""
    variable rtecapture
    variable interruptedbydongle ""
    variable processingHostError false
    typevariable restart_done 0

    method wait_for_restart {{t 90}} {
	UTF::Message DBG $options(-name) "waiting..."
	set timer [after [expr {$t * 1000}] set [mytypevar restart_done] TIMEOUT]
	vwait [mytypevar restart_done]
	UTF::Message DBG $options(-name) "wait over $restart_done"
	after cancel $timer
	if {$restart_done eq "TIMEOUT"} {
	    $self worry "restart timeout"
	    $self reboot
	}
	set restart_done 0
    }

    # Internal callback for fileevent below
    method getdata {request} {
	set fd $msgfile($request)
	if {![eof $fd]} {
	    set msg [gets $fd]

	    if {[regexp {root login on} $msg]} {
		# no need to log all these login messages
		return
	    }
	    # Lose timestamp, since we're adding our own.
	    regsub {^\[\s*(\d+)\.(\d+)\] } $msg {} msg

	    if {[regexp {TRAP type } $msg]} {
		$self consolemsg $msg
		return
	    }
	    # Pass CONSOLE messages on to rte handler CONSOLE messages
	    # frequently interrupt host messages, so collect partial
	    # messages and report them after the rte messages have
	    # been handled.
	    if {[regexp {(.*)CONSOLE: (.*)} $msg - pre post]} {
		append interruptedbydongle $pre
		if {[info exists rtecapture]} {
		    append rtecapture "\n$post"
		}
		$self consolemsg $post
		return
	    } elseif {$interruptedbydongle ne ""} {
		UTF::Message LOG $options(-name) $interruptedbydongle
		set interruptedbydongle ""
	    }
	    if {[regexp {Restart of interfaces done} $msg]} {
		UTF::Message INFO $options(-name) $msg
		UTF::Message DBG $options(-name) "done!"
		set restart_done 1
		return 1
	    }
	    if {[regexp {pciedev_shared invalid} $msg]} {
		if {!$processingHostError} {
		    try {
			set processingHostError true
			# Give dongle console a chance to report problems
			# first, if any
			UTF::Sleep 2

			# Only use the first of these.  Asserts get priority
			if {![info exists ::UTF::panic]} {
			    set ::UTF::panic $msg
			}
			UTF::Message FAIL $options(-name) $msg
			if {!$processingDongleError} {
			    try {
				set processingDongleError true
				# This may mean the dongle locked up.
				# Dump and reload.
				$self upload "dongle_dhd.dmp"
				catch {
				    $self reload
				}
			    } finally {
				set processingDongleError false
			    }
			}
		    } finally {
			# DHD may retry the load
			after 10000 [list set [myvar processingHostError] false]
		    }
		}
		return 1
	    }
	    if {[regexp {Kernel panic:.*|assert|Bad trx header|insmod: unresolved symbol |RPC call .*return wait err|Out of Memory: Killed process} $msg] ||
		      ($oops ne "" && [regexp {Code: |rebooting} $msg])} {
		UTF::Message FAIL $options(-name) $msg
		# If there's already an assert, don't overwrite it
		if {![info exists ::UTF::panic] ||
		    ![regexp -nocase \
			  {assert|trap|Oops|Memory leak|RPC call .*return wait err} \
			  $::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		if {[$base cget -initialized] eq 1} {
		    # If we were connected normally, signal to
		    # reestabish connection, otherwise leave it alone.
		    $base configure -initialized 0
		}
		if {$oops ne ""} {
		    if {[catch {$self gdboops} ret]} {
			UTF::Message WARNING $options(-name) $ret
		    }
		    set oops ""
		}
	    } elseif {[regexp {Reading ::} $msg]} {
		set stuckreading 1
		UTF::Message LOG $options(-name) $msg
	    } elseif {[regexp {proc: table is full} $msg]} {
		UTF::Message LOG $options(-name) $msg
		$self rexec -n ps
	    } elseif {[regexp {oom-killer: } $msg]} {
		UTF::Message FAIL $options(-name) $msg
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		# Only known way to recover from this is to reboot and
		# reload.  Shouldn't need to semaphore this.
		catch {
		    $self -n "reboot;:"
		    UTF::Sleep 25
		    $self reload
		}
	    } elseif {![$base common_getdata $msg]} {
		UTF::Message LOG $options(-name) $msg
		if {[regexp {Oops in |Unhandled kernel|traps\.c} $msg]} {
		    set oops "$msg\n"
		} elseif {$oops ne "" &&
			  [regexp {module:\s+|epc\s+|\[<([\da-f]+)>\]} $msg]} {
		    append oops "$msg\n"
		}
	    }
	} else {
	    # Leave non-blocking for safer close.  Any error messages
	    # should have been collected earlier.
	    close $fd
	    UTF::Message LOG $options(-name) "Log closed $fd"
	    unset msgfile($request)
	}
    }

    UTF::doc {
	# [call [arg host] [cmd gdboops] [lb][file file][rb]
	#                                [lb][file src][rb]]

	# Process Oops log with gdb and attempt to report function and
	# line number where the router crashed.  When called by hand,
	# [file file] should be provided, as a file containing the
	# Oops log.  [file src] may also be provided as the location
	# of the corresponding source tree.  [file src] defaults to
	# the auto detected source tree from the last image load.
	# The Oops log and source tree will be made available internally
	# when called as part of the background message handling.
    }
    method gdboops {{file ""} {src ""}} {

	# copy oops locally, then clear the object oops since we may
	# get nested by the io calls below.
	set oopscpy $oops
	set oops ""

	if {$file ne ""} {
	    set fd [open $file]
	    set oopscpy [read $fd]
	    close $fd
	}
	if {$src eq ""} {
	    regsub {/} $options(-name) {.} src
	    set src [$self relay -noinit -s -q cat "$src-src"]
	}
	if {![file isdirectory $src]} {
	    error "Source code not found"
	}
	set linux "$src/linux/linux"
	foreach msg [split $oopscpy "\n"] {
	    if {[regexp {\mOops in |Unhandled kernel|traps\.c} $msg]} {
		set oopscmd "set width 0\n"
		append oopscmd "file $linux/vmlinux_dbgsym\n"
	    } elseif {[regexp {\mmodule:\s+(\S+)\s+([0-9a-f]+)} $msg - m a]} {
		set sim "$m/${m}_dbgsym.o"
		if {$m eq "wl_high"} {
		    set sim "wl/$sim"
		}
		append oopscmd \
		    "add-symbol-file $linux/drivers/net/$sim 0x$a+0x60\n"
	    } elseif {[regexp {\mepc\s+:\s+([0-9a-f]+)\s+Not tainted} \
			   $msg - epc]} {
		append oopscmd "l *0x$epc\n"
	    } elseif {[regexp {\[<([\da-f]+)>\]} $msg]} {
		foreach t [regexp -all -inline {[\da-f]{8}} $msg] {
		    append oopscmd "info line *0x$t\n"
		}
	    }
	}
	set f [open "/tmp/oops.cmd" w]
	catch {file attributes "/tmp/oops.cmd" -permissions 00666}
	puts $f $oopscmd
	UTF::Message LOG $options(-name) $oopscmd
	close $f
	set cmd $::UTF::projtools/linux/bin/mipsel-linux-gdb
	append cmd {-q -n < /tmp/oops.cmd | sed s/\".*\\/src/\"src/}
	set oops ""
	UTF::Message LOG $options(-name) $cmd
	catch {exec {*}$cmd} ret
	regsub -line -all \
	    {.*add symbol table.*\n|.*\.text_addr =.*\n} $ret {} ret
	UTF::Message LOG $options(-name) $ret

	# Replace Code error with function and line, if found
	if {(![info exists ::UTF::panic] ||
	     [regexp -nocase {Code|Kernel panic} $::UTF::panic]) &&
	    [regexp -line {0x[[:xdigit:]]+ is in (.*\))} $ret - fandl]} {
	    set ::UTF::panic "Oops in $fandl"
	}
    }

    UTF::doc {
	# [call [arg host] [method open_messages] [lb][arg file][rb]]

	# Open system message log on [arg host] and log new messages
	# to the UTF log.  [method open_messages] will return
	# immediately, while messages will continue to be logged in
	# the background until [method close_messages] is called.
	# [arg file] defaults to [file /var/log/messages].  Multiple
	# loggers can be opened, so long as they log different [arg
	# file]s.  Attempts to log the same [arg file] multiple times
	# will be logged and ignored.
    }

    method open_messages { {file ""} } {
	if {$file == ""} {
	    set file $options(-console)
	}
	set id [$self cget -lan_ip]
	if {[info exists msgfile($id:$file)]} {
	    return
	}
	UTF::Message LOG $options(-name) "Open $file"
	if {[catch {$self serialrelay socket $file} ret]} {
	    $self worry "$file: $ret"
	    return
	}
	set msgfile($id:$file) $ret
	fconfigure $msgfile($id:$file) -blocking 0 -buffering line
	puts $msgfile($id:$file) ""; # Trigger NPC telnet reconnect
	fileevent $msgfile($id:$file) readable [mymethod getdata $id:$file]
	UTF::Sleep 1
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method close_messages] [lb][arg file][rb]]

	# Close a system message log previously opened by [method
	# open_messages].  An error will be reported if [arg file]
	# does not match that of a previous invocation of [method
	# open_messages].
    }
    method close_messages { {file ""} } {
	if {$file == ""} {
	    set file $options(-console)
	}
	set id [$self cget -lan_ip]
	if {[info exists msgfile($id:$file)] &&
	    [file channels $msgfile($id:$file)] ne ""} {
	    UTF::Message LOG $options(-name) "Close $file $msgfile($id:$file)"
	    if {[set pid [pid $msgfile($id:$file)]] ne ""} {
		# Processes to wait for
		catch {exec kill {*}$pid} ret
		UTF::Message LOG $options(-name) $ret
	    }
	    # Leave non-blocking for safer close.  Any error messages
	    # should have been collected earlier.
	    close $msgfile($id:$file)
	    unset msgfile($id:$file)
	} else {
	    UTF::Message LOG $options(-name) "Close $file (not open)"
	}
	return
    }

   # Semaphore to prevent host-side error recovery if dongle-side
    # recovery is already underway.
    variable processingDongleError false

    variable TRAPLOG

    method consolemsg {msg} {
	set needreload 0

	if {$msg eq ""} {
	    return
	}

	regsub {^\d{6}\.\d{3} } $msg {} msg; # strip timestamps

	if {[info exists TRAPLOG]} {
	    # If FWID is set we're collecting new-style trap messages
	    append TRAPLOG "$msg\n"
	}

	if {[regexp {ASSERT pc} $msg]} {
	    # Downgrade these to warnings, since they will be handled
	    # by the trap handler later
	    UTF::Message WARN "$options(-name)>" $msg
	} elseif {[regexp {FWID (\d\d-[[:xdigit:]]{1,8})} $msg]} {
	    UTF::Message INFO "$options(-name)>" $msg
	    set TRAPLOG "$msg\n"
	} elseif {!$processingDongleError &&
		  [regexp {TRAP |ASSERT.*|Trap type .*| Overflow |Bad |Dongle trap|PSM microcode watchdog fired|PSM WD!} $msg]} {
	    set processingDongleError true
	    if {![info exists TRAPLOG]} {
		# No FWID, but we want to capture these anyway
		set TRAPLOG "$msg\n"
	    }
	    UTF::Message FAIL "$options(-name)>" $msg

	    if {$options(-assertrecovery) &&
		[regexp -nocase {assert|trap} $msg]} {
		set needreload 1
	    }

	    # Give DHD driver a moment to collect trap log
	    UTF::Sleep 0.2 quiet

	    package require UTF::RTE
	    set trap [UTF::RTE::parse_traplog $self $TRAPLOG]
	    if {$trap ne ""} {
		set msg $trap
	    }

	    # If there's already an assert, don't overwrite it
	    if {![info exists ::UTF::panic] ||
		![regexp -nocase {assert|trap} $::UTF::panic] ||
		[regexp -nocase {Dongle assert file ""} $::UTF::panic]} {
		set ::UTF::panic $msg
	    }
	    if {$needreload} {
		catch {
		    # Try to reload driver
		    $self reload
		}
	    }
	    set processingDongleError false
	} elseif {[regexp {reclaim section (.*): Returned (\d+) bytes \(pre-reclaim: (\d+)\)} $msg - s r p]} {
	    set reclaim($s) $r
	    set pre_reclaim($s) $p
	    UTF::Message LOG "$options(-name)>" $msg
	} elseif {[regexp {reclaim(.*): Returned (\d+) } $msg - s r]} {
	    set reclaim($s) $r
	    UTF::Message LOG "$options(-name)>" $msg
	} elseif {![$base common_getdata $msg "$options(-name)>"]} {
	    UTF::Message LOG "$options(-name)>" $msg
	}
    }

    method {route add} {net mask gw} {
	$self rexec -n route add -net $net netmask $mask gw $gw
    }

    method {route del} {net mask gw} {
	$self rexec -n route del -net $net netmask $mask gw $gw
    }

    method {route replace} {net mask gw} {
	catch {$self route del $net $mask $gw}
	$self route add $net $mask $gw
    }

    method init {} {
	$self open_messages
    }

    method deinit {} {
	$self close_messages
	$self configure -initialized 0
    }

    UTF::doc {
	# [call [arg host] [method freekb]]

	# Returns free mem kbytes using "mu" RTE command on the dongle.
    }

    method freekb {} {
	catch {$self rte mu} ret
	if {![regexp {Free: (\d+)} $ret - free]} {
	    error "Free mem not found ($ret)"
	}
	expr {$free / 1024.0}
    }

    method pre_reclaim {} {
	array get pre_reclaim
    }

    method rte_available {} {
	return 1
    }

    method rte {args} {
	try {
	    set rtecapture ""
	    $self dhd -i $options(-device) cons $args
	    UTF::Sleep 1
	} finally {
	    set ret $rtecapture
	    unset rtecapture
	}
	set ret
    }

    # Dongle methods

    method hndrte_src {} {
	if {![info exists hndrte_src]} {
	    regsub {/} $options(-name) {.} name
	    if {[catch {$self relay rexec -noinit -s -q \
			    ls -l /tmp/$name-hndrte-src.lnk} hndrte_src]} {
		regsub {/dongle/rte/.*} [$self hndrte_exe] {} hndrte_src
	    } else {
		regsub {.*-> } $hndrte_src {} hndrte_src
	    }
	}
	set hndrte_src
    }

    method hndrte_exe {} {
	if {![info exists hndrte_exe]} {
	    regsub {/} $options(-name) {.} name
	    set hndrte_exe [$self relay rexec -noinit -s -q \
				ls -l /tmp/$name-hndrte-exe.lnk]
	    regsub {.*-> } $hndrte_exe {} hndrte_exe
	}
	set hndrte_exe
    }

    method findassert {file line} {
	UTF::RTE::findassert $options(-name) $file $line [$self hndrte_src]
    }

    method findtrap {trap stack} {
	UTF::RTE::findtrap $options(-name) $trap $stack [$self hndrte_exe]
    }

    UTF::doc {
	# [call [arg host] [method boardname]]

	# Query device boardtype
    }

    variable boardname
    method boardname {} {
	if {![info exists boardname]} {
	    set ret [regexp -inline {\w+} [$self uname -m]]
	    if {$ret eq "mips"} {
		set ret [$self -n {awk '/system type/{print \$4}' /proc/cpuinfo}]
	    } else {
		set ret [$self -n cat /proc/device-tree/bolt/board]
		regsub {\0} $ret {} ret
	    }
	    regsub {BCM9?} $ret {} ret
	    set boardname $ret
	}
	set boardname
    }

    UTF::doc {
	# [call [arg host] [method wlname] [arg dev]]

	# Return wl<n> name corresponding to this device, eg eth2 may
	# correspond to wl0
    }

    # Cache wl names
    variable wlc -array {}
    method wlname {dev} {
	if {![info exists wlc($dev)]} {
	    if {[regexp {^wl} $dev]} {
		set wlc($dev) $dev
	    } elseif {[regexp {^b(wl.*)} $dev - wlname]} {
		set wlc($dev) $wlname
	    } else {
		if {[catch {$self wl -i $dev ver} ret]} {
		    if {[regexp {Not Enough Resources} $ret]} {
			# Fallback for reduced memory systems
			set wlc($dev) "wl0"
		    }
		    error $ret
		}
		if {[regexp -line {^(wl\d+):} $ret - wlname]} {
		    set wlc($dev) $wlname
		} else {
		    error "Unable to determine wl driver name for device $dev"
		}
	    }
	}
	return $wlc($dev)
    }


    method whatami {{STA ""}} {
	if {[catch {$self boardname} b]} {
	    set b "<unknown>"
	}
	if {$STA ne ""} {
	    if {[catch {$STA chipname} c]} {
		set c "<unknown>"
	    }
	    # Chipname contains rev information
	    if {[lindex $c 0] ne $b} {
		# Router / Wireless Rev
		set b "$b/$c"
	    } else {
		# Single-chip combo wireless router
		set b $c
	    }
	}
	return "STBAP $b"
    }

    UTF::doc {
	# [call [arg host] [method sumcheck] [arg local] [arg remote]]

	# Compare checksums of local and remote copies of a file.
	# Returns zero if sums match and 1 if sums differ, or if the
	# remote copy is missing.  Useful for checking to see if a
	# copy is needed before invoking [method copyto] for files
	# that rarely change.
    }
    method sumcheck {local remote} {
	expr {[catch {$self md5sum $remote} ret] ||
	      [lindex $ret 0] ne [lindex [exec md5sum $local] 0]}
    }

    variable setup_iperf 0
    method setup_iperf {} {
	set achive $::UTF::projarchives/unix/UTF
	if {$setup_iperf} {
	    return
	}
	set setup_iperf 1
	set s "$archive/iperf_[$self uname -m]_[$self uname -r]"
	if {[$self sumcheck $s /bin/iperf]} {
	    $self -n -x killall -q iperf
	    $self copyto $s /bin/iperf
	}
    }

    # AP only - no WAN
    method wan {} {}

    # Peer passthroughs
    UTF::PassThroughMethod lan -lanpeer
    UTF::PassThroughMethod relay -relay
    UTF::PassThroughMethod serialrelay -serialrelay


}

# Retrieve manpage from last object
UTF::doc [UTF::STBAP man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF
    UTF::Router Rout1 -lan_ip 192.168.1.1 -sta {STA3 eth2}
    STA3 wl status
    -> Associated with bssid: 6A:41:EA:2F:A0:D4        SSID: "Test381"

    # [example_end]
}

UTF::doc {
    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also wl]
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [see_also [uri APdoc.cgi?UTF::Base.tcl UTF::Base]]
    # [see_also [uri APdoc.cgi?UTF::Cygwin.tcl UTF::Cygwin]]
    # [see_also [uri APdoc.cgi?UTF::Linux.tcl UTF::Linux]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
