#!/bin/env utf
#
# $Id$
# $Copyright Broadcom Corporation$
#
#
# Written by: Robert J. McMahon November 2012
#
# $Copyright Broadcom Corporation$
package require snit
package require UTF
package require math

package provide UTF::netstats 2.0

snit::type UTF::tcpstats {
    typevariable CMD "/usr/sbin/ss -ieomn"
    option -tx ""
    option -rx ""
    option -period -default 0.200
    option -dst -default ""
    option -src -default ""
    option -dport -default ""
    option -messages -type boolean -default 0
    option -silent -type boolean -default 0
    option -tx -configuremethod __configurepoa
    option -rx -configuremethod __configurepoa
    option -bidir -type boolean -default 0
    option -color "black"

    variable fid
    variable utfmsgtag
    variable everyid -array {}
    variable rxhost ""
    variable txhost ""
    variable samples -array {}
    variable sample_pending 0
    variable cancel_pending 0
    variable gfids -array {}

    constructor {args} {
	set utfmsgtag [namespace tail $self]
	$self configurelist $args
    }
    destructor {
	foreach fid [array names gfid] {
	    catch {close $fid}
	}
	$self stop
    }
    method inspect {} {
	parray samples
    }
    method start {args} {
	if {$options(-dst) eq ""} {
	    set options(-dst) [$rxhost ipaddr]
	}
	if {$options(-src) eq ""} {
	    set options(-src) [$txhost ipaddr]
	}
	if {![info exists everyid(tx)] && [[$options(-tx) lan] hostis Linux]} {
	    set everyid(tx) [UTF::Every $options(-period) [mymethod __sample tx]]
	}
	if {$options(-bidir) && ![info exists everyid(rx)] && [[$options(-rx) lan] hostis Linux]} {
	    set everyid(rx) [UTF::Every $options(-period) [mymethod __sample rx]]
	}
    }
    method stop {args} {
	if {[info exists everyid(rx)]} {
	    catch {UTF::Every cancel $everyid(rx)}
	}
	if {[info exists everyid(tx)]} {
	    catch {UTF::Every cancel $everyid(tx)}
	}
	array unset everyid *
    }
    method clear {} {
	array unset samples *
    }
    method liveplot {args} {
	UTF::Getopts {
	    {stat.arg "rtt" "stat to plot"}
	    {style.arg "lines" "gnuplot style"}
	    {title.arg "" "graph title"}
	    {file.arg "" "output file name"}
	    {distribution ""}
	    {max.arg "150" ""}
	}
	if {$(stat) eq "all"} {
	    set stats [array names samples]
	} else {
	    set stats $(stat)
	}
	set host [$options(-tx) lan]
	foreach stat $stats {
	    if {![info exists gfid($stat)]} {
		set gfid($stat) [open "|/usr/bin/gnuplot" w+]
	    }
	    if {[llength $samples(${host},$stat)]} {
		set mean [eval [concat math::stats $samples(${host},$stat)]]
		foreach m $mean {
		    lappend r [format %2.2f $m]
		}
		UTF::Message INFO $utfmsgtag "$m : $samples(${host},$stat)"
	    }
	    if {$(distribution)} {
		set MAX $(max)
		set min [expr {round([eval [concat math::min $samples(${host},$stat)]])}]
		set max [expr {round([eval [concat math::max $samples(${host},$stat)]])}]
		for {set jx 0} {$jx <= $MAX} {incr jx} {
		    set d($jx) 0
		}
		foreach x $samples(${host},$stat) {
		    set v [expr {round($x)}]
		    if {$v <= $MAX} {
			incr d($v) +1
		    }
		}
		UTF::Streamslib::grapharray d -title "$(title) $stat" -style "boxes fill solid"
	    } else {
		puts $gfid($stat) "set title \"$(title)\\nStats:$r\""
		puts $gfid($stat) {set terminal png}
		if {$(file) ne ""} {
		    puts $gfid($stat)  "set output \"${(file)}.png\""
		}
		puts $gfid($stat) "plot '-' title \"$stat\" with $(style)"
		foreach y $samples(${host},$stat) {
		    puts $gfid($stat) "$y"
		}
		puts $gfid($stat) "e"
	    }
	    flush $gfid($stat)
	}
    }

    #  ESTAB      0      3573664        192.168.1.170:60002        192.168.1.101:60002  timer:(on,240ms,0) ino:293942 sk:f53eed00
    #  mem:(r0,w3606432,f657504,t0) ts sack bic wscale:7,7 rto:245 rtt:45.75/0.75 cwnd:1134 send 287.1Mbps rcv_space:14600
    method __sample {which} {
	if {$which eq "tx"} {
	    set host [$options(-tx) lan]
	    set dstip $options(-dst)
	    set srcip $options(-src)
	}
	if {$which eq "rx"} {
	    set host [$options(-rx) lan]
	    set srcip $options(-dst)
	    set dstip $options(-src)
	}
	if {$options(-messages)} {
	    set cmd [concat $host rexec -noinit $CMD dst $dstip src $srcip]
	} else {
	    set cmd [concat $host rexec -silent -quiet -noinit $CMD dst $dstip src $srcip]
	}
	if {$options(-dport) ne ""} {
	    set cmd [concat $cmd "dport eq :$options(-dport)"]
	}
	set output [eval $cmd]
	foreach line [split $output "\n"] {
	    if {$options(-messages)} {
		UTF::Message TCPSTAT $utfmsgtag $line
	    }
	    if {[regexp {ESTAB\s+([0-9]+)\s+([0-9]+)} $line - rxq txq]} {
		lappend samples($host,txq) $txq
		lappend samples($host,rxq) $rxq
	    } else {
		if {[regexp {mem:\(r([0-9]+),w([0-9]+),f([0-9]+).+rto:([0-9.]+)\srtt:([0-9.]+)/([0-9.]+).+cwnd:([0-9]+).+send\s([0-9.]+(K|M|G))bps} $line - rwin twin fwin rto rtt rttvar cwnd send]} {
		    lappend samples($host,rwin) $rwin
		    lappend samples($host,twin) $twin
		    lappend samples($host,fwin) $fwin
		    lappend samples($host,rto) $rto
		    lappend samples($host,rtt) $rtt
		    lappend samples($host,rttvar) $rttvar
		    lappend samples($host,cwnd) $cwnd
		    lappend samples($host,send) $send
		    set rtt [format %0.3f $rtt]
		    set rttvar [format %0.3f $rttvar]
		    if {!$options(-silent)} {
			UTF::Message SOCK-${which}+$options(-color) ${utfmsgtag}-${host} "txq:${txq}\tT:${twin}\tRTT:${rtt}\tRTTVAR:${rttvar}\tTxRate:${send}bps\tcwnd:${cwnd}\trxq:${rxq}\tR:${rwin}"
		    }
		}
	    }
	}
    }
    method __configurepoa {option value} {
	switch -exact -- $option {
	    "-tx" {
		set options(-tx) $value
		set txhost [namespace tail [$value lan]]
	    }
	    "-rx" {
		set options(-rx) $value
		set rxhost [namespace tail [$value lan]]
	    }
	}
    }
}