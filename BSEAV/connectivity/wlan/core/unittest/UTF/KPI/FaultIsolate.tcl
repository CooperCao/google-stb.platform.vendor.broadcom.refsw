#!/bin/env utf
#
#
# Routine to parse and make prhdrs human readable
#
# Written by: Robert J. McMahon March 2012
#
# $Id$
# $Copyright Broadcom Corporation$
#
package require UTF
package provide UTF::prhdrsdecode 2.0

namespace eval UTF::prhdrsdecode {
    variable txpkt -array {}
    proc decodefsm {which} {
	if {$options(-msgdecodecolor) ne ""} {
	    # FC: 0x0080 (0x80) Dur: 0x0000
	    if {[regexp {assoc_state} $msg]} {
		UTF::Message LOG $options(-name) $msg
		return
	    }
	    if {[regexp {TSO hdr} $msg]} {
		set txpkt(state,$options(-name)) "TXPARSE"
		#			UTF::Message DEBUG $options(-name) "TXPARSE"
		set txpkt(seqno,$options(-name)) {}
		set txpkt(ackseqno,$options(-name)) {}
		return
	    }
	    if {[regexp {txpkt \(MPDU\) Complete} $msg]} {
		if {![info exists txpkt(seqno,$options(-name))]} {
		    UTF::Message DEBUG $options(-name) "MPDU ACK IGNORED"
		    return
		}
		set txpkt(state,$options(-name)) "TXACKSEQPARSE"
		return
	    }
	    if {![info exists txpkt(state,$options(-name))]} {
		return
	    }
	    # UTF::Message DUMP $options(-name) $msg
	    switch -exact $txpkt(state,$options(-name)) {
		"TXPARSE" {
		    if {[regexp {TxFrameID\s(0x[0-9a-fA-F]+)} $msg - seqno]} {
			set txpkt(seqno,$options(-name)) $seqno
			return
		    }
		}
		"TXACKSEQPARSE" {
		    if {[regexp {^FrameID: (0x[0-9a-fA-F]+)} $msg - seqno]} {
			#			UTF::Message DEBUG $options(-name) "ACKSEQNO=$seqno"
			set txpkt(ackseqno,$options(-name)) $seqno
		    }
		    #			    UTF::Message DEBUG $options(-name) "TXACKFLAGPARSE"
		    set txpkt(state,$options(-name)) "TXACKFLAGPARSE"
		}
		"TXACKFLAGPARSE" {
		    if {[regexp {^ACK ([01])} $msg - ackflag]} {
			UTF::Message PKT+$options(-msgdecodecolor) $options(-name) "SEQACK:[string tolower $txpkt(ackseqno,$options(-name))] ACK=$ackflag"
			set txpkt(ackseqno,$options(-name)) {}
		    }
		}
	    }
	    if {[regexp {^FC:\s(0x[0-9A-Fa-f]{4,4})} $msg - fc]} {
		set fc [string tolower $fc]
		set hmsg {}
		switch -exact $fc {
		    "0x0000" {
			set hmsg "(Assoc Req)"
		    }
		    "0x0010" {
			set hmsg "(Assoc Resp)"
		    }
		    "0x0810" {
			set hmsg "(Retry Assoc Resp)"
		    }
		    "0x0020" {
			set hmsg "(Re-Assoc Req)"
		    }
		    "0x0030" {
			set hmsg "(Re-Assoc Resp)"
		    }
		    "0x0040" {
			set hmsg "(Probe Req)"
		    }
		    "0x0050" {
			set hmsg "(Probe Resp)"
		    }
		    "0x0080" {
			set hmsg "(Beacon)"
		    }
		    "0x00b0" {
			set hmsg "(Auth)"
		    }
		    "0x0208" {
			set hmsg "(DS Beacon)"
		    }
		    "0x0148" {
			set hmsg "(ToDS/QoS Data)"
		    }
		}
		if {[info exists txpkt(seqno,$options(-name))]} {
		    set msg "SEQNO :[string tolower $txpkt(seqno,$options(-name))] $hmsg $msg"
		}
		UTF::Message PKT+$options(-msgdecodecolor) $options(-name) $msg
	    }
	} else {
	    UTF::Message LOG $options(-name) $msg
	}
    }
}