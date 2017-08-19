#!/bin/env utf
# -*-tcl-*-

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 5fda78594384bf0872bb773317c7e856086f2b12 $
# $Copyright Broadcom Corporation$
#

namespace eval UTF {}; # Used by pkg_mkindex
package provide UTF::Multiperf 2.0

package require snit
package require UTF::doc

UTF::doc {
    # [manpage_begin UTF::Multiperf n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Multiperf TCP/UDP support}]
    # [copyright {2011 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]
    # [list_begin definitions]

    # [call [cmd UTF::Multiperf] [arg PAIRS]
    #       [lb][option -noping][rb]
    #       [lb][option -means][rb]
    #       [lb][option -i] [arg interval][rb]
    #       [lb][option -l] [arg length][rb]
    #       [lb][option -u][rb]
    #       [lb][option -p] [arg port][rb]
    #       [lb][option -t] [arg timeout][rb]
    #       [lb][option -w] [arg window][rb]
    #       [lb][arg {args ...}][rb]]

    # Runs one or more iperf throughput tests and returns a list of
    # agregate totals, one entry per intermediate iperf result.  Eg,
    # if [cmd UTF::Multiperf] arguments [option -i] [arg 2] [option
    # -t] [arg 10] are used, there will be 5 results returned, one for
    # each 2-second sample.[para]

    # NB: If you want error checking done on the throughput results,
    # then use controlchart.test or write your own results processing
    # routine. This routine does no validation of results and may not
    # return the expected number of samples.

    # [list_begin options]

    # [opt_def [arg PAIRS]]

    # An even length list of source and sink STA objects.

    # [opt_def [option -noping]]

    # Skip pretest ping check

    # [opt_def [option -pingmax] [arg num]]

    # Max number of pings to verify connectivity.

    # [opt_def [option -w] [arg window]]

    # Specify TCP Window size.  This is interpreted differently
    # according to each interface's host type.  On Linux and MacOS
    # systems, [method tcptune] is used to configure the TCP/IP stack.
    # On Windows, the option is passed on to the [cmd iperf]
    # commandline.  Default is OS dependent.

    # [opt_def [option -i] [arg interval]]

    # Seconds between periodic bandwidth reports. Default: 0.5

    # [opt_def [option -l] [arg length]]

    # Length of datagram. Default: 1492

    # [opt_def [option -b] [arg bandwidth]]

    # Desired bandwidth. non-zero implies UDP, Default: 0 implies TCP.
    # For multi-stream testing, -b can either be a single element,
    # which will be divided equally among the streams, or it can be a
    # list of rates corresponding to the number of streams in the
    # test.

    # [opt_def [option -u]]

    # Use UDP. If b=0, then b defaults to 1mbps.

    # [opt_def [option -p] [arg port]]

    # Port number. Default: 5001

    # [opt_def [option -t] [arg time]]

    # Time in seconds to transmit for. Default: 10

    # [opt_def [arg {args ...}]]

    # Additional [arg arg] are passed on to each [cmd iperf]
    # commandline.

    # [list_end]
}

namespace eval UTF::Multiperf {
    set newport 5001

    proc ToS {t} {
	switch $t {
	    BE {
		return "0x00"
	    }
	    BK {
		return "0x20"
	    }
	    VI {
		return "0x80"
	    }
	    VO {
		return "0xc0"
	    }
	    default {
		if {[string isnum $t]} {
		    return $t
		} else {
		    error "Illegal ToS: $t"
		}
	    }
	}
    }
}

