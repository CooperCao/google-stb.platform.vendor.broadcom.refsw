#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::Wiced 2.0

package require snit
package require UTF::doc
package require UTF::Test::APRetrieveSecurity

UTF::doc {
    # [manpage_begin UTF::StaticAP n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Static AP/Router support}]
    # [copyright {2016 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::Static host object, wrapper for unmanaged AP/Routers.

    # This is a stub, hosting pre-configured static settings but unable
    # to query or change the device in any way.

    # [list_begin definitions]

}

snit::type UTF::Wiced {
    UTF::doc {
	# [call [cmd UTF::StaticAP] [arg host]
	#	[option -sta] [arg {{name {options} ...}}]
	#	[lb][option -lan_ip] [arg address][rb]
	#	[lb][option -name] [arg name][rb]
	#       [lb][option -lanpeer] [arg peer][rb]]

	# Create a new StaticAP host object.
	# [list_begin options]

	# [opt_def [option -sta] {{name {options} ...}}]

	# List of WiFi interfaces on the Device, and their
	# configurations.  Usually one 2g and one 5g interface.

	# Each interface may be provided with a list of settings.  See
	# example below.  At least one interface must be specified.

	# [opt_def [option -lan_ip] [arg address]]

	# IP address of the device.  If the device is a Router, this
	# will be the internal-facing LAN address.  Defaults to
	# 192.168.1.1

	# [opt_def [option -name] [arg name]]

	# Specify an alternate [arg name] to use when logging.  The
	# default is the came as the [cmd host].

	# [opt_def [option -lanpeer] [arg peer]]

	# Specify an alternate host to use for performance tests, etc.
	# Usually a PC connected to one of the Router's LAN ports.
	# The default is to run performance tests directly against the
	# device.

	# [list_end]
	# [list_end]

	# [para]
	# Router objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    # base handles any other options and methods
    component base -inherit yes

    option -sta
    option -name
    option -relay localhost
    option -retries 1
    option -console
    option -lanpeer
    option -tcpslowstart 0
    option -image
    option -brand linux-external-dongle-sdio
    option -type
    option -date "%date%"
    option -tag
    option -customer bcm
    option -wlinitcmds
    option -sdk "/root/Wiced-SDK"
#    option -sdk "/cygdrive/c/WICED-SDK-3.7.0"
    option -platform "BCM943909WCD1_3.B1"
    option -sdkfw "43909/43909B0.bin"

    variable stas {}

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using UTF::Base %AUTO% -ssh false \
	    -nosharedwep true -escan true -noibss true -tcpwindow 0 \
	    -nocal true -nobx true
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	foreach {sta dev} [UTF::Staexpand $options(-sta)] {
	    UTF::STA ::$sta -host $self {*}$dev
	    lappend stas ::$sta
	}
    }

    destructor {
	foreach {sta dev} $options(-sta) {
	    catch {$sta destroy}
	}
    }

    method make_sdk {args} {
	if {[$options(-relay) info type] eq "::UTF::Cygwin" ||
	    ([$options(-relay) info type] eq "::UTF::STA" &&
	     [$options(-relay) hostis Cygwin])} {
	    set make "./make.exe"
	} else {
	    set make "./make"
	}
	$self relay -n -t 90 \
	"cd $options(-sdk) &&\
$make CONSOLE_ENABLE_WL=1 test.console-$options(-platform) $args"
    }

    method findimages {args} {
	set image [$self cget -image]

	if {$args == "" || $args == "-all"} {
	    set args [concat $args $image]
	}

	# Findimages start
	UTF::Message LOG $options(-name) "findimages $args"

	# Defaults from object
	UTF::GetKnownopts [subst {
	    {all "return all matches"}
	    {ls "Report ls -l"}
	    {showpattern "Show search pattern"}
	    {brand.arg "[$self cget -brand]" "brand"}
	    {tag.arg "$options(-tag)" "Build Tag"}
	    {type.arg "$options(-type)" "Host build Type"}
	    {date.arg "$options(-date)" "Build Date"}
	    {customer.arg "$options(-customer)" "Customer"}
	}]

	set file [lindex $args end]

	# Set search tag to firmware tag by default.  Once object type
	# is determined below, this may switch to dhd_tag.
	set tag $(tag)
	set brand $(brand)
	set date $(date)
	set customer $(customer)

	if {$file eq ""} {
	    # Only add the .bin suffix if it doesn't already have a
	    # .bin or .trx suffix
	    if {[regexp {\.bin|\.trx} $(type)]} {
		set file $(type)
	    } else {
		set file "$(type).bin"
	    }
	} elseif {[file isdirectory $file]} {
	    # Look for a driver, assuming this is part of a developer
	    # workspace.
	    set type $(type)
	    if {![regexp {([^.]*)\.(.*)} $type - typedir typesuffix]} {
		set typedir $type
		set typesuffix "bin"
	    }
	    regsub {/rtecdc$} $typedir {} typedir
	    return [glob "$file{/../build/dongle/$typedir,}/rtecdc.$typesuffix"]
	} elseif {[file exists $file]} {
	    return $file
	}

	if {[regexp {/rtecdc\.(bin|trx|romlsim\.trx)$} $file]} {
	    # Firmware hasn't been copied - pull directly from firmware build
	    set brand hndrte-dongle-wl
	    set tail "{build/dongle,src/dongle/rte/wl/builds}/$file"
	} elseif {[regexp {\.(trx|bin)$} $file]} {
	    set tail [file join release $customer firmware $file]
	} else {
	    error "Unexpected request: $file"
	}

	if {[regexp {_REL_} $tag]} {
	    set tag "{PRESERVED/,ARCHIVED/,}$tag"
	} else {
	    set tag "{PRESERVED/,}$tag"
	}

	set pattern [file join \
			 $::UTF::projswbuild/build_linux \
			 $tag $brand $date* "$tail{,.gz}"]

	if {$(showpattern)} {
	    UTF::Message INFO $options(-name) $pattern
	}
	UTF::SortImages [list $pattern] \
	    {*}[expr {$(ls)?"-ls":""}] \
	    {*}[expr {$(all)?"-all":""}]
    }

    method load {args} {
	UTF::GetKnownopts {
	    {n "Just copy the files, don't actually load the driver"}
	    {all "ignored"}
	    {ls "ignored"}
	}
	UTF::Message INFO $options(-name) "Load Wiced"
        # git pull or git reset --hard, git pull --rebase
	$self relay -n -t 60 \
	    "cd $options(-sdk) && git reset --hard && git pull --rebase"
	# copy FW
	set file [$self findimages {*}$args]
	UTF::Message LOG $options(-name) "Found $file"

	$options(-relay) copyto $file \
	    $options(-sdk)/resources/firmware/$options(-sdkfw)

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}
	$self reload
    }

    method reload {args} {
	$self reboot
	$self _postload
    }

    method _postload {args} {
	UTF::Sleep 1
	if {$options(-wlinitcmds) ne ""} {
	    foreach cmd [split $options(-wlinitcmds) ";"] {
		$self rexec {*}$cmd
	    }
	}
	set ret [$self wl ver]
	regexp {version (.*)} $ret - ret
	return $ret
    }

    method reboot {args} {
	$self make_sdk download run
    }

    method unload {args} {
	$self reboot
	return
    }

    method escanresults {args} {
	set results [$self rexec scan]
	regsub {.*---+\n} $results {} results
	regsub {\n\nEnd of scan results} $results {} results
	set ret ""
	foreach line [split $results \n] {
	    if {[regexp {\d+\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(.+) } $line - mode bssid rssi rate chan sec ssid]} {
		append ret [subst {
SSID: "$ssid"
Mode: $mode  RSSI: $rssi dBm	SNR: ?? dB	noise: -?? dBm	Flags: RSSI 	Channel: $chan
BSSID: $bssid
		}]
	    } else {
		$self warn "Unparsed: $line"
	    }
	}
	return $ret
    }

    method wl {args} {
	set pargs $args
	UTF::GetKnownopts {
	    {silent "Suppress logging"}
	    {u "Unsupported is ok"}
	}
	if {$(silent)} {
	    set s -s
	} else {
	    set s ""
	}

	# fetch any outstanding messages
	$self rexec wlog

	if {[lindex $args 0] eq "escanresults"} {
	    return [$self escanresults]
	}

	set code [catch {$self rexec {*}$s wl {*}$args} ret]
	set ec $::errorCode

	# Special processing for failures

	# Translate error codes
	if {[regexp {wl(?:\.exe)?: error (-\d+)} $ret - i]} {
	    set ret "wl: [UTF::wlerror $i]"
	    UTF::Message LOG $options(-name) $ret
	    if {!$(u)} {
		# -u should only apply to Unsupported, but Wiced
		# doesn't give consistent error codes
		set code 1
	    }
	}

	# Special processing for success

	# Strip verbiage
	if {[lsearch $args ssid] >= 0} {
	    regexp {(?:Current |^)SSID: "(.*)"} $ret - ret
	} elseif {[lsearch $args bssid] >= 0} {
	    regsub {bssid is } $ret {} ret
	} elseif {[lsearch $args cur_etheraddr] >= 0} {
	    regsub {cur_etheraddr } $ret {} ret
	}

	return -code $code -errorcode $ec $ret
    }

    method ifconfig {dev args} {
	if {$args eq "dhcp"} {
	    $warn "Unimplemented"
	} elseif {[llength $args] eq 0} {
	    $self rexec status
	} elseif {[llength $args] eq 1} {
	    $self rexec set_ip [join [lreverse [split [lindex $args 0] "."]] "."] \
		0.255.255.255 0.0.0.0
	} else {
	    $warn "Unimplemented"
	}
	$self rexec status
    }

    UTF::doc {
	# [call [arg host] [method ping] [arg target]
	#	[lb][opt -c] [arg count][rb]
	#       [lb][opt -s] [arg size][rb]]

	# Ping from [arg host] to [arg target].  Target may be a STA
	# or a Hostname/IP address.  Returns success if a response
	# packet is received and error if not.  Returns success if a
	# response packet is received before [arg count] (default 5)
	# packets have been sent.  Packet size option [opt -s] [arg
	# size] is ignored.
    }

    method ping {target args} {
	set msg "ping $target $args"
	UTF::Getopts {
	    {c.arg "5" "Count"}
	    {s.arg "56" "Size"}
	} args
	if {[info commands $target] == $target} {
	    set target [$target ipaddr]
	}
	# Loop, since Wiced ping doesn't handle short-circuit "count"
	for {set c 0} {$c < $(c)} {incr c} {
	    set ret [$self rexec ping $target -l $(s)]
	    $self rexec wlog
	    if {[regexp {Ping Reply} $ret]} {
		return
	    }
	}
	error "ping failed"
    }

    method join {AP} {
	set SSID [$AP cget -ssid]
	if {[set security [$AP cget -security]] eq ""} {
	    UTF::Test::APRetrieveSecurity $AP
	    set security [$AP cget -security]
	}

	switch $security {
	    open {
		set amode open
		set key "dummy"
	    }
	    wep {
		set amode "wep"
		set key [$AP cget -wepkey]
	    }
	    tkippsk {
		set amode wpa_tkip
		set key [$AP cget -wpakey]
	    }
	    tkippsk2 {
		set amode wpa2_tkip
		set key [$AP cget -wpakey]
	    }
	    aespsk {
		set amode wpa_aes
		set key [$AP cget -wpakey]
	    }
	    aespsk2 {
		set amode wpa2
		set key [$AP cget -wpakey]
	    }
	    default {
		error "Unsupported: $security"
	    }
	}
	set ip [[lindex $stas 0] cget -ipaddr]
	set APIP [$AP ipaddr]
	$self rexec wlog
	if {$ip eq "dhcp"} {
	    $self rexec join $SSID $amode $key
	} else {
	    $self rexec join $SSID $amode $key $ip 255.255.255.0 $APIP
	    UTF::Sleep 6
	}
	$self rexec wlog
	if {[$self cget -post_assoc_hook] ne ""} {
	    eval [string map [list %S $self] [$self cget -post_assoc_hook]]
	}
	try {
	    $self ping $APIP -c 10
	} finally {
	    $self rexec status
	}
    }

    method ipaddr {dev} {
	if {[regexp -line {IP Addr +: (.*)} [$self rexec status] - addr]} {
	    return $addr
	} else {
	    error "No IP address"
	}
    }

    method macaddr {dev} {
	if {[regexp -line {MAC address is: (.*)} [$self rexec get_mac_addr] - addr]} {
	    return $addr
	} else {
	    error "No Mac address"
	}
    }

    method rte_available {} {
	return 1
    }

    UTF::doc {
	# [call [arg host] [method freekb]]

	# Returns free mem kbytes using "mu" RTE command on the dongle.
    }

    method freekb {} {
	if {![regexp {Heap Free: (\d+)} [$self wl memuse] - free]} {
	    error "Free mem not found"
	}
	expr {$free / 1024.0}
    }

    UTF::doc {
	# [call [arg host] [method maxmem]]

	# Returns maxmem bytes using "mu" RTE command on the dongle
	# serial port.
    }

    method maxmem {} {
	if {![regexp {Heap Total: (\d+)} [$self wl memuse] - mm]} {
	    error "Max memory not found"
	}
	return $mm
    }

    UTF::doc {
	# [call [arg host] [method boardname]]

	# Returns "StaticAP"
    }

    method boardname {} {
	return "StaticAP"
    }

    UTF::doc {
	# [call [arg host] [method whatami]]

	# Returns "StaticAP"
    }

    method whatami {STA} {
	if {[catch {$STA chipname} c]} {
	    UTF::Message WARN $options(-name) $c
	    set c "<unknown>"
	}
	return "[$STA hostis] $c"
    }

    UTF::doc {
	# [call [arg host] [method wan]]

	# Returns empty as we don't support a WAN on this device.
    }

    # Peer passthroughs
    UTF::PassThroughMethod relay -relay

    variable socket
    variable partial ""
    variable result ""
    variable stack ""
    variable depth 1

    method __getdata {} {
	# Internal read handler for the console socket.  Appends
	# non-empty responses from the console to the response stack.
	# On EOF it closes the socket.
	set msg [read $socket]
	regsub -all {\r} $msg {} msg
	if {$msg ne ""} {
	    append partial $msg
	    while {[regexp {^(.*?)\n(.*)$} $partial - lines partial]} {
		UTF::_Message LOG $options(-name) $lines
		append result "$lines\n"
		if {[regexp {^> (.*)$} $partial - partial]} {
		    lappend stack $result
		    set result ""
		}
	    }
	    if {[regexp {^> (.*)$} $partial - partial]} {
		lappend stack $result
		set result ""
	    }
	}
	if {[eof $socket]} {
	    UTF::Message LOG $options(-name) EOF
	    close $socket; # nonblocking, so close should never fail
	    unset socket
	}
    }

    method __putdata {line} {
	# If socket is missing, or not readable, reopen it.  The call
	# to __getdata forces a read on the socket in order to provoke
	# an error if the socket needs reopening.  We don't expect
	# __getdata to have any data to read, but if it does it will
	# be reported as normal.  The "set socket" will catch the case
	# where the socket timed-out just as we were about to use it.
	# If calls are nested, only the outer layer gets to open the
	# socket.

	if {$depth < 2 && [catch {$self __getdata; set socket} ret]} {
	    # Missing socket variable and non-opened channel are
	    # normal.  Other failures should be reported.
	    if {![regexp {"socket": no such variable|find channel} $ret]} {
		UTF::Message WARN $options(-name) $ret
	    }

	    # (re)init stack
	    set stack ""

	    set socket [$options(-relay) socket $options(-console)]
	    fconfigure $socket -blocking 0 -buffering line -translation binary
	    fileevent $socket readable [mymethod __getdata]
	}
	# We don't expect the write to fail since it's nonblocking and
	# we already verified the socket was connected
	puts $socket $line
    }

    UTF::doc {
	# [call [arg host] [method waitfor] [arg pattern]]

	# vwaits collecting results until the results match the regexp
	# specified by [arg pattern].  If [arg pattern] already
	# matches then no vwait is done.  This is useful for waiting
	# for async processes to start up.  The caller should still
	# call [method close] later to clean up.
    }

    method __waitfor {pattern} {
	if {[regexp $pattern $result]} {
	    # Already found
	    return $result
	}
	while {1 || (![$self timed_out] && ![eof $fd])} {
	    vwait [myvar result]
	    if {[regexp $pattern $result]} {
		return $result
	    }
	}
	throw NotFound "pattern not found"
    }

    method __close {} {
	# Collect results.  Depth is used to measure recursive calls.
	# Collect enough results to satisfy the current depth then the
	# outer calls can unwind the stack to return their results in
	# the right order.  Timeouts will be recorded on the stack
	# just like data so we shouldn't get out of sync.
	while {[llength $stack] < $depth} {
	    # Need more results
	    incr depth
	    vwait [myvar stack]
	    incr depth -1
	}

	set response [lindex $stack end]
	set stack [lreplace $stack end end]

	return $response
    }



    method rexec {args} {
	# general
	set handler [UTF::Wiced::Rexec %AUTO% -base $self -args $args]
	$handler open
	if {[$handler async]} {
	    return $handler
	} else {
	    $handler close
	}
    }

}

