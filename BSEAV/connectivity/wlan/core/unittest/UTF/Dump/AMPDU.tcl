#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: f1f6979b04fa6d981ea8b030cae518ff71ea01db $
# $Copyright Broadcom Corporation$
#

package provide UTF::Dump::AMPDU 2.0

package require snit
package require UTF::doc

snit::type UTF::Dump::AMPDU {

    # config files.
    pragma -canreplace yes

    option -raw

    ### AMPDU Density

    variable MPDUdens
    variable MPDUdens%
    variable MPDUdens_tot

    method MPDUdens {} {
	if {![info exists MPDUdens]} {
	    set rest $options(-raw)
	    set MPDUdens_tot 0
	    set MPDUdens {}
	    set MPDUdens% {}
	    if {[regexp {MPDUdens(.*)} $rest - rest]} {
		set ix 1
		while {[regexp {^\s*:\s+(.*)} $rest - rest]} {
		    while {[regexp {^(\d+)(?:\s+\(\d+%\))?\s+(.*)} $rest - p rest]} {
			#puts ">>>[string range $rest 0 100]<<<"
			regsub {^0+(\d)} $p {\1} p
			lappend MPDUdens $ix $p
			incr MPDUdens_tot $p
		    }
		}
		if {$MPDUdens_tot} {
		    foreach m $MPDUdens {
			lappend MPDUdens% \
			    [format "%.2f" [expr {100.0 * $m / $MPDUdens_tot}]]
		    }
		}
		#puts "><>$rest<><"
	    }
	}
	set MPDUdens
    }

    method MPDUdens% {} {
	$self MPDUdens
	set MPDUdens%
    }

    # Rates

    variable TXVHT
    variable TXVHT%
    variable TXVHT_tot

    method TXVHT {} {
	$self _vht_rates "TX VHT" TXVHT
    }

    method TXVHT% {} {
	$self % TXVHT
    }

    variable TXVHTs
    variable TXVHTs%
    variable TXVHTs_tot

    method TXVHTs {} {
	if {![info exists TXVHTs]} {
	    # Fetch base VHT info
	    $self TXVHT

	    $self _vht_rates "TX VHT SGI" TXVHTSGI
	    set TXVHTs_tot $TXVHT_tot
	    $self _sgisub TXVHT TXVHTSGI TXVHTs
	}
	set TXVHTs
    }

    method TXVHTs% {} {
	$self % TXVHTs
    }

    variable TXMCSs
    variable TXMCSs%
    variable TXMCSs_tot

    method TXMCSs {} {
	if {![info exists TXMCSs]} {
	    # Fetch base MCS info
	    $self TXMCS

	    $self _mcs_rates "TX MCS SGI" TXMCSSGI
	    set TXMCSs_tot $TXMCS_tot
	    $self _sgisub TXMCS TXMCSSGI TXMCSs
	}
	set TXMCSs
    }

    method TXMCSs% {} {
	$self % TXMCSs
    }


    variable RXVHT
    variable RXVHT%
    variable RXVHT_tot

    method RXVHT {} {
	$self _vht_rates "RX VHT" RXVHT
    }

    method RXVHT% {} {
	$self % RXVHT
    }
    method RXVHT_tot {} {
	set RXVHT_tot
    }

    variable RXVHTs
    variable RXVHTs%
    variable RXVHTs_tot

    method RXVHTs {} {
	if {![info exists RXVHTs]} {
	    # Fetch base MCS info
	    $self RXVHT

	    $self _mcs_rates "TX MCS SGI" RXVHTSGI
	    set RXVHTs_tot $RXVHT_tot
	    $self _sgisub RXVHT RXVHTSGI RXVHTs
	}
	set RXVHTs
    }

    method RXVHTs% {} {
	$self % RXVHTs
    }


    variable TXMCS
    variable TXMCS%
    variable TXMCS_tot

    method TXMCS {} {
	$self _mcs_rates "TX MCS" TXMCS
    }

    method TXMCS% {} {
	$self % TXMCS
    }

    variable RXMCS
    variable RXMCS%
    variable RXMCS_tot

    method RXMCS {} {
	$self _mcs_rates "RX MCS" RXMCS
    }

    method RXMCS% {} {
	$self % RXMCS
    }

    variable RXMCSs
    variable RXMCSs%
    variable RXMCSs_tot

    method RXMCSs {} {
	if {![info exists RXMCSs]} {
	    # Fetch base MCS info
	    $self RXMCS

	    $self _mcs_rates "RX MCS SGI" RXMCSSGI
	    set RXMCSs_tot $RXMCS_tot
	    $self _sgisub RXMCS RXMCSSGI RXMCSs
	}
	set RXMCSs
    }

    method RXMCSs% {} {
	$self % RXMCSs
    }

    variable VHTPER
    variable VHTPER%

    method VHTPER {} {
	$self _vht_rates "VHT PER" VHTPER
    }
    method VHTPER% {} {
	$self VHTPER
	$self TXVHT

	set VHTPER% {}
	array set P $TXVHT
	foreach {n m} $VHTPER {
	    if {$P($n) > 0} {
		lappend VHTPER% $n [format "%.2f" [expr {100.0 * $m / $P($n)}]]
	    } else {
		lappend VHTPER% $n -1
	    }
	}
	set VHTPER%
    }

    variable MCSPER
    variable MCSPER%

    method MCSPER {} {
	$self _mcs_rates "MCS PER" MCSPER
    }
    method MCSPER% {} {
	$self MCSPER
	$self TXMCS
	set MCSPER% {}
	array set P $TXMCS
	foreach {n m} $MCSPER {
	    if {$P($n) > 0} {
		lappend MCSPER% $n [format "%.2f" [expr {100.0 * $m / $P($n)}]]
	    } else {
		lappend MCSPER% $n -1
	    }
	}
	set MCSPER%
    }

    variable Frameburst
    variable Frameburst_tot
    variable Frameburst%

    method Frameburst {} {
	if {![info exists Frameburst]} {
	    set rest $options(-raw)
	    set Frameburst_tot 0
	    set Frameburst {}
	    if {[regexp {Frameburst histogram:\s*(.*)} $rest - rest]} {
		set f 1
		while {[regexp {^(\d+)\s+(.*)} $rest - p rest]} {
		    #puts ">>>[string range $rest 0 100]<<<"
		    lappend Frameburst $f $p
		    incr Frameburst_tot $p
		    incr f
		}
	    }
	}
	set Frameburst
    }

    method Frameburst% {} {
	$self % Frameburst
    }

    method TXMode {} {
	# Returns the most popular TX rate and its usage %
	foreach {key count} [concat [$self TXMCS%] [$self TXVHT%]] {
	    if {![info exists best] || $count >= $best} {
		set best $count
		set mode $key
	    }
	}
	if {[info exists mode]} {
	    return [list $mode $best]
	} else {
	    return {}
	}
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

    method _mcs_rates {key hist} {
	upvar $hist HIST
	if {![info exists HIST]} {
	    upvar ${hist}_tot TOT
	    set rest $options(-raw)
	    set TOT 0
	    set HIST {}
	    # Remap proprietary 11n rates
	    array set prop11n {33 87 34 88 35 99 36 100 37 101 38 102}
	    if {[regexp "$key\(.*)" $rest - rest]} {
		set m 0
		while {[regexp {^\s*:\s+(.*)} $rest - rest]} {
		    while {[regexp {^(\d+)(?:\(\d+%\))?\s+(.*)} $rest - p rest]} {
			#puts ">>>[string range $rest 0 100]<<<"
			if {[info exists prop11n($m)]} {
			    set name $prop11n($m)
			} else {
			    set name $m
			}
			lappend HIST $name $p
			incr TOT $p
			incr m
		    }
		}
	    }
	}
	set HIST
    }

    method _vht_rates {key hist} {
	upvar $hist HIST
	if {![info exists HIST]} {
	    upvar ${hist}_tot TOT
	    set rest $options(-raw)
	    set TOT 0
	    set HIST {}
	    if {[regexp "$key\(.*)" $rest - rest]} {
		set s 1
		while {[regexp {^\s*:\s+(.*)} $rest - rest]} {
		    set m 0
		    while {[regexp {^(\d+)(?:\(\d+%\))?\s+(.*)} $rest - p rest]} {
			#puts ">>>[string range $rest 0 100]<<<"
			lappend HIST "${m}x$s" $p
			incr TOT $p
			incr m
		    }
		    incr s
		}
	    }
	}
	set HIST
    }

    method _sgisub {hist histsgi hists} {
	upvar $hist HIST
	upvar ${hist}_tot HIST_tot
	upvar $histsgi HISTSGI
	upvar $hists HISTs
	upvar ${hists}_tot HISTs_tot

	set HISTs_tot $HIST_tot
	set HISTs {}

	array set R $HIST
	array set S $HISTSGI
	foreach {n m} $HIST {
	    if {![info exists S($n)]} {
		set S($n) 0
	    }
	    set r [expr {$m - $S($n)}]
	    lappend HISTs $n $r ${n}s $S($n)
	}
    }


}

if {0} {
set 11n "
HOST_ENAB 1 UCODE_ENAB 0 4331_HW_ENAB 0 AQM_ENAB 0
AMPDU Tx counters:
txampdu 681 txmpdu 819 txmpduperampdu 2 noba 2 (1%)
retry_ampdu 2 retry_mpdu 2 (1%) txfifofull 0
fbr_ampdu 0 fbr_mpdu 0
txregmpdu 0 txreg_noack 0 txfifofull 0 txdrop 0 txstuck 0 orphan 0
txrel_wm 817 txrel_size 0 sduretry 0 sdurejected 0
txr0hole 0 txrnhole 0 txrlag 0 rxunexp 0
txaddbareq 4 rxaddbaresp 4 txlost 0 txbar 4 rxba 0 txdelba 0 
txampdu_sgi 0 txampdu_stbc 0 txampdu_mfbr_stbc 0
fb_clr 0 fb_nclr 0

ini 1 ini_off 0 ini_on 1 ini_poff 0 ini_pon 0 nbuf 0
Supr Reason: pmq(0) flush(0) frag(0) badch(0) exptime(0) uf(0) abs(0) pps(0)

TX MCS  :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  681(100%)
TX VHT  :MPDUdens: 608 (89%) 037 (5%) 019 (2%) 012 (1%) 001 (0%) 001 (0%) 003 (0%) 000 (0%)
Retry   : 000
Till End: 000
TX MCS SGI:
TX VHT SGI:
MCS to AMPDU tables:
20:16:d8:72:dc:91: max_pdu 32 release 32
	txdrop 0 txstuck 0 txaddbareq 4 txrlag 0 sdurejected 0
	txmpdu 819 txlost 0 txbar 4 txreg_noack 0 noba 2 rxaddbaresp 4
	txnoroom 0
	ba_state 1 ba_wsize 64 tx_in_transit 0 tid 0 rem_window 64
	start_seq 0x33b max_seq 0x33a tx_exp_seq 0x33b bar_ackpending_seq 0x255
	bar_ackpending 0 alive 0 retry_bar 0
	atf 0 ta 4000us tm 1000us rso:0x0

AMPDU Rx counters:
rxdelba 0 rxunexp 0
rxampdu_sgi 0 rxampdu_stbc 0 

rxampdu 2 rxmpdu 2 rxmpduperampdu 1 rxht 1 rxlegacy 0
rxholes 64 rxqed 0 rxdup 0 rxnobapol 0 rxstuck 0 rxoow 0 rxoos 0
rxaddbareq 1 rxbar 1 txba 0
txaddbaresp 1
resp 0
RX MCS  :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  2(100%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
RX VHT  :
RX MCS SGI:
RX VHT SGI:
20:16:d8:72:dc:91: 
	rxampdu 2 rxmpdu 2 rxlegacy 0 rxbar 1 rxdelba 0
	rxholes 64 rxstuck 0 rxoow 0 rxdup 0



"

set 11ac "
HOST_ENAB 0 UCODE_ENAB 0 4331_HW_ENAB 0 AQM_ENAB 1
AMPDU Tx counters:
txregmpdu 0 txreg_noack 0 txfifofull 0 txdrop 0 txstuck 0 orphan 0
txrel_wm 16710 txrel_size 704 sduretry 0 sdurejected 0
aggfifo_w 0 epochdeltas 0 mpduperepoch 0
epoch_change reason: plcp 7 rate 0 fbr 0 reg 0 link 0 seq no 0
txr0hole 0 txrnhole 0 txrlag 0 rxunexp 0
txaddbareq 0 rxaddbaresp 0 txlost 0 txbar 0 rxba 0 txdelba 0 
txmpdu_sgi 16973 txmpdu_stbc 0
ini 1 ini_off 0 ini_on 1 ini_poff 0 ini_pon 0 nbuf 0
tcp_ack_ratio 2/0 total 17440/48082 dequeued 30642 multi_dequeued 0
Supr Reason: pmq(0) flush(0) frag(0) badch(0) exptime(0) uf(0) pps(0)
TX MCS  :
MCS PER :
TX VHT  :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  15(0%)  0(0%)  17428(99%)
VHT PER :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  3(0%)
--------------------------
tot_mpdus 17443 tot_ampdus 1955 mpduperampdu 9
agg stop reason: len 0 (0%) ampdu_mpdu 0 (0%) bawin 5 (0%) epoch 0 (0%) fempty 11094 (99%)
Frameburst histogram:  0  930  997  0  0  0  0  0 avg 3
--------------------------
MPDUdens:  20 (1%) 766 (39%)   3 (0%)   9 (0%)   2 (0%)  13 (0%)   6 (0%)   9 (0%)
        :  12 (0%)  30 (1%) 190 (9%) 227 (11%) 102 (5%) 311 (15%) 139 (7%)  23 (1%)
        :  10 (0%)  13 (0%)  11 (0%)   9 (0%)   3 (0%)   6 (0%)   4 (0%)   9 (0%)
        :   3 (0%)   1 (0%)   1 (0%)   2 (0%)   1 (0%)   2 (0%)   0 (0%)   8 (0%)
        :   0 (0%)   5 (0%)   0 (0%)   0 (0%)   0 (0%)   1 (0%)   0 (0%)   0 (0%)
        :   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)
        :   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)
        :   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)   0 (0%)   4 (0%)
