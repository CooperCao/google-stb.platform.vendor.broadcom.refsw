#!/bin/env utf
# -*-tcl-*-

#
# AWDL utility functions
#

# $Id: c68996c542b04959102099c3bc6552475dc4ae3f $
# $Copyright Broadcom Corporation$
#

package require UTF

package provide UTF::AWDL 2.0

#################################################################################
#################################################################################

namespace eval UTF::AWDL {

    # AWDL Only	 "AAAAAAAASAAAAAAA"
    # AWDL+Infra "AAAAAABBSAAAAABB"
    # Idle       "SAAIIIIISAAIIIII"

    proc chan_seq {template args} {
	UTF::Getopts {
	    {A.arg "149/80" "AWDL Chanspec"}
	    {B.arg "" "Infrastructure BSS chanspec"}
	    {S.arg "6" "Service chanspec"}
	}
	if {![regexp {^[ASBI]{16}$} $template]} {
	    error "Invalid AWDL channel template: $template"
	}
	regsub {0x} [string tolower [UTF::chanspecx $(A)]] {} Ax
	if {$(B) eq ""} {
	    set Bx 0000
	} elseif {$(B) eq $(A)} {
	    set Bx $Ax
	} else {
	    regsub {0x} [string tolower [UTF::chanspecx $(B)]] {} Bx
	}
	if {$(S) eq $(A)} {
	    set Sx $Ax
	} elseif {$(S) eq $(B)} {
	    set Sx $Bx
	} else {
	    regsub {0x} [string tolower [UTF::chanspecx $(S)]] {} Sx
	}
	set Ix 0000

	set seq 0x
	foreach T [split $template {}] {
	    switch $T {
		A { append seq $Ax }
		B { append seq $Bx }
		S { append seq $Sx }
		I { append seq $Ix }
	    }
	}
	return $seq
    }

    proc create {P bssid} {

	UTF::Message INFO $P "Create AWDL interface"
	if {![$P wl apsta]} {
	    $P wl ap 0
	}
	$P wl up
	set localmac [string replace [$P macaddr] 0 1 02]
	set PI [$P wl_interface_create awdl -b $bssid -m $localmac]
	$PI ifconfig local
	set PI
    }

    proc idle {PI bssid args} {
	UTF::Getopts {
	    {A.arg "149/80" "AWDL Chanspec"}
	    {B.arg "" "Infrastructure BSS chanspec"}
	    {S.arg "6" "Service chanspec"}
	}

	UTF::Message INFO $PI "Set AWDL to idle"

	regsub {[^\d].*} $(A) {} chan

	$PI wl up
	$PI wl bss up
	$PI wl awdl_config 3
	$PI wl awdl_election_metric 1
	$PI wl awdl_af_hdr ff:ff:ff:ff:ff:88 0x7f010203
	$PI wl awdl_pktlifetime 10
	$PI wl awdl_sync_params 440 16 16 10 16 6
	$PI wl awdl_osoc_chan $chan
	$PI wl awdl_payload 0x5566779900888899fefe
	$PI wl awdl_oob_af_auto $bssid ff:ff:ff:ff:ff:88 0 10 7 20 6 3 65535 5000 1122334455667788
	$PI wl awdl_election_tree 0 0 1 -50 0 -90 -90 -90 1 1 16 0 0
	$PI wl awdl_af_rssi -80
	$PI wl awdl_awbcnoffset 0
	$PI wl awdl_maxnomaster 8
	$PI wl awdl_chan_seq 2 3 0 0xffff [chan_seq "SAAIIIIISAAIIIII" -A $(A) -S $(S)]
	$PI wl awdl_extcounts 3 0 0 0
	$PI wl awdl 1
	$PI wl awdl_presencemode 4
	$PI wl awdl_aftxmode 0
	$PI wl status
	$PI wl awdl 0
	set PI
    }


    proc perfconfig {PI bssid args} {
	UTF::Getopts {
	    {A.arg "149/80" "AWDL Chanspec"}
	    {B.arg "" "Infrastructure BSS chanspec"}
	    {S.arg "6" "Service chanspec"}
	}
	UTF::Message INFO $PI "Reconfigure channel sequence for performance"
	$PI wl awdl 0
	$PI wl awdl_sync_params 440 16 16 10 16 6
	regsub {[^\d].*} $(A) {} chan
	$PI wl awdl_osoc_chan $chan
	if {$(B) eq ""} {
	    set (B) $(A)
	}
	$PI wl awdl_chan_seq 2 3 0 0xffff [chan_seq "AAAAAABBSAAAAABB" \
					       -A $(A) -B $(B) -S $(S)]

	$PI wl awdl_extcounts 3 3 3 3
	$PI wl awdl_presencemode 4
	#$PI wl up
	#$PI wl bss up
	$PI wl awdl 1
	$PI wl awdl_oob_af_auto $bssid ff:ff:ff:ff:ff:88 0 10 7 20 6 3 65535 5000 112233445566778800
    }

    proc connect {PI peermac} {
	UTF::Message INFO $PI "Add peer"
	$PI wl awdl_peer_op add $peermac 1 0x2d1a7f1017ffff000000000000000000000000000000000000000000bf0c3258810ffaff0000faff0000
    }
}