snit::type UTF::Wiced::Rexec {
    option -base -readonly yes
    option -args -readonly yes
    component base

    variable name
    variable fd
    variable data ""
    variable Timer
    variable timer
    variable timeout
    variable before
    variable timed_out 0
    variable pargs -array {}

    constructor {args} {
	$self configurelist $args
	set base $options(-base)
	set name [$base cget -name]
    }

    method open {} {
	set rargs $options(-args)
	#UTF::Message DBG $name "rexec open $rargs"
	if {[catch {cmdline::typedGetoptions rargs {
	    {async "Return handle to be closed later"}
	    {keepnewline "Retain a trailing newline"}
	    {timeout.integer 30 "Seconds to timeout if no data received"}
	    {Timeout.integer 300 "Seconds to timeout if command not completed"}
	    {expect.integer 0 "Seconds to report failure, even if command completed"}
	    {desc.arg "" "Description"}
	    {quiet "Don't log commandline"}
	    {silent "Don't log results"}
	    {noinit "Don't run init code"}
	    {binary "Preserve binary data"}
	    {in "Don't close stdin"}
	    {x "Ignore exit codes"}
	    {2 "Don't dup 2 onto 1"}
	    {K "Don't retry (used by retries to prevent recursion)"}
	    {prefix.arg "" "Logging prefix, to identify concurrent streams"}
	    {clock "Add timestamps"}
	} "rexec options"} ret]} {
	    error $ret $ret
	} else {
	    array set pargs $ret
	}

	if {![info exists pargs(desc)]} {
	    set pargs(desc) [join $rargs]
	}

	# Scale command timout
	set Timeout [expr {$pargs(Timeout)*1000}]
	# Scale IO timout
	set timeout [expr {$pargs(timeout)*1000}]

	set vargs {}
	foreach a $rargs {
	    if {[regexp {\W} $a]} {
		lappend vargs "'$a'"
	    } else {
		lappend vargs $a
	    }
	}
	set cmd [join $vargs { }]

	# Total timeout can't be less than IO timeout
	if {$Timeout < $timeout} {
	    set Timeout $timeout
	}

	# Start command timer
	set Timer [after $Timeout [mymethod kill]]

	# Start command timer
	set timer [after $timeout [mymethod kill]]

	$base __putdata "$cmd\r"
    }

    method timed_out {} {
	if {!$timed_out &&
	    [catch {after info $timer; after info $Timer}]} {
	    set timed_out 1
	}
	set timed_out
    }

    method async {} {
	info exists pargs(async)
    }

    method kill {} {
	UTF::Message LOG $name "Timeout: $pargs(desc): resetting..."
	after cancel $timer; # clear io timer
	after cancel $Timer; # clear command timer
	set timed_out 1
	set data $data
	$base reload
    }

    UTF::doc {
	# [call [arg host] [method waitfor] [arg pattern]]

	# vwaits collecting results until the results match the regexp
	# specified by [arg pattern].  If [arg pattern] already
	# matches then no vwait is done.  This is useful for waiting
	# for async processes to start up.  The caller should still
	# call [method close] later to clean up.
    }

    method waitfor {pattern} {
	$base __waitfor $pattern
    }

    method close {} {
	if {![$self timed_out]} {
	    set response [$base __close]
	}
	after cancel $Timer; # clear command timer
	after cancel $timer; # clear io timer

	if {$response eq "TIMEOUT"} {
	    error $response
	}
	regsub {^[^\n]*\n} $response {} response
	if {![info exists pargs(keepnewline)]} {
	    regsub {\n$} $response {} response
	}
	return $response
    }



}

# Retrieve manpage from last object
UTF::doc [UTF::Wiced man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]

    # [example_end]
}

UTF::doc {
    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
