#!/bin/env utf
#
# $Id: 418d02f2de49016a1f9b9acb33b80999d843188f $
# $Copyright Broadcom Corporation$
#
package require snit
package require UTF

package provide UTF::AMPDUMonitor 2.0

snit::type UTF::AMPDUMonitor {
    typevariable D4 "3.267"
    typevariable E2 "2.660"

    option -ichart -default 0 -validatemethod __validateboolean -configuremethod __configureboolean
    option -sta
    option -sampleinterval -default 1.0
    option -setupcount -default 25
    option -verbose -type boolean -default 0

    variable everyid
    variable samples -array {}
    variable ichart -array {}
    variable previous {}

    constructor {args} {
	$self configurelist $args
    }

    destructor {
	$self stop
    }

    method inspect {} {
	if {[info exists everyid]} {
	    puts "everyid: $everyid"
	}
	parray samples
	parray ichart
    }

    method start {} {
	if {![info exists everiyd]} {
	    set hosttype [$options(-sta) hostis]
	    switch -exact $hosttype {
		"Router" {
		    set everyid [UTF::Every $options(-sampleinterval) "[mymethod __pollampdu_router]"]
		}
		"Linux" {
		    set everyid [UTF::Every $options(-sampleinterval) "[mymethod __pollampdu_linux]"]
		}
		default {
		    UTF::Message ERROR "$self" "No ampdu parser for $hosttype"
		}
	    }
	}
    }

    method stop {} {
	if {[info exists everyid]} {
	    UTF::Every cancel $everyid
	    unset everyid
	}
    }

    method restart {} {
	if {[info exists everyid]} {
	    UTF::Every cancel $everyid
	    $self stats -clear
	    UTF::Sleep $options(-sampleinterval)
	    set everyid [UTF::Every $options(-sampleinterval) "[mymethod __pollampdu]"]
	}
    }

    method stats {args} {
	set option [lindex $args 0]
	switch -exact -- $option {
	    "-clear" {
		unset samples
		array set samples {}
		unset ichart
		array set ichart {}
		set previous {}
	    }
	    "-get" {
	    }
	}
    }

    method __pollampdu_router {} {
	set sta $options(-sta)
	if {$options(-verbose)} {
	    set output [$sta wl dump ampdu]
	} else {
	    set output [$sta wl -silent dump ampdu]
	}
	# txampdu 0 txmpdu 0 txmpduperampdu 0 noba 0 (0%)
	# retry_ampdu 0 retry_mpdu 0 (0%) txfifofull 0
	# fbr_ampdu 0 fbr_mpdu 0
	# txregmpdu 0 txreg_noack 0
	# txrel_wm 0 txrel_size 0 sduretry 0 sdurejected 0
	# txdrop 0 txstuck 0 orphan 0
	# txr0hole 0 txrnhole 0 txrlag 0
	# txaddbareq 0 rxaddbaresp 0 txbar 0 rxba 0
	# rxampdu 0 rxmpdu 0 rxmpduperampdu 0 rxht 0 rxlegacy 0
	# rxholes 0 rxqed 0 rxdup 0 rxnobapol 0 rxstuck 0 rxoow 0
	# rxaddbareq 0 txaddbaresp 0 rxbar 0 txba 0
	# txdelba 0 rxdelba 0 rxunexp 0
	# txampdu_sgi 0 rxampdu_sgi 0, txampdu_stbc 0 rxampdu_stbc 0txampdu_mfbr_stbc 0
	# resp 2 ini 4 ini_off 0 ini_on 4 ini_poff 0 ini_pon 0 nbuf 0
	# Supr Reason: pmq(0) flush(0) frag(0) badch(0) exptime(0) uf(0)

	if {[regexp {txampdu ([\d]+) txmpdu ([\d]+) txmpduperampdu ([\d]+) noba ([\d]+) \(([\d]+)%\)\nretry_ampdu ([\d]+) retry_mpdu ([\d]+) \(([\d]+)%\) txfifofull ([\d]+)\nfbr_ampdu ([\d]+) fbr_mpdu ([\d]+)\ntxregmpdu ([\d]+) txreg_noack ([\d]+)\ntxrel_wm ([\d]+) txrel_size ([\d]+) sduretry ([\d]+) sdurejected ([\d]+)\ntxdrop ([\d]+) txstuck ([\d]+) orphan ([\d]+)\ntxr0hole ([\d]+) txrnhole ([\d]+) txrlag ([\d]+)\ntxaddbareq ([\d]+) rxaddbaresp ([\d]+) txbar ([\d]+) rxba ([\d]+)\nrxampdu ([\d]+) rxmpdu ([\d]+) rxmpduperampdu ([\d]+) rxht ([\d]+) rxlegacy ([\d]+)\nrxholes ([\d]+) rxqed ([\d]+) rxdup ([\d]+) rxnobapol ([\d]+) rxstuck ([\d]+) rxoow ([\d]+)\nrxaddbareq ([\d]+) txaddbaresp ([\d]+) rxbar ([\d]+) txba ([\d]+)\ntxdelba ([\d]+) rxdelba ([\d]+) rxunexp ([\d]+)\ntxampdu_sgi ([\d]+) rxampdu_sgi ([\d]+),? txampdu_stbc ([\d]+) rxampdu_stbc ([\d]+)[ ]?txampdu_mfbr_stbc ([\d]+)\nresp ([\d]+) ini ([\d]+) ini_off ([\d]+) ini_on ([\d]+) ini_poff ([\d]+) ini_pon ([\d]+) nbuf ([\d]+)\n(.*)(RX\s*MCS\s*:.*)(TX\s*MCS\s*:.*)MPDUdens:\s*(.*)?Retry\s*:}\
		 $output - \
		 samples(txampdu) \
		 samples(txmpdu) \
		 samples(txmpduperampdu) \
		 samples(noba) \
		 samples(noba_percent) \
		 samples(retry_ampdu) \
		 samples(retrympdu) \
		 samples(retry_mpdu_percent) \
		 samples(txfifofull) \
		 samples(fbr_ampdu) \
		 samples(fbr_mpdu) \
		 samples(txregmpdu) \
		 samples(txregnoack)\
		 samples(txrel_wm) \
		 samples(txrel_size) \
		 samples(sduretry) \
		 samples(sudrejected) \
		 samples(txdrop) \
		 samples(txstuck) \
		 samples(orphan) \
		 samples(txr0hole) \
		 samples(txrnhole) \
		 samples(txrlag) \
		 samples(txaddbareq) \
		 samples(rxaddbaresp) \
		 samples(txbar) \
		 samples(rxba) \
 		 samples(rxampdu) \
		 samples(rxmpdu) \
		 samples(rxmpduperampdu) \
		 samples(rxht) \
		 samples(rxlegacy) \
 		 samples(rxholes) \
		 samples(rxqed) \
		 samples(rxdup) \
		 samples(rxnobapol) \
		 samples(rxstuck) \
 		 samples(rxoow) \
		 samples(rxaddbareq) \
		 samples(txaddbareq) \
		 samples(rxdbar) \
		 samples(txba) \
		 samples(txdelba) \
		 samples(rxdelba) \
		 samples(rxunexp) \
		 samples(txampdu_sgi) \
		 samples(rxampdu_sgi) \
		 samples(txampdu_stbc) \
		 samples(rxampdu_stbc) \
		 samples(txampdu_mfbr_stbc) \
		 samples(resp) \
		 samples(ini) \
		 samples(ini_off) \
		 samples(ini_on) \
		 samples(ini_poff) \
		 samples(ini_pon) \
		 samples(nbuf) \
		 tmp \
	         rxmcshistogram \
	         txmcshistogram \
		 mpdudenshistogram \
		]} {
	    if {$samples(rxampdu) > 0} {
		set samples(rxsgi) [expr {$samples(rxampdu_sgi) / $samples(rxampdu) }]
	    }
	    if {$samples(txampdu) > 0} {
		set samples(txsgi) [expr {$samples(txampdu_sgi) / $samples(txampdu) }]
	    }
	    # UTF::Message DEBUG+blue "" "$txmcshistogram"
	    # UTF::Message DEBUG+blue "" "$rxmcshistogram"
	    ParseRXMCSRates $rxmcshistogram $samples(rxampdu) samples
	    # ParseRXSGIRates $rxsgihistogram samples
	    # parray samples
	    $self __compute_ampdu_ichart
	} else {
	    UTF::Message WARN $self "AMDPU parse miss"
	}
	$sta wl -silent ampdu_clear_dump
	set previous [clock clicks -milliseconds]
    }


    method __pollampdu_linux {} {
	set sta $options(-sta)
	if {$options(-verbose)} {
	    set output [$sta wl dump ampdu]
	} else {
	    set output [$sta wl -silent dump ampdu]
	}
	#MAC_ENAB 0, HW_ENAB 0 HOST_ENAB 1 UCODE_ENAB 0
	#txampdu 5968 txmpdu 73441 txmpduperampdu 13 noba 219 (4%)
	#retry_ampdu 1857 retry_mpdu 10051 (14%) fbr_ampdu 550 fbr_mpdu 1345
	#txregmpdu 0 txreg_noack 0 txfifofull 0
	#txrel_wm 7152 txrel_size 63312 sduretry 2272 sdurejected 0
	#txdrop 0 txstuck 0 orphan 0
	#txr0hole 0 txrnhole 0 txrlag 0
	#txaddbareq 0 rxaddbaresp 0 txbar 1 rxba 0
	#txdelba 0 rxdelba 0 rxunexp 0
	#txampdu_sgi 0 rxampdu_sgi 1 txampdu_stbc 0 rxampdu_stbc 0 txampdu_mfbr_stbc 0

	#rxampdu 1 rxmpdu 1 rxmpduperampdu 1 rxht 0 rxlegacy 0
	#rxholes 0 rxqed 0 rxdup 0 rxnobapol 0 rxstuck 0 rxoow 0
	#rxaddbareq 0 txaddbaresp 0 rxbar 0 txba 0
	#resp 1 ini 2 ini_off 0 ini_on 2 ini_poff 0 ini_pon 0 nbuf 240
	#Supr Reason: pmq(0) flush(0) frag(0) badch(0) exptime(0) uf(0)
	#RX MCS  :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        # :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  1(100%)
	#TX MCS  :  16(0%)  0(0%)  39(0%)  0(0%)  129(2%)  0(0%)  0(0%)  0(0%)
        # :  0(0%)  0(0%)  0(0%)  0(0%)  66(1%)  1128(18%)  655(10%)  3935(65%)
	#MPDUdens: 160 (2%) 409 (6%)  84 (1%) 190 (3%) 338 (5%) 416 (6%) 189 (3%) 106 (1%)
        # :  14 (0%)   1 (0%)   0 (0%)   3 (0%)   0 (0%)   5 (0%)   0 (0%) 4052 (67%)
	#Retry   :  99 532 698 738 710 458 391 458
        # : 506 486 441 439 460 462 462 462
	#Till End:   0  48  25  51  69  51  25   9
        # :   1   1   0   0   1   6 203 462
	#RX MCS SGI:  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
        #  :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  1(100%)
	#TX MCS SGI:
	#RX MCS STBC:
	#TX MCS STBC:
	#MCS to AMPDU tables:
	#00:90:4c:2f:0b:01: max_pdu 16 release 16
        #rxholes 0 rxstuck 0 txdrop 0 txstuck 0
        #txaddbareq 2 txaddbaresp 1 txrlag 0 sdurejected 0 txmpdu 16272793 txbar 24
        #txreg_noack 0 noba 21422 rxampdu 78 rxmpdu 78 rxlegacy 0 rxdup 0
        #rxoow 0 rxaddbaresp 2 rxdelba 0 rxbar 1

	if {[regexp {txampdu ([\d]+) txmpdu ([\d]+) txmpduperampdu ([\d]+) noba ([\d]+) \(([\d]+)%\)\nretry_ampdu ([\d]+) retry_mpdu ([\d]+) \(([\d]+)%\) fbr_ampdu ([\d]+) fbr_mpdu ([\d]+)\ntxregmpdu ([\d]+) txreg_noack ([\d]+ ) txfifofull ([\d]+)\ntxrel_wm ([\d]+) txrel_size ([\d]+) sduretry ([\d]+) sdurejected ([\d]+)\ntxdrop ([\d]+) txstuck ([\d]+) orphan ([\d]+)\ntxr0hole ([\d]+) txrnhole ([\d]+) txrlag ([\d]+)\ntxaddbareq ([\d]+) rxaddbaresp ([\d]+) txbar ([\d]+) rxba ([\d]+)\nrxampdu ([\d]+) rxmpdu ([\d]+) rxmpduperampdu ([\d]+) rxht ([\d]+) rxlegacy ([\d]+)\nrxholes ([\d]+) rxqed ([\d]+) rxdup ([\d]+) rxnobapol ([\d]+) rxstuck ([\d]+) rxoow ([\d]+)\nrxaddbareq ([\d]+) txaddbaresp ([\d]+) rxbar ([\d]+) txba ([\d]+)\ntxdelba ([\d]+) rxdelba ([\d]+) rxunexp ([\d]+)\ntxampdu_sgi ([\d]+) rxampdu_sgi ([\d]+),? txampdu_stbc ([\d]+) rxampdu_stbc ([\d]+)[ ]?txampdu_mfbr_stbc ([\d]+)\nresp ([\d]+) ini ([\d]+) ini_off ([\d]+) ini_on ([\d]+) ini_poff ([\d]+) ini_pon ([\d]+) nbuf ([\d]+)\n(.*)(RX\s*MCS\s*:.*)(TX\s*MCS\s*:.*)MPDUdens:\s*(.*)?Retry\s*:}\
		 $output - \
		 samples(txampdu) \
		 samples(txmpdu) \
		 samples(txmpduperampdu) \
		 samples(noba) \
		 samples(noba_percent) \
		 samples(retry_ampdu) \
		 samples(retrympdu) \
		 samples(retry_mpdu_percent) \
		 samples(fbr_ampdu) \
		 samples(fbr_mpdu) \
		 samples(txregmpdu) \
		 samples(txregnoack)\
		 samples(txrel_wm) \
		 samples(txrel_size) \
	      samples(txfifofull) \
		 samples(sduretry) \
		 samples(sudrejected) \
		 samples(txdrop) \
		 samples(txstuck) \
		 samples(orphan) \
		 samples(txr0hole) \
		 samples(txrnhole) \
		 samples(txrlag) \
		 samples(txaddbareq) \
		 samples(rxaddbaresp) \
		 samples(txbar) \
		 samples(rxba) \
 		 samples(rxampdu) \
		 samples(rxmpdu) \
		 samples(rxmpduperampdu) \
		 samples(rxht) \
		 samples(rxlegacy) \
 		 samples(rxholes) \
		 samples(rxqed) \
		 samples(rxdup) \
		 samples(rxnobapol) \
		 samples(rxstuck) \
 		 samples(rxoow) \
		 samples(rxaddbareq) \
		 samples(txaddbareq) \
		 samples(rxdbar) \
		 samples(txba) \
		 samples(txdelba) \
		 samples(rxdelba) \
		 samples(rxunexp) \
		 samples(txampdu_sgi) \
		 samples(rxampdu_sgi) \
		 samples(txampdu_stbc) \
		 samples(rxampdu_stbc) \
		 samples(txampdu_mfbr_stbc) \
		 samples(resp) \
		 samples(ini) \
		 samples(ini_off) \
		 samples(ini_on) \
		 samples(ini_poff) \
		 samples(ini_pon) \
		 samples(nbuf) \
		 tmp \
	         rxmcshistogram \
	         txmcshistogram \
		 mpdudenshistogram \
		]} {
	    if {$samples(rxampdu) > 0} {
		set samples(rxsgi) [expr {$samples(rxampdu_sgi) / $samples(rxampdu) }]
	    }
	    if {$samples(txampdu) > 0} {
		set samples(txsgi) [expr {$samples(txampdu_sgi) / $samples(txampdu) }]
	    }
	    # UTF::Message DEBUG+blue "" "$txmcshistogram"
	    # UTF::Message DEBUG+blue "" "$rxmcshistogram"
	    ParseRXMCSRates $rxmcshistogram $samples(rxampdu) samples
	    # ParseRXSGIRates $rxsgihistogram samples
	    # parray samples
	    $self __compute_ampdu_ichart
	} else {
	    UTF::Message WARN $self "AMDPU parse miss"
	}
	$sta wl -silent ampdu_clear_dump
	set previous [clock clicks -milliseconds]
    }

    method __compute_ampdu_ichart {} {
	if {$previous != {}} {
	    set timedelta [expr [clock clicks -milliseconds] - $previous]
	} else {
	    return
	}
	foreach key [array names samples] {
	    # Normalize values over time (unless already normalized)
	    if {![regexp {_percent$} $key]} {
		set value [expr $samples($key) * 1000.0 / $timedelta]
	    }
#	    puts "k: $key v: $value"
	    if {![info exists ichart(${key},meansum)]} {
		set ichart(${key},meansum) $value
		set ichart(${key},prev) $value
		set ichart(${key},rsum) 0
		set ichart(${key},icc_count) 1
		continue
	    } else {
		set ichart(${key},meansum) [expr $ichart(${key},meansum) + $value]
		set R [expr abs($value - $ichart(${key},prev))]
		set ichart(${key},rsum) [expr $ichart(${key},rsum) + $R]
		set ichart(${key},prev) $value
		incr ichart(${key},icc_count) +1
	    }
	    set count $ichart(${key},icc_count)
	    set meanmean [expr $ichart(${key},meansum)/$count]
	    set Rmean [expr $ichart(${key},rsum)/($count - 1)]

	    # Calculate X-bar and Rs limits based on moving range
	    # Ishikawa, Kaoru
	    set Ux [expr {$meanmean + $E2*$Rmean}]
	    set Lx [expr {$meanmean - $E2*$Rmean}]
	    if {$Lx < 0} {
		set Lx 0
	    }
	    set UR [expr {$D4*$Rmean}]
	    # Log events and display any messages
	    if {$count > $options(-setupcount)} {
		set f "%.2f"
		if {$value < $Lx} {
		    set msg [format "$f \[$f - $f\], range $f \[$f] LOW" $value $Lx $Ux $R $UR]
		} elseif {$value > $Ux} {
		    set msg [format "$f \[$f - $f\], range $f \[$f] HIGH" $value $Lx $Ux $R $UR]
		} elseif {$R > $UR} {
		    set msg [format "$f \[$f - $f\], range $f \[$f] WIDE" $value $Lx $Ux $R $UR]
		} else {
		    continue
		}
		UTF::Message LOG $self "$key $msg"
	    }
	}
    }

    proc ParseRXMCSRates {dump pkts SAMPLES} {
	upvar $SAMPLES samples
	# Extract MCS rate histogram
	if {[regexp {RX MCS  :  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)} \
		 $dump - \
		 samples(rxmcs,0) samples(rxmcs,1) samples(rxmcs,2) \
		 samples(rxmcs,3) samples(rxmcs,4) samples(rxmcs,5) \
		 samples(rxmcs,6) samples(rxmcs,7) \
		 samples(rxmcs,8) samples(rxmcs,9) samples(rxmcs,10) \
		 samples(rxmcs,11) samples(rxmcs,12) samples(rxmcs,13) \
		 samples(rxmcs,14) samples(rxmcs,15) \
		 samples(rxmcs,16) samples(rxmcs,17) samples(rxmcs,18) \
		 samples(rxmcs,19) samples(rxmcs,20) samples(rxmcs,21) \
		 samples(rxmcs,22) samples(rxmcs,23)]} {
	    set rows 3
	    # If 3row fails, try older 2row
	} elseif {[regexp {RX MCS  :  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)} \
		       $dump - \
		       samples(rxmcs,0) samples(rxmcs,1) samples(rxmcs,2) \
		       samples(rxmcs,3) samples(rxmcs,4) samples(rxmcs,5) \
		       samples(rxmcs,6) samples(rxmcs,7) \
		       samples(rxmcs,8) samples(rxmcs,9) samples(rxmcs,10) \
		       samples(rxmcs,11) samples(rxmcs,12) samples(rxmcs,13) \
		       samples(rxmcs,14) samples(rxmcs,15)]} {
	    set rows 2
	} elseif {[string compare "RX MCS  :\n"  $dump] == 0} {
	    for {set ix 0} {$ix < 24} {incr ix} {
		set samples(rxmcs,$ix) 0
		return
	    }
	} else {
	    UTF::Message DEBUG $self "MCS parse miss $dump"
	    return
	}
	set kx [expr {$rows * 8}]
	if {$pkts > 0} {
	    for {set ix 0} {$ix < $kx} {incr ix} {
		set samples(rxmcs,$ix) [expr {1.0 * $samples(rxmcs,$ix)/$pkts}]
	    }
	}
    }

    proc ParseRXSGIRates {dump SAMPLES} {
	# Check to see if sgi was used
	if {[regexp {RX MCS SGI:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)} \
		 $dump - \
		 samples(rxsgi,0) samples(rxsgi,1) samples(rxsgi,2) \
		 samples(rxsgi,3) samples(rxsgi,4) samples(rxsgi,5) \
		 samples(rxsgi,6) samples(rxsgi,7) \
		 samples(rxsgi,8) samples(rxsgi,9) samples(rxsgi,10) \
		 samples(rxsgi,11) samples(rxsgi,12) samples(rxsgi,13) \
		 samples(rxsgi,14) samples(rxsgi,15) \
		 samples(rxsgi,16) samples(rxsgi,17) samples(rxsgi,18) \
		 samples(rxsgi,19) samples(rxsgi,20) samples(rxsgi,21) \
		 samples(rxsgi,22) samples(rxsgi,23)]} {
	    set rows 3
	    # If 3row fails, try older 2row
	    elseif {[regexp {RX MCS SGI:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)} \
			  $dump - \
			  samples(rxsgi,0) samples(rxsgi,1) samples(rxsgi,2) \
			  samples(rxsgi,3) samples(rxsgi,4) samples(rxsgi,5) \
			  samples(rxsgi,6) samples(rxsgi,7) \
			  samples(rxsgi,8) samples(rxsgi,9) samples(rxsgi,10) \
			  samples(rxsgi,11) samples(rxsgi,12) samples(rxsgi,13) \
			  samples(rxsgi,14) samples(rxsgi,15)]} {
		set rows 2
	    }
	    else {
		UTF::Message DEBUG $self "SGI parse miss $dump"
		return
	    }
	}
	set kx [expr {$rows * 8}]
	if {$samples(rxampdu_sgi) > 0} {
	    for {set ix 0} {$ix < $kx} {incr ix} {
		set samples(rxsgi,$ix) [expr {1.0 * $samples(rxsgi,$ix) / $samples(rxampdu_sgi)}]
	    }
	}
    }
}
