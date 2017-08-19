#! /bin/env tclsh8.4
#
# This script parses cycle prints of simulator output
# and gives through put value.
# $Id: Cyclemeasure.tcl,v 1.2 2007-03-01 11:21:27 $
#
# Usage: change WORKDIR to top of source code that has dongle/rte/sim.
#        You can also mention COMPILE targets.
#        You can set mailling list FROM and TO.


#Set path and compile type
set WORKDIR "/projects/hnd_software/work/vinaysg/hndrtesim"

set COMPILE { \
nodebug-optsize-ag-cdc-usb-ccx-wme-cram-wlcq-wlcnt-arm7tdmi-thumb
}
#nodebug-optsize-ag-cdc-sdio-ccx-wme-cram-wlcq-wlcnt-arm7tdmi-thumb


#Set variables to zero, Total tx, rx cycle count and mips
set cycles 0
set bus 0
set Ccount 0
set mips 0

#Variables required to send automated mail
set FROM "vinaysg@broadcom.com"
#set FROM "hwnbuild@broadcom.com"
set TO "vinaysg@broadcom.com"
#set TO "vinaysg@broadcom.com,dmahesh@broadcom.com,ash@broadcom.com"

set TblRow1 "\t\t\tTCP-PUT\t\t|\t\tTCP-GET"
set TblRowsep "\n------------------------------------------------------------------------------------------------------------------\n"
set TblRow2 "Tcp-tx in cycles | Tcp-ack in cycles | MIPS in mips | Tcp-rx in cycles | Tcp-ack in cycles | MIPS in mips"
set body ""
set TblRows ""

#Variables required to parse for cycle counts in tx and rx for different frame size
set begin "Associated"
set txline "Generate D11 frame"
set usbrxline "Generate USB frame"
set sdiorxline "Generate SDIO frame"

# [call [ParseForCcount] [arg filename] [arg framesize] [arg direction]]
#Parse the file [arg filename] on tx/rx [arg direction] side to get cycle counts
#for framesize [arg framesize].
proc ParseForCcount {filename framesize bus direction} {

	global txline usbrxline sdiorxline begin Ccount
	set Ccount 0

	if {$direction == "tx"} {

		set start "$txline ($framesize)"

		if {$bus == "usb"} {
			set end "$usbrxline"
		} elseif {$bus == "sdio"} {
			set end "$sdiorxline"
		}

	} elseif {$direction == "rx"} {

		if {$bus == "usb"} {
			set start "$usbrxline ($framesize)"
		} elseif {$bus == "sdio"} {
			set start "$sdiorxline ($framesize)"
		}

		set end $txline
	}


	if {[catch {open $filename r} fd]} {
		puts "Cannot open file: $filename"
	}

	set STATE END
	set PASSFIRSTFOUND FALSE

	while {[gets $fd line] >= 0} {
		if {($STATE == "END") && ([regexp $begin $line])} {
			set STATE ASSOCIATE
		}

		if {[string match "$start" $line] && ($STATE == "ASSOCIATE")} {
			if {$PASSFIRSTFOUND == "TRUE"} {
				set STATE START
			} else {
				set PASSFIRSTFOUND TRUE
			}
		}

		if {($STATE == "START")} {
			if {[regexp {Interim profile .* \d+ cycles} $line]} {
				continue	
			} elseif {[regexp {.* \d+ cycles} $line]} {
				regexp {\d+} $line cycles
				#puts $cycles
				set	Ccount [expr $Ccount + $cycles]
			}
		}

    	if {($STATE == "START") && [regexp $end $line]} { 
			set STATE END
			break 
		}
	}
		close $fd
}

# [call [cmd MipsUsage] 
#       [arg tcpdata] [arg tcpack] [arg tcpthr] [arg tcppayload]

# Calculate Million Instructions per second(MIPS) using parameters
#       [arg tcpdata] [arg tcpack] [arg tcpthr] [arg tcppayload]
proc MipsUsage {tcpdata tcpack tcpthr tcppayload} {

	global mips
	set mips [expr [expr $tcpthr * [expr $tcpdata + $tcpack]] / $tcppayload]
}

# [call [cmd SendMail] [arg TO] [arg FROM] [arg body]]

# Send email message to recipient(s) [arg TO] with 
# message body [arg body].  [arg TO] may be a single
# recipient or a list.  The sender will be [arg FROM].
# The message will be sent as "text/plain".
proc SendMail {TO FROM body} {

    package require smtp
    package require mime

    set token [mime::initialize -canonical text/plain -string $body]
    smtp::sendmessage $token \
	-debug 0 \
	-header [list From $FROM] \
	-header [list To $TO] \
	-header [list Subject "MIPS usage"] \
	-servers "smtphost.broadcom.com"
    mime::finalize $token
}


#iterate cyclemeasure for all targets
foreach target $COMPILE {

	cd $WORKDIR
	cd src/dongle/rte/sim/

	if { [ regexp {usb} $target ] } {
		set bus "usb"
	} elseif { [ regexp {sdio} $target ] } {
		set bus "sdio"
	}

	#incase make clean do not find anything to clean
	catch {exec make clean} ret

	exec make $target
   
	cd $target
	exec armsd -exec rtesim > $WORKDIR/output.txt 2>&1 &
	exec sleep 270
	set pid [exec ps | grep armsd | awk {{print $1}}]
	exec kill -9 $pid
	
	set tcpthr 40
	set tcppayload 1460
	
	#TCP-PUT
	ParseForCcount $WORKDIR/output.txt 1500 $bus tx
	set txdata $Ccount
	puts "Tx cycles $txdata"
	ParseForCcount $WORKDIR/output.txt 64 $bus rx
	set rxack  $Ccount
	puts "Rx cycles $rxack"
	MipsUsage  $txdata $rxack $tcpthr $tcppayload
	set mtcpput $mips
	
#	#TCP-GET
	ParseForCcount $WORKDIR/output.txt 1500 $bus rx
	set rxdata $Ccount
	ParseForCcount $WORKDIR/output.txt 64 $bus tx
	set txack  $Ccount
	MipsUsage  $rxdata $txack $tcpthr $tcppayload
	set mtcpget $mips
	
	set TblRows "$TblRows $TblRowsep TARGET NAME $TblRowsep $target $TblRowsep $TblRow1 $TblRowsep $TblRow2 $TblRowsep \t $txdata\t\t$rxack\t\t$mtcpput\t\t$rxdata\t\t$txack\t\t$mtcpget\n"

}

cd $WORKDIR

set Note "\n*NOTE\nSource : TOT dongle/rte/sim, ARMulator:ADS1.2, Tcp Throughput $tcpthr Mbps, Tcp Payload $tcppayload \n Tcp data is tx frame size 1500bytes \n Tcp ack is Rx frame size 64bytes \n"

set body "$body $TblRows \n $Note"

SendMail $TO $FROM $body
