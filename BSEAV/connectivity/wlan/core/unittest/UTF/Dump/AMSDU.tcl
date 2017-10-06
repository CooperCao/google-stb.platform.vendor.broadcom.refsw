#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::Dump::AMSDU 2.0

package require snit
package require UTF::doc

snit::type UTF::Dump::AMSDU {

    # config files.
    pragma -canreplace yes

    option -raw

    variable TxMSDUdens
    variable TxMSDUdens_tot
    variable TxMSDUdens%

    method TxMSDUdens {} {
	$self _MSDUdens "TxMSDUdens" TxMSDUdens
    }
    method TxMSDUdens% {} {
	$self % TxMSDUdens
    }

    variable RxMSDUdens
    variable RxMSDUdens_tot
    variable RxMSDUdens%

    method RxMSDUdens {} {
	$self _MSDUdens "RxMSDUdens" RxMSDUdens
    }
    method RxMSDUdens% {} {
	$self % RxMSDUdens
    }


    ### internal

    method % {hist} {
	upvar ${hist}% HIST%
	if {![info exists HIST%]} {
	    upvar $hist HIST
	    upvar ${hist}_tot HIST_tot
	    $self $hist
	    set HIST% {}
	    foreach {n m} $HIST {
		if {$HIST_tot} {
		    lappend HIST% $n \
			[format "%.2f" [expr {100.0 * $m / $HIST_tot}]]
		} else {
		    lappend HIST% $n 0
		}
	    }
	}
	set HIST%
    }

    method _MSDUdens {key hist} {
	upvar $hist HIST
	if {![info exists HIST]} {
	    upvar ${hist}_tot TOT
	    set rest $options(-raw)
	    set TOT 0
	    set HIST {}
	    if {[regexp "$key:\\s*\(.*)" $rest - rest]} {
		set d 0
		#puts ">>>[string range $rest 0 100]<<<"
		while {[regexp {^(\d+)\(\s*\d+%\)\s+(.*)} $rest - p rest]} {
		    #puts ">>>[string range $rest 0 100]<<<"
		    set p [UTF::clean_number $p]; # discard leading zeros
		    lappend HIST $d $p
		    incr TOT $p
		    incr d
		}
	    }
	}
	set HIST
    }




}

if {0} {
set 11ac "
amsdu_agg_block 0 amsdu_rx_mtu 11398 rcvfifo_limit 0
amsdu_rxcap_big 1 fifo_lowm 1 fifo_hiwm 1
0 amsdu_deagg_state 0
1 amsdu_deagg_state 0
2 amsdu_deagg_state 0
0 agg_allowprio 1 agg_bytes_limit 3835 agg_sf_limit 2
1 agg_allowprio 1 agg_bytes_limit 3835 agg_sf_limit 2
2 agg_allowprio 1 agg_bytes_limit 3835 agg_sf_limit 2
3 agg_allowprio 1 agg_bytes_limit 3835 agg_sf_limit 2
4 agg_allowprio 1 agg_bytes_limit 3835 agg_sf_limit 2
5 agg_allowprio 1 agg_bytes_limit 3835 agg_sf_limit 2
6 agg_allowprio 1 agg_bytes_limit 3835 agg_sf_limit 2
7 agg_allowprio 1 agg_bytes_limit 3835 agg_sf_limit 2
agg_openfail 0
agg_passthrough 52
agg_block 0
agg_amsdu 0
agg_msdu 0
agg_stop_tailroom 0
agg_stop_sf 0
agg_stop_len 0
agg_stop_lowwm 0
deagg_msdu 11414668
deagg_amsdu 5707347
deagg_badfmt 0
deagg_wrongseq 1
deagg_badsflen 0
deagg_badsfalign 0
deagg_badtotlen 0
deagg_openfail 0
deagg_swdeagglong 0
deagg_flush 10
tx_pkt_free_ignored 0
tx_padding_in_tail 0
tx_padding_in_head 0
tx_padding_no_pad 0
TxMSDUdens:     27( 0%) 5711306(99%)

TxAMSDU Len:  0-1k  5711333(100%)
RxMSDUdens:      0( 0%)     27( 0%) 5706316(99%)

RxAMSDU Len:  0-1k  5706343(100%)
"

UTF::Dump::AMSDU testampdu -raw $11ac

foreach i {
    TxMSDUdens TxMSDUdens%
    RxMSDUdens RxMSDUdens%
} {
    puts "$i: [testampdu $i]"
}
}
