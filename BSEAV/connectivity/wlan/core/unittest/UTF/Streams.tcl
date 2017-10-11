#!/bin/env utf
#
#
# Streams object used to manage multiple traffic streams
#
# Author: Robert McMahon
# Date: October 2010-
#
# Notes:  Tcptrace at http://www.tcptrace.org requires pcap
# "e.g. yum install libpcap-devel"
#
# Requires iperf 2.0.8 or better
#
# $Id$
# $Copyright Broadcom Corporation$
# package require snit
package require UTF
package require base64
package require ip
package require md5
package require UTF::Multicast
package require UTF::Streamslib
package require UTF::RateLimiter
package require UTF::math

package provide UTF::Streams 2.0

snit::type UTF::stream {
    typevariable IPTABLESCMD "/sbin/iptables"
    typevariable TCCMD "/sbin/tc"
    typevariable STARTTIMEOUT 2000
    typevariable CLOSETIMEOUT 5000
    typevariable FASTTIMEOUT 3000
    typevariable DEFAULTTIMEOUT 3000
    typevariable TXCLOSEACK 250
    typevariable multicast_base "239.0.0.0"
    typevariable multicast_offset 0
    typevariable IPERFPORTBASE "61000"
    typevariable tc_minorid_running "100"
    typevariable tc_flowid_running "500"
    typevariable IPERFPORTOFFSET 0
    typevariable iperfrestorestates -array {}
    typevariable iptablescache -array {}
    typevariable setupneeded -array {}
    typevariable LOGGINGLEVEL "DISPLAY"
    typevariable STREAMSIPTABLESSTRING "Rule installed by UTF::Streams"
    typevariable D4 "3.267"
    typevariable E2 "2.660"
    typevariable defaultplotter {}
    typevariable L2HDRSIZE "32"
    typevariable TCPDUMPSETTLE 850
    typevariable NTPLOCK -array {}
    typevariable TC -array {}
    typevariable TXQUEUELEN 1000
    typevariable TCPTRACECMD "/usr/bin/tcptrace"
    typevariable DEFAULTRWIN "16K"
    typevariable LANPEERIX -array {}
    typevariable POLICYROUTEREF -array {}

    typemethod allstreams {args} {
	if {$LOGGINGLEVEL eq "DISPLAY" || $LOGGINGLEVEL eq "DEBUG"} {
	    UTF::Message API-I STREAMS "typemethod allstreams called with $args"
	}
	set instances [$type info instances]
	set faillist {}
	set cmd [string tolower [lindex $args 0]]
	if {$cmd eq {stats}} {
	    append args " -ignore_clearsynch"
	    if {[lindex $args 1] eq {-clear}} {
		set cmd clear
	    }
	}
	if {$cmd eq {linkcheck} && [lindex $args 1] eq {-now}} {
	    foreach instance $instances {
		eval $instance linkcheck_reset
	    }
	    set args [lsearch -all -not -inline $args {-now}]
	}
	set res {}
	foreach instance $instances {
	    if {$cmd eq {start} || $cmd eq {stop} || $cmd eq {linkcheck}} {
		if {[catch {eval $instance $cmd [lrange $args 1 end]} err]} {
		    UTF::Message ERROR STREAMS "linkcheck: [$instance cget -name] $instance"
		    lappend faillist [$instance cget -name]
		}
	    } else {
		set res [concat $res [eval [concat $instance $args]]]
	    }
	}
	if {$cmd eq {start} || $cmd eq {stop} || $cmd eq {clear}} {
	    foreach aggsampler [::UTF::StreamStatAggregate info instances] {
		$aggsampler $cmd
	    }
	}
	if {$LOGGINGLEVEL eq "DISPLAY" || $LOGGINGLEVEL eq "DEBUG"} {
	    UTF::Message API-E STREAMS "typemethod allstreams $args done"
	}
	if {[llength $faillist]} {
	    UTF::Message ERROR STREAMS $faillist
	    error "allstreams had one or more exceptions: $faillist"
	}
	return $res
    }

    typemethod logginglevel {args} {
	if {$args eq {}} {
	    return $LOGGINGLEVEL
	}
	set level [string toupper [lindex $args 0]]
	switch -exact $level {
	    "ANALYZE" -
	    "DISPLAY" -
	    "DEBUG" {
		set LOGGINGLEVEL $level
	    }
	    default {
		set msg "Invalid logging level $level requested"
		UTF::Message ERROR STREAMS $msg
		error $msg
	    }
	}
    }
    typemethod incrbaseport {{value 100}} {
	set IPERFPORTBASE [expr {$value + $IPERFPORTBASE}]
    }
    typemethod createstreams {num} {
	set streamlist ""
	for {set ix 0} {$ix < $num} {incr ix} {
	    lappend streamlist [uplevel #0 $type create %AUTO%]
	}
	return $streamlist
    }

    typemethod exitstreams {} {
	eval [mytypemethod allstreams "destroy"]
	eval [mytypemethod restoreiperfservices]
    }

    typemethod setdefaultplotter {name} {
	set defaultplotter $name
    }

    typemethod getdefaultplotter {} {
	return $defaultplotter
    }

    typemethod restoreiperfservices {} {
	if {![array exists iperfrestorestates]} {
	    return
	}
	if {$LOGGINGLEVEL eq "DISPLAY" || $LOGGINGLEVEL eq "DEBUG"} {
	    UTF::Message API-I STREAMS "typemethod restoreiperf called : [array names iperfrestorestates]"
	}
	foreach host [array names iperfrestorestates] {
	    if {$iperfrestorestates($host) eq "start"} {
		if {[catch {$host rexec -n {sc query IPerfService | grep STATE}}]} {
		    $host -n iperf -s -D
		}
		$host -n sc config IPerfService start= auto
		# $host -n sc start IPerfService
		$host -n sc failure IPerfService reset= 0 actions= restart/60000
		if  {[iperfquerystate $host] ne $iperfrestorestates($host)} {
		    UTF::Message WARN $host "Unable to restore IperfService to $iperfrestorestates($host)"
		}
	    }
	    unset iperfrestorestates($host)
	}
	if {$LOGGINGLEVEL eq "DISPLAY" || $LOGGINGLEVEL eq "DEBUG"} {
	    UTF::Message API-E STREAMS "typemethod restoreiperf exit"
	}
    }
    typemethod clock {} {
	set secs [clock seconds]
	set ms [clock clicks -milliseconds]
	set base [expr { $secs * 1000 }]
	set fract [expr { $ms - $base }]
	if { $fract >= 1000 } {
	    set diff [expr { $fract / 1000 }]
	    incr secs $diff
	    incr fract [expr { -1000 * $diff }]
	}
	return $secs.[format %03d $fract]
    }
    typemethod init_txshaper {host dev} {
	if {[catch {info exists $TC($host,$dev)}]} {
	    catch {eval [concat $host rexec ifconfig $dev txqueuelen $TXQUEUELEN]}
	    catch {eval [concat $host rexec $TCCMD qdisc del dev $dev root]}
	    catch {eval [concat $host rexec $IPTABLESCMD -F -t mangle]}
	    eval [concat $host rexec $TCCMD qdisc add dev $dev root handle 1: htb]
	    set TC($host,$dev) 1
	}
    }
    typemethod get_next_flowid {} {
	incr tc_flowid_running
	return $tc_flowid_running
    }
    typemethod hexpand {n} {
	if {[regexp -nocase {^(\d+(?:.\d+)?)k$} $n - n]} {
	    expr {1000 * $n}
	} elseif {[regexp -nocase {^(\d+(?:.\d+)?)m$} $n - n]} {
	    expr {1000000 * $n}
	} elseif {[regexp -nocase {^(\d+(?:.\d+)?)g$} $n - n]} {
	    # Return a double to avoid int wrapping
	    expr {1000000000.0 * $n}
	} else {
	    set n
	}
    }
    typemethod hformat {args} {
	set results {}
	if {[llength $args] == 1} {
	    set args [lindex $args 0]
	}
	foreach value $args {
	    set tmp [format %0.0f $value]
	    set v [expr {abs($value)}]
	    if {$tmp eq "0" || $v < 1000} {
		if {$v < 1} {
		    set value [format $value %.3f]
		} elseif {$v < 10} {
		    set value [format $value %.2f]
		} elseif {$v < 100} {
		    set value [format $value %.1f]
		} else {
		    set value [format $value %.0f]
		}
		lappend results $value
		continue
	    } elseif {[string index $value 0] eq "-"} {
		set sign {-}
		set value [string range $tmp 1 end]
	    } else {
		set sign {}
		set value $tmp
	    }
	    set numchars [string length $value]
	    switch -exact $numchars {
		"4" -
		"5" -
		"6" { lappend results "${sign}[format %0.3f [expr {1.0 * $value / 1000}]]K"}
		"7" -
		"8" -
		"9" { lappend results "${sign}[format %0.3f [expr {1.0 * $value / 1000000}]]M"}
		"10" -
		"11" -
		"12" { lappend results "${sign}[format %0.2f [expr {1.0 * $value / 1000000000}]]G"}
		"13" -
		"14" -
		"15" { lappend results "${sign}[format %0.1f [expr {1.0 * $value / 1000000000000}]]T"}
		"16" -
		"17" -
		"18" { lappend results "${sign}[format %0.1f [expr {1.0 * $value / 1000000000000000}]]P"}
		default {
		    lappend results $value
		}
	    }
	}
	return $results
    }
    typemethod config_tcptrace {{filename ""}} {
	if {$filename ne ""} {
	    set TCPTRACECMD $filename
	} else {
	    set TCPTRACECMD "/usr/local/bin/tcptrace"
	}
	if {![file exists $TCPTRACECMD]} {
	    error "$TCPTRACECMD does not exist"
	}
	if {![file executable $TCPTRACECMD]} {
	    error "$TCPTRACECMD does not executable"
	}
    }
    option -tx -configuremethod __configurepoas
    option -rx -configuremethod __configurepoas
    option -transmitsta -validatemethod __validatesta -configuremethod __configurepoas
    option -multicast -type boolean -default 0 -readonly true
    option -receivesta -validatemethod __validatesta -configuremethod __configurepoas
    option -protocol -default "UDP" -validatemethod __validateprotocol -configuremethod __configureprotocol
    option -txdisplay -type boolean -default 0
    option -rxdisplay -type boolean -default 0
    option -eventmessages -type boolean -default 0
    option -eventcallback ""
    option -reportinterval -default "0.5" -configuremethod __configureoptionwithapply
    option -latencyreportinterval -default "2.0"
    option -latencymax -default {}
    option -traffictype -default "BESTEFFORT" -validatemethod __validatetraffictype -configuremethod __configuretraffictype
    option -rate -default "-1" -validatemethod __validaterate -configuremethod __configureoptionwithapply
    option -pktsize -default 1470 -type integer -configuremethod __configureoptionwithapply
    option -pps -default 244 -type integer -configuremethod __configureoptionwithapply
    option -ttl -default 255 -type integer -configuremethod __configureoptionwithapply
    option -tos -default 0x0 -configuremethod __configureoptionwithapply
    option -msgcolor -validatemethod __validatemsgcolor -configuremethod __configuremsgcolor
    option -dstgrpip -configuremethod __configureoptionwithapply -default -1
    option -dstport -configuremethod __configureoptionwithapply -default auto
    option -dstip -configuremethod __configureoptionwithapply
    option -advancedstats -type boolean -default 1
    option -activestats -type boolean -default 0
    option -trigger_callback
    option -trigger_armed -type boolean -default 0
    option -installroutes -default -1
    option -udprx_winpktcnt -configuremethod __configureoptionwithapply -default 5000
    option -udptx_winpktcnt -configuremethod __configureoptionwithapply -default 250
    option -bind -type boolean -default 1
    option -dupadjust -type boolean -default 1
    option -restart_on_fsherror -type boolean -default 0
    option -restart_on_iperferror -type boolean -default 1
    option -debug_msgs -type boolean -default 0
    option -api_msgs -type boolean -default 0
    option -clearsynch -type boolean -default 0
    option -sniff_rate_limiter -type integer -default 1 ;# units of seconds
    option -rfsniffer
    option -sniffnodes -default ""
    option -holdppsconstant -type boolean -default 0
    option -nice -type boolean -default 0
    option -sniff_callback
    option -dwell -type integer -default 3
    option -caltries -type integer -default 180
    option -iperfseconds -type integer -default 5356800
    option -oversubscribe -default 500M
    option -txstart_timeout -type integer -default 3000
    option -latencycount -type integer -default 25
    # b, k, m, g, a for bit, Kbit, Mbit, Gbit, adaptive bit
    option -format "b"
    option -clockip -default ""
    option -latency -type boolean -default 0
    option -linkcheckfailcount -type integer -default 4
    option -w -default -1 -configuremethod __configureoptionwithapply
    option -shaperrate -validatemethod __validaterate -configuremethod __configureoptionwithapply -default ""
    option -shaperceil -validatemethod __validaterate -configuremethod __configureoptionwithapply -default ""
    option -quantum -type integer -default 1500
    option -tcptune -type boolean -default 0
    option -tcptrace -type boolean -default 0
    option -linetype -default "auto"
    option -name -default "" -configuremethod __configurename
    option -txnulldetect -type boolean -default 0
    option -rxnulldetect -default -1
    option -key -readonly true -default {}
    option -rwin -default -1
    option -linkcheck -type boolean -default 1
    option -t -default 10
    option -staticarp -type boolean -default 0
    option -noblock -type boolean -default 0
    option -policyrouting -default -1
    option -readdelay -default -1
    option -initcwnd -default -1 -readonly true
    option -initrtt -default -1 -readonly true
    option -initrttvar -default -1 -readonly true
    option -fwlatency -type boolean -default false
    option -pdfs -type boolean -default false
    option -tcpdebug -type boolean -default false

    variable txstate "OFF"
    variable rxstate "OFF"
    variable rxfids
    variable txfids {}
    variable samples -array {}
    variable timestamps -array {}
    variable fid2pid -array {}
    variable fid2device -array {}
    variable fid2afterid -array {}
    variable handlerstate -array {}
    variable iperfcmds -array {}
    variable runningstats -array {}
    variable starttime {}
    variable restorecmds -array {}
    variable restart_on_txclose 0
    variable routes -array {}
    variable outoforder_posted -array {}
    variable linkcheck_flags -array {}
    variable noreentrancy -array {}
    variable utf_txmsgtag {}
    variable utf_rxmsgtag {}
    variable utf_msgtag {}
    variable restart_pending 0
    variable clearstats_synch 0
    variable userdata {}
    variable myip
    variable sniffstate
    variable sniff_fid2device -array {}
    variable sniff_buffs -array {}
    variable samplersenabled 1
    variable latency -array {}
    variable lan2sta -array {}
    variable iperfseconds
    variable reqpktsize
    variable reqwinsize
    variable reqrate
    variable rexec_cmd
    variable rpopen_cmd
    variable activestats -array {}
    variable doonetimesetup
    variable exceptions -array {}
    variable tc_minorid
    variable HNDLRTAG
    variable tcptrace -array {}
    variable ccplots -array {}
    variable cctests -array {}
    variable liveplots -array {}
    variable arpstate "UNKNOWN"
    variable pdfToT -array {}
    variable pdfDHD -array
    variable pdfDHDd -array {}
    variable pdfFWd1 -array {}
    variable pdfFWd2 -array {}
    variable pdfFWd3 -array {}
    variable pdfFWd4 -array {}

    constructor {args} {
	incr multicast_offset
	incr tc_minorid_running
	set tc_minorid $tc_minorid_running
	switch -exact $LOGGINGLEVEL {
	    "ANALYZE"  {
		$self configure -txdisplay 0 -rxdisplay 0 -api_msgs 0 -debug_msgs 0
	    }
	    "DISPLAY" {
		$self configure -txdisplay 1 -rxdisplay 1 -api_msgs 1 -debug_msgs 0
	    }
	    "DEBUG" {
		$self configure -txdisplay 1 -rxdisplay 1 -api_msgs 1 -debug_msgs 1
	    }
	}
	# Have pktsize take priority in configuration
	set index [lsearch $args "-pktsize"]
	if {$index ne -1} {
	    $self configure -pktsize [lindex $args [expr {$index + 1}]]
	    set args [lreplace $args $index [expr {$index + 1}]]
	}
	$self configurelist $args
	if {$options(-dstport) eq "auto"} {
	    incr IPERFPORTOFFSET
	    set options(-dstport) [expr {$IPERFPORTBASE + $IPERFPORTOFFSET}]
	}
	if {$options(-multicast)} {
	    if {$options(-dstgrpip) eq "-1"} {
		set options(-dstgrpip) [ipIncr $multicast_base $multicast_offset]
	    }
	    if {$options(-installroutes) eq "-1"} {
		set options(-installroutes) 0
	    }
	} else {
	    set options(-dstgrpip) "-1"
	}
	if {$options(-protocol) eq "TCP"} {
	    if {$options(-initcwnd) eq "max" && $options(-w) ne "-1"} {
		set  options(-initcwnd) [UTF::kexpand $options(-w)]
	    } elseif  {$options(-initcwnd) ne "-1"} {
		set  options(-initcwnd) [expr {int([UTF::kexpand $options(-initcwnd)])}]
	    }
	}
	if {$options(-installroutes) eq "-1" && ($options(-initrtt) ne "-1" || $options(-initrttvar) ne "-1" || $options(-initcwnd) ne "-1")} {
	    set options(-installroutes) "1"
	}
	# Use a scale factor per every 50 streams to set the timeout
	set FASTTIMEOUT [expr {(([llength [$type info instances]] / 50) + 1) * $DEFAULTTIMEOUT}]
	if {$options(-name) eq ""} {
	    set utf_txmsgtag "[namespace tail $self]-tx"
	    set utf_rxmsgtag "[namespace tail $self]-rx"
	    set utf_msgtag "[namespace tail $self]"
	    set options(-name) $utf_msgtag
	}
	set sniffstate "OFF"
	set latency(fids) {}
	if {[catch {set ::tcl_interactive} flag] || !$flag} {
	    set rexec_cmd "rexec"
	    set rpopen_cmd "rpopen"
	} else {
	    set rexec_cmd "rexec -quiet -silent"
	    set rpopen_cmd "rpopen -quiet"
	}
	set tcptrace(pcaps) {}
	set tcptrace(endtimes) {}
	set doonetimesetup 1
	if {$options(-rxnulldetect) eq "-1" && $options(-reportinterval) < 0.5} {
	    set options(-rxnulldetect) 0
	}
    }
    destructor {
	if {[catch {$self stop} err]} {
	    UTF::Message ERROR $utf_msgtag $err
	}
	catch {$self tcptrace stop}
	foreach sta [concat $options(-transmitsta) $options(-receivesta)] {
	    if {[catch {$self __dorestore $sta} err]} {
		UTF::Message ERROR $utf_msgtag $err
	    }
	}
	# Use a scale factor per every 50 streams to set the timeout
	set FASTTIMEOUT [expr {(([llength [$type info instances]] / 50) + 1) * $DEFAULTTIMEOUT}]
	foreach endpoint [concat $options(-receivesta) $options(-transmitsta)] {
	    if {[$endpoint cget -lanpeer] ne ""} {
		catch {incr LANPEERIX($endpoint) -1}
	    }
	}
	# If killing the last stream, do any final cleanup
	if {[llength [UTF::stream info instances]] == 1} {
	    if {[catch {eval [mytypemethod restoreiperfservices]} err]} {
		UTF::Message ERROR $utf_msgtag $err
	    }
	    array unset LANPEERIX *
	}
	if {$options(-installroutes) eq "1"} {
	    catch {$self delete_host_route}
	}
	if {$options(-shaperrate) ne ""} {
	    $self clear_shapers
	}
	if {[info exists sniffstate] && $sniffstate ne "OFF"} {
	    $self sniff stop
	}
	if {$arpstate eq "STATIC"} {
	    $self arp delete
	}
	$self tcptrace xfer
    }
    method id {args} {
	UTF::GetKnownopts {
	    {exclude.arg "" "attributes to exclude from the hash"}
	}
	set key "$options(-key) $args"
	set exceptlist "-rxdisplay -txdisplay -clockip -debug_msgs -api_msgs -format -msgcolor -rfsniffer -linetype -tcptrace -name -dstport"
	foreach x $(exclude) {
	    lappend exceptlist "-[string trimleft $x {-}]"
	}
	foreach n [lsort [array names options]] {
	    if {[lsearch -exact $exceptlist $n] == -1} {
		lappend key $n $options($n)
	    }
	}
	regsub -all {[/<>]} $key "." key
	set hash [::md5::md5 -hex $key]
	UTF::Message INFO $utf_msgtag "${hash}=$key"
	return $hash
    }

    method ID {args} {
	UTF::GetKnownopts {
	    {exclude.arg "" "attributes to exclude from the hash"}
	}
	set key "$options(-key) $args"
	set includelist "-holdppsconstant -initcwnd -initrtt -initrttvar -iperfseconds -multicast -pktsize -pps -protocol -rate -rx -receivesta -shaperrate -tos -transmitsta -tx -w"
	foreach x $(exclude) {
	    lappend exceptlist "-[string trimleft $x {-}]"
	}
	foreach n [lsort $includelist] {
	    if {[lsearch -exact $exceptlist $n] == -1} {
		lappend key $n $options($n)
	    }
	}
	regsub -all {[/<>]} $key "." key
	set hash [::md5::md5 -hex $key]
	UTF::Message INFO $utf_msgtag "${hash}=$key"
	return $hash
    }


    method staname {lan} {
	return $lan2sta($lan)
    }
    method set_userdata {args} {
	set userdata $args
    }
    method get_userdata {args} {
	return $userdata
    }
    method inspect {} {
	puts "\nActive fids:"
	catch {puts "\  $fid2device($txfid)(tx)=$txfid"}
	if {[info exists rxfids]} {
	    foreach rxfid $rxfids {
		puts "\  $fid2device($rxfid)(rx)=$rxfid"
	    }
	}
	if {[info exists ::tcl_interactive_developer]} {
	    puts "\nInternal variables:"
	    if {[info exists fid2device]} {
		parray fid2device
	    }
	    if {[info exists fid2pid]} {
		parray fid2pid
	    }
	    if {[info exists fid2afterid]} {
		parray fid2afterid
	    }
	    if {[info exists handlerstate]} {
		parray handlerstate
	    }
	    if {[info exists iperfcmds]} {
		parray iperfcmds
	    }
	    if {[info exists iperfrestorestates]} {
		parray iperfrestorestates
	    }
	}
    }
    method introspect {{sleeptime 1}} {
	set restore(rxdisplay) $options(-rxdisplay)
	set restore(txdisplay) $options(-txdisplay)
	if {$options(-protocol) eq "tcp"} {
	    set proto "t"
	} else {
	    set proto "u"
	}
	UTF::Message INSPECT $utf_txmsgtag "Tx status: [$self status -tx]"
	UTF::Message INSPECT $utf_rxmsgtag "Rx status: [$self status -rx]"
	UTF::Message INSPECT $utf_txmsgtag "Tx IP:port [$options(-transmitsta) ipaddr]:$options(-dstport)"
	UTF::Message INSPECT $utf_rxmsgtag "Rx IP:port [$options(-receivesta) ipaddr]:$options(-dstport)"
	catch {$options(-transmitsta) rexec ps -ef | grep iperf}
	catch {$options(-receivesta) rexec ps -ef | grep iperf}
	catch {$options(-transmitsta) ping $options(-receivesta)}
	if {[$options(-transmitsta) hostis MacOS]} {
	    catch {$options(-transmitsta) rexec arp -alnx}
	} else {
	    catch {$options(-transmitsta) rexec arp -env}
	}
	if {[$options(-receivesta) hostis MacOS]} {
	    catch {$options(-receivesta) rexec arp -alnx}
	} else {
	    catch {$options(-receivesta) rexec arp -env}
	}
	UTF::Message INSPECT $utf_msgtag "TX netstat"
	if {[$options(-transmitsta) hostis MacOS]} {
	    $options(-transmitsta) rexec netstat -$proto -n -v -f inet
	} else {
	    $options(-transmitsta) rexec netstat -$proto -n -v
	}
	UTF::Message INSPECT $utf_msgtag "RX netstat"
	if {[$options(-receivesta) hostis MacOS]} {
	    $options(-receivesta) rexec netstat -$proto -n -v -f inet
	} else {
	    $options(-receivesta) rexec netstat -$proto -n -v
	}
	$options(-receivesta) rexec netstat -$proto -ln
	catch {$options(-transmitsta) rexec netstat -rn}
	catch {$options(-receivesta) rexec netstat -rn}
	UTF::Message INSPECT $utf_msgtag "TX and RX"
	set options(-rxdisplay) 1; set options(-txdisplay) 1
	UTF::Sleep [expr {2 * $options(-reportinterval)}] quiet
	UTF::Message INSPECT $utf_msgtag "TX only"
	set options(-rxdisplay) 0; set options(-txdisplay) 1
	UTF::Sleep [expr {2 * $options(-reportinterval)}] quiet
	UTF::Message INSPECT $utf_msgtag "RX only"
	set options(-rxdisplay) 1; set options(-txdisplay) 0
	UTF::Sleep [expr {2 * $options(-reportinterval)}] quiet
	set options(-rxdisplay) $restore(rxdisplay); set options(-txdisplay) $restore(txdisplay)
	$self sniff on
	UTF::Sleep [expr {4 * $options(-reportinterval)}] quiet
	$self sniff off
    }
    method __validateprotocol {option value} {
	set tmp [string toupper $value]
	if {!([string equal $tmp "UDP"] || [string equal $tmp "TCP"])} {
	    set msg "Invalid protocol $tmp"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	if {$tmp eq "TCP" && $options(-multicast)} {
	    set msg "Cannot use TCP on multicast stream"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
    }
    method __validaterate {option value} {
	set value [string toupper $value]
	if {![regexp {([0-9.]+)[GKM]?[\s]*$} $value - intvalue]} {
	    set msg "Invalid rate format. Valid examples are 1G, 10M and 9K"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	if {![string is double $intvalue]} {
	    set msg "Must use integer for numeric value"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
    }
    method __validatemsgcolor {option value} {
	set coloroptions "default black red blue yellow green cyan white magenta"
	if {[lsearch $coloroptions $value] == -1} {
	    UTF::Message ERROR $utf_msgtag "$option of $value not valid.  Possible values are: $coloroptions \n(Note: capitalize first letter for bold, e.g. \"red\" for normal and \"Red\" for bold)"
	    error "msgcolor"
	}
    }
    method __validatesta {option value} {
	if {$option eq "-transmitsta" && [llength $value] > 1} {
	    set msg "transmitsta takes single value"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	} else {
	    $self __validatestas $option $value
	}
    }
    method __validatestas {option value} {
	foreach sta $value {
	    if {[catch {$sta info type} stype] || $stype ne "::UTF::STA"} {
		set msg "$sta is not a STA object"
		UTF::Message ERROR $utf_msgtag $msg
		error $msg
	    }
	}
    }
    method __validatetraffictype {option value} {
	set enumerated_types "HDVIDEO STDVIDEO VIDEO G711 VOICE VOIP BESTEFFORT BESTEFFORT_SMALL BESTEFFORT_LARGE LATENCY BACKGROUND BK_SMALL BK_LARGE"
	set value [string toupper $value]
	if {[lsearch $enumerated_types $value] == -1} {
	    set msg "$option $value not valid, possible values are $enumerated_types"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
    }
    method __compoundconfigure {option value} {
	switch -exact -- $option {
	    "-rate" {
		set tmp [rate2numeric $value]
		# Use dummy just to test the range as valid
		set dummy [$self __numeric2rate $tmp]
		set options(-pps) [expr {round ($tmp / (8.0 * $options(-pktsize)))}]
		set options(-holdppsconstant) 0
	    }
	    "-pps" {
		set tmp [expr {8 * $options(-pktsize) * $value}]
		set options(-rate) [$self __numeric2rate $tmp]
		set options(-holdppsconstant) 1
	    }
	    "-pktsize" {
		# If the pps is to be held constant, adjust the rate
		if {$options(-holdppsconstant)} {
		    set tmp [expr {8 * $options(-pps) * $value}]
		    set options(-rate) [$self __numeric2rate $tmp]
		} else {
		    # otherwise just update the pps calculation
		    set tmp [rate2numeric $options(-rate)]
		    set options(-pps) [expr {round ($tmp / (8.0 * $value))}]
		}
	    }
	    "-tos" {
		switch -exact $value {
		    "0x20" {
			set options(-traffictype) "BK"
		    }
		    "0x0" -
		    "0x00" {
			set options(-traffictype) "BEST"
		    }
		    "0x80" {
			set options(-traffictype) "VI"
		    }
		    "0xC0" -
		    "0xc0" {
			set options(-traffictype) "VO"
		    }
		}
	    }
	}
    }
    method __configureoptionwithapply {option value} {
	if {$options($option) eq $value} {
	    return
	}
	$self __compoundconfigure $option $value
	set options($option) $value
	if {$txstate eq "ON" && $rxstate eq "ON"} {
	    $self stop; $self start
	} elseif {$rxstate eq "ON"} {
	    $self stop; $self rxstart
	} elseif {$txstate eq "ON"} {
	    $self stop; $self txstart
	}
    }
    method __configurename {option value} {
	set options(-name) $value
	set utf_txmsgtag "${options(-name)}-tx"
	set utf_rxmsgtag "${options(-name)}-rx"
	set utf_msgtag "$options(-name)"
    }
    method __configureprotocol {option value} {
	$self __configureoptionwithapply $option [string toupper $value]
    }
    method __configuremsgcolor {option value} {
	if {[string toupper $value] eq "DEFAULT"} {
	    set options(-msgcolor) {}
	} else {
	    set options(-msgcolor) $value
	}
    }
    method __configuretraffictype {option value} {
	set value [string toupper $value]
	set options($option) $value
	switch -exact $value {
	    "HDVIDEO" {
		set options(-tos) 0x80
		set options(-pktsize) 1470
		set options(-rate) 15M
	    }
	    "STDVIDEO" {
		set options(-tos) 0x80
		set options(-pktsize) 1470
		set options(-rate) 5M
	    }
	    "VIDEO" {
		set options(-tos) 0x80
		set options(-pktsize) 1470
		set options(-rate) 5M
	    }
	    "VOICE" {
		set options(-tos) 0xc0
		set options(-pktsize) 200
		set options(-rate) 8K
	    }
	    "G711" {
		set options(-tos) 0xc0
		# RTP 12 bytes + two 80 byte G711 samples
		set options(-pktsize) 172
		set options(-rate) 70K
	    }
	    "VOIP" {
		set options(-tos) 0xc0
		set options(-pktsize) 200
		set options(-rate) 8K
	    }
	    "BESTEFFORT_SMALL" {
		set options(-tos) 0x0
		set options(-pktsize) 4
		set options(-rate) 1M
	    }
	    "BESTEFFORT" {
		set options(-tos) 0x0
		set options(-pktsize) 512
		set options(-rate) 1M
	    }
	    "BK_LARGE" {
		set options(-tos) 0x20
		set options(-pktsize) 1470
		set options(-rate) 1M
	    }
	    "BK_SMALL" {
		set options(-tos) 0x20
		set options(-pktsize) 64
		set options(-rate) 1M
	    }
	    "BACKGROUND" {
		set options(-tos) 0x20
		set options(-pktsize) 512
		set options(-rate) 1M
	    }
	    "BESTEFFORT_LARGE" {
		set options(-tos) 0x0
		set options(-pktsize) 1470
		set options(-rate) 1M
	    }
	    "LATENCY" {
		set options(-tos) 0x0
		set options(-pktsize) 1470
		set options(-rate) 10M
	    }
	    default {
		UTF::Message ERROR $utf_msgtag "Unknown traffic type $value"
	    }
	}
    }
    method __lanpeerselect {dut} {
	set lanpeers [$dut cget -lanpeer]
	if {$lanpeers eq ""} {
	    error "-lanpeer needs to be set for $dut"
	}
	# Install full host routes if any of the lanpeers are on the same host
	set cnt [llength $lanpeers]
	set unique [lsort -unique $lanpeers]
	if {$cnt > 1 && [expr {$cnt ne $unique}] && $options(-installroutes) eq "-1"} {
	    set options(-installroutes) 1
	}
	if {![info exists LANPEERIX($dut)]} {
	    set LANPEERIX($dut) 0
	} else {
	    incr LANPEERIX($dut)
	    set LANPEERIX($dut) [expr {$LANPEERIX($dut) % [llength $lanpeers]}]
	}
	return [lindex $lanpeers $LANPEERIX($dut)]
    }
    
    #
    #  Configure point of attachments
    #
    method __configurepoas {option value} {
	if {$options($option) eq $value} {
	    return
	}
	set options($option) $value
	switch -exact -- $option {
	    "-tx" {
		if {[$value cget -lanpeer] ne ""} {
		    set value [$self __lanpeerselect $value]
		}
		set options(-transmitsta) $value
		set lan2sta($options(-transmitsta)) $value
		set setupneeded($options(-transmitsta)) 1
	    }
	    "-rx" {
		set options(-receivesta) ""
		foreach endpoint $value {
		    if {[$endpoint cget -lanpeer] ne ""} {
			set endpoint [$self __lanpeerselect $endpoint]
		    }
		    lappend options(-receivesta) $endpoint
		    set lan2sta($endpoint) $endpoint
		    set setupneeded($endpoint) 1
		}
		if {[llength $options(-receivesta)] > 1} {
		    set options(-multicast) 1
		    set options(-protocol) "UDP"
		}
	    }
	    "-transmitsta" {
		set lan2sta($value) $value
		set setupneeded($options(-transmitsta)) 1
	    }
	    "-receivesta" {
		foreach sta $value {
		    set setupneeded($sta) 1
		    set lan2sta($sta) $sta
		}
		if {[llength $options(-receivesta)] > 1} {
		    set options(-multicast) 1
		    set options(-protocol) "UDP"
		}
	    }
	}
	if {$options(-transmitsta) eq $options(-receivesta)} {
	    UTF::Message WARN $utf_msgtag "stream using the same end point for tx and rx"
	}
	if {$txstate eq "ON" && $rxstate eq "ON"} {
	    $self stop; $self stats -clear; $self start
	    UTF::Message INFO $utf_msgtag "stats cleared due to endpoint change"
	} elseif {$rxstate eq "ON"} {
	    $self stop; $self stats -clear; $self rxstart
	    UTF::Message INFO $utf_msgtag "stats cleared due to endpoint change"
	} elseif {$txstate eq "ON"} {
	    $self stop; $self stats -clear; $self txstart
	    UTF::Message INFO $utf_msgtag "stats cleared due to endpoint change"
	}
    }
    method whatami {} {
	return "UTF::stream"
    }
    method tos2txt {} {
	switch -exact $options(-tos) {
	    "0x20" {
		set options(-traffictype) "BK"
	    }
	    "0x0" -
	    "0x00" {
		set options(-traffictype) "BE"
	    }
	    "0x80" {
		set options(-traffictype) "VI"
	    }
	    "0xC0" -
	    "0xc0" {
		set options(-traffictype) "VO"
	    }
	}
    }
    method label {} {
	if {$options(-name) ne ""} {
	    set sname $options(-name)
	} else {
	    set sname [namespace tail $self]
	}
	if {$options(-protocol) eq "UDP"} {
	    set label "udp[$self tos2txt]($options(-pktsize)B)"
	} else {
	    set label "tcp[$self tos2txt]"
	    if {$options(-rwin) eq "-1"} {
		append label "($DEFAULTRWIN)"
	    } else {
		append label "(${options(-rwin)})"
	    }
	}
	if {$options(-rate) eq "-1"} {
	    if {$options(-protocol) eq "UDP"} {
		append label "-1M"
	    }
	} else {
	    append label "-${options(-rate)}"
	}
	if {[string tolower $options(-protocol)] ne [string tolower $options(-name)]} {
	    append label ":$sname"
	}
	return "$label"
    }
    method linkcheck {args} {
	UTF::Getopts {
	    {now "Check for traffic at this moment in time"}
	    {strict "Fail check if there is any packet loss"}
	    {loose "Ignore transmitter"}
	    {txstrict "Fail check if the offered varies from the actual"}
	}
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_msgtag "linkcheck invoked"
	}
	if {$txstate ne "ON" && [array names linkcheck_flags] eq {}} {
	    set msg "Stream never started and linkcheck called"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	if {$(now)} {
	    $self linkcheck_reset
	}
	# Guarantee at least 1/2 second or -linkcheckfailcount report intervals before failing
	set already [expr {[UTF::stream clock] - $linkcheck_flags(start)}]
	set duration [expr {$options(-linkcheckfailcount) * $options(-reportinterval)}]
	if {$duration < 0.5} {
	    set duration 0.5
	}
	set waits [expr {$duration  - $already}]
	if {$waits > 0} {
	    set linkcheck_flags(wd) 0
	    set aid [after [expr {round($waits * 1000)}] [list set [myvar linkcheck_flags(wd)] 1]]
	}
	set stalist $options(-receivesta)
	set lossy ""
	# Look at transmitter if protocol is UDP
	if {$options(-protocol) eq "UDP" && !$(loose)} {
	    set stalist [concat $options(-transmitsta) $stalist]
	}
	while {1} {
	    if {[info exists aid]} {
		vwait [myvar linkcheck_flags]
	    }
	    # Check all handlers
	    foreach device $stalist {
		if {[catch {set linkcheck_flags($device)} x] || !$x} {
		    continue
		} else {
		    set stalist [lsearch -all -not -inline $stalist $device]
		}
	    }
	    #  Break the loop
	    #   1) if the stream is in or transitions to off state
	    #   2) all the handlers have triggered their event flag
	    #   3) the watchdog isn't set meaning linkcheck time has expired
	    if {$txstate ne "ON" || ![llength $stalist] || [catch {after info $aid}]} {
		break
	    }
	}
	# Check for strict options on UDP traffic
	if {$options(-protocol) eq "UDP"} {
	    if {$(strict)} {
		set lost [$self stats -lost]
		if {!$options(-multicast)} {
		    if {$(now)} {
			set m [lsearch -all -not -inline -start end $lost 0]
			if {$m ne "" } {
			    lappend lossy "${device}"
			}
		    } else {
			set m [lsearch -all -not -inline  $lost 0]
			if {$m ne ""} {
			    lappend lossy "${device},[llength $m]/[llength $lost]"
			}
		    }
		} else {
		    foreach rxvalue $lost {
			set dut [lindex $rxvalue 0]
			set l [lindex $rxvalue 1]
			if {$(now)} {
			    set m [lsearch -all -not -inline -start end $l 0]
			    if {$m ne ""} {
				lappend lossy "${dut}"
			    }
			} else {
			    set m [lsearch -all -not -inline  $l 0]
			    if {$m ne ""} {
				lappend lossy "${dut},[llength $m]/[llength $l]"
			    }
			}
		    }
		}
	    } elseif {$(txstrict)} {
		if {!$options(-multicast)} {
		    set oa [$self stats -offeredactual]
		    if {!$(now)} {
			set oa [lsort -increasing $oa]
		    }
		    if {[lindex $oa end] < 90 } {
			lappend lossy "${device}"
		    }
		}
	    }
	}
	catch {after cancel $aid}
	set errmsg ""
	if {[llength $stalist]} {
	    set errmsg "Links down=($stalist) "
	}
	if {[llength $lossy]} {
	    append errmsg "Lossy=($lossy)"
	}
	if {$errmsg ne ""} {
	    if {$options(-api_msgs)} {
		UTF::Message API-E $utf_msgtag "linkcheck FAILED, $errmsg"
	    }
	    error "Linkcheck fail"
	} elseif {$options(-api_msgs)} {
	    UTF::Message API-E $utf_msgtag "linkcheck PASSED"
	}
    }
    method samplers {args} {
	UTF::Getopts {
	    {disable ""}
	    {enable ""}
	    {status ""}
	}
	if {$(enable)} {
	    if {!$samplersenabled} {
		$self linkcheck_reset
		set samplersenabled 1
	    }
	} elseif {$(disable)} {
	    set samplersenabled 0
	}
	return $samplersenabled
    }
    method __startopts {args} {
	UTF::Getopts {
	    {t.arg "" "time in seconds to run stream"}
	    {w.arg "" "socket buffer size override"}
	    {l.arg "" "packet size override"}
	    {b.arg "" "bps override"}
	}
	if {$(t) ne ""} {
	    set iperfseconds [expr {int($(t))}]
	} else {
	    set iperfseconds $options(-iperfseconds)
	}
	if {$(w) ne ""} {
	    set reqwinsize $(w)
	} else {
	    set reqwinsize $options(-w)
	}
	if {$(b) ne ""} {
	    set reqrate $(b)
	} else {
	    set reqrate $options(-rate)
	}
	if {$(l) ne ""} {
	    set reqpktsize [UTF::kexpand $(l)]
	} else {
	    set reqpktsize $options(-pktsize)
	}
    }
    method start {args} {
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_msgtag "stream START invoked : rxstate = $rxstate"
	}
	if {$options(-transmitsta) eq {}} {
	    set msg "Transmitter not configured"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	if {!$options(-multicast) && ($options(-transmitsta) eq $options(-receivesta))} {
	    UTF::Message WARN $utf_msgtag "TX=RX=$options(-receivesta)"
	}
	if {[catch {$options(-transmitsta) ipaddr} SRC]} {
	    UTF::Message WARN $utf_rxmsgtag "[$options(-tx) lan] No local src ip address available"
	    unset SRC
	}
	if {[catch {$options(-receivesta) ipaddr} DST]} {
	    UTF::Message WARN $utf_txmsgtag "[$options(-rx) lan] No remote dst ip address available"
	    unset DST
	}
	if {!$options(-multicast) && [info exists SRC] && [info exists DST] && ($DST eq $SRC)} {
	    UTF::Message WARN $utf_msgtag "src ip=dest ip=$SRC"
	}
	set handlerstate(cmd) "STARTING"
	eval [concat $self __startopts $args]
	if {$doonetimesetup} {
	    $self __doonetimesetup
	    set doonetimesetup 0
	}
	# Check for pending exceptions and try to clear them
	foreach device [array names exceptions] {
	    catch {$self __iperf_kill_remote_pid -name $device}
	}
	if {[array get exceptions] ne ""} {
	    error "Exceptions persist on [array names exceptions]"
	}
	# Synchronize the distributed clocks
	if {$options(-clockip) ne ""} {
	    foreach sta [concat $options(-transmitsta) $options(-receivesta)] {
		if {[$sta cget -lan_ip] ne $options(-clockip)} {
		    if {![info exists NTPLOCK($sta)]} {
			set NTPLOCK($sta) 1
			set cmd [concat $sta rexec ntpdate -b -p1 $options(-clockip)]
			if {[catch {eval $cmd} err]} {
			    UTF::Message WARN $sta $err
			}
			unset NTPLOCK($sta)
		    }
		}
	    }
	}
	#
	#  Start order matters, if unicast start the server (receiver first)
	#  and if multicast start the client (transmitter first.)
	#
	if {$options(-multicast)} {
	    $self txstart
	    $self rxstart
	} else {
	    if {$options(-receivesta) eq {}} {
		set msg "Unicast receiver not configured"
		UTF::Message ERROR $utf_msgtag $msg
		error $msg
	    }
	    if {![catch {$self rxstart} err]} {
		if {[catch {$self txstart} err]} {
		    if {$options(-rx) ne $options(-receivesta)} {
			set rxmsg "RX:($options(-rx), $options(-receivesta))"
		    } else {
			set rxmsg "RX:($options(-rx))"
		    }
		    if {$options(-tx) ne $options(-transmitsta)} {
			set txmsg "TX:($options(-tx), $options(-transmitsta))"
		    } else {
			set txmsg "TX:($options(-tx))"
		    }
		    set msg "Transmit start fail:$utf_msgtag $txmsg $rxmsg, stop invoked"
		    UTF::Message ERROR $utf_txmsgtag $msg
		    catch {$self stop}
		    error $msg
		}
	    } else {
		set msg "rx start failed : $err"
		UTF::Message ERROR $utf_rxmsgtag $msg
		error $msg
	    }
	}
	set samplersenabled 1
	set restart_pending 0
	if {$options(-latency) && ![info exists latency(afterid)]} {
	    $self __samplelatency
	}
	if {$options(-api_msgs)} {
	    UTF::Message API-E $utf_msgtag "stream start DONE : txstate = $txstate rxstate = $rxstate"
	}
    }
    method show_txshaper {} {
	catch {eval [concat $options(-transmitsta) rexec $TCCMD -s qdisc show dev [$options(-transmitsta) cget -device]]} output
	catch {eval [concat $options(-transmitsta) rexec $TCCMD -s class show dev [$options(-transmitsta) cget -device]]} output
	catch {eval [concat $options(-transmitsta) rexec $TCCMD -s filter show dev [$options(-transmitsta) cget -device]]} output
	catch {eval [concat $options(-transmitsta) rexec $IPTABLESCMD -t mangle -L]} output
    }
    method clear_shapers {} {
	if {![$options(-transmitsta) hostis Linux]} {
	    error "traffic shaping unsupported on this host"
	}
	foreach s [concat $options(-receivesta) $options(-transmitsta)] {
	    set host $s
	    set dev [$s cget -device]
	    catch {eval [concat $host rexec $TCCMD qdisc del dev $dev root]}
	    catch {eval [concat $host rexec $IPTABLESCMD -F -t mangle]}
	    catch {unset TC($host,$dev)}
	    set ip [$options(-transmitsta) ipaddr]
	    catch {unset iptablescache($host,$dev,$ip,TCP,$options(-dstport))}
	    catch {unset iptablescache($host,$dev,$ip,UDP,$options(-dstport))}
	    foreach rx $options(-receivesta) {
		set ip [$rx ipaddr]
		catch {unset iptablescache($host,$dev,$ip,TCP,$options(-dstport))}
		catch {unset iptablescache($host,$dev,$ip,UDP,$options(-dstport))}
	    }
	}
    }
    method update_shaper_rate {r} {
	set r [UTF::stream hexpand $r]
	if {$r > 1000000} {
	    set srate "[expr {$r / 1000000}]mbit"
	} else {
	    set srate "[expr {$r / 1000}]kbit"
	}
	if {$options(-shaperceil) ne ""} {
	    set r [UTF::stream hexpand $options(-shaperceil)]
	}
	if {$r > 1000000} {
	    set sceil "[expr {$r / 1000000}]mbit"
	} else {
	    set sceil "[expr {$r / 1000}]kbit"
	}
	set host $options(-transmitsta)
	set dev [$options(-transmitsta) cget -device]
	eval [concat $host rexec $TCCMD class replace dev $dev parent $TC($host,$dev):1 classid $TC($host,$dev):$tc_minorid htb rate $srate ceil $sceil quantum $options(-quantum)]
    }
    # Useful debug tools:
    # watch -n 1  /sbin/tc -s -d class show dev $DEV
    # watch -n 1 -d /sbin/iptables -t mangle -nvL
    # iptraf or iptraf-ng
    # If using vlan interfaces make sure to set
    # txqueuelen to nonzero value, e.g. ifconfig p21p1.1000 txqueuelen 1000
    method install_txshaper {} {
	if {![$options(-transmitsta) hostis Linux]} {
	    error "traffic shaping unsupported on this host"
	}
	set host $options(-transmitsta)
	set dev [$options(-transmitsta) cget -device]
	UTF::stream init_txshaper $host $dev
	set r [UTF::stream hexpand $options(-shaperrate)]
	if {$r > 1000000} {
	    set srate "[expr {$r / 1000000}]mbit"
	} else {
	    set srate "[expr {$r / 1000}]kbit"
	}
	if {$options(-shaperceil) ne ""} {
	    set r [UTF::stream hexpand $options(-shaperceil)]
	}
	if {$r > 1000000} {
	    set sceil "[expr {$r / 1000000}]mbit"
	} else {
	    set sceil "[expr {$r / 1000}]kbit"
	}
	set quantum $options(-quantum)
	eval [concat $host rexec $TCCMD class replace dev $dev parent $TC($host,$dev): classid $TC($host,$dev):1 htb rate 1000mbit quantum $quantum]
	eval [concat $host rexec $TCCMD class replace dev $dev parent $TC($host,$dev):1 classid $TC($host,$dev):$tc_minorid htb rate $srate ceil $sceil quantum $quantum]
	set srcip [$options(-transmitsta) ipaddr]
	if {![info exists iptablescache($host,$dev,$srcip,$options(-protocol),$options(-dstport))]} {
	    set iptablescache($host,$dev,$srcip,$options(-protocol),$options(-dstport)) [UTF::stream get_next_flowid]
	    eval [list $host rexec $IPTABLESCMD -A OUTPUT -t mangle -p $options(-protocol) -s $srcip --dport $options(-dstport) -j MARK --set-mark $iptablescache($host,$dev,$srcip,$options(-protocol),$options(-dstport)) -m comment --comment \"$STREAMSIPTABLESSTRING\"]
	}
	eval [concat $options(-transmitsta) rexec $TCCMD filter replace dev [$options(-transmitsta) cget -device] protocol ip parent $TC($host,$dev): handle $iptablescache($host,$dev,$srcip,$options(-protocol),$options(-dstport)) fw flowid $TC($host,$dev):$tc_minorid]
    }
    method mydstip {} {
	if {$options(-multicast)} {
	    set dstip $options(-dstgrpip)
	} elseif {$options(-dstip) ne ""} {
	    set dstip $options(-dstip)
	} else {
	    set dstip [$options(-receivesta) ipaddr]
	}
	if {$dstip ne {} && [::ip::IPv4? $dstip]} {
	    return $dstip
	} else {
	    set msg "IP address error for $options(-receivesta)"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
    }

    method txstart {args} {
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_txmsgtag "txstart called : txstate = $txstate"
	}
	if {$doonetimesetup} {
	    $self __doonetimesetup
	    set doonetimesetup 0
	}
	if {$txstate ne "ON"} {
	    if {$options(-protocol) eq "TCP" && $options(-tcpdebug)} {
		$self sniff on -ack
	    }
	    if {[regexp {(\d+)} $options(-rate) - requested_rate]} {
		if {$requested_rate <= 0} {
		    set msg "tx start called with invalid rate: $options(-rate)"
		    UTF::Message ERROR $utf_msgtag $msg
		    error $msg
		}
	    } else {
		set msg "tx start called with unknown rate: $options(-rate)"
		UTF::Message ERROR $utf_msgtag $msg
		error $msg
	    }
	    if {$args ne {} || ![info exists reqwinsize]} {
		eval [concat $self __startopts $args]
	    }
	    $self linkcheck_reset
	    set dstip [$self mydstip]
	    if {$options(-shaperrate) ne ""} {
		$self install_txshaper
	    }
	    $self __iptables_install
	    if {$options(-protocol) eq "UDP" && $options(-fwlatency)} {
		set cmdopts "-E "
	    } else {
		set cmdopts "-e "
	    }
	    if {$reqwinsize ne "default" && $options(-w) ne "default"} {
		if {$reqwinsize ne "-1"} {
		    append cmdopts "-w $reqwinsize "
		} elseif {$options(-w) ne "-1"} {
		    append cmdopts "-w $options(-w) "
		} elseif {$options(-protocol) eq "UDP"} {
		    append cmdopts "-w [expr {$options(-udptx_winpktcnt) * $options(-pktsize)}] "
		}
	    }
	    if {$options(-protocol) eq "UDP"} {
		append cmdopts "-l $reqpktsize -u -b [rate2numeric $reqrate] --realtime "
	    }
	    if {$options(-bind)} {
		if {!([catch {$options(-transmitsta) ipaddr} myip]) && [::ip::IPv4? $myip]} {
		    append cmdopts "-B $myip "
		} else {
		    set msg "$options(-transmitsta) No ip bind address"
		    UTF::Message ERROR $utf_msgtag $msg
		    error $msg
		}
	    }
	    if {[$options(-transmitsta) hostis Cygwin WinDHD WinBT]} {
		append cmdopts "-R "
	    }
	    if {$options(-tcptune) && $reqwinsize ne "default"} {
		if {$reqwinsize eq "-1"} {
		    $options(-transmitsta) tcptune 0
		} else {
		    $options(-transmitsta) tcptune $reqwinsize
		}
	    }
	    if {$options(-protocol) eq "TCP"  && $options(-rate) ne "-1"} {
		append cmdopts "-b $options(-rate) "
	    }
	    if {[catch {set iperfcmd [$options(-transmitsta) cget -iperf]}]} {
		set msg "No -iperf attribute found for $options(-transmitsta)"
		UTF::Message ERROR $utf_txmsgtag $msg
		catch {$self stop}
		error $msg
	    } elseif {$iperfcmd eq ""} {
		set msg "-iperf attribute not set for $options(-transmitsta)"
		UTF::Message ERROR $utf_txmsgtag $msg
		catch {$self stop}
		error $msg
	    }
	    set cmd "$iperfcmd -c $dstip $cmdopts -i $options(-reportinterval) -f$options(-format) -S $options(-tos) -T $options(-ttl) -t $iperfseconds -p $options(-dstport)"
	    if {$options(-nice) && [$options(-transmitsta) hostis Cygwin Linux]} {
		set cmd "nice $cmd"
	    }
	    if {$options(-api_msgs)} {
		UTF::Message INFO $utf_txmsgtag "calling rpopen: $cmd"
	    }
	    if {[catch {$options(-transmitsta) rpopen -noinit "$cmd"} fid]} {
		$self __dorestore $options(-transmitsta)
		set msg "txstart: ropen fail : $fid"
		UTF::Message ERROR $utf_txmsgtag $msg
		error $msg
	    }
	    set iperfcmds($fid) $cmd
	    fconfigure $fid -blocking 1 -buffering line
	    set fid2device($fid) $options(-transmitsta)
	    set handlerstate($fid) "INIT"
	    set iperfcmds($fid,hdr) 0
	    set noreentrancy($fid) 0
	    fileevent $fid readable [mymethod __iperf_txhandler $fid]
	    if {$options(-debug_msgs)} {
		UTF::Message DEBUG $utf_txmsgtag "rpopen read handler ($fid2device($fid),$fid) : [fileevent $fid readable]"
	    }
	    set handlerstate(wd) "PENDING"
	    if {!$options(-noblock)} {
		set watchdog [after $options(-txstart_timeout) [list set [myvar handlerstate(wd)] "TIMEOUT"]]
		while {$handlerstate($fid) ne "READY"} {
		    if {$options(-api_msgs)} {
			UTF::Message VWAIT $utf_txmsgtag "vwait on READY : state is $handlerstate($fid) "
		    }
		    vwait [myvar handlerstate]
		    if {$handlerstate(wd) ne "TIMEOUT"} {
			continue
		    } else {
			break
		    }
		}
		if {$handlerstate(wd) eq "TIMEOUT"} {
		    set msg "iperf tx start timeout"
		    UTF::Message ERROR $utf_txmsgtag $msg
		    if {$options(-protocol) eq "TCP" && $options(-tcpdebug)} {
			foreach end "$options(-transmitsta) $options(-receivesta)" {
			    if {[$end hostis MacOS]} {
				$end ifconfig
				$end rexec netstat -t -n -v -f inet
			    } else {
				$end ifconfig
				$end rexec netstat -t -n -v -a | grep [$end ipaddr]
				$end rexec iptables -L
			    }
			}
		    }
		    catch {$self __close_handler_actions $fid}
		    if {$options(-protocol) eq "TCP" && $options(-tcpdebug)} {
			$self sniff off
		    }
		    error $msg
		} else {
		    if {$options(-api_msgs)} {
			UTF::Message VWAIT $utf_txmsgtag "vwait DONE : state is $handlerstate($fid) "
		    }
		    after cancel $watchdog
		}
		if {$options(-protocol) eq "TCP" && $options(-tcpdebug)} {
		    $self sniff off
		}

	    } else {
		UTF::Sleep 0 quiet
	    }
	}
	if {$options(-api_msgs)} {
	    UTF::Message API-E $utf_txmsgtag "txstart done : txstate = $txstate"
	}
	if {$options(-protocol) eq "UDP"} {
	    set tmp [rate2numeric $options(-rate)]
	    set msg "protocol=UDP Offered rate=$options(-rate) pktsize=$options(-pktsize) delay=[expr {int($options(-pktsize) * 8 * 1000000.0 / $tmp)}]us"
	    UTF::Message INFO $utf_txmsgtag "TX start: $msg"
	} elseif {$options(-tcptrace)} {
	    $self tcptrace start
	}
    }
    method rxstart {args} {
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_rxmsgtag "rxstart called : rxstate = $rxstate"
	}
	if {$rxstate ne "ON"} {
	    if {$args ne {} || ![info exists reqwinsize]} {
		eval [concat $self __startopts $args]
	    }
	    if {!$options(-multicast) && [llength $options(-receivesta)] > 1} {
		set rxs [lindex $options(-receivesta) 0]
		UTF::Message WARN $utf_msgtag "More than one receiver configured, first  of $rxs will be used."
	    } else {
		set rxs $options(-receivesta)
	    }
	    if {[llength $rxs] == 0} {
		if {!$options(-multicast)} {
		    set msg "No receivers configured and rxstart called."
		    UTF::Message ERROR $utf_msgtag $msg
		    error $msg
		} else {
		    UTF::Message INFO $utf_msgtag "multicast src only"
		    return
		}
	    }
	    set fidlist ""
	    foreach rx $rxs {
		if {$options(-protocol) eq "UDP" && $options(-pdfs)} {
		    set cmdopts "-E "
		} else {
		    set cmdopts "-e "
		}
		if {$options(-protocol) eq "TCP" && $options(-readdelay) ne "-1"} {
		    append cmdopts "-q $options(-readdelay) "
		}
		if {$options(-multicast)} {
		    append cmdopts "-B $options(-dstgrpip) "
		} else {
		    append cmdopts "-B [$rx ipaddr] "
		}
		if {[$rx hostis Cygwin WinDHD WinBT]} {
		    append cmdopts "-R "
		}
		if {$options(-w) ne "default" && $reqwinsize ne "default"} {
		    if {$reqwinsize ne "-1"} {
			append cmdopts "-w $reqwinsize "
		    } elseif {$options(-w) ne "-1"} {
			append cmdopts "-w $options(-w) "
		    } elseif {$options(-protocol) eq "UDP"} {
			append cmdopts "-w [expr {$options(-udprx_winpktcnt) * $options(-pktsize)}] "
		    }
		}
		if {$options(-protocol) eq "UDP"} {
		    append cmdopts "-l $reqpktsize -u --realtime "
		} elseif {$options(-rwin) eq "-1"} {
		    append cmdopts "-l [UTF::kexpand $DEFAULTRWIN]"
		}
		if {$options(-rwin) ne "-1"} {
		    append cmdopts "-l [UTF::kexpand $options(-rwin)]"
		}
		if {$options(-tcptune) && $reqwinsize ne "default"} {
		    if {$reqwinsize eq "-1"} {
			$rx tcptune 0
		    } else {
			$rx tcptune $reqwinsize
		    }
		}
		if {[catch {set iperfcmd [$rx cget -iperf]}]} {
		    set msg "No -iperf attribute found for $rx"
		    UTF::Message INFO $utf_rxmsgtag $msg
		    catch {$self stop}
		    error $msg
		} elseif {$iperfcmd eq ""} {
		    set msg "-iperf attribute not set for $rx"
		    UTF::Message INFO $utf_rxmsgtag $msg
		    catch {$self stop}
		    error $msg
		}
		set cmd "$iperfcmd -s $cmdopts -i $options(-reportinterval) -f$options(-format) -S $options(-tos) -p $options(-dstport)"
		if {$options(-nice) && [$rx hostis Cygwin Linux]} {
		    set cmd "nice $cmd"
		}
		if {$options(-api_msgs)} {
		    UTF::Message INFO $utf_rxmsgtag "calling rpopen: $cmd"
		}
		if {[catch {$rx rpopen -noinit "$cmd"} fid]} {
		    $self __dorestore $rx
		    set msg "rxstart $rx rpopen fail : $fid"
		    UTF::Message ERROR $utf_rxmsgtag $msg
		    error $msg
		}
		set iperfcmds($fid) "$cmd"
		lappend fidlist $fid
		fconfigure $fid -blocking 1 -buffering line
		set fid2device($fid) $rx
		set handlerstate($fid) "INIT"
		set iperfcmds($fid,hdr) 0
		set noreentrancy($fid) 0
		fileevent $fid readable [mymethod __iperf_rxhandler $fid]
		if {$options(-debug_msgs)} {
		    UTF::Message DEBUG $utf_rxmsgtag "rpopen read handler ($fid2device($fid),$fid) : [fileevent $fid readable]"
		}
	    }
	    #
	    #  Wait on all the iperf instances
	    #
	    set watchdog [after $FASTTIMEOUT [list set [myvar handlerstate(wd)] "TIMEOUT"]]
	    set handlerstate(wd) "PENDING"
	    set waiting [llength $fidlist]
	    while {$waiting} {
		if {$options(-api_msgs)} {
		    UTF::Message VWAIT $utf_rxmsgtag "vwait on READY : $fidlist"
		}
		vwait [myvar handlerstate]
		if {$handlerstate(wd) eq "TIMEOUT"} {
		    set waiting 0
		} else {
		    foreach fid $fidlist {
			if {$handlerstate($fid) eq "READY"} {
			    set fidlist [lsearch -all -not -inline $fidlist $fid]
			}
		    }
		    set waiting [llength $fidlist]
		}
	    }
	    if {$handlerstate(wd) eq "TIMEOUT"} {
		set msg "Time out waiting for rx iperf(s) ready"
		UTF::Message ERROR $utf_rxmsgtag $msg
		catch {$self __close_handler_actions $fid}
		if {!$options(-multicast)} {
		    error $msg
		}
	    } else {
		if {$options(-api_msgs)} {
		    UTF::Message VWAIT $utf_rxmsgtag "vwait DONE"
		}
		after cancel $watchdog
		if {$options(-protocol) eq "TCP" && $options(-tcpdebug)} {
		    set end "$options(-receivesta)"
		    if {[$end hostis MacOS]} {
			$end rexec netstat -t -n -v -f inet
		    } else {
			$end rexec netstat -t -n -v -a | grep [$end ipaddr]
		    }
		}
	    }
	    if {![llength $fidlist]} {
		set rxstate "ON"
	    } else {
		set rxstate "ON-PARTIAL"
	    }
	}
	if {$options(-api_msgs)} {
	    UTF::Message API-E $utf_rxmsgtag "rxstart done : rxstate = $rxstate"
	}
    }
    # Special rx start for multicast
    # allows duplicate rx sockets from
    # the same device
    method __mcast_rxstart {sta} {
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_rxmsgtag "mcast rxstart called for $sta"
	}
	set mcastoption "-B $options(-dstgrpip)"
	set windowsize [expr {$options(-udprx_winpktcnt) *($options(-pktsize) + $L2HDRSIZE)}]
	set cmdopts "-e -t 180 -l $options(-pktsize) -u -w $windowsize "
	if {[$sta hostis Cygwin WinDHD WinBT]} {
	    append cmdopts "-R "
	}
	set cmd "[$sta cget -iperf] -s $cmdopts -i $options(-reportinterval) -f$options(-format) -t $iperfseconds -p $options(-dstport) $mcastoption"
	if {$options(-nice) && [$sta hostis Cygwin Linux]} {
	    set cmd "nice $cmd"
	}
	if {$options(-api_msgs)} {
	    UTF::Message INFO $utf_rxmsgtag "calling rpopen: $cmd"
	}
	if {[catch {$sta rpopen -noinit "$cmd"} fid]} {
	    set msg "rxstart $sta rpopen fail : $fid"
	    UTF::Message ERROR $utf_rxmsgtag $msg
	    error $msg
	}
	set iperfcmds($fid) "$cmd"
	lappend fidlist $fid
	fconfigure $fid -blocking 1 -buffering line
	set fid2device($fid) $sta
	set handlerstate($fid) "INIT"
	set iperfcmds($fid,hdr) 0
	set noreentrancy($fid) 0
	fileevent $fid readable [mymethod __iperf_rxhandler $fid]
	if {$options(-debug_msgs)} {
	    UTF::Message DEBUG $utf_rxmsgtag "rpopen read handler ($fid2device($fid),$fid) : [fileevent $fid readable]"
	}
	#
	#  Wait the iperf instance
	#
	set watchdog [after $FASTTIMEOUT [list set [myvar handlerstate(wd)] "TIMEOUT"]]
	set handlerstate(wd) "PENDING"
	if {$options(-api_msgs)} {
	    UTF::Message VWAIT $utf_rxmsgtag "vwait on READY : $fid"
	}
	while {1} {
	    vwait [myvar handlerstate]
	    if {$handlerstate(wd) eq "TIMEOUT"} {
		set msg "Time out waiting for rx iperf(s) ready"
		UTF::Message ERROR $utf_rxmsgtag $msg
		catch {$self __close_handler_actions $fid}
		break
	    } elseif {$handlerstate($fid) eq "READY"} {
		if {$options(-api_msgs)} {
		    UTF::Message VWAIT $utf_rxmsgtag "vwait DONE"
		}
		after cancel $watchdog
		break
	    }
	}
    }
    # Special rx leave for multicast
    method __mcast_rxleave {sta} {
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_rxmsgtag "mcast leave called for $sta"
	}
	# lookup sta from fid
	set found ""
	foreach fid [array names fid2device] {
	    if {$fid2device($fid) eq $sta} {
		set found $fid
		break
	    }
	}
	if {$found eq ""} {
	    set msg "STA $sta not found in join list"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	set handlerstate(wd) "PENDING"
	set handlerstate($found) "PENDING"
	set watchdog [after $FASTTIMEOUT [list set [myvar handlerstate(wd)] "TIMEOUT"]]
	#
	#  Send a SIGHUP
	#
	if {![catch {exec kill -s HUP $fid2pid($found)} err]} {
	    UTF::Message INFO $utf_msgtag "Kill signal -HUP sent to ($fid2device($found),$found) : $iperfcmds($found)"
	} else {
	    UTF::Message ERROR $utf_msgtag "Kill -s HUP fail : $err ($fid2pid($found),$found)"
	}
	while {$handlerstate($found) ne "CLOSED"} {
	    if {$options(-api_msgs)} {
		UTF::Message VWAIT $utf_msgtag "vwait on CLOSE: $found"
	    }
	    vwait [myvar handlerstate]
	    if {$handlerstate(wd) eq "TIMEOUT"} {
		UTF::Message WARN $utf_msgtag "Time out waiting for process kills: $found"
		set handlerstate($found) "CLOSING-WERROR"
		fileevent $found readable {}
		UTF::Message WARN $utf_msgtag "Attempting remote kill for $fid2device($found),$found"
		if {[catch {$self __iperf_kill_remote_pid -fid $found} err]} {
		    UTF::Message ERROR $utf_msgtag "Remote kill FAILED $fid2device($found),$found : $err"
		}
		UTF::Message INFO $utf_msgtag "Streams forcing closing of ($fid2device($found),$found)"
		if {[catch {$self __close_handler_actions $found} err]} {
		    UTF::Message ERROR $utf_msgtag "close handler FAILED ($fid2device($found),$found) : $err"
		}
		break
	    } elseif {$options(-api_msgs)} {
		UTF::Message VWAIT $utf_msgtag "vwait DONE"
		after cancel $watchdog
	    }
	}
	if {$options(-api_msgs)} {
	    UTF::Message API-E $utf_msgtag "leave for $sta DONE"
	}
    }
    method mjoin {args} {
	if {!$options(-multicast)} {
	    set msg "only can join a multicast stream"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	$self __validatestas none $args
	foreach sta $args {
	    if {[catch {$self __mcast_rxstart $sta} err]} {
		set msg "$sta $err"
		UTF::Message ERROR $utf_rxmsgtag $msg
		error $msg
	    } else {
		if {[lsearch $options(-receivesta) $sta] eq -1} {
		    lappend options(-receivesta) $sta
		}
	    }
	}
    }
    method mleave {args} {
	if {!$options(-multicast)} {
	    set msg "only can leave a multicast stream"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	$self __validatestas none $args
	foreach sta $args {
	    if {[catch {$self __mcast_rxleave $sta} err]} {
		set msg "$sta $err"
		UTF::Message ERROR $utf_rxmsgtag $msg
		error $msg
	    } else {
		set options(-receivesta) [lsearch -all -not -inline $options(-receivesta) $sta]
	    }
	}
    }
    method stop {args} {
	UTF::Getopts {
	    {sigint ""}
	}
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_msgtag "stream STOP invoked : txstate = $txstate rxstate = $rxstate"
	}
	#
	#  Kill any watch periodics first
	#
	foreach fid [array names fid2afterid] {
	    catch {after cancel $fid2afterid($fid)}
	}
	array unset fid2afterid *
	set handlerstate(cmd) "STOPPING"
	catch {$self __samplelatencystop}
	catch {$self tcptrace stop}
	#
	# It's cleaner to kill the transmitter before the receiver(s)
	#
	set stoplist ""
	if {[info exists rxfids]} {
	    set stoplist $rxfids
	}
	if {$(sigint)  && $txfids ne ""} {
	    set tmp $options(-txdisplay)
	    set options(-txdisplay) 1
	    set samplerstate $samplersenabled
	    set samplersenabled 0
	    UTF::Message INFO $utf_msgtag "Signal -INT sent to ($fid2device($txfids),$txfids) : $iperfcmds($txfids)"
	    set handlerstate(wd) "PENDING"
	    set watchdog [after $TXCLOSEACK [list set [myvar handlerstate(wd)] "TIMEOUT"]]
	    catch {exec kill -s INT $fid2pid($txfids)} err
	    vwait [myvar handlerstate]
	    set options(-txdisplay) $tmp
	    set samplersenabled $samplerstate
	}
	if {$txfids ne {}} {
	    set stoplist [concat $txfids $stoplist]
	}
	foreach fid $stoplist {
	    set handlerstate($fid) "CLOSING"
	    #
	    #  Send a SIGHUP to the client (tx) and SIGHUP to servers
	    #
	    if {![catch {exec kill -s HUP $fid2pid($fid)} err]} {
		UTF::Message INFO $utf_msgtag "Kill signal -HUP sent to ($fid2device($fid),$fid) : $iperfcmds($fid)"
	    } else {
		UTF::Message ERROR $utf_msgtag "Kill -s HUP fail : $err ($fid2pid($fid),$fid)"
	    }
	}
	if {[llength [array names fid2pid]]} {
	    set handlerstate(wd) "PENDING"
	    set watchdog [after $FASTTIMEOUT [list set [myvar handlerstate(wd)] "TIMEOUT"]]
	    while {[array names fid2pid] ne {}} {
		if {$options(-api_msgs)} {
		    UTF::Message VWAIT $utf_msgtag "vwait on CLOSE: [array names fid2pid]"
		}
		vwait [myvar handlerstate]
		if {$handlerstate(wd) eq "TIMEOUT"} {
		    UTF::Message WARN $utf_msgtag "Time out waiting for process kills: [array names fid2pid]"
		    foreach fid [array names fid2pid] {
			#
			# Some sort of error occurred, try to make
			# sure the iperf is killed.
			# Don't use the current fsh session as it
			# could be the problem. Unregister
			# the event handler so the states
			# are driven only in this context
			set handlerstate($fid) "CLOSING-WERROR"
			fileevent $fid readable {}
		    }
		    foreach fid [array names fid2pid] {
			UTF::Message WARN $utf_msgtag "Attempting remote kill for $fid2device($fid),$fid"
			if {[catch {$self __iperf_kill_remote_pid -fid $fid} err]} {
			    UTF::Message ERROR $utf_msgtag "Remote kill FAILED $fid2device($fid),$fid : $err"
			}
			UTF::Message INFO $utf_msgtag "Streams forcing closing of ($fid2device($fid),$fid)"
			if {[catch {$self __close_handler_actions $fid} err]} {
			    UTF::Message ERROR $utf_msgtag "close handler FAILED ($fid2device($fid),$fid) : $err"
			}
		    }
		    # Force the fid2pid arry to {} though this should have
		    # been done by the above loop and close_handler_actions
		    # If execution gets here it's a coding error.
		    if {[array names fid2pid] ne {}} {
			UTF::Message PERROR $utf_msgtag "array names should be null and is not"
			array set fid2pid {}
		    }
		} elseif {$options(-api_msgs)} {
		    UTF::Message VWAIT $utf_msgtag "vwait DONE"
		}
	    }
	    after cancel $watchdog
	}
	unset handlerstate(cmd)
	if {$options(-api_msgs)} {
	    UTF::Message API-E $utf_msgtag "stream stop DONE : txstate = $txstate rxstate = $rxstate"
	}
	if {[llength [UTF::stream info instances]]} {
	    foreach aggsampler [::UTF::StreamStatAggregate info instances] {
		$aggsampler stop
	    }
	}
    }
    method __restart {} {
	if {!$restart_pending} {
	    # RJM - need to check host health, iperf stale
	    # fsh, etc and watch for reentrancy.
	    if {[catch {$self start}]} {
		UTF::Message HNDLR $utf_msgtag "RESTART FAILED and now disabled"
	    }
	}
    }
    method {arp install} {} {
	if {$arpstate eq "STATIC"} {
	    $self arp delete
	}
	if {[catch {$options(-tx) lan arp -s [$options(-rx) lan ipaddr] [$options(-rx) lan macaddr]} res]} {
	    error $res
	} else {
	    set arpstate "STATIC"
	    return
	}
    }
    method {arp delete} {} {
	if {[catch {$options(-tx) lan arp -d [$options(-rx) lan ipaddr]} res]} {
	    UTF::Message WARN $utf_msgtag $res
	}
    }
    method clear {} {
	$self stats -clear
    }
    method stats {args} {
	UTF::GetKnownopts {
	    {meanminmax "mean minimum maximum format"}
	    {mma "modified moving average"}
	    {cclimits "return running control chart limits"}
	    {silent "no log messages"}
	    {sum "return the sum of the stats"}
	    {timestamps "append the timestamps list to the results"}
	    {ignore_clearsynch ""}
	    {seconds ""}
	    {units "Return a units label for the stat"}
	    {h "human readable format"}
	}
	set results ""
	set statsusage {<-help | -all | -bytes | -clear | -count | -cwnd | -jitter | -latency | -lost | -lostper | -offeredactual | -outoforder | -pdf  | -pktcount | -pktlatency | -per | -pps | -rate | -rateevents | -rtt | -starttime | -stoptime | -timestamps | -txbytes | -txpktcount | -txrate | -uptime > [-meanminmax | -mma | -cclimits | -seconds | -sum | -timestamps | -units]}
	if {![llength $args]} {
	    UTF::Message WARN $utf_msgtag "usage: stats $statsusage"
	    return
	}
	if {[lindex $args 0] eq "options"} {
	    if {$options(-protocol) eq "TCP"} {
		return [list "-all" "-bytes" "-clear" "-count" "-cwnd" "-rate" "-rateevents" "-rtt" "-starttime" "-stoptime" "-txbytes" "-uptime"]
	    } else {
		return [list "-all" "-bytes" "-clear" "-count" "-jitter" "-latency" "-lost" "-lostper" "-offeredactual" "-outoforder" "-per" "-pdf" "-pktcount" "-pktlatency" "-pps" "-rate" "-rateevents" "-starttime" "-stoptime" "-txbytes" "-txpktcount" "-txrate" "-uptime" "-units"]
	    }
	}
	set statargs $args
	set filterix [lsearch -exact $args "-include"]
	if {$filterix != -1} {
	    set statoption "timestamps"
	    set t1 [lindex $args [expr {$filterix + 1}]]
	    set t2 [lindex $args [expr {$filterix + 2}]]
	    if {$t1 ne ""  && [string is double $t1]} {
		if {$t2 eq ""  || ![string is double $t2]} {
		    set t2 [UTF::stream clock]
		    set args [lreplace $args $filterix [expr {$filterix + 1}]]
		} else {
		    set args [lreplace $args $filterix [expr {$filterix + 2}]]
		}
	    } else {
		error "usage: -include <time1> \[<time2>\]"
	    }
	    set ignore_clearsynch 1
	} else {
	    set statoption ""
	    if {!$(meanminmax)} {
		set meanminmax 0
	    } else {
		set meanminmax 1
		set statoption "utfmean"
	    }
	    if {!$(mma)} {
		set mmastats 0
	    } else {
		set mmastats 1
		set statoption "utfmma"
	    }
	    if {$(cclimits)} {
		set statoption "utfcclimits"
	    }
	    if {$(sum)} {
		set statoption "sum"
	    }
	    if {$(silent)} {
		set silent 1
	    }
	    if {$(seconds)} {
		set statoption "seconds"
	    }
	    if {$(timestamps)} {
		lappend statoption "timestamps"
	    }
	    if {$(ignore_clearsynch)} {
		set ignore_clearsynch 1
	    } else {
		set ignore_clearsynch 0
	    }
	    # RJM - fix the below for all modifiers
	    if {$mmastats && $meanminmax} {
		UTF::Message WARN $utf_msgtag "modifiers  -meanminmax and -mma are mutually exclusive"
		return
	    }
	}
	set option [lindex $args 0]
	switch -exact -- $option {
	    "-clear" {
		if {!$ignore_clearsynch && $options(-clearsynch) && $txstate eq "ON"} {
		    set clearstats_synch 0
		    set watchdog [after [expr {int($options(-reportinterval) * 2000)}] [list set [myvar clearstats_synch] "-1"]]
		    vwait [myvar clearstats_synch]
		    if {$clearstats_synch eq "-1"} {
			UTF::Message WARN $utf_msgtag "Clear on sample taken timeout occurred"
		    } else {
			after cancel $watchdog
		    }
		}
		array unset samples *
		array unset pdfToT *
		array unset pdfDHD *
		catch {unset pdfDHD}
		array unset pdfDHDd *
		array unset pdfFW1d *
		array unset pdfFW2d *
		array unset pdfFW3d *
		array unset pdfFW4d *
		array unset pdfFW5d *
		array unset timestamps *
		array unset runningstats *
		set starttime {}
		$self linkcheck_reset
	    }
	    "-all" {
		if {$meanminmax} {
		    foreach ix [array names samples] {
			if {[llength $samples($ix)] > 1}  {
			    catch {puts "samples($ix) = [UTF::MeanMinMax $samples($ix)]"}
			} else {
			    puts "samples($ix) = $samples($ix)"
			}
		    }
		} elseif {$mmastats} {
		    # need to write this
		} else {
		    parray samples
		    parray runningstats
		}
		set tx $options(-transmitsta)
		if {[info exists samples($tx,rate)]} {
		    puts "samples(count) = [llength $samples($tx,rate)]"
		}
	    }
	    "-cwnd" {
		if {$(units)} {
		    return "Kbytes"
		}
		set results [$self __getsamples cwnd $statoption]
	    }
	    "-jitter" {
		if {$(units)} {
		    return "Time (milliseconds)"
		}
		set results [$self __getsamples jitter $statoption]
	    }
	    "-latency" {
		if {$(units)} {
		    return "Time (s)"
		}
		set results [$self __getsamples latency $statoption]
	    }
	    "-pktlatency" {
		if {$(units)} {
		    return "Time (s)"
		}
		set results [$self __getsamples pktlatency $statoption]
	    }
	    "-lost" {
		if {$(units)} {
		    return "Packets"
		}
		set results [$self __getsamples lost $statoption]
	    }
	    "-pdf" {
		if {[array exists pdfDHD]} {
		    set metric DHD
		} else {
		    set metric ToT
		}
		foreach index [array names pdf${metric}] {
		    set pdf${metric}([format %0.3f [expr {$index/100.0}]]) [set pdf${metric}($index)]
		    unset pdf${metric}($index)
		}
		return [array get pdf${metric}]
	    }
	    "-pktcount" {
		if {$(units)} {
		    return "Packets (RX)"
		}
		set results [$self __getsamples pktcount $statoption]
	    }
	    "-per" {
		if {$(units)} {
		    return "PER (Rx/Tx)%"
		}
		set results [expr {1.0 - [$self __getsamples pktcount sum] / [$self __getsamples txpktcount sum]}]
	    }
	    "-pps" {
		if {$(units)} {
		    return "PPS (RX)"
		}
		set results [$self __getsamples pps $statoption]
	    }
	    "-offeredactual" {
		if {$(units)} {
		    return "Packets (Rx/TX)%"
		}
		set results [$self __getsamples offeredactual $statoption]
	    }
	    "-outoforder" {
		if {$(units)} {
		    return "Packets"
		}
		set results [$self __getsamples outoforder $statoption]
	    }
	    "-rate" -
	    "-rates" {
		if {$(units)} {
		    return "Thruput (RX bits/s)"
		}
		set results [$self __getsamples rate $statoption]
	    }
	    "-rtt" {
		if {$(units)} {
		    return "microseconds"
		}
		set results [$self __getsamples rtt $statoption]
	    }
	    "-starttime" {
		if {$(units)} {
		    return "Time (seconds)"
		}
		set results $starttime
	    }
	    "-stoptime" {
		if {$(units)} {
		    return "Time (seconds)"
		}
		if {![catch {lindex $samples(events) end} lastevent]} {
		    if {[lindex $lastevent 0] eq "TXOFF"} {
			return [lindex $lastevent 1]
		    }
		}
		return
	    }
	    "-txpktcount" {
		if {$(units)} {
		    return "Packets (TX)"
		}
		set results [$self __getsamples txpktcount $statoption]
	    }
	    "-txpps" {
		if {$(units)} {
		    return "PPS (TX)"
		}
	        if {$mmastats} {
		    set results [$self __getsamples txpktcount utfmma]
		    set results [list [expr {[lindex $results 0] / $options(-reportinterval)}] [expr {[lindex $results 1] / $options(-reportinterval)}] [expr {[lindex $results 2] / $options(-reportinterval)}]]
		} else {
		    set results [$self __getsamples txpktcount seconds]
		}
	    }
	    "-bytes" {
		if {$(units)} {
		    return "Bytes (RX)"
		}
		set results [$self __getsamples bytes $statoption]
	    }
	    "-txbytes" {
		if {$(units)} {
		    return "Bytes (RX)"
		}
		set results [$self __getsamples txbytes $statoption]
	    }
	    "-txrate" -
	    "-txrates" {
		if {$(units)} {
		    return "Offered (TX bits/s)"
		}
		set results [$self __getsamples txrate $statoption]
	    }
	    "-rateevents" {
		set results [$self __getsamples rateevents]
	    }
	    "-count" {
		if {$(units)} {
		    return "Sample Count (TX)"
		}
		set results [llength [$self __getsamples txrate {}]]
	    }
	    "-uptime" {
		if {$(units)} {
		    return "Time (seconds)"
		}
		if {[$self status -silent]} {
		    set results [format %0.3f [expr {[UTF::stream clock] - $starttime}]]
		} else {
		    set results [format %0.3f [expr {[$self stats -stoptime] - $starttime}]]
		}
	    }
	    "-h" -
	    "-help" -
	    "?" {
		puts "stats options are: $statsusage"
	    }
	    default {
		UTF::Message WARN $utf_msgtag "Unknown stats request of $option"
	    }
	}
	# Filter samples based on timestamps
	if {[info exists t1]} {
	    if {$starttime eq "" || $t2 < $t1} {
		return
	    }
	    if {$options(-multicast)} {
		foreach rx $results {
		    set times [lindex $rx 2]
		    set end [expr {[llength $times] - 1}]
		    # RJM, brute force for now, better is a binary search
		    set ix 0
		    while {$t1 > [lindex $times $ix]} {
			incr ix
			if {$ix >= $end} {
			    break
			}
		    }
		    set jx $ix
		    while {[lindex $times $jx] <= $t2} {
			incr jx
			if {$jx >= $end} {
			    break
			}
		    }
		    if {!$ix && !$jx} {
			return
		    }
		    set filtered [lrange [lindex $rx 1] $ix $jx]
		    if {$(sum)} {
			set sum 0.0
			foreach f $filtered {
			    set sum [expr {$sum + $f}]
			}
			set r $sum
		    } elseif {$(meanminmax)} {
			set r [list [UTF::MeanMinMax $filtered]]
		    } elseif {$(timestamps)} {
			set r [list "$filtered" "[lrange [lindex $rx 2] $ix $jx]"]
		    } else {
			set r [list "$filtered"]
		    }
		    lappend filteredresults [concat [lindex $rx 0] $r]
		}
		set results $filteredresults
	    } else {
		set times [lindex $results 1]
		set end [expr {[llength $times] -1}]
		# RJM, brute force for now, better is a binary search
		set ix 0
		while {$t1 > [lindex $times $ix]} {
		    incr ix
		    if {$ix >= $end} {
			break
		    }
		}
		set jx $ix
		while {[lindex $times $jx] <= $t2} {
		    incr jx
		    if {$jx >= $end} {
			break
		    }
		}
		if {!$ix && !$jx} {
		    return
		}
		#	    puts "ix:$ix jx:$jx t1:$t1 t2:$t2"
		if {$(sum)} {
		    set sum 0.0
		    if {![catch {lrange [lindex $results 0] $ix $jx} l]} {
			if {$option ne "-pktlatency"} {
			    foreach n $l {
				set sum [expr {$n + $sum}]
			    }
			} else {
			    set meansum 0; set minsum 0;
			    set maxsum 0; set stdevsum 0;
			    foreach n $l {
				foreach {mean min max stdev} [split $n /] {}
				if {$mean eq "-nan"} {
				    continue
				}
				set meansum [expr {$meansum + $mean}]
				set minsum [expr {$minsum + $min}]
				set maxsum [expr {$maxsum + $max}]
				set stdevsum [expr {$stdevsum + $stdev}]
			    }
			    set sum [list $meansum $minsum $maxsum $stdev]
			}
			return $sum
		    } else {
			return
		    }
		} elseif {$(timestamps)} {
		    if {[catch {list [lrange [lindex $results 0] $ix $jx] [lrange [lindex $results 1] $ix $jx]} results]} {
			return
		    }
		} elseif {$(meanminmax)} {
		    if {$option eq "-pktlatency"} {
			set num 0
			foreach sample [lrange [lindex $results 0] $ix $jx] {
			    foreach {smean smin smax stdev} [split $sample /] {}
			    if {$smean eq "-nan"} {
				continue
			    }
			    if {$num} {
				set mean [expr {$smean + $mean}]
				if {[expr {$smin < $min}]} {
				    set min [string trim $smin]
				}
				if {[expr {$smax > $max}]} {
				    set max [string trim $smax]
				}
			    } else {
				set mean $smean
				set min $smin
				set max $smax
			    }
			    incr num
			}
			set results [list [format %0.3f [expr {$mean / $num}]] $min $max]
		    } elseif {[catch {UTF::MeanMinMax [lrange [lindex $results 0] $ix $jx]} results]} {
			return
		    }
		} else {
		    if {[catch {lrange [lindex $results 0] $ix $jx} results]} {
			return
		    }
		}
	    }
	}
	if {$options(-api_msgs) && ![info exists silent]} {
	    UTF::Message STATS $utf_msgtag "stats $statargs : $results"
	}
	if {$(h)} {
	    if {!$options(-multicast) || [regexp {^\-tx} $option]} {
		return [UTF::stream hformat $results]
	    } else {
		set r $results
		set results {}
		foreach rx $r {
		    set tmp [UTF::stream hformat [lindex $rx 1]]
		    if {[lindex $rx 2] eq ""} {
			lappend results "[lindex $rx 0] [list $tmp]"
		    } else {
			lappend results "[lindex $rx 0] [list $tmp] [list [lindex $rx 2]]"
		    }
		}
	    }
	}
	return $results
    }
    method controlchart {args} {
	UTF::Getopts {
	    {stat.arg "rate" "stat of interest"}
	    {history.arg "30" "control chart history"}
	    {key.arg "" "add to key"}
	    {shortkey "use a shortened self ID as the key"}
	    {exclude.arg "" "Stream attributes to exclude in the control chart key"}
	}
	if {[lsearch -exact [$self stats options] -$(stat)] == -1} {
	    error "No such stat of $(stat)"
	}
	if {[$self stats -count] < 4} {
	    error "Controlchart aborted due to insufficient samples (count=[$self stats -count]) for stat=$(stat)"
	}
	package require UTF::ControlChart
	if {$(shortkey)} {
	    set myid [$self ID -exclude $(exclude)]
	} else {
	    set myid [$self id -exclude $(exclude)]
	}
	UTF::ControlChart CC -key "$(stat) $myid $(key)" -history $(history) -title "$options(-protocol) $(stat)" -units "bs" -norangecheck 1
	UTF::Message DEBUG $utf_msgtag "cckey: [CC cget -key]"
	set boundsresults [CC addsample [$self stats -$(stat) -meanminmax]]
	if {[regexp {(HIGH|LOW|WIDE|ZERO)} $boundsresults]} {
	    set cctests($(stat)) $boundsresults
	} else {
	    set cctests($(stat)) "PASS"
	}
	set ccplots($(stat)) [CC plotcontrolchart $boundsresults]
	CC destroy
	return $ccplots($(stat))
    }
    method getcctests {stat} {
	return $cctests($stat)
    }
    method test {args} {
	UTF::Getopts {
	    {stat.arg "rate" "stat of interest"}
	    {key.arg "" "add to key"}
	    {settle.arg "0" "time to let traffic settle after start"}
	    {ampdu "Dump ampdu stats"}
	    {nowlstats "Don't dump wl stats"}
	}
	if {[lsearch -exact [$self stats options] -$(stat)] == -1} {
	    error "No such stat of $(stat)"
	}
	if {!$(nowlstats)} {
	    catch {$options(-rx) wl counters}
	    catch {$options(-tx) wl counters}
	}
	if {$(ampdu)} {
	    if {[catch {$options(-rx) wl dump ampdu}]} {
		catch {$options(-rx) wl dump_clear ampdu}
	    }
	    if {[catch {$options(-tx) wl dump ampdu}]} {
		catch {$options(-tx) wl dump_clear ampdu}
	    }
	}
	$self start
	if {$(settle)} {
	    UTF::Sleep $(settle)
	}
	if {$options(-linkcheck)} {
	    $self linkcheck -now
	}
	if {$(settle) || $options(-linkcheck)} {
	    $self stats -clear
	}
	UTF::Sleep $options(-t)
	$self stop
	if {!$(nowlstats)} {
	    catch {$options(-rx) wl counters}
	    catch {$options(-tx) wl counters}
	}
	if {$(ampdu)} {
	    catch {$options(-rx) wl dump ampdu}
	    catch {$options(-tx) wl dump ampdu}
	}
	$self controlchart -stat $(stat) -key $options(-key)
	set g [$self plot -text "[lindex [$self stats -$(stat) -meanminmax -h] end]"]
	set res "$g [string range $ccplots($(stat)) 5 end]"
	if {$cctests($(stat)) ne "PASS"} {
	    error $res
	} else {
	    return $res
	}
    }
    method user_event {event} {
	$self __addsample "events" [list $event [UTF::stream clock]]
    }
    method events {} {
	if {[info exists samples(events)]} {
	    return $samples(events)
	}
    }
    #
    # Do any setup needed for STAs (and Hosts) before
    # starting the stream.  This is a onetime setup
    # where subsequent start/stops won't invoke this logic
    #
    method __doonetimesetup {} {
	foreach sta [concat $options(-transmitsta) $options(-receivesta)] {
	    if {[$sta hostis Cygwin WinDHD WinBT]} {
		if {$options(-api_msgs)} {
		    UTF::Message API-I $utf_msgtag "disable iperf : sta = $sta"
		}
		$self __disableiperfservice $sta
		if {$options(-api_msgs)} {
		    UTF::Message API-E $utf_msgtag "disable iperf exit : sta = $sta"
		}
	    }
	}
	if {$options(-installroutes) eq "1"} {
	    if {$options(-multicast)} {
		$self install_mcast_default_route
	    }
	    $self install_host_route
	}
	if {$options(-staticarp)} {
	    $self arp install
	}
    }
    method __dorestore {sta} {
	if {[info exists restorecmds($sta)]} {
	    foreach restorecmd $restorecmds($sta) {
		if {[catch {eval $restorecmd} err]} {
		    UTF::Message ERROR $sta "restore: $err"
		}
	    }
	    unset restorecmds($sta)
	}
    }
    method status {args} {
	UTF::Getopts {
	    {tx "check tx status"}
	    {rx "check rx status"}
	    {silent "no log messages"}
	}
	if {!$(silent) && $options(-api_msgs)} {
	    UTF::Message API-I $utf_msgtag "stream status called with args : $args"
	}
	set rc 1
	if {!$(tx)  && !$(rx)} {
	    set (tx) 1
	    set (rx) 1
	}
	if {$(tx)} {
	    if {!$(silent) && $options(-api_msgs)} {
		UTF::Message $utf_msgtag INFO "transmitter $txstate"
	    }
	    if {$txstate ne "ON"} {
		set rc 0
	    }
	}
	if {$(rx)} {
	    if {!$(silent) && $options(-api_msgs)} {
		UTF::Message $utf_msgtag INFO "receiver(s) $rxstate"
	    }
	    if {$rxstate ne "ON"} {
		set rc 0
	    }
	}
	if {!$(silent) && $options(-api_msgs)} {
	    UTF::Message API-E $utf_msgtag "stream status return of $rc"
	}
	return $rc
    }
    method measurerate {{holdtime 3}} {
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_msgtag "measurerate called : holdtime = $holdtime"
	}
	set ratembs {}
	set values [$self stats -rate]
	if {!$options(-multicast)} {
	    set first [llength $values]
	} else {
	    set values [lindex [lindex $values 0] 1]
	    set first [llength $values]
	}
	UTF::Sleep $holdtime
	set values [$self stats -rate]
	if {!$options(-multicast)} {
	    set values [lrange $values $first end]
	} else {
	    set values [lindex [lindex $values 0] 1]
	    set values [lrange $values $first end]
	}
	if {[llength $values] < 2} {
	    $self linkcheck -now
	    return 0
	}
	set mmacount 0
	foreach value $values {
	    incr mmacount +1
	    if {![info exists mmaprev]} {
		set mmaprev $value
		continue
	    }
	    set mma [expr {$mmaprev + (($value - $mmaprev) / $mmacount)}]
	    set mmaprev $value
	}
	UTF::Message INFO $utf_msgtag "rate: $mma"
	if {$mma ne {} } {
	    set ratembs [expr {int($mma / 1000000)}]
	} else {
	    UTF::Message ERROR $utf_msgtag "No rate samples found"
	    set ratembs 0
	}
	if {$options(-api_msgs)} {
	    UTF::Message API-E $utf_msgtag "measurerate called : holdtime = $holdtime"
	}
	return $ratembs
    }
    #
    # Calibrate a UDP streams offered rate to an actual/offered percentage,
    # defaulting to 95%.
    #
    method calibrate_actualoffered {{calactualoffered 95} {tolerance 2}} {
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_msgtag "calibrate called : value = $calactualoffered"
	}
	if {$options(-protocol) eq "TCP"} {
	    set msg "Unsupported request to calibrate an actual/offered rate for a TCP stream"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	if {$txstate eq "ON" || $rxstate eq "ON"} {
	    set msg "Cannot calibrate an active stream tx=$txstate rx=$rxstate"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	set ao_min [expr {$calactualoffered - $tolerance}]
	set ao_max [expr {$calactualoffered + $tolerance}]
	if {$ao_max > 100} {
	    set ao_max 100
	}
	if {$ao_min < 1} {
	    set ao_min 1
	}
	# Enable advanced stats
	set restoresetting $options(-advancedstats)
	set options(-advancedstats) 1
	#
	# Find a reasonable starting point by oversubscribing
	# the channel and measuring the actual rate
	#
	set prevconfrate $options(-rate)
	set options(-rate) $options(-oversubscribe)
	$self stats -clear
	$self start
	set udprate [$self measurerate 15]
	set linkok 0
	if {[catch {$self linkcheck}]} {
	    UTF::Message WARN $utf_msgtag "linkcheck failed"
	    set udprate 1
	} else {
	    set linkok 1
	}
	set maxadj [expr {int($udprate / 3)}]
	if {$maxadj < 10} {
	    set maxadj 10
	}
	$self stop
	if {$udprate == 0} {
	    if {[lindex [$self stats -rate -meanminmax] 0] > 1000} {
		set udprate 1
	    }
	}
	if {$udprate <=0 || $udprate eq {} } {
	    set options(-advancedstats) $restoresetting
	    set options(-rate) $prevconfrate
	    set msg "Calibrate failed on oversubscribe: rate = $udprate"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	set options(-rate) ${udprate}M
	set bestrate $udprate
	#
	#  Now calibrate to actual/offered plusminus 1 or
	#  to an offered rate at which the step toggles between
	#  +1 and -1.  (meaning, that's the closest we're
	#  going to get.)
	#
	$self start
	set goodenough 5
	set loopcontrol $options(-caltries)
	set hunts "$udprate $udprate"
	set huntfactor 1
	while {$loopcontrol} {
	    incr loopcontrol -1
	    $self stats -clear
	    UTF::Sleep $options(-dwell)
	    if {[catch {$self linkcheck}]} {
		UTF::Message WARN $utf_msgtag "linkcheck failed"
		break
	    } else {
		set linkok 1
	    }
	    set tmp [$self stats -offeredactual -mma]
	    if {$options(-multicast)} {
		set measurement [expr {int([lindex [lindex [lindex $tmp 0] 1] 0])}]
	    } else {
		set measurement [expr {int([lindex $tmp 0])}]
	    }
	    #
	    # Reasons to break the loop and assume
	    # this is a good as it gets
	    # 1) No received traffic
	    # 2) No transmitted traffic
	    # 3) The requested tx rate diverges from the actual
	    # 4) There has been 5 or more transitions
	    #
	    if {$measurement eq {} } {
		UTF::Message ERROR $utf_msgtag "Calibrate error due null offereactual stats sample"
		break
	    }
	    set txrate [lindex [$self stats -txrate -mma] 0]
	    if {$txrate eq {}} {
		UTF::Message ERROR $utf_msgtag "Calibrate error due to null tx rate"
		break
	    }
	    set request [expr {$udprate * 1000000.0}]
	    if {[expr {($request - $txrate) / $request}] > 0.35} {
		UTF::Message INFO $utf_msgtag "Requested transmit rate diverged from measured transmit rate"
		set bestrate [expr {int($txrate/1000000)}]
		break
	    }
	    if {[transitions $hunts] >= $goodenough} {
		break
	    }
	    UTF::Message CALIB $utf_msgtag "Rate=${udprate}M Actual/Offered=$measurement"
	    if {$measurement > $ao_max} {
		if {$udprate > $bestrate} {
		    set bestrate $udprate
		    set maxadj [expr {int($udprate / 3)}]
		}
		if {[lindex $hunts end] < [lindex $hunts end-1]} {
		    set huntfactor 0
		} else {
		    incr huntfactor +1
		}
		set adj [expr {int(pow($huntfactor,2))}]
		if {$adj > $maxadj} {
		    set adj $maxadj
		}
		set udprate [expr {$udprate + $adj}]
	    } elseif {$measurement < $ao_min} {
		if {[lindex $hunts end] > [lindex $hunts end-1]} {
		    set huntfactor 0
		} else {
		    incr huntfactor +1
		}
		set adj [expr {int(pow($huntfactor,2))}]
		set udprate [expr {$udprate - $adj}]
		if {$udprate < 1} {
		    set udprate 1
		}
	    } else {
		set adj 0
		set huntfactor 0
	    }
	    lappend hunts $udprate
	    UTF::Message INFO "Hunts" "M:$measurement factor:$adj rates: $hunts"
	    #
	    #  Use the self configure method because it will
	    #  start and stop the stream to apply the new rate
	    #
	    $self configure -rate ${udprate}M
	}
	if {!$loopcontrol} {
	    UTF::Message WARN $utf_msgtag "Couldn't calibrate stable udp offer rate"
	}
	$self stop
	$self stats -clear
	set options(-advancedstats) $restoresetting
	set options(-rate) $prevconfrate
	if {$options(-api_msgs)} {
	    UTF::Message API-E $utf_msgtag "calibrate return : requested rate=${udprate}M measured rate ${bestrate}M"
	}
	if {$linkok} {
	    return $bestrate
	} else {
	    error "link error"
	}
    }
    method __iperf_txhandler {fid} {
	if {$noreentrancy($fid)} {
	    UTF::Message WARN $utf_txmsgtag "reentrancy ignored $fid"
	    return
	}
	set noreentrancy($fid) 1
	set buf [gets $fid]
	if {![eof $fid]} {
	    if {$options(-txdisplay)} {
		UTF::Message HNDLR+$options(-msgcolor) $utf_txmsgtag "$buf ($fid2device($fid),$fid)"
	    }
	    if {[regexp {.+[0-9\.]+ sec[ ]+([0-9.]+) ([KM])?Bytes[ ]+([0-9\.]+) ([KMG]?)bits/sec(\s+([0-9]+)/([0-9]+)\s+([0-9]+)\s+([0-9]+)K/([0-9]+))?} $buf - bytes byteunits rate units eplus writes errwrite rtry cwnd rtt]} {
		if {$bytes > 0} {
		    set linkcheck_flags($fid2device($fid)) 1
		}
		# Cancel any pending null sample watches
		if {![catch {after cancel $fid2afterid($fid)}]} {
		    unset fid2afterid($fid)
		}
		if {$samplersenabled} {
		    set timestamp [UTF::stream clock]
		    lappend timestamps($fid2device($fid)) $timestamp
		    if {$starttime eq {}} {
			set starttime $timestamp
			if {$options(-api_msgs)} {
			    UTF::Message INFO $utf_msgtag "stream STARTTIME set to $starttime"
			}
		    }
		    if {$options(-txnulldetect) && $txstate eq "ON" && $handlerstate(cmd) ne "STOPPING"} {
			set t [expr {int($options(-reportinterval) * 2000)}]
			set fid2afterid($fid) [after $t [mymethod __iperf_nullevent $fid]]
		    }
		}
		set rate [format %0.1f [expr {[unitscalefactor $units] * $rate}]]
		set bytes [format %0.1f [expr {[unitscalefactor $byteunits] * $bytes}]]
		$self __addsample "$fid2device($fid),txrate" $rate
		if {$eplus ne {}} {
		    $self __addsample "$fid2device($fid),rtt" $rtt
		    $self __addsample "$fid2device($fid),cwnd" $cwnd
		    $self __addsample "$fid2device($fid),writes" $writes
		}
		# Keep the rx and tx synched, i.e. don't count tx bytes when
		# rx is in the off state
		if {$handlerstate($fid) eq {READY}} {
		    $self __addsample "$fid2device($fid),txbytes" $bytes
		}
		if {$options(-protocol) eq "UDP"} {
		    set txpktcount [format %0.1f [expr {$bytes * 1.0 / $reqpktsize}]]
		    $self __addsample "$fid2device($fid),txpktcount" $txpktcount
		    if {$options(-advancedstats)} {
			$self __compute_mma "$fid2device($fid),txrate" $rate
			if {$rxstate ne "OFF"} {
			    $self __compute_mma "$fid2device($fid),txbytes" $bytes
			}
			$self __compute_mma "$fid2device($fid),txpktcount" $txpktcount
		    }
		} elseif {$options(-advancedstats)} {
		    $self __compute_mma "$fid2device($fid),txrate" $rate
		    if {$handlerstate($fid) eq {READY}} {
			$self __compute_mma "$fid2device($fid),txbytes" $bytes
		    }
		}
		if {$options(-multicast)} {
		    set clearstats_synch 1
		}
		$self __triggercheck tx
	    } elseif {[regexp {^[-]{40}} $buf]} {
		incr iperfcmds($fid,hdr)
		if {$iperfcmds($fid,hdr) == 2} {
		    set txstate "GOTO_ON"
		}
	    } elseif {[regexp {^Client connecting to} $buf]} {
		set txstate "GOTO_ON"
	    } elseif {[regexp {^\[ ID\] Interval       Transfer     Bandwidth} $buf]} {
		# silently ignore this title bar
	    } elseif {[regexp {Waiting for server threads to complete\. Interrupt again to force quit\.} $buf]} {
		set handlerstate($fid) "CLOSING"
		if {[catch {exec kill -s HUP $fid2pid($fid)} err]} {
		    UTF::Message HNDLR_ERR $utf_txmsgtag "$err $fid2device($fid) "
		    # let the upper level code (non event handler context)
		    # recover from this
		} else {
		    UTF::Message HNDLR $utf_txmsgtag "Kill signal -HUP sent to ($fid2device($fid),$fid) : $iperfcmds($fid)"
		}
	    } elseif {[regexp {No such file or directory} $buf]} {
		UTF::Message HNDLR_ERR $utf_txmsgtag "$buf ($fid2device($fid),$fid)"
		close $fid
	    } elseif {[regexp {write failed: Operation not permitted} $buf]} {
		if {![$options(-transmitsta) hostis Cygwin WinDHD WinBT]} {
		    catch {eval [concat $options(-transmitsta) rexec $IPTABLESCMD -L]} output
		}
	    } elseif {[regexp {Connection to fsh tunnel lost} $buf]} {
		UTF::Message HNDLR_ERR $utf_txmsgtag "$buf ($fid2device($fid),$fid)"
		if {![catch {after cancel $fid2afterid($fid)}]} {
		    unset fid2afterid($fid)
		}
		if {$options(-restart_on_fsherror) && !$restart_pending} {
		    UTF::Message HNDLR_RESTART $utf_txmsgtag "Restart posted per fsh tunnel error ($fid2device($fid),$fid)"
		    set restart_pending 1
		    after 1000 [mymethod __restart]
		}
	    } elseif {!$options(-txdisplay)} {
		UTF::Message HNDLR_ $utf_txmsgtag "$buf ($fid2device($fid),$fid)"
	    }
	    if {$txstate eq "GOTO_ON"} {
		if {![catch {after cancel $fid2afterid($fid)}]} {
		    unset fid2afterid($fid)
		}
		set fid2pid($fid) [pid $fid]
		set txfids $fid
		set handlerstate($fid) "READY"
		set txstate "ON"
		$self __addsample "events" [list "TXON" [UTF::stream clock]]
		if {$options(-api_msgs)} {
		    UTF::Message HNDLR $utf_txmsgtag "Handler now SAMPLING ($fid2device($fid),$fid)"
		}
		if {$options(-txnulldetect)} {
		    set t [expr {int($options(-reportinterval) * 2000)}]
		    set fid2afterid($fid) [after $t [mymethod __iperf_nullevent $fid]]
		}
	    }
	} else {
	    if {$handlerstate($fid) ne "INIT"} {
		if {$options(-txdisplay) || $handlerstate(cmd) ne "STOPPING"} {
		    UTF::Message HNDLR $utf_txmsgtag "Close actions for event handler ($fid2device($fid),$fid)"
		}
		$self __close_handler_actions $fid
	    } else  {
		if {$options(-txdisplay)} {
		    UTF::Message HNDLR $utf_txmsgtag "Closing event handler ($fid2device($fid),$fid)"
		}
		fconfigure $fid -blocking 0
		if {[catch {close $fid} err]} {
		    UTF::Message HNDLR_ERR $utf_txmsgtag "$err $fid2device($fid) $fid"
		}
	    }
	}
	set noreentrancy($fid) 0
    }
    method __iperf_rxhandler {fid} {
	if {$noreentrancy($fid)} {
	    UTF::Message WARN $utf_rxmsgtag "reentrancy ignored $fid"
	    return
	}
	set noreentrancy($fid) 1
	set buf [gets $fid]
	if {![eof $fid]} {
	    if {$options(-rxdisplay)} {
		UTF::Message HNDLR+$options(-msgcolor) $utf_rxmsgtag "$buf ($fid2device($fid),$fid)"
	    }
	    if {([string equal $options(-protocol) UDP] && [regexp {^\[[\s0-9]+\] ([0-9\.\-]+) sec\s+([0-9.]+) ([KM])?Bytes\s+([0-9\.]+) ([KMG]?)bits/sec\s+([0-9\.]+)\sms\s+([0-9]+)/\s*([0-9]+)\s+\((-?[0-9ane\+\.]+)%\)\s+(ToT:)?\s*(-?[0-9an\.\-]+/\s*-?[0-9an\.\-]+/\s*-?[0-9an\.\-]+/\s*-?[0-9an\.\-]+)?(\s+DHD:\s*)?(-?[0-9an\.\-]+/\s*-?[0-9an\.\-]+/\s*-?[0-9an\.\-]+/\s*-?[0-9an\.\-]+)?(\sDHDd:.*FW4d:[\s\-0-9./]+)?(\sms\s+([0-9]+)\spps)?} $buf - iperf_interval bytes byteunits rate units jitter lost totpktcount lostper tottag customa dhdtag customb fwdeltas nocare pps]) || ([string equal $options(-protocol) TCP] && [regexp {^\[[\s0-9]+\] ([0-9\.\-]+) sec[ ]+([0-9.]+) ([KM])?Bytes[ ]+([0-9\.]+) ([KMG]?)bits/sec} $buf - iperf_interval bytes byteunits rate units])} {
		if {$bytes > 0} {
		    set linkcheck_flags($fid2device($fid)) 1
		}
		if {![catch {after cancel $fid2afterid($fid)}]} {
		    unset fid2afterid($fid)
		}
		if {!$outoforder_posted($fid)}  {
		    $self __addsample "$fid2device($fid),outoforder" 0
		} else {
		    set outoforder_posted($fid) 0
		}
		if {$samplersenabled} {
		    set timestamp [UTF::stream clock]
		    if {$starttime eq {}} {
			set starttime $timestamp
			if {$options(-api_msgs)} {
			    UTF::Message INFO $utf_rxmsgtag "stream STARTTIME set to $starttime"
			}
		    } else {
			foreach {t1 t2} [split $iperf_interval -] {}
			if {[expr {!$t1 && (($t2-$t1) > $options(-reportinterval))}]} {
			    set noreentrancy($fid) 0
			    return
			}
		    }
		    lappend timestamps($fid2device($fid)) $timestamp
		    if {$options(-rxnulldetect) && $txstate eq "ON" && $handlerstate(cmd) ne "STOPPING"} {
			set t [expr {int($options(-reportinterval) * 2000)}]
			set fid2afterid($fid) [after $t [mymethod __iperf_nullevent $fid]]
		    }
		    set rate [format %0.1f [expr {[unitscalefactor $units] * $rate}]]
		    set bytes [format %0.1f [expr {[unitscalefactor $byteunits] * $bytes}]]
		    $self __addsample "$fid2device($fid),bytes" $bytes
		    if {$options(-advancedstats)} {
			$self __compute_mma "$fid2device($fid),rate" $rate
		    }
		    $self __addsample "$fid2device($fid),rate" $rate
		    if {[string equal $options(-protocol) UDP]} {
			if {$lost < 0 && !$options(-dupadjust)} {
			    set pktcount $totpktcount
			} else {
			    set pktcount [expr {int($totpktcount - $lost)}]
			}
			$self __addsample "$fid2device($fid),pktcount" $pktcount
			$self __addsample "$fid2device($fid),lost" $lost
			if {[catch {set lostper [expr {$lostper}]}]} {
			    set lostper -0
			}
			$self __addsample "$fid2device($fid),lostper" $lostper
			# Use the custom field to detect if this is a legacy iperf
			# that doesn't have PPS and latency enhancments
			if {$customb ne {}} {
			    set pktlatency $customb
			} elseif {$customa ne {}} {
			    set pktlatency $customa
			}
			if {$pktlatency eq {-/-/-/-}} {
			    $self __addsample "$fid2device($fid),pktlatency" {-0/-0/-0/-0}
			} else {
			    $self __addsample "$fid2device($fid),pktlatency" $pktlatency
			}
			if {$pps eq {}} {
			    set pps [expr {round($pktcount / $options(-reportinterval))}]
			}
			$self __addsample "$fid2device($fid),pps" $pps
			# iperf reports jitter in milliseconds - convert to seconds
			$self __addsample "$fid2device($fid),jitter" [expr {$jitter / 1000.0}]
		    }
		    set tx $options(-transmitsta)
		    if {![catch {lindex $samples($tx,txrate) end} denom]} {
			set denom [lindex $samples($tx,txrate) end]
			if {$denom > 0} {
			    set offeredactual [format "%2.2f" [expr {100.0 * $rate / $denom}]]
			} else {
			    set offeredactual "-0"
			}
			$self __addsample "$fid2device($fid),offeredactual" $offeredactual
			if {$options(-advancedstats)} {
			    $self __compute_mma "$fid2device($fid),offeredactual" $offeredactual
			}
		    }
		    if {$options(-advancedstats) && [string equal $options(-protocol) UDP]} {
			$self __compute_mma "$fid2device($fid),pktcount" $pktcount
			$self __compute_mma "$fid2device($fid),lost" $lost
			$self __compute_mma "$fid2device($fid),jitter" $jitter
			$self __compute_mma "$fid2device($fid),bytes" $bytes
			$self __compute_mma "$fid2device($fid),pps" $pps
		    }
		    if {!$options(-multicast)} {
			set clearstats_synch 1
		    }
		    $self __triggercheck rx
		}
	    } elseif {[regexp {.*PDF:([a-zA-z]+)\(bins/size=.+,10/90=([0-9]+)/([0-9]+)\)=([0-9: ]+)} $buf - metric ten ninety ipdf]} {
		foreach data $ipdf {
		    foreach {bin val} [split $data :] {}
		    incr pdf${metric}($bin) $val
		}
	    } elseif {[regexp {^\[ ID\] Interval       Transfer     Bandwidth} $buf]} {
		# silently ignore the title bar
	    } elseif {[regexp { ([0-9]+)[ ]+datagrams received out-of-order} $buf - oocnt]} {
		$self __addsample "$fid2device($fid),outoforder" $oocnt
		set outoforder_posted($fid) 1
	    } elseif {[regexp {^[-]{40}} $buf]} {
		incr iperfcmds($fid,hdr)
		if {$iperfcmds($fid,hdr) == 2} {
		    set fid2pid($fid) [pid $fid]
		    lappend rxfids $fid
		    set handlerstate($fid) "READY"
		    if {$options(-api_msgs)} {
			UTF::Message HNDLR $utf_rxmsgtag "Handler now SAMPLING ($fid2device($fid),$fid)"
		    }
		    # Set to 1 so the first sample for ooo will be skipped/delayed
		    set outoforder_posted($fid) 1
		    if {$options(-rxnulldetect) && $txstate eq "ON" && $handlerstate(cmd) ne "STOPPING"} {
			set t [expr {int($options(-reportinterval) * 2000)}]
			set fid2afterid($fid) [after $t [mymethod __iperf_nullevent $fid]]
		    }
		}
	    } elseif {[regexp {Waiting for server threads to complete\. Interrupt again to force quit\.} $buf]} {
		set handlerstate($fid) "CLOSING"
		if {[catch {exec kill -s HUP $fid2pid($fid)} err]} {
		    UTF::Message HNDLR $utf_rxmsgtag  "$err ($fid2device($fid)) : $iperfcmds($fid) ${fid}"
		    # let the upper level code (non event handler context)
		    # recover from this
		} else {
		    UTF::Message HNDLR $utf_rxmsgtag "Kill signal -HUP sent to ($fid2device($fid),$fid) : $iperfcmds($fid)"
		}
	    } elseif {[regexp {No such file or directory} $buf]} {
		UTF::Message HNDLR_ERR $utf_rxmsgtag $buf
		close $fid
	    } elseif {[regexp {Connection to fsh tunnel lost} $buf] || [regexp {IPerf Service is removed.} $buf]} {
		UTF::Message HNDLR_ERR $utf_rxmsgtag "$buf ($fid2device($fid),$fid)"
		if {$options(-restart_on_iperferror) && [regexp {IPerf Service is removed.} $buf]} {
		    if {$options(-multicast)} {
			UTF::Message HNDLR $utf_rxmsgtag "rx handler requesting start ($fid2device($fid),$fid)"
			after idle "$self start"
		    } else {
			set restart_on_txclose 1
			UTF::Message INFO $utf_rxmsgtag "Will restart stream on txclose error ($fid2device($fid),$fid)"
		    }
		    UTF::Message HNDLR $utf_rxmsgtag "rx handler requesting start"
		    after 100 "$self start"
		}
		catch {$self __close_handler_actions $fid}
	    } elseif {[regexp {bind failed: Address already in use} $buf] && [$options(-receivesta) hostis Linux]} {
		if {!$options(-rxdisplay)} {
		    UTF::Message HNDLR_ $utf_rxmsgtag "$buf ($fid2device($fid),$fid)"
		}
		$options(-receivesta) rexec "ss --[string tolower $options(-protocol)] -a -n"
	    } elseif {!$options(-rxdisplay)} {
		# ignore superfulous SUM messages
		if {![regexp {^\[SUM\] } $buf]} {
		    UTF::Message HNDLR_ $utf_rxmsgtag "$buf ($fid2device($fid),$fid)"
		}
	    }
	} else {
	    if {$handlerstate($fid) ne "INIT"} {
		if {$options(-rxdisplay) || $handlerstate(cmd) ne "STOPPING"} {
		    UTF::Message HNDLR $utf_rxmsgtag "Close actions for event handler ($fid2device($fid),$fid)"
		}
		$self __close_handler_actions $fid
	    } else {
		if {$options(-rxdisplay)} {
		    UTF::Message HNDLR $utf_rxmsgtag "Closing event handler ($fid2device($fid),$fid)"
		}
		fconfigure $fid -blocking 0
		if {[catch {close $fid} err]} {
		    UTF::Message HNDLR_ERR $utf_rxmsgtag "$err $fid2device($fid) $fid"
		}
	    }
	}
	set noreentrancy($fid) 0
    }
    method __close_handler_actions {fid} {
	catch {fileevent $fid readable {}}
	if {$txfids eq $fid} {
	    set type "tx"
	    set txfids {}
	} else {
	    set type "rx"
	}
	if {![catch {after cancel $fid2afterid($fid)}]} {
	    unset fid2afterid($fid)
	}
	unset fid2pid($fid)
	fconfigure $fid -blocking 0
	if {[catch {close $fid} rperr]} {
	    if {$rperr eq {child process exited abnormally}} {
		set msgtype INFO
	    } else {
		set msgtype WARN
	    }
	    UTF::Message $msgtype $utf_msgtag "close ($fid2device($fid),$fid) : $rperr"
	} else {
	    if {$options(-${type}display)} {
		UTF::Message INFO ${utf_msgtag}-${type}  "Closed : ($fid2device($fid),$fid)"
	    }
	}
	# if the local fsh pid is still here
	# try to kill it
	if {![catch {pid $fid} localpid]} {
	    UTF::Message INFO $utf_msgtag "Sending kill to local pid $fid2device($fid) pid $localpid"
	    if {[catch {kill -s KILL $localpid} err]} {
		UTF::Message WARN $utf_msgtag "Local kill failed: $fid2device($fid) $pid"
	    }
	}
	if {$type eq "tx"} {
	    set txstate "OFF"
	    $self __addsample "events" [list "TXOFF" [UTF::stream clock]]
	} else {
	    set rxfids [lsearch -all -not -inline $rxfids $fid]
	    if {![llength $rxfids]} {
		set rxstate "OFF"
	    }
	}
	unset fid2device($fid)
	set handlerstate($fid) "CLOSED"
	unset noreentrancy($fid)
	if {$restart_on_txclose && $type eq "tx"} {
	    set restart_on_txclose 0
	    UTF::Message HNDLR $utf_msgtag "close handling requesting start"
	    after idle "$self start"
	}
    }
    method __triggercheck {rxtx} {
	if {$options(-trigger_armed) && $options(-trigger_callback) ne {}} {
	    if {[catch {eval [concat $options(-trigger_callback) $rxtx $self]} err]} {
		UTF::Message ERROR $utf_msgtag "userdefined trigger: $err"
		set options(-trigger_armed) 0
	    }
	}
    }
    method __iperf_nullevent {fid} {
	unset fid2afterid($fid)
	if {!$samplersenabled} {
	    return
	}
	if {$txstate eq "ON" && $handlerstate(cmd) ne "STOPPING"} {
	    if {$options(-multicast) || $fid ne $txfids} {
		set clearstats_synch 1
	    }
	    set t [expr {int($options(-reportinterval) * 1000)}]
	    set fid2afterid($fid) [after $t [mymethod __iperf_nullevent $fid]]
	}
	$self __add_null_sample $fid [expr {[UTF::stream clock] - $options(-reportinterval)}]
    }
    method __add_null_sample {fid timestamp} {
	if {$starttime eq {}} {
	    set starttime $timestamp
	    if {$options(-api_msgs)} {
		UTF::Message INFO ${utf_msgtag} "stream STARTTIME set to $starttime"
	    }
	}
	lappend timestamps($fid2device($fid)) $timestamp
	if {$txfids eq $fid} {
	    $self __addsample "$fid2device($fid),txrate" "-0"
	    $self __addsample "$fid2device($fid),txpktcount" "-0"
	    $self __addsample "$fid2device($fid),txbytes" "-0"
	    set hndlrtype "tx"
	    if {$options(-advancedstats)} {
		$self __compute_mma "$fid2device($fid),txrate" 0.0
		$self __compute_mma "$fid2device($fid),txpktcount" 0.0
		$self __compute_mma "$fid2device($fid),txbytes" 0.0
	    }
	} else {
	    set hndlrtype "rx"
	    $self __addsample "$fid2device($fid),rate" "-0"
	    $self __addsample "$fid2device($fid),pktcount" "-0"
	    $self __addsample "$fid2device($fid),lost" "-0"
	    $self __addsample "$fid2device($fid),jitter" "-0"
	    $self __addsample "$fid2device($fid),outoforder" "-0"
	    $self __addsample "$fid2device($fid),bytes" "-0"
	    $self __addsample "$fid2device($fid),pps" "-0"
	    if {$options(-advancedstats)} {
		$self __compute_mma "$fid2device($fid),rate" 0.0
		$self __compute_mma "$fid2device($fid),pktcount" 0.0
		$self __compute_mma "$fid2device($fid),lost" 0.0
		$self __compute_mma "$fid2device($fid),jitter" 0.0
		$self __compute_mma "$fid2device($fid),outoforder" 0.0
		$self __compute_mma "$fid2device($fid),bytes" 0.0
	    }
	}
	$self __triggercheck $hndlrtype
	if {($options(-protocol) eq "UDP" && $options(-rxdisplay)) || ($options(-protocol) eq "TCP" && $options(-txdisplay))} {
	    UTF::Message HNDLR ${utf_msgtag}-$hndlrtype "NULL sample ($fid2device($fid),$fid)"
	}
    }
    method __addsample {key value} {
	if {$samplersenabled  || $key eq "events"} {
	    lappend samples($key) $value
	}
	return
    }
    method __getsamples {key {statoption ""}} {
	set results {}
	set mcresults {}
	if {[lsearch -exact $statoption "timestamps"] != -1} {
	    set statoption [lsearch -all -not -inline $statoption "timestamps"]
	    set appendtimestamps 1
	} else {
	    set appendtimestamps 0
	}
	if {$key eq "latency"} {
	    if {![info exists samples(latency)]} {
		return
	    }
	    foreach x $samples(latency) {
		lappend values [format %.6f [lindex $x 0]]
		lappend times [lindex $x 1]
	    }
	    if {$statoption eq "sum"} {
		set sum 0
		foreach value $values {
		    set sum [expr {$sum + $value}]
		}
		return [format %.6f $sum]
	    }
	    if {$appendtimestamps} {
		return "[list $values] [list $times]"
	    } else {
		return $values
	    }
	} elseif {$key eq "txrate" || $key eq "txpktcount" || $key eq "txbytes" || $key eq "rtt" || $key eq "cwnd"} {
	    set tx $options(-transmitsta)
	    if {$statoption eq "seconds" && $key eq "txrate"} {
		set statoption ""
	    }
	    if {[info exists samples($tx,$key)]} {
		switch -exact $statoption {
		    "" {
			set results $samples($tx,$key)
		    }
		    "utfmean" {
			set results [UTF::MeanMinMax $samples($tx,$key)]
		    }
		    "utfmma" {
			set results [$self __get_mma "$tx,$key"]
		    }
		    "utfcclimits" {
			set results [$self __get_cclimits "$tx,$key"]
		    }
		    "sum" {
			set results 0.0
			foreach sample $samples($tx,$key) {
			    set results [expr {$results + $sample}]
			}
		    }
		    "seconds" {
			set results ""
			foreach sample $samples($tx,$key) {
			    lappend results [format %0.1f [expr {1.0 * $sample / $options(-reportinterval)}]]
			}
		    }
		}
		if {$appendtimestamps} {
		    set results [list $results $timestamps($tx)]
		}
	    }
	} else {
	    set rxs $options(-receivesta)
	    if {$statoption eq "seconds" && $key eq "rate"} {
		set statoption ""
	    }
	    foreach rx $rxs {
		set results ""
		if {[info exists samples(${rx},$key)]} {
		    switch -exact $statoption {
			"" {
			    set results $samples($rx,$key)
			}
			"utfmean" {
			    if {$key ne "pktlatency"} {
				set results [UTF::MeanMinMax $samples($rx,$key)]
			    } else {
				set cnt 0
				foreach sample $samples($rx,$key) {
				    foreach {smean smin smax stdev} [split $sample /] {}
				    if {$smean eq "-nan"} {
					continue
				    }
				    if {[expr {$cnt > 0}]} {
					set mean [expr {$smean + $mean}]
					if {[expr {$smin < $min}]} {
					    set min [string trim $smin]
					}
					if {[expr {$smax > $max}]} {
					    set max [string trim $smax]
					}
				    } else {
					set mean $smean
					set min $smin
					set max $smax
				    }
				    incr cnt
				}
				set results [list [format %0.3f [expr {$mean / $cnt}]] $min $max]
			    }
			}
			"utfmma" {
			    set results [$self __get_mma "$rx,$key"]
			}
			"utfcclimits" {
			    set results [$self __get_cclimits "$rx,$key"]
			}
			"sum" {
			    if {$key ne "pktlatency"} {
				set rxresult 0.0
				foreach sample $samples($rx,$key) {
				    set rxresult [expr {$rxresult + $sample}]
				}
				set results $rxresult
			    } else {
				set meansum 0; set minsum 0;
				set maxsum 0; set stdevsum 0;
				foreach sample $samples($rx,$key) {
				    foreach {mean min max stdev} [split $sample /] {}
				    set meansum [expr {$meansum + $mean}]
				    set minsum [expr {$minsum + $min}]
				    set maxsum [expr {$maxsum + $max}]
				    set stdevsum [expr {$stdevsum + $stdev}]
				}
				set results [list $meansum $minsum $maxsum $stdev]
			    }
			}
			"seconds" {
			    set rxresult ""
			    foreach sample $samples($rx,$key) {
				lappend rxresult [format %0.1f [expr {1.0 * $sample / $options(-reportinterval)}]]
			    }
			    set results $rxresult
			}
		    }
		    if {$appendtimestamps} {
			# outoforder and timestamps can be one sample
			# delayed, correct it here if needed.
			# This is due to the way iperf displays
			# the out of order line seperately and after
			# the sample line.
			if {$key eq "outoforder" && [llength $results] != [llength $timestamps($rx)]} {
			    set results [list $results [lrange $timestamps($rx) 0 end-1]]
			} else {
			    set results [list $results $timestamps($rx)]
			}
		    } elseif {$options(-multicast)} {
			set results [list $results]
		    }
		    if {$options(-multicast)} {
			if {[catch {set r [concat $lan2sta($rx) $results]}]} {
			    set r [concat $rx $results]
			}
			lappend mcresults $r
		    }
		}
	    }
	}
	if {$mcresults ne ""} {
	    return $mcresults
	} else {
	    return $results
	}
    }
    #
    # Fit a line to a running sample set
    #
    # slope = ( SUMx*SUMy - n*SUMxy ) / ( SUMx*SUMx - n*SUMxx );
    # y_intercept = ( SUMy - slope*SUMx ) / n;
    #
    # Note: Since the samples are roughly the same using a
    # normalized value for x of 1 should be sufficient for
    # most uses.
    method __compute_leastsquares {key yvalue {xvalue 1}} {
	if {![info exists runningstats(${key},count)]} {
	    set runningstats(${key},sumx) 0
	    set runningstats(${key},sumxx) 0
	    set runningstats(${key},sumy) 0
	    set runningstats(${key},sumxy) 0
	    set runningstats(${key},count) 0
	}
	#  (SUMx*SUMy - n*SUMxy ) / ( SUMx*SUMx - n*SUMxx)
	set x $runningstats(${key},count)
	set sumx [expr {$runningstats(${key},sumx) + $x}]
	set sumy [expr {$runningstats(${key},sumy) + $yvalue}]
	set sumxx [expr {$runningstats(${key},sumxx) + ($x * $x)}]
	set sumxy [expr {$runningstats(${key},sumxy) + ($x * $yvalue)}]
	set numerator [expr {($sumx * $sumy) - (($x +1 ) * $sumxy)}]
	set denominator [expr {($sumx * $sumx) - (($x + 1) * $sumxx)}]
	if {$denominator} {
	    set slope [expr {$numerator / $denominator}]
	    set average [expr {($sumy - ($slope * $sumx)) / ($x + 1)}]
	    set samples(${key},yintercept) $average
	    set samples(${key},slope) $slope
	}
	#
	#  Store running values need for next interation
	#
	set runningstats(${key},sumx) [expr {double($sumx)}]
	set runningstats(${key},sumxx) [expr {double($sumxx)}]
	set runningstats(${key},sumy) [expr {double($sumy)}]
	set runningstats(${key},sumxy) [expr {double($sumxy)}]
	set runningstats(${key},count) [expr {$runningstats(${key},count) + $xvalue}]
    }
    #
    # Compute a modified moving average
    #
    # MAt = MAt-1 + 1/n * (Pt - (MAt-1))
    #
    method __compute_mma {key value} {
	if {![info exists samples(${key},mma)]} {
	    set mmaprev 0.0
	    set runningstats(${key},mmacount) 1
	} else {
	    set mmaprev $samples(${key},mma)
	    incr runningstats(${key},mmacount) +1
	}
	set samples(${key},mma) [expr {$mmaprev + (($value - $mmaprev) / $runningstats(${key},mmacount))}]
	$self __compute_leastsquares "${key},mma" $samples(${key},mma)
	# $self __compute_icontrolchart $key $value
    }
    method __get_mma {key} {
	# need at least two samples
	if {[info exists runningstats(${key},mma,count)] && $runningstats(${key},mma,count) >= 2} {
	    return "$samples(${key},mma) $samples(${key},mma,slope) $samples(${key},mma,yintercept)"
	}
    }
    method __get_cclimits {key} {
	set results ""
	if {[info exists samples(${key},Ux)]} {
	    foreach ucl $samples(${key},Ux) lcl $samples(${key},Lx) ur $samples(${key},UR) {
		lappend results "$ucl $lcl $ur"
	    }
	}
	return $results
    }
    method controlchart_reset {} {
	foreach key [array names samples] {
	    catch {unset samples(${key},meansum)}
	}
    }
    method __compute_icontrolchart {key value} {
	if {[string is integer $value]} {
	    append value ".0"
	}
	if {![info exists samples(${key},meansum)]} {
	    set samples(${key},meansum) $value
	    set samples(${key},prev) $value
	    set samples(${key},rsum) 0
	    set samples(${key},icc_count) 1
	    lappend samples(${key},Ux) "reset"
	    lappend samples(${key},Lx) "reset"
	    lappend samples(${key},UR) "reset"
	    return
	} else {
	    set samples(${key},meansum) [expr {$samples(${key},meansum) + $value}]
	    set R [expr {abs($value - $samples(${key},prev))}]
	    set samples(${key},rsum) [expr {$samples(${key},rsum) + $R}]
	    set samples(${key},prev) $value
	    incr samples(${key},icc_count) +1
	}
	set count $samples(${key},icc_count)
	set meanmean [expr {$samples(${key},meansum)/$count}]
	set Rmean [expr {$samples(${key},rsum)/($count - 1)}]
	# Calculate X-bar and Rs limits based on moving range
	# Ishikawa, Kaoru
	set Ux [expr {$meanmean + $E2*$Rmean}]
	set Lx [expr {$meanmean - $E2*$Rmean}]
	set UR [expr {$D4*$Rmean}]
	lappend samples(${key},Ux) $Ux
	lappend samples(${key},Lx) $Lx
	lappend samples(${key},UR) $UR
	# Check limits
	set msg ""
	set reason ""
	set f "%.2f"
	if {$value < $Lx} {
	    set msg [format "$f \[$f - $f\], range $f \[$f\] LOW" $value $Lx $Ux $R $UR]
	    set reason "LOW"
	} elseif {$value > $Ux} {
	    set msg [format "$f \[$f - $f\], range $f \[$f\] HIGH" $value $Lx $Ux $R $UR]
	    set reason "HIGH"
	} elseif {$R > $UR} {
	    set msg [format "$f \[$f - $f\], range $f \[$f\] WIDE" $value $Lx $Ux $R $UR]
	    set reason "WIDE"
	} else {
	    return
	}
	if {$options(-eventmessages)} {
	    set msgcolor $options(-msgcolor)
	    UTF::Message LOG+$msgcolor $utf_msgtag "$key $msg"
	}
	if {$options(-eventcallback) ne ""} {
	    eval [concat $options(-eventcallback) $self $key [list $value $reason $Lx $Ux $UR]]
	}
    }
    method __disableiperfservice {sta} {
	set host [$sta cget -host]
	if {[info exists iperfrestorestates($host)]} {
	    return
	}
	if  {[iperfquerystate $host] eq "start"} {
	    catch {$host sc stop IperfService}
	    set currstate [iperfquerystate $host]
	    if {$currstate eq "stop"} {
		set iperfrestorestates($host) "start"
	    } else {
		UTF::Message WARN $sta "Could not disable iperf service"
	    }
	}
    }
    proc transitions {values} {
	set count 0
	set ix 1
	while {$ix < [llength $values]} {
	    if {([lindex $values [expr {$ix - 1}]] > 0 && [lindex $values $ix] < 0) || ([lindex $values [expr {$ix - 1}]] < 0 && [lindex $values $ix] > 0) || ([lindex $values $ix] == [lindex $values [expr {$ix - 1}]])} {
		incr count
	    }
	    incr ix
	}
	return $count
    }
    proc iperfquerystate {host {retry 3}} {
	if {[catch {$host sc query IperfService} output]} {
	    if {[regexp {The specified service does not exist as an installed service} $output]} {
		return "notservice"
	    } else {
		UTF::Message ERROR $host "Cannot query IperfService: $output"
		return "unknown"
	    }
	} elseif {[regexp {The specified service does not exist as an installed service} $output]} {
	    return "notservice"
	}
	while {$retry} {
	    if {[regexp {\s+STATE\s+: (1|4)\s+(STOPPED|RUNNING)} $output - num state]} {
		switch -exact $state {
		    "RUNNING" {
			return "start"
		    }
		    "STOPPED" {
			return "stop"
		    }
		    default {
			UTF::Message ERROR $utf_msgtag "Program error"
		    }
		}
	    } else {
		incr retry -1
		UTF::Sleep 1.0
	    }
	}
	UTF::Message WARN $host "Could not get stable iperf services state"
    }
    # RJM: Fix this to protect control net from stream traffic
    # iptables -A OUTPUT -d 192.168.0.0/16 -o eth1 -j ACCEPT
    # iptables -A OUTPUT -d 192.168.0.0/16 -j DROP
    method __iptables_install {} {
	set sta $options(-transmitsta)
	if {![$sta hostis Linux] || [$sta ipaddr] eq [$sta cget -lan_ip]} {
	    return
	}
	set dev [$sta cget -device]
	set src [$sta ipaddr]
	set dstip [$self mydstip]

	#
	# iptables rules are applied at the stream instance level and with
	# a STA method but applies across instances and across STAs, i.e.
	# the host level.  Therefore, check to see if this exact rule
	# already exists per another stream using the same rule.
	#
	set host [$sta cget -host]
	# Either sta or dev not known or the rule is already
	# installed by streams so just return now
	if {$sta eq {} || $dev eq {} || [info exists iptablescache($host,$dev,$dstip)]} {
	    return
	}
	if {$options(-api_msgs)} {
	    UTF::Message API-I $utf_msgtag "installing iptables entries"
	}
	if {![catch {$sta rexec $IPTABLESCMD -A OUTPUT -s $src -d $dstip -o $dev -j ACCEPT -m comment --comment \"$STREAMSIPTABLESSTRING\"} res]} {
	    lappend restorecmds($sta) [list $sta rexec $IPTABLESCMD -D OUTPUT -s $src -d $dstip -o $dev -j ACCEPT -m comment --comment \"$STREAMSIPTABLESSTRING\"]
	    if {![catch {$sta rexec $IPTABLESCMD -A OUTPUT -s $src -d $dstip -j DROP -m comment --comment \"$STREAMSIPTABLESSTRING\"} res]} {
		lappend restorecmds($sta) [list $sta rexec $IPTABLESCMD -D OUTPUT -s $src -d $dstip -j DROP -m comment --comment \"$STREAMSIPTABLESSTRING\"]
		lappend restorecmds($sta) [list unset iptablescache($host,$dev,$dstip)]
		set iptablescache($host,$dev,$dstip) 1
	    } else {
		UTF::Message ERROR $utf_msgtag "$IPTABLESCMD : $res"
	    }
	} else {
	    UTF::Message ERROR $utf_msgtag "$IPTABLESCMD : $res"
	}
	if {$options(-api_msgs)} {
	    UTF::Message API-E $utf_msgtag "iptables done"
	}
	return
    }
    #
    #  This is a hammer to kill a remote iperf session.  Find the remote
    #  pid and issue a kill signal to it.
    #
    method __iperf_kill_remote_pid {args} {
	UTF::Getopts {
	    {fid.arg ""}
	    {name.arg ""}
	}
	if {$(fid) ne ""} {
	    set fid $(fid)
	    set device $fid2device($fid)
	} elseif {$(name) ne ""} {
	    set fid "none"
	    set device $(name)
	} else {
	    error "program error"
	}
	if {![$device hostis Linux MacOS]} {
	    UTF::Message WARN $utf_msgtag "Iperf remote kill not supported. ($device,$fid)"
	    return
	}
	if {$options(-api_msgs)} {
	    UTF::Message EXCPT $utf_msgtag "EXCEPTION enter: trying to kill remote iperf for ($device,$fid)"
	}
	if {[catch {$device rexec -x pkill -fx 'iperf .+ -p $options(-dstport)'}]} {
	    UTF::Message EXCPT $utf_msgtag "stream exception due to $device pkill fail"
	    set exceptions($device) "pkill -fx iperf .+ -p $options(-dstport)"
	} else {
	    catch {unset exceptions($device)}
	}
	if {$options(-api_msgs)} {
	    UTF::Message EXCPT $utf_msgtag "EXCEPTION exit: ($device,$fid)"
	}
    }
    method {sniff start} {args} {
	if {[llength $args]} {
	    $self sniff on $args
	} else {
	    $self sniff on
	}
    }
    method {sniff on} {args} {
	UTF::Getopts {
	    {igmp "enable igmp filter"}
	    {mac "enable mac addresses in display"}
	    {ack "enable tcp ack filter"}
	    {arp "only sniff arp traffic"}
	    {period.arg "1" "rate limiter period"}
	    {threshold.arg "6" "duplicate threshold"}
	    {dhcp "enable dhcp filter"}
	    {promiscuous "enable promiscuous mode"}
	}
	if {[catch {$options(-transmitsta) ipaddr} myip] && [::ip::IPv4? $myip]} {
	    set msg "stream src ip not available"
	    UTF::Messsage ERROR $utv_msgtag $msg
	    error $msg
	}
	set dstip [$self mydstip]
	foreach sta [concat $options(-transmitsta) $options(-receivesta) $options(-rfsniffer) $options(-sniffnodes)] {
	    #
	    # Start tcpdump, filtering on src and dest ips
	    #
	    if {![$sta hostis Linux MacOS DHD]} {
		set msg "tcpdump not supported on $sta"
		UTF::Message WARN $utf_msgtag $msg
		continue
	    }
	    if {$sta eq $options(-rfsniffer)} {
		set dev prism0
		set snarf [expr {144 + 96}]
	    } else {
		set dev [$sta cget -device]
		set snarf 128
	    }
	    if {$(mac)} {
		set tcpdumpcmd "/usr/sbin/tcpdump -e -i $dev -s $snarf -tt -nn"
	    } else {
		set tcpdumpcmd "/usr/sbin/tcpdump -i $dev -s $snarf -tt -nn"
	    }
	    if {!$(promiscuous)} {
		set tcpdumpcmd "$tcpdumpcmd -p"
	    }
	    if {!$options(-multicast)} {
		if {$(arp)} {
		    set cmd "$tcpdumpcmd -l arp"
		} elseif {$(dhcp)} {
		    set cmd "$tcpdumpcmd port 67 or port 68"
		} else {
		    set cmd "$tcpdumpcmd -l arp or port 67 or port 68 or \\\\([string tolower $options(-protocol)] and src $myip and dst $dstip and dst port $options(-dstport)\\\\)"
		}
	    } else {
		if {$(igmp)} {
		    set cmd "$tcpdumpcmd -l proto 2"
		} elseif {$(ack)} {
		    set cmd "$tcpdumpcmd -l proto \\\\([string tolower $options(-protocol)] and src $myip and dst $dstip and dst port $options(-dstport)\\\\) and 'tcp[tcpflags] & (tcp-syn|tcp-ack) != 0'"
		} else {
		    set cmd "$tcpdumpcmd -l proto 2 or \\\\([string tolower $options(-protocol)] and src $myip and dst $dstip and dst port $options(-dstport)\\\\)"
		}
	    }
	    if {[catch {eval [concat $sta rpopen $cmd]} fid]} {
		set msg "sniff start failed: $fid"
		UTF::Message ERROR $utf_msgtag $msg
		error $msg
	    } else {
		set sniff_buffs($fid,state) "INIT"
		set sniff_fid2device($fid) "$sta"
		set sniff_buffs($fid,lastline) ""
		set sniff_buffs($fid,lastdisplay) 0
		set sniff_buffs($fid,dupcount) 0
		set sniff_buffs($fid,count) 0
		set sniff_error($fid) 0
	    }
	    #
	    # Setup read handler for the tcpdump
	    #
	    set sniff_buffs(rl,$fid) [UTF::RateLimiter %AUTO% -period $(period) -threshold $(threshold)]
	    fconfigure $fid -blocking 1 -buffering line
	    fileevent $fid readable [mymethod __tcpdump_handler $fid [$sta cget -device]]
	}
	set sniff_buffs(wd) "PENDING"
	set watchdog [after 6000 [list set [myvar sniff_buffs(wd)] "TIMEOUT"]]
	UTF::Message SNIFF $utf_msgtag "Sniff enabled"
	while {1} {
	    vwait [myvar sniff_buffs]
	    set which ""
	    foreach fid [array names sniff_fid2device] {
		if {$sniff_buffs($fid,state) ne "LISTENING"} {
		    append which "$sniff_fid2device($fid) "
		}
	    }
	    if {$which eq ""} {
		after cancel $watchdog
		break
	    } elseif {$sniff_buffs(wd) eq "TIMEOUT"} {
		UTF::Message SNIFF $utf_msgtag "SNIFF TIMEOUT:$which"
		break
	    }
	}
	set sniffstate "ON"
    }
    method {sniff stop} {} {
	$self sniff off
    }
    method {sniff off} {} {
	foreach fid [array names sniff_fid2device] {
	    if {[catch {exec kill -s HUP [pid $fid]} err]} {
		set msg "Kill failed for SNIFF $fid"
		UTF::Message ERROR $utf_msgtag $msg
	    }
	    after 300 {set done 1}
	    vwait done
	    catch {close $fid}
	    unset sniff_fid2device($fid)
	}
	UTF::Message SNIFF $utf_msgtag "Sniff disabled"
	set sniffstate "OFF"
    }
    method {tcptrace stop} {args} {
	if {![info exists tcptrace(fid)]} {
	    return
	}
	UTF::Message INFO $utf_msgtag "Kill signal -HUP sent to tcpdump($tcptrace(fid))"
	if {[catch {exec kill -s HUP [pid $tcptrace(fid)]} err]} {
	    error $err
	}
	if {[info exists tcptrace(fid)]} {
	    set tcptrace(wd) "PENDING"
	    set watchdog [after 1000 [list set [myvar tcptrace(wd)] "TIMEOUT"]]
	    vwait [myvar tcptrace(wd)]
	    catch {after cancel $watchdog}
	    if {$tcptrace(wd) eq "TIMEOUT"} {
		UTF::Message ERROR $utf_msgtag "tcpdump timeout"
		# RJM try the bigger hammer here
		catch {eval $tcptrace(hardkill)}
		set tcptrace(wd) "PENDING"
		set watchdog [after 1000 [list set [myvar tcptrace(wd)] "TIMEOUT"]]
		vwait [myvar tcptrace(wd)]
		catch {after cancel $watchdog}
		if {$tcptrace(wd) eq "TIMEOUT"} {
		    UTF::Message ERROR $utf_msgtag "tcpdump hardkill failed"
		    catch {close $tcptrace(fid)}
		}
		catch {unset tcptrace(fid)}
		catch {unset tcptrace(hardkill)}
	    }
	}
    }
    method {tcptrace xfer} {args} {
	UTF::Getopts {
	    {unzip  "unzip files after transfer"}
	    {timeout.arg "360" "rexec io and command timeout in seconds"}
	}
	if {[catch {llength $tcptrace(pcaps)} l] || !$l} {
	    return
	}
	set res {}
	set asyncids {}
	foreach src $tcptrace(pcaps) {
	    if {[catch {$options(-transmitsta) rexec stat -t /usr/bin/pbzip2}]} {
		set cmd [concat $options(-transmitsta) rexec -async -timeout $(timeout) -Timeout $(timeout) /usr/bin/bzip2 -f /tmp/$src]
	    } else {
		set cmd [concat $options(-transmitsta) rexec -async -timeout $(timeout) -Timeout $(timeout) /usr/bin/pbzip2 -f /tmp/$src]
	    }
	    UTF::Message INFO $utf_msgtag "Async: $cmd"
	    lappend asyncids [eval $cmd]
	}
	foreach asyncid $asyncids {
	    if {[catch {$asyncid close} err]} {
		UTF::Message ERROR $utf_msgtag "Async error: $err"
	    }
	}
	foreach src $tcptrace(pcaps) timestamp $tcptrace(endtimes) {
	    UTF::Message TRACE $utf_msgtag "TCPTrace capture at: $timestamp"
	    set ix 0
	    while {[file exists $tcptrace(dir)/${src}_t${ix}.bz2]} {
		incr ix
	    }
	    if {[catch {eval [concat $options(-transmitsta) copyfrom /tmp/${src}.bz2 $tcptrace(dir)/${src}_t${ix}.pcap.bz2]} err]} {
		error $err
	    }
	    if {[catch {eval [concat $options(-transmitsta) rexec rm -f /tmp/${src}.bz2]} err]} {
		error $err
	    }
	    set cmd "$TCPTRACECMD -lr $tcptrace(dir)/${src}_t${ix}.pcap.bz2"
	    catch {localhost rexec -t 30 $cmd}
	    set cmd "$TCPTRACECMD -lW $tcptrace(dir)/${src}_t${ix}.pcap.bz2"
	    catch {localhost rexec -t 30 $cmd}
	    set cmd "$TCPTRACECMD -R --output_prefix=\"${src}_t${ix}\" $tcptrace(dir)/${src}_t${ix}.pcap.bz2"
	    #	    catch {localhost rexec -t 30 $cmd}
	    if {$(unzip)} {
		catch {localhost rexec -t 30 bunzip2 $tcptrace(dir)/${src}_t${ix}.pcap.bz2}
		lappend res "http://$::UTF::WebServer$tcptrace(dir)/${src}_t${ix}.pcap"
	    } else {
		lappend res "http://$::UTF::WebServer$tcptrace(dir)/${src}_t${ix}.pcap.bz2"
	    }
	}
	set tcptrace(pcaps) {}
	set tcptrace(endtimes) {}
	catch {unset tcptrace(dir)}
	return $res
    }
    method {tcptrace start} {args} {
	UTF::Getopts {
	    {s.arg "128" "snarf length in bytes"}
	}
	if {$options(-protocol) ne "TCP"} {
	    error "protocol must be TCP"
	}
	if {[info exists tcptrace(fid)]} {
	    return
	}
	if {$options(-name) ne ""} {
	    set sname $options(-name)
	} else {
	    set sname [namespace tail $self]
	}
	regsub -all {\.} [$self mydstip] _ ip
	set sname "${sname}_$ip"
	if {![info exists tcptrace(dir)]} {
	    if {[info exists ::tcl_interactive] && $::tcl_interactive} {
		set tcptrace(dir) [file join [exec pwd] pcap]
	    } elseif {[info exists ::UTF::Logdir]} {
		set tcptrace(dir) [file join $::UTF::Logdir pcap]
	    } else {
		error "directory error to store pcap"
	    }
	    if {![file exists $tcptrace(dir)]} {
		if {[catch {file mkdir $tcptrace(dir)} res]} {
		    error "Unable to make directory $tcptrace(dir) $res"
		}
	    } elseif {![file writable $tcptrace(dir)]} {
		error "directory $tcptrace(dir) not writeable"
	    }
	    if {[info exists ::UTF::RecordStack]} {
		set fullindex [join $UTF::RecordStack "_"]
		set tcptrace(dir) [file join $tcptrace(dir) $fullindex]
	    }
	    if {![file exists $tcptrace(dir)]} {
		if {[catch {file mkdir $tcptrace(dir)} res]} {
		    error "Unable to make directory $tcptrace(dir) $res"
		}
	    } elseif {![file writable $tcptrace(dir)]} {
		error "directory $tcptrace(dir) not writeable"
	    }
	    set dest [file join $tcptrace(dir) ${sname}-tx]
	    set tcptrace(ix) 0
	} else {
	    incr tcptrace(ix)
	}
	set tcptrace(src) ${sname}-tx_s$tcptrace(ix)
	set dev [$options(-transmitsta) cget -device]
	set cmd "/usr/sbin/tcpdump -i $dev -p -s $(s) -tt -nn -l arp or [string tolower $options(-protocol)] and host [$self mydstip] and port $options(-dstport) -w /tmp/$tcptrace(src)"
	set tcptrace(hardkill) "$options(-transmitsta) rexec -x pkill -fx '$cmd'"
	if {[catch {eval [concat $options(-transmitsta) rpopen $cmd]} tcptrace(fid)]} {
	    set msg "tcpdump start failed: $tcptrace(fid)"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	}
	#
	# Setup read handler for the tcpdump
	#
	fconfigure $tcptrace(fid) -blocking 0 -buffering line
	fileevent $tcptrace(fid) readable [mymethod __tcptrace_handler $tcptrace(fid)]
    }
    method __tcptrace_handler {fid} {
	set len [gets $fid buf]
	if {[eof $fid]} {
	    unset tcptrace(fid)
	    set tcptrace(wd) "CLOSED"
	    fconfigure $fid -blocking 1
	    if {![catch {close $fid} err]} {
		if {$options(-debug_msgs)} {
		    UTF::Message DEBUG $utf_msgtag "tcpdump $fid closed"
		}
	    } else  {
		set msg "tcpdump close $fid $err"
		UTF::Message ERROR $utf_msgtag $msg
	    }
	    lappend tcptrace(pcaps) $tcptrace(src)
	    lappend tcptrace(endtimes) [clock format [clock seconds] -format "%T"]
	    UTF::Message INFO $utf_msgtag "Trace=$tcptrace(src)"
	} elseif {$len > 0} {
	    UTF::Message DUMP $utf_msgtag $buf
	}
    }
    method liveplot {args} {
	UTF::Getopts {
	    {style.arg "lines" "gnuplot style"}
	    {title.arg "rate" "graph title"}
	    {stat.arg "rate" "stat to plot"}
	}
	if {[catch {open "|/usr/bin/gnuplot -persist" w+} res]} {
	    error $res
	}
	set liveplots($(stat)) $res
	set ::env(DISPLAY) ":1"
	if {[catch {puts $liveplots($(stat)) {set terminal x11}} err]} {
	    close $liveplots($(stat))
	    error $err
	}
	if {[catch {puts $liveplots($(stat)) "set title \"$(title)\""} err]} {
	    close $liveplots($(stat))
	    error $err
	}
	if {[catch {puts $liveplots($(stat)) "set format y \"%0.1s%c\""} err]} {
	    close $liveplots($(stat))
	    error $err
	}
	if {[catch {puts $liveplots($(stat)) "set yrange \[0:*\]"} err]} {
	    close $liveplots($(stat))
	    error $err
	}
	if {[catch {puts $liveplots($(stat)) "plot '-' using 1:2 with $(style)"} err]} {
	    close $liveplots($(stat))
	    error $err
	}
	set s [$self stats -$(stat) -timestamps]
	set xn [lindex $s 1]
	set yn [lindex $s 0]
	set starttime [$self stats -starttime]
	foreach x $xn y $yn {
	    if {[catch {puts $liveplots($(stat)) "[expr {$x - $starttime}] $y"} err]} {
		close $liveplots($(stat))
		error $err
	    }
	}
	if {[catch {puts $liveplots($(stat)) "e"} err]} {
	    close $liveplots($(stat))
	    error $err
	}
	flush $liveplots($(stat))
	close $liveplots($(stat))
    }
    method plot {args} {
	UTF::Getopts {
	    {all ""}
	    {title.arg "" "Title for Graph"}
	    {stat.arg "rate" "stat to plot"}
	    {outputtype.arg "png" "Output type"}
	    {graphsize.arg "1024,768" "Graph size"}
	    {thumbsize.arg "64,32" "Thumbnail size"}
	    {style.arg "lines" "Gnuplot style"}
	    {ylabel.arg "" "Text for ylabel"}
	    {xlabel.arg "" "Text for xlabel"}
	    {xrange.arg "0:" "Default xrange"}
	    {yrange.arg "0:" "Default yrange"}
	    {text.arg "" "report text"}
	    {append ""}
	    {smoothing}
	}
	set statlist $(stat)
	if {$(all) && $options(-protocol) eq "UDP"} {
	    set statlist "rate lost jitter"
	}
	foreach stat $statlist {
	    if {$(text) ne ""} {
		set text $(text)
	    } elseif {$options(-name) eq ""} {
		set text "[namespace tail $self]-$stat"
	    } else {
		set text "$options(-name)-$stat"
	    }
	    if {$options(-tos) != 0} {
		set t " $options(-tos)"
	    } else {
		set t ""
	    }
	    if {$options(-rwin) != -1} {
		set t " readsize=$options(-rwin)"
	    }
	    if {$(title) eq ""} {
		if {$options(-multicast)} {
		    set title "$options(-tx)->$options(-rx) \[MCAST$t\]"
		} else {
		    set title "$options(-tx)->$options(-rx) \[$options(-protocol)$t\]"
		}
	    } else {
		set title $(title)
	    }
	    set p($stat) [UTF::streamgraph %AUTO% -title $title -streams "$self" -stat $stat -reporttext $text -yrange $(yrange) -graphsize $(graphsize) -smoothing $(smoothing)]
	    if {[set events [$self get_userdata]] ne {}} {
		$p($stat) configure -xtics $events
	    }
	    if {$(append)} {
		set r($stat) [$p($stat) plot -append]
	    } else {
		set r($stat) [$p($stat) plot]
	    }
	    $p($stat) destroy
	}
	return $r($stat)
    }
    method incr_dstport {} {
	incr IPERFPORTOFFSET
	set options(-dstport) [expr {$IPERFPORTBASE + $IPERFPORTOFFSET}]
    }
    method restart {args} {
	UTF::Getopts {
	    {newsocket "Restart the stream forcing iperf to use a new socket"}
	}
	$self stop
	if {$(newsocket)} {
	    $self incr_dstport
	}
	$self start
    }
    method __tcpdump_handler {fid dev} {
	if {[eof $fid]} {
	    $sniff_buffs(rl,$fid) destroy
	    if {![catch {close $fid} err]} {
		if {$options(-debug_msgs)} {
		    UTF::Message DEBUG $utf_msgtag "tcpdump $fid closed"
		}
		set sniff_buffs($fid,state) "CLOSED"
	    } else  {
		set msg "tcpdump close $fid $err"
		UTF::Message ERROR $utf_msgtag $msg
		set sniff_buffs($fid,state) "ERROR"
	    }
	    return
	}
	set buf [gets $fid]
	if {[regexp "^listening on $dev" $buf]} {
	    set sniff_buffs($fid,state) "LISTENING"
	    return
	}
	if {[regexp {([0-9]+) packets dropped by kernel} $buf]} {
	    $sniff_buffs(rl,$fid) flush
	    UTF::_Message SNIFF $sniff_fid2device($fid) $buf
	    return
	}
	if {![regexp {^([0-9]+\.[0-9]{6,6})(.*)} $buf - tt line]} {
	    return
	}
	$sniff_buffs(rl,$fid) message $line SNIFF $sniff_fid2device($fid)
	if {$options(-sniff_callback) ne ""} {
	    if {[catch {$options(-sniff_callback) $self $sniff_fid2device($fid) $line} err]} {
		set msg "sniff callback error: $err"
		UTF::Message ERROR $utf_msgtag $msg
	    }
	}
    }
    method __samplelatency {} {
	if {[catch {$options(-transmitsta) ipaddr} myip] && [::ip::IPv4? $myip]} {
	    set msg "stream src ip not available"
	    UTF::Messsage ERROR $utv_msgtag $msg
	    error $msg
	}
	set dstip [$self mydstip]
	if {[llength $latency(fids)]} {
	    $self __samplelatencystop
	}
	array unset latency *
	set latency(fids) {}
	set latency(sum) 0
	set latency(sumcount) 0
	set latency(starttime) [clock clicks -milliseconds]
	# Synchronize the distributed clocks
	if {$options(-clockip) ne ""} {
	    foreach sta [concat $options(-transmitsta) $options(-receivesta)] {
		if {[$sta cget -lan_ip] ne $options(-clockip)} {
		    if {![info exists NTPLOCK($sta)]} {
			set NTPLOCK($sta) 1
			set cmd [concat $sta $rexec_cmd ntpdate -b -p1 $options(-clockip)]
			if {[catch {eval $cmd}]} {
			    UTF::Message WARN $utf_msgtag "$sta failed clock synch"
			}
			unset NTPLOCK($sta)
		    }
		}
	    }
	}
	foreach sta [concat $options(-transmitsta) $options(-receivesta)] {
	    #
	    # Start tcpdump, filtering on src and dest ips
	    #
	    if {![$sta hostis Linux MacOS]} {
		set msg "tcpdump not supported on $sta"
		UTF::Message WARN $utf_msgtag $msg
		continue
	    }
	    set dev [$sta cget -device]
	    set snarf 64
	    if {$options(-multicast)} {
		set cmd "/usr/sbin/tcpdump -i $dev -p -s $snarf -tt -X -nn -l proto 2 or \\\\([string tolower $options(-protocol)] and src $myip and dst $dstip and dst port $options(-dstport)\\\\)"
	    } else {
		set cmd "/usr/sbin/tcpdump -i $dev -p -s $snarf -tt -X -nn -l [string tolower $options(-protocol)] and src $myip and dst $dstip and dst port $options(-dstport)"
	    }
	    if {[catch {eval [concat $sta $rpopen_cmd $cmd]} fid]} {
		set msg "latency start failed: $fid"
		UTF::Message ERROR $utf_msgtag $msg
		error $msg
	    } else {
		lappend latency(fids) $fid
		set latency($fid,state) "START"
	    }
	}
	foreach fid $latency(fids) {
	    #
	    # Setup read handler for the tcpdump
	    #
	    fconfigure $fid -blocking 1 -buffering line
	    fileevent $fid readable [mymethod __tcpdump_latency_handler $fid]
	}
	set latency(settling) 1
	set latency(settlingid) [after $TCPDUMPSETTLE  [list set [myvar latency(settling)] 0]]
    }
    method __tcpdump_latency_handler {fid} {
	# UTF::Message DEBUG $fid "$latency($fid,state)"
	if {[eof $fid]} {
	    # UTF::Message DEBUG $fid "CLOSING"
	    set latency(fids) [lsearch -all -not -inline $latency(fids) $fid]
	    if {![catch {close $fid} err]} {
		if {$options(-debug_msgs)} {
		    UTF::Message DEBUG $utf_msgtag "tcpdump $fid closed"
		}
		set latency($fid,state) "CLOSED"
	    } else  {
		set msg "tcpdump close $fid $err"
		UTF::Message ERROR $utf_msgtag $msg
		set latency($fid,state) "ERROR"
	    }
	    return
	}
	set rc [gets $fid buf]
	if {[regexp {tcpdump} $buf]} {
	    return
	}
	if {$latency(settling)} {
	    return
	}
	switch -exact $latency($fid,state) {
	    "START" {
		if {[regexp {^([0-9]+\.[0-9]{6,6})(.*)} $buf - pkttime]} {
		    set latency($fid,state) "MID"
		    set latency($fid,tmp) $pkttime
		}
	    }
	    "MID" {
		# Make sure this line is from an ip packet
		if {[regexp {^\s*0x0000:  45} $buf]} {
		    set latency($fid,state) "END"
		} else {
		    set latency($fid,state) "START"
		}
	    }
	    "END" {
		set latency($fid,state) "START"
		if {![regexp {^\s*0x0010: } $buf]} {
		    set latency($fid,state) "START"
		    return
		}
		set seqno [lrange $buf 7 8]
		if {![info exists latency($seqno,seq)]} {
		    set latency($seqno,seq) $latency($fid,tmp)
		} else {
		    set latencyvalue [expr {abs($latency($fid,tmp) - $latency($seqno,seq))}]
		    if {$options(-latencymax) ne {} && $latencyvalue > $options(-latencymax)} {
			set latency($fid,state) "START"
			return
		    }
		    if {$LOGGINGLEVEL ne "ANALYZE"} {
			UTF::Message SAMPL $utf_msgtag "latency=[format %.6f $latencyvalue] seqno=[join $seqno {}] t2=$latency($fid,tmp) t1=$latency($seqno,seq)"
		    }
		    set latency(sum) [expr {$latency(sum) + $latencyvalue}]
		    incr latency(sumcount)
		    if {$latency(sumcount) > $options(-latencycount)} {
			set avg [expr {$latency(sum)/$latency(sumcount)}]
			$self __addsample latency [list $avg [UTF::stream clock]]
			if {$LOGGINGLEVEL ne "ANALYZE"} {
			    UTF::Message STAT "$utf_msgtag" "latency avg=[format %.6f $avg]"
			}
			$self __samplelatencystop
		    }
		}
	    }
	    default {
		return
	    }
	}
    }
    method __samplelatencystop {} {
	# UTF::Message DEBUG "" "CALL STOP"
	if {[info exists noreentrant(latency)]} {
	    return
	}
	set noreentrant(latency) 1
	if {[info exists latency(settlingid)]} {
	    after cancel $latency(settlingid)
	}
	if {[info exists latency(afterid)]} {
	    after cancel $latency(afterid)
	}
	if {[llength $latency(fids)]} {
	    foreach fid $latency(fids) {
		set latency($fid,state) "STOP"
		fileevent $fid readable {}
	    }
	    foreach fid $latency(fids) {
		if {[catch {exec kill -s HUP [pid $fid]} err]} {
		    set msg "Kill failed for SNIFF $fid"
		    # set output [$device rexec {ps -ef | grep tcpdump}]
		    UTF::Message ERROR $utf_msgtag $msg
		}
		catch {close $fid}
		set latency(fids) [lsearch -all -not -inline $latency(fids) $fid]
	    }
	}
	if {$options(-latency) && $txstate eq "ON"} {
	    set runtime [expr {[clock clicks -milliseconds] - $latency(starttime)}]
	    set adj_interval [expr {int((1000 * $options(-latencyreportinterval)) - $runtime)}]
	    #
	    #  Set a minimum of 100 ms to reschedule
	    #
	    if {$adj_interval < 100} {
		set adj_interval 100
	    }
	    set latency(afterid) [after $adj_interval [mymethod __samplelatency]]
	    unset noreentrant(latency)
	}
    }
    method linkcheck_reset {} {
	unset linkcheck_flags
	set linkcheck_flags(start) [UTF::stream clock]
	foreach index [concat $options(-transmitsta) $options(-receivesta)] {
	    set linkcheck_flags($index) 0
	}
    }
    method __host_route {cmd} {
	set src [$options(-transmitsta) ipaddr]
	set dstip [$self mydstip]
	if {$options(-multicast)} {
	    set type "multicast"
	} else {
	    set type "unicast"
	}
	if {[$options(-transmitsta) hostis Cygwin WinDhd]} {
	    if {[catch {$options(-tranmitsta) ip route $cmd $dstip mask 255.255.255.255 [$options(-tranmitsta)]} err]} {
		UTF::Message WARN $utf_msgtag "Error during route add $err"
	    }
	} else {
	    set device [$options(-transmitsta) cget -device]
	    set lanpeers [$options(-tx) cget -lanpeer]
	    if {$options(-policyrouting) == "1" || \
		    $options(-initrtt) ne "-1" || \
			$options(-initrttvar) ne "-1" || \
			$options(-initcwnd) ne "-1" || \
			($options(-policyrouting) == "-1" && ([llength $lanpeers] > 1))} {
		set offset [lsearch $lanpeers $options(-transmitsta)]
		set policynumber [expr {int (100 + $offset)}]
		if {$cmd eq "del" && [info exists POLICYROUTEREF(${policynumber})]} {
		    incr POLICYROUTEREF(${policynumber}) -1
		    if {$POLICYROUTEREF(${policynumber}) <= 0} {
			UTF::Message INFO "" "ip rules on $options(-transmitsta) before del"
			catch {$options(-transmitsta) ip rule list}
			catch {$options(-transmitsta) ip route del $dstip table $policynumber}
			catch {$options(-transmitsta) ip rule del from $src table $policynumber}
			catch {$options(-transmitsta) ip route flush table $policynumber}
			UTF::Message INFO "" "ip rules on $options(-transmitsta) after del"
			catch {$options(-transmitsta) ip rule list}
			unset POLICYROUTEREF(${policynumber})
		    }
		}
		if {$cmd ne "del"} {
		    if {![info exists POLICYROUTEREF(${policynumber})]} {
			set POLICYROUTEREF(${policynumber}) 1
			set output [$options(-transmitsta) ip rule list]
			foreach line [split $output "\n"] {
			    if {[regexp "\.+from $src lookup (\[0-9\]+)" $line - $policynumber]} {
				$options(-transmitsta) ip rule del from $src table $policynumber
			    }
			}
			$options(-transmitsta) ip rule add from $src table $policynumber
		    } else {
			incr POLICYROUTEREF(${policynumber})
		    }
		    catch {$options(-transmitsta) ip rule list}
		    set routecmd [list ip route replace ${dstip}/32 dev $device table $policynumber]
		    if {$options(-initcwnd) ne "-1"} {
			lappend routecmd initcwnd [expr  {int ($options(-initcwnd) / 536)}]
		    }
		    if {$options(-initrtt) ne "-1"} {
			lappend routecmd rtt ${options(-initrtt)}
		    }
		    if {$options(-initrttvar) ne "-1"} {
			lappend routecmd rttvar ${options(-initrttvar)}
		    }
		    $options(-transmitsta) rexec $routecmd
		}
		$options(-transmitsta) ip route flush cache
	    } else {
		catch {$options(-transmitsta) ip route $cmd $type ${dstip}/32 dev $device}
	    }
	    if {$options(-multicast)} {
		foreach rx $options(-receivesta) {
		    catch {$rx ip route $cmd $type ${dstip}/32 dev [$rx cget -device]}
		}
	    }
	}
    }
    method delete_host_route {} {
	$self __host_route del
    }
    method install_host_route {} {
	$self __host_route replace
    }
    #
    #  Add a net multicast route into the hosts routing table.  Delete
    #  any previous entries first making sure this route is the only one.
    #
    method install_mcast_default_route {} {
	$options(-transmitsta) ip route replace multicast "224.0.0.0/4" dev [$options(-transmitsta) cget -device]
	foreach rx $options(-receivesta) {
	    $rx ip route replace multicast "224.0.0.0/4" dev [$rx cget -device]
	}
    }
    method activestats_sample {} {
	set t [UTF::stream clock]
	lappend activestats(t) $t
	if {[$options(-tx) hostis Router] || [string equal [$options(-tx) cget -ap] 1]} {
	    catch {$options(-tx) wl counters} activestats(ap,$t)
	    catch {$options(-rx) wl counters} activestats(sta,$t)
	} elseif {[$options(-rx) hostis Router] || [string equal [$options(-rx) cget -ap] 1]} {
	    catch {$options(-rx) wl counters} activestats(sta,$t)
	    catch {$options(-tx) wl counters} activestats(ap,$t)
	}
    }
    method activestats_parse {} {
	set t0 [lindex $activestats(t) end-1]
	set t1 [lindex $activestats(t) end]
	set msg ""
	if {[regexp {txbcnfrm\s+([0-9]+)} $activestats(ap,$t0) - b0] && [regexp {txbcnfrm\s+([0-9]+)} $activestats(ap,$t1) - b1]} {
	    set apb [expr {1.0 * ($b1 - $b0) / ($t1 - $t0)}]
	    set msg "tx=[format %.2f $apb] beacon/s,"
	}
	if {[regexp {rxbeaconmbss\s+([0-9]+)} $activestats(sta,$t0) - b0] && [regexp {rxbeaconmbss\s+([0-9]+)} $activestats(sta,$t1) - b1]} {
	    set stab [expr {1.0 * ($b1 - $b0) / ($t1 - $t0)}]
	    append msg "rx=[format %.2f $stab] beacon/s"
	}
	UTF::Message INFO $utf_msgtag "Beacon ratio=[format %.2f [expr {$stab / $apb}]] \[${msg}\]"
    }
    #
    # Private helper routines
    #
    proc ip2x {ip {validate 0}} {
	set octets [split $ip .]
	if {[llength $octets] != 4} {
	    set octets [lrange [concat $octets 0 0 0] 0 3]set
	}
	if {$validate} {
	    foreach oct $octets {
		if {$oct < 0 || $oct > 255} {
		    return -code error "invalid ip address"
		}
	    }
	}
	binary scan [binary format c4 $octets] H8 x
	return $x
    }
    proc x2ip {hex} {
	set r {}
	set hex "0x$hex"
	set bin [binary format I [expr {$hex}]]
	binary scan $bin c4 octets
	foreach octet $octets {
	    lappend r [expr {$octet & 0xFF}]
	}
	return [join $r .]
    }
    proc ipIncr {addr {value 1}} {
	set a 0x[ip2x $addr]
	set b [expr {$a + $value}]
	set c [format %x $b]
	return [x2ip $c]
    }
    proc grpip2MAC {ipaddr} {
	if {[regexp {2(2[4-9])|(3[0-9])\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})} $ipaddr - ip2 ip3 ip4]} {
	    set mac2 [expr {$ip2 & 127}]
	} else {
	    UTF::Message ERROR $utf_msgtag "$ipaddr is not a valid multicast group ip addr"
	}
	return [format "01.00.5e.%02x.%02x.%02x" $mac2 $ip3 $ip4]
    }
    proc unitscalefactor {units} {
	switch -exact [string toupper $units] {
	    "M" {
		return 1000000.0
	    }
	    "K" {
		return 1000.0
	    }
	    "" {
		return 1
	    }
	    "G" {
		return 1000000000.0
	    }
	    default {
		error "Unknown units $units"
	    }
	}
    }
    method  __numeric2rate {value} {
	if {[string is double $value] && $value < 1000} {
	    set msg "Invalid rate numeric $value must be greater than 1000"
	    UTF::Message ERROR $utf_msgtag $msg
	    error $msg
	} else {
	    set value [rate2numeric $value]
	    # Always use K units
	    return "[expr {int($value / 1000)}]K"
	}
    }
    proc n2rate {value} {
	if {$value > 1000 && $value < 1000000} {
	    return "[format %0.0f [expr {$value / 1000}]]K"
	} elseif {$value > 1000000 && $value < 1000000000} {
	    return "[format %0.0f [expr {$value / 1000000}]]M"
	} elseif {$value > 1000000000 && $value < 1000000000000} {
	    return "[format %0.0f [expr {$value / 1000000000.0}]]G"
	} else {
	    return [expr {int($value)}]
	}
    }
    proc rate2numeric {value} {
	if {$value eq "-1"} {
	    return 1000000
	}
	if {[string is double $value]} {
	    return [format %0.0f $value]
	}
	set scale [unitscalefactor [string index $value end]]
	set numeric [string range $value 0 end-1]
	return [expr {$scale * $numeric}]
    }
}


snit::type UTF::streamplotter {
    option -streamlist -validatemethod __validateisstream
    option -stattypes -default {rate pktcount bytes jitter lost outoforder}
    
    method __validateisstream {option value} {
	foreach stream $value {
	    if {[catch {$stream info type} stype] || $stype ne "::UTF::stream"} {
		error "$stream is not a UTF::stream object"
	    }
	}
    }
    
    constructor {args} {
    }
    
    destructor {
    }
    
    method plotstats {args} {
	UTF::Getopts {
	    {path.arg ""}
	    {streams.arg ""}
	    {plotname.arg ""}
	}
	
	set streams $(streams)
	regsub -all {[/<>]} $(plotname) "-" plotname
	set path $(path)
	
	if {$path eq {}} {
	    set path [pwd]
	}
	if {$streams eq {}} {
	    set streams [::UTF::stream info instances]
	}
	if {$plotname eq {}} {
	    set plotname temp
	}
	
	set strlen [llength $streams]
	if {!$strlen} {
	    UTF::Message WARN $self "Call to plotstats when no stream is created"
	    return {}
	}
	set pstreams {}
	foreach stream $streams {
	    if {[$stream stats -starttime] ne {} } {
		lappend pstreams $stream
	    }
	}
	#
	# Evaluate the least starttime
	# Convert the starttime to Integer
	#
	if {![llength $pstreams]} {
	    UTF::Message WARN $self "Call to plotstats when no stream is started."
	    return {}
	}
	
	set firsttime [[lindex $pstreams 0] stats -starttime]
	foreach stream $pstreams {
	    set curtime [$stream stats -starttime]
	    if {($curtime < $firsttime)} {
		set firsttime $curtime
	    }
	}
	#set firsttime [expr {int($firsttime)}]
	#
	# End of stream initialization
	#
	#
	# file/directory Initialization
	#
	set gpdir $path
	set gpfilename "$gpdir/gphelper"
	set gpfile [open $gpfilename w]
	if {![file exist $gpfilename]} {
	    UTF::Message ERROR "" "Can not create graph files"
	    return -1
	}
	set delfiles $gpfilename
	set returnfiles {}
	set endblock {
	    foreach delfile $delfiles {
		file delete $delfile
	    }
	    unset delfiles
	}
	foreach stattype $options(-stattypes) {
	    set gpfiletxt {}
	    append gpfiletxt "unset size\nset border\nset key\nset autoscale\nset logscale y\nset xtics auto\nset ytics auto\nset output \"$gpdir/$plotname-$stattype.png\"\nset terminal png\nset xlabel \"time\"\nset ylabel \"$stattype\"\nset format y \"%.0s%c\"\nplot"
	    lappend returnfiles "$gpdir/$plotname-$stattype.png"
	    foreach stream $pstreams {
		set streamtype [namespace tail $stream]
		set filename "$gpdir/$plotname-$streamtype-$stattype"
		set datafile [open $filename w]
		puts "$filename"
		if {![file exist $filename]} {
		    eval $endblock
		    UTF::Message ERROR "" "Can not create datafile"
		    return -1
		}
		lappend delfiles $filename
		append gpfiletxt " \"$filename\" using 1:2 with lines title \'$streamtype\',"
		set plotdata [$stream stats -$stattype -timestamps]
		if {[llength $plotdata]} {
		    set values [lindex $plotdata 0]
		    regsub -all { \-0} $values { 0} values
		    regsub -all {\-0 } $values {0 } values
		    set timestamps [lindex $plotdata 1]
		    #
		    #  SO:  Remove below test after more testing
		    #
		    set vlength [llength $values]
		    set tlength [llength $timestamps]
		    if {$vlength != $tlength} {
			UTF::Message ERROR $stream "$stattype number of values $vlength does not match number of timestamps $tlength \n"
		    } else {
			set strdata "# ${filename}\n"
			foreach value $values timestamp $timestamps {
			    append strdata "[expr {$timestamp - $firsttime}] \t $value \n"
			}
			#UTF::Message DATA $stream "$strdata"
			puts $datafile $strdata
		    }
		} else {
		    UTF::Message WARN $self "Call to plotstats when no stats of $stattype are available."
		}
		close $datafile
	    }
	    set gpfiletxt [string trimright $gpfiletxt {,}]
	    regsub -all "set xlabel \"time\"" $gpfiletxt "set xlabel \"time \([clock format [expr {int($firsttime)}]]\)\"" gpfiletxt
	    regsub "set ylabel \"rate\"" $gpfiletxt "set ylabel \"rate (bps)\"" gpfiletxt
	    
	    # Add thumbnail
	    if {[string equal $stattype "lost"] || [string equal $stattype "outoforder"]} {
		append gpfiletxt "\nunset logscale y"
	    }
	    append gpfiletxt "\nunset xtics\nunset ytics\nunset key\nunset xlabel\nunset ylabel\nunset border\nset output \"$gpdir/$plotname-${stattype}_sm.png\"\nset size 0.15,0.08\nreplot"
	    puts $gpfile $gpfiletxt
	}
	close $gpfile
	if {[catch {exec $::UTF::Gnuplot $gpfilename} results]} {
	    UTF::Message WARN $self "GNUPLOT catch message: $results"
	}
	# Trim white space from the thumnails to make them as small
	# as possible.  With a suitably recent gnuplot we may be
	# able to skip this step.
	foreach file $returnfiles {
	    set sm [file rootname $file]_sm[file extension $file]
	    if {[catch {exec /usr/bin/mogrify -transparent white \
			    -colors 3 -trim $sm} \
		     ret]} {
		UTF::Message WARN "mogrify" $ret
	    }
	}
	eval $endblock
	return $returnfiles
    }
}

snit::type UTF::streamgraph {
    typevariable GNUPLOT_COMMAND {}
    typevariable JSDIR "http://www.sj.broadcom.com/projects/hnd_sig_ext4/rmcmahon/gnuplotfiles"
    typemethod config_gnuplot {} {
	if {$GNUPLOT_COMMAND eq {}} {
	    set latestver 0
	    set latestpatch 0
	    set searchpath [list /tools/bin/ /usr/local/bin/ /usr/bin/]
	    foreach searchdir $searchpath {
		set gnuplot [file join $searchdir gnuplot]
		if {[file executable $gnuplot]} {
		    set ret [exec $gnuplot --version]
		    if {![regexp {gnuplot (\d+\.\d+)\spatchlevel\s(\d+)?} $ret -- version patchlevel]} {
			continue
		    }
		    if {($version > $latestver) || ($version == $latestver && $patchlevel > $latestpatch)} {
			set ::env(GNUPLOTVER) "${latestver}.$latestpatch"
			set GNUPLOT_COMMAND $gnuplot
			set ::UTF::Gnuplot $gnuplot
		    }
		}
	    }
	    if {$GNUPLOT_COMMAND eq {}} {
		UTF::Message ERROR "" "$GNUPLOT_COMMAND not executable"
		error "no gnuplot"
	    } else {
		UTF::Message INFO "" "$GNUPLOT_COMMAND [exec $GNUPLOT_COMMAND --version]"
	    }
	}
	return $GNUPLOT_COMMAND
    }
    option -title -default {}
    option -outfile -default {} -readonly true
    option -graphcache -default {} -readonly true
    option -key {}
    option -streams {}
    option -stat -default "rate" -validatemethod __validatestat -configuremethod __configurestat -readonly true
    option -yticsynch -type boolean -default 0
    option -xticsynch -type boolean -default 1
    option -fontsize -default "medium"
    option -outputtype -default "png"
    option -reporttext -default ""
    option -graphsize -default "640,480"
    option -thumbsize -default "64,32"
    option -legendmax -type integer -default 10
    option -showmax -type boolean -default 0
    option -linetype -default "auto"
    option -details -type boolean -default 1
    option -cutoff -default 10
    option -smoothing -type boolean -default 0
    option -xrange -default {}
    option -yrange -default {}
    option -with -default "lines"
    option -xtics -default {}
    option -lmargin -default "0.11"
    option -rssirange -default {}
 
    variable multiplotid 0
    variable dataid -array {}
    variable gphelper
    variable gpdata -array {}
    variable line -array {}
    variable mpid2index -array {}
    variable mpidtics -array {}
    variable dataid2mpid -array {}
    variable sta2mpid -array {}
    variable mpidinfo -array {}
    variable autolt
    variable outfile
    variable timeoffset 0
    variable pngimagename
    variable utf_msgtag {}
    variable compositeflag
    variable smoothing
    variable streams2plot
    
    constructor {args} {
	set pngimagename ""
	# Setup default graphcache directory.
	if {$options(-graphcache) eq ""} {
	    if {[info exists ::tcl_interactive] && $::tcl_interactive} {
		set options(-graphcache) [file join [exec pwd] graphcache]
	    } elseif {[info exists ::UTF::Logdir]} {
		set options(-graphcache) [file join $::UTF::Logdir graphcache]
	    } else {
		error "StreamGraph: Unable to find default for -graphcache.  Please use -graphcache or set UTF::SummaryDir"
	    }
	}
	if {![file exists $options(-graphcache)]} {
	    if {[catch {file mkdir $options(-graphcache)} res]} {
		error "StreamGraph : unable to make directory $options(-graphcache) $res"
	    }
	} elseif {![file writable $options(-graphcache)]} {
	    error "StreamGraph : directory $options(-graphcache) not writeable"
	}
	$self configurelist $args
	set utf_msgtag "[namespace tail $self]"
	UTF::streamgraph config_gnuplot
    }
    destructor {
    }
    method inspect {} {
	parray mpid2index
	parray mpidtics
	parray line
    }
    method __validatestat {option value} {
	set types "rate rtt cwnd jitter latency lost lostper pktcount pktlatency pps outoforder bytes txbytes txrate"
	set value [string tolower $value]
	foreach v $value {
	    if {[lsearch $types $v] == -1} {
		error "value $v for option $option incorrect, use values $types"
	    }
	}
    }
    method __configurestat {option value} {
	set value [string tolower $value]
	if {$options($option) eq $value} {
	    return
	}
	set options($option) $value
    }
    method __add_line {args} {
	UTF::Getopts {
	    {title.arg ""}
	    {dataset.arg ""}
	    {mpid.arg ""}
	    {lt.arg "1"}
	    {lw.arg "2"}
	    {segments.arg ""}
	    {stat.arg "rate"}
	}
	set prevtime ""
	set xvalues [lindex $(dataset) 0]
	set yvalues [lindex $(dataset) 1]
	set xcount [llength $xvalues]
	set ycount [llength $yvalues]
	set diff [expr {abs($xcount - $ycount)}]
	if {$diff} {
	    UTF::Message WARN $utf_msgtag "Sample count inconsistency: x=$xcount , y=$ycount"
	    if {$xcount > $ycount} {
		set xvalues [lrange [lindex $(dataset) 0] 0 end-$diff]
	    } else {
		set yvalues [lrange [lindex $(dataset) 1] 0 end-$diff]
	    }
	}
	if {$smoothing || $options(-smoothing)} {
	    # find step transitions
	    set transitions ""
	    foreach stream $streams2plot {
		set events [$stream events]
		#		UTF::Message DEBUG $stream "$events"
		if {[$stream status]} {
		    lappend events "END [UTF::stream clock]"
		}
		foreach event $events {
		    lappend transitions [format %0.1f [lindex $event 1]]
		}
	    }
	    set transitions [lsort -unique -increasing -real $transitions]
	    set tmp ""
	    set res ""
	    set endix 0
	    set startix 0
	    UTF::Message INFO $utf_msgtag "events: $transitions"
	    for {set ix 1} {$ix < [expr {[llength $transitions] - 1}]} {incr ix} {
		set token [lindex $transitions $ix]
		set endix [expr {[lsearch [lsort -real -increasing [concat $xvalues $token]] $token] - 1}]
		if {[expr {($endix - $startix) < 1}]} {
		    continue
		}
		set tmp [UTF::math::filters::tcl_lowpass -values "[lrange $yvalues $startix $endix]" -cutoff $options(-cutoff)]
		UTF::Message INFO $utf_msgtag "SEG ${startix}:$endix smoothed: $tmp"
		set res [concat $res $tmp]
		set startix [expr {$endix + 1}]
	    }
	    if {[llength $yvalues] > $startix} {
		set tmp [UTF::math::filters::tcl_lowpass -values "[lrange $yvalues $startix end]" -cutoff $options(-cutoff)]
		UTF::Message INFO $utf_msgtag "SEG ${endix}:end smoothed: $tmp"
		set res [concat $res $tmp]
	    }
	    set yvalues $res
	}
	set fid [open $gpdata($(stat)) "a"]
	set indices ""
	fconfigure $fid -buffering full
	foreach x $xvalues y $yvalues {
	    if {$prevtime ne {}  && [llength $(segments)]} {
		if {[test_spans_segment $prevtime $x $(segments)]} {
		    puts -nonewline $fid "\n\n"
		    if {![info exists dataid($(stat))]} {
			set dataid($(stat)) 0
		    }
		    lappend indices "$dataid($(stat))"
		    set line(titles,$dataid($(stat))) ""
		    set line(color,$dataid($(stat))) $(lt)
		    set line(width,$dataid($(stat))) $(lw)
		    incr dataid($(stat)) +1
		}
	    }
	    set prevtime $x
	    set x [expr {$x - $timeoffset}]
	    puts $fid "[expr {$x}] [expr {$y}]"
	    if {![info exist mpidtics(x,min)] || $x < $mpidtics(x,min)} {
		set mpidtics(x,min) $x
		set mpidtics(x,minlog) [expr {$x + $timeoffset}]
	    }
	    if {![info exist mpidtics(x,max)] || $x > $mpidtics(x,max)} {
		set mpidtics(x,max) $x
		set mpidtics(x,maxlog) [expr {$x + $timeoffset}]
	    }
	    if {![info exist mpidtics(y,min)] || $y < $mpidtics(y,min)} {
		set mpidtics(y,min) $y
	    }
	    if {![info exist mpidtics(y,max)] || $y > $mpidtics(y,max)} {
		set mpidtics(y,max) $y
	    }
	    if {![info exist mpidtics(y,min,$(mpid))] || $y < $mpidtics(y,min,$(mpid))} {
		set mpidtics(y,min,$(mpid)) $y
	    }
	    if {![info exist mpidtics(y,max,$(mpid))] || $y > $mpidtics(y,max,$(mpid))} {
		set mpidtics(y,max,$(mpid)) $y
	    }
	}
	puts -nonewline $fid "\n\n"
	close $fid
	if {![info exists dataid($(stat))]} {
	    set dataid($(stat)) 0
	}
	set id $dataid($(stat))
	lappend indices "$dataid($(stat))"

	if {$(mpid) eq {}} {
	    set mpid {mpid#0}
	} else {
	    set mpid $(mpid)
	}
	lappend mpid2index($mpid) $indices
	set line(color,$dataid($(stat))) $(lt)
	set line(width,$dataid($(stat))) $(lw)
	set line(titles,$dataid($(stat))) $(title)
	incr dataid($(stat)) +1
	return
    }
    method __addstreams {streams} {
	set streamlist {}
	set first {}
	# Find the first start time
	foreach stream $streams {
	    set curtime [$stream stats -starttime]
	    if {$curtime ne {}} {
		if {$first eq {} || $curtime < $timeoffset} {
		    set first $stream
		    set timeoffset $curtime
		}
		lappend streamlist $stream
	    } else {
		UTF::Message WARN $utf_msgtag "Ignoring empty $stream"
	    }
	}
	# use first as a proxy for no streams having been started
	# and hence just return with null.
	if {$first eq {} } {
	    return 0
	}
	set statlist $options(-stat)
	set streamlist [concat $first [lsearch -all -not -inline $streamlist $first]]
	foreach stat $statlist {
	    foreach stream $streamlist {
		set restoresetting [$stream cget -api_msgs]
		if {$restoresetting} {
		    set stats_cmd "$stream stats -$stat -timestamps"
		} else {
		    set stats_cmd "$stream stats -$stat -timestamps -silent"
		}
		$stream configure -api_msgs 1
		set data [eval $stats_cmd]
		$stream configure -api_msgs $restoresetting
		if {![llength $data]} {
		    continue
		}
		set onsegments [compute_txonsegments [$stream events] [$stream status -tx] [$stream stats -starttime]]
		set label [$stream label]
		if {[$stream cget -multicast]} {
		    set lt [$self __getnextlt]
		    foreach rxdata $data {
			set sta [lindex $rxdata 0]
			set values [list [lindex $rxdata 2] [lindex $rxdata 1]]
			if {![info exists sta2mpid($sta,$stat)]} {
			    set mpid [$self __multiplot_add $sta]
			    set sta2mpid($sta,$stat) $mpid
			} else {
			    set mpid $sta2mpid($sta,$stat)
			}
			if {[$stream cget -linetype] ne "auto"} {
			    set lt [$stream cget -linetype]
			} else {
			    set lt [$self __getnextlt]
			}
			$self __add_line -dataset $values -title $label -mpid $mpid -lt $lt -segments $onsegments -stat $stat
		    }
		} else {
		    set sta [$stream cget -receivesta]
		    if {$stat eq "pktlatency"} {
			set means {}
			set mins {}
			set maxes {}
			foreach sample [lindex $data 0] {
			    set sample [split $sample /]
			    set mean [lindex $sample 0]
			    set min [lindex $sample 1]
			    set max [lindex $sample 2]
			    if {$mean eq "-nan"} {
				lappend means -0
				lappend mins -0
				lappend maxes -0
			    } else {
				lappend means [expr {$mean / 1000}]
				lappend mins [expr {$min / 1000}]
				lappend maxes [expr {$max/ 1000}]
			    }
			}
			set avgvalues [list [lindex $data 1] $means]
			set minvalues [list [lindex $data 1] $mins]
			set maxvalues [list [lindex $data 1] $maxes]
			if {![llength $avgvalues]} {
			    continue
			}
		    } else {
			set values [list [lindex $data 1] [lindex $data 0]]
			if {![llength $values]} {
			    continue
			}
		    }
		    if {$compositeflag} {
			if {![info exists sta2mpid(composite,$stat)]} {
			    set mpid [$self __multiplot_add]
			    set sta2mpid(composite,$stat) $mpid
			} else {
			    set mpid $sta2mpid(composite,$stat)
			}
		    } else {
			if {![info exists sta2mpid($sta,$stat)]} {
			    set mpid [$self __multiplot_add $sta]
			    set sta2mpid($sta,$stat) $mpid
			} else {
			    set mpid $sta2mpid($sta,$stat)
			}
		    }
		    if {[$stream cget -linetype] ne "auto"} {
			set lt [$stream cget -linetype]
		    } else {
			set lt [$self __getnextlt]
		    }
		    if {$stat eq "pktlatency"} {
			if {!$compositeflag} {
			    $self __add_line -dataset $avgvalues -title "${label}(mean)" -mpid $mpid -lt $lt \
				-segments $onsegments -lw 2 -stat $stat
			    set lt [$self __getnextlt]
			    $self __add_line -dataset $minvalues -title "${label}(min)" -mpid $mpid -lt $lt \
				-segments $onsegments -lw 0.5 -stat $stat
			    set lt [$self __getnextlt]
			    $self __add_line -dataset $maxvalues -title "${label}(max)" -mpid $mpid -lt $lt \
				-segments $onsegments -lw 0.5 -stat $stat
			} else {
			    $self __add_line -dataset $maxvalues -title "${label}(max)" -mpid $mpid -lt $lt \
				-segments $onsegments -lw 1 -stat $stat
			}
		    } else {
			$self __add_line -dataset $values -title $label -mpid $mpid -lt $lt -segments $onsegments -stat $stat
		    }
		}
	    }
	}
	return 1
    }
    method __multiplot_add {{title ""}} {
	set id "mpid#$multiplotid"
	incr multiplotid +1
	set mpidinfo($id) $title
	return $id
    }
    # lt chooses a particular line type:
    # -1=black
    #  1=red 2=grn 3=blue 4=purple
    #  5=aqua 6=brn 7=orange 8=light-brn
    method __getnextlt {} {
	if {![info exist autolt]} {
	    set autolt 1
	} else {
	    incr autolt +1
	    if {$autolt > 8} {
		set autolt 1
	    }
	}
	return $autolt
    }
    method plot {args} {
	UTF::GetKnownopts {
	    {composite ""}
	    {smoothing ""}
	    {append ""}
	}
	if {$(composite)} {
	    set compositeflag 1
	} else {
	    set compositeflag 0
	}
	if {$(smoothing)} {
	    set smoothing 1
	} else {
	    set smoothing 0
	}
	if {$args ne {}} {
	    set overlay [lindex $args 0]
	    foreach data $overlay {
		foreach {timeval attnval} $data {}
		if {![info exists attnmin]} {
		    set attnmin $attnval
		}
		if {![info exists attnmax]} {
		    set attnmax $attnval
		}
		if {$attnval < $attnmin} {
		    set attnmin $attnval
		}
		if {$attnval > $attnmax} {
		    set attnmax $attnval
		}
	    }
	    UTF::Message INFO "overlay" "$overlay"
	}

	# Use as a flag for an existing plot
	set multiplots [array names mpid2index]
	if {[llength $multiplots]} {
	    foreach stat $options(-stat) {
		set stat [string tolower $stat]
		set dataid($stat) 0
	    }
	    unset stat
	    set multiplotid 0
	    array unset line *
	    array unset mpid2index *
	    array unset mpidtics *
	    array unset dataid2mpid *
	    array unset sta2mpid *
	    catch {unset autolt}
	    set pngimagename ""
	}
	if {$options(-streams) eq {}} {
	    set streams2plot [UTF::stream info instances]
	} else {
	    foreach s $options(-streams) {
		if {[catch {$s info type} snittype] || $snittype ne "::UTF::stream"} {
		    # See if exists in UTF::Test namespace
		    set fqn "::UTF::Test::$s"
		    if {[catch {$fqn info type} snittype] || $snittype ne "::UTF::stream"} {
			error "$s not of type UTF::stream"
		    } else {
			lappend streams2plot $fqn
		    }
		} else {
		    lappend streams2plot $s
		}
	    }
	}

	if {[llength $options(-stat)] > 1 && $streams2plot > 1 && !$(composite) } {
	    UTF::Message INFO "" "plot \"${options(-stat)}\" with $streams2plot of streams with composite option"
	    set compositeflag 1
	}

	if {$options(-outfile) eq {}} {
	    set outfile [file join $options(-graphcache) $options(-key)]
	    if {![file exist $outfile]} {
		catch {file mkdir $outfile}
	    }
	    set outfile [file join $outfile [join $options(-stat) "_"]]
	} else {
	    set outfile [file join $options(-graphcache) $options(-outfile)]
	}
	# Create a unique filename for graph data and gpc files
	if {$options(-outfile) eq {}} {
	    set ix 0
	    while {[file exists ${outfile}_$ix.png]} {
		incr ix
	    }
	    set outfile "${outfile}_${ix}"
	}
	#each stat is saved in separate data file
	foreach stat $options(-stat) {
	    set gpdata($stat) "${outfile}_${stat}.data"
	}
	#only one gpc file
	set gphelper "${outfile}.gpc"

	if {[llength $streams2plot]} {
	    UTF::Message INFO $utf_msgtag "Plotting $options(-stat) for $streams2plot"
	    if {![$self __addstreams $streams2plot]} {
		UTF::Message ERROR $self "plot method called without a stream ever started"
		error "plot method called without a stream ever started"
	    }
	    set multiplots [array names mpid2index]
	} else {
	    UTF::Message WARN $utf_msgtag "plot called with nothing to draw"
	    return
	}
	# Open the gnuplot helper file
	set G [open "$gphelper" w]
	fconfigure $G -buffering line
	# Generate the graph title
	if {$options(-title) eq {}} {
	    set title "[string toupper $options(-stat)]\\n[clock format [clock seconds]]"
	} else {
	    set title "$options(-title) \\n[clock format [clock seconds]]"
	}
	if {$smoothing || $options(-smoothing)} {
	    append title " (S:$options(-cutoff))"
	}
	# Add the time stamps in utf log format to assist graph to log analysis
	if {![info exists mpidtics(x,minlog)] || ![info exists mpidtics(x,maxlog)]} {
	    error "$options(-stat) no stats available in log."
	}
	set utf_log_start [clock format [expr {int($mpidtics(x,minlog))}] -format %H:%M:%S]
	set utf_log_end [clock format [expr {int($mpidtics(x,maxlog))}] -format %H:%M:%S]

	# Create the list of plots needed
	set otypes [list png thumbnail]
	if {$options(-outputtype) eq "canvas"} {
	    if {[UTF::GnuplotVersion] < 4.4} {
		UTF::Message ERROR $utf_msgtag "gnuplot version [UTF::GnuplotVersion] doesn't support canvas element"
	    } else {
		set otypes [list canvas png thumbnail]
	    }
	}
	if {$options(-outputtype) eq "svg"} {
	    if {[UTF::GnuplotVersion] < 4.6} {
		UTF::Message ERROR $utf_msgtag "gnuplot version [UTF::GnuplotVersion] doesn't support svg element"
	    } else {
		set otypes [list svg png thumbnail]
	    }
	}
	foreach otype $otypes {
	    puts $G {reset; unset multiplot}
	    switch -exact $otype {
		"canvas" {
		    set out ${outfile}_canvas.html
		    puts $G "set output \"${out}\""
		    puts $G "set terminal canvas standalone mousing size 1024,768 jsdir \"$JSDIR\""
		}
		"png" {
		    set out ${outfile}.png
		    puts $G "set output \"${out}\""
		    if {[UTF::GnuplotVersion] > 4.0} {
			puts $G  "set terminal png size $options(-graphsize)"
		    } else {
			puts $G {set terminal png}
		    }
		}
		"svg" {
		    set out ${outfile}.html
		    puts $G "set output \"${out}\""
		    puts $G "set terminal svg enhanced size $options(-graphsize)"
		}
		"thumbnail" {

		    #if options(-stat) is not a list, only need to have one thumb.png
		    #but this need to be specified early in helper file so that other
		    #attributes can be set properly
		    #if options(-stat) is a list, we need seperate thumb.png for each of the stat.
		    #It is handled in multiplot loop
		    if {[llength $options(-stat)] == 1} {
			set out ${outfile}_${options(-stat)}_thumb.png
			puts $G "set output \"${out}\""
			set tmp [split $options(-thumbsize) ,]
			set x [lindex $tmp 0]
			set yscale [expr {1 + (([llength $multiplots] - 1) * 0.5)}]
			set y [expr {int([lindex $tmp 1] * $yscale)}]
			if {[UTF::GnuplotVersion] > 4.0} {
			    puts $G "set terminal png transparent size ${x},${y} crop"
			} else {
			    puts $G "set terminal png transparent picsize ${x} ${y}"
			}
		    }
		}
	    }
	    UTF::Message INFO $utf_msgtag "Plotting: $out"
	    if {[llength $streams2plot] <= $options(-legendmax) && $otype ne "thumbnail"} {
		#puts $G {set key top right}
		puts $G {set key outside top center horizontal maxcols auto maxrows auto}
		if {[UTF::GnuplotVersion] > 4.4} {
		    puts $G {set key font ',9'}
		}
	    } else {
		puts $G {unset key}
	    }
	    puts $G {set grid}
	    puts $G {set autoscale fix}
	    puts $G {set size 1.0,1.0}
	    puts $G {set origin 0.0,0.0}
	    if {[llength $multiplots] > 1} {
		if {$otype ne "thumbnail"} {
		    #all subplots aligned at same lmargin
		    puts $G  "set lmargin at screen $options(-lmargin)"
		    #multiplot in columnwise and downwards
		    puts $G "set multiplot layout [llength $multiplots],1 columnsfirst downwards scale 1,1 title \"$title\" font \",10\""
		} else {
		    #for multiplot for multiples STAs, need to set thumbnail as multiplot
		    if {[llength $options(-stat)] == 1} {
			puts $G "set multiplot layout 1,[llength $multiplots] scale 1,1"
		    }
		}
	    } else {
		if {$otype ne "thumbnail"} {
		    puts $G "set title \"$title\""
		}
	    }

	    set statindex 0
	    foreach mpid $multiplots {
		if  {[llength $options(-stat)] > 1} {
		    #NEED TO HANDLE MULTIPLE STATS + MULTIPLE STAs WITHOUT "composite"
		    set stattoplot [lindex $options(-stat) $statindex]
		} else {
		    set stattoplot $options(-stat)
		}
		if {$otype eq "thumbnail" && [llength $options(-stat)] > 1} {
		    #generating separate thumbnail files in case of multiplot for multiple stats
		    set out ${outfile}_${stattoplot}_thumb.png
		    puts $G "set output \"${out}\""
		    set tmp [split $options(-thumbsize) ,]
		    set x [lindex $tmp 0]
		    set yscale [expr {1 + (([llength $multiplots] - 1) * 0.5)}]
		    set y [expr {int([lindex $tmp 1] * $yscale)}]
		    if {[UTF::GnuplotVersion] > 4.0} {
			puts $G "set terminal png transparent size ${x},${y} crop"
		    } else {
			puts $G "set terminal png transparent picsize ${x} ${y}"
		    }
		}
		set xmin [expr {int($mpidtics(x,min))}]
		set tmp [expr {int($mpidtics(x,max))}]
		if {[expr {$mpidtics(x,max) - $tmp}] > 0.1} {
		    set xmax [expr {$tmp + 1}]
		} else {
		    set xmax $tmp
		}
		set rangeincr [expr {round(($xmax - $xmin) / 10)}]
		if {$rangeincr <= 0} {
		    set rangeincr 1
		}
		if {$options(-xtics) ne ""} {
		    foreach xtic $options(-xtics) {
			puts $G "set xtics add \(\"[lindex $xtic 0]\" [lindex $xtic 1]\)"
		    }
		}

		if {$options(-xrange) ne ""} {
		    puts $G "set xrange \[$options(-xrange)\]"
		} elseif {$options(-xticsynch)} {
		    # RJM - may need auto_align
		    puts $G "set xrange \[${mpidtics(x,min)}:${mpidtics(x,max)}\]"
		} else {
		    puts $G {set xrange [*:*]}
		}
		if {$options(-yticsynch)} {
		    set ymin $mpidtics(y,min)
		    set ymax $mpidtics(y,max)
		} else {
		    set ymin $mpidtics(y,min,$mpid)
		    set ymax $mpidtics(y,max,$mpid)
		}
		# Add 20% spacing
		set ymin [expr {$ymin - ($ymin *0.2)}]
		set ymax [expr {$ymax + ($ymax *0.2)}]
		# If integers, set space by 5
		if {$ymax > 5} {
		    set ymax [format %0.1f $ymax]
		    set rangeincr [format %0.0f [expr {($ymax - $ymin) / 5.0}]]
		} else {
		    set rangeincr [expr {abs(($ymax - $ymin) / 5.0)}]
		}
		# if ymin is zero, set zero axis to dotted line
		if {$ymin == 0} {
		    puts $G {set xzeroaxis}
		}
		# if all zeros, adjust graph
		if {$ymin == 0 && $ymax == 0} {
		    set ymin -1; set ymax 1; set rangeincr 1; set precision 0;
		} else {
		    if {$options(-yrange) ne ""} {
			set tmp $options(-yrange)
		    } else {
			set tmp [range_auto_align $ymin $ymax $rangeincr]
		    }
		    set ymin [lindex $tmp 0]
		    set ymax [lindex $tmp 1]
		    set rangeincr [lindex $tmp 2]
		    set precision [lindex $tmp 3]
		}
		if {[llength $options(-stat)] > 1} {
		    puts $G "set title \"$stattoplot\""
		}
		set ylabelunits [[lindex $streams2plot 0] stats -$stattoplot -units -silent]
		puts $G "set ylabel \"$ylabelunits\""
		if {$stattoplot ne "lost"} {
		    puts $G "set format y \"%0.1s%c\""
		} else {
		    puts $G {unset format}
		}

		if {[info exists overlay] && [llength $overlay] && $otype ne "thumbnail"} {
		    if {$options(-rssirange) ne ""} {
			set tmp [split $options(-rssirange) :]
			set near [lindex $tmp 0]
			set far [lindex $tmp end]
		    }
		    set slotwidth 5
		    set slots [expr {($attnmax - $attnmin) / $slotwidth}]
 		    if {[expr ($attnmax - $attnmin) % $slotwidth]} {
			incr slots
		    }
		    puts $G "set y2tics"
		    if {[info exists near]} {
			puts $G "set y2label \"Attn dB \ RSSI dBm\""
			puts $G {set grid x y2}
		    } else {
			puts $G "set y2label \"Attn dB\""
		    }
		    for {set kx 0} {$kx < $slots} {incr kx} {
			set y2val [expr {($kx * $slotwidth) + $attnmin}]
			if {[info exists near]} {
			    set y2rssi [expr {$near - ($kx * $slotwidth)}]
			    puts $G "set y2tics add \(\"${y2val}/$y2rssi\" $y2val\)"
			} else {
			    puts $G "set y2tics add \(\"${y2val}\" $y2val\)"
			}
		    }
		}
		if {$ymin ne "" && $ymax ne "" && $rangeincr ne ""} {
		    #		    puts $G "set ytics $ymin,$rangeincr,$ymax"
		}
		if {$options(-yticsynch)} {
		    puts $G "set yrange \[$ymin:$ymax\]"
		} elseif {$options(-yrange) ne ""} {
		    puts $G "set yrange \[$options(-yrange)\]"
		} else {
		    puts $G {set yrange [*:*]}
		}
		if {$otype ne "thumbnail"} {
		    puts $G "unset label"
		    #only plot key for the first subplot (top) for multiple stats multiplot
		    if {[llength $options(-stat)] > 1 && $mpid ne [lindex $multiplots 0]} {
			puts $G {unset key}
		    }
		    #only plot xlabel for the last subplot (bottom)
		    if {$mpid eq [lindex $multiplots end]} {
			puts $G "set xlabel \"time (s)\\n$utf_log_start - $utf_log_end\""
		    } else {
			puts $G {set format x ""}
		    }
		}
		# plot the lines (and/or segments) in the sub graph
		set plottxt "plot "
		foreach lineid $mpid2index($mpid) {
		    foreach segment $lineid {
			if {$otype ne "thumbnail"} {
			    append plottxt "\"$gpdata($stattoplot)\" index $segment using 1:2 with $options(-with) lt $line(color,$segment) lw $line(width,$segment) title \"$line(titles,$segment)\", "
			} else {
			    append plottxt "\"$gpdata($stattoplot)\" index $segment using 1:2 with $options(-with) lt $line(color,$segment), "
			}
		    }
		}
		set plottxt [string trimright $plottxt {, }]
		if {$otype eq "thumbnail"} {
		    puts $G {unset xtics; unset ytics; unset key; unset xlabel; unset ylabel; unset border; unset grid; unset yzeroaxis; unset xzeroaxis; unset title; set lmargin 0; set rmargin 0; set tmargin 0; set bmargin 0}
		}
		# Add any overlays
		if {[info exists overlay] && [llength $overlay] && $otype ne "thumbnail"} {
		    append plottxt ", \"-\" using 1:2 axes x1y2 notitle with lines lc -1 lt 0 lw 2\n"
		    foreach data $overlay {
			append plottxt  "[lindex $data 0] [lindex $data 1]\n"
		    }
		    append plottxt {e}
		}
		puts $G $plottxt
		incr statindex
	    }
	}
	close $G
	# Run gnuplot on plot file.  Catch is needed because gnuplot
	# often writes to stderr.  Report any output even if we think
	# it was ok.
	catch {exec $GNUPLOT_COMMAND $gphelper} results
	UTF::Message WARN "gnuplot" $results
	foreach stat $options(-stat) {
	    if {[file exists ${outfile}_${stat}_thumb.png] && \
		    [UTF::GnuplotVersion] <= 4.0} {
		catch {exec /usr/bin/mogrify -transparent white -depth 24 -colors 6 -trim  ${outfile}_${stat}_thumb.png} results
		UTF::Message WARN "mogrify" $results
	    }
	    set fd [open "${outfile}_${stat}_thumb.png"]
	    fconfigure $fd -translation binary
	    set data($stat) [base64::encode -maxlen 0 [read $fd]]
	    close $fd
	}
	if {$options(-reporttext) eq {}} {
	    set text4htmllink $options(-stat)
	} else {
	    set text4htmllink $options(-reporttext)
	}
	if {$options(-showmax)} {
	    append text4htmllink " [format %0.f [expr {round ($mpidtics(y,min) / 1000000)}]]-[format %0.f [expr {round ($mpidtics(y,max) / 1000000)}]]M"
	}
	unset streams2plot
	set res "html:"
	if {[file exists ${outfile}_canvas.html]} {
	    foreach stat $options(-stat) {
		if {$options(-stat) ne $text4htmllink} {
		    set s $text4htmllink
		} else {
		    set s $stat
		}
		append res "<!--\[if IE\]><img src=\"${outfile}_${stat}_thumb.png\" alt=\"url\" /><a href=\"${outfile}.png\">$st</a><!\[endif\]-->"
		append res "<!\[if !IE\]><img src=\"data:image/png;base64,$data($stat)\" alt=\"data\" /><a href=\"${outfile}_canvas.html\">$s</a>\<!\[endif\]>"
	    }
	} elseif {[file exists ${outfile}.html]} {
	    foreach stat $options(-stat) {
		if {$options(-stat) ne $text4htmllink} {
		    set s $text4htmllink
		} else {
		    set s $stat
		}
		append res "<!--\[if IE\]><img src=\"${outfile}_${stat}_thumb.png\" alt=\"url\" /><a href=\"${outfile}.html\">$st</a><!\[endif\]-->"
		append res "<!\[if !IE\]><img src=\"data:image/png;base64,$data($stat)\" alt=\"data\" /><a href=\"${outfile}.html\">$s</a>\<!\[endif\]>"
	    }
	} else {
	    foreach stat $options(-stat) {
		if {$options(-stat) ne $text4htmllink} {
		    set s $text4htmllink
		} else {
		    set s $stat
		}
		append res "<!--\[if IE\]><a href=\"${outfile}.png\"><img src=\"${outfile}_${stat}_thumb.png\" alt=\"url\" /></a><a href=\"${outfile}.png\">$s</a><!\[endif\]-->"
		append res "<!\[if !IE\]><a href=\"${outfile}.png\"><img src=\"data:image/png;base64,$data($stat)\" alt=\"data\" /></a><a href=\"${outfile}.png\">$s</a>\<!\[endif\]>"
	    }
	}
	if {!$(append)} {
	    return $res
	} else {
	    return " [string range $res 5 end]"
	}
    }
    method get_imagename {} {
	return $pngimagename
    }
    proc test_spans_segment {timea timeb segmentlist} {
	foreach segment $segmentlist {
	    if {(($timeb  >= [lindex $segment 0]) && ($timeb  <= [lindex $segment 1])) && !(($timea  >= [lindex $segment 0]) && ($timea  <= [lindex $segment 1]))} {
		return 1
	    }
	}
	return 0
    }
    proc compute_txonsegments {onoffevents initstate {start ""}} {
	set onsegments ""
	if {$initstate} {
	    set state "ON"
	    set timestamp $start
	} else {
	    set state "OFF"
	}
	foreach x $onoffevents {
	    set newstate [lindex $x 0]
	    set newtimestamp [lindex $x 1]
	    switch -exact $state {
		"OFF" {
		    if {$newstate eq "TXON"} {
			set timestamp $newtimestamp
			set state "ON"
		    }
		}
		"ON" {
		    if {$newstate eq "TXOFF"} {
			lappend onsegments [list $timestamp $newtimestamp]
			set state "OFF"
		    }
		}
	    }
	}
	if {$state eq "ON"} {
	    lappend onsegments [list $timestamp [UTF::stream clock]]
	}
	return $onsegments
    }
    proc precision_detect {min max increment} {
	if {$max < 1} {
	    return 0
	}
	set prev $min
	set precision 0
	set a(1) 0
	set a(2) 1
	set a(0) 2
	while {1} {
	    set next [expr {$prev + $increment}]
	    set significand_a [string range $prev 0 $a([expr {[string length $prev] % 3}])]
	    set significand_b [string range $next 0 $a([expr {[string length $next] % 3}])]
	    if {$significand_a == $significand_b && \
		    ([string index $prev $precision] == [string index $next $precision])} {
		incr precision +1
		set prev $min
	    } elseif {$next >= $max} {
		break
	    } else {
		set prev $next
	    }
	    if {$precision > [string length $max]} {
		set precision [expr {[string length $max] - 1}]
		break
	    }
	}
	return [expr {$precision + 1}]
    }
    proc range_auto_align {min max increment} {
	if {$max < 1} {
	    return 0
	}
	return [list $min $max $increment [precision_detect $min $max $increment]]
    }
}

snit::type UTF::StreamStatAggregate {
    option -range -type integer -default -1
    option -stat -default "rate" -readonly true
    option -period -default 1.0 -readonly true
    option -logging -type boolean -default true
    option -linetype -type integer -default 1
    option -streams -default ""
    variable samples -array {}
    variable afterid
    variable timestamps {}
    variable utf_msgtag
    variable eventtimes -array {}
    variable state
    constructor {args} {
	$self configurelist $args
	if {$options(-stat) eq ""} {
	    $self configure -stat [namespace tail $self]
	}
	set utf_msgtag [namespace tail $self]
	if {$options(-range) eq "-1"} {
	    if {$options(-period) >= 1} {
		set options(-range) 1
	    } elseif {$options(-period) <= 0.1} {
		set options(-range) 4
	    } else {
		set options(-range) 2
	    }
	}
	# Check to see if autostart is needed now
	if {$options(-streams) eq ""} {
	    set streams [UTF::stream info instances]
	} else {
	    set streams $options(-streams)
	}
	set state INIT
	foreach stream $streams {
	    if {[$stream status -silent]} {
		set state STARTING
		set afterid [after idle [mymethod start]]
		break
	    }
	}
    }
    destructor {
	$self stop
    }
    method inspect {} {
	catch {puts "afterid=$afterid"}
	if {[array exists samples]} {
	    parray samples
	}
    }
    method start {} {
	if {$state eq "STARTING"} {
	    catch {after cancel $afterid}
	    catch {unset afterid}
	    catch {UTF::stream allstreams -linkcheck -now}
	    set state RUNNING
	    set afterid [after idle [mymethod __dointerpolate]]
	} elseif {![info exists afterid]} {
	    set afterid [after [expr {int($options(-period) * 1000)}] [mymethod __dointerpolate]]
	}
	return
    }
    method stop {} {
	if {[info exists afterid]} {
	    catch {after cancel $afterid}
	    unset afterid
	}
	set state STOPPED
    }
    method clear {} {
	array unset samples  *
	array unset eventtimes *
	set timestamps {}
    }
    method get {{which ""}} {
	if {$which ne ""} {
	    return [list $samples($which) $timestamps]
	} else {
	    return [list [array get samples] $timestamps]
	}
    }
    method stats {args} {
	UTF::GetKnownopts {
	    {meanminmax "mean minimum maximum format"}
	    {h "human readable format"}
	}
	set res {}
	if {[info exists samples(All)]} {
	    if {$(meanminmax)} {
		set res [UTF::MeanMinMax $samples(All)]
	    } else {
		set res $samples(All)
	    }
	    if {$(h)} {
		set res [UTF::stream hformat $res]
	    }
	}
	return $res
    }
    method plot {args} {
	UTF::GetKnownopts {
	    {title.arg "" "Title for Graph"}
	    {outputtype.arg "png" "Output type"}
	    {graphsize.arg "1024,768" "Graph size"}
	    {thumbsize.arg "64,32" "Thumbnail size"}
	    {htmltxt.arg "" "Text for HTML link"}
	    {style.arg "lines" "Gnuplot style"}
	    {ylabel.arg "" "Text for ylabel"}
	    {xlabel.arg "" "Text for xlabel"}
	    {xrange.arg "0:" "Default xrange"}
	    {yrange.arg "0:" "Default yrange"}
	    {append "return an appendable url"}
	    {smoothing "Run data against a low pass filter"}
	    {pointtype.arg "1" "Gnuplot point type"}
	    {linetype.arg "1" "Gnuplot line type"}
	    {precision.arg "0" "Precision of x-axis (or decimal places)"}
	    {yprecision.arg "0" "Precision of y-number (or decimal places)"}
	}
	if {![info exists samples(All)]} {
	    error "Plot method called with no aggregate samples"
	}
	set GNUPLOT_COMMAND [::UTF::streamgraph config_gnuplot]
	set otype $(outputtype)
	if {[info exists ::tcl_interactive] && $::tcl_interactive} {
	    set graphcache [file join [exec pwd] graphcache]
	} elseif {[info exists ::UTF::Logdir]} {
	    set graphcache [file join $::UTF::Logdir graphcache]
	} else {
	    error "Graph: Unable to find directory for graphcache."
	}
	if {![file exists $graphcache]} {
	    if {[catch {file mkdir $graphcache} res]} {
		error "Graph : unable to make directory $graphcache $res"
	    }
	} elseif {![file writable $graphcache]} {
	    error "Graph : directory $graphcache not writeable"
	}
	set ix 0
	while {1} {
	    set outfile [file join $graphcache "${utf_msgtag}_sum"]
	    if {[file exists ${outfile}_$ix.$otype]} {
		incr ix
	    } else {
		set datafile "${outfile}_$ix.data"
		set out ${outfile}_$ix.$otype
		break
	    }
	}
	set fid [open $datafile w]
	if {![file exist $datafile]} {
	    set msg "Cannot create gnu plot data file"
	    UTF::Message ERROR "" $msg
	    error $msg
	} elseif {![info exists timestamps]} {
	    error "no received data"
	} else {
	    foreach indice [concat [lsearch -all -not -inline [array names samples] All] All] {
		if {$(smoothing)} {
		    set samples($indice) [UTF::math::filters::tcl_lowpass -values "[lrange $yvalues $startix $endix]" -cutoff 5]
		}
		catch {unset found}
		set kx 0
		if {$indice ne "All"} {
		    foreach x $timestamps {
			if {$x < $eventtimes(start,$indice)} {
			    incr kx
			} else {
			    break
			}
		    }
		}
		set firsttimestamp [lindex $timestamps 0]
		foreach x [lrange $timestamps $kx end] y $samples($indice) {
		    if {$y eq {}} {
			break
		    }
		    set x [expr {$x - $firsttimestamp}]
		    puts $fid "$x  $y"
		    set found 1
		    if {![info exists ymax] || $y > $ymax} {
			set ymax $y
		    }
		    if {![info exists ymin] || $y < $ymin} {
			set ymin $y
		    }
		}
		if {[info exists found]} {
		    puts $fid "\n"
		}
	    }
	    close $fid
	}
	set ymaxr [expr {$ymax + (($ymax - $ymin) * 0.1)}]
	if {$ymin > 0} {
	    set tmp [expr {$ymin - (($ymax - $ymin) * 0.1)}]
	    if {$tmp > 0} {
		set ymin $tmp
	    }
	}
	set G  [open ${outfile}_$ix.gpc w]
	puts $G "set output \"${out}\""
	if {[UTF::GnuplotVersion] > 4.0} {
	    puts $G  "set terminal png size $(graphsize)"
	} else {
	    set tmp [split $(graphsize) ,]
	    set x [lindex $tmp 0]
	    set y [lindex $tmp 1]
	    puts $G "set terminal png picsize ${x} ${y}"
	}
	if {$(title) ne ""} {
	    set title "$(title)\\nPeriod=${options(-period)}s MMARange=$options(-range)"
	} else {
	    set title "Aggregate Graph: $utf_msgtag\\nPeriod=$options(-period)s MMARange=$options(-range)"
	}
	set xlabel "[clock format [expr {int([lindex $timestamps 0])}] -format %T]-[clock format [expr {int([lindex $timestamps end])}] -format %T]\\n[clock format [clock seconds]]"
	puts $G {set key top right}
	puts $G {set grid}
	puts $G "set title \"$title\""
	puts $G "set xlabel \"$xlabel\""
	if {$(ylabel) ne ""} {
	    puts $G "set ylabel \"$(ylabel)\""
	}
	puts $G "set format x \"%.${(precision)}s%c\""
	puts $G "set yrange \[$(yrange)\]"
	puts $G "set format y \"%.${(yprecision)}s%c\""
	puts $G "set xrange \[$(xrange)\]"
	set index 0
	set lw 1;
	if {$options(-linetype) eq "auto" || $options(-linetype) eq "1"} {
	    set lt 2
	} else {
	    set lt $options(-linetype)
	}
	set plottxt "plot "
	foreach indice [concat [lsearch -all -not -inline [array names samples] All] All] {
	    if {$indice eq "All"} {
		set lt 1; set lw 2
	    } else {
		set lw 1
	    }
	    append plottxt "\"$datafile\" index $index using 1:2 with lines linetype $lt linewidth $lw title \"$indice\", "
	    incr index
	    incr lt
	    if {$lt > 8} {
		set lt 2
	    }
	}
	set plottxt [string trimright $plottxt {, }]
	puts $G "set multiplot"
	puts $G "set origin 0,0"
	puts $G "$plottxt"
	puts $G "unset multiplot"
	puts $G "set output \"${outfile}_${ix}_thumb.png\""
	if {[UTF::GnuplotVersion] > 4.0} {
	    puts $G "set terminal png transparent size $(thumbsize) crop"
	} else {
	    set tmp [split $(thumbsize) ,]
	    set x [lindex $tmp 0]
	    set y [lindex $tmp 1]
	    puts $G "set terminal png transparent picsize ${x} ${y}"
	}
	puts $G {unset xtics; unset ytics; unset key; unset xlabel; unset ylabel; unset border; unset grid; unset yzeroaxis; unset xzeroaxis; unset title; set lmargin 0; set rmargin 0; set tmargin 0; set bmargin 0;}
	set index 0
	set lw 1;
	if {$options(-linetype) eq "auto" || $options(-linetype) eq "1"} {
	    set lt 2
	} else {
	    set lt $options(-linetype)
	}
	set plottxt "plot "
	foreach indice [concat [lsearch -all -not -inline [array names samples] All] All] {
	    if {$indice eq "All"} {
		set lt 1; set lw 2
	    } else {
		set lw 1
	    }
	    append plottxt "\"$datafile\" index $index using 1:2 with lines linetype $lt linewidth $lw title \"$indice\", "
	    incr index
	    incr lt
	    if {$lt > 8} {
		set lt 2
	    }
	}
	set plottxt [string trimright $plottxt {, }]
	puts $G "$plottxt"
	close $G
	if {[catch {exec $GNUPLOT_COMMAND ${outfile}_$ix.gpc} results]} {
	    UTF::Message WARN "" "GNUPLOT catch message: $results"
	}
	UTF::Message INFO "" "Graph done: $out"
	set fd [open "${outfile}_${ix}_thumb.png"]
	fconfigure $fd -translation binary
	set thumbdata [base64::encode -maxlen 0 [read $fd]]
	close $fd
	if {$(htmltxt) eq ""} {
	    set text4htmllink "Sum"
	} else {
	    set text4htmllink $(htmltxt)
	}
	set res "html:<!--\[if IE\]><img src=\"${outfile}_${ix}.${otype}\" alt=\"url\" /><a href=\"${outfile}_${ix}.$otype\">$text4htmllink</a><!\[endif\]--><!\[if !IE\]><img src=\"data:image/png;base64,$thumbdata\" alt=\"data\" /><a href=\"${outfile}_${ix}.$otype\">$text4htmllink</a>\<!\[endif\]>"
	if {$(append)} {
	    return " [string range $res 5 end]"
	} else {
	    return $res
	}
    }
    method __dointerpolate {} {
	set sum 0
	set now [UTF::stream clock]
	if {![info exists eventtimes(start,All)]} {
	    set eventtimes(start,All) $now
	}
	if {$options(-streams) eq ""} {
	    set streams [UTF::stream info instances]
	} else {
	    set streams $options(-streams)
	}
	foreach stream $streams {
	    set STARx "[$stream cget -tx]:[$stream cget -rx]:[$stream cget -dstport]"
	    if {![$stream status -silent]} {
		if {[info exists eventtimes(start,$STARx)]} {
		    set eventtimes(stop,$STARx) $now
		}
		continue
	    }
	    if {[catch {$stream stats -$options(-stat) -seconds -silent} tmp]} {
		continue
	    }
	    if {![info exists eventtimes(start,$STARx)]} {
		set eventtimes(start,$STARx) $now
	    }
	    if {[llength $tmp]} {
		set k($stream) $tmp
	    }
	}
	set msg {}
	if {[array exists k]} {
	    # Compute the modified moving average of last n samples
	    foreach stream [array names k] {
		set values [lrange $k($stream) end-[expr {$options(-range) - 1}] end]
		# puts "$stream val: $values"
		if {[llength $values] == 1} {
		    set mma $values
		} elseif {[llength $values] > 1} {
		    set mma [UTF::math::modified_moving_average $values]
		} else {
		    set mma -0.0
		}
		if {[catch {expr {$mma + $sums(All)}} sums(All)]} {
		    set sums(All) $mma
		}
		set STARx "[$stream cget -tx]:[$stream cget -rx]:[$stream cget -dstport]"
		if {[catch {expr {$mma + $sums($STARx)}} sums($STARx)]} {
		    set sums($STARx) $mma
		}
	    }
	    if {[info exists sums(All)]} {
		lappend samples(All) $sums(All)
	    } else {
		lappend samples(All) 0
		set sums(All) 0
	    }
	    set msg "ALL:[UTF::stream hformat $sums(All)] "
	    foreach index [lsearch -all -not -inline [array names sums] All] {
		lappend samples($index) $sums($index)
		 append msg "${index}:[UTF::stream hformat $sums($index)] "
	    }
	    lappend timestamps $now
	    if {$options(-logging) } {
		UTF::_Message STATS $utf_msgtag $msg
	    }
	}
	set afterid [after [expr {int($options(-period) * 1000)}] [mymethod __dointerpolate]]
    }
}

if {$::tcl_interactive} {
    #
    # Create a default plotter object and setup the exit callback
    # when in interactive mode
    #
    UTF::stream logginglevel "ANALYZE"
    UTF::Message INFO {} "LOGGINGLEVEL set to ANALYZE"
    set defaultplotter [::UTF::streamplotter create %AUTO%]
    UTF::stream setdefaultplotter $defaultplotter
    UTF::Message INFO {} "Default streams plotter $defaultplotter instantiated"
    if {![catch {package present TclReadLine}]} {
	TclReadLine::addExitHandler ::UTF::stream exitstreams
    } else {
	lappend ::__install_interactive_exit_callbacks [list ::UTF::stream exitstreams]
    }
    unset defaultplotter
}