proc UTF::Multiperf {PAIRS args} {
    UTF::Message DBG "" "Multiperf [list $PAIRS] $args"

    if {[info exists UTF::MultiperfPortCycling]} {
	global UTF::Multiperf::newport
    } else {
	set newport 5001
    }

    UTF::GetKnownopts [subst {
	{nolanx "No LAN expansion - use only primary endpoint"}
	{noping "Skip pretest ping check"}
	{pingmax.arg "10" "Max number of pings to verify connectivity"}
	{means "Report mean values, instead of intermediate datapoints"}
	{datagrams.arg "" "Variable to return count of sent datagrams"}
	{comp.arg "" "Variable to return component results"}
	{txrate.arg "" "Variable to return reported tx rate for UDP"}
	{i.arg ".5" "report interval"}
	{l.arg "" "length of datagram"}
	{rxl.arg "" "length of read buffer"}
	{S.arg "" "IP type-of-service"}
	{b.arg "0" "bandwidth - non-zero implies UDP"}
	{u "UDP (equivalent to -b 1m)"}
	{p.arg "$newport" "port number"}
	{t.arg 10 "Timeout"}
	{w.arg "" "Window size"}
	{e "Enhanced stats"}
	{P.arg "1" "Parallel Threads"}
	{callback.arg "" "Callback to issue after iperf has started"}
	{kpps "Repord KPPS instead of throughput"}
    }]

    if {[info exists ::UTF::MultiperfE]} {
	set (e) 1
    }

    if {$(comp) ne ""} {
	upvar $(comp) components
	set components {}
    }

    if {$(datagrams) ne ""} {
	upvar $(datagrams) datagrams
	set datagrams 0
    }

    if {$(txrate) ne ""} {
	upvar $(txrate) txrate
    }
    set txrate 0

    if {$(pingmax) == 0} {
	set (noping) 1
    }

    # expected number of samples
    set expected [expr {int(($(t)+0.0)/$(i))}]

    # newport is incremented whenever a new iperf server is started.
    # Connections using an iperf daemon continue to use the orinal
    # port (p).
    set newport $(p)

    # Mimic iperf args:
    # -b x enables UDP
    # -u is equivalent to -b 1m
    set tcp 0
    if {$(b) eq "0"} {
	if {$(u)} {
	    set $(b) "1m"
	} else {
	    set tcp 1
	}
    }
    set pl [expr {[llength $PAIRS] / 2}]
    set bl [llength $(b)]
    if {$bl == 1} {
	if {![regexp {([\d\.]+)(.*)} $(b) - bnum radix]} {
	    error "-b format not recognised"
	}
	set bdiv [expr {(0.0+$bnum) / $pl}]
	set (b) ""
	for {set i 0} {$i < $pl} {incr i} {
	    lappend (b) $bdiv$radix
	}
    } elseif {$bl ne $pl} {
	error "-b must contain one rate, or the same number of rates as test pairs"
    }

    if {$(S) eq {}} {
	set (S) [list {}]
    }
    set ToSl [llength $(S)]
    if {$ToSl == 1} {
	set ToS0 [lindex $(S) 0]
	for {set i 1} {$i < $pl} {incr i} {
	    lappend (S) $ToS0
	}
    } elseif {$ToSl ne $pl} {
	error "-S must contain one value, or the same number of values as test pairs"
    }

    if {$tcp && $(l) eq ""} {
	# Set iperf 20x tcp defaults to match iperf 170
	set (l) "8k"
    }

    if {$(l) eq {}} {
	set (l) [list {}]
    }
    set ll [llength $(l)]
    if {$ll == 1} {
	set l0 [lindex $(l) 0]
	for {set i 1} {$i < $pl} {incr i} {
	    lappend (l) $l0
	}
    } elseif {$ll ne $pl} {
	error "-l must contain one length, or the same number of lengths as test pairs"
    }

    # Objects with a lanpeer are automatically replaced with their
    # peers.  If an object has more than one peer, set up datastreams
    # to all of them and divide offer rate proportionally.
    set P {}
    set newb {}
    set newS {}
    set newl {}
    foreach {S D} $PAIRS b $(b) ToS $(S) l $(l) {
	set exp 0
	if {[llength $S] > 1 || [set slp [$S cget -lanpeer]] eq ""} {
	    # Need to make sure they are not both empty
	    set slp $S
	}
	if {[llength $D] > 1 || [set dlp [$D cget -lanpeer]] eq ""} {
	    set dlp $D
	}
	set slen [llength $slp]
	set dlen [llength $dlp]
	if {$slen >= $dlen} {
	    set maxlen $slen
	} else {
	    set maxlen $dlen
	}
	for {set i 0} {$i < $maxlen} {incr i} {
	    # pull from peer lists round-robin
	    set sl [lindex $slp [expr {$i % $slen}]]
	    set dl [lindex $dlp [expr {$i % $dlen}]]
	    lappend P $sl $dl
	    lappend newS $ToS
	    lappend newl $l
	    incr exp
	    if {$(nolanx)} {
		# No LAN expansion - use only primary endpoint
		break
	    }
	}
	# Divide UDP offer rate according to how much we expanded the
	# endpoints.
	if {![regexp {([\d\.]+)(.*)} $b - bnum radix]} {
	    error "-b format not recognised"
	}
	set bdiv [expr {(0.0+$bnum) / $exp}]
	for {set i 0} {$i < $exp} {incr i} {
	    lappend newb $bdiv$radix
	}
    }
    set PAIRS $P
    unset P
    set (b) $newb
    set (S) $newS
    set (l) $newl
    unset newb
    unset newS
    unset newl

    # When running multiple streams, in theory the first and last
    # samples will be biased high due to inevitable differences in
    # start time.  Here is experimental code to adjust the interval
    # timing to add extra start and end samples, then discard their
    # results.  I have yet to see any case where this actually
    # produces a better report therefore this code remains disabled by
    # default.
    if {[info exists UTF::MultiperfTrim] && [llength $PAIRS] > 2} {
	set _MultiperfTrim 1
	# Reduce interval to add extra samples for trimming
	set (i) [expr {$(i) *  $expected / ($expected+2.0)}]
	incr expected 2
    }

    if {[info exists UTF::PushNewIperf]} {
	foreach S $PAIRS {
	    if {[catch {
		$S setup_iperf
	    } ret]} {
		UTF::Message WARN $S $ret
	    }
	}
    }

    foreach {S D} $PAIRS b $(b) ToS $(S) l $(l) {
	set pth "Datapath: $S->$D"
	if {$b > 0} {
	    append pth " -b $b"
	}
	if {$ToS ne ""} {
	    append pth " -S $ToS"
	}
	if {$l > 0} {
	    append pth " -l $l"
	}
	UTF::Message INFO "" $pth
    }

    if {!$tcp} {
	# Note -fB is a sneaky way of ensuring the reports from the
	# udp sender do not match the data collector regexp.
	lappend args -u -fB
	# Oversubscribed UDP may need time to drain.  Don't reduce
	# this time without validating recovery after timeouts.
	set Timeout [expr {int($(t) + 7)}]
    } else {
	if {[info exists UTF::TcpReadStats]} {
	    lappend args -fm
	} else {
	    lappend args -fb -i $(i)
	}
	# Allow TCP some time to recover in bad conditions
	set Timeout [expr {int($(t) + 10)}]
    }

    set tcpslowstart 0
    if {$tcp} {
	# This is a TCP run
	foreach {SRC SNK} $PAIRS {
	    # Find max txpslowstart of all the SRCs in seconds
	    if {[$SRC cget -tcpslowstart] > $tcpslowstart} {
		set tcpslowstart [$SRC cget -tcpslowstart]
	    }
	}
	if {$tcpslowstart > 0} {
	    # Convert to samples, rounding up
	    set tcpslowstart [expr {int(ceil(double($tcpslowstart)/$(i)))}]
	    # Increase time to cover extra samples
	    set (t) [expr {$(t) + $tcpslowstart*$(i)}]
	}
    }

    # Update transmitter args since they may be slightly different
    # from the UDP reciever args above.
    if {[lsearch $args -n] < 0} {
	lappend args -t $(t)
    }
    if {$(e)} {
	# If extended stats are turned on, then show the tx side of
	# TCP as well.
	lappend args -i $(i)
    }
    array set tuned {}
    # Setup commands to start iperf on each SRC.
    set pair 0
    set cmds {}
    set servers ""
    set clients ""

    foreach {SRC SNK} $PAIRS {
	if {![info exists ip($SNK)]} {
	    set ip($SNK) [$SNK ipaddr]
	}
	if {[info exists UTF::BindIperf] && ![info exists ip($SRC)]} {
	    set ip($SRC) [$SRC ipaddr]
	}
	if {!$(noping) && ![info exists pinged($SRC,$SNK)]} {
	    $SRC ping $ip($SNK) -c $(pingmax)
	    # No need to ping same pair twice
	    set pinged($SRC,$SNK) 1
	}
    }

    foreach {SRC SNK} $PAIRS {
	set sargs $args
	if {[info exists UTF::BindIperf]} {
	    lappend sargs -B $ip($SRC)
	}
	if {$(e)} {
	    lappend sargs -e
	}
	if {[set b [lindex $(b) $pair]] > 0} {
	    lappend sargs -b $b
	}
	if {[set S [lindex $(S) $pair]] ne ""} {
	    lappend sargs -S [UTF::Multiperf::ToS $S]
	}
	if {[set l [lindex $(l) $pair]] ne ""} {
	    lappend sargs -l $l
	} elseif {[$SRC hostis Cygwin WinDHD] &&
		  [regexp {^[4-7]} [$SRC cget -osver]]} {
	    # Windows fragments large UDP packets, making the
	    # iperf default give poor performance.
	    lappend sargs -l 1024
	}

	if {$tcp} {
	    # This will be a TCP run.  Set TCP window, which will also
	    # make sure IPerf is set up for TCP
	    if {$(w) ne ""} {
		if {![info exists tuned($SNK)]} {
		    set tuned($SNK) [$SNK tcptune $(w)]
		}
		if {![info exists tuned($SRC)]} {
		    set tuned($SRC) [$SRC tcptune $(w)]
		}
		if {$tuned($SRC) && $(w) ne 0} {
		    lappend sargs -w $(w)
		}
	    } elseif {[$SNK hostis Cygwin WinDHD]} {
		# TCP window not set, still need to check IPerf
		$SNK IPerfService InstallCheck
	    }
	} else {
	    if {$(w) ne ""} {
		if {![info exists tuned($SRC)]} {
		    set tuned($SRC) [$SRC udptune $(w)]
		}
		if {$tuned($SRC) && $(w) ne 0} {
		    lappend sargs -w $(w)
		}
	    }
	}
	if {$(P) > 1} {
	    lappend sargs -P $(P)
	}
	if {$tcp && [$SNK cget -iperfdaemon] &&
	    ![info exists UTF::TcpReadStats]} {
	    # TCP run using an iperf daemon on the original port.
	    set port $(p)
	} else {
	    # Start a temporary iperf server, either for UDP or
	    # because there is no daemon installed.

	    # Ensure that there is no iperf daemon running on any
	    # windows systems, since they can only run one.
	    if {[$SNK hostis Cygwin WinDHD]} {
	        $SNK IPerfService Remove
	    }
	    # Cycle port numbers
	    if {[incr newport] > 5065} {
		set newport 5002
	    }
	    set port $newport

	    # Map rx port (must match tx port due to SWWLAN-93485)
	    lassign [$SRC portmap get $ip($SNK) $port $tcp] mip mp
	    set rxl $(rxl)
	    if {$tcp} {
		# TCP with non-daemon listener
		if {[info exists UTF::TcpReadStats]} {
		    set rargs [list -P $(P) -i $(i) -fb]
		} else {
		    set rargs [list -P $(P) -fB]
		}
		if {$(w) ne "" && $(w) ne 0 && $tuned($SNK)} {
		    lappend rargs -w $(w)
		}
		if {$rxl eq ""} {
		    # Set iperf 20x tcp defaults to match iperf 170
		    set rxl "8k"
		}
	    } else {
		# UDP listener
		set rargs [list -P $(P) -i $(i) -fb -u]
		if {$(w) ne ""} {
		    if {![info exists tuned($SNK)]} {
			set tuned($SNK) [$SNK udptune $(w)]
		    }
		    if {$tuned($SNK) && $(w) ne 0} {
			lappend rargs -w $(w)
		    }
		}
	    }
	    if {$rxl ne ""} {
		lappend rargs -l $rxl
	    }
	    if {$(e)} {
		lappend rargs -e
	    }
	    # No need to specify port if it's already default (common case)
	    if {$mp != 5001} {
		set pp "-p $mp"
	    } else {
		set pp ""
	    }
	    set cmd [list $SNK rexec -x -async -K -t $Timeout -T $Timeout]
	    if {[llength $PAIRS] > 2} {
		set n "\[$pair\] "
		lappend cmd -prefix $n
	    } else {
		set n ""
	    }
	    set fd [eval [concat $cmd [$SNK cget -iperf] -s $pp $rargs]]

	    lappend servers $fd

	    set name($fd) $n

	    set src($fd) $SNK
	}

	# Map tx port
	lassign [$SRC portmap get $ip($SNK) $port $tcp] mip mp

	# No need to specify port if it's already default (common case)
	if {$mp != 5001} {
	    set pp "-p $mp"
	} else {
	    set pp ""
	}
	# Iperf client
	set cmd [list $SRC rexec -x -async -K -t $Timeout -T $Timeout]
	if {[llength $PAIRS] > 2} {
	    set n "\[$pair\] "
	    lappend cmd -prefix $n
	} else {
	    set n ""
	}
	lappend cmds [concat $cmd [$SRC cget -iperf] -c $mip $pp $sargs]
	lappend names $n
	set src($pair) $SRC
	incr pair
    }

    # If we launched one or more servers - give them a chance to
    # start up.  Wait for the "listening" response.
    foreach fd $servers {
	if {$::tcl_version < 8.6} {
	    try {
		$fd waitfor "listening"
	    } catch -info info -msg msg {
		onerr {NotFound} {
		    UTF::Message LOG $name($fd) "iperf server failed to start"
		} * {
		    error $msg $info
		}
	    }
	} else {
	    try {
		$fd waitfor "listening"
	    } trap {NotFound} {} {
		UTF::Message LOG $name($fd) "iperf server failed to start"
	    }
	}
    }
    set pair 0
    foreach cmd $cmds n $names {
	set fd [eval $cmd]

	lappend clients $fd

	set src($fd) $src($pair)
	set name($fd) $n
	incr pair
    }
    if {$(callback) ne ""} {
	uplevel 1 $(callback)
    }
    foreach fd [concat $servers $clients] {
	set data($fd) [$fd close]
    }

    # Compute final results
    set means {}
    set tot {}
    if {$(kpps)} {
	set units kpps
    } else {
	set units mbps
    }
    foreach fd [concat $servers $clients] {

	set i 0
	set sum 0
	set results {}
	if {![info exists data($fd)]} {
	    continue
	}
	foreach line [split $data($fd) \n] {
	    if {$(P) > 1 && ![regexp {\[SUM\]} $line]} {
		# Parallel test - discard component streams
		continue
	    }
	    if {[regexp {0\.0+- *0\.0+ sec} $line]} {
		UTF::Message WARN $src($fd) "Ignoring Bogus zero-length record"
		continue
	    }
	    if {$txrate == 0 && [regexp {([\d.]+) Bytes/sec} $line - txrate]} {
		# Only use first rate report from the client
		# Convert to same units as offer rate
		set txrate [expr {$txrate * 8.0/1000000.0}]
		UTF::Message WARN $src($fd) "TX rate: $txrate Mbps"
		continue
	    }
	    if {![regexp {([\d.]+) bits/sec} $line - p]} {
		continue
	    }
	    if {$(kpps)} {
		if {[regexp {(\d+) pps} $line - p]} {
		    set point [expr {$p / 1000.0}]
		} else {
		    continue
		}
	    } else {
		set point [expr {$p / 1000000.0}]
	    }
	    lappend results $point
	    set sum [expr {$sum + $point}]
	    if {[llength $tot] > $i} {
		set tot [lreplace $tot $i $i [expr {[lindex $tot $i]+$point}]]
	    } else {
		lappend tot $point
	    }
	    incr i
	    if {$i >= $expected} {
		break
	    }
	}
	set n $name($fd)
	if {$results ne ""} {
	    lappend means [expr {$sum / $i}]
	    lappend components $results
	    UTF::Message LOG $src($fd) "${n}Results($units): $results"
	}
	if {[info exists datagrams] &&
	    [regexp {Sent (\d+) datagrams} $data($fd) - dg]} {
	    incr datagrams $dg
	}
    }
    if {$tot eq {}} {
	set tot 0
    }


    if {$tcpslowstart} {
	UTF::Message LOG Total \
	    "TCP Slow start, dropping $tcpslowstart sample[expr {$tcpslowstart>1?"s":""}]"
	incr tcpslowstart -1
	set tot [lreplace $tot 0 $tcpslowstart]
    }

    if {[info exists _MultiperfTrim]} {
	UTF::Message LOG Total "Results($units): $tot"
	UTF::Message LOG Total \
	    "Trimming first and last samples to mitigate sync issues. "
	set means [lreplace [lreplace $means 0 0] end end]
	set tot [lreplace [lreplace $tot 0 0] end end]
    }

    if {[llength $means] != 1} {
	UTF::Message LOG Total "Results($units): $tot"
    }

    if {$(means)} {
	return $means
    } else {
	return $tot
    }
}

UTF::doc {
    # [list_end]

    # [example_begin]
    # Example:
    # utf> UTF::Multiperf {UTFTestMw_1 UTFTestE_1 UTFTestN_1 UTFTestE_1} -i 2 -w 512k
    # 284.459008 261.914624 204.275712 234.749952 250.707968
    # [example_end]


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