TX MCS SGI:
TX VHT SGI:  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  15(0%)  0(0%)  16958(99%)
TX MCS STBC:
TX VHT STBC:
MCS to AMPDU tables:
00:90:4c:1d:22:c7: max_pdu 64 release 32
	txdrop 0 txstuck 0 txaddbareq 0 txrlag 0 sdurejected 0
	txmpdu 17440 txlost 0 txbar 0 txreg_noack 0 noba 0 rxaddbaresp 0
	txnoroom 0
	ba_state 1 ba_wsize 64 tx_in_transit 0 tid 0 rem_window 64
	bytes_in_flight 0
	start_seq 0x422 max_seq 0x421 tx_exp_seq 0x422 bar_ackpending_seq 0x1
	bar_ackpending 0 alive 0 retry_bar 0
AMPDU Rx counters:
rxdelba 0 rxunexp 0
rxampdu_sgi 9181 rxampdu_stbc 0
rxampdu 9225 rxmpdu 349918 rxmpduperampdu 38 rxht 0 rxlegacy 0
rxholes 0 rxqed 31 rxdup 0 rxnobapol 0 rxstuck 0 rxoow 0 rxoos 0
rxaddbareq 0 rxbar 0 txba 0
txaddbaresp 0
resp 1
RX MCS  :
RX VHT  :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  3(0%)  0(0%)  9222(99%)
RX MCS SGI:
RX VHT SGI:  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  3(0%)  0(0%)  9178(99%)
00:90:4c:1d:22:c7: 
	rxampdu 9225 rxmpdu 9225 rxlegacy 0 rxbar 0 rxdelba 0
	rxholes 0 rxstuck 0 rxoow 0 rxdup 0
"

UTF::Dump::AMPDU testampdu -raw $11ac

foreach i {
    MPDUdens MPDUdens%
    TXVHT TXVHT% TXVHTs TXVHTs%
    TXMCS TXMCS% TXMCSs TXMCSs%
    MCSPER MCSPER%
    RXVHT RXVHT% RXVHTs RXVHTs%
    RXMCS RXMCS% RXMCSs RXMCSs%
    VHTPER VHTPER%
    Frameburst Frameburst%
    TXMode
} {
    puts "$i: [testampdu $i]"
}
}
