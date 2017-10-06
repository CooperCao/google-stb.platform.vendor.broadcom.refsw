#!/bin/env utf

#
# UTF Framework reusable test script utilities
# Based on snit
# $Copyright Broadcom Corporation$
#

namespace eval UTF::utils {}
package provide UTF::utils 2.0

# UTF modules
package require UTF::Aeroflex
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::Base
package require UTF::ControlChart
package require UTF::Cygwin
package require UTF::doc
package require UTF::Linux
package require UTF::DHD
package require UTF::DSL
package require UTF::HSIC
package require UTF::MemChart
package require UTF::WinBT
package require UTF::WinDHD
package require UTF::MacOS
package require UTF::Router
package require UTF::Sniffer
package require UTF::Test::ConnectAPSTA
package require base64

UTF::doc {
    # [manpage_begin UTF::utils n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF reusable test script utilities}]
    # [copyright {2005 Broadcom Corporation}]
    # [require UTF::utils]
    # [description]
    # [para]

    # UTF::utils contains various reusable script utility routines
    # intended for use by higher level scripts. See sample usage
    # of the utilities in Test/Sanity.test.

    # [list_begin definitions]

}

#====================================================================
# Ensure ::localhost is always defined
if {![info exists ::localhost]} {
    if {[info exists env(HOSTNAME)]} {
       set ::localhost $env(HOSTNAME)
    } elseif {[info exists env(HOST)]} {
       set ::localhost $env(HOST)
    } else {
       set ::localhost unknown
    }
}

# Selected procedures & methods automatically try their action
# again on failure. max_tries specifies how many times to try 
# before throwing an error.
if {![info exists ::max_tries]} {
    set ::max_tries 3
}

# Define common type lists.
# Objects that dont support the wl command are not included in main lists, eg: WinBT, Airport.
# DSL is largely gone from testing.
# Sniffers are treated separately.
set ::ap_oem_list   "Airport"
set ::ap_type_list  "Router DSL $::ap_oem_list"
set ::coex_sta_type_list "DHD HSIC MacOS Android"
set ::sta_type_list "$::coex_sta_type_list Cygwin Linux WinDHD"
set ::ssh_type_list "$::sta_type_list WinBT Power::Agilent"

# Make list of valid Attenuator Groups available to other scripts
set ::attn_grp_type_list "AttnGroup"

#====================================================================
UTF::doc {
    # [call [cmd UTF::access_counters] [arg STA] [opt args]]
    # Accesses and stores wl counters for [arg STA].
    # Valid args are: savecurrent getupdate showall showdelta pkteng silent [para]

    # [opt savecurrent] gets all wl counters, saves current counter values and 
    # sets counter deltas to null[para]

    # [opt getupdate] gets all wl counters, computes counter deltas based on previously
    # saved counter values and saves new counter values[para]

    # [opt showall] returns list of all data in format: name=value=delta[para]

    # [opt showdelta] returns list of only the items that changed in format:
    # name=value=delta[para]

    # [opt pkteng] includes pkteng counter data along with wl counter data[para]

    # [opt silent] does not show raw counter data in the logfile[para]

    # Typical usage is to first call access_counters with savecurrent silent,
    # then do more tests and then call access_counters again with getupdate
    # silent showdelta to see what changed.
}

proc UTF::access_counters {STA args} {

    # Check calling args.
    set sta_name [UTF::get_name $STA]
    UTF::Message INFO "$sta_name" "access_counters: args=$args"
    set valid_args "savecurrent getupdate showall showdelta pkteng silent"
    if {[llength $args] < 1} {
        error "$sta_name access_counters ERROR: must have at least 1 calling arg: $valid_args"
    }
    foreach item $args {
        if {[lsearch $valid_args $item] < 0} {
            error "$sta_name access_counters ERROR: invalid arg: $item, should be: $valid_args"
        }
    }

    # Get wl counters initial snapshot values, update existing
    # snapshot values as necessary.
    if {[regexp {savecurrent|getupdate} $args]} {
        # For savecurrent, clean out any previous values.
        if {[regexp {savecurrent} $args]} {
            set names [array names ::counters_array]
            # puts "names=$names"
            foreach item $names {
                # Comma in regexp pattern ensures exact matches on STA.
                if {[regexp "^$STA," $item]} {
                    # puts "unsetting ::counters_array($item)"
                    unset ::counters_array($item)
                }
            }
        }

        # Setup base command.
        set base_cmd rexec
        if {[regexp {silent} $args]} {
            append base_cmd " -silent -quiet"
        } 
        set counters_cmd "$base_cmd wl counters"

        # Always get wl counters.
        set counters [eval $STA $counters_cmd]
        set counters [split $counters "\n"]

        # Optionally get pkteng_stats.
        if {[regexp {pkteng} $args]} {
            set pkteng_cmd "$base_cmd wl pkteng_stats"
            set catch_resp [catch {eval $STA $pkteng_cmd} catch_msg]
            if {$catch_resp == 0} {
                # Clean up data so it can be parsed by common code below
                regsub -all {\n} $catch_msg " " catch_msg
                regsub -nocase {lost frame count} $catch_msg "lostframecount" catch_msg
                regsub -nocase {signal to noise ratio} $catch_msg "signaltonoiseratio" catch_msg
                lappend counters $catch_msg

            } else {
                UTF::Message WARN "$sta_name" "access_counters: $pkteng_cmd: $catch_msg"
            }
        }

        # Get current counter data, which is many lines formatted as:
        # 1) counter: value1 ... valueN
        # 2) counter1 value1 ... counterN valueN
        set total_counters 0
        foreach line $counters {
            if {[regexp {:} $line]} {
                set format 1 ;# list of numbers
                set fields [split $line ":"]
            } else {
                set format 2 ;# single number
                set fields [split $line " "]
            }
            foreach {cn cv} $fields {

                # Error checking
                set cn [string trim $cn]
                set cv [string trim $cv]
                # puts "format=$format cn=$cn cv=$cv"
                if {$cn == "" || $cv == ""} {
                    # Get lots of blank cn & cv...
                    continue
                }
                if {![regexp {^[\d\.\-\s]+$} $cv]} {
                    UTF::Message WARN "$sta_name" "access_counters: ignoring cn=$cn cv=$cv"
                    continue
                }

                # Store new values accordingly.
                incr total_counters
                if {[regexp {savecurrent} $args]} {
                    # Set counter delta to null, save new counter value.
                    set ::counters_array($STA,$cn,current) $cv
                    set ::counters_array($STA,$cn,delta) ""

                } else {
                    # Update counter delta, save new counter value.
                    if {[info exists ::counters_array($STA,$cn,current)]} {
                        set old_cv $::counters_array($STA,$cn,current)
                    } else {
                        set old_cv 0
                    }
                    if {$format == "1"} {
                        # Format 1, list of numbers.
                        if {$old_cv != $cv} {
                            set delta $cv ;# new list of numbers used as delta
                        } else {
                            set detla ""
                        }
                    } else {
                        # Format 2, single number. 
                        set delta [expr $cv - $old_cv]
                        if {$delta == 0} {
                            set delta ""
                        }

                    }
                    # puts "cn=$cn delta=$delta"
                    set ::counters_array($STA,$cn,current) $cv
                    set ::counters_array($STA,$cn,delta) $delta
                }
            }
        }
        if {![regexp {silent} $args]} {
            UTF::Message INFO "$sta_name" "access_counters: total_counters=$total_counters"
        }
    }

    # Show counter values as necessary. Return names/values/deltas as a list.
    set result ""
    if {[regexp {showall|showdelta} $args]} {
        set names [array names ::counters_array]
        set names [lsort $names]
        # puts "names=$names"
        foreach item $names {
            # Comma in regexp pattern ensures exact matches on STA.
            # Ignore the delta names.
            if {[regexp "^$STA,.*current" $item]} {
                set temp [split $item ","]
                set cn [lindex $temp 1] ;# get counter name from array index
                set cv $::counters_array($STA,$cn,current)
                set delta $::counters_array($STA,$cn,delta)
                # puts "matched ::counters_array($item) cn=$cn cv=$cv delta=$delta"
                if {[regexp {showall} $args]} {
                    # Return all counter values. No need to log values again.
                    lappend result "$cn=$cv=$delta"
                } else {
                    # Show & return only items that changed.
                    if {$delta != ""} {
                        lappend result "$cn=$cv=$delta"
                        if {![regexp {silent} $args]} {
                            UTF::Message INFO "$sta_name" "$cn=$cv delta=$delta"
                        }
                    }
                }
            }
        }
    }

    # Return list of names/values/deltas
    set result [string trim $result]
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::ampdu_clear] [arg AP] [arg STA] [opt nohistograms=0] [opt is_oem_ap=0]]

    # Clears the [arg AP] & [arg STA] ampdu stats if [opt nohistograms] is 0.
    # If [opt is_oem_ap] is 1, leaves the [arg AP] alone.
}

proc UTF::ampdu_clear {AP STA {nohistograms 0} {is_oem_ap 0}} {

    # Performance optimization.
    UTF::Message INFO "" "ampdu_clear AP=$AP STA=$STA nohistograms=$nohistograms is_oem_ap=$is_oem_ap"
    if {$nohistograms != 0} {
        return
    }

    # wl ampdu clear AP & STA
    if {$is_oem_ap == 0} {
        # Do only for BRCM Real AP & SoftAP
        set catch_resp [catch {$AP wl ampdu_clear_dump} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR "$AP" "catch_resp=$catch_resp catch_msg=$catch_msg"
        }
    }
    set catch_resp [catch {$STA wl ampdu_clear_dump} catch_msg]
    if {$catch_resp != 0} {
        UTF::Message ERROR "$STA" "catch_resp=$catch_resp catch_msg=$catch_msg"
    }
    return
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::ap_ampdu] [arg AP] [opt nohistograms=0] [opt is_oem_ap=0]]

    # Queries the [arg AP] ampdu stats if [opt nohistograms] is 0.
    # If [opt is_oem_ap] is 1, leaves the [arg AP] alone.[para]

    # Returns CSV string with counts for: RX MCS, TX MCS, MPDU,
    # RX MCS SGI, TX MCS SGI, MCS PER, RX VHT, TX VHT, RX VHT SGI, TX VHT SGI, VHT PER,
}

proc UTF::ap_ampdu {AP {nohistograms 0} {is_oem_ap 0}} {
    UTF::Message INFO "$AP" "ap_ampdu AP=$AP nohistograms=$nohistograms is_oem_ap=$is_oem_ap"

    # If AP is known to be in trouble, or nohistograms!=0, or is_oem_ap!=0 return.
    set msg " , , , , , , , , , , , ," ;# Need 1 comma for each 12 fields returned
    set var "::${AP}_state"
    if {[set $var] != "OK" || $nohistograms != "0" || $is_oem_ap != "0"} {
        return $msg
    }

    # Get wl dump ampdu.
    set catch_resp [catch {$AP wl dump ampdu} catch_msg]
    UTF::save_device_state $AP $catch_msg
    if {$catch_resp != 0 || $catch_msg == "N/A"} {
        UTF::Message ERROR "$AP" "catch_msg=$catch_msg"
        return $msg
    }

    # NB: For SISO devices, max MCS is 7, so there will be only one row of MCS stats!
    # NB: Sometimes there will be a MAC address after these strings.
    #     So parsing has to avoid picking up: 01:23:...

    # RX MCS  :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  6220(100%)
    # Look for 1 line or more of RX MCS stats.
    # set catch_msg "RX MCS :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  7(1%)\n\
    #			    :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  14(2%)\n\
    #			    :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  21(3%)\n\
    #			    :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  28(4%) "
    if {![regexp -nocase {RX\s*MCS(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg rx_mcs]} {
        set rx_mcs ""
    }

    # Look for 1 line or more of TX MCS stats.
    if {![regexp -nocase {TX\s*MCS(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg tx_mcs]} {
        set tx_mcs ""
    }

    # Look for 1 line or more of MPDUdens stats.
    # NB: Not always multiples of 8! Can be only 1!
    # set catch_msg "MPDUdens:   0 884 123  88 118 154  47 268 1134 341  97  56  18  34  51 326 3474" ;# test code
    if {![regexp -nocase {MPDUdens(\s*:(\s*\d+\s*\(\d+%\)\s*){1,}){1,}} $catch_msg mpdu_dens]} {
        # Old format is list of space separated digits.
        if {![regexp {MPDUdens\s*:[\d\s]+\n} $catch_msg mpdu_dens]} {
            set mpdu_dens ""
        }
    }

    # Look for 1 line or more of RX MCS SGI stats.
    if {![regexp -nocase {RX\s*MCS\s*SGI(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg rx_mcs_sgi]} {
        set rx_mcs_sgi ""
    }

    # Look for 1 line or more of TX MCS SGI stats.
    # set catch_msg "TX MCS SGI: \n00:90:4c:14:4a:ab: max_pdu 8 release 8" ;# no sgi data followed by mac addr
    if {![regexp -nocase {TX\s*MCS\s*SGI(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg tx_mcs_sgi]} {
        set tx_mcs_sgi ""
    }

    # Look for 1 line or more of MCS PER stats.
    if {![regexp -nocase {MCS\s*PER(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg mcs_per]} {
        set mcs_per ""
    }

    # Look for 1 line or more of RX VHT stats.
    # set catch_msg "RX VHT  :  1(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%) \
    #                        :  0(0%)  2(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  8(0%) \
    #                        :  0(0%)  0(0%)  3(0%)  0(0%)  0(0%)  0(0%)  5470(5%)  8836(93%)  27(0%)  0(0%)"
    if {![regexp -nocase {RX\s*VHT(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg rx_vht]} {
        set rx_vht ""
    }

    # Look for 1 line or more of TX VHT stats.
    # set catch_msg "TX VHT  :  1(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%) \
    #                        :  0(0%)  2(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  8(0%) \
    #                        :  0(0%)  0(0%)  3(0%)  0(0%)  0(0%)  0(0%)  5471(5%)  8833(93%)  290(0%)  0(0%)"
    set ap_vht_len 0
    if {[regexp -nocase {TX\s*VHT(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg tx_vht]} {
	# The purpose here is to get the number of fields, either 10 or 12.
	regexp -nocase {^TX\s*VHT(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,1}} $tx_vht tx_vht_tmp
	regexp -line {TX\s*VHT\s*:\s*(.*)} $tx_vht_tmp - tx_vht_final
	set ap_vht_len [llength $tx_vht_final]
    } else {
        set tx_vht ""
    }

    # Look for 1 line or more of RX VHT SGI stats.
    # set catch_msg "RX VHT SGI :  1(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%) \
    #                           :  0(0%)  2(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  8(0%) \
    #                           :  0(0%)  0(0%)  3(0%)  0(0%)  0(0%)  0(0%)  540(5%)  884(93%)  72(0%)  0(0%)"
    if {![regexp -nocase {RX\s*VHT\s*SGI(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg rx_vht_sgi]} {
        set rx_vht_sgi ""
    }

    # Look for 1 line or more of TX VHT SGI stats.
    # set catch_msg "TX VHT SGI :  1(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%) \
    #                           :  0(0%)  2(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  8(0%) \
    #                           :  0(0%)  0(0%)  3(0%)  0(0%)  0(0%)  0(0%)  5(5%)  8(93%)  300(0%)  0(0%)"
    if {![regexp -nocase {TX\s*VHT\s*SGI(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg tx_vht_sgi]} {
        set tx_vht_sgi ""
    }

    # Look for 1 line or more of VHT PER stats.
    if {![regexp -nocase {VHT\s*PER(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg vht_per]} {
        set vht_per ""
    }

    # Cleanup distribution strings, leaving just the raw frame counts.
    set i 1
    set result $ap_vht_len  ;# put it as first element, and will remove it later in rvr1.test. 
    foreach list [list $rx_mcs $tx_mcs $mpdu_dens $rx_mcs_sgi $tx_mcs_sgi $mcs_per $rx_vht $tx_vht $rx_vht_sgi $tx_vht_sgi $vht_per] {
	if {$i == 6 || $i == 11} {
	    # For PER data, extract percentage numbers.
	    regsub -all {\d+\(} $list "" list 	;# remove '8(' part
	    regsub -all {%\)}   $list "" list 	;# remove '%)' part
  	} else {
	    # For others, remove percentage numbers.
	    regsub -all {\(\d+%\)} $list "" list ;# remove percentage numbers (nn%)
	}
        regsub -all {\n} 	$list ""  list 	;# remove new-lines
        regsub -all {:} 	$list ""  list 	;# remove colons
        regsub -all {[a-zA-Z]} 	$list ""  list	;# remove text
        regsub -all {\s+} 	$list " " list 	;# compress consecutive whitespace into single space

	# Save a copy.
	if {$i == 2 || $i == 8} {
	    set tx_saved $list
 	}

	# If there is TX data, use original PER data. Otherwise, replace it with -1.
	if {$i == 6 || $i == 11} {
	    set j 0
	    set list_tmp ""
	    foreach element $tx_saved {
		if {$element == 0} {
		    append list_tmp "-1 "
		} else {
		    append list_tmp "[lindex $list $j] "
		}
		incr j
	    }
	    set list $list_tmp
	}

	# Fill in some data to avoid gnuplot errors.
        set list [string trim $list]
        if {$list == "" || $list == "0"} {
	    if {$i == 6 || $i == 11} {
            	set list "-1 -1 -1 -1 -1 -1 -1 -1"
	    } else {
            	set list ""
	    }
        }

	# Result is CSV formatted.
        append result " ${list},"
	incr i
    }

    # Log & return result.
    UTF::Message INFO "" "ap_ampdu distributions: ap_vht_len,rx_mcs,tx_mcs,mpdu_dens,rx_mcs_sgi,tx_mcs_sgi,mcs_per,rx_vht,tx_vht,rx_vht_sgi,tx_vht_sgi,vht_per,=$result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::post_to_dbux] [arg STA] [arg is_private_build] [arg test_type]]

    # Post test information to dbux for each STA object in test.
}

proc UTF::post_to_dbux {STA {is_private_build 0} {test_type ""}} {
    # Determine test type
    if {$test_type == ""} {
	set type "rvr"
    } else {
	set type $test_type
    }

    # Static values
    set report_url [UTF::LogURL summary.html]
    set server_url "http://wlan-systems.sj.broadcom.com/api/report/save"

    # Get board info
    set sta_revinfo [$STA revinfo]
    regexp -line {boardid:? (0x\w+)} $sta_revinfo - sta_board_id
    regexp -line {boardvendor:? (0x\w+)} $sta_revinfo - sta_board_vendor
    regexp -line {boardrev:? (\w+)} $sta_revinfo - sta_board_rev
    regexp -line {chipnum:? (0x\w+)} $sta_revinfo - sta_chip_id
    regexp -line {chiprev:? (0x\w+)} $sta_revinfo - sta_chip_rev

    # Get driver info
    set sta_driver_id ""
    if {$is_private_build == 1} {
 	set sta_driver_id "private_build"
    } else {
    	set id ""
    	set catch_resp [catch {$STA findimages} catch_msg]
    	set id [UTF::get_build_id $catch_msg]
	# Following regex will match: 7.10:2014.1.14.4, 7.10.82:2014.1.14.4, TOT:2014.1.14.4, trunk:2014.1.14.4
	regexp -line {((((\d+.)?\d+.\d+)|(TOT)|(trunk)):(\d+.){3}\d+)} $id - sta_driver_id
    }

    # Get STA os info
    set sta_os [check_host_os $STA "dbux"]

    UTF::Message INFO "dbux" "chipid=$sta_chip_id"
    UTF::Message INFO "dbux" "chiprev=$sta_chip_rev"
    UTF::Message INFO "dbux" "boardid=$sta_board_id"
    UTF::Message INFO "dbux" "boardvendor=$sta_board_vendor"
    UTF::Message INFO "dbux" "boardrev=$sta_board_rev"
    UTF::Message INFO "dbux" "driver=$sta_driver_id"
    UTF::Message INFO "dbux" "username=$::tcl_platform(user)"
    UTF::Message INFO "dbux" "os=$sta_os"
    UTF::Message INFO "dbux" "type=$type"
    UTF::Message INFO "dbux" "link=$report_url"

    # Post to dbux
    set ret [catch {exec /tools/bin/wget -nv -O- "$server_url?chipid=$sta_chip_id&chiprev=$sta_chip_rev&boardid=$sta_board_id&boardrev=$sta_board_rev&boardvendor=$sta_board_vendor&driver=$sta_driver_id&username=$::tcl_platform(user)&os=$sta_os&type=$type&link=$report_url"} msg]

    UTF::Message INFo "dbux" "$msg"

    if {[regexp {Report\s+was\s+successfully\s+saved\s+to\s+dBux} $msg]} {
	UTF::Message INFO "dbux" "Test information was successfully posted to dbux"
    } else {
        error "Failed to post to dbux"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::ap_txbf_stats] [arg AP] [arg STAs]]
    # Dump [arg AP] txbf.
    # Returns CSV string with: ApTxbfm, ApTxndp, ApRxsf, ApSfRatio
}

proc UTF::ap_txbf_stats {AP stalist} {
    UTF::Message INFO "" "ap_txbf_stats: AP=$AP STAs=$stalist"

    # Local variables
    set txbfm_counter 0
    set txndp_counter 0
    set rxsf_counter  0
    set txbfm_delta   0
    set txndp_delta   0
    set rxsf_delta    0
    set sf_ratio      0
    set txndp_delta_tmp 0
    set rxsf_delta_tmp  0

    # Initialize global counters if not yet
    if {![info exists ::last_txbfm_counter]} {
        set ::last_txbfm_counter 0
        set ::last_txndp_counter 0
        set ::last_rxsf_counter  0
    }

    # If 'dump txbf' is not supported on older chips, return a null list.
    set nulllist " , , , ,"

    # Counter size is 16 bits (65535)
    #set ceiling 65535

    # get $AP wl dump txbf
    UTF::Message INFO "" "Dump AP txbf after iperf"
    set retcode [catch {$AP wl dump txbf} ret] 
    UTF::save_device_state $AP $ret
    if {$retcode != 0 || $ret == "N/A"} {
        UTF::Message ERROR "" "ap_txbf_stats: ret=$ret"
        return $nulllist
    }

    # Sample data from wl dump txbf:
    #   txndpa 3314 txndp 3314 txsf 0 txcwrts 0 txcwcts 0 txbfm 19022
    #   rxndpa 0 bferptrdy 0 rxsf 3101 rxcwrts 0 rxcwcts 0    
     
    # Try to match "txbfm 19022" 
    if {[regexp -line {\s+txbfm\s+(\d+)} $ret - txbfm_counter]} {
	set txbfm_delta [expr {$txbfm_counter - $::last_txbfm_counter}]
    	if {$txbfm_delta < 0} {
	    #set txbfm_delta [expr {$ceiling - $::last_txbfm_counter + $txbfm_counter}]
	    set txbfm_delta 0
   	}
	UTF::Message INFO "" "ap_txbf_stats: txbfm=$txbfm_counter last_txbfm=$::last_txbfm_counter delta=$txbfm_delta"
	set ::last_txbfm_counter $txbfm_counter
    }

    # Try to match "txndp 3314"
    if {[regexp -line {\s+txndp\s+(\d+)} $ret - txndp_counter]} {
	set txndp_delta [expr {$txndp_counter - $::last_txndp_counter}]
	if {$txndp_delta < 0} {
	    #set txndp_delta [expr {$ceiling - $::last_txndp_counter + $txndp_counter}]
	    set txndp_delta 0
 	}
	UTF::Message INFO "" "ap_txbf_stats: txndp=$txndp_counter last_txndp=$::last_txndp_counter delta=$txndp_delta"
	set ::last_txndp_counter $txndp_counter
	set txndp_delta_tmp $txndp_delta
    }

    # Try to match "rxsf 3101"
    if {[regexp -line {\s+rxsf\s+(\d+)} $ret - rxsf_counter]} {
	set rxsf_delta [expr {$rxsf_counter - $::last_rxsf_counter}]
	if {$rxsf_delta < 0} {
	    #set rxsf_delta [expr {$ceiling - $::last_rxsf_counter + $rxsf_counter}]
	    set rxsf_delta 0
	}
        UTF::Message INFO "" "ap_txbf_stats: rxsf=$rxsf_counter last_rxsf=$::last_rxsf_counter delta=$rxsf_delta"
	set ::last_rxsf_counter $rxsf_counter
	set rxsf_delta_tmp $rxsf_delta
    }

    # Calculate ratio of rxsf and txndp
    if {$txndp_delta_tmp > 50} {
	if {$::mumode == "mu"} {
	    set sf_ratio [expr {(($rxsf_delta_tmp / double($txndp_delta_tmp)) / [llength $stalist]) * 100}]
	} else {
	    set sf_ratio [expr {($rxsf_delta_tmp / double($txndp_delta_tmp)) * 100}]
	}
	set sf_ratio [expr round($sf_ratio)]
    } else {
	set sf_ratio 0
    }
    UTF::Message INFO "" "ap_txbf_stats: sfratio=$sf_ratio"

    set result ""
    foreach list [list $txbfm_delta $txndp_delta $rxsf_delta $sf_ratio] {
	if {$list == 0} {
	    set list ""
	}
	append result " ${list}," ;# Comma is required so that data is properly populated in .csv file.
    }

    # log & return result
    UTF::Message INFO "" "ap_txbf_stats: result=$result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::ap_mutx_stats] [arg AP] [arg STAs]]

    # Dump [arg AP] mutx. 
    # Returns CSV strings: "ApTxAsMuSta0 ... ApTxAsMuStaN", "ApTxTotSta0 ... ApTxTotStaN" 
}

proc UTF::ap_mutx_stats {AP stalist} {

    # Generate null list.
    foreach s $stalist {
	lappend nulllist 0
    }
    set nulllist [list $nulllist $nulllist] ;# To match what to be returned.

    # If AP is known to be in trouble, return.
    set var "::${AP}_state"
    if {[set $var] != "OK"} {
        return $nulllist
    }

    # Get $AP wl dump mutx.
    set retcode [catch {$AP wl dump mutx} ret]
    UTF::save_device_state $AP $ret
    if {$retcode != 0 || $ret == "N/A"} {
        UTF::Message ERROR "" "ap_mutx_stats: retcode=$retcode ret=$ret"
        return $nulllist
    }

    # Sample data from wl dump mutx with 4 clients.
    # ===============================================================
    # MU BFR capable and configured; MU feature is ON, mutx is ON, AC policy is ON
    # BW policy = 3 AC policy = 0
    # Maximum MU clients: 8
    # MU clients:
    # [0] 00:90:4c:74:c0:8b rssi -41 nrx 1
    # BSS: wl0.0 00:90:4c:1d:27:28
    # mutx: tot_tx 106647 tx_as_mu 105027 nlost_mu 0 nlost_su 0
    # [1] 00:90:4c:74:c3:b1 rssi -42 nrx 1
    # BSS: wl0.0 00:90:4c:1d:27:28
    # mutx: tot_tx 106653 tx_as_mu 104614 nlost_mu 0 nlost_su 0
    # [2] 00:90:4c:74:c3:4f rssi -43 nrx 1
    # BSS: wl0.0 00:90:4c:1d:27:28
    # mutx: tot_tx 106263 tx_as_mu 104595 nlost_mu 0 nlost_su 0
    # [3] 00:90:4c:74:c3:49 rssi -41 nrx 1
    # BSS: wl0.0 00:90:4c:1d:27:28
    # mutx: tot_tx 106161 tx_as_mu 104451 nlost_mu 0 nlost_su 0
    # Total mutx: tx_as_mu 418687 nlost_mu 0 nlost_su 0
    # ===============================================================

    set tx_as_mu {}
    set tx_as_mu_tmp {}
    set tx_tot {}
    set tx_tot_tmp {}
    set all_sta_mac {}

    # Match STA macaddr.
    # Following line code generates (use \[\d+\] so not match AP macaddr; use ?: so not return sub-match):
    # sta_mac_list={[0] 00:90:4c:74:c0:8b} {[1] 00:90:4c:74:c3:b1} {[2] 00:90:4c:74:c3:4f} {[3] 00:90:4c:74:c3:49}
    set sta_mac_list [regexp -nocase -all -inline {\[\d+\]\s+(?:[0-9a-f]{2}:){5}[0-9a-f]{2}} $ret]

    # Extract STA macaddr.
    # Following line code generates:
    # sta_mac_list=00:90:4c:74:c0:8b 00:90:4c:74:c3:b1 00:90:4c:74:c3:4f 00:90:4c:74:c3:49
    set sta_mac_list [regexp -nocase -all -inline {(?:[0-9a-f]{2}:){5}[0-9a-f]{2}} $sta_mac_list]

    # Match 'tx_as_mu 105027'. The $tx_as_mu list variable will contain following afterwards:
    # tx_as_mu = { tx_as_mu 105027} { tx_as_mu 104614} { tx_as_mu 104595} { tx_as_mu 104451} { tx_as_mu 418687}
    set tx_as_mu [regexp -all -inline {\s+tx_as_mu\s+\d+} $ret]

    # Extract frame numbers except last one which is for Total and we don't need it.
    # After this code block, $tx_as_mu should contain:
    # tx_as_mu = 105027 104614 104595 104451
    for {set i 0} {$i < [llength $tx_as_mu] - 1} {incr i} {
        regexp -line {(\d+)} [lindex $tx_as_mu $i] - j
        lappend tx_as_mu_tmp $j
    }

    # Match 'tot_tx 106647' - it is total frames transmitted (su + mu). 
    # Use same logic as above, except we do need last one.
    set tx_tot [regexp -all -inline {\s+tot_tx\s+\d+} $ret]
    for {set i 0} {$i < [llength $tx_tot]} {incr i} {
        regexp -line {(\d+)} [lindex $tx_tot $i] - j
        lappend tx_tot_tmp $j
    }

    # Qualify results by STA macaddr to make sure that a result is for a right STA.
    set tx_as_mu {} ;# reset
    set tx_tot {}   ;# reset
    foreach sta $stalist {
	set i 0
	set muframes ""
	set totframes ""
	set stamac [$sta macaddr]
	lappend all_sta_mac $stamac ;# for printing log only
	foreach logmac $sta_mac_list {
	    if {$stamac == $logmac} {
		set muframes [lindex $tx_as_mu_tmp $i] 
		set totframes [lindex $tx_tot_tmp $i] 
	    }
	    incr i
	}

	if {$muframes == ""} {
	    set muframes 0
	    set totframes 0
	}

	lappend tx_as_mu $muframes
	lappend tx_tot $totframes
    }

    # log & return result
    UTF::Message INFO "" "ap_mutx_stats: stalist=$stalist"
    UTF::Message INFO "" "ap_mutx_stats: stamaclist=$all_sta_mac"
    UTF::Message INFO "" "ap_mutx_stats: tx_as_mu=$tx_as_mu"
    UTF::Message INFO "" "ap_mutx_stats: tx_tot=$tx_tot"
    return [list $tx_as_mu $tx_tot]
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::ap_pktq_stats] [arg AP] [opt nohistograms=0] [opt is_oem_ap=0]]

    # Queries the [arg AP] packet common queue stats if [opt nohistograms] is 0.
    # If [opt is_oem_ap] is 1, leaves the [arg AP] alone.[para]

    # Returns CSV string with: ApPktRequested, ApPktStored, ApPktDropped, 
    # ApPktRetried, ApPktRtsFailed, ApPktRetryDropped, ApPktAcked, ApPktRetryRatio, ApPktRetryDropRatio,
}

proc UTF::ap_pktq_stats {AP {nohistograms 0} {is_oem_ap 0}} {
    UTF::Message INFO "$AP" "ap_pktq_stats AP=$AP nohistograms=$nohistograms is_oem_ap=$is_oem_ap"

    # If AP is known to be in trouble, or nohistograms!=0, or is_oem_ap!=0, return.
    set msg " , , , , , , , , ," ;# Need 1 comma per field normally returned
    set var "::${AP}_state"
    if {[set $var] != "OK" || $nohistograms != "0" || $is_oem_ap != "0"} {
        return $msg
    }

    # get wl pktq_stats
    set catch_resp6 [catch {$AP wl pktq_stats} catch_msg6]
    UTF::save_device_state $AP $catch_msg6
    if {$catch_resp6 != 0 || $catch_msg6 == "N/A"} {
	UTF::Message ERROR "$AP" "catch_msg=$catch_msg6"
	return $msg
    }

    # Sample data from wl pktq_stats
    # prec:		Precedence
    # rqstd:		Number packets sent by OS
    # stored:		Number packets accepted into the driver  
    # dropped:		Number packets REJECTED by the driver (capacity limit) 
    # retried:		Count of retries of packets in the air
    # rtsfail:		Count of RTS not receiving CTS		
    # rtrydrop:		Count of packets in the air ABANDONED due to retry limit hit
    # psretry:		Number of packets saved by PS pretend 
    # acked:		Number of packet ACKs received
    # utlisatn:		Max number of packets queued at once 		
    # q length:		Max capacity to queue packets
    # Data Mbits/s:	Data throughput (average since previous)
    # Phy Mbits/s: 
    # v5:		Version of pktq_stats command
    #
    # prec:   rqstd,  stored, dropped, retried, rtsfail,rtrydrop, psretry,   acked,utlisatn,q length,Data Mbits/s,Phy Mbits/s (v5)
    #   00:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   01:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   02:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   03:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   04:  121329,  121329,       0,    4125,      17,       0,       0,  121329,      31,    1368,   143.62,    312.65
    #   05:      10,      10,       0,       -,       -,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   06:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   07:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   08:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   09:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   10:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   11:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   12:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   13:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   14:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
    #   15:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1368,     0.00,      0.00
 
    
    set rqstd_sum    	0
    set stored_sum   	0
    set dropped_sum  	0
    set retried_sum  	0
    set rtsfail_sum  	0
    set rtrydrop_sum 	0
    set acked_sum    	0
    set retry_ratio  	0
    set retrydrop_ratio 0

    # Parse the data - processing row by row
    foreach i [list 00: 01: 02: 03: 04: 05: 06: 07: 08: 09: 10: 11: 12: 13: 14: 15:] {

	if {[regexp -nocase [subst -nocommands -nobackslashes {\s*$i\s+((\s*(\d+|-),\s*){10,}){1}}] $catch_msg6 pktq_stats]} {
	    # Data is in format of "4215,"
	    set rqstd    [lindex $pktq_stats 1]	    
    	    set stored   [lindex $pktq_stats 2]
    	    set dropped  [lindex $pktq_stats 3]
    	    set retried  [lindex $pktq_stats 4]
    	    set rtsfail  [lindex $pktq_stats 5]
    	    set rtrydrop [lindex $pktq_stats 6]
	    set acked    [lindex $pktq_stats 8]

	    # Replace "-" with "0" to satisfy addition operation
            regsub {\-} $rqstd    "0" rqstd
            regsub {\-} $stored   "0" stored
            regsub {\-} $dropped  "0" dropped
            regsub {\-} $retried  "0" retried
            regsub {\-} $rtsfail  "0" rtsfail
            regsub {\-} $rtrydrop "0" rtrydrop
            regsub {\-} $acked    "0" acked

 	    # Remove trailing "," to satisfy addition operation
	    regsub {,} $rqstd    "" rqstd
	    regsub {,} $stored   "" stored
	    regsub {,} $dropped  "" dropped
	    regsub {,} $retried  "" retried
	    regsub {,} $rtsfail  "" rtsfail
	    regsub {,} $rtrydrop "" rtrydrop
	    regsub {,} $acked    "" acked

   	    # Get sum for each column	
	    set rqstd_sum    [expr {$rqstd_sum    + $rqstd}] 
	    set stored_sum   [expr {$stored_sum   + $stored}] 
	    set dropped_sum  [expr {$dropped_sum  + $dropped}] 
	    set retried_sum  [expr {$retried_sum  + $retried}] 
	    set rtsfail_sum  [expr {$rtsfail_sum  + $rtsfail}] 
	    set rtrydrop_sum [expr {$rtrydrop_sum + $rtrydrop}] 
	    set acked_sum    [expr {$acked_sum    + $acked}]
    	}
    }

    # Calculate retry_ratio and retrydrop_ratio
    if {$stored_sum <= 0} {
	# Can't divided by zero
	set retry_ratio 0
	set retrydrop_ratio 0
    } else {
	# Normal case
	set retry_ratio [expr {1 - ($acked_sum / double($stored_sum + $retried_sum))}]
	set retry_ratio [expr round($retry_ratio * 100)]

	set retrydrop_ratio [expr {$rtrydrop_sum / double($stored_sum)}]
	set retrydrop_ratio [expr round($retrydrop_ratio * 100)]
    }

    set result ""
    foreach list [list $rqstd_sum $stored_sum $dropped_sum $retried_sum $rtsfail_sum $rtrydrop_sum $acked_sum $retry_ratio $retrydrop_ratio] {
	append result " ${list}," ;# Comma here is required so data will be populated in .csv properly.
    }  

    # log & return result
    UTF::Message INFO "" "pktq_stats: rqstd=$rqstd_sum stored=$stored_sum dropped=$dropped_sum retried=$retried_sum rtsfail=$rtsfail_sum rtrydrop=$rtrydrop_sum acked=$acked_sum retry_ratio=$retry_ratio retrydrop_ratio=$retrydrop_ratio"
    UTF::Message INFO "" "pktq_stats: result=$result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::ap_stats] [arg AP] [arg STA] [arg StaMac] [opt is_oem_ap=0]]

    # Queries the [arg AP] stats for [arg STA].
    # If [opt is_oem_ap] is 1, leaves the [arg AP] alone.[para]

    # Returns CSV string with: apAssocAuthen, apConnectTime, apIdleTime, apMcs, apNss, apBw,
    # apRxRate, apState, apTxFailures, apTxPkts, apTxRate,
}

proc UTF::ap_stats {AP STA StaMac {is_oem_ap 0}} {
    UTF::Message INFO "$AP" "ap_stats: AP=$AP STA=$STA StaMac=$StaMac is_oem_ap=$is_oem_ap"

    # Looks like STA is no longer used!

    # If AP is known to be in trouble or is OEM AP, return.
    set msg " , , , , , , , , , ,"
    set var "::${AP}_state"
    if {[set $var] != "OK" || $is_oem_ap != "0"} {
        return $msg
    }

    # Initialize results. gnuplot responds better to 0 than n/a.
    set apAssocAuthen 0 
    set apConnectTime 0
    set apIdleTime 0
    set apMcs ""
    set apNss ""
    set apBw ""
    set apRxRate 0
    set apState "NULL"
    set apTxFailures 0
    set apTxPkts 0
    set apTxRate 0

    # Get sta_info from AP
    set catch_resp1 [catch {$AP wl sta_info $StaMac} catch_msg1]
    if {$catch_resp1 != 0} {
        UTF::Message ERROR "$AP" "catch_msg1=$catch_msg1"
    }
    UTF::save_device_state $AP $catch_msg1

    # Get apTxRate
    if {[regexp -nocase {last\s*tx\s*pkt:\s*(\d+)\s*kbps} $catch_msg1 - rate]} {
        set apTxRate [expr double($rate) / 1000] ;# convert to Mb/s
    } else {
        # Try wl rate command instead
        if {[set $var] != "OK"} {
            return $msg
        }
        set catch_resp2 [catch {$AP wl rate} catch_msg2]
        UTF::save_device_state $AP $catch_msg2
        # puts "catch_msg2=$catch_msg2"
        if {[regexp -nocase {([\.\d]+)\s*Mbps} $catch_msg2 - rate]} {
            set apTxRate $rate ;# rate is already in Mb/s
        } else {
            set apTxRate 0
        }
    }

    # Get apIdleTime
    if {[regexp -nocase {idle\s*(\d+)\s*seconds} $catch_msg1 - time]} {
        set apIdleTime $time
    }

    # Get apConnectTime
    if {[regexp -nocase {in\s*network\s*(\d+)\s*seconds} $catch_msg1 - time]} {
        set apConnectTime $time
    }

    # Get apState, save as global.
    if {[regexp -nocase {state:\s*(.*)flags} $catch_msg1 - state]} {
        set apState [string trim $state]
    }
    set apState [string toupper $apState]
    set temp "::${AP}_assoc"
    set $temp $apState
    set temp "::${AP}_msg"
    set $temp $catch_msg1

    # Convert apState to a numeric value that can be plotted as a line graph.
    if {[regexp -nocase {ASSOCIATED} $apState] && [regexp {AUTHENTICATED} $apState]} {
        set apAssocAuthen $::ap_scale_factor
    } elseif {![regexp -nocase {NULL} $apState]} {
        set apAssocAuthen [expr $::ap_scale_factor / 2]
    }

    # Get apTxPkts
    if {[regexp -nocase {tx\s*pkts:\s*(\d+)} $catch_msg1 - pkts]} {
        set apTxPkts $pkts
    }

    # Get apTxFailures
    if {[regexp -nocase {tx\s*failures:\s*(\d+)} $catch_msg1 - fail]} {
        set apTxFailures $fail
    }

    # Get apRxRate
    if {[regexp -nocase {last\s*rx\s*pkt:\s*(\d+)\s*kbps} $catch_msg1 - rate]} {
        set apRxRate [expr double($rate) / 1000] ;# convert to Mb/s
    }

    # Get apMcs, apNss and apBw
    if {[set $var] != "OK"} {
        return $msg
    }
    set catch_resp4 [catch {$AP wl nrate} catch_msg4]
    UTF::save_device_state $AP $catch_msg4
    if {[regexp -nocase {mcs\s*index\s*(\d+)} $catch_msg4 - mcs]} {
    	# Sample nrate msg for 11n: "mcs index 15 stf mode 3 auto"
        set apMcs $mcs
    } elseif {[regexp -nocase {vht\s*mcs\s*(\d+)\s*Nss\s*(\d+).*bw(\d)} $catch_msg4 - mcs nss bw]} {
    	# Sample nrate msg for 11ac: "vht mcs 9 Nss 3 Tx Exp 0 bw80 ldpc sgi auto"
        set apMcs $mcs
        set apNss $nss
        set apBw  $bw
    }

    # Return CSV formatted results.
    UTF::Message INFO "" "ap_stats: apAssocAuthen=$apAssocAuthen\
         apConnectTime=$apConnectTime apIdleTime=$apIdleTime\
         apMcs=$apMcs apNss=$apNss apBw=$apBw apRxRate=$apRxRate apState=$apState\
         apTxFailures=$apTxFailures apTxPkts=$apTxPkts apTxRate=$apTxRate"
    return "$apAssocAuthen, $apConnectTime, $apIdleTime, $apMcs, $apNss, $apBw, $apRxRate, $apState, $apTxFailures, $apTxPkts, $apTxRate,"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::auth_devices]]

    # Used to authorize yourself with all the devices in a testbed
    # that use SSH the first time you use the testbed.
}

proc UTF::auth_devices { } {

    # Authorize all devices that support SSH access: Cygwin, Linux, MacOS
    # Objects that delegate to Cygwin or Linux or MacOS are also picked up.
    catch {::AppleCore configure -onall false}
    UTF::OnAll UTF::Cygwin authorize
    UTF::OnAll UTF::Linux authorize
    UTF::OnAll UTF::MacOS authorize
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::CCDF] [arg list] [opt low_lim=0.001]]

    # Queries the [arg list] CCDF with [opt low_lim] is 0.001.

    # Returns CCDF_ratio list
}

proc UTF::CCDF {list {low_lim 0.001}} {

    # Calculate delay_CCDF list
    set delay_CCDF 0
    set CCDF_ratio ""
    set total 0
    foreach j $list {
        set total [expr $total + $j]
    }
    set delay_CCDF $total
    set t $total
    foreach j $list {
	set t [expr $t - $j]
	lappend delay_CCDF $t
    }
    set delay_CCDF [lreplace $delay_CCDF end end]

    # Calculate CCDF_ratio list
    # Total is the first number in value list
    set total [lindex $delay_CCDF 0]
    if {$total == 0} {
	set total 1 ;# prevent divide by 0 errors
    }
    foreach z $delay_CCDF {
	# Convert data to percentage
	set z [UTF::clean_number $z]
	# puts "z=$z"
	set z [expr double($z * 100) / $total]
	set z [format "%.2f" $z]
	if {$z < $low_lim} {
	    set z $low_lim
	}
	lappend CCDF_ratio $z
    }
    return $CCDF_ratio
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_chanspec] [arg sta_list] [arg chanspec]]

    # Throws an error if any of the STA in [arg sta_list] are NOT on
    # the specified [arg chanspec]. Otherwise returns null.
}

proc UTF::check_chanspec {sta_list chanspec } {

    # Sanity checks
    set sta_list [string trim $sta_list]
    set chanspec [string trim $chanspec]
    if {$sta_list == "" || $chanspec == ""} {
        error "check_chanspec ERROR: null parameter(s) sta_list=$sta_list chanspec=$chanspec"
    }

    # Check STA are all on the correct chanspec.
    set success 0
    set cnt 0
    UTF::Try "Check chanspec $chanspec" {
        UTF::Message INFO "" "check_chanspec: sta_list=$sta_list chanspec=$chanspec"
        set err_list ""

        # Process all items, so we see exactly who is on what channel.
        foreach item $sta_list {
            if {[UTF::is_oem_ap $item]} {
                UTF::Message WARN $item "check_chanspec: skipping $item, OEMAP"
                continue
            }
            incr cnt
            set ch ""
            set ch [$item wl chanspec]
            set ch [lindex $ch 0] ;# grab first token
            if {$ch == $chanspec} {
                UTF::Message INFO $item "check_chanspec: $item chanspec $ch"
                incr success
            } else {
                UTF::Message WARN $item "check_chanspec: $item chanspec $ch NE $chanspec"
                append err_list " $item chanspec $ch NE $chanspec"
            }
        }

        # Show errors, if any.
        if {$err_list == ""} {
            return
        } else {
	    UTF::Message WARN $item "$err_list"
        }
    }

    # Check for failures.
    if {$cnt == $success} {
        return
    } else {
        #error "Skipping $chanspec"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_connection_list] [arg i] [arg conn_list] [arg softap=0]]

    # Used to validate a connection list. [arg i] is the list number,
    # just used for log trace info. [arg conn_list] is expected
    # to be a space separated list of valid UTF STA object names.
    # The first item in the list is expected to be a Router STA. The
    # second and subsequent items are expected to be endpoint STAs.[para]

    # [arg softap]=1 indicates that the first item in the list will function
    # as a Router STA. If the first item has the -ap option set to 1, 
    # then you dont need to specify [arg softap]=1.[para]

    # Returns: OK, ERROR
}

proc UTF::check_connection_list {i conn_list {softap 0}} {

    # Ignore null lists.
    set conn_list [string trim $conn_list]
    if {$conn_list == ""} {
        return OK
    }

    # Log the calling data
    UTF::Message INFO "" "UTF::check_connection_list: Checking (conn$i)=$conn_list softap=$softap"

    # There must be at least 2 items in the list.
    set status OK
    if {[llength $conn_list] < 2} {
        UTF::Message ERROR "" "UTF::check_connection_list: ERROR: need at least 2 items in list"
        set status ERROR
    }

    # Check router item & -ap option
    # NB: The plan is for the real APs to have -ap=1 by default.
    # NB: Calling arg softap will be 0 most of the time, so -ap takes precedance.
    set ap_opt 0
    set rtr [lindex $conn_list 0]
    set rtr [string trim $rtr]
    set catch_resp [catch {set ap_opt [$rtr cget -ap]} catch_msg]
    UTF::Message INFO "" "$rtr has -ap=$ap_opt"
    if {[string is true -strict $ap_opt] || [string is true -strict $softap]} {
        # Router can be any valid WLAN STA type
        set catch_resp [catch {UTF::check_sta_type $rtr $::ap_type_list $::sta_type_list} catch_msg]
    } else {
        # Router must be real AP only
        set catch_resp [catch {UTF::check_sta_type $rtr $::ap_type_list} catch_msg]
    }
    if {$catch_resp != 0} {
        UTF::Message ERROR "" "$catch_msg"
        set status ERROR
    }

    # Check STA items.
    set sta [lrange $conn_list 1 end]
    set sta [string trim $sta]
    foreach item $sta {
        set catch_resp [catch {UTF::check_sta_type $item $::sta_type_list} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR "" "$catch_msg"
            set status ERROR
        }
    }

    # Return the overall status
    return $status
}

#====================================================================

UTF::doc {
    # [call [cmd UTF::check_disk_usage] [arg STAs]]

    # Used to check the disk utilization on one or more [arg STAs]. [arg STAs]
    # can be a list of high level device objects or a list of STA objects.
    # If STA is an AP, the associated lanpeer & wanpeer PC are checked.
    # Puts error messages on the main UTF web report if any of the device have
    # disk utilization GE 90%.[para]

    # Returns null.
}

proc UTF::check_disk_usage { STAs } {

    # Create list of valid STA, including lanpeer & wanpeer for AP.
    set err_list ""
    set sta_list ""
    foreach STA $STAs {

        # Get log file name for STA
        set name [UTF::get_name $STA]

        # Check name exists in config file.
        set type ""
        set catch_resp [catch {set type [$STA info type]} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR $name "check_disk_usage: $catch_msg"
            append err_list " $name not defined in config file<br>"
            continue
        }

        # Expand AP to cover lanpeer & wanpeer.
        if {[UTF::is_ap $STA]} {
            # AP
            foreach item "lanpeer wanpeer" {
                set val ""
                catch {set val [$STA cget -${item}]}
                if {[llength $val]} {
                    foreach v $val {
                        set parent [$v cget -host]
                        set host [$parent cget -name]
                        append sta_list " $host"
                    }
                }
            }
        } else {
            # STA
            append sta_list " $name"
        }
    }
    set sta_list [lsort -unique $sta_list]
    UTF::Message INFO "" "check_disk_usage: sta_list=$sta_list"

    # Check disk utilization of each STA
    foreach item $sta_list {
    
        # Check disk utilization.
        # NB: dont ask for /cygdrive as that is Windows specific
        set catch_resp [catch {set df [$item rexec -n df . ]} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR $item "check_disk_usage: df: $catch_msg"
            append err_list " $item could not get df<br>"
            continue
        }
    
        # Parse out nn%
        if {![regexp {\s+(\d+)\s*%} $df - percent]} {
            UTF::Message ERROR $item "check_disk_usage: parsing failure, df=$df"
            append err_list " $item parsing failure, df=$df<br>"
            continue
        }

        # If dest >= 90% full, send error.
        if {$percent >= 90} {
            UTF::Message ERROR $item "check_disk_usage: ${percent}% full!!!"
            append err_list " $item ${percent}% full!!!<br>"
        } else {
            UTF::Message INFO $item "check_disk_usage: ${percent}% full"
        }    
    }

    # If there are any error msgs, push them on to UTF web page.
    if {$err_list != ""} {
        UTF::Try "Check_Disk_Usage" {
            error "html: <font color=\"red\"><b>$err_list</b></font>"
        }
    }
}

#====================================================================

UTF::doc {
    # [call [cmd UTF::check_host_os] [arg name] [opt mode=""]]

    # Used to check the host OS of [arg name]. [arg name] can be a high
    # level device object or a STA object.[para]

    # Returns one of: Linux_fcN, WinXP, Vista, Win7, MacOS, null

    # If mode=dbux, returns one of: Linux, WinXP, Win7, Win8, Win9, MacOS, null  
    # (The simplified OS names are requested by dBux team) 
}

proc UTF::check_host_os { name {mode ""} } {

    # Check name exists in config file.
    set catch_resp [catch {set type [$name info type]} catch_msg]
    if {$catch_resp != 0} {
        error "UTF::check_host_os ERROR: object name=$name is not defined in the config file!"
    }

    # If object is a STA, get the parent object name & type.
    if {$type == "::UTF::STA"} {
        set parent_name [$name cget -host]
        set parent_type [$parent_name info type]
    } else {
        set parent_name $name
        set parent_type $type
    }
    set parent_type [lindex [split $parent_type "::"] end] ;# get tail end of type
    
    # Look for Windows objects.
    if {[lsearch -exact "Cygwin WinBT WinDHD" $parent_type] >= 0} {

        # WinBT objects rely on querying the actual OS to get the
        # correct version configured in -osver
        if {$parent_type == "WinBT"} {
            $name checkOs
        }

        # NB: MS-DOS reports Vista=6.0 & Win7=6.1, which is what Cygwin shows.
        # Translate osver to text string. UTF config files set osver=7 for Win7, osver=8 for Win8.
        set ver [$name cget -osver]
        if {[regexp {^5} $ver]} {
            return "WinXP"
        } elseif {[regexp {^6} $ver]} {
            return "Vista"
        } elseif {[regexp {^7} $ver]} {
            return "Win7"
        } elseif {[regexp {^8$} $ver]} {
            return "Win8"
        } elseif {[regexp {^864$} $ver]} {
	    if {$mode == "dbux"} {
		return "Win8"
	    } else {
            	return "Win8_64bit"
	    }
        } elseif {[regexp {^81$} $ver]} {
            if {$mode == "dbux"} {
                return "Win8"
            } else {
                return "Win8.1"
            }
        } elseif {[regexp {^8164$} $ver]} {
            if {$mode == "dbux"} {
                return "Win8"
            } else {
                return "Win8.1_64bit"
            }
        } elseif {[regexp {^9$} $ver]} {
            return "Win9"
        } elseif {[regexp {^964$} $ver]} {
            if {$mode == "dbux"} {
                return "Win9"
            } else {
                return "Win9_64bit"
            }
        } else {
            return "Unknown"
        }

    # Look for Linux objects
    } elseif {[lsearch -exact "Linux DHD Sniffer" $parent_type] >=0} {
        # Get fcN version from host as UTF object dont have a method to access
        # the kernel variable in each Linux based object.
        set ver [$name uname -r] 
        set fc_ver ""
        regexp -nocase {[\._](fc\d+)} $ver - fc_ver
	if {$mode == "dbux"} {
	    return "Linux"
	} else {
            return "Linux_${fc_ver}"
	}

    # Look for MacOS objects
    } elseif {[lsearch -exact "MacOS" $parent_type] >= 0} {
        return MacOS
    } elseif {[lsearch -exact "HSIC" $parent_type] >= 0} {
        return Linux

    } else {
        # Everything else returns null.
        return ""
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_host_type] [arg name] [lb][arg args][rb]]

    # Used to check the object type of [arg name]. [arg name] can be a high
    # level device object or a STA object. [arg args] can be a list of object
    # types that the host type will be checked against.[para]

    # When there are no args, returns object type string.
    # When there are args, returns 1 if host type matches one of the args,
    # otherwise 0.
}

proc UTF::check_host_type {name args} {

    # Get host object type.
    set type [$name info type]
    set type [lindex [split $type "::"] end]
    # puts "check_host_type name=$name args=$args type=$type"

    # For STA objects, get the type of the parent object.
    if {$type =="STA"} {
        set parent [$name cget -host]
        set type [$parent info type]
        set type [lindex [split $type "::"] end]
        # puts "check_host_type parent=$parent type=$type"
    }

    # If there are no additional arguments, simply return the object type.
    set args [string trim $args]
    if {$args eq ""} {
        return $type
    }

    # If type matches any of the args, return 1, otherwise 0.
    # puts "check_host_type args=$args type=$type"
    if {[lsearch -exact $args $type] >= 0} {
        return 1
    } else {
        return 0
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_image_pattern] [arg pattern]]
    # [arg pattern] pattern describing the directory path to 
    # be searched for images.[para]

    # Because it is very hard and frustrating to figure out which 
    # parameter in a pattern / path is wrong and causing no images
    # to be found, this routine provides error checking of the directory
    # structure. Basically we descend the pattern / path one directory
    # at a time, verifying that the pattern is valid and may have a chance
    # of finding some images.[para]

    # Returns OK or ERROR
}

proc UTF::check_image_pattern {pattern} {

    # Use a non-greedy regsub pattern to remove items in curly 
    # braces. Pattern is: open-brace non-braces close-brace.
    UTF::Message INFO "" "check_image_pattern checking: $pattern"
    regsub -all {\{[^{^}]*\}} $pattern "" pattern
    # puts "pattern=$pattern"

    # Separate the pattern into list of directories & the file.
    set dir_list [file dirname "$pattern"]
    set dir_list [split $dir_list "/"]
    set file [file tail "$pattern"]
    # puts "dir_list=$dir_list file=$file"

    # Check each directory exists, starting at the top of the tree.
    set dir ""
    foreach item $dir_list {

        # Skip nulls.
        set item [string trim $item]
        # puts "item=$item"
        if {$item == ""} {
            continue
        }

        # For wildcards, expand the wildcard to a specific directory
        # that matches the wildcard.
        if {[regexp {\*} $item]} {
            set sub_dir_list [exec ls $dir]
            regsub -all {\n} $sub_dir_list " " sub_dir_list
            set sub_dir_list [lsort -decreasing $sub_dir_list]
            UTF::Message INFO "" "expanding $item, found: $sub_dir_list"
            foreach sub_item $sub_dir_list {
                # puts "item=$item sub_item=$sub_item"
                if {$item == "*" || [string match "*$item*" $sub_item]} {
                    # If this sub_item is a directory, use it.
                    if {[file isdirectory "$dir/$sub_item"]} {
                        set item $sub_item
                        UTF::Message INFO "" "using: $item"
                        break
                    }
                }
            }
        }

        # Check this specific directory exists.
        set dir "${dir}/${item}"
        if {[file isdirectory "$dir"]} {
            UTF::Message INFO "" "found: $dir"
        } else {
            UTF::Message ERROR "" "directory=$dir not found"
            return ERROR
        }
    }

    # Now check for files.
    set file_list [exec ls $dir]
    regsub -all {\n} $file_list " " file_list
    UTF::Message INFO "" "found files: $file_list"
    if {[string match "*$file*" $file_list]} {
        UTF::Message INFO "" "found at least one match for: $file"
        return OK
    } else {
        UTF::Message ERROR "" "directory path is OK, but no\
            matching files found for: $file"
        return ERROR
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_mac_addr] [arg sta_list]]

    # Throws an error if any of the STA in [arg sta_list] are using
    # the same MAC address. Otherwise returns null.
}

proc UTF::check_mac_addr {sta_list} {

    # Sanity checks
    set sta_list [lsort -unique $sta_list]
    if {$sta_list == ""} {
        error "check_mac_addr ERROR: sta_list must not be null!"
    }

    # Get MAC addr for each STA.
    set mac_list ""
    UTF::Message INFO "" "check_mac_addr sta_list=$sta_list"
    foreach item $sta_list {
        if {[UTF::is_oem_ap $item]} {
            UTF::Message WARN $item "check_mac_addr skipping $item, OEMAP"
            continue
        }
        set catch_resp [catch {set mac [$item macaddr]} catch_msg]
        if {$catch_resp != 0} {
            error "ERROR: check_mac_addr $item could not get MAC address, $catch_msg"
        }
        lappend mac_list $mac
    }
    UTF::Message INFO "" "check_mac_addr mac_list=$mac_list"

    # Check for duplicate MAC addresses.
    set sta_cnt [llength $mac_list]
    set unique_mac [lsort -unique $mac_list]
    set mac_cnt [llength $unique_mac]
    if {$sta_cnt == $mac_cnt} {
        return
    } else {
        error "ERROR: check_mac_addr found duplicate MAC address, sta_cnt=$sta_cnt unique_mac=$unique_mac mac_cnt=$mac_cnt"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_pid] [arg object] [arg pid] [arg verbose]]
    # [arg object] host object to check an existing process id on.
    # [arg pid] integer process id to look for.
    # [arg verbose] flag controls logging of pid details. Default=0
    # means no logging. Any other value gives full logging.[para]

    # Returns 1 when process id is found, 0 for process id not found.
}

proc UTF::check_pid {object pid {verbose 0}} {

    # Get object host_os, also validates object is defined in the
    # config file.
    set host_os [check_host_os $object]

    # Check pid is integer
    if {![regexp {^\d+$} $pid]} {
        error "UTF::check_pid ERROR: pid=$pid is not integer,\
            object=$object"
    }

    # Get list of active pids from object
    set pid_list [UTF::get_pid_list $object $verbose]

    # Parse pid_list one line at a time, for each pid.
    set pid_list [split $pid_list "\n"]
    foreach line $pid_list {
        # puts "line=$line"
        # Linux / Cygwin format: userid pid etc
        if {![regexp {\S+\s+(\d+)\s+} $line - x]} {
            set x ""
        }

        # If pid is found, return 1.
        if {$pid == $x} {
            if {$verbose != 0} {
                UTF::Message INFO "" "UTF::check_pid pid=$pid matched x=$x, line=$line"
            }
            return 1
        }
    }

    # pid was not found, return 0.
    return 0
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_pid_by_name] [arg object] [arg name]
    # [arg verbose]]
    # [arg object] host object to check existing process id on.
    # [arg name] process name to be terminated. Wildcard case insensitive 
    # matching of the process table contents is used.
    # [arg verbose] flag controls logging of pid details. Default=0
    # means no logging. Any other value gives full logging.[para]

    # Returns space separated list of integer process id, if any, from the
    # process table contents that match the [arg name].
}

proc UTF::check_pid_by_name {object name {verbose 0}} {

    # Get object host_os, also validates object is defined in the
    # config file.
    set host_os [check_host_os $object]

    # Check name is not null.
    set name [string trim $name]
    if {$name == ""} {
        error "UTF::check_pid_by_name ERROR: name must not be null, object=$object"
    }

    # Get list of active pids from object
    set pid_list [UTF::get_pid_list $object $verbose]

    # Parse pid_list one line at a time, looking for matching names.
    set match_list ""
    set pid_list [split $pid_list "\n"]
    foreach line $pid_list {

        # Use wildcard, case insensitive matching for name.
        if {![regexp -nocase $name $line]} {
            # puts "skipping: $line"
            continue
        }

        # Linux / Cygwin format: userid pid etc
        if {[regexp {\S+\s+(\d+)\s+} $line - x]} {
            # Log & save the matching pids
            UTF::Message INFO "" "UTF::check_pid_by_name: $object $name matched: $line"
            set match_list "$match_list $x"
        }

    }

    # Return the match_list
    set match_list [string trim $match_list]
    if {$match_list == ""} {
        UTF::Message INFO "" "UTF::check_pid_by_name: $object nothing matched $name"
    }
    return $match_list
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_rte_mu_output] [arg sta_name] [arg field_name]
    # [arg op] [arg expected_value]]

    # Used to get the [arg sta_name] serial console "rte mu" output and 
    # parse it for the value of the specified [arg field_name]. If the
    # optional criteria parameters [arg op] [arg expected_value] are specified,
    # then the value found must match the specified criteria.[para]

    # The values for op are: null, EQ, NE, GE, LE. The defaults for [arg op]
    # & [arg expected_value] are null.[para]

    # Returns the value of the requested field_name. If the optional criteria
    # are specified, throws an error if the value does not meet the specified
    # criteria.
}

proc UTF::check_rte_mu_output { sta_name field_name {op ""} {expected_value ""}} {

    # Sanity check on sta_name
    set sta_name [string trim $sta_name]
    if {$sta_name == ""} {
        error "check_rte_mu_output ERROR: sta_name may not be null!"
    }

    # Sanity checks on field_name
    set field_name [string trim $field_name]
    if {$field_name == ""} {
        error "check_rte_mu_output ERROR: sta_name=$sta_name\
            field_name may not be null!"
    }

    # Find the console location, return if not defined.
    set console "" ;# NB: Cygwin object dont have console
    set catch_resp [catch {set console [$sta_name cget -console]} catch_msg]
    set console [string trim $console]
    if {$console == "" || [string match -nocase "*var*log*" $console]} {
       UTF::Message INFO "" "check_rte_mu_output $sta_name does not have a serial console, console=$console"
       return "no serial console"
    }

    # Get the "rte mu" command output.
    UTF::Message INFO "" "check_rte_mu_output checking $sta_name console=$console"
    set console_output ""
    set catch_resp [catch {set console_output [$sta_name rte mu]} catch_msg]
    if {$catch_resp == 0} {
        UTF::Message INFO $sta_name "check_rte_mu_output console_output=$console_output"
    } else {
        error "check_rte_mu_output ERROR sta_name=$sta_name\
            console not responding, catch_msg=$catch_msg\
            \nconsole_output=$console_output"
    }

    # Parse out the value for the desired field_name.
    set match no
    set result ""
    set console_output [split $console_output "\n"]
    foreach line $console_output {
        if {[regexp -nocase "$field_name" "$line"]} {
            # We found the desired field, set the match flag and save remainder of the line.
            set match yes
            regsub -nocase "$field_name" "$line" "" result
            break
        }
    }

    # Did we find the field_name?
    if {$match == "no"} {
        error "check_rte_mu_output ERROR: sta_name=$sta_name \
            field_name=$field_name not found!"
    }

    # Remove trailing/extra data from the result.
    set result [lindex $result 0] ;# take first token only
    set result [split $result "("] ;# remove (nnnK) etc
    set result [lindex $result 0]  ;# should now be simple number/string
    set result [string trim $result]
    UTF::Message INFo $sta_name "check_rte_mu_output field_name=$field_name\
        matched line=$line result=$result op=$op expected_value=$expected_value"

    # Handle the optional criteria checking parameters.
    set op [string trim $op]
    set op [string toupper $op]
    set expected_value [string trim $expected_value]
    if {$op == ""} {
        # No op was specified, so result is not checked.
        return $result

    } elseif {$op == "EQ"} {
        # Check result EQ expected_value.
        if {$result == $expected_value} {
            return $result
        } else {
            UTF::Message ERROR $sta_name "check_rte_mu_output ERROR: result=$result\
                op=$op MISMATCH expected_value=$expected_value"
            error $result
        }

    } elseif {$op == "NE"} {
        # Check result NE expected_value.
        if {$result != $expected_value} {
            return $result
        } else {
            UTF::Message ERROR $sta_name "check_rte_mu_output ERROR: result=$result\
                op=$op MISMATCH expected_value=$expected_value"
            error $result
        }

    } elseif {$op == "GE"} {
        # Check result EQ expected_value.
        if {$result >= $expected_value} {
            return $result
        } else {
            UTF::Message ERROR $sta_name "check_rte_mu_output ERROR: result=$result\
                op=$op MISMATCH expected_value=$expected_value"
            error $result
        }
  
    } elseif {$op == "LE"} {
        # Check result EQ expected_value.
        if {$result <= $expected_value} {
            return $result
        } else {
            UTF::Message ERROR $sta_name "check_rte_mu_output ERROR: result=$result\
                op=$op MISMATCH expected_value=$expected_value"
            error $result
        }

    } else {
        error "check_rte_mu_output ERROR: sta_name=$sta_name\
            field_name=$field_name invalid op=$op, should be: EQ|NE|GE|LE|null"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_sniffer_capture_steps] [arg sniffer] [arg sniffercapture]]

    # Used to check syntax of sniffercapture start/stop steps. Commands
    # are expected to be in pairs, a start point followed by a stop point.
    # To run the sniffer continuously on the RvR rampdown curve for steps 44 thru 50,
    # specify: d44 d50. To run the sniffer continuously on the bottom of the RvR curve
    # for rampdown steps 80 thru 103 thru rampup steps 80, specify: d80 u80. To run
    # the sniffer continuously on the RvR rampup curve for steps 55 thru 45, specify: u55 u45.
    # You can use combinations to specify multiple portions of the RvR curve,
    # as many start/stop pairs as you like, eg: d44 d50 d80 u80 u55 u45 [para]

    # For fastrampup, specify: fnn, where nn is fastrampup sequence number,
    # not the attenuator value. This is done so you can distinguish between
    # many fastrampup steps all using the same attenuator value.[para] 

    # NB: You will be subject to disk space limits on both the sniffer local
    # disk and the projects disk to which the .pcap file is copied. [para]

    # Return null if no issues found, throws an error otherwise.
}

proc UTF::check_sniffer_capture_steps {sniffer sniffercapture} {

    # When sniffercapture is null, we are done.
    set sniffercapture [string trim $sniffercapture]
    if {$sniffercapture == ""} {
        return
    }

    # Check we have a valid sniffer.
    set sniffer [string trim $sniffer]
    if {$sniffer == ""} {
        error "check_sniffer_capture_steps ERROR: sniffer must not be null"
    }
    set resp [UTF::check_host_type $sniffer Sniffer]
    if {$resp == 0} {
        error "check_sniffer_capture_steps ERROR: $sniffer not type Sniffer"
    }

    # Commands must be in pairs.
    set cnt [llength $sniffercapture]
    if {[expr $cnt % 2] != 0} {
        error "check_sniffer_capture_steps ERROR: sniffercapture=$sniffercapture commands must be in pairs, cnt=$cnt"
    }

    # Commands are expected to be in logical sequence flow.
    # First the down steps, then the up steps.
    # For down step pairs, stop step > start step, reverse for up step pairs.
    set last_dir ""
    foreach cmd $sniffercapture {
        set cmd [string tolower $cmd]
        if {![regexp {^([dfu])(\d+)$} $cmd - dir step]} {
            error "check_sniffer_capture_steps ERROR: cmd=$cmd not in format: dnn or fnn or unn"
        }
        # puts "cmd=$cmd dir=$dir step=$step"

        # Initialization.
        if {$last_dir == ""} {
            set last_dir $dir
            if {$dir == "d" || $dir == "f"} {
                set last_step -1
            } else {
                set last_step 1000
            }
        }

        # Check direction is in sequence.
        if {(($dir == "d" || $dir == "u") && $last_dir == "f") ||
            ($dir == "d" && $last_dir == "u")} {
            UTF::Message WARN "" "check_sniffer_capture_steps cmd=$cmd dir=$dir\
		in $sniffercapture out of sequence, allowable for pingpong tests"
        }

        # Check step is in sequence.
        if {($dir == "d" && $last_dir == "d" && $step <= $last_step) ||\
            ($dir == "u" && $last_dir == "u" && $step >= $last_step) ||\
            ($dir == "f" && $last_dir == "f" && $step <= $last_step)} {
            UTF::Message WARN "" "check_sniffer_capture_steps cmd=$cmd step=$step\
		in $sniffercapture out of sequence, allowable for pingpong tests"
        }

        # Save the current cmd dir/step
        set last_dir $dir
        set last_step $step
    }
    # No errors found.
    return
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_sta_type] [arg sta_name] [arg desired_types]]

    # Used to check the [arg sta_name] object exists in the config file, that
    # is a valid STA, and the parent object type is one of the list of
    # [arg desired_types]. [arg desired_types] can be "*" to allow any type.[para]

    # Returns null if [arg sta_name] is defined and matches one of the
    # [arg desired_types], otherwise throws an error. 
}

proc UTF::check_sta_type { sta_name desired_types } {

    # Get type info for sta_name. This also determines if the object exists.
    set catch_resp [catch {$sta_name info type} catch_msg]
    if {$catch_resp != 0} {
        error "UTF::check_sta_type ERROR: object sta_name=$sta_name is not\
            defined in the config file !"
    }

    # Check that the object is a STA
    if {$catch_msg != "::UTF::STA"} {
        # To help the novice user, show actual STA names for this object
        set sta_list ""
        set catch_resp2 [catch {set sta_list [$sta_name cget -sta]} catch_msg2]
        set stas ""
        foreach {sta dev} $sta_list {
            lappend stas $sta
        }
        error "UTF::check_sta_type ERROR: $sta_name is type $catch_msg,\
            must be type ::UTF::STA, meaning one of the items in the object -sta list: $stas"
    }

    # Check the object parent/host is the one of the desired types.
    if {$desired_types == "*"} {
        return
    }
    foreach type $desired_types {
        if {[$sta_name hostis $type]} {
            return
        }
    }

    # Object parent did not match any of the list of desired types.
    error "UTF::check_sta_type ERROR: $sta_name host type [$sta_name hostis]\
        is not one of the desired_types=$desired_types !"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::check_wired_if] [arg STA]]
    # Throws an error if any of the [arg STA] -sta ethN ports is the 
    # wired ethernet port for the host. Ensures host will not go "deaf"
    # if a script does "ifconfig ethN down" on that port. If the host
    # does go "deaf", a power cycle or physical console access is needed
    # to recover the host.
} 

proc UTF::check_wired_if {STA} {

    # Get list of -sta info & -lan_ip for object.
    set sta_list [$STA cget -sta]
    set lan_ip [$STA cget -lan_ip]
    set lan_ip [string trim $lan_ip]
    if {$lan_ip == ""} {
        set lan_ip [UTF::get_name $STA]
    }

    # Find IP address for lan_ip.
    if {[regexp {^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$} $lan_ip]} {
        set host_ip_addr $lan_ip
    } else {
        set host_ip_addr [UTF::get_ip_addr $lan_ip]
    }
    UTF::Message INFO "$lan_ip" "UTF::check_wired_if sta_list=$sta_list lan_ip=$lan_ip host_ip_addr=$host_ip_addr"
    set host_ip_addr [string trim $host_ip_addr]
    if {$host_ip_addr == ""} {
        UTF::Message ERROR "$lan_ip" "UTF::check_wired_if Could not get IP\
            addr for $STA, checks on ethN not done."
        return FAIL
    }
    
    # Check each STA for IP addr.
    foreach {staN ethN} $sta_list {
        set sta_ip [$staN ipaddr]
        if {$sta_ip == $host_ip_addr} {
            error "UTF::check_wired_if STA: $staN $ethN ERROR:\
                sta_ip=$sta_ip EQ host_ip_addr=$host_ip_addr, PC will go \"deaf\"\
                if scripts do \"ifconfig $ethN down\" while loading the driver.\
                Please fix your config file and try again."
        }
    }
    return OK
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::choose_attn_grp] [arg va] [arg attngrp] [arg attngrp2]
    # [arg AP] [arg STA]]
    # Choose attn_grp to use from hierachy of possibilities (only apply to attngrp).
    # User may specify [arg attngrp] & [arg attngrp2] on the command line
    # which will take precedance. The [arg AP] & [arg STA] may have the option
    # -attngrp defined, which are next in the hierarchy. Many testrigs
    # have attenuator group ALL defined, which is the next choice.[para]

    # The final choice is to create a new attenuator group dynamically
    # using ports 1, 2, 3 & 4 on the attenuator [arg va]. This final choice
    # is provided to enable backwards compatibility for some scripts
    # that hardcoded ports 1 thru 4 for use.[para]

    # result is stored in variable ::rvr_attn_grp & ::rvr_attn_grp2
}

proc UTF::choose_attn_grp {va attngrp attngrp2 AP STA} {
    # Get possible attn_grps to use.
    set attngrp [string trim $attngrp] ;# from command line
    set attngrp2 [string trim $attngrp2] ;# from command line
    set ap_grp [$AP cget -attngrp]
    set ap_grp [string trim $ap_grp]
    set sta_grp [$STA cget -attngrp]
    set sta_grp [string trim $sta_grp]

    # Does attenuator group ALL exist? Many testrigs have it.
    set other_grp ""
    set catch_resp [catch {ALL info type} catch_msg]
    if {$catch_resp == "0"} {
        set other_grp ALL
    }

    # Command line option takes precedance. Then we check STA object
    # option, then AP object option, and finally ALL.
    set ::rvr_attn_grp ""
    set ::rvr_attn_grp2 ""
    if {$attngrp != ""} {
        set ::rvr_attn_grp $attngrp
    } elseif {$sta_grp != ""} {
        set ::rvr_attn_grp $sta_grp
    } elseif {$ap_grp != ""} {
        set ::rvr_attn_grp $ap_grp
    } elseif {$other_grp != ""} {
        set ::rvr_attn_grp $other_grp
    }
    if {$attngrp2 != ""} {
	set ::rvr_attn_grp2 $attngrp2
    }
    UTF::Message INFO "" "choose_attn_grp found command line:\
        attngrp=$attngrp attngrp2=$attngrp2 config file: sta_grp=$sta_grp ap_grp=$ap_grp other_grp=$other_grp"
    
    # Check the attngrp exists and is one of the supported group types.
    if {$::rvr_attn_grp != ""} {
        UTF::check_host_os $::rvr_attn_grp ;# checks existence
        set resp [eval UTF::check_host_type $::rvr_attn_grp $::attn_grp_type_list]
        if {$resp == "1"} {
	    if {$::rvr_attn_grp2 != ""} {
            	UTF::Message INFO "" "choose_attn_grp using ::rvr_attn_grp=$::rvr_attn_grp ::rvr_attn_grp2=$::rvr_attn_grp2"
	    } else {
            	UTF::Message INFO "" "choose_attn_grp using ::rvr_attn_grp=$::rvr_attn_grp"
	    }
            return
        } else {
            error "choose_attn_grp ERROR: $::rvr_attn_grp is not one of types: $::attn_grp_type_list"
        }
    }

    # We are now going to create our own attenuator group.
    # Make sure attenuator va is not null.
    set va [string trim $va]
    if {$va == ""} {
        error "choose_attn_grp ERROR: Must specify variable attenuator, va"
    }

    # DEPRACATED! Please use attngrp! Supported for old Aeroflex object,
    # but NOT the newer ones!
    # Make sure va exists and is type Aeroflex.
    UTF::check_host_os $va ;# checks existence
    set type Aeroflex
    set resp [UTF::check_host_type $va $type]
    if {$resp != "1"} {
        error "choose_attn_grp ERROR: $va is not type $type!"
    }
    
    # Create attenuator group.
    set grp RvrTemp
    set ports "1 2 3 4"
    $va createGroup $grp $ports
    set ::rvr_attn_grp $grp
    UTF::Message INFO "" "choose_attn_grp created ::rvr_attn_grp=$::rvr_attn_grp ports=$ports"
    return
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::cleanup_temp_files] [arg server_path] [arg sta_list]
    # [arg sta_path]]

    # Removes all files from [arg server_path]. From each STA in [arg sta_list],
    # removes all files in [arg sta_path]. Returns integer count of files
    # & directories deleted.
}

proc UTF::cleanup_temp_files {server_path sta_list sta_path} {

    # Set result counters
    set delete_files 0
    set delete_directories 0

    # Take out the trash on the server_path.
    set server_path [string trim $server_path]
    if {$server_path != ""} {

        # Remove all files in the server_path
        set tmp_files [exec find $server_path -type f]
        foreach file $tmp_files {
            UTF::Message INFO "" "deleting $file"
            set catch_resp [catch {file delete -force $file} catch_msg]
            if {$catch_resp == 0} {
                incr delete_files
            } else {
                UTF::Message ERROR "" "Could not delete $file $catch_msg"
            }
        }

        # Clean out sub-directories, if any. Dont use -force option
        # just to be careful.
        foreach dir "xp vista win7" {
            UTF::Message INFO "" "deleting $server_path/$dir"
            set catch_resp [catch {file delete $server_path/$dir} catch_msg]
            if {$catch_resp == 0} {
                incr delete_directories
            } else {
                UTF::Message ERROR "" "Could not delete $server_path/$dir $catch_msg"
            }
        }
    }

    # Take out the trash on each testrig PC
    set sta_path [string trim $sta_path]
    if {$sta_path != ""} {
        foreach STA $sta_list {

            # Remove all files in the sta_path
            set sta_files [$STA find $sta_path -type f]
            foreach file $sta_files {
                # UTF::Message INFO "$STA" "deleting $file"
                set catch_resp [catch {$STA rm -f $file} catch_msg]
                if {$catch_resp == 0} {
                    incr delete_files
                } else {
                    UTF::Message ERROR "" "Could not delete $STA $file $catch_msg"
                }
            }

            # Clean out sub-directories, if any. Cygwin doesnt have the rmdir
            # command, so we must use rm -rf, much as I would rather not.
            foreach dir "xp vista win7" {
                set catch_resp [catch {$STA rm -rf $sta_path/$dir} catch_msg]
                if {$catch_resp == 0} {
                    incr delete_directories
                } else {
                    UTF::Message ERROR "" "Could not delete $STA $sta_path/$dir $catch_msg"
                }
            }
        }
    }

    # Return stats.
    return "deleted $delete_files files, $delete_directories directories"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::collect_rpopen_data] [arg timeout_sec] [arg timed_host_fd_list]
    # [arg timed_output] [arg untimed_host_fd_list] [arg untimed_output]]

    # [arg timeout_sec] timeout, in seconds, after which error recovery
    # action is initiated.[para]

    # [arg timed_host_fd_list] space seperated list of the hosts and associated file
    # descriptors for the processes running on the remote hosts. These processes
    # are expected to finish within the [arg timeout_sec] period. Eg: cx01tst1 file3
    # cx01tst3 file5 [para]

    # [arg timed_output] name of the variable in which output that came from 
    # the above timed_list of hosts & file descriptors will be stored. The data,
    # if any, is formatted in pairs of host name and string. Each separate
    # output line from a host is a separate element in the list.[para]

    # [arg untimed_host_fd_list] space seperated list of the hosts and associated file
    # descriptors for the processes running on the remote hosts. These processes
    # are expected run much longer and are not subject to the [arg timeout_sec]
    # period. Eg: cx01tst2 file8 cx01tst4 file9 [para]

    # [arg untimed_output] name of the variable in which output that came from 
    # the above untimed_list of hosts & file descriptors will be stored. The data,
    # if any, is formatted in pairs of host name and string. Each separate
    # output line from a host is a separate element in the list.[para]

    # This routine take pairs of host names and file descriptors and collects
    # up any output data from the file descriptors. Typically the file
    # descriptor represents a remote command pipeline open process that
    # is running an independant parallel task on a remote PC. Each parallel
    # task may generate output asynchronously while it runs.[para]

    # The calling routine should setup a fileevent. As a minimum, it should do
    # "set ::utils_reading READY". The fileevent can do other things if needed.
    # If the fileevent is not setup, a 1 second timer will ensure your data
    # is correctly received. The fileevent will ensure the data is received
    # a bit more quickly.[para]

    # When a file descriptor gets an EOF, that file descriptor is closed.
    # When all the timed file descriptors are closed, this routine returns.
    # If a timed file descriptor fails to complete within the specified
    # [arg timeout_sec] period, that file descriptor is terminated. The
    # untimed file descriptors are left to carry on as they see fit.
}

#====================================================================
proc UTF::collect_rpopen_data {timeout_sec timed_host_fd_list timed_output\
    untimed_host_fd_list untimed_output} {
    upvar $timed_output timed_response
    upvar $untimed_output untimed_response

    # Check timeout is numeric, GE 1.
    if {![regexp {^\d+$} $timeout_sec] || $timeout_sec < 1} {
        error "UTF::collect_rpopen_data ERROR: invalid timeout_sec=$timeout_sec,\
            must be integer, GE 1"
    }

    # We need at least one pair of host & fd in one of the lists.
    if {[llength $timed_host_fd_list] < 2 && [llength $untimed_host_fd_list] < 2} {
        error "UTF::collect_rpopen_data ERROR: Need at least one host & fd\
            specified in either of: timed_host_fd_list=$timed_host_fd_list\
            untimed_host_fd_list=$untimed_host_fd_list"
    }

    # Check that output variables are not null & not the same.
    set timed_output [string trim $timed_output]
    set untimed_output [string trim $untimed_output]
    if {$timed_output == "" || $untimed_output == ""} {
        error "UTF::collect_rpopen_data ERROR: output variables must not be\
            null, timed_output=$timed_output untimed_output=$untimed_output"
    }
    if {$timed_output == $untimed_output} {
        error "UTF::collect_rpopen_data ERROR: output variables must not be\
            the same, timed_output=$timed_output untimed_output=$untimed_output"
    }

    # Load each list host names & file descriptor parameters into an array.
    UTF::Message INFO "" "UTF::collect_rpopen_data\
        timeout_sec=$timeout_sec timed_host_fd_list=$timed_host_fd_list\
            timed_output=$timed_output untimed_host_fd_list=$untimed_host_fd_list\
            untimed_output=$untimed_output"
    set cnt_timed 0
    set cnt_untimed 0
    set i 0
    foreach {host_fd_list timing} [list $timed_host_fd_list timed\
        $untimed_host_fd_list untimed] {
        # puts "host_fd_list=$host_fd_list timing=$timing"
        foreach {host fd} $host_fd_list {
            set host [string trim $host]
            set fd [string trim $fd]
            if {$host == "" || $fd == ""} {
                continue
            }

            # Get proper log host name for case of STA.
            set logname [UTF::get_name $host]

            # Store data in next array column.
            incr i
            incr cnt_$timing
            set array(host,$i) $host
            set array(logname,$i) $logname
            set array(fd,$i) $fd
            set array(timing,$i) $timing ;# each fd is shown as timed or untimed

            # configure fd options
            set catch_resp [catch {fconfigure $fd -buffering line -buffersize 1024} catch_msg]
            if {$catch_resp != 0} {
                UTF::Message WARN "$logname" "UTF::collect_rpopen_data fconfigure $fd\
                catch_resp=$catch_resp catch_msg=$catch_msg"
            }
        }
    }
    # puts "i=$i cnt_timed=$cnt_timed cnt_untimed=$cnt_untimed"

    # Wait for any of the file descriptors to produce output.
    set start_sec [clock seconds]
    while { 1 } {

        # In order to implement a timer, we need to periodically go 
        # through this while loop so we can check on overall elapsed
        # time and take recovery action as needed. So we use a 1 sec timer
        # to ensure this occurs.

        # Vwait needs to see some code in the current routine / file that
        # conditionaly writes to the variable that vwait will watch. If the
        # after statement is removed, you get TCL error "can't wait for
        # variable ::utils_reading, would wait forever".
        after 1000 {set ::utils_reading 1}

        # All the file descriptor event handlers have been told to 
        # write to the same global variable ::utils_reading. We use
        # vwait to wait for output from any of the file descriptors.
        vwait ::utils_reading

        # vwait received an event. We may have some output to process now.
        # Check each not-null fd in the array.
        set timed_done yes
	    for {set j 1} {$j <= $i} {incr j} {

            # Ignore any blank fd, we are done with them.
            set fd $array(fd,$j)
            set host $array(host,$j)
            set logname $array(logname,$j)
            set timing $array(timing,$j)
            if {$fd == ""} {
                continue
            } elseif {$timing == "timed"} {
                set timed_done no
            }

            # Try to get data from fd.
            set msg ""
            set catch_resp [catch {set msg [gets $fd]} catch_msg]
            if {$catch_resp != 0} {
                # This fd may have been valid earlier, but it is now expired.
                UTF::Message WARN $logname "UTF::collect_rpopen_data fd=$fd $catch_msg"
                set array(fd,$j) "" ;# we are done with this fd
                continue
            }

            # Process data from fd.
            if {[eof $fd]} {
                UTF::Message INFO $logname "UTF::collect_rpopen_data got normal EOF, closing fd=$fd host=$host"
                # Put file descriptor back to blocking so that close can get a valid return status.
                fconfigure $fd -blocking 1
                if {[catch {close $fd} ret]} {
                    UTF::Message ERROR $logname "UTF::collect_rpopen_data ERROR closing fd=$fd $ret"
                }
                set array(fd,$j) "" ;# we are done with this fd

            } elseif {![fblocked $fd]} {
                # Keep timed output separate from untimed_output.
                # Add timestamps to the _response data.
                set hhmmss [clock format [clock seconds] -format "%T"]
                if {$timing == "timed"} {
                    lappend timed_response "$hhmmss $host $msg" ;# dont use logname here!
                } else {
                    lappend untimed_response "$hhmmss $host $msg" ;# dont use logname here!
                }
                UTF::Message INFO $logname $msg
            }
        }

        # Are we done or timed out?
        # NB: If we had only untimed fd, then we exit loop only on timeout.
        set elapsed_sec [expr [clock seconds] - $start_sec]
        if {($timed_done == "yes" && $cnt_timed > 0) || $elapsed_sec > $timeout_sec} {
            break
        }
    }

    # If we timed out, terminate the pids associated for each timed file
    # descriptor that has not already been closed. Leave the untimed file
    # descriptors alone!
    if {$elapsed_sec > $timeout_sec} {
       	for {set j 1} {$j <= $i} {incr j} {
            set fd $array(fd,$j)
            set host $array(host,$j)
            set timing $array(timing,$j)
            if {$fd != "" && $timing == "timed"} {
                set pid [pid $fd]
                UTF::Message WARN "" "UTF::collect_rpopen_data timeout, terminating fd=$fd pid=$pid for host=$host"
                set catch_resp [catch {exec kill $pid} catch_msg]
                if {$catch_resp != 0} {
                    UTF::Message ERROR "" "UTF::collect_rpopen_data did not terminate pid=$pid: $catch_msg"
                }
            }   
        }
    }

    # To simplify debugging, show list of untimed fd that are still open.
    if {$cnt_untimed > 0} {
        set untimed_list ""
        for {set j 1} {$j <= $i} {incr j} {
            set fd $array(fd,$j)
            set host $array(host,$j)
            set timing $array(timing,$j)
            if {$fd != "" && $timing == "untimed"} {
                lappend untimed_list "$host $fd"
            }
        }
        UTF::Message INFO "" "UTF::collect_rpopen_data still active untimed host & fd: $untimed_list"
    }

    # Thats it. Output from fd is stored in the variable names passed to us.
    return ""
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::convert_chanspec] [arg channel] [arg bandwidth] [opt chanspec]]

    # Converts [arg channel] & [arg bandwidth] to standard format. If available,
    # [opt chanspec] in format 0xhhhh is the preferred input.
    # EG: channel=36 and bandwidth=40 maps to 36l, also chanspec=0x1d26 maps to 36l.
}

proc UTF::convert_chanspec {channel bandwidth {chanspec -1}} {

    # Chanspec is preferred source of info.
    if {$chanspec != "-1" && $chanspec != ""} {
        set chanspec [string tolower $chanspec]
        set chanspec [string trim $chanspec]
        if {![regexp {^0x[\da-f]+$} $chanspec]} {
            error "UTF::convert_chanspec ERROR: $chanspec not format 0xhhhh!"
        }

        # Define chanspecs - new 11AC format
        # Listed in ascending hex order, so we can check for reuse/duplication.
        set chan(0x1001) 1
        set chan(0x1002) 2
        set chan(0x1003) 3
        set chan(0x1004) 4
        set chan(0x1005) 5
        set chan(0x1006) 6
        set chan(0x1007) 7
        set chan(0x1008) 8
        set chan(0x1009) 9 
        set chan(0x100a) 10
        set chan(0x100b) 11

        set chan(0x1803) 1l
        set chan(0x1804) 2l
        set chan(0x1805) 3l
        set chan(0x1806) 4l
        set chan(0x1807) 5l
        set chan(0x1808) 6l
        set chan(0x1809) 7l

        set chan(0x1903) 5u
        set chan(0x1904) 6u
        set chan(0x1905) 7u
        set chan(0x1906) 8u
        set chan(0x1907) 9u
        set chan(0x1908) 10u
        set chan(0x1909) 11u

        # Define chanspecs - older format
        set chan(0x1b24) 36
        set chan(0x1b28) 40
        set chan(0x1b2c) 44
        set chan(0x1b30) 48
        set chan(0x1b34) 52
        set chan(0x1b38) 56
        set chan(0x1b3c) 60
        set chan(0x1b40) 64
        set chan(0x1b95) 149
        set chan(0x1b99) 153
        set chan(0x1b9d) 157
        set chan(0x1ba1) 161
        set chan(0x1ba5) 165

        set chan(0x1d26) 36l
        set chan(0x1d2e) 44l
        set chan(0x1d36) 52l
        set chan(0x1d3e) 60l
        set chan(0x1d97) 149l
        set chan(0x1d9f) 157l
 
        set chan(0x1e26) 40u
        set chan(0x1e2e) 48u
        set chan(0x1e36) 56u
        set chan(0x1e3e) 64u
        set chan(0x1e97) 153u
        set chan(0x1e9f) 161u

        set chan(0x2b01) 1
        set chan(0x2b02) 2
        set chan(0x2b03) 3
        set chan(0x2b04) 4
        set chan(0x2b05) 5
        set chan(0x2b06) 6
        set chan(0x2b07) 7
        set chan(0x2b08) 8
        set chan(0x2b09) 9
        set chan(0x2b0a) 10
        set chan(0x2b0b) 11

        set chan(0x2d03) 1l
        set chan(0x2d04) 2l
        set chan(0x2d05) 3l
        set chan(0x2d06) 4l
        set chan(0x2d07) 5l
        set chan(0x2d08) 6l
        set chan(0x2d09) 7l

        set chan(0x2e03) 5u
        set chan(0x2e04) 6u
        set chan(0x2e05) 7u
        set chan(0x2e06) 8u
        set chan(0x2e07) 9u
        set chan(0x2e08) 10u
        set chan(0x2e09) 11u

        # Define chanspecs - new 11AC format
        set chan(0xd024) 36
        set chan(0xd028) 40
        set chan(0xd02c) 44
        set chan(0xd030) 48
        set chan(0xd034) 52
        set chan(0xd038) 56
        set chan(0xd03c) 60 
        set chan(0xd040) 64
        set chan(0xd095) 149
        set chan(0xd099) 153
        set chan(0xd09d) 157
        set chan(0xd0a1) 161
        set chan(0xd0a5) 165

        set chan(0xd826) 36l
        set chan(0xd82e) 44l
        set chan(0xd836) 52l
        set chan(0xd83e) 60l
        set chan(0xd897) 149l
        set chan(0xd89f) 157l

        set chan(0xd926) 40u
        set chan(0xd92e) 48u
        set chan(0xd936) 56u
        set chan(0xd93e) 64u
        set chan(0xd997) 153u
        set chan(0xd99f) 161u

        # 11AC only chanspecs
        set chan(0xe02a) 36/80
        set chan(0xe03a) 52/80
        set chan(0xe09b) 149/80
        set chan(0xe12a) 40/80
        set chan(0xe13a) 56/80
        set chan(0xe19b) 153/80
        set chan(0xe22a) 44/80
        set chan(0xe23a) 60/80
        set chan(0xe29b) 157/80 
        set chan(0xe32a) 48/80
        set chan(0xe33a) 64/80
        set chan(0xe39b) 161/80

        # Look up channel mnemonic
        if {[info exists chan($chanspec)]} {
            set result $chan($chanspec)
            UTF::Message INFO "" "convert_chanspec chanspec=$chanspec result=$result" 
            return $result
        } else {
            error "UTF::convert_chanspec ERROR: no entry for: $chanspec"
        }
    }

    # Define known channels
    set chan(1,20) 1
    set chan(1,40) 1l
    set chan(2,20) 2
    set chan(2,40) 2l
    set chan(3,20) 3
    set chan(3,40) 3l
    set chan(4,20) 4
    set chan(4,40) 4l
    set chan(5,20) 5
    set chan(5,40) 5l
    set chan(6,20) 6
    set chan(6,40) 6l
    set chan(7,20) 7
    set chan(7,40) 7l
    set chan(8,20) 8
    set chan(8,40) 8u
    set chan(9,20) 9
    set chan(9,40) 9u
    set chan(10,20) 10
    set chan(10,40) 10u
    set chan(11,20) 11
    set chan(11,40) 11u

    # What to do about 5u, 6u, 7u? use 0xhhhh chanspec...

    set chan(36,20) 36
    set chan(36,40) 36l
    set chan(40,20) 40
    set chan(40,40) 40u
    set chan(44,20) 44
    set chan(44,40) 44l
    set chan(48,20) 48
    set chan(48,40) 48u
    set chan(52,20) 52
    set chan(52,20) 52l
    set chan(56,20) 56
    set chan(56,40) 56u
    set chan(60,20) 60
    set chan(60,40) 60l
    set chan(64,20) 64
    set chan(64,40) 64u

    set chan(149,20) 149
    set chan(149,40) 149l
    set chan(153,20) 153
    set chan(153,40) 153u
    set chan(157,20) 157
    set chan(157,40) 157l
    set chan(161,20) 161
    set chan(161,40) 161u
    set chan(165,20) 165

    # Look up channel mnemonic
    if {[info exists chan($channel,$bandwidth)]} {
        set result $chan($channel,$bandwidth)
        UTF::Message INFO "" "convert_chanspec channel=$channel bandwidth=$bandwidth result=$result" 
        return $result
    } else {
        error "UTF::convert_chanspec ERROR: no entry for: $channel,$bandwidth"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::delay_stats] [arg STA] [opt is_oem_ap=0] [opt low_lim=0.001]]

    # Queries the [arg STA] delay stats with [opt low_lim] is 0.001.
    # Leave [arg STA] alone if it is an OEM AP.

    # Returns two CSV strings with counts for: Delay_Hist, Delay_CCDF,
}

proc UTF::delay_stats {STA {is_oem_ap 0} {low_lim 0.001}} {

    # If STA is known to be in trouble, or is_oem_ap!=0, return.
    UTF::Message INFO "$STA" "delay_stats STA=$STA is_oem_ap=$is_oem_ap low_lim=$low_lim"
    set msg " , ," ;# Need 1 comma per field normally returned.
    set var "::${STA}_state"
    if {[set $var] != "OK" || $is_oem_ap != "0"} {
        return $msg
    }

    # get wl dump dlystats
    set catch_resp [catch {$STA wl dump dlystats} catch_msg]
    UTF::save_device_state $STA $catch_msg
    if {$catch_resp != 0 || $catch_msg == "N/A"} {
        UTF::Message ERROR "$STA" "catch_msg=$catch_msg"
        return $msg
    }

    # Look for 1 line or more of Delay Histogram stats. Watch out for numbers in header.
    # set catch_msg "Dly Hist * 1ms bins * \n\: 11(0%) 13942(83%) 1862(11%) 765(4%) 35(0%) 0(0%) 0(0%) 0(0%)"
    if {![regexp {Dly\s*Hist[^\n]+\n((\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,})} $catch_msg - delay_hist]} {
        set delay_hist ""
    }
    # puts "delay_hist=$delay_hist"

    # Cleanup distribution strings, leaving just the raw frame counts.
    set CCDF_ratio ""
    set result ""
    set list $delay_hist
    regsub -all {\n} $list "" list ;# remove new-lines
    regsub -all {:} $list "" list ;# remove colons
    regsub -all {\(\d+%\)} $list "" list ;# remove (nn%)
    regsub -all {[a-zA-Z]} $list "" list;# remove text
    regsub -all {\s+} $list " " list ;# compress consecutive whitespace into single space
     
    set delay_hist [string trim $list]
    if {$delay_hist == "" || $delay_hist == "0"} {
        set delay_hist "0 0 0 0 0 0 0 0" ;# ensure some data to avoid gnuplot errors
    } 
    set CCDF_ratio [UTF::CCDF $delay_hist $low_lim]
     
    append result " ${delay_hist}," ;# result is CSV formatted
    append result " ${CCDF_ratio}," ;# result is CSV formatted

    # Log & return result.
    UTF::Message INFO "$STA" "delay_stats distributions: delay_hist,CCDF_ratio=$result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::do_post_test_analysis] [arg test_script]
    # [arg options_array]]

    # Runs the testrig specific script specified in optional variable
    # ::UTF::PostTestAnalysis. When this variable is present, it is
    # expected to specify the full pathname to an executable program,
    # such as a TCL script or a bash shell script. The stdout is parsed 
    # looking for results such as a web link.[para]

    # The users script will be passed the following parameters:
    # 1) the directory containing the log files for the tests just run
    # 2) the name of the [arg test_script] that just ran
    # 3) list of options name=value pairs that were used by [arg test_script] [para]

    # Results are passed to the UTF web summary page.
}

proc UTF::do_post_test_analysis {test_script options_array} {
    upvar $options_array opt_array ;# gives R/W access to options_array

    # Did the user specify a script to run?
    if {![info exists ::UTF::PostTestAnalysis]} {
       UTF::Message INFO "" "do_post_test_analysis no variable ::UTF::PostTestAnalysis found"
       return
    }

    # Run users script and parse output.
    set user_script $::UTF::PostTestAnalysis
    set fn [file tail $user_script] ;# short script name
    UTF::Try "PostTestAnalysis $fn" {

        # Check script exists & is executable.
        if {![file exists "$user_script"]} {
            error "$user_script not found!"
        }
        if {![file executable "$user_script"]} {
            error "$user_script not executable!"
        }

        # Get log directory
        set log_dir [file dirname "$UTF::Logfile"]

        # Get testrig name
        UTF::setup_config_testrig

        # Get stream being tested
        set stream [UTF::get_stream_name end]
        set stream [string trim $stream]
        if {$stream == "---" || $stream == ""} {
            # We may be able to extract usefull stream info from other options
            foreach item "branch statag rtrtag tag" {
                if {[info exists opt_array($item)]} {
                    set temp $opt_array($item)
                    set temp [string trim $temp]
                    # puts "item=$item temp=$temp"
                    if {$temp != "auto" && $temp != ""} {
                        # Save first part of string up to first "_", if any.
                        set temp [split $temp "_"]
                        set stream [lindex $temp 0]
                        break
                    }
                }
            }
        }
        if {$stream == "NIGHTLY"} {
            set stream TOT
        }
        # puts "stream=$stream"

        # Did user specify aux_sge_queue for background job log processing?
        if {[info exists ::aux_sge_queue]} {
            set aux_sge $::aux_sge_queue
        } else {
            set aux_sge "-" ;# pass non-null value to keep parsing simple.
        }

        # Did user specify aux_lsf_queue for background job log processing?
        if {[info exists ::aux_lsf_queue]} {
            set aux_lsf $::aux_lsf_queue
        } else {
            set aux_lsf "-" ;# pass non-null value to keep parsing simple.
        }

        # Get sorted list of options.
        set names [array names opt_array]
        set names [lsort $names]
        set opt_list ""
        foreach item $names {
            lappend opt_list "$item=$opt_array($item)"
        }
        set opt_list [string trim $opt_list]
        UTF::Message INFO "" "do_post_test_analysis aux_sge=$aux_sge aux_lsf=$aux_lsf test_script=$test_script opt_list: $opt_list"

        # Add an entry to the testrig history. This facilitates backtracking to 
        # recent equivalent tests on the same testrig.
        set history_fn "[file dirname $log_dir]/$::testrig.test.history"
        set catch_resp [catch {set out [open $history_fn a]} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR "" "do_post_test_analysis: could not open $history_fn catch_msg=$catch_msg"
        } else {
            UTF::Message INFO "" "do_post_test_analysis updating $history_fn"
            puts $out "$::testrig $stream $test_script $log_dir $opt_list"
        }
        catch {close $out}

        # Run script, capture stdout.
        set result ""
        set status PASS
        UTF::Message INFO "" "do_post_test_analysis pwd: [pwd]"
        UTF::Message INFO "" "do_post_test_analysis running: $user_script"
        set catch_msg ""
        set catch_resp [catch {exec $user_script $log_dir $aux_sge $aux_lsf $test_script $opt_list} catch_msg]
        UTF::Message INFO "" "do_post_test_analysis catch_resp=$catch_resp catch_msg=$catch_msg"
        if {$catch_resp != 0} {
            set status FAIL
        }

        # Parse out one or more hyperlinks.
        UTF::Message INFO "" "do_post_test_analysis looking for hyperlinks"
        set catch_msg [split $catch_msg \n]
        foreach line $catch_msg {
            if {[regexp -nocase {(http:\S+)\s*$} $line - url]} {
                UTF::Message INFO "" "do_post_test_analysis url=$url"
                if {$result == ""} {
                    set result "html:"
                }
                set url [string trim $url]
                set short_url [file rootname [file tail $url]]
                append result " <a href=\"$url\">$short_url</a><br>"
            }
        }

        # If no url found, show last bit of output.
        set result [string trim $result]
        if {$result == ""} {
            # Grab last 100 chars of output for result.
            set len [string length "$catch_msg"]
            set start [expr $len - 100]
            set result [string range $catch_msg $start end]
            regsub -all {\n} $result " " result
        }

        # Push result up to main web summary page
        if {$status == "PASS"} {
            return "$result"
        } else {
            error "$result"
        }
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::enh_load] [arg STA] [lb][arg args][rb]]

    # Wrapper around the load method to add build type info
    # to the data returned.[para]
}

proc UTF::enh_load {STA args} {

    # Call the method load, & save output.
    set resp [eval $STA load $args]

    # Try to figure out what build image was really loaded.
    set load_type [$STA cget -image]
    set load_type [split $load_type "/"]
    set load_type [lrange $load_type 6 end]
    set load_type [join $load_type "/"]
    set load_type [string trim $load_type]
 
    if {$load_type == "" || $load_type == "/"} {
        # The -image didnt have any current info, so get the default
        # values the object would normally use. Some objects have -type,
        # -file or -sys, but no gurantees.
        set load_type ""
        foreach option "-date -brand -type -version -file -sys" {
            set temp ""
            set catch_resp [catch {set temp [$STA cget $option]} catch_msg]
            if {$catch_resp == 0} {
                set load_type "$load_type $temp"
                if {$option == "-file"} {
                    # When we find -file info, we dont need to look at -sys.
                    break
                }
            }
        }
        set load_type [join $load_type "/"]
    }

    # Return the load response & load_type
    return "$resp $load_type"
}

#===================================================================
UTF::doc {
    # [call [cmd UTF::find_consecutive_values] [arg num_list] [arg count]
    # [opt direction=forward] [opt index=0] [opt increment=1]]

    # Used by RvR routines to locate where an event first occurred
    # and stayed the same value for a while. Example is keeping track of when
    # we lost beacons or when we got beacons back.  [arg num_list]
    # is usually a list of pairs, usually integers, attenuation & value.
    # [arg count] is the minimum number of consecutive points of equal value you
    # are looking for in a row. Larger [arg count] gives higher degree of 
    # confidence in the result. The algorithm is greedy where possible, trying
    # to get the largest number of pairs possible from which to choose the
    # final result. [opt direction] is forward, the default or reverse
    # to choose the direction to scan the [arg num_list]. [opt index] is used to select
    # which result to return, 0=first, end=last. [opt increment] specifies
    # the expected spacing of consecutive  values, default is 1. There are 
    # times when attenuator step is set to 2, so expected spacing will be 2.[para]

    # Returns: Scans in [arg direction] thru the pairs in [arg num_list] looking for
    # [arg count] or more consecutive values with [arg increment] spacing, returns
    # the [arg index] attenuation point.[para]

    # EG: num_list is: {33 1} {38 2} {39 2} {40 2} {41 2}, count is 3,
    # direction is forward, index is 0, routine returns 38    
}

proc UTF::find_consecutive_values {num_list count {direction forward} {index 0} {increment 1}} {
    UTF::Message INFO "" "find_consecutive_values num_list=$num_list\
        count=$count direction=$direction index=$index increment=$increment"

    # Sanity checks
    set num_list [string trim $num_list]
    set count [string trim $count]
    if {$count == "" || $count < 1 || ![regexp {^\d+$} $count]} {
       set count 3
       UTF::Message WARN "" "find_consecutive_values invalid count, set to: $count"
    }
    set direction [string trim $direction]
    set direction [string tolower $direction]
    if {$direction != "forward" && $direction != "reverse"} {
       set direction forward
       UTF::Message WARN "" "find_consecutive_values invalid direction, set to: $direction"
    }
    set index [string trim $index]
    set index [string tolower $index]
    if {$index == "" || $index < 0 || (![regexp {^\d+$} $index] && $index != "end")} {
       set index 0
       UTF::Message WARN "" "find_consecutive_values invalid index, set to: $index"
    }
    set increment [string trim $increment]
    # Comment out when adding -attnstep=0.5 support. 02/04/2014
    #if {$increment == "" || ![regexp {^[\-\d]+$} $increment]} {
    #    # NB: increment=0 is needed for fastrampup tests
    #    set increment 1 ;# negative is OK.
    #    UTF::Message WARN "" "find_consecutive_values invalid increment, set to: $increment"
    #}

    # For direction=reverse, flip the pairs in num_list
    if {$direction == "reverse"} {
        set scan_list ""
        foreach pair $num_list {
            set scan_list "\{$pair\} $scan_list" ;# prepend list
        }
    } else {
       set scan_list $num_list 
    }
    # puts "direction=$direction scan_list=$scan_list"

    # Look for count consecutive pairs with equal value in scan_list
    # For say, 3 consecutive items, cons_cnt will be 2.
    set cons_cnt 0
    set cons_lim [expr $count - 1]
    set previous_attn ""
    set previous_val ""
    set result_list ""
    foreach pair $scan_list {

        # Get data from current pair
        set attn [lindex $pair 0]
        set val [lindex $pair 1] ;# OK to be null
        # puts "pair=$pair attn=$attn val=$val"

        # Save first pair data.
        if {$previous_attn == "" } {
            set previous_attn $attn
            set previous_val $val
            set result_list ""
            lappend result_list $pair
            continue
        }

        # Is the value the same as previous value?
        # NB: Can be alpha string here.
        if {"$val" != "$previous_val"} {
            # Do we have enough consecutive results?
            # Doing the check here allows for greedy collection of pairs.
            if {$cons_cnt >= $cons_lim} {
                break
            } 
            set cons_cnt 0
            set previous_attn $attn
            set previous_val $val
            set result_list ""
            lappend result_list $pair
            # puts "RESET VALUE MISMATCH"
            continue
        }

        # Is item the immediate consecutive value after previous item?
        # NB: consecutive is based on the increment value, may or may not be 1.
        set delta [expr $attn - $previous_attn]
        # puts "previous_attn=$previous_attn previous_val=$previous_val attn=$attn\
        #     val=$val delta=$delta"
        if {$delta == $increment} {
            incr cons_cnt
            # puts "CONSECUTIVE: cons_cnt=$cons_cnt"
            set previous_attn $attn
            # No need to save val
            lappend result_list $pair
            continue

        } else {
            # We found a non-consecutive value. Do we have enough consecutive results?
            # Doing the check here allows for greedy collection of pairs.
            if {$cons_cnt >= $cons_lim} {
                break
            }
            set cons_cnt 0
            set previous_attn $attn
            set previous_val $val
            set result_list ""
            lappend result_list $pair
            # puts "RESET NOT CONSECUTIVE"
            continue
        }
    }

    # Do we have enough consecutive results?
    # Also handles case of count=1 and only 1 pair in num_list
    UTF::Message INFO "" "find_consecutive_values cons_cnt=$cons_cnt\
        cons_lim=$cons_lim result_list=$result_list"
    if {$cons_cnt >= $cons_lim} {
        # Use index to choose from result_list
        set pair [lindex $result_list $index]
        return  [lindex $pair 0] ;# return attn from pair
    } 

    # Desired consecutive count NOT found. Happens frequently.
    return ""
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::find_oem_ap_chanspec] [arg oem_ssid] [arg oem_band] [arg STA]]

    # Uses [arg STA] to scan the airwaves to find out what channel
    # OEM AP with [arg oem_ssid] is currently using. [arg oem_ssid] is
    # used as a pattern for matching the desired AP. Some OEM AP
    # broadcast on both bands simultaneously. [arg oem_band] is used 
    # to choose between bands in this case. 
    # Returns chanspec & actual oem_ssid or throws an error.
}

proc UTF::find_oem_ap_chanspec {oem_ssid oem_band STA} {

    # Checks on eom_band.
    UTF::Message INFO "" "find_oem_ap_chanspec oem_ssid=$oem_ssid oem_band=$oem_band STA=$STA"
    if {$oem_band != "2.4" && $oem_band != "5" && $oem_band != ""} {
        UTF::Message WARN "" "find_oem_ap_chanspec: invalid oem_band=$oem_band, should be 2.4 or 5, set to null"
        set oem_band ""
    }

    # The OEM AP & STA are supposed to be loaded. If not, we are toast.
    set name [UTF::get_name $STA]
    catch {$STA wl up}
    catch {$STA wl mpc 0}
    catch {$STA wl disassoc}
    catch {$STA wl band auto}
    catch {$STA wl scan}
    UTF::Sleep 5 $name "Wait for scan..."
    set catch_resp [catch {set scanresults [$STA wl scanresults]} catch_msg]
    if {$catch_resp != 0} {
        error "find_oem_ap_chanspec ERROR: $name wl scan failed: $catch_msg"
    }

    # Find channel being used by oem_ssid.
    set scanresults [split $scanresults \n]
    set bw ""
    set chan ""
    set chanspec ""
    set oem_ssid [string trim $oem_ssid]
    set ssid ""
    foreach line $scanresults {
        # First look for the desired oem_ssid
        # NB: Mac K10B shows 2 different SSID, so we pattern match for the ssid.
        # mc56tst1   SSID: "mc56ap2"
        # mc56tst1   SSID: "mc56ap2 (5 GHz)"
        if {$ssid == ""} {
            if {[regexp -nocase {^\s*SSID:\s+(.+)$} $line - x]} {
                regsub -all {\"} $x "" x
                if {[regexp -nocase $oem_ssid $x]} {
                    # We found the correct AP, save it.
                    set ssid $x
                }
            }
            continue
        }

        # Then look for channel & bandwidth.
        if {$ssid != ""} {
            if {[regexp -nocase {channel\s+(\d+)\s+(\d+)MHz(\s+\S+)?} $line - x y z]} {
                set chan $x
                set bw $y
                set chanspec $z
                regsub -all {\"} $chan "" chan
                regsub -all {\"} $bw "" bw
                regsub -all {\s+\(|\)} $chanspec "" chanspec

                # Is there a preferred band?
                if {($oem_band == "2.4" && $chan >= 36) || ($oem_band == "5" && $chan < 36)} {
                    UTF::Message INFO "$name" "find_oem_ap_chanspec channel $chan not on desired band $oem_band GHz, keep searching..." 
                    set bw ""
                    set chan ""
                    set chanspec ""
                    set ssid ""
                    continue
                }

                # We found the desired ssid & band.
                set result [UTF::convert_chanspec $chan $bw $chanspec]
                UTF::Message INFO $name "find_oem_ap_chanspec chan=$chan bw=$bw\
		    chanspec=$chanspec oem_band=$oem_band GHz ssid=$ssid result=$result"
                return "$result $ssid"
            }
            continue
        }
    }

    # Couldnt find desired ssid or channel.
    error "find_oem_ap_chanspec ERROR: could not find OEM AP $oem_band GHz ssid $oem_ssid in wl scanresults!"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::find_stas] [arg STAs] [arg types] [opt loose=0]]

    # If [arg STAs] is null, finds STAs that match the list of [arg types].
    # If [arg STAs] is not null, this list of STAs will be checked
    # against the list of [arg types]. If [opt loose] is 1, checking of
    # STA type will be skipped. Returns list of STAs.
}

proc UTF::find_stas {STAs types {loose 0}} {

    # If no sta specified, find all the sta in the testrig that match types.
    set STAs [string trim $STAs]
    if {$STAs == ""} {

        # Find all STA of each type, including clones.
        foreach type $types {
            append STAs " [UTF::get_names_values $type sta names -all]"
        }

        # Remove lan/wan peers, if any.
        if {[info exists ::lan_peer_sta_list] && [info exists ::wan_peer_sta_list]} {
            foreach item "$::lan_peer_sta_list $::wan_peer_sta_list" {
               set i [lsearch -exact $STAs $item]
               if {$i >= 0} {
                   # puts "dropping i=$i item=$item"
                   set STAs [lreplace $STAs $i $i]
               }
            }
        }
    }

    # We need at least one sta to proceed.
    set STAs [lsort -unique $STAs]
    set STAs [string trim $STAs]
    if {$STAs == ""} {
        error "UTF::find_stas ERROR: At least one STA of type $types is\
            needed for this test!"
    }
    # puts "STAs=$STAs"

    # Check STAs are all the correct type.
    foreach sta $STAs {
        if {$loose == 1} {
            # Check that this is a STA, but dont worry about type.
            UTF::check_sta_type $sta *
        } else {
            # STA must be one of specified types.
            UTF::check_sta_type $sta $types
        }
    }

    # Return STAs
    return $STAs
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_ap_rssi] [arg AP] [arg STA]]

    # Returns the rssi from the AP for the specified STA.[para]

    # NB: This only works when the STA is associated to the AP!
}

proc UTF::get_ap_rssi {AP STA} {

    # The AP indexes RSSI info by the mac addr of the desired STA
    # that is associated.
    set sta_macaddr [$STA macaddr]

    # By default the, the AP does not collect RSSI info. The first time
    # we ask the AP for the info, we expect to get back 0. This triggers
    # the AP to start collecting the RSSI info. On subsequent tries, we
    # expect to get a valid RSSI value. However testing has shown that it
    # takes about 10 seconds for the RSSI to stabilize.
    set valid_rssi_cnt 0
    for {set i 0} {$i <= 30} {incr i} {
        set ap_rssi ""
        catch {set ap_rssi [$AP wl rssi $sta_macaddr]}
        if {$ap_rssi != "" && [regexp {^\-\d+$} $ap_rssi]} {
            incr valid_rssi_cnt
            if {$valid_rssi_cnt >= 10} {
                return $ap_rssi
            }
        }
        UTF::Sleep 1
    }

    # RSSI was not available. 
    return "N/A"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_attn_start_value]]

    # Gets the attenuator start value from setuptestbed string.
    # Returns integer
}

proc UTF::get_attn_start_value {} {

    # Get users preferred default for the attenuator.
    set result 15 ;# intermediate value in case user didnt specify one.
    if {[info exists ::UTF::SetupTestBed]} {
        UTF::Message INFO "" "get_attn_start_value ::UTF::SetupTestBed=$::UTF::SetupTestBed"
        if {[regexp -nocase {ALL\s+attn\s+(\d+)} $::UTF::SetupTestBed - temp]} {
            set result $temp
        }
    }

    # Put limit of result
    if {$result > 25} {
        set result 25
    }
    UTF::Message INFO "" "get_attn_start_value result=$result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_build_id] [arg image]]

    # Used to extract the build id from an image pathname.
}

proc UTF::get_build_id {image} {

    # WLAN Builds
    if {[regexp {.*build_} $image]} {
        # Remove text upto & including build_xxxx/
        regsub {.*build_[^/]+/} $image {} id
        # puts "image=$image id=$id"

        # Remove trailing text /build...
        regsub {/build.*} $id {} id
  
        # Remove trailing text /release...
        regsub {/release.*} $id {} id

        # Remove trailing text /src...
        regsub {/src.*} $id {} id

        # Remove PRESERVED/, ARCHIVED/
        regsub {PRESERVED/|ACHIVED/} $id {} id

        # Convert NIGHTLY to TOT
        regsub {NIGHTLY} $id {TOT} id

        # Get brand info in middle.
        set id [split $id /]
        set tag [lindex $id 0]
        set brand [lrange $id 1 end-1]
        set date [lindex $id end]

        # Reformat id for PR standard, easy to copy, no manual reformatting.
        regsub -all "_" $tag "." tag
        set id "${tag}:${date} $brand"

    } else {
        # BlueTooth builds: remove filename
        set id [file dirname "$image"]
        # puts "image=$image id=$id"

        # Keep directory name 6 onwards
        set id [split $id "/"]
        set id [lrange $id 6 end]
        set id [join $id "/"]
    } 

    # Clean up result for web page.
    set id [string trim $id]
    if {$id == ""} {
        set id "&nbsp;"
    }
    # Return id
    UTF::Message INFO "" "get_build_id image=$image id=$id"
    return $id
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_build_notes] [arg image]]

    # Used to extract the selected items from an image pathname.
}

proc UTF::get_build_notes {image} {

    # Look for bmac in image name
    set notes ""
    if {[regexp {\-bmac} $image]} {
        lappend notes bmac
    }
    if {[regexp {high\d+.sys} $image]} {
        lappend notes bmac
    }

    # Look for pktfilter in image name
    if {[regexp {\-pktfilter} $image]} {
        lappend notes pktfilter
    }

    # Clean up result for web page.
    set notes [lsort -unique $notes]
    set notes [string trim $notes]
    if {$notes == ""} {
        set notes "&nbsp;"
    }

    # Return notes
    return $notes
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_build_status] [arg image] [arg code]]

    # Used to get status info from an image & return code.
}

proc UTF::get_build_status {image code} {

    # Look for clues that there is a problem.
    if {[regexp -nocase {(no matching images|file not found|error)} $image - msg]} {
        set status "<font color=\"red\"><b>$msg</b></font>"
        return $status
    } elseif  {$code != 0} {
        set status "<font color=\"red\"><b>issue</b></font>"
        return $status
    } else {
        set status  "<font color=\"green\"><b>OK</b></font>"
        return $status
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_host_mac_addr] [arg host]]

    # On [arg host] finds the MAC address of the first
    # wired ethernet port. Returned MAC address is
    # in the format: xx:xx:xx:xx:xx:xx
}

proc UTF::get_host_mac_addr { host } {

    # Get object host_os, also validates object is defined in the
    # config file.
    set host_os [check_host_os $host]

    # Commands to get MAC address and parsing are OS dependant
    if {$host_os == "WinXP" || $host_os == "Vista"} { 
        # Windows - use: ipconfig /all
        # Even if the media is disconnected, its MAC will show up.
        set resp [$host rexec -quiet -silent ipconfig /all]
        # puts "resp=$resp"

        # We want to ignore all the Wireless, Bluetooth and
        # Tunnel adaptors. Look for the first Ethernet wired
        # connection.
        set lines [split $resp "\n"]
        set lines [string tolower $lines]
        set max_lines [llength $lines]
        set i [lsearch $lines {*ethernet*local*area*connection*}]
        if {$i < 0} {
            # We didnt find a wired connection ==> error
            error "$host get_host_mac_addr ERROR: could not find a wired\
                Ethernet connection, resp=$resp"
        }
        set matched_line [lindex $lines $i]

        # Now look for the physical address. If we hit wireless, 
        # bluetooth or tunnel, we are in trouble. 
        for {set j $i} {$j < $max_lines} {incr j} {
            set line [lindex $lines $j]
            # puts "j=$j line=$line"
            if {[regexp {wireless|bluetooth|tunnel} $line]} {
                error "$host get_host_mac_addr ERROR: could not find a MAC\
                address for i=$i: $matched_line, stopped parsing at\
                j=$j: $line resp=$resp"
            } elseif {[regexp {physical\s*address.*([\-,a-f,0-9]{17})} $line - mac]} {
                # Log & return results
                regsub -all {\-} $mac ":" mac ;# convert to linux format
                UTF::Message INFO "$host" "get_host_mac_addr matched_line=$matched_line mac=$mac"
                return $mac
            }
        }

        # Should not go here!
        error "$host get_host_mac_addr ERROR: could not find a MAC\
            address for i=$i: $matched_line, hit end of lines, resp=$resp"

    } elseif {$host_os == "Linux" || $host_os == "MacOS"} {
        # If the media is disconnected, its MAC will NOT show up.
        set resp [$host rexec -quiet -silent ifconfig]
        # puts "resp=$resp"

        # There does not seem to be any way to tell a wired adaptor
        # from a wireless adaptor. However the wired adaptors are 
        # usually the first one(s) in the list.
        set lines [split $resp "\n"]
        set lines [string tolower $lines]

        # Look for the HWaddr.
        foreach line $lines {
            # puts "line=$line"
            if {[regexp {hwaddr|ether.*([:,a-f,0-9]{17})} $line - mac]} {
                # Log & return results
                UTF::Message INFO "$host" "get_host_mac_addr matched_line=$line mac=$mac"
                return $mac
            }
        }

        # Should not go here!
        error "$host get_host_mac_addr ERROR: could not find a MAC\
            address, hit end of lines, resp=\n$resp"
  
    } else {
        error "$host get_host_mac ERROR: host_os=$host_os not supported!"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_host_name] [arg ip_addr]]

    # Takes ip_addr in format nnn.nnn.nnn.nnn, returns host name string.
}

proc UTF::get_host_name { ip_addr } {

    # Start by using ypmatch directories.
    set ip_addr [string trim $ip_addr]
    set catch_resp [catch {set temp [exec ypmatch $ip_addr hosts.byaddr]} catch_msg]
    if {$catch_resp == 0} {
        set host [lindex $temp 1]
        UTF::Message INFO "" "UTF::get_host_name ypmatch ip_addr=$ip_addr host=$host"
    } else {
        # Unfortunately ping doesnt do a reverse translation of ip_addr to host
        # name, so we are out of options at this point.
        set host ""
        UTF::Message ERROR "" "UTF::get_host_name ypmatch failed,\
            could not get host name for ip_addr=$ip_addr, catch_msg=$catch_msg"
    }

    # Return host name
    set host [string trim $host]
    set host [string tolower $host]
    return $host
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_ip_addr] [arg host]]

    # Get ip address for specified host name for the connection the lab backbone
    # network. This routine will work even if the host is not responding.[para]

    # While UTF has a method ipaddr, it works only on the STA objects for the
    # wireless connections, not the connection to the lab backbone network.
    # Also, it wont work if the host is not responding.[para]

    # Takes host name string, returns ip_addr in format nnn.nnn.nnn.nnn
}

proc UTF::get_ip_addr { host } {

    # Start by using ypmatch directories.
    # NB: ypmatch expects lower case names only.
    set host [string tolower $host]
    set host [string trim $host]
    set catch_resp [catch {set temp [exec ypmatch $host hosts]} catch_msg]
    if {$catch_resp == 0} {
        set ip_addr [lindex $temp 0]
        UTF::Message INFO "" "UTF::get_ip_addr ypmatch host=$host ip_addr=$ip_addr"
    } else {
        # Now try pinging the host. Even if the host is not responding, 
        # ping will translate the host name to IP address and show us
        # that data. This trick does not work if the local host OS is
        # Solaris, due to the terse Solaris ping response.
        set temp ""
        catch {set temp [exec ping $host -c 1]} catch_msg

        # Parse ip address from ping response, both temp & catch_msg.
        if {[regexp {(\d+\.\d+\.\d+\.\d+)} "$temp $catch_msg" - x1]} {
            set ip_addr $x1
            UTF::Message INFO "" "UTF::get_ip_addr ping host=$host ip_addr=$ip_addr"
        } else {
            set ip_addr ""
            UTF::Message ERROR "" "UTF::get_ip_addr parsing ping\
                 response failed, could not get ip address for host=$host, catch_msg=$catch_msg"
        }
    }

    # Return ip_addr
    set ip_addr [string trim $ip_addr]
    set ip_addr [string tolower $ip_addr]
    return $ip_addr
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_name] [arg object]]
    # [arg object] STA or host level object name.[para]

    # This routine is mainly used to get the correct
    # name of an object to show in the log file. It is
    # accepts either a STA name or a higher level host
    # name as the calling parameter. The allows the higher
    # level scripts to not worry about STA vs host distinctions.[para]
    
    # Returns name of host level object.
}

proc UTF::get_name {object} {

    # Get proper host name for case of STA.
    set catch_resp [catch {set x [$object cget -host]} catch_msg]
    if {$catch_resp == 0} {
        set host $x
    } else {
        set host $object
    }
    if {$host == ""} {
        set host $object
    }

    # Return name.
    set catch_resp [catch {set name [$host cget -name]} catch_msg]
    if {$catch_resp != 0} {
        set name $host
    }
    if {$name == ""} {
        set name $host
    }

    return "$name"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_names_values] [arg type] [arg option] [arg ctl] [arg -all]]

    # Used to get object option names & values based on object type.
    # Type must be specified. Option defaults to null. Ctl defaults
    # to names. If you want names and values returned, then specify
    # ctl=names+values[para]

    # By default, data for high level objects only are shown. If you want to
    # see the auto generated objects like ::UTF::DHD::Linux1, then specify
    # the -all option.[para]

    # There are times when you need to get only the Router STA list or
    # only the Cygwin STA list, etc. The example below shows the issue:[para] 

    # UTF::STA info instances [para]

    # This shows all STA regardless of type: ::lan ::wl4a1 ::wan ::wl4a2
    # ::AP ::wl4a3 ::wl4a4 [para]
    
    # This routine allows you to extract the object -option names or name & value
    # pairs based on the object type. Examples:[para]

    # UTF::get_names_values UTF::Router sta [para]

    # AP[para]

    # UTF::get_names_values UTF::Router sta names+values[para]

    # {AP eth2}
}

proc UTF::get_names_values {type {option ""} {ctl names} args} {

    # When someone comes up with a good use for type=all, I will
    # add code to support that. Until then, you specify a single
    # type value for each call to this proc.

    # Get high level list of ojbects for the specified type
    set catch_resp [catch {set object_list [$type info instances]} catch_msg]
    if {$catch_resp != 0} { 
        UTF::Message ERROR "" "UTF::get_names_values type=$type\
            option=$option ctl=$ctl ERROR: $catch_msg"
        return
    }
    set object_list [string trim $object_list]
    set object_list [lsort $object_list]

    # If appropriate, remove lower level auto generated objects.
    if {![string match -nocase *-all* $args]} {
        set temp_list ""
        foreach item $object_list {
            if {[regexp -nocase {^::UTF::.*::.*$} $item]} {
                # puts "UTF::get_names_values removing: $item"
            } else {
                set temp_list "$temp_list $item"
            }
        }
        set object_list $temp_list
    }

    # Now remove the infrastructure objects.
    set temp_list ""
    set infrastructure_list ""
    foreach item $object_list {
        set infra ""
        set catch_resp [catch {set infra [$item cget -infrastructure]} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR "" "UTF::get_names_values $type $item could not cget\
                -infrastructure option: $catch_msg"
            # We make a leap of faith here and assume that this object is NOT
            # an infrastructure item, and keep keep going so that it will be 
            # included in the object_list. Infrastructure objects are usually
            # properly planned and have the infrastructure flag defined and set
            # accordingly. This error is probably caused by an oversight in a new
            # object.
        }
        set infra [string trim $infra]
        set infra [string tolower $infra]
        if {[string is true -strict $infra]} {
            set infrastructure_list "$infrastructure_list $item"
        } else {
            set temp_list "$temp_list $item"
        }
    }
    set object_list $temp_list
    if {$infrastructure_list != ""} {
        UTF::Message INFO "" "UTF::get_names_values skipping\
            infrastructure_list=$infrastructure_list"
    }

    # If option=null, just return the object name list.
    set option [string trim $option]
    if {$option == ""} {
        return $object_list
    }

    # Get option names & values for each object
    set result ""
    set ctl [string trim $ctl]
    set ctl [string tolower $ctl]
    foreach item $object_list {
        set catch_resp [catch {set name_value [$item cget -$option]} catch_msg]
        if {$catch_resp != 0} { 
            set msg "UTF::get_names_values type=$type option=$option ctl=$ctl ERROR: $catch_msg"
            puts "$msg"
            error "$msg"
        }

        # STA names with "%" need to be expanded to full list of STA names.
        if {$option == "sta" && [regexp {^names} $ctl]} {
            set name_value [UTF::Staexpand "$name_value"]
        }

        # Save non-blank names. Values are saved if ctl=names+values.
        set name_value [string trim $name_value]
        if {$name_value == ""} {
            continue
        }
        if {$ctl == "names+values"} {
            # Save both name & value
            if {[llength $name_value] >= 2} { 
                lappend result "$name_value"
            } else {
                lappend result "$name_value {}"
            }

        } else {
            # Save name(s) only.
            foreach {name value} $name_value {
                lappend result "$name"
            }
        }
    }

    # Return sorted list
    set result [lsort -dictionary $result]
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_new_pids_by_name] [arg object] [arg name]
    # [arg current_pids] [arg pid_cnt] [arg verbose]]

    # Get new pids that match the specified name that are not already
    # in the current_pid list. 
    # [arg object] host object to get new pids from.
    # [arg name] process name to be found. Wildcard case insensitive
    # matching of the process table contents is used.
    # [arg current_pids] space separated list of integer process id's
    # already running that match arg name.
    # [arg pid_cnt] integer count of new pids expected to be found.
    # -1 means dont check how many pids are found or not.
    # [arg verbose] flag controls logging of pid details. Default=0
    # means no logging. Any other value gives full logging.[para]

    # Returns space separated list of new pids matching the specified
    # name. Throws an error if the pid_cnt doesnt match the actual
    # number of new matching pids found.
}

proc UTF::get_new_pids_by_name {object name current_pids pid_cnt {verbose 0}} {

    # Log calling parameters
    UTF::Message INFO "" "UTF::get_new_pids_by_name object=$object\
        name=$name current_pids=$current_pids pid_cnt=$pid_cnt verbose=$verbose"

    # Get pids that match name.
    set all_pids [UTF::check_pid_by_name $object $name $verbose]

    # Find the new pids that are NOT in the current_pids list.
    set new_pids [UTF::list_delta $current_pids $all_pids]

    # If requested, verify we got the expected number of new pids.
    if {$pid_cnt >= 0 && [llength $new_pids] != $pid_cnt} {
        error "UTF::get_new_pids_by_name ERROR: found new pids: $new_pids,\
           expected $pid_cnt new pids!"
    }

    # Return new_pids
    UTF::Message INFO "" "UTF::get_new_pids_by_name found: $new_pids"
    return $new_pids
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_oem_ap_ssid] [arg AP]]

    # From the -nvram option in the config file, gets the SSID that OEM [arg AP] is
    # expected to broadcast. Returns: string or throws error.
}

proc UTF::get_oem_ap_ssid { AP } {

    # Look for SSID that OEM AP is supposed to be using.
    set nvram [$AP cget -nvram]
    if {![regexp {wl0_ssid=(\S+)} $nvram - ssid]} {
        error "get_oem_ap_ssid ERROR: For OEM AP $AP, you need to specify\
            wl0_ssid=<ssid> in the config file -nvram option so the script will\
            know it is connecting to the correct AP!"
    }
    regsub -all {\\|\"} $ssid "" ssid
    UTF::Message INFO $AP "get_oem_ap_ssid found ssid=$ssid"
    return $ssid
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_pathloss] [arg pathloss] [arg STA] [arg band] [arg chanspec]]

    # Retrieves pathloss from testrig config file, which can be band, STA and
    # channel specific. Command line [arg pathloss] will override config file.
}

proc UTF::get_pathloss {pathloss STA band chanspec} {

    # Cleanup calling parms
    set pathloss [string trim $pathloss]
    set STA [string trim $STA]
    # TCL doesnt like variables with "." embedded. Remove band fractional number.
    set band [split $band "."]
    set band [lindex $band 0]
    regsub -all {[gG]} $band "" band
    set chanspec [string trim $chanspec]

    # Command line pathloss takes precedance.
    if {$pathloss == ""} {

        # Ensure there is a numeric default pathloss.
        set pathloss 0 

        # Define hierarchy of testbed pathloss config file variables to check.
        # Start at testrig level, progress by band, then STA, STA+band and STA+channel.
        set pathloss_vars "::testbed_pathloss ::testbed_pathloss_${band}G\
            ::${STA}_pathloss ::${STA}_pathloss_${band}G ::${STA}_pathloss_${chanspec}"
        foreach var $pathloss_vars {
            if {[info exists $var]} {
                set pathloss [set $var]
                UTF::Message INFO "" "get_pathloss found $var=$pathloss"
            }
        }
    } else {
        UTF::Message INFO "" "get_pathloss using command line pathloss=$pathloss"
    }

    # Pathloss must be number, 0 or more.
    if {![regexp {^[\d\.]+$} $pathloss] || $pathloss < 0} {
        error "get_pathloss ERROR:  invalid pathloss $pathloss, must be numeric, 0 or more"
    }

    # Return pathloss.
    return $pathloss
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_pid_list] [arg object] [arg verbose]]
    # [arg object] host object to get active pid list from.
    # [arg verbose] flag controls logging of pid details. Default=0
    # means no logging. Any other value gives full logging.[para]

    # Returns text string with complete list of active processes,
    # 1 line per process.
}

proc UTF::get_pid_list {object {verbose 0}} {

    # Determine OS used by the object. This also validates the object
    # is defined in the config file.
    set host_os [UTF::check_host_os $object]
    set host_name [$object cget -name]

    # Set command according to host_os
    if {$host_os == "WinXP" || $host_os == "Vista" || $host_os == "Win7"} {
        set cmd "ps -efW" ;# -W tells Cygwin to show Windows processes
    } else {
        # Linux, MacOS, etc
        set cmd "ps -ef"
    }

    # Get list of processes from host object.
    # puts "checking object=$object host_name=$host_name host_os=$host_os cmd=$cmd"
    set pid_list [$object rexec -quiet -silent $cmd]

    # Return pid_list
    if {$verbose != 0} { 
        UTF::Message INFO "$host_name" "UTF::get_pid_list object=$object pid_list=$pid_list"
    }
    return $pid_list
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_stream_name] [arg index]]

    # Returns the stream short name from ::branch_list.[para]

    # [arg index] allows you to specify which branch to
    # choose in case there are multiple branches being
    # tested. Choices are: 0, 1, 2 ... end
}

proc UTF::get_stream_name { index } {

    # Check ::branch_list exists
    if {![info exists ::branch_list]} {
        set ::branch_list ""
    }

    # Remove any unknown, PRESERVED & ARCHIVED from ::branch list
    regsub -all -nocase {unknown} $::branch_list "" stream
    regsub -all -nocase {PRESERVED/*} $stream "" stream
    regsub -all -nocase {ARCHIVED/*} $stream "" stream
    set stream [string trim $stream]

    # Pick the item specified by index.
    set stream [lindex $stream $index]

    # blank stream & private build paths are mapped to ---.
    if {$stream == "" || [regexp {/.*/} $stream] } {
        set stream ---
    }

    # Get the short from name, eg: PBR_BRANCH_N_M ==> PBR
    regsub -all -nocase {_.*} $stream "" stream

    # Log and return stream.
    UTF::Message INFO "" "UTF::get_stream_name index=$index\
        branch_list=$::branch_list stream=$stream"
    return $stream
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_temperature] [arg AP] [arg STA] [arg musta]]

    # Call 'wl phy_tempsense' to get device temperature in C.
    # Return temperature readings for AP and STAs.

    # Returns CSV string with: ApTemp, StaTemp(s)
}

proc UTF::get_temperature {AP STA musta} {

    set result ""

    # Get AP temperature.
    set ap_temp  ""
    catch {$AP wl phy_tempsense} ret
    regexp -line {(\d+)\s+\(.*\)} $ret - ap_temp  ;# Try to match first number in "68 (0x44)"
    if {$ap_temp == ""} {
	set ap_temp 0
    }
    append result "$ap_temp,"  ;# Comma is needed so data is properly populated in .csv file

    # Get STA temperature.
    foreach S "$STA $musta" {
	set sta_temp ""
	catch {$S wl phy_tempsense} ret 
	regexp -line {(\d+)\s+\(.*\)} $ret - sta_temp  ;# Try to match first number in "68 (0x44)"
	if {$sta_temp == ""} {
	    set sta_temp 0
	}
    	append result " $sta_temp," ;# Comma is needed so data is properly populated in .csv file
    }

    # log & return result.
    UTF::Message INFO "" "get_temperature: result=$result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::get_testnum]]

    # Returns the digits for the currently running test,
    # eg: 10.12.33. If run outside of the UTF::Try block, you will
    # get the name of your default UTF log file.
}

proc UTF::get_testnum { } {

    set testnum [file tail $UTF::Logfile]
    set testnum [file rootname $testnum]
    set testnum [string trim $testnum]
    return $testnum
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::gnuplot_lines] [arg graph_title] [arg xaxis_title]
    # [arg xaxis_data_type] [arg yaxis_title] [arg y2axis_title] [arg msg]
    # [arg list_series]]

    # Routine will plot an arbitrary number of series of lines on the 
    # same graph. Each data series may or may not use the same set of
    # sample X-axis points. Gnuplot sorts out the data and show lines
    # with point markers so you can see were the data samples line up
    # or not.[para]

    # [arg graph_title] Text string that goes above the graph.
    # [arg xaxis_title] Text string that goes along the bottom X-axis
    # of the graph.
    # [arg xaxis_data_type] specifies the type of data being supplied 
    # for the X-axis data, values: time|numeric|category . Gnuplot needs additional
    # instructions to correctly process time or category based X-axis data.
    # [arg yaxis_title] Text string that goes along the left Y-axis of the graph.
    # [arg y2axis_title] Text string that goes along the right Y-axis of the graph.
    # [arg msg] Text string to show on hyperlink that points to the graph.
    # [arg list_series] List of one or more data series to be graphed. Series
    # must be formated as described below. There must be at least one data
    # sample series supplied to this routine.[para]

    # Data sample series must be formated as one of the following:[para]

    # 1) series_name_string <x1y1|x1y2> Xvalue1 Yvalue1 ... XvalueN YvalueN[para]

    # 2) series_name_string <x1y1|x1y2> time1 Yvalue1 ... timeN YvalueN[para]

    # The series_name_string is used to label the line graph in the legend area.
    # The axis choice x1y1 or x1y2 specify which axis to plot that data series on.
    # The times are formated as: HH:MM:SS . The values are integer or decimal
    # numbers.[para]

    # Returns an html formated string that can be passed up to the UTF 
    # summary web page. The html string contains data for the thumbnail
    # sized graphic, and a hypelink to the full sized graphic file. Variable
    # ::gnuplot_lines_png contains path to .png file.[para]

    # There is a self-test routine for UTF::gnuplot_lines called UTF::gnuplot_test.
}

# Test code for routine below.
proc UTF::gnuplot_test {{num all}} {

    if {$num == 1 || $num == "all"} {
        # Sample COEX timeplot
        set series1 {"BT series 1"   x1y1 11:02:03 3.3 11:02:05  3.4 11:02:07  3.3 11:02:09 3.3}
        set series2 {"BT series 2"   x1y1 11:02:02 3.2 11:02:04  3.3 11:02:06  3.2 11:02:09 3.2}
        set series3 {"WLAN series 3" x1y2 11:02:00 13.3 11:02:05 13.5 11:02:07 13.3 11:02:11 13.3}
        set series4 {"WLAN series 4" x1y2 11:02:01 13.1 11:02:07 13.4 11:02:08 13.3 11:02:10 13.2}
        set ::series_list1 ""
        lappend ::series_list1 "$series1" "$series2" "$series3" "$series4"
        UTF::gnuplot_lines "Main graph top level title" "Time MM:SS" time "BT Mb/s" "WLAN Mb/s" "Time Plot" "$::series_list1"
    }

    if {$num == 2 || $num == "all"} {
        # Sample X-Y plot
        set series5 {"series 5" x1y1 1 5 2 4 3 3 4 4 6 5}
        set series6 {"series 6" x1y1 1 5 2.5 4 3 3 4 4 5 5}
        set ::series_list2 ""
        lappend ::series_list2 "$series5" "$series6"
        UTF::gnuplot_lines "Main graph top level title" "Whatever..." numeric "$" "" "Line Plot" "$::series_list2"
    }

    if {$num == 3 || $num == "all"} {
        # Example for channel sweep. Not all channel numbers are in each series.
        set series7 {"43236 BW20"       x1y1 Ch1 5.0 Ch3 5.4 Ch5 5.3 Ch11 5.6 Ch36 6.2 Ch40 4.4 Ch64 5.9}
        set series8 {"43236 BW40 Lower" x1y1 Ch1 5.1 Ch3 5.5 Ch5 6.3          Ch36 6.4}
        set series9 {"43236 BW40 Upper" x1y1                 Ch5 1.1 Ch11 1.6          Ch40 1.2 Ch64 1.9}
        set ::series_list3 ""
        lappend ::series_list3 "$series7" "$series8" "$series9"
        UTF::gnuplot_lines "Channel Sweep Demo Graph" "Channel #" category "Mb/s" "" "Channel Sweep" "$::series_list3"
    }

    if {$num == 4 || $num == "all"} {
        # Really wide timeplot
        set series10 "" 
        for {set i 0} {$i < 1000} {incr i} {
            append series10 " [clock format $i -format %T] $i"
        }
        set series10  "\"Time series 1\" x1y1 $series10"
        set ::series_list4 ""
        lappend ::series_list4 "$series10"
        UTF::gnuplot_lines "Main graph top level title" "Time MM:SS" time "xyz Mb/s" "" "Time Plot" "$::series_list4"
    }
}

proc UTF::gnuplot_lines {graph_title xaxis_title xaxis_data_type yaxis_title\
    y2axis_title msg list_series} {
    UTF::Message INFO "" "UTF::gnuplot_lines graph_title=$graph_title\
        xaxis_title=$xaxis_title xaxis_data_type=$xaxis_data_type\
        yaxis_title=$yaxis_title y2axis_title=$y2axis_title msg=$msg\
        list_series=$list_series"
    set ::gnuplot_lines_png "" ;# some calling routines want just the filename

    # Check there is some data to plot
    set list_series [string trim $list_series]
    if {$list_series == ""} {
        error "UTF::gnuplot_lines ERROR: no data to plot. list_series is null!"
    }

    # Cleanup calling data.
    set xaxis_data_type [string trim $xaxis_data_type]
    set xaxis_data_type [string tolower $xaxis_data_type]

    # Get proper name of log dir. Dont feed "~" to gnuplot!
    set dir [file nativename [file dir $UTF::Logfile]]

    # Get a filename that is unique in dir, based on test number.
    set self abc
    set testnum [UTF::get_testnum]
    for {set i 1} {$i < 100} {incr i} {
        set fn  "$dir/$testnum.$i.png"
        if {![file exists "$fn"]} {
            set self "$testnum.$i"
            break
        }
    }

    # Create gnuplot script output file name. Existing file, if any, is blown away.
    set plot_file "$dir/$self.plot"
    set catch_resp [catch {set out [open $plot_file w]} catch_msg]
    if {$catch_resp != 0} {
       error "gnuplot_lines ERROR: could not open $plot_file catch_msg=$catch_msg"
    }

    # More initialization
    set i 0
    set max_samples 0
    set plot_cmds_lg "" ;# cmds for large .png
    set plot_cmds_sm "" ;# cmds for small .png
    set temp_file_list "$plot_file" ;# files to erase when done
    set valid_series 0 ;# count of series that have data points
    set y(min,1) 0 ;# keep track of left Y-axis data range
    set y(max,1) 0
    set y(min,2) 0 ;# keep track of right Y-axis data range
    set y(max,2) 0

    # How the data gets stored in temporary files depends on xaxis_data_type.
    if {$xaxis_data_type == "numeric" || $xaxis_data_type == "time"} {

        # Each series of data numeric/values or time/values needs to be put in a
        # separate data file, with one x/value pair per line. Collect the highest
        # sample count.
        foreach series $list_series {

            # Ignore null series
            set series [string trim $series]
            if {$series == ""} {
                continue
            }

            # Open data file for this series. Existing file, if any, is blown away.
            incr i
            set tmp_file "$dir/$self.$i.data"
            append temp_file_list " $tmp_file"
            set catch_resp [catch {set tmp [open $tmp_file w]} catch_msg]
            if {$catch_resp != 0} {
                error "gnuplot_lines ERROR: could not open $tmp_file catch_msg=$catch_msg"
            }

            # Extract the title & axes
            set title [lindex $series 0]
            set axes [lindex $series 1]
            set axes [string tolower $axes]
            set axes [string trim $axes]
            if {$axes != "x1y1" && $axes != "x1y2"} {
                UTF::Message WARN "" "gnuplot_lines title=$title, invalid axes=$axes, using x1y1"
                set axes x1y1
            }
            set series [lreplace $series 0 1]

            # Write data to tmp_file, one time/value pair per line.
            # Keep track of min/max values for each separate Y-axis.
            set total_samples 0
            foreach {time value} $series {
                puts $tmp "$time $value"
                incr total_samples
                if {$axes == "x1y1" && $value < $y(min,1)} {
                    set y(min,1) $value
                }
                if {$axes == "x1y1" && $value > $y(max,1)} {
                    set y(max,1) $value
                }
                if {$axes == "x1y2" && $value < $y(min,2)} {
                    set y(min,2) $value
                }
                if {$axes == "x1y2" && $value > $y(max,2)} {
                    set y(max,2) $value
                }
            }

            # Close the tmp file
            catch {close $tmp}

            # Keep track of largest sample count. This determines the graphs pixel width.
            if {$total_samples > $max_samples} {
                set max_samples $total_samples
            }

            # Add plot commands for this data file only if there are data samples.
            # NB: For time based X-axis, MUST have "using columns" spec
            # NB: The "using spec" MUST be before the "with spec"
            if {$plot_cmds_lg != ""} {
                # Use comma seperator between each file & parameters.
                set plot_cmds_lg "$plot_cmds_lg ,"
                set plot_cmds_sm "$plot_cmds_sm ," 
            }
            if {$total_samples > 0} {
                incr valid_series
                set plot_cmds_lg "$plot_cmds_lg \"$tmp_file\" using 1:2 axes $axes with linespoints t \"$title\""
                set plot_cmds_sm "$plot_cmds_sm \"$tmp_file\" using 1:2 axes $axes with lines"
            } else {
                UTF::Message WARN "" "gnuplot_lines ignoring series that has no data: $title"
            }
        }

    } else {
        # For category data, we create a single data file, with a column for each series.
        # We load each series into series_array, collecting all the different categories
        # to be plotted into a single list . Series are allowed to have some categories
        # missing.
        set category_list ""
        foreach series $list_series {

            # Ignore null series
            set series [string trim $series]
            if {$series == ""} {
                continue
            }

            # Extract the title & axes
            incr i ;# series counter
            set title [lindex $series 0]
            set series_array($i,title) $title
            set axes [lindex $series 1]
            set axes [string tolower $axes]
            set axes [string trim $axes]
            if {$axes != "x1y1" && $axes != "x1y2"} {
                UTF::Message WARN "" "gnuplot_lines title=$title, invalid axes=$axes, using x1y1"
                set axes x1y1
            }
            set series_array($i,axes) $axes
            set series [lreplace $series 0 1]

            # Load remaining series data into series_array.
            # Keep track of min/max values for each separate Y-axis.
            # Collect category values.
            set total_samples 0 
            foreach {category value} $series {
                lappend category_list $category
                set ::series_array($i,data,$category) $value ;# 3rd index avoids stomping on title, axes
                incr total_samples
                if {$axes == "x1y1" && $value < $y(min,1)} {
                    set y(min,1) $value
                }
                if {$axes == "x1y1" && $value > $y(max,1)} {
                    set y(max,1) $value
                }
                if {$axes == "x1y2" && $value < $y(min,2)} {
                    set y(min,2) $value
                }
                if {$axes == "x1y2" && $value > $y(max,2)} {
                    set y(max,2) $value
                }
            }
            if {$total_samples > 0} {
                incr valid_series
            }
        }

        # Check for valid data
        if {$valid_series == 0} {
            error "gnuplot_lines ERROR: no valid series or category data found!"
        }
        # Get sorted unique list of categories
        set category_list [lsort -unique -dictionary $category_list]

        # max sample count is number of categories. This determines the graphs pixel width.
        set max_samples [llength $category_list]

        # Open data file
        set tmp_file "$dir/$self.1.data"
        append temp_file_list " $tmp_file"
        set catch_resp [catch {set tmp [open $tmp_file w]} catch_msg]
        if {$catch_resp != 0} {
            error "gnuplot_lines ERROR: could not open $tmp_file catch_msg=$catch_msg"
        }

        # Add category titles 
        set titles "category"
        for {set j 1} {$j <= $i} {incr j} { 
            set titles "$titles \"$series_array($j,title)\""
        }
        puts $tmp "$titles"

        # Add one line of data per category. 
        foreach category $category_list {
            set line "\"$category\""
            for {set j 1} {$j <= $i} {incr j} { 
                if {[info exists ::series_array($j,data,$category)]} {
                    set line "$line $::series_array($j,data,$category)"
                } else {
                    set line "$line \"\"" ;# Add "" for missing items.
                }
            }
            puts $tmp $line
        }

        # Close the tmp file
        catch {close $tmp}

        # Compose plot commands
        set plot_cmds_lg "" 
        set plot_cmds_sm ""
        for {set j 1} {$j <= $i} {incr j} {
            # NB: trailing commas on commands NOT allowed!
            # Plot command for large png
            set k [expr $j + 1]
            if {$plot_cmds_lg == ""} {
                set plot_cmds_lg "\"$tmp_file\" using $k:xtic\(1\)"
            } else {
                set plot_cmds_lg "$plot_cmds_lg , '' using $k"
            }
            set plot_cmds_lg "$plot_cmds_lg with linespoints title \"$series_array($j,title)\" axes $series_array($j,axes)" 

            # Plot command for thumbnail png.
            if {$plot_cmds_sm == ""} {
                set plot_cmds_sm "\"$tmp_file\" using $k"
            } else {
                set plot_cmds_sm "$plot_cmds_sm , '' using $k"
            }
            set plot_cmds_sm "$plot_cmds_sm with lines axes $series_array($j,axes)" 
        }
    }

    # On Unix, gnuplot uses some relative size, 1-5+.
    # Set the xwidth for the graph.
    set xwidth [expr $max_samples * .02]
    if {$xwidth < 1} {
        set xwidth 1
    }

    # Set gnuplot version specific values.
    set gpver [UTF::GnuplotVersion]
    if {$gpver > 4.0} {
        # This width is still readable, but triggers the browsers magnifying glass.
        set max_width 4
    } else {
        # NB: On older gnuplot, if xwidth is too big, title doesnt appear
        # on graph. Also, key/legend on outside gets lost.
        set max_width 1.9
    }
    if {$xwidth > $max_width} {
        set xwidth $max_width
    }

    # NB: There is an issue in gnuplot about doing 2 graphs from the same plot
    # file. If you do the larger one first, then the smaller one, gnuplot consumes
    # 100% of the CPU over several seconds. If you do the smaller plot first,
    # its way faster. So we do the small_png first!

    # ==================================================================
    # Create thumbnail sized _sm.png in a separate file. Turn off text labels,
    # border, tics & legend.
    set small_png "$dir/${self}_sm.png"
    puts $out "set output \"$small_png\""
    if {$gpver > 4.0} {
        puts $out "set terminal png size 96,39 notransparent"
        puts $out {set tmargin -1; set bmargin -1}
        puts $out {set lmargin -1; set rmargin -1}
    } else {
        puts $out "set terminal png notransparent"
        puts $out "set size 0.15,0.08"
    }
    puts $out "unset border"
    puts $out "unset key"
    puts $out "unset title"
    puts $out "unset xlabel"
    puts $out "unset ylabel"
    puts $out "unset y2label" 
    if {$xaxis_data_type == "time"} {
        puts $out "set xdata time"
        puts $out "set timefmt \"%H:%M:%S\""
    }
    puts $out "unset xtics"
    puts $out "unset ytics"
    puts $out "unset y2tics"
    # Setting yrange ensures mogrified graph doesnt magnify issues.
    puts $out "set yrange \[$y(min,1):\]"
    puts $out "set y2range \[$y(min,2):\]"
    puts $out "unset grid"
    puts $out "plot $plot_cmds_sm\n"

    # ==================================================================
    # Start full sized plot. Add the gnuplot script boiler plate items.
    set png_file "$dir/$self.png"
    set timestamp [clock format [clock seconds] -format "%Y-%m-%d %H:%M:%S"]
    puts $out "set output \"$png_file\""
    if {$gpver > 4.0} {
        puts $out "set terminal png size [expr {640*$xwidth}],480 notransparent medium"
        puts $out {set tmargin -1; set bmargin -1}
        puts $out {set lmargin -1; set rmargin -1}
    } else {
        puts $out "set terminal png notransparent"
        puts $out "set size $xwidth,1"
    }
    puts $out "set border"
    if {$xwidth <= 1} {
        puts $out "set key outside"
    } else {
        # For wide graphs, put key/legend inside graph at bottom left
        puts $out "set key bottom left"
    }
    puts $out "set title \"$graph_title\""
    puts $out "set xlabel \"\\n$xaxis_title           $timestamp\""
    puts $out "set ylabel \"$yaxis_title\""
    puts $out "set y2label \"$y2axis_title\""
    if {$xaxis_data_type == "time"} {
        puts $out "set xdata time"
        puts $out "set timefmt \"%H:%M:%S\""
        puts $out "set xtics rotate by 90"
        if {$gpver >= 4.4} {
            # Needed for F15. Rotated xtics show up inside main graph unless
            # you push them down.
            puts $out "set xtics offset 0,graph -0.15"
        }
    } else {
        puts $out "set xtics"
    }

    # Set the ytic increments based on collected data ranges.
    # Choose from a fixed series of values to ensure gridlines
    # from left y-axis ytics will line up with right y-axis y2tics.
    # We also ensure the origin (0) is shown on both y-axes, so that
    # minor data fluctuations arent exagerated by gnuplot.
    for {set i 1} {$i <= 2} {incr i} {
        # Find largest range that spans 0 on the Y-axis.
        set y(range,$i) [expr abs($y(max,$i) - $y(min,$i))]
        set temp1 [expr abs($y(min,$i))]
        set temp2 [expr abs($y(max,$i))]
        if {$temp1 > $y(range,$i)} {
            set y(range,$i) $temp1
        }
        if {$temp2 > $y(range,$i)} {
            set y(range,$i) $temp2
        }

        # Choose ytic size based on 5 major ticks minimum covering the Y range.
        set y(tic,$i) [expr $y(range,$i) / 5]

        # Adjust tic size to one of the fixed values below so that
        # we get reasonable looking values on the Y-axes.
        if {$y(tic,$i) < 0.02} {
            set y(tic,$i) 0.01
        } elseif {$y(tic,$i) >= 0.02 && $y(tic,$i) < 0.05} {
            set y(tic,$i) 0.02
        } elseif {$y(tic,$i) >= 0.05 && $y(tic,$i) < 0.1} {
            set y(tic,$i) 0.05
        } elseif {$y(tic,$i) >= 0.1 && $y(tic,$i) < 0.2} {
            set y(tic,$i) 0.1
        } elseif {$y(tic,$i) >= 0.2 && $y(tic,$i) < 0.5} {
            set y(tic,$i) 0.2
        } elseif {$y(tic,$i) >= 0.5 && $y(tic,$i) < 1.0} {
            set y(tic,$i) 0.5
        } elseif {$y(tic,$i) >= 1.0 && $y(tic,$i) < 2.0} {
            set y(tic,$i) 1.0
        } elseif {$y(tic,$i) >= 2.0 && $y(tic,$i) < 5.0} {
            set y(tic,$i) 2.0
        } elseif {$y(tic,$i) >= 5.0 && $y(tic,$i) < 10.0} {
            set y(tic,$i) 5.0
        } elseif {$y(tic,$i) >= 10.0 && $y(tic,$i) < 25.0} {
            set y(tic,$i) 10.0
        } else {
            set y(tic,$i) 25.0
        }

        # Set the Y axis start value to 0 to include origin on Y-axes. We have to allow
        # for negative start range.
        set y(start,$i) 0
        if {$y(min,$i) < 0} {
            set temp [expr int($y(min,$i) / $y(tic,$i))]
            if {[expr  $temp * $y(tic,$i)] != $y(tic,$i)} {
                incr temp -1
            }
            set y(start,$i) [expr $temp * $y(tic,$i)]
        }
    }

    # If either Y-axis has no data, use the settings of the other Y-axis.
    # This prevents the Y-axis from getting lots of small tics.
    if {$y(min,1) == 0 && $y(max,1) == 0} {
        set y(tic,1) $y(tic,2)
        set y(start,1) $y(start,2)
    }
    if {$y(min,2) == 0 && $y(max,2) == 0} {
        set y(tic,2) $y(tic,1)
        set y(start,2) $y(start,1)
    }

    # Last of the plot commands
    puts $out "set ytics  $y(start,1),$y(tic,1)"
    puts $out "set y2tics $y(start,2),$y(tic,2)"
    puts $out "set yrange  \[$y(start,1):\]"
    puts $out "set y2range \[$y(start,2):\]"
    puts $out "set grid ytic"
    puts $out "plot $plot_cmds_lg"

    # Close the script file, or wgnplot will read from the
    # end of the file and do nothing...
    close $out

    # Have gnuplot run the script file. gnuplot doesnt always return
    # any errors if something goes wrong. However warnings do show
    # up as errors to TCL, so we filter out the warnings.
    catch {file delete $png_file}
    if {$valid_series > 0} {
        set catch_resp [catch {exec $::UTF::Gnuplot $plot_file} catch_msg]

        # Filter out the gnuplot warnings.
        if {$catch_resp != 0 && ![string match -nocase *warning* $catch_msg]\
            && ![string match -nocase "*font*" $catch_msg]} {
            error "gnuplot_lines ERROR: gnuplot: $catch_msg"
        }

        # Did we get an output file?
        if {[file exists "$png_file"]} {
            # puts "found $png_file file"
        } else {
            error "gnuplot_lines ERROR: $png_file file not created, $catch_msg"
        }

    } else {
       error "gnuplot_lines ERROR: no valid data series found!"
    }

    # Create even smaller image. Use catch in case mogrify is
    # not installed on the localhost.
    set catch_resp [catch {exec /usr/bin/mogrify -transparent white -trim $small_png} catch_msg]
    if {$catch_resp != 0} {
        UTF::Message WARN "" "gnuplot_lines mogrify: $catch_msg"
    }

    # Convert small png into a base64-encoded data: url.
    set f [open $small_png]
    fconfigure $f -translation binary
    set data [base64::encode -maxlen 0 [read $f]]
    close $f

    # Clean up all the temp files. We keep the the small_png
    # file for the benefit of IE users.
    foreach tmp $temp_file_list {
        catch {file delete $tmp}
    }

    # Add thumb-nail & full-size png files to results string.
    set ::gnuplot_lines_png $png_file ;# some calling routines want just the filename
    return "html:<!--\[if IE]><img src=\"$small_png\" alt=\"url\" /><!\[endif]--><!\[if !IE]><img src=\"data:image/png;base64,$data\" alt=\"data\" /><!\[endif]> <a href=\"$png_file\">$msg</a>"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::gnuplot_rvr_lines] [arg csv_file] [arg out_prefix] [arg title]
    # [arg direction] [arg yaxis] [arg type] [arg list_style_legends_columns_lt_lw_pt_ps]
    # [opt rampdown] [opt zerosuppress] [opt normalize] [opt logscale] [opt cleanedges] 
    # [opt fastrampup]]

    # Takes the specified [arg csv_file] that was generated by the RvR test
    # and plots RvR line graphs with title [arg title] & [arg direction],
    # [arg yaxis] text & [arg type] pathloss for the specified parameter groups
    # in [arg list_style_legends_columns_lt_lw_pt_ps].[para]

    # [arg list_legends_columns_lt_lw_pt_ps] is formated as a list: style1 legend1
    # column1 lt1 lw1 pt1 ps1 ... styleN legendN columnN ltN lwN ptN psN[para]

    # The styleN are gnuplot styles, like linespoints, errorlines, etc.
    # The legendN are text strings. The columnN are column integer number indexes
    # for the data in the [arg csv_file] that is to be plotted. The ltN, lwN, ptN and psN
    # are style line values defined by gnuplot.[para]

    # When style=errorlines, it is assumed that the low and high values to be
    # plotted are in the 2 CSV columns to the immediate right of the column
    # specified for the mean value.[para]

    # When style=image, only one set of data can be plotted in heat map graph.
    # The complete range of numbers is expected to be in a space separated list
    # of numbers in the single CSV column. Also, gnuplot version 4.2 or higher
    # is needed for image heatmap graphs.[para]

    # [opt rampdown] allows graphing of only the descending left side of an
    # RvR graph and ignores the right side rampup data. [opt zerosuppress]
    # removes zeros from the data, usefull for RSSI plots. [opt normalize]
    # will convert style=image data to percentages before drawing the graph in
    # a linear % scale. [opt logscale] will use data as is  
    # when drawing the graph in a logarithmic % scale.
    # [opt cleanedges] will remove leading and trailing zeros from the edges
    # of the curve, but not the bottom trough. [opt fastrampup] is needed to
    # ensure X-tics are handled properly for noncontinuguous fast rampup graph.
    # [opt xcol] is used for choosing which column in the CSV file is used for
    # the X-axis values. The default is column 0. [para]

    # Returns filename [arg out_prefix].png [para]

    # There are a number of environment variables that can be used to override the
    # default graph properties. These variables are:
    #   ::gnuplot_rvr_title   - custom RvR graph title
    #   ::gnuplot_rvr_xaxis   - custom X-axis title 
    #   ::gnuplot_rvr_xtics   - custom X-asix tics (default to 10; can be set in UTF config file) 
    #   ::gnuplot_rvr_xlabel  - custom X-axis label
    #   ::gnuplot_rvr_ylabel  - custom Y-axis label
    #   ::gnuplot_rvr_palette - custom palette
    #   ::gnuplot_rvr_logscale_low_limit
    #
}

proc UTF::gnuplot_rvr_lines {csv_file out_prefix title direction yaxis type\
    list_styles_legends_columns_lt_lw_pt_ps args} {
    UTF::Message INFO "" "gnuplot_rvr_lines csv_file=$csv_file\
        out_prefix=$out_prefix title=$title direction=$direction yaxis=$yaxis type=$type\
        list_styles_legends_columns_lt_lw_pt_ps=$list_styles_legends_columns_lt_lw_pt_ps\
        args=$args"

    # Initialization
    if {[regexp {^/\S+} $out_prefix]} {
        # Calling routine has provided a fullpath for output files.
        set dir ""
    } else {
        set dir [file nativename [file dirname $UTF::Logfile]]
    }
    if {![info exists ::gnuplot_rvr_title]} {
        set ::gnuplot_rvr_title "RvR -"
    }
    if {![info exists ::gnuplot_rvr_xaxis]} {
        set ::gnuplot_rvr_xaxis "Path Loss (dB)"
    }
    if {![info exists ::gnuplot_rvr_xtics]} {
        set ::gnuplot_rvr_xtics 10
    }
    if {![regexp {^\d+$} $::gnuplot_rvr_xtics] || $::gnuplot_rvr_xtics < 1} {
        set ::gnuplot_rvr_xtics 10
        UTF::Message WARN "" "gnuplot_rvr_lines: Invalid ::gnuplot_rvr_xtics, using $::gnuplot_rvr_xtics"
    }
    if {![info exists ::gnuplot_rvr_palette]} {
        set ::gnuplot_rvr_palette {( 0 "black", 500 "blue", 10000 "red", 50000 "yellow", 90000 "white")}
    }
    if {![info exists ::gnuplot_rvr_logscale_low_limit]} {
	set ::gnuplot_rvr_logscale_low_limit 0.001
    }

    # Open the csv_file for reading.
    if {![file exists "$csv_file"]} {
        error "gnuplot_rvr_lines ERROR: file $csv_file not found"
    }
    set csvIn [open $csv_file r] 

    # Check out_prefix is not null.
    set out_prefix [string trim $out_prefix]
    if {$out_prefix == ""} {
        error "gnuplot_rvr_lines ERROR: out_prefix is null"
    }

    # Check title is not null.
    set title [string trim $title]
    if {$title == ""} {
        error "gnuplot_rvr_lines ERROR: title is null"
    }

    # Check direction is not null.
    set direction [string trim $direction]
    if {$direction == ""} {
        error "gnuplot_rvr_lines ERROR: direction is null"
    }

    # Check type is not null.
    #set type [string trim $type]
    #if {$type == ""} {
    #    error "gnuplot_rvr_lines ERROR: type is null"
    #}

    # Check yaxis is not null.
    set yaxis [string trim $yaxis]
    if {$yaxis == ""} {
        error "gnuplot_rvr_lines ERROR: yaxis is null"
    }

    # Check we have some data to plot.
    set list_styles_legends_columns_lt_lw_pt_ps [string trim $list_styles_legends_columns_lt_lw_pt_ps]
    if {$list_styles_legends_columns_lt_lw_pt_ps == ""} {
        error "gnuplot_rvr_lines ERROR: you must specify at least one style/legend/column/lt/lw/pt/ps parameter group"
    }

    # Create one output .gpN file for each style/legend/column/line/point that is to be plotted.
    set array_max -1
    set array(style) ""
    set array(column) ""
    set array(file) ""
    set array(handle) ""
    set array(legend) ""
    set array(lt) ""
    set array(lw) ""
    set array(ps) ""
    set array(pt) ""
    foreach {style legend column lt lw pt ps} $list_styles_legends_columns_lt_lw_pt_ps {
        # Check legend not null
        set style [string trim $style]
        set legend [string trim $legend]
        set lt [string trim $lt]
        set lw [string trim $lw]
        set pt [string trim $pt]
        set ps [string trim $ps]
        if {$style == "" || $lt == "" || $lw == "" || $pt == "" || $ps == ""} {
            UTF::Message WARN "" "gnuplot_rvr_lines: ignoring $style $legend $column $lt $lw $pt $ps"
            continue
        }

        # Check column is integer 0 or more.
        set column [string trim $column]
        if {![regexp {^\d+$} $column] || $column < 0} {
            UTF::Message WARN "" "gnuplot_rvr_lines: ignoring $legend $column $lt $lw $pt $ps"
            continue
        }

        # Open a temp file for this series. Existing file, if any, is blown away.
        incr array_max
        set tmp_file "$dir/${out_prefix}.gp${array_max}"
        set catch_resp [catch {set handle [open $tmp_file w 0644]} catch_msg]
        if {$catch_resp != 0} {
            error "gnuplot_rvr_lines ERROR: could not open $tmp_file catch_msg=$catch_msg"
        }
        lappend array(style) $style
        lappend array(column) $column
        lappend array(file) $tmp_file
        lappend array(handle) $handle
        lappend array(legend) $legend
        lappend array(lt) $lt
        lappend array(lw) $lw
        lappend array(pt) $pt
        lappend array(ps) $ps
        set nonzero_array($column) "" ;# track where nonzero data starts
    }

    # Did we get at least one valid style/legend/column/lt/lw/pt/ps group?
    # parray array
    if {$array_max < 0} {
        error "gnuplot_rvr_lines ERROR: no valid pairs found in list_styles_legends_columns_lt_lw_pt_ps=$list_styles_legends_columns_lt_lw_pt_ps"
    }

    # Set gnuplot version specific values.
    set gpver [UTF::GnuplotVersion]
    if {$gpver > 4.0} {
        # This width is still readable, but triggers the browsers magnifying glass.
        set max_width 4
    } else {
        # NB: On older gnuplot, if xwidth is too big, title doesnt appear
        # on graph. Also, key/legend on outside gets lost.
        set max_width 1.9
    }

    # Extra checks for style=image.
    set y_max 0 ;# needed for correct y-range for image graphs.
    if {[regexp {image} $array(style)]} {
        # There can be only one style/legend/column/lt/lw/pt/ps group.
        if {$array_max > 0} {
            error "gnuplot_rvr_lines ERROR: for style=image, you can plot\
                only 1 image heatmap graph at a time!"
        }

        # Need gnuplot 4.2 or higher for image heatmaps.
        if {$gpver < 4.2} {
            error "ERROR: gnuplot_rvr_lines installed gnuplot is version $gpver,\
                you need 4.2 or higher to do image heatmap graphs!"
        }

        # Experience has shown that we need to know the maximum y value for
        # the whole graph before we start putting the x-y-z data in the .gpN
        # file. If we start off with say y_max 7, and part way thru the file
        # we start putting out sequences with y_max = 15, gnuplot gets very
        # upset and gives the error: NOTICE:  A triangle/quadrangle/pentagon
        # clipping algorithm needs to be added for pixels at the boundary. Image
        # may lie outside borders in some instances. To avoid this error, we
        # read the whole file just looking for y_max up front.
        set csvLineNum 0
        while {![eof $csvIn]} {
            set line [gets $csvIn]
            # Parse CSV formatted data, skip non-numeric header lines.
            set fields [split $line ","]
            set str [ lindex $fields 0 ]
            if {![regexp {^[\d\.]+$} $str]} {
                continue
            }

            # Extract the desired column, find y_max.
            set column [lindex $array(column) $array_max]
            set value  [lindex $fields $column]
            set value  [string trim $value]
            set len [llength $value]
            if {[expr $len - 1] > $y_max} {
                set y_max [expr $len - 1]
            }
            incr csvLineNum
        }

        # Add 0.5 so top row will be same size as other rows.
        set y_max [expr $y_max + 0.5]

        # Reset the CSV file handle to the start of the file. This ensures the
        # next while loop gets some data.
        seek $csvIn 0
    }

    # Set flag for processing only the rampdown left side of the RvR curve.
    set rampdown 0
    if {[regexp {rampdown} $args]} {
        set rampdown 1
    }

    # Set flag to suppress zeros on graph.
    set zerosuppress 0
    if {[regexp {zerosuppress} $args]} {
        set zerosuppress 1
    }

    # Set flags for logscale, normalize and PER image data on graph.
    set cbtitle "Count #"
    set normalize 0
    set logscale 0
    set perhistogram 0
    if {[regexp {logscale} $args]} {
        set logscale 1
        set cbtitle "PerCent %"
    }  
    if {[regexp {normalize} $args]} {
	set normalize 1
	set cbtitle "PerCent %"
    }
    if {[regexp {perhistogram} $args]} {
	set perhistogram 1
	set cbtitle "PerCent %"
    }

    # Set flag to clean zeros on from just the edges of the graph.
    set cleanedges 0
    if {[regexp {cleanedges} $args]} {
        set cleanedges 1
    }

    # Set flag for fastrampup.
    set fastrampup 0
    if {[regexp {fastrampup} $args]} {
        set fastrampup 1
    }

    # Choose CSV column to use for X-axis data
    if {![regexp {xcol\s*(\d+)} $args - xcol]} {
        set xcol 0 ;# default for backwards compatibility
    }

    # Process the csv_file one line at a time, adding data to each of the .gpN files.
    set csvLineNum 0
    set found_data "no"
    set hdr_cnt 0 
    set last_loss -1
    set last_xtic ""
    set lines_read_since_xtic 0
    set NewXLabel ""
    set path_loss -1
    set start_loss ""
    set start_rampup 0
    set xtic_cnt 0
    set z_max 1 ;# needed for correct color bar z-range for image graphs.
    set z_min 2000000000
    while {![eof $csvIn]} {
        set line [gets $csvIn]
        # Parse CSV formatted data, skip non-numeric header lines.
        set fields [split $line ","]
        set str [ lindex $fields $xcol ]
        if {![regexp {^[\d\.]+$} $str]} {
            # For backwards compatibility, ie xcol=0, we skip all non-numeric lines.
            # For xcol>0, we assume there are 2 header lines max. This allows for 
            # non-numeric data to be used on the X-axis.
            incr hdr_cnt
            if {$xcol == 0 || $hdr_cnt <= 2} {
               continue
            }
        }
        incr lines_read_since_xtic

        # Keep track of loss. If descending and rampdown=1, we are done.
        set path_loss [lindex $fields 0]
        if {$xcol == 0 && $path_loss <= $last_loss && $rampdown == 1} {
            UTF::Message INFO "" "gnuplot_rvr_lines: rampdown=$rampdown stopping at path_loss=$path_loss"
            break
        }

        # For fastrampup graph, save the start_loss
        if {$xcol == 0 && $start_loss == "" && $fastrampup == 1} {
            set start_loss $path_loss
        }

        # We want to extract a meaningful series of X-axis xtic labels.
        # By default, we pick data values that are multiples of 10.
        # This is also done to force the X-axis to increase then decrease,
        # so RvR curve is shown descending, then ascending.
        if {![regexp {^[\d\.\-]+$} $path_loss]} {
           set path_loss -1
        }
	# Comment out when adding -attnstep=0.5 support. 02/04/2014
        #set path_loss [expr int($path_loss)]
        #if {[expr $path_loss % $::gnuplot_rvr_xtics] == 0 || $xcol > 0}
        if {[expr fmod($path_loss, $::gnuplot_rvr_xtics)] == 0 || $xcol > 0} {
	    set path_loss [expr int($path_loss)]
            # Gnuplot doesnt like trailing commas for xtic labels.
            # Only add comma before adding more data. Avoid repeated
            # duplicate labels for fastrampup graph.
            if {$fastrampup == 0 || ($fastrampup == 1 && $path_loss != $last_xtic) || $xcol > 0} {
                if {$NewXLabel != ""} {
                   set NewXLabel "$NewXLabel,"
                }
                if {$xcol == 0} {
                    set NewXLabel "$NewXLabel\"$path_loss\" $csvLineNum"
                    set last_xtic $path_loss
                } else {
                    set NewXLabel "$NewXLabel\"$str\" $csvLineNum"
                }
                incr xtic_cnt
                set lines_read_since_xtic 0
            }
        }

	if {[expr fmod($path_loss, 5)] == 0} {
	    set path_loss [expr int($path_loss)]
	}

        # We need an X-tic showing the start of the rampup.
        if {$xcol == 0 && $start_rampup == 0 && $path_loss <= $last_loss} {
            # We have just started the rampup.
            if {$NewXLabel != ""} {
               set NewXLabel "$NewXLabel,"
            }
            set NewXLabel "$NewXLabel\"$path_loss\" $csvLineNum"
            set start_rampup 1
            set last_xtic $path_loss
            incr xtic_cnt
            set lines_read_since_xtic 0
        }

        # For fastrampup, we may not get an X-tic for the final attenuator
        # value, even though it is repeated, so we force 1 value into the list
        # when ramping up.
        if {$xcol == 0 && $fastrampup == 1 && $start_loss != "-" && $path_loss <= $last_loss &&\
            $path_loss == $start_loss} {
            if {$path_loss != $last_xtic} {
                if {$NewXLabel != ""} {
                   set NewXLabel "$NewXLabel,"
                }
                set NewXLabel "$NewXLabel\"$path_loss\" $csvLineNum"
                set start_loss "-"
                set last_xtic $path_loss
                incr xtic_cnt
                set lines_read_since_xtic 0
            }
        }
        set last_loss $path_loss

        # It is possible with some attnstep settings that we wont have nice
        # data value multiples of 10. Or we start off with nice multiples of 10,
        # but due to the way we start/stop the test based on the presence of 
        # beacons, combined with attnstep>1, we may end up with odd numbers only.
        # So we add every 10th data point, regardless of value to ensure we have
        # xtics all along the graph.
        if {$lines_read_since_xtic >= $::gnuplot_rvr_xtics} {
            # Gnuplot doesnt like trailing commas for xtic labels.
            # Only add comma before adding more data.
            if {$NewXLabel != ""} {
                set NewXLabel "$NewXLabel,"
            }
            if {$xcol == 0} {
                set NewXLabel "$NewXLabel\"$path_loss\" $csvLineNum"
            } else {
                set NewXLabel "$NewXLabel\"$str\" $csvLineNum"
            }
            incr xtic_cnt
            set lines_read_since_xtic 0
        }

        # Value to store in .gpN file is extracted by column from CSV line.
        # We store the csvLineNum in the .gpN files as gnuplot
        # is using them as a relative offset on the X-axis. The
        # xtic labels provide the actual values for the X-axis.
        # NB: This effectively rounds the pathloss to integer values on the graph.
        for {set i 0} {$i <= $array_max} {incr i} {

            # Get gnuplot parameters from array
            set style  [lindex $array(style) $i]
            set column [lindex $array(column) $i]
            set handle [lindex $array(handle) $i]
            set value  [lindex $fields $column]
            set value  [string trim $value]
            if {$value != ""} {
                set found_data "yes"
            }

            # Crunch non-numeric values to 0. Otherwise gnuplot generates random
            # values in the graph. Space separated list of numbers is OK.
            if {![regexp {^[\d\.\-\s]+$} $value]} {
                UTF::Message ERROR "" "gnuplot_rvr_lines: crunched value=$value to 0, column=$column csvLineNum=$csvLineNum"
                set value 0
            }

            # Keep track of path_loss for the first time this specific column
            # shows a nonzero data value. This is needed for cleaning zero data
            # from the leading (left) & trailing (right) sides of the graph.
            if {$nonzero_array($column) == "" && $value != "" && $value != 0} {
                set nonzero_array($column) $path_loss
            }

            # RSSI graph usually need ALL zero values suppressed.
            if {$zerosuppress && $value == 0} {
                # Dont add entry to .gpN file
                continue
            }

            # Composite graphs need the leading / trailing edges cleaned.
            # NB: We want to display all the zeros in trough of the RvR graph.
            # Logic for cleaning leading (left) edge of graph.
            if {$cleanedges && $nonzero_array($column) == "" && $value == 0} {
                # Dont add entry to .gpN file
                continue
            }

            # Logic for cleaning trailing (right) edge of graph.
            if {$cleanedges && $nonzero_array($column) != "" &&\
                $path_loss <= $nonzero_array($column) && $value == 0} {
                # Dont add entry to .gpN file
                continue
            }

            # For errorlines, we assume that the low / high values are in
            # the columns adjacent to the mean value.
            if {$style == "errorlines"} {
                set j [expr $column + 1]
                set low_value [lindex $fields $j]
                set k [expr $column + 2]
                set high_value [lindex $fields $k]
                puts $handle "$csvLineNum $value $low_value $high_value"

            } elseif {$style == "image"} {

                # The error: "GNUPLOT (plot_image):  Number of pixels cannot be 
                # factored into integers matching grid." seems to be triggered
                # if you dont fill out all the data points in the heatmap. So
                # we add zeros as needed to pad the data out to full size.
                if {$value == ""} {
		    if {$perhistogram == 1} {
			set value -1
		    } else {
                    	set value 0
		    }
                }
                set len [llength $value]
                # NB: y_max is usually not integer, usually ends in .5
                for {set m $len} {$m < [expr $y_max]} {incr m} {
		    if {$perhistogram == 1} {
                    	set value "$value -1"
		    } else {
                    	set value "$value 0"
		    }
                }
                set len [llength $value]

                # Value is expected to be a list of numbers. The y value is implied
                # from the position of the number in the list. If requested, convert data to percentage.

                # Total up numbers in value list.
                set total 0
                if {$normalize == 1} {
                    # Total up numbers in value list
                    foreach z $value {
                        set z [UTF::clean_number $z]
                        set total [expr $total + $z];#z can be decimal fraction
                    }
                }		
                if {$total == 0} {
                    set total 1 ;# prevent divide by 0 errors
                }

                # We output the data as {x y z} triplets, one triplet per line.
                set y 0
                foreach z $value {
                    if {$normalize == 1} {
                        # Convert data to percentage.
                        set z [UTF::clean_number $z] 
                        set z [expr double($z * 100) / $total]
                        set z [format "%.2f" $z]
                    }
                    puts $handle "$csvLineNum $y $z"
                    incr y
                    if {$z > $z_max } {
                        set z_max $z
                    }
                    if {$z < $z_min } {
                        set z_min $z
                    }
                }

                # The gnuplot heatmap.dem suggests you need a blank line after every
                #  row. Testing shows it is not needed, but also doesnt do any harm.
                puts $handle ""

            } else {
                puts $handle "$csvLineNum $value"
            }
        }
        incr csvLineNum
    }

    # Cleanup from creating .gpN files.
    for {set i 0} {$i <= $array_max} {incr i} {
        set handle [lindex $array(handle) $i]
        catch {close $handle}
    }
    catch {close $csvIn}

    # Check if we found any data.
    if {$found_data == "no"} {
        # Take out the trash.
        for {set i 0} {$i <= $array_max} {incr i} {
            set file [lindex $array(file) $i]
            catch {file delete $file}
        }
        return "no_data_to_plot!"
    }

    # Generate the main plot command for gnuplot.
    set plot_cmd ""
    for {set i 0} {$i <= $array_max} {incr i} {
        set file   [lindex $array(file) $i]
        set legend [lindex $array(legend) $i]
        set style  [lindex $array(style) $i]
        if {$plot_cmd != ""} {
            set plot_cmd "${plot_cmd},"
        }
        # Line styles are defined below, one per field being plotted.
        set plot_cmd "$plot_cmd \"$file\" title \"$legend\" with $style ls [expr $i + 1]"
    }

    # Width of graph depends on how many xtics there are to plot.
    if {$NewXLabel == ""} {
        set NewXLabel "\"$path_loss\" $csvLineNum"
        set xtic_cnt 1
    }
    set xwidth [expr $xtic_cnt * .05]
    if {$xwidth < 1} {
        set xwidth 1
    }
    if {$xwidth > $max_width} {
        set xwidth $max_width
    }

    # Generate the gpc file.
    # Note: All .gpc files are removed afterwards.    
    # Note: If you want to see them for debugging purpose, comment out the deletion code later in this routine. 
    set gpc_file "$dir/${out_prefix}.gpc"
    set gpc_handle [open $gpc_file w 0644]
    set png_file "$dir/${out_prefix}.png"
    if {$gpver > 4.0} {
        puts $gpc_handle "set terminal png size [expr {640*$xwidth}],480 notransparent medium"
        puts $gpc_handle {set tmargin -1; set bmargin -1}
        puts $gpc_handle {set lmargin -1; set rmargin -1}
    } else {
        puts $gpc_handle "set terminal png notransparent"
        puts $gpc_handle "set size $xwidth,1"
    }
    puts $gpc_handle "set output \"$png_file\""
    if {$direction == "-" || $direction == "_"} {
        puts $gpc_handle "set title '$::gnuplot_rvr_title $title'"
    } else {
        puts $gpc_handle "set title '$::gnuplot_rvr_title $title ($direction)'"
    }
    #set timestamp [clock format [clock seconds] -format "%Y-%m-%d %H:%M:%S"]
    #puts $gpc_handle "set xlabel '$type $::gnuplot_rvr_xaxis           $timestamp'"
    puts $gpc_handle "set xlabel '$type $::gnuplot_rvr_xaxis'"
    puts $gpc_handle "set ylabel '$yaxis'"
    puts $gpc_handle "set grid"

    # For wide graphs, put key/legend inside graph at bottom left
    #if {$xwidth <= 1} {
    #    puts $gpc_handle "set key outside center under horizontal"
    #} else {
    #    puts $gpc_handle "set key bottom left"
    #}
    puts $gpc_handle "set key below"

    # Add 'mutx gain' labels onto composite graphs, only if test is for SU-MU comparison.
    if {[regexp {^sumu$} $::apmodes] || [regexp {^musu$} $::apmodes]} {
    	set y_position 0.9
	foreach i $::mutx_gain_list {
	    set ch   [lindex $i 0]
	    set gain [lindex $i 1]
	    if {$gain == -1} {
	    	puts $gpc_handle "set label 'mutx gain (ch=$ch) = error' at graph 0.35, graph $y_position"
	    } else {
	    	set gain [expr int($gain * 100)]
	    	puts $gpc_handle "set label 'mutx gain (ch=$ch) = $gain%' at graph 0.35, graph $y_position"
	    }
	    set y_position [expr $y_position - 0.05]
	}
    }

    # Cut xtics down to fit on graph xwidth 1.9.
    #UTF::Message INFO "" "NewXLabel-before=$NewXLabel"
    set NewXLabel [gnuplot_trim_xtics $NewXLabel 60 $xcol]
    #UTF::Message INFO "" "NewXLabel-after=$NewXLabel"

    # The calling routine may choose to give us a series of labels & values 
    # for use on X-axis and Y-axis. This allows for non-numeric categories to be
    # plotted, such as VHT rates: 1x0 ... 1x9 ... 3x9
    if {[info exists ::gnuplot_rvr_ylabel]} {
        puts $gpc_handle "set ytics($::gnuplot_rvr_ylabel)"
    }
    if {[info exists ::gnuplot_rvr_xlabel]} {
        puts $gpc_handle "set xtics($::gnuplot_rvr_xlabel)"
    } else {
        puts $gpc_handle "set xtics($NewXLabel)"
    }

    # Add line styles.
    for {set i 0} {$i <= $array_max} {incr i} {
        # Create one line style per column being plotted.
        set lt [lindex $array(lt) $i]
        set lw [lindex $array(lw) $i]
        set ps [lindex $array(ps) $i]
        set pt [lindex $array(pt) $i]
        # Style index must be > 0.
        # Parameters for style line must be in order shown in gnuplot reference manual. 
        puts $gpc_handle "set style line [expr $i + 1] lt $lt lw $lw pt $pt ps $ps"
    }
    if {$style == "image"} {
        # Additional items needed for image heatmap graphs.
        puts $gpc_handle "set pm3d map corners2color c3"
        if {$logscale == 1} {
            # z_min must not be 0 on a log scale.
            set z_low_lim $::gnuplot_rvr_logscale_low_limit
            if {$z_min < $z_low_lim} {
                set z_min $z_low_lim
            }
            puts $gpc_handle "set log cb"
            puts $gpc_handle "set cbrange \[$z_min:$z_max\]"
        } elseif {$z_max < 5} {
            set z_min 0 ;# for backwards compatibility
            puts $gpc_handle "set cbrange \[$z_min:$z_max\]"
        } else {
            # normalize to 100%
            set z_max 100
            puts $gpc_handle "set cbrange \[$z_min:$z_max\]"
        }
        puts $gpc_handle "set palette defined  $::gnuplot_rvr_palette"
        puts $gpc_handle "set cblabel '$cbtitle'"
        puts $gpc_handle "set yrange \[-0.5:$y_max\]"
        puts $gpc_handle "splot $plot_cmd" ;# splot shows grid against image.
    } else {
        puts $gpc_handle "plot $plot_cmd" ;# plot works with errorlines, etc
    }
    puts $gpc_handle "quit"
    close $gpc_handle

    # Generate the png file for Throughput
    catch {file delete $png_file}
    set catch_resp [catch {exec $::UTF::Gnuplot "$gpc_file"} catch_msg]
    UTF::Message INFO "" "gnuplot_rvr_lines: catch_resp=$catch_resp catch_msg=$catch_msg"

    # Filter out the gnuplot warnings.
    if {$catch_resp != 0 && ![string match -nocase *warning* $catch_msg]\
        && ![string match -nocase "*font*" $catch_msg]} {
        error "gnuplot_rvr_lines ERROR: gnuplot: $catch_msg"
    }

    # Did we get an output file? Gnuplot has been known to generate a null
    # file, which is why we check for errors (excluding warnings) above. 
    if {![file exists "$png_file"]} {
        error "gnuplot_rvr_lines ERROR: $png_file file not created, $catch_msg"
    }

    # Take out the trash.
    for {set i 0} {$i <= $array_max} {incr i} {
        set file [lindex $array(file) $i]
        catch {file delete $file}
    }
    catch {file delete $gpc_file}

    # Return the .png file.
    UTF::Message INFO "" "gnuplot_rvr_lines: created $png_file"
    return "$png_file"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::gnuplot_rvr_test]]
    # Test routine for gnuplot_rvr_lines
}

proc UTF::gnuplot_rvr_test { } {

    # Produce lines graphs
    for {set i 0} {$i < 255} {incr i 8} {
        set parm_list ""
        for {set j 0} {$j < 8} {incr j} {
            set k [expr $i + $j]
            append parm_list " linespoints a 3 $k 1 $k 1"
        }
        puts "i=$i parm_list=$parm_list"
        UTF::gnuplot_rvr_lines ~/rvr1.csv "o$i" "t" "d" "y" "a" $parm_list
    }

    # Produce color histogram.
    set parm_list ""
    append parm_list " image a 13 1 1 1 1"
    set i 300
    puts "i=$i parm_list=$parm_list"
    UTF::gnuplot_rvr_lines ~/rvr1.csv "o$i" "t" "d" "y" "a" $parm_list
    incr i
    UTF::gnuplot_rvr_lines ~/rvr1.csv "o$i" "t" "d" "y" "a" $parm_list xcol 5
}

#====================================================================
proc UTF::gnuplot_trim_xtics {list max xcol} {

    # Routine to compact xtics.
    set orig_list $list
    set list [split $list ,]
    set cnt [llength $list]
    set result ""
    if {$cnt <= $max || $xcol > 0} {
        # xcol>0 means there may be non-numeric data for X-tics.
        # For the time being, use them all.
        return $orig_list
    }

    # First step to remove adjacent items with same attenuation value.
    set prev_attn ""
    foreach pair $list {
        set attn [lindex $pair 0]
        set xval [lindex $pair 1]
        if {$attn != $prev_attn} {
            lappend result $pair
            set prev_attn $attn
        } else {
            # puts "gnuplot_trim_xtics dropping adjacent: $pair"
        }
    }

    # Second step is to keep every Nth xtic to compress the list length down
    # to the user specified max.
    set cnt [llength $result]
    if {$max <= 0} {
        set max 1 ;# prevent divide by 0 error
    }
    set ratio [expr double($cnt) / $max]
    # puts "ratio=$ratio"
    set result2 ""
    if {$ratio > 1.0} {
        # More compression is needed.
        # Round ratio up to nearest integer.
        set ratio [expr int($ratio)]
        incr ratio
        # puts "ratio=$ratio"
        set i 0
        foreach pair $result {
            # Keep every "ratio" pair.
            if {[expr $i % $ratio] == 0} {
                # puts "keep i=$i $pair"
                lappend result2 $pair
            } else {
                # puts "drop i=$i $pair"
            }
            incr i
        }

    } else {
        # No further comression needed
        set result2 $result
    }

    # Create comma separated result. Make sure last xtic doesnt have a trailing comma.
    # Gnuplot chokes if it does!
    set cnt [llength $result2]
    set result2 [join $result2 ,]
    regsub {,$} $result2 "" result2
    # puts "gnuplot_trim_xtics cnt: $cnt result2: $result2"
    return $result2
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::hping_send] [arg src_sta] [arg src_port] [arg dest_ip]
    # [arg dest_port] [arg protocol] [arg pkt_count] [arg pkt_interval]
    # [arg pkt_data_length] [arg data_filepath] [arg expected_response]]

    # Sends UDP|TCP|ICMP packet(s) from the [arg src_sta] to the [arg dest_ip]
    # containing the data in [arg data_filepath].[para] 

    # [arg src_sta] the name of STA object that will send the packets.
    # [arg src_port] the source UDP/TCP port number. When sending multiple
    # packets, this routine will increment the src_port number for each
    # packet to make them unique. Not used for icmp.
    # [arg dest_ip] the destination IP address.
    # [arg dest_port] the destination UDP/TCP port number. Not used for icmp.
    # [arg protocol] the transport protocol to be used, tcp|udp|icmp.
    # [arg pkt_count] how many packets to send.
    # [arg pkt_interval] time interval between packets.
    # [arg pkt_data_length] byte count length of the packet data payload.
    # [arg data_filepath] the full file path for the data that will be send.
    # This file is expected be on the [arg src_sta] host machine. Set to null
    # if you want hping to generate its own data strings.
    # [arg expected_response] 0=packets are expected back from the dest_ip,
    # 1=no response is expected from the dest_ip, null=dont check the response,
    # default is null.[para]

    # Installation of hping is fully automated for Linux objects. The Windows
    # installation requires a human to click on the GUI installation, so it
    # is left as a manual process. The required files are located in:
    # /projects/hnd_archives/win/UTF. See the hping_README.txt for more details.
}

proc UTF::hping_send {src_sta src_port dest_ip dest_port protocol pkt_count\
    pkt_interval pkt_data_length data_filepath {expected_response ""}} {
    UTF::Message INFO "" "hping_send: src_sta=$src_sta\
        src_port=$src_port dest_ip=$dest_ip dest_port=$dest_port\
        protocol=$protocol pkt_count=$pkt_count pkt_interval=$pkt_interval\
        pkt_data_length=$pkt_data_length data_filepath=$data_filepath\
        expected_response=$expected_response"

    # Map protocol parameter into format hping will accept.
    set protocol [string tolower $protocol]
    set protocol [string trim $protocol]
    if {$protocol == "tcp"} {
        set transport "" ;# tcp is hping default
    } elseif {$protocol == "icmp"} {
        set transport  "--icmp"
    } else {
        set transport "--udp"
    }

    # hping prefers to know exactly which interface to use.
    # If the device is null, usually the case for Windows, get
    # the IP address to resolve which interface to use. 
    set device [$src_sta cget -device]
    if {$device == ""} {
       set device [$src_sta ipaddr]
    }

    # Please note that hping only throws an error if it recieves 0 packets
    # in response to what it sent. If hping sends 5 packets and receives 3
    # response packets, hping declares success. This behavior is bypassed
    # below by calling hping in a loop, sending 1 packet at a time.

    # Send hping packets & check for errors. There is a bug in hping
    # when sending -c N packets, and pkt_data_length doesnt exactly 
    # match the contents of the data_filepath. Testing has shown that 
    # every second packet will have random garbage in the packet data
    # payload. The workaround is to always send 1 packet per call to
    # hping, and call hping N times in a loop.
    set expected_response [string trim $expected_response]
    for {set i 1} {$i <= $pkt_count} {incr i} {
        # The options passed to hping depend on the proctol and the 
        # presence of a user supplied data file.
        # NB: We MUST specify "-c 1" or hping keeps sending packets forever
        # and never returns!
        set data_filepath [string trim $data_filepath]
        if {$protocol == "icmp"} {
            if {$data_filepath == ""} {
                set catch_resp [catch {$src_sta rexec hping $dest_ip -I $device\
                    $transport -d $pkt_data_length -c 1} catch_msg]
            } else {
                set catch_resp [catch {$src_sta rexec hping $dest_ip -I $device\
                    $transport -d $pkt_data_length -c 1\
                    --file $data_filepath} catch_msg]
            }
        } else {
            if {$data_filepath == ""} {
                set catch_resp [catch {$src_sta rexec hping $dest_ip -I $device\
                    $transport -s $src_port -p $dest_port -d $pkt_data_length -c 1\
                    } catch_msg]
            } else {
                set catch_resp [catch {$src_sta rexec hping $dest_ip -I $device\
                    $transport -s $src_port -p $dest_port -d $pkt_data_length -c 1\
                    --file $data_filepath} catch_msg]
            }
        }
        UTF::Message INFO "" "hping_send: packet i=$i catch_resp=$catch_resp"
        if {$expected_response != "" && $expected_response != $catch_resp} {
            error "hping_send ERROR: i=$i expected_response=$expected_response != catch_resp=$catch_resp, catch_msg=$catch_msg"
        }

        # When looking through packet traces in ethereal, it really helps
        # if each packet is unique. This can be achieved by incrementing the
        # src_port number. This also ensures that the receiving device
        # will perceive each packet as belonging to a different socket.
        incr src_port
        UTF::Sleep $pkt_interval "$::localhost"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::init_device] [arg object] [arg cmd_list]]

    # Runs the commands in [arg cmd_list] on [arg object].
    # Uses standard mapping of %S to object name. Returns null.
}

proc UTF::init_device {object cmd_list} {

    # Get host name.
    set name [UTF::get_name $object]
    UTF::Message INFO $name "\ninit_device $object \n"

    # If OEM AP, return.
    if {[UTF::is_oem_ap $object]} {
        return
    }

    # Run commands
    foreach cmd $cmd_list {
        # Use standard mapping of %S to object name.
        set cmd [string map [list %S $object] $cmd]

        # Run cmd, log errors.
        #UTF::Message INFO $name $cmd
	if {[catch $cmd ret_msg]} {
            UTF::Message WARN $name $ret_msg
	}
    }
    return
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::is_ap] [arg AP]]

    # Returns 1 if [arg AP] is an AP, 0 otherwise.
    # Throws an error if [arg AP] is not defined in the config file.
}

proc UTF::is_ap { AP } {

    # Get AP type, check if its in the AP list.
    set ap_type [UTF::check_host_type $AP]
    if {[lsearch -exact $::ap_type_list $ap_type] >= 0} {
       set ap 1 ;# AP
    } else {
       set ap 0 
    }
    UTF::Message LOG $AP "is_ap=$ap ap_type=$ap_type"
    return $ap
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::is_oem_ap] [arg AP]]

    # Returns 1 if [arg AP] is an OEM AP, 0 otherwise. 
    # Throws an error if [arg AP] is not defined in the config file.[para]

    # The issue with OEM AP is that we dont have any wl command or control
    # over the device.
}

proc UTF::is_oem_ap { AP } {

    # Get AP type, check if its in the OEM AP list.
    set ap_type [UTF::check_host_type $AP]
    if {[lsearch -exact $::ap_oem_list $ap_type] >= 0} {
       set oem_ap 1 ;# OEM AP
    } else {
       set oem_ap 0
    }
    UTF::Message LOG $AP "is_oem_ap=$oem_ap ap_type=$ap_type"
    return $oem_ap
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::is_softap] [arg AP] [arg noerror=0]]

    # Returns 1 if [arg AP] is actually a regular STA with option -ap=1.
    # Returns 0 if [arg AP] is a Real AP, regardless of option -ap value.
    # Throws an error if [arg AP] is not defined in the config file, or
    # is not an AP or is not a STA object, meaning named in an
    # objects -sta { .... } list. Will return -1 if [arg noerror] is set
    # to 1, instead of throwing error.
}

proc UTF::is_softap { AP {noerror 0}} {

    # Check AP is defined in config file and is a STA object.
    set catch_resp [catch {UTF::check_sta_type $AP *} catch_msg]
    if {$catch_resp != 0} {
        if {$noerror == "1"} {
            UTF::Message INFO "$AP" "is_softap: $catch_msg"
            return -1
        } else {
            error "is_softap ERROR: $catch_msg"
        }
    }

    # Get AP attributes.  
    set is_ap  [UTF::is_ap $AP]
    set is_sta [UTF::is_sta $AP]
    set ap_opt [$AP cget -ap]

    # Both Real AP & SoftAP can have -ap=1
    set soft_ap -1 ;# invalid result
    if {$is_ap == "1"} {
        # This is an AP, regardless of -ap value
        set soft_ap 0 ;# RealAP
    }
    if {$is_sta == "1" && $ap_opt == "1"} {
        set soft_ap 1 ;# SoftAP
    }

    # Log & return results
    UTF::Message INFO "$AP" "is_softap: soft_ap=$soft_ap is_ap=$is_ap is_sta=$is_sta ap_opt=$ap_opt"
    if {$soft_ap < 0 && $noerror == "0"} {
        error "is_softap ERROR: $AP not Real AP or SoftAP"
    } else {
        return $soft_ap
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::is_sta] [arg STA]]

    # Returns 1 if [arg STA] is a STA, 0 otherwise.
    # Throws an error if [arg STA] is not defined in the config file.
}

proc UTF::is_sta { STA } {

    # Get STA type, check if its in the STA list.
    set sta_type [UTF::check_host_type $STA]
    if {[lsearch -exact $::sta_type_list $sta_type] >= 0} {
       set sta 1 ;# STA
    } else {
       set sta 0 
    }
    UTF::Message INFO $STA "is_sta=$sta sta_type=$sta_type"
    return $sta
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::join_oem_ap_retry] [arg OEM_SSID] [arg STA]
    # [opt chanspec=null] [opt tries=3] [opt security=open]]

    # Makes a connection from [arg STA] to AP advertizing [arg OEM_SSID].
    # If [arg chanspec] is not null, uses [arg chanspec] as part of the 
    # join request. Some OEM AP broadcast the same ssid on both bands,
    # so [arg chanspec] can direct the join to the desired band.
    # Tries [arg tries] times before giving up. [arg security] open is 
    # supported.
}

proc UTF::join_oem_ap_retry {OEM_SSID STA {chanspec ""} {tries 3} {security open}} {

    # Try multiple times to join OEM_SSID
    set name [UTF::get_name $STA]
    set chanspec [string trim  $chanspec]
    for {set i 1} {$i <= $tries} {incr i} {
        UTF::Message INFO $name "join_oem_ap_retry: $OEM_SSID $STA CH$chanspec $security Try: $i"

        # Setup STA for security.
        catch {$STA wl radio on}
        catch {$STA wl apsta}
        catch {$STA wl ap 0}
        catch {$STA wl up}
        catch {$STA wl disassoc}
        if {$security == "open"} {
            catch {$STA wl wsec 0}
            catch {$STA wl wpa_auth 0}
            catch {$STA wl sup_wpa 0}
            if {$chanspec == ""} {
                catch {$STA wl join "$OEM_SSID" imode bss amode open}
            } else {
                catch {$STA wl join "$OEM_SSID" imode bss amode open --chanspecs=$chanspec}
            }

        } elseif {$security == "aespsk2"} {
            # While this algorithm will allow you to join with aespsk2, setting keys doesnt
            # always work. DHCP worked once, but never again. There are many things
            # to coordinate with the AP. Currently not automated for OEM AP.
            error "join_oem_ap_retry ERROR: security=$security not supported!"
            catch {$STA wl wsec 7}
            catch {$STA wl wpa_auth 0}
            catch {$STA wl sup_wpa 1}
            catch {$STA wl set_pmk {"I8A_zmUYDoVDnZFHT2N3Ki_k7zYDBweyt2cAX5zhjOoH5orLZ9QAvpMYaQ"}}
            catch {$STA wl join $OEM_SSID imode bss amode wpa2psk}

        } else {
            error "join_oem_ap_retry ERROR: security=$security not supported!"
        }

        # Wait after join, then check assoc.
        UTF::Sleep 10
        set catch_resp [catch {$STA wl assoc} catch_msg]
        if {![regexp -nocase {not\s+associated} $catch_msg]} {
            UTF::Message INFO $name "join_oem_ap_retry: $OEM_SSID $STA CH$chanspec $security Try: $i OK"
            break
        } else {
            UTF::Message ERROR $name "join_oem_ap_retry: $OEM_SSID $STA CH$chanspec $security Try: $i failed"
        }

        # Wait a bit and try again
        if {$i == $tries} {
            break
        }
        UTF::Sleep 10
    }

    # Check keys
    if {$security != "open"} {
        catch {$STA wl keys}
    }

    # Get IP address
    set ip [$STA cget -ipaddr]
    $STA ifconfig $ip

    # Always check rssi & chanspec
    set rssi ""
    catch {set rssi [$STA wl rssi]}
    set chanspec ""
    catch {set chanspec [$STA wl chanspec]}

    # Return status depends on which attempt succeeded or not.
    if {$i == 1 && ![regexp -nocase {not\s+associated} $catch_msg]} {
        # Succeeded the first time.
        return "RSSI: $rssi CH: $chanspec"
    } elseif {![regexp -nocase {not\s+associated} $catch_msg]} {
        # Succeeded on retry, shows as an error to get
        # peoples attention to potential unreliablity.
        error "join_oem_ap_retry succeeded on try: $i RSSI: $rssi CH: $chanspec"
    } else {
        # Tried N times, still failed, dont test this device any more.
        UTF::remove_test_item $STA
        error "join_oem_ap_retry ERROR: join $OEM_SSID $STA CH$chanspec $security failed, tried $tries times!"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::list_delta] [arg list1] [arg list2]]

    # Returns list of items that are in list2 but not in list1.
}

proc UTF::list_delta {list1 list2} {
    # Find items in list2 that are not in list1
    set delta ""
    foreach item $list2 { 
        set i [lsearch -exact $list1 $item]
        if {$i < 0} {
            # item is NOT in list1, save it.
            lappend delta $item
        }
    }

    # Return delta items.
    set delta [string trim $delta]
    return $delta
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::load_rtr_retry] [arg AP] [arg args]]

    # Makes use of optional power capabilities while loading a driver
    # on an [arg AP]. On failure, will retry the load again if optional power
    # is available. On second failure, additional diagnostic commands are
    # tried before the third and final load is tried. [arg args] are passed
    # to method load. If the [arg args] include -norestore, the AP defaults
    # will not be restored.[para]

    # Returns driver version info string.
}

proc UTF::load_rtr_retry {AP args} {
    set name [UTF::get_name $AP]
    UTF::Message INFO $name "\n\n\nUTF::load_rtr_retry: $AP $args\n\n\n"
    set norestore 0
    if {[regexp {\-norestore} $args]} {
        set norestore 1
        regsub {\-norestore} $args "" args
        puts "norestore=$norestore args=$args"
    }

    # If possible, power cycle the router.
    set power_data [$AP cget -power]
    set power_data [string trim $power_data]
    if {$power_data != "" } {
        UTF::Sleep 1 $name "Be nice to power controller" ;# WTI issue
        $AP power off
        UTF::Sleep 3 $name "Power-off delay before 1st load attempt"
        $AP power on
        UTF::Sleep 30 $name "Wait for router to load"
    }

    # Force the lan peer, if any, to update its dhcp.
    set lan_peer [string trim [$AP cget -lanpeer]]
    if {$lan_peer != ""} {
        UTF::Message INFO $name "\n\n\nUTF::load_rtr_retry: Updating DHCP\n\n\n"
        set catch_resp [catch {$AP lan ifconfig [$lan_peer cget -ipaddr]} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR $name "UTF::load_rtr_retry: DHCP failed for\
                lan_peer=$lan_peer, using static IP"
            set catch_resp [catch {$AP lan ifconfig 192.168.1.50} catch_msg]
            if {$catch_resp != 0} {
                UTF::Message ERROR $name "UTF::load_rtr_retry: Static IP failed"
            }
        } elseif {[catch {$lan_peer add_networks $AP} ret]} {
	    UTF::Message ERROR $name "UTF::load_rtr_retry: add_networks failed: $ret"
	}
    }

    # First try loading the router OS.
    UTF::Message INFO $name "\n\n\nUTF::load_rtr_retry: Try #1\n\n\n"
    set catch_resp1 [catch {eval UTF::enh_load $AP $args} catch_msg1]
    UTF::Message INFO $name "UTF::load_rtr_retry: Try #1\
        catch_resp1=$catch_resp1 catch_msg1=$catch_msg1"

    # If load failed and we have power control, try loading again.
    set catch_resp2 ""
    set catch_msg2 ""
    set catch_resp3 ""
    set catch_msg3 ""
    if {$catch_resp1 != 0 && $power_data != "" } {
        UTF::Message INFO $name "\n\n\nUTF::load_rtr_retry: Try #2\n\n\n"
        UTF::Sleep 1 $name "Be nice to power controller" ;# WTI issue
        $AP power off
        UTF::Sleep 3 $name "Power-off delay before 2nd load attempt"
        $AP power on
        UTF::Sleep 30
        set catch_resp2 [catch {eval UTF::enh_load $AP $args} catch_msg2]
        UTF::Message INFO $name "UTF::load_rtr_retry: Try #2\
            catch_resp2=$catch_resp2 catch_msg2=$catch_msg2"
        if {$catch_resp2 != 0} {
            # Start of 3rd try
            UTF::Message INFO $name "\n\n\nUTF::load_rtr_retry: Additional diagnostics\n\n\n"
            UTF::Sleep 1 $name "Be nice to power controller" ;# WTI issue
            $AP power off
            UTF::Sleep 3 $name "Power-off delay before 2nd load attempt"
            $AP power on
            UTF::Sleep 30

            # Try repeated wl ver commands. While the router really has its version
            # info in nvram variable os_version, we dont check that as UTF tends to
            # cache that info. We want a commmand that will go all the way out to the
            # router no matter what.
            for {set i 1} {$i <= 15} {incr i} {
                UTF::Message INFO $name "\n\n\nUTF::load_rtr_retry: Diagnostic wl ver command #$i\n\n\n"
                set catch_resp [catch {$AP wl ver} catch_msg]
                if {$catch_resp == 0} {
                    break
                } 
                UTF::Message WARN $name "$catch_msg"
                UTF::Sleep 2
            }

            # If wl ver commands all failed, try restore defaults.
            if {$catch_resp != 0} {
                UTF::Message INFO $name "\n\n\nUTF::load_rtr_retry: Diagnostic restore defaults\n\n\n"
                set catch_resp [catch {$AP restore_defaults} catch_msg]
                if {$catch_resp == 0} {
                    # Restore succeeded, no need to do it again at end.
                    set norestore 1
                } else {
                    UTF::Message WARN $name "UTF::load_rtr_retry: Restore defaults catch_msg: $catch_msg"
                }
            }

            # Load for 3rd time
            UTF::Message INFO $name "\n\n\nUTF::load_rtr_retry: Try #3\n\n\n"
            set catch_resp3 [catch {eval UTF::enh_load $AP $args} catch_msg3]
            UTF::Message INFO $name "UTF::load_rtr_retry: Try #3\
                catch_resp3=$catch_resp3 catch_msg3=$catch_msg3"
            if {$catch_resp3 != 0} {
                # No more testing for this router
                UTF::remove_test_item $AP
                error "Try #1 FAIL: $catch_msg1 Try #2 FAIL: $catch_msg2 Try #3 FAIL: $catch_msg3"
            }
        }
    }

    # Optionally restore the router defaults.
    set catch_resp4 0
    set catch_msg4 ""
    if {!$norestore} {
        UTF::Message INFO $name "\n\n\nRestoring defaults $AP\n\n\n"
        set catch_resp4 [catch {$AP restore_defaults} catch_msg4]
        if {$catch_resp4 != 0} {
            set catch_msg4 "restore_default: $catch_msg4"
            UTF::Message ERROR $name "$catch_msg4"
        }
    }

    # Try repeated wl ver commands. While the router really has its version
    # info in nvram variable os_version, we dont check that as UTF tends to
    # cache that info. We want a commmand that will go all the way out to the
    # router no matter what.
    for {set i 1} {$i <= 15} {incr i} {
        UTF::Message INFO $name "\n\n\nUTF::load_rtr_retry: Final diagnostic wl ver command #$i\n\n\n"
        set catch_resp [catch {$AP wl ver} catch_msg]
        if {$catch_resp == 0} {
            break
        } 
        UTF::Message WARN $name "$catch_msg"
        UTF::Sleep 2
    }

    # Return router version info
    if {$catch_resp1 == 0} {
        if {$catch_resp4 == 0} {
            return "$catch_msg1"
        } else {
            error "Try #1 PASS: $catch_msg1 $catch_msg4"
        }
    } elseif {$catch_resp2 == 0} {
        error "Try #1 FAIL: $catch_msg1 Try #2 PASS: $catch_msg2 $catch_msg4"
    } else {
        error "Try #1 FAIL: $catch_msg1 Try #2 FAIL: $catch_msg2 Try #3 PASS: $catch_msg3 $catch_msg4"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::load_sta_retry] [arg STA] [arg args]]

    # Makes use of optional power_sta & device_reset capabilities
    # while loading a driver on the [arg STA]. On failure, will power cycle /
    # reset the [arg STA] if optional power_sta or device_reset are available.
    # If second [arg STA] load attempt fails, host is power cycled and a third
    # and final [arg STA] load attempt is made. If the final load attempt fails,
    # the host is power cycled again to leave it in a clean state for
    # whatever test runs next on the testrig.
    # [arg args] are passed to the method load.[para]

    # Returns driver version info string.
}

proc UTF::load_sta_retry {STA args} {
    # If necessary, look up -sta name.
    set name [UTF::get_name $STA]
    set object_type [UTF::check_host_type $STA]
    set catch_resp [catch {$STA info type} catch_msg]
    if {$catch_msg != "::UTF::STA"} {
        set STA [lindex [$STA cget -sta] 0]
    }
    UTF::Message INFO $name "UTF::load_sta_retry: $object_type $STA $args"

    # NB: Most devices need to do unload or PC will often lock up when doing load.
    # PC can still lock up even if unload is done.
    # Linux devices with BCM9EMBADPT can hang if we dont unload driver.
    if {$object_type != "WinBT"} {
        UTF::Message INFO $name "\n\n\nUTF::load_sta_retry: Unload driver\n\n\n"
        set catch_resp [catch {$STA unload} catch_msg]
        if {$catch_resp != 0} {
           UTF::Message ERROR $name $catch_msg
        }
    }

    # Some devices may have external power control available.
    # For WLAN devices, this usually needs to be called explicitly.
    # For BT devices, the load method automatically looks for power_sta.
    UTF::Message INFO $name "\n\n\nUTF::load_sta_retry: Try #1\n\n\n"
    set power_sta [$STA cget -power_sta]
    set power_sta [string trim $power_sta]
    if {$power_sta != ""} {
        UTF::Sleep 1 $name "Be nice to power controller" ;# WTI issue
        $STA power_sta off
        UTF::Sleep 3 $name "UTF::load_sta_retry Power-off delay\
            before 1st load attempt"
        $STA power_sta on
    }

    # Some devices may have a device reset relay available.
    # For WLAN devices, this needs to be called explicitly.
    # For BT devices, the load method automatically looks for device reset.
    set dev_reset [$STA cget -device_reset]
    set dev_reset [string trim $dev_reset]
    if {$dev_reset != "" } {
        $STA device_reset
    }

    # First try loading the STA.
    set catch_resp1 [catch {eval UTF::enh_load $STA $args} catch_msg1]
    UTF::Message INFO $name "UTF::load_sta_retry: Try #1\
        catch_resp1=$catch_resp1 catch_msg1=$catch_msg1"
    if {$catch_resp1 == 0 } {
        # STA loaded cleanly, return version info
        return "load_sta_retry $STA Try #1 PASS: $catch_msg1"
    } elseif {[regexp -nocase "removed.*rebooting" $catch_msg1] ||\
        [regexp -nocase "devcon.*completed" $catch_msg1] ||\
        [regexp -nocase "recovered" $catch_msg1]} {
        # STA recovered from loading issues on the first try
        error "load_sta_retry $STA Try #1 PASS: $catch_msg1"
    }
    set catch_msg1 [string range $catch_msg1 0 99]

    # Diagnostic code. NB: It is possible to run WLAN & BT on samep PC!
    if {$object_type != "WinBT"} {
        UTF::Message INFO $name "diagnostics"
        set catch_resp [catch {$STA wl ver} catch_msg]
        if {$catch_resp == 0} {
            error "load_sta_retry $STA Try #1 PASS: recovered $catch_msg1"
        }
        UTF::Message INFO $name "catch_resp=$catch_resp catch_msg=$catch_msg"
    }
    if {$object_type == "Cygwin" || $object_type == "WinDHD"} {
        set catch_resp [catch {$STA devcon enable} catch_msg]
        UTF::Message INFO $name "catch_resp=$catch_resp catch_msg=$catch_msg"
        set catch_resp [catch {$STA wl ver} catch_msg]
        UTF::Message INFO $name "catch_resp=$catch_resp catch_msg=$catch_msg"
        if {$catch_resp == 0} {
            error "load_sta_retry $STA Try #1 PASS: recovered via extra devcon enable"
        }
    }

    # We keep trying to load the STA.
    UTF::Message INFO $name "\n\n\nUTF::load_sta_retry: Try #2\n\n\n"
    # WAR PR90847 - Cygwin & WinDHD devices need to do unload
    # or PC will often lock up when doing load. Can still lock
    # up even if unload is done.
    if {$object_type == "Cygwin" || $object_type == "WinDHD"} {
        set catch_resp [catch {$STA unload} catch_msg]
        if {$catch_resp != 0} {
           UTF::Message ERROR $name $catch_msg
        }
    }
    if {$power_sta != "" } {
        UTF::Sleep 1 $name "Be nice to power controller" ;# WTI issue
        $STA power_sta off
        UTF::Sleep 3 $name "UTF::load_sta_retry Power-off delay\
            before 2nd load attempt"
        $STA power_sta on
    }
    if {$dev_reset != "" } {
        $STA device_reset
    }

    # Second try loading the STA.
    set catch_resp2 [catch {eval UTF::enh_load $STA $args} catch_msg2]
    UTF::Message INFO $name "UTF::load_sta_retry: Try #2\
        catch_resp2=$catch_resp2 catch_msg2=$catch_msg2"
    if {$catch_resp2 == 0||\
        [regexp -nocase "removed.*rebooting" $catch_msg2] ||\
        [regexp -nocase "devcon.*completed" $catch_msg2] ||\
        [regexp -nocase "recovered" $catch_msg2]} {
        # We loaded on the second try. Record the
        # error from the first try. We will keep on
        # testing this device.
        error "load_sta_retry $STA Try #1 FAIL: $catch_msg1 Try #2 PASS: $catch_msg2"
    }
    set catch_msg2 [string range $catch_msg2 0 99]

    # Diagnostic code. NB: It is possible to run WLAN & BT on samep PC!
    if {$object_type != "WinBT"} {
        UTF::Message INFO $name "diagnostics"
        set catch_resp [catch {$STA wl ver} catch_msg]
        if {$catch_resp == 0} {
            error "load_sta_retry $STA Try #1 FAIL: $catch_msg1 Try #2 PASS: recovered $catch_msg2"
        }
        UTF::Message INFO $name "catch_resp=$catch_resp catch_msg=$catch_msg"
    }
    if {$object_type == "Cygwin" || $object_type == "WinDHD"} {
        set catch_resp [catch {$STA devcon enable} catch_msg]
        UTF::Message INFO $name "catch_resp=$catch_resp catch_msg=$catch_msg"
        set catch_resp [catch {$STA wl ver} catch_msg]
        UTF::Message INFO $name "catch_resp=$catch_resp catch_msg=$catch_msg"
        if {$catch_resp == 0} {
            error "load_sta_retry $STA Try #1 FAIL: $catch_msg1 Try #2 PASS: recovered via extra devcon enable"
        }
    }

    # Now power cycle the host.
    # NB: power cycle success is reported as OK, not PASS, to avoid
    # confusing scripts that parse this routines output.
    UTF::Message INFO $name "\n\n\nUTF::load_sta_retry: PowerCycle #1\n\n\n"
    set catch_resp3 [catch {$STA shutdown_reboot} catch_msg3]
    UTF::Message INFO $name "UTF::load_sta_retry: PowerCycle #1\
        catch_resp3=$catch_resp3 catch_msg3=$catch_msg3"
    if {$catch_resp3 != 0} {
        error "load_sta_retry $STA Try #1 FAIL: $catch_msg1 Try #2 FAIL: $catch_msg2 PowerCycle #1 FAIL: $catch_msg3"
    }
    set catch_msg3 [string range $catch_msg3 0 99]

    # We keep trying to load the STA.
    UTF::Message INFO $name "\n\n\nUTF::load_sta_retry: Try #3\n\n\n"
    # WAR PR90847 - Cygwin & WinDHD devices need to do unload
    # or PC will often lock up when doing load. Can still lock
    # up even if unload is done.
    if {$object_type == "Cygwin" || $object_type == "WinDHD"} {
        set catch_resp [catch {$STA unload} catch_msg]
        if {$catch_resp != 0} {
           UTF::Message ERROR $name $catch_msg
        }
    }
    if {$power_sta != "" } {
        UTF::Sleep 1 $name "Be nice to power controller" ;# WTI issue
        $STA power_sta off
        UTF::Sleep 3 $name "UTF::load_sta_retry Power-off delay\
            before 3rd load attempt"
        $STA power_sta on
    }
    if {$dev_reset != "" } {
        $STA device_reset
    }

    # Third/final try loading the STA.
    set catch_resp4 [catch {eval UTF::enh_load $STA $args} catch_msg4]
    UTF::Message INFO $name "UTF::load_sta_retry: Try #3\
        catch_resp4=$catch_resp4 catch_msg4=$catch_msg4"
    if {$catch_resp4 == 0||\
        [regexp -nocase "removed.*rebooting" $catch_msg4] ||\
        [regexp -nocase "devcon.*completed" $catch_msg4] ||\
        [regexp -nocase "recovered" $catch_msg4]} {
        # We loaded on the third try. Record all the
        # errors from the previous tries. We will keep on
        # testing this device.
        error "load_sta_retry $STA Try #1 FAIL: $catch_msg1 Try #2 FAIL: $catch_msg2 PowerCycle #1 OK: $catch_msg3 Try #3 PASS: $catch_msg4"
    }
    set catch_msg4 [string range $catch_msg4 0 99]

    # Diagnostic code. NB: It is possible to run WLAN & BT on samep PC!
    if {$object_type != "WinBT"} {
        UTF::Message INFO $name "diagnostics"
        set catch_resp [catch {$STA wl ver} catch_msg]
        if {$catch_resp == 0} {
            error "load_sta_retry $STA Try #1 FAIL: $catch_msg1 Try #2 FAIL: $catch_msg2 PowerCycle #1 OK: $catch_msg3 Try #3 PASS: recovered $catch_msg4"
        }
        UTF::Message INFO $name "catch_resp=$catch_resp catch_msg=$catch_msg"
    }
    if {$object_type == "Cygwin" || $object_type == "WinDHD"} {
        set catch_resp [catch {$STA devcon enable} catch_msg]
        UTF::Message INFO $name "catch_resp=$catch_resp catch_msg=$catch_msg"
        set catch_resp [catch {$STA wl ver} catch_msg]
        UTF::Message INFO $name "catch_resp=$catch_resp catch_msg=$catch_msg"
        if {$catch_resp == 0} {
            error "load_sta_retry $STA Try #1 FAIL: $catch_msg1 Try #2 FAIL: $catch_msg2 PowerCycle #1 OK: $catch_msg3 Try #3 PASS: recovered via extra devcon enable"
        }
    }

    # No more testing for this device.
    UTF::remove_test_item $STA

    # Clean up for next set of tests.
    UTF::Message INFO $name "\n\n\nUTF::load_sta_retry: PowerCycle #2\n\n\n"
    set catch_resp5 [catch {$STA shutdown_reboot} catch_msg5]
    UTF::Message INFO $name "UTF::load_sta_retry: PowerCycle #2\
        catch_resp5=$catch_resp5 catch_msg5=$catch_msg5"
    if {$catch_resp5 == 0} {
        error "load_sta_retry $STA Try #1 FAIL: $catch_msg1 Try #2 FAIL: $catch_msg2 PowerCycle #1 OK: $catch_msg3 Try #3 FAIL: $catch_msg4 PowerCycle #2 OK: $catch_msg5"
    } else {
        error "load_sta_retry $STA Try #1 FAIL: $catch_msg1 Try #2 FAIL: $catch_msg2 PowerCycle #1 OK: $catch_msg3 Try #3 FAIL: $catch_msg4 PowerCycle #2 FAIL: $catch_msg5"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::manage_dmp_files] [arg host]]

    # Renames MEMORY.DMP file as MEMORY_<date>_<time>_<pc name>.DMP
    # By default, keeps 20 .DMP files on disk, erasing older ones as 
    # needed. Optional config file variable [arg ::<host>_save_dmp_files]
    # controls how many .DMP files are kept on a given host, and can be
    # configured differently for each host.
}

proc UTF::manage_dmp_files {host} {

    # Check if config file has variable <host>_save_dmp_files.
    set var "::${host}_save_dmp_files"
    if {[info exists $var]} {
        set max_dmp_files [set $var]
    } else {
        set max_dmp_files 20
    }
    if {![regexp {^\d+$} $max_dmp_files]} {
        set max_dmp_files 20
    }
    # puts "max_dmp_files=$max_dmp_files"

    # Some users may not wish to keep .dmp files.
    if {$max_dmp_files == 0} {
        return
    }

    # Save the memory.dmp file as MEMORY_<date>_<time>_<host>.DMP
    set date [clock format [clock seconds] -format "%Y%m%d_%H%M%S"]
    # set host ""
    # catch {set host [$self cget -name]}
    # catch {set host [$self cget -host]}
    set path "/cygdrive/c/Windows"
    set dest "MEMORY_${date}_${host}.DMP"
    regsub -all {:} $dest "" dest
    set catch_resp [catch {$host rexec mv -f $path/MEMORY.DMP $path/$dest} catch_msg]

    # Get the list & count of .DMP & .dmp files from remote PC. 
    set dmp_file_list ""
    set catch_resp [catch {$host rexec -silent -quiet ls $path/*.DMP} catch_msg]
    if {$catch_resp == 0} {
        append  dmp_file_list " $catch_msg"
    }
    set catch_resp [catch {$host rexec -silent -quiet ls $path/*.dmp} catch_msg]
    if {$catch_resp == 0} {
        append  dmp_file_list " $catch_msg"
    }
    set dmp_file_list [lsort $dmp_file_list]
    set dmp_file_cnt [llength $dmp_file_list]
    if {$dmp_file_cnt <= $max_dmp_files} {
        return
    }

    # Delete excess .dmp files
    set delete_max [expr $dmp_file_cnt - $max_dmp_files]
    for {set i 0} {$i < $delete_max} {incr i} {
        set file [lindex $dmp_file_list $i]
        catch {$host rexec rm -f $file}
    }
    return
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::monitor_pids] [arg timeout_seconds] [arg object1]
    # [arg pid1] [arg description1] ... [arg objectN] [arg pidN] 
    # [arg descriptionN]]
    # Used to monitor one or more long running processes on different hosts.
    # [arg Timeout_seconds] specifies the maximum time processes are allowed
    # to run. Processes that exceed the maximum timeout_seconds are terminated.
    # [arg object1] is the object name of the first host to be monitored.
    # [arg pid1] is the integer process id of the process on the host
    # object1 to be monitored.
    # [arg description1] is the text string description of pid1.
    # [arg objectN], [arg pidN] & [arg descriptionN] are similar and allow for
    # multiple processes, possbibly of different host objects, to be monitored
    # all in parallel. [para]

    # Returns null if all processes terminate by themselves within the
    # allocated [arg timeout_seconds] period. Otherwise throws an error 
    # showing which processes exceeded the allocated [arg timeout_seconds].
}

proc UTF::monitor_pids {timeout_seconds args} {

    # Log the calling data
    UTF::Message INFO "" "UTF::monitor_pids\
        timeout_seconds=$timeout_seconds args=$args"

    # Check timeout_seconds is a positive integer
    if {![regexp {^\d+$} $timeout_seconds] || $timeout_seconds < 1} {
        error "UTF::monitor_pids timeout_seconds=$timeout_seconds\
           must be a positive integer!"
    }

    # Load calling args into local array, along with appropriate 
    # commands for monitoring & terminating processes.
    set array_max 0
    foreach {object pid description} $args {

        # Ignore null objects/pids
        set object [string trim $object]
        set pid [string trim $pid]
        if {$object == "" || $pid == ""} {
            continue
        } 

        # Check pid is integer
        if {![regexp {^\d+$} $pid]} {
            error "UTF::monitor_pids ERROR: pid=$pid is not integer,\
                object=$object description=$description"
        }

        # Save data in new row in monitor_array
        incr array_max
        set monitor_array($array_max,description) $description
        set monitor_array($array_max,object) $object
        set monitor_array($array_max,pid) $pid
    }

    # Check that there is really something to do.
    if {$array_max < 1} {
        UTF::Message INFO "" "UTF::monitor_pids: Nothing to do!"
        return
    }

    # Adjust timing parameters
    if {$timeout_seconds < 5} {
        set timeout_seconds 5
    }
    set wait_sec 3 ;# process poll cycle time in seconds
    if {$timeout_seconds < $wait_sec} {
        set wait_sec $timeout_seconds
    }

    # Loop to monitor PIDs every wait_sec seconds
    set pids_not_running_list ""
    set start_sec [clock seconds]
    set verbose 0 ;# for debug traces, set to 1
    set i 0
    while { 1 } {

        # Wait a bit
        incr i
        UTF::Sleep $wait_sec "$::localhost" 

        # Check each process that is believed to still be running.
        set still_running 0
        for {set j 1} {$j <= $array_max} {incr j} {

            # Skip over processes that are already known to have terminated.
            set object $monitor_array($j,object)
            set pid $monitor_array($j,pid)
            set description $monitor_array($j,description)
            if {$object == "" || $pid == ""} {
                continue
            }

            # Check if process is still running.
            set resp [UTF::check_pid $object $pid $verbose]
            # puts "check_pid object=$object pid=$pid verbose=$verbose resp=$resp"
            if {$resp == 0} {
                # pid has terminated by itself, remove from array
                lappend pids_not_running_list "$object $pid $description i=$i"
                UTF::Message INFO "" "UTF::monitor_pids: $object $pid $description terminated by itself i=$i"
                set monitor_array($j,object) ""
                set monitor_array($j,pid) ""
                set monitor_array($j,description) ""

            } else {
                # This pid is still running
                incr still_running
            }
        }

        # Have we timed out?
        set now_sec [clock seconds]
        set delta_sec [expr $now_sec - $start_sec]
        if {$delta_sec >= $timeout_seconds} {
            UTF::Message ERROR "" "UTF::monitor_pids: timed out waiting for $still_running pids, i=$i delta_sec=$delta_sec"
            break
        }

        # If no pids are still running, we are done, return.
        if {$still_running == 0} {
            UTF::Message INFO "" "UTF::monitor_pids: pids_not_running_list=$pids_not_running_list delta_sec=$delta_sec"
            return
        }
    }

    # We have timed out. Terminate the remaining pids.
    set error_list ""
    for {set j 1} {$j <= $array_max} {incr j} {

        # Skip over processes that are already known to have terminated.
        set object $monitor_array($j,object)
        set pid $monitor_array($j,pid)
        set description $monitor_array($j,description)
        if {$object == "" || $pid == ""} {
            continue
        }

        # Terminate the remaining pids
        UTF::Message INFO "" "UTF::monitor_pids: $object $pid $description being terminated"
        lappend error_list "$object $pid $description TIMED OUT"
        set catch_resp [catch {UTF::terminate_pid $object $pid $verbose} catch_msg]
        if {$catch_resp != 0} {
             lappend error_list "pid=$pid NOT terminated"
        }
    }

    # Throw a error with the details of what happened.
    error "UTF::monitor_pids pids_not_running_list=$pids_not_running_list\
        error_list=$error_list"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::open_connection_retry] [arg AP] [arg STA]
    # [opt tries=3] [opt security=open] [opt chanspec=null]]
    # Used to open a connection between [arg AP] and [arg STA].
    # If the connection fails, it will be retried. [opt tries]
    # controls how many total tries are made, default is 3.
    # [opt security] defaults to open. [opt chanspec] defaults
    # to null. [opt chanspec] is needed for use with SoftAP.[para]

    # Returns the connection RSSI & chanspec.
}

proc UTF::open_connection_retry {AP STA {tries 3} {security open} {chanspec ""}} { 
    set name [UTF::get_name $STA]
    UTF::Message INFO $name "UTF::open_connection_retry: $AP $STA tries=$tries security=$security chanspec=$chanspec"

    # Clean up tries parameter
    set tries [string trim $tries]
    if {![regexp {^\d+$} $tries]} {
        set tries 3
        UTF::Message WARN $name "UTF::open_connection_retry: Tries set to: $tries"
    }

    # There have been times when tests would have
    # passed if we had tried to open the connection
    # after the first failure. So we try multiple times.
    for {set j 1} {$j <= $tries} {incr j} {
        UTF::Message INFO $name "\nMake connection: $AP $STA Try: $j\n"

        if {$chanspec == ""} {  
            set catch_resp [catch {UTF::Test::ConnectAPSTA $AP $STA -security $security} catch_msg]
        } else {
            set catch_resp [catch {UTF::Test::ConnectAPSTA $AP $STA -security $security -chanspec $chanspec} catch_msg]
        }

        if {$catch_resp == 0} {
            UTF::Message INFO $name "\nConnection OK: $AP $STA Try: $j\n"
            break
        } else {
            UTF::Message ERROR $name "\nConnection failed: $AP $STA Try: $j: $catch_msg\n"
        }
    }

    # Always check status, rssi & channel
    catch {$STA wl status}
    set rssi ""
    catch {set rssi [$STA wl rssi]}
    set chanspec ""
    catch {set chanspec [$STA wl chanspec]}

    # Return status depends on which attempt succeeded or not.
    if {$j == 1 && $catch_resp == 0} {
        # Succeeded the first time.
        return "RSSI=$rssi CH=$chanspec"
    } elseif {$catch_resp == 0} {
        # Succeeded on retry, shows as an error to get peoples attention to potential unreliablity.
        error "(Connection succeeded on try: $j) RSSI=$rssi CH=$chanspec"
    } else {
        # Tried N times, still failed, dont test this device any more.
        UTF::remove_test_item $STA
        error "Connection failed after $tries tries!"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::open_wet_retry] [arg AP] [arg WET]
    # [opt tries=3] [opt security=open]]
    # Used to open a wet connection between [arg AP] and [arg WET].
    # If the connection fails, it will be retried. [opt tries]
    # controls how many total tries are made, default is 3.
    # [opt security] defaults to open.[para]

    # Assumes loaded builds can do WET functionality, and 
    # and have wlN_mode=wet.

    # Returns the connection RSSI & chanspec.
}

proc UTF::open_wet_retry {AP WET {tries 3} {security open}} { 

    # Setup
    set name [UTF::get_name $WET]
    UTF::Message INFO $name "UTF::open_wet_retry: $AP $WET tries=$tries security=$security"
    set catch_resp ""
    set ch ""
    set rssi ""
    set ssid [$AP wl ssid]

    # WET link should come up by itself.
    for {set j 1} {$j <= $tries} {incr j} {

        # Wait to see if link comes up by itself.
        UTF::Message INFO $name "\n\n\nOpen WET $AP $WET Try: $j\n\n\n"  
        UTF::Sleep 30 $name
        set status [$WET wl status]
        if {[regexp -nocase {not associated} $status]} {
            # Try to bring the link up.
            catch {$WET wl disassoc}
            UTF::Sleep 2 $name
            catch {$WET wl join $ssid}
            continue

        } else {
            # Ping between devices
            set catch_resp [catch {$WET ping $AP} catch_msg]
            if {$catch_resp != 0} {
                # Ping failed, force a new connection
                catch {$WET wl disassoc}
                UTF::Sleep 2 $name
                catch {$WET wl join $ssid}
                continue
            }

            # Collect info
            if {![regexp -nocase {RSSI:\s+([\-\d]+)} $status - rssi]} {
                set rssi ""
            }
            set ch [$WET wl chanspec]
            break
        }
    }

    # Return results.
    if {$j == 1} {
        # Succeeded the first time.
        return "RSSI: $rssi CH: $ch"
    } elseif {$catch_resp == 0} {
        # Succeeded on retry, shows as an error to get
        # peoples attention to potential unreliablity.
        error "Open WET succeeded on try: $j RSSI: $rssi CH: $ch"
    } else {
        # Tried N times, still failed, dont test this
        # device any more.
        UTF::remove_test_item $WET
        error "Open WET failed, tried $tries times!"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::parse_rpopen_data] [arg data_list] [arg host]
    # [arg regular_expression]]
    # Used to parse the list data returned by collect_rpopen_data.
    # Case insensitive parsing is done.
    # [arg data_list] list formated with sets of timestamp, host name
    # and text strings. Typically there is only a single line of data in 
    # each list set.
    # [arg host] host name to be used in parsing process.
    # [arg regular_expression] regular expression used in parsing process.[para]

    # Returns the string that the host and the regular expression matched.
    # Throws an error if no match occurred.
} 

proc UTF::parse_rpopen_data { data_list host regular_expression } { 

    # Match the host name in the list set, then match
    # on the regular expression. Ignore timestamps.
    foreach item $data_list {
        # puts "item=$item"
        if {![regexp -nocase " $host " $item]} {
            continue
        }
        if {[regexp -nocase $regular_expression $item - result]} {
            return $result
        }
    }

    # No match ==> error!
    error "UTF::parse_rpopen_data ERROR: no match for host=$host\
        regular_expression=$regular_expression in data_list=$data_list"
}

#====================================================================

UTF::doc {
    # [call [cmd UTF::private_build_tag] [arg file]]

    # Returns the TAG used to check out a private build tree.  [arg
    # file] should be any checked-out file in the tree. Also supports
    # path from public build servers.
}

proc UTF::private_build_tag {file} {
    # Use explicit path for svn to avoid broken OSS tools
    set catch_resp [catch {set svn_info [exec $::UTF::projtools/linux/bin/svn info $file]} catch_msg]
    if {$catch_resp == 0} {
        UTF::Message INFO "" "private_build_tag: file=$file\nsvn_info=$svn_info"
    } else {
        UTF::Message ERROR "" "private_build_tag: file=$file\ncatch_msg=$catch_msg"
        set svn_info ""
    }

    # Look for SVN URL.
    if {[regexp -line {^URL: (.*)} $svn_info - url]} {
        # puts "url=$url"
        if {[regexp {wlansvn/proj/branches/([^/]+)} $url - branch]} {
            return $branch
        } elseif {[regexp {wlansvn/proj/trunk} $url]} {
            return NIGHTLY
        } else {
            error "Unexpected URL: $url"
        }

    # Look for public build server path
    # NB: path be hnd/swbuild or hnd_swbuild !
    } elseif {[regexp {^/projects/hnd.swbuild/build_[^/]+/([^/]+)} $file - branch]} {
        UTF::Message INFO "" "private_build_tag: Matched public build server: $branch"
        return $branch
    } else {
        error "Unexpected file: $file"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::process_cc_results] [arg result_code]
    # [arg html_string] [arg trend] [opt key=null]]

    # Routine used process the control chart results. [arg result_code]
    # & [arg html_string] are the raw results from controlchart.test.
    # If [arg trend] is 0, the results will PASS/FAIL as normal. If [arg trend] 
    # is 1, most results will be shown as PASS, with the actual results 
    # being collected in variable ::process_cc_results_list. This allows for 
    # additional post processing and subsequent PASS/FAIL determination. The [arg key]
    # & actual result are appended as a list to ::process_cc_results_list. The
    # calling routine needs to initialize ::process_cc_results_list as appropriate.[para]

    # Returns html string with thumbnail image & weblink. 
}

proc UTF::process_cc_results {result_code html_string trend {key ""}} {

    # For trend=0, PASS/FAIL as usual.
    if {$trend == 0} {
        if {$result_code == 0} {
            # Return shows up as PASS on the web page.
            return "$html_string"

        } else {
            # Throwing error shows up as a FAIL on the web page.
            error "$html_string"
        }
    }

    # For trend=1, save the real result in ::process_cc_results_list
    # and ususually return PASS. Start by getting the last few charactars of html_string.
    if {![regexp {(.{50})$} $html_string - end_str]} {
        set end_str ""
    }

    # Look for "result </a>" in end_str.
    # NB: rvr1.test may append "no beacons, skipped iperf" to end of html_string.
    set real_result ""
    set real_code 0
    if {[regexp -nocase {</a>} $end_str]} {
        # Found </a>, now look for OK, ZERO, WIDE, LOW, HIGH & NARROW.
        if {[regexp -nocase {(OK|ZERO|WIDE|LOW|HIGH|NARROW)} $end_str - real_result]} {
            # Got one of the standard controlchart results.
        } else {
            set real_result "OTHER"
        }

    } else {
        # We continue to fail on everything else, such as ping failed & no route to host.
        set real_result "OTHER"
        set real_code 1
    }

    # Log & save real result for post-processing.
    set real_result [string toupper $real_result]
    UTF::Message INFO "" "process_cc_results: trend=$trend end_str=$end_str real_result=$real_result"
    lappend ::process_cc_results_list [list $key $real_result]
    if {$real_code == 0} {
        return "$html_string"
    } else {
        error "$html_string"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::progressive_recovery] [arg STAS] [arg keepgoing]
    # [arg security] [opt startup=0] [opt fb=0] [opt ch=null] [opt softap=null]]
    # Looks at state information for each device in list [arg STAS]
    # and reloads the driver and/or reboots the host as necessary and
    # opens connection type [arg security]. Updates the device state
    # information. Uses ::rvr_ap_init & ::rvr_sta_init command lists
    # to setup devices.[para]

    # If recovery count is exceeded, stops the tests. If [arg keepgoing]
    # is 1, tests continue on no matter what. [arg startup] is needed for
    # case of ::recovery_max = 0. [arg fb] specifies frameburst setting.
    # [opt ch] the desired chanspec, is needed for recovery of SoftAP
    # devices.[para]

    # Returns no=calling script should keep going, yes=calling script
    # should stop the tests.
}

proc UTF::progressive_recovery {STAS keepgoing security {startup 0} {fb 0}\
    {ch ""} {oem_band ""} {stoponerror 0}} {

    # Check required variables are defined.
    if {![info exists ::recovery_max]} {
        set ::recovery_max 3
        UTF::Message INFO "" "progressive_recovery: recovery_max=$::recovery_max"
    }
    if {![info exists ::recovery_cnt]} {
        set ::recovery_cnt 0
        UTF::Message INFO "" "progressive_recovery: recovery_cnt=$::recovery_cnt"
    }

    # Some users may not want any recovery.
    if {$::recovery_max <= 0 && $startup == 0 && $stoponerror == 0} {
        UTF::Message INFO "" "progressive_recovery: recovery_max=$::recovery_max, no recovery will be done."
        return no
    }

    # See if any devices are not OK.
    set devices_not_ok ""
    foreach item $STAS {
        # Check state for this item
        set state_var "::${item}_state"
        if {![info exists $state_var]} {
            puts "progressive_recovery: $item NO state"
            lappend devices_not_ok $item
        } else {
            set state [set $state_var]
            puts "progressive_recovery: $item $state"
            if {$state != "OK"} {
                lappend devices_not_ok $item
            }
        }
    }
    set devices_not_ok [string trim $devices_not_ok]
    UTF::Message INFO "" "progressive_recovery: STAS=$STAS\
        keepgoing=$keepgoing recovery_max=$::recovery_max\
        recovery_cnt=$::recovery_cnt security=$security startup=$startup\
        fb=$fb devices_not_ok=$devices_not_ok ch=$ch oem_band=$oem_band\
        stoponerror=$stoponerror"

    # If devices are all OK, we are done.
    if {$devices_not_ok == "" } {
        return no
    }

    # Should we continue recovery attempts?
    # We allow recovery for case of test startup, regardless.
    if {$::recovery_cnt >= $::recovery_max && $startup == 0} {
        if {$keepgoing == "1"} {
            # User wants to keep going no matter what. Devices are left
            # in their current state in the hopes that the expected failures
            # will occur quickly and get the test over and done with.
            return no
        } else {
            # Return gracefully, so calling script can cleanup properly.
            UTF::Try "progressive_recovery stopping tests" {
                UTF::Message ERROR "" "progressive_recovery: stopping tests,\
                    recovery_cnt=$::recovery_cnt GE recovery_max=$::recovery_max,\
                    devices_not_ok=$devices_not_ok"
                error "devices_not_ok=$devices_not_ok"
            }
            return yes
        }
    }

    # If requested, check each item in STAS is responding. If necessary, reload 
    # driver and/or reboot the host PC. Process all devices so we have info
    # needed at end to setup connections.
    set done_recovery no
    set ap_list ""
    set sta_list ""
    foreach item $STAS {

        # Check item exists.
        set catch_resp [catch {$item info type} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR "" "progressive_recovery: $item $catch_msg"
            continue
        }

        # Reload & reboot cmds depend on the object type. Process all devices 
        # so we have info needed at end to setup connections.
        set is_ap     [UTF::is_ap $item]
        set is_oem_ap [UTF::is_oem_ap $item]
        set is_softap [UTF::is_softap $item 1] ;# 1 means noerror
        if {$is_ap == "1"} {
            # Real BRCM AP & EOM AP
            set reload_cmd "UTF::reload_rtr $item"
            set reboot_cmd "" ;# reload = reboot for router
            lappend ap_list $item
            if {[info exists ::rvr_ap_init] && $is_oem_ap == "0"} {
                set init_cmds $::rvr_ap_init
            } else {
                set init_cmds ""
            }

        } elseif {$is_softap == "1"} {
            # For SoftAP, use STA cmds, but show in ap_list.
            # puts "\n\nSoftAP: $item\n\n"
            set reload_cmd "UTF::reload_sta $item"
            set reboot_cmd "$item shutdown_reboot"
            lappend ap_list $item
            if {[info exists ::rvr_ap_init]} {
                set init_cmds $::rvr_ap_init
            } else {
                set init_cmds ""
            }

        } else {
            # Regular STA
            # puts "\n\nSTA: $item\n\n"
            set reload_cmd "UTF::reload_sta $item"
            set reboot_cmd "$item shutdown_reboot"
            lappend sta_list $item
            if {[info exists ::rvr_sta_init]} {
                set init_cmds $::rvr_sta_init
            } else {
                set init_cmds ""
            }
        }
        # puts "item=$item is_ap=$is_ap is_oem_ap=$is_oem_ap is_soft_ap=$is_softap reload_cmd=$reload_cmd reboot_cmd=$reboot_cmd"

        # Do we have a state variable for this item?
        set state_var "::${item}_state"
        if {![info exists $state_var]} {
            set $state_var Check
        }
        set state [set $state_var]
        UTF::Message INFO "" "progressive_recovery: item=$item state=$state"

        # Is the state a valid value?
        if {$state != "Check" && $state != "DriverDown" && $state != "HostDown" &&\
            $state != "OK"} {
            UTF::Message ERROR "" "progressive_recovery: item=$item invalid state=$state, set to Check"
            set $state_var Check
            set state Check
        }

        # If device is OK, we are done for that device. 
        if {$state == "OK"} {
            continue
        }

        # If necessary, check the device & update state. 
        if {$state == "Check"} {
            if {$is_oem_ap == "1"} {
                set $state_var OK ;# here's hoping...
            } else {
                set catch_resp [catch {set resp [$item wl ver]} catch_msg]
                if {$catch_resp == 0} {
                    set $state_var OK
                } else {
                    set $state_var DriverDown
                    UTF::Message ERROR "" "progressive_recovery: item=$item catch_msg=$catch_msg"
                }
                UTF::save_device_state $item $catch_msg
            }
            set state [set $state_var]
            UTF::Message INFO "" "progressive_recovery: item=$item state=$state"

            # If device is OK, we are done for that device. 
            if {$state == "OK"} {
                continue
            }
        }

        # If necessary, reload the driver.
        if {$state == "DriverDown"} {
            UTF::Try "progressive_recovery reload $item driver" {
                set done_recovery yes
                set catch_resp1 [catch {$reload_cmd} catch_msg1]
                set catch_msg2 ""
                if {$is_oem_ap == "1"} {
                    set $state_var OK ;# if power cycle OEM AP didnt work, we are toast!
                } else {
                    set catch_resp2 [catch {set resp [$item wl ver]} catch_msg2]
                    set $state_var OK
                    UTF::save_device_state $item $catch_msg2 
                    UTF::init_device $item $init_cmds
                    if {$fb} {
                        catch {$item wl frameburst 1}
                    }
                }
                # Always report recovery as an error so its on the main web page.
                set state [set $state_var]
                UTF::Message INFO "" "progressive_recovery: item=$item state=$state"
                error "$catch_msg1 $catch_msg2"
            }

            # If device is OK, we are done for that device.
            set state [set $state_var]
            if {$state == "OK"} {
                continue
            }
        }

        # Finally we reboot the host PC and reload the driver.
        UTF::Try "progressive_recovery shutdown/reboot $item host, reload driver" {
            set done_recovery yes
            set catch_resp1 [catch {$reboot_cmdr} catch_msg1]
            set catch_resp2 [catch {$reload_cmd} catch_msg2]
            set catch_resp3 ""
            if {$is_oem_ap == "1"} {
                set $state_var OK ;# if power cycle OEM AP didnt work, we are toast!
            } else {
                set catch_resp3 [catch {set resp [$item wl ver]} catch_msg3]
                set $state_var OK
                UTF::save_device_state $item $catch_msg3
                UTF::init_device $item $init_cmds
                if {$fb} {
                    catch {$item wl frameburst 1}
                }
            }
            # Always report recovery as an error so its on the main web page.
            set state [set $state_var]
            UTF::Message INFO "" "progressive_recovery: item=$item state=$state"
            error "$catch_msg1 $catch_msg2 $catch_msg3"
        }
    }

    # If we have an AP and STA that are OK, try to connect them.
    set ap_list [string trim $ap_list]
    set sta_list [string trim $sta_list]
    set ch [string trim $ch]
    UTF::Message INFO "" "progressive_recovery: startup=$startup ap_list=$ap_list sta_list=$sta_list security=$security ch=$ch"
    if {$startup == "0" && $ap_list != "" && $sta_list != ""} {
        set AP [lindex $ap_list 0] ;# use first AP only
        set is_oem_ap [UTF::is_oem_ap $AP]
        set state_var "::${AP}_state"
        set state [set $state_var]
        if {$state == "OK"} {

            # AP is OK, try to connect each STA.
            foreach STA $sta_list {
                set state_var "::${STA}_state"
                set state [set $state_var]
                if {$state == "OK"} {
                    UTF::Try "progressive_recovery setup $security connection $AP $STA" {
                        set done_recovery yes
                        if {$is_oem_ap == "1"} {
                            set oem_ssid [UTF::get_oem_ap_ssid $AP]
                            set resp [UTF::find_oem_ap_chanspec $oem_ssid $oem_band $STA]
                            set ch [lindex $resp 0]
                            set oem_ssid [lrange $resp 1 end] ;# may have multiple tokens
                            set catch_resp [catch {UTF::join_oem_ap_retry $oem_ssid $STA $ch 1 $security} catch_msg]

                        } elseif {$ch == ""} {
                            set catch_resp [catch {UTF::Test::ConnectAPSTA $AP $STA -security $security} catch_msg]
                        } else {
                            set catch_resp [catch {UTF::Test::ConnectAPSTA $AP $STA -security $security -chanspec $ch} catch_msg]
                        }
                        # Always report recovery as an error, so its on the main web page.
                        error "$catch_msg"
                    }
                }
            }
        }
    }

    # If we took recovery action, kick the counter. We deliberately count
    # multiple recovery actions in one call to this routine as 1 recovery.
    if {$done_recovery == "yes" && $startup== "0"} {
        incr ::recovery_cnt
    }

    # There may still be a device that is not OK. We continue onwards
    # allowing for subsequent attempts to recover the device.
    return no
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::read_csv_data] [arg pathname] [arg args]]

    # Opens specified [arg pathname] file and extracts data from
    # the CSV formatted columns specified by [arg args]. Returns
    # list of lists of data, one list per line in the file.
}

proc UTF::read_csv_data {pathname args} {

    # Open the specified data file for reading.
    set catch_resp [catch {set in [open $pathname r]} catch_msg]
    if {$catch_resp != 0} {
       error "read_csv_data ERROR: could not open $pathname catch_msg=$catch_msg"
    }
    # puts "in=$in"
    
    # Read specified CSV columns in args.
    set i 0
    set result ""
    while {![eof $in]} {
        # Parse CSV formatted data, skip non-numeric header lines.
        incr i
        set line [gets $in]
        set fields [split $line ","]
        set str [lindex $fields 0]
        if {![regexp {^[\d\.]+$} $str]} {
            continue
        }
        # puts "line $i: $line"

        # Collect requested columns.
        set col_list ""
        foreach col $args {
            set temp [lindex $fields $col]
            set temp [string trim $temp]
            lappend col_list $temp
        }
        # puts "i=$i col_list=$col_list"
        lappend result $col_list
    }

    # Log & return result.
    UTF::Message INFO "" "read_csv_data: $pathname args: $args result: $result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::reload_rtr] [arg AP]]

    # Makes use of optional power capabilities while reloading a
    # driver on an [arg AP]. Returns driver version info string.
}

proc UTF::reload_rtr {AP} {
    set name [UTF::get_name $AP]
    UTF::Message INFO $name "UTF::reload_rtr: $AP"

    # Power cycle the router if possible.
    set power_data [$AP cget -power]
    set power_data [string trim $power_data]
    if {$power_data != "" } {
        $AP power off
        UTF::Sleep 3 $name "UTF::reload_rtr Power-off delay\
            before reload"
        $AP power on
    }

    # Watch for OEM AP.
    if {[is_oem_ap $AP]} {
        UTF::Sleep 15 ;# Hopefully OEM AP recovers
        return OEM

    } else {
        # Now do soft reboot of the BRCM AP.
        $AP reboot
        return [$AP nvram get os_version]
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::reload_sta] [arg STA]]

    # Makes use of optional power_sta & device_reset capabilities
    # while reloading a driver on a [arg STA]. If inital reload fails,
    # reboots PC and does second reload.[para]

    # Returns driver version info string if available.
}

proc UTF::reload_sta {STA} {

    # If necessary, look up -sta name.
    set name [UTF::get_name $STA]
    set object_type [UTF::check_host_type $STA]
    set catch_resp [catch {$STA info type} catch_msg]
    if {$catch_msg != "::UTF::STA"} {
        set STA [lindex [$STA cget -sta] 0]
    }
    UTF::Message INFO $name "\n\nUTF::reload_sta: $object_type $STA\n\n"

    # NB: Most devices need to do unload or PC will often lock up when doing load.
    # PC can still lock up even if unload is done.
    # Linux devices with BCM9EMBADPT can hang if we dont unload driver.
    if {$object_type != "WinBT"} {
        UTF::Message INFO $name "\n\n\nUTF::reload_sta: unload driver\n\n\n"
        set catch_resp [catch {$STA unload} catch_msg]
        if {$catch_resp != 0} {
           UTF::Message ERROR $name $catch_msg
        }
    }

    # Some devices may have external power control available.
    # For WLAN devices, this needs to be called explicitly.
    # For BT devices, the load method automatically looks for power_sta.
    UTF::Message INFO $name "\n\n\nUTF::reload_sta: reload driver\n\n\n"
    set power_sta [$STA cget -power_sta]
    set power_sta [string trim $power_sta]
    if {$power_sta != "" } {
        $STA power_sta off
        UTF::Sleep 3 $name "UTF::reload_sta Power-off delay\
            before reload"
        $STA power_sta on
    }
        
    # Some devices may have a device reset relay available.
    # For WLAN devices, this needs to be called explicitly.
    # For BT devices, the load method automatically looks for device reset.
    set dev_reset [$STA cget -device_reset]
    set dev_reset [string trim $dev_reset]
    if {$dev_reset != "" } {
       $STA device_reset
    }

    # Now reload the STA. On failure, reboot PC.
    set catch_resp [catch {$STA reload} resp]
    if {$catch_resp != 0} {
        UTF::Message ERROR $STA "\n\nUTF::reload_sta: Reload failed, rebooting PC\n\n"
        $STA shutdown_reboot
        UTF::Message INFO $STA "\n\nUTF::reload_sta: Try second reload\n\n"
        set resp [$STA reload]
    }

    # Get version info.
    if {$object_type != "WinBT"} {
        set ver  [$STA wl ver]
    } else {
        # BT doesnt have wl command!
        set ver ""
    }
    regsub -all {\n} $ver " " ver
    if {$catch_resp == 0} {
        return "$resp $ver"
    } else {
        error "UTF::reload_sta recovered by rebooting PC $resp $ver"
    }    
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::remove_duplicate_objects] [arg object_list]]
    # Used to remove duplicate objects from a list. Objects in list
    # can be host devices or STAs.[para]

    # Returns the updated object_list.
} 

proc UTF::remove_duplicate_objects { object_list } { 

    # Check for duplicate objects based on host name and ip_addr.
    set ip_list "";# unique list of IP addresses
    set name_list "" ;# unique list of host names (not objects)
    set result "" ;# resulting list of unique objects
    UTF::Message INFO "" "UTF::remove_duplicate_objects: object_list=$object_list"

    foreach item $object_list {
        # Ignore blank items
        set item [string trim $item]
        if {$item == ""} {
            continue
        }

        # Filter out invalid items.
        set catch_resp [catch {set x [$item cget -name]} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR "" "$catch_msg"
            continue
        }

        # In the case of a STA, first get the host object. Always get the name.
        set catch_resp [catch {set host_name [$item cget -host]} catch_msg]
        if {$catch_resp == 0} {
            set host_object [$host_name cget -name]
        } else {
            set host_object [$item cget -name]
        } 
   
        # If -lan_ip option is blank, use the object name instead.
        set lan_ip [$host_object cget -lan_ip]
        set lan_ip [string trim $lan_ip]
        if {$lan_ip == ""} {
            set lan_ip [$host_object cget -name]
            set lan_ip [string trim $lan_ip]
        }
        set lan_ip [string tolower $lan_ip]
        UTF::Message INFO "" "UTF::remove_duplicate_objects: item=$item host_object=$host_object lan_ip=$lan_ip"

        # Get host & ip addr info based on the object -lan_ip data.
        if {[regexp {^\d+\.\d+\.\d+\.\d+$} $lan_ip]} {
            # lan_ip is a valid IP address
            set host_ip $lan_ip
            set host_name [UTF::get_host_name $lan_ip]
        } else {
            # lan_ip is a host name string.
            set host_ip [UTF::get_ip_addr $lan_ip]
            set host_name $lan_ip
        }

        # Make sure both host_name & host_ip are not blank.
        if {$host_name == "" && $host_ip == ""} {
            set msg "ERROR: item=$item has null host_name & host_ip!"
            UTF::Message ERROR "" "UTF::remove_duplicate_objects: $msg"
            continue
        }

        # Some users have multiple objects that point to the same
        # physical host name/IP address. We need to filter out these duplicate objects.
        if {($host_name != "" && [lsearch -exact $name_list $host_name] >= 0) ||\
            ($host_ip   != "" && [lsearch -exact $ip_list $host_ip] >= 0)} {
            UTF::Message WARN "" "UTF::remove_duplicate_objects: Skipping duplicate object $item\
                host_name=$host_name host_ip=$host_ip name_list=$name_list ip_list=$ip_list"
            continue
        }

        # Save the current name & IP, add item to result.
        if {$host_name != ""} {
            lappend name_list $host_name
        }
        if {$host_ip != ""} {
           lappend ip_list $host_ip
        }
        lappend result $item
    }

    # Log & return results
    UTF::Message INFO "" "UTF::remove_duplicate_objects: result=$result"
    return "$result"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::remove_item_exact] [arg list] [arg item]]
    # Used to do an exact removal of ALL occurances of [arg item]
    # from [arg list]. This avoids pattern matching issues where
    # you could mangle token xyzabc when removing token xyz. Returns
    # the updated [arg list].
} 

proc UTF::remove_item_exact {list item} {

    # Look for ALL exact matches of item in list
    while { 1 } {
        set i [lsearch -exact $list $item]
        # puts "list=$list item=$item i=$i"
        if {$i >=0} {
            # We found another item, update list again.
            set list [lreplace $list $i $i]
        } else {
            # We removed ALL occurances
            break
        }
    }

    # Return updated list
    return $list
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::remove_test_item] [arg device]]
    # Used to remove a host [arg device] and its STAs from the lists of items
    # to be tested. If [arg device] is type STA, then the corresponding 
    # host will be removed. 
} 

proc UTF::remove_test_item {device} {

    # Is the device a STA?
    set type [$device info type]
    # puts "UTF::remove_test_item device=$device type=$type"
    if {$type =="::UTF::STA"} {
        # Get host & name for this STA
        set host [$device cget -host]
        set name [$host cget -name]
        set sta_list $device
    } else {
        # Get list of STA for this host device
        set sta_list [$device cget -sta]
        set name [$device cget -name]
    }

    # Host object names may have leading "::".
    regsub {^::} $device "" device
    UTF::Message INFO "" "remove_test_item: device=$device type=$type name=$name sta_list=$sta_list"

    # Remove device from high level host device lists
    # If there are objects with similar names, eg tst2 & tst2a, we need
    # to be able to delete tst2 without mangling tst2a, so we use -exact
    # option on lsearch.
    # puts "remove_test_item before endpoint_device_list=$::endpoint_device_list\
        router_device_list=$::router_device_list"
    if {![info exists ::endpoint_device_list]} {
        UTF::Message WARN "" "UTF::remove_test_item: variable ::endpoint_device_list not defined!"
        return
    }
    set i [lsearch -exact $::endpoint_device_list $name]
    if {$i >= 0} {
        set ::endpoint_device_list [lreplace $::endpoint_device_list $i $i]
    }
    set ::endpoint_device_list [string trim $::endpoint_device_list]
    set i [lsearch -exact $::router_device_list $name]
    if {$i >= 0} {
        set ::router_device_list [lreplace $::router_device_list $i $i]
    }
    set ::router_device_list [string trim $::router_device_list]
    UTF::Message INFO "" "remove_test_item: after\
        endpoint_device_list=$::endpoint_device_list\
        router_device_list=$::router_device_list"

    # Remove all STAs from the STA lists.
    foreach {sta x} $sta_list {
        set i [lsearch -exact $::endpoint_sta_list $sta]
        if {$i >= 0} {
            set ::endpoint_sta_list [lreplace $::endpoint_sta_list $i $i]
        }
        set i [lsearch -exact $::endpoint_sta_bt_list $sta]
        if {$i >= 0} {
            set ::endpoint_sta_bt_list [lreplace $::endpoint_sta_bt_list $i $i]
        }
        set i [lsearch -exact $::router_sta_list $sta]
        if {$i >= 0} {
            set ::router_sta_list [lreplace $::router_sta_list $i $i]
        }
    }
    set ::endpoint_sta_list [string trim $::endpoint_sta_list]
    set ::endpoint_sta_bt_list [string trim $::endpoint_sta_bt_list]
    set ::router_sta_list [string trim $::router_sta_list]
    UTF::Message INFO "" "remove_test_item: endpoint_sta_list=$::endpoint_sta_list\
        endpoint_sta_bt_list=$::endpoint_sta_bt_list router_sta_list=$::router_sta_list"

    # Finally, turn off messages from the device.
    catch {$device deinit}
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::reset_all_bt_devices]]
    # Resets all BlueTooth devices in your testrig. This ensures that any
    # residual BlueTooth ACL/SCO/ESCO connections are removed. If this is not 
    # done, you can end up with low WLAN throughput, as the Coex devices will
    # allocate 50% of the antenna usage for an active BlueTooth
    # connection, even if there is no actual traffic on the BlueTooth
    # connection. In order to get a consistent maximum WLAN throughput,
    # you need to reset all the BlueTooth devices before running WLAN
    # tests. Otherwise you may experience random, difficult to reproduce
    # issues, where the WLAN traffic is only 50% of what you expected.
}

proc UTF::reset_all_bt_devices { } {

    # Look for WinBT devices. No need to include clones. We are trying
    # to reset the hardware. No need to reset the same hardware once per clone.
    set bt_list "[UTF::get_names_values UTF::WinBT sta]"
    set bt_list [lsort $bt_list]
    if {$bt_list == ""} {
        UTF::Message INFO "" "\n\n\nreset_all_bt_devices: no BlueTooth devices found.\n\n\n"
        return
    }
    UTF::Message INFO "" "\n\n\nreset_all_bt_devices: bt_list=$bt_list\n\n\n"

    # Reset BT device power and hardware reset, if available.
    # Setup object options and corresponding commands to run.
    set opt_cmd_list [list -power_sta "power_sta cycle" -device_reset device_reset]
    set errors 0
    foreach BT $bt_list {
        foreach {opt cmd} $opt_cmd_list {
            set val [$BT cget $opt]
            set val [string trim $val]
            if {$val != ""} {
                set catch_resp [catch {$BT $cmd} catch_msg]
                if {$catch_resp != 0} {
                    incr errors
                    UTF::Message ERROR "$BT" "$catch_msg"
                }
            }
        }
    }

    # Did we get any errors?
    if {$errors == 0} {
        return
    } else {
        error "ERROR: reset_all_bt_devices got $errors errors"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::restart_iperf] [arg tput] [arg args]]
    # If [arg tput] is 0, restart iperf to cleanup any zombie
    # processes that may be left from RvR tests on STA list [arg args].
}

proc UTF::restart_iperf {tput args} {

    # Do only when tput is 0.
    if {$tput != 0} {
        return
    }
    
    # Iperf restart sequence depends on host OS.
    foreach item $args {
        # Some people dont have iperf daemon, just a single use instance.
        if {![$item cget -iperfdaemon] ||
	    [info exists UTF::TcpReadStats]} {
            UTF::Message INFO "$item" "restart_iperf does not use iperf daemon"
            continue
        }

        if {[$item hostis DHD Linux]} {
            # Linux OS
            catch {$item service iperf restart}
        } elseif {[$item hostis Cygwin WinDHD]} {
            # Windows OS
            catch {$item IPerfService restart}
        } elseif {[$item hostis MacOS]} {
            # MAC OS
            $item rexec -x launchctl stop net.nlanr.iperf
        } else {
            $item warn "Don't know how to restart iperf on [$item hostis] yet!"
        }
    }
    return
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::rvr_symmetry_test] [arg file] [arg col_mean]
    # [arg col_min] [arg col_max] [arg col_ctl_low] [arg col_ctl_upp]]
    # Reads data from .CSV [arg file] and reformats data for symmetry
    # test. Columns to use are specified by the other parameters. Returns
    # the name of the new file with reformatted data.
}

proc UTF::rvr_symmetry_test {file col_mean col_min col_max col_ctl_low col_ctl_upp} {

    # Open input file.
    set catch_resp [catch {set in [open $file r]} catch_msg]
    if {$catch_resp != 0} {
        error "rvr_symmetry_test ERROR: could not open $file catch_msg=$catch_msg"
    }

    # Check column args are numeric.
    foreach arg "$col_mean $col_min $col_max $col_ctl_low $col_ctl_upp" {
        set arg [string trim $arg]
        if {![regexp {^[\d\-]+$} $arg]} {
            error "rvr_symmetry_test ERROR: invalid column paramter: $arg"
        }
    }

    # Open output file.
    set out_file "${file}.sym"
    set catch_resp [catch {set out [open $out_file w]} catch_msg]
    if {$catch_resp != 0} {
        error "rvr_symmetry_test ERROR: could not open $out_file catch_msg=$catch_msg"
    }

    # Add output file header lines
    puts $out "RvR Symmetry Test,"
    puts $out "loss, rampdown mean, min, max, ctl low, ctl upp, rampup mean, min, max, ctl low, ctl upp, symm errors,"

    # Read input file, save data in stats_array. Keep rampdown data separate
    # from rampup data.
    set direction down
    set line_cnt 0
    set max_loss 0
    set max_tput 0
    set previous_loss ""
    while {![eof $in]} {

        # Read next line
        incr line_cnt
        set line ""
        set catch_resp [catch {gets $in line} catch_msg]
        if {$catch_resp != 0} {
            error "rvr_symmetry_test ERROR: reading input in=$in line_cnt=$line_cnt catch_msg=$catch_msg"
        }
        set line [string trim $line]

        # Get desired fields from line, skip non-numeric headers.
        set fields [split $line ","]
        set loss [lindex $fields 0]
        set loss [string trim $loss]
        if {$loss == "" || ![regexp {^\d+$} $loss]} {
            continue
        }
        if {$loss > $max_loss} {
            set max_loss $loss
        }

        # Have we changed from rampdown to rampup data?
        if {$direction == "down" && $loss <= $previous_loss} {
            # puts "\n\nloss=$loss changing direction to up\n\n" 
            set direction up
        }
        set previous_loss $loss

        # Save data for column parameters GE 0.
        set mean [lindex $fields $col_mean]
        set stats_array($loss,$direction,mean) $mean
        if {$mean > $max_tput} {
            set max_tput $mean
        }
        set min ""
        if {$col_min >= 0} {
            set min [lindex $fields $col_min]
            set stats_array($loss,$direction,min) $min
            if {$min > $max_tput} {
                set max_tput $min
            }
        }
        set max ""
        if {$col_max >= 0} {
            set max [lindex $fields $col_max]
            set stats_array($loss,$direction,max) $max
            if {$max > $max_tput} {
                set max_tput $max
            }
        }
        set ctl_low ""
        if {$col_ctl_low >= 0} {
            set ctl_low [lindex $fields $col_ctl_low]
            set stats_array($loss,$direction,ctl_low) $ctl_low
        }
        set ctl_upp ""
        if {$col_ctl_upp >= 0} {
            set ctl_upp [lindex $fields $col_ctl_upp]
            set stats_array($loss,$direction,ctl_upp) $ctl_upp
        }

        # puts "line_cnt=$line_cnt loss=$loss direction=$direction mean=$mean min=$min\
        #     max=$max ctl_low=$ctl_low ctl_upp=$ctl_upp"
    }

    # Use max_tput to create a scale factor for the symmetry error data.
    # Symmetry error data is shown discretely on the bottom 10% of graph.
    if {$max_tput < 40} {
        set max_tput 40 ;# based on legacy devices
    }
    set sym_error_scale [expr int($max_tput * 0.10)]

    # Load in the known control chart errors, if any.
    if {[info exists ::rvr_symmetry_errors]} {
        # ::rvr_symmetry_errors is a comma separated list of loss values.
        set symm_err_list [split $::rvr_symmetry_errors ","]
        foreach loss $symm_err_list {
            set loss [string trim $loss]
            set stats_array($loss,sym_err) $sym_error_scale
        }
    }

    # Dump stats_array to output file. Rampdown data & rampup data are shown
    # on the same line. Show in order of decreasing attenuation, like rampup.
    set out_cnt 0
    for {set i $max_loss} {0 <= $i} {incr i -1} {
        set line "$i,"
        set found_data no
        foreach dir "down up" {
            if {[info exists stats_array($i,$dir,mean)]} {
                set mean $stats_array($i,$dir,mean)
                set found_data yes
            } else {
                set mean "0"
            }
            if {[info exists stats_array($i,$dir,min)]} {
                set min $stats_array($i,$dir,min)
                set found_data yes
            } else {
                set min "0"
            }
            if {[info exists stats_array($i,$dir,max)]} {
                set max $stats_array($i,$dir,max)
                set found_data yes
            } else {
                set max "0"
            }
            if {[info exists stats_array($i,$dir,ctl_low)]} {
                set ctl_low $stats_array($i,$dir,ctl_low)
                set found_data yes
            } else {
                set ctl_low "0"
            }
            if {[info exists stats_array($i,$dir,ctl_upp)]} {
                set ctl_upp $stats_array($i,$dir,ctl_upp)
                set found_data yes
            } else {
                set ctl_upp "0"
            }
            append line " $mean, $min, $max, $ctl_low, $ctl_upp,"
        }

        # Look for symmetry error data, if any.
        if {[info exists stats_array($i,sym_err)]} {
            set sym_err $stats_array($i,sym_err)
            set found_data yes
        } else {
            set sym_err "0"
        }
        append line " $sym_err,"

        # Save only lines where we found some data.
        if {$found_data == "yes"} {
            puts $out "$line"
            incr out_cnt
        }
    }

    # Clean up, return out_file name.
    catch {close $in}
    catch {close $out}
    return $out_file
}


#====================================================================
UTF::doc {
    # Setup connection for one STA. 
    # (This routine is moved from Test/rvr1.test to here) 
}

proc UTF::setup_connection {AP sta chanspec security wet} {
    UTF::Try "Setup $security security connection for $sta" {
	# This is a performance optimization. If pings work in both directions, we are done.
	set retcode [catch {$sta ping $AP -c 2} ret]
	if {$retcode == 0} {
	    set retcode [catch {$AP ping $sta -c 2} ret]
	    if {$retcode == 0} {
	    	return "(skipped)"
	    } else {
		UTF::Message WARN "" $ret
	    }
	} else {
	    UTF::Message WARN "" $ret
	}

	# Allow 3 tries to open connection
	set tries 3

	# WET test is handled differently
	if {$wet} {
	    return [UTF::open_wet_retry $AP $sta $tries $security]
	}

	# Make association of STA to AP
	set retcode [catch {UTF::open_connection_retry $AP $sta $tries $security} ret]
	if {$retcode == 0} {
	    return $ret
	} else {
	    error $ret
	}

	# Allow for error when connection opens OK on the retry
	if {$retcode != 0 && ![regexp {succeeded\s+on\s+try:} $ret]} {
	    error "Halting tests, could not open connection!"
	}
    }

    # Tell AP to start collecting rssi info
    #catch {$AP wl rssi [$sta macaddr]}

    # Check chanspec and stop test if it fails
    UTF::check_chanspec "$AP $sta" $chanspec
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::save_device_state] [arg device] [arg msg]]
    # Looks at the last [arg msg] received by [arg device] for clues
    # there may be a problem. Sets variable ::[arg device]_state to DriverDown
    or HostDown as appropriate.[para]

    # The higher level script is expected to set the state to OK. This
    # routine only changes state to DriverDown or HostDown. It never sets it to OK!
}

proc UTF::save_device_state {device msg} {

    # Look for selected messages that indicate the driver is down.
    set var "::${device}_state"
    set name [UTF::get_name $device]
    if {[regexp -nocase {wl.*driver.*adapter.*not.*found} $msg] ||\
        [regexp -nocase {no.*wireless.*adapters.*were.*found} $msg] ||\
        [regexp -nocase {No.*Broadcom.*Wireless.*Adapter.*found} $msg] ||\
        [regexp -nocase {\s+Timeout$} $msg]} {
        set $var DriverDown
        UTF::Message ERROR $name "save_device_state: DriverDown device=$device msg=$msg"
        return
    }

    # Look for selected messages that indicate the host PC is down.
    if {[regexp -nocase {no.*route.*to.*host} $msg] ||\
        [regexp -nocase {child.*killed.*software.*termination} $msg]} {
        set $var HostDown
        UTF::Message ERROR $name "save_device_state HostDown device=$device msg=$msg"
        return
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::set_ap_nvram] [arg AP] [arg args]]
    # For [arg AP], sets nvram with the parameters in [arg args].
    # Restarts [arg AP] only if there was a change to nvram.
    # [arg args] should be in format: name=value [para] 

    # If name starts with wlN, the N will be swapped to the appropriate
    # 0 or 1 for the specific STA. If the name starts with a specific wl0 
    # or wl1, that name will be used as is.[para]

    # Returns 0=no nvram change & AP not restarted, 1=nvarm changed & AP was restarted.
    # Throws an error if appropriate.
}

proc UTF::set_ap_nvram {AP args} {
    UTF::Message INFO "" "set_ap_nvram: AP=$AP args=$args"

    # Get the correct wlN prefix for the AP STA.
    set wlname [$AP wlname]
    regsub -all {[a-zA-Z]} $wlname "" wlname
    # puts "wlname=$wlname"

    # Process each arg, should be in format: name=value
    set changed_nvram no
    foreach pair $args {

        # Extract data, check for nulls.
        set temp [split $pair "="]
        set name [lindex $temp 0]
        set name [string trim $name]
        set value [lindex $temp 1]
        set value [string trim $value]
        # puts "set_ap_nvram  pair=$pair name=$name value=$value"
        if {$name == "" || $value == ""} {
            error "set_ap_nvram ERROR: got null name=$name value=$value"
        }

        # Do we need to change the name?
        if {[regexp {^wlN_} $name]} {
            regsub {^wlN_} $name "wl${wlname}_" name
        }
        # puts "name=$name"

        # Get current nvram setting, if any.
        set nvram_value [$AP nvram get $name]
        set nvram_value [string trim $nvram_value]

        # If desired value is not current value, change it.
        if {$nvram_value != $value} {
            $AP nvram set $name=$value
            set changed_nvram yes
        }
    }

    # Restart AP only if we actually changed nvram.
    if {$changed_nvram == "no"} {
        return 0
    }
    $AP nvram commit
    $AP restart
    UTF::Sleep 5
    return 1
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::set_attn_grp] [arg step] [arg attngrp] [arg fixedattn]
    # Set the attenuator group [arg attngrp] to the specified [arg step] or [arg fixedattn].
    # Logs repeated attenuator errors. Will power cycle
    # the attenuator if appropriate. Stops the tests completely if the 
    # attenuator is totally unresponsive.[para]

    # Calling routine needs to initialize variables ::attn_error_cnt and
    # ::rvr_attn_max_errors as appropriate.[para]

    # If variable ::rvr_attn_power is defined, the the attenuator can
    # be power cycled after a hard error occurs.
}

proc UTF::set_attn_grp {step attngrp fixedattn} { 

    # Set the attenuator group. In case of error, one retry is done.
    for {set i 0} {$i <= 1} {incr i} {
	if {$fixedattn == ""} {
            set catch_resp [catch {$attngrp attn $step} catch_msg]
	} else {
	    UTF::Message INFO "" "User requested fixed attenuation $fixedattn on $attngrp"
	    set catch_resp [catch {$attngrp attn $fixedattn} catch_msg]
	}
	if {$catch_resp == 0} {
	    if {$fixedattn == ""} {
            	UTF::Message INFO "" "set_attn_grp: attngrp=$attngrp step=$step i=$i catch_resp=$catch_resp catch_msg=$catch_msg"
	    } else {
		UTF::Message INFO "" "set_attn_grp: attngrp=$attngrp step=$fixedattn i=$i catch_resp=$catch_resp catch_msg=$catch_msg"
	    }
	    return
        }

	if {$fixedattn == ""} {
            UTF::Message ERROR "" "set_attn_grp: attngrp=$attngrp step=$step i=$i catch_resp=$catch_resp catch_msg=$catch_msg"
	} else {
	    UTF::Message ERROR "" "set_attn_grp: attngrp=$attngrp step=$fixedattn i=$i catch_resp=$catch_resp catch_msg=$catch_msg"
	}
        UTF::Sleep 3
    }

    # If we have exceeded the allowable attenuator errors,
    # we halt the tests.
    if {[info exists ::attn_error_cnt]} {
        incr ::attn_error_cnt
    } else {
        set ::attn_error_cnt 1
    }
    if {![info exists ::rvr_attn_max_errors]} {
        set ::rvr_attn_max_errors 3
    }
    if {$::rvr_attn_max_errors > 0 && $::attn_error_cnt > $::rvr_attn_max_errors} {
        UTF::Try "set_attn_grp $attngrp $step" {
            error "$::attn_error_cnt attenuator errors, halting tests!"
        }

        # Throw error outside the UTF::Try block to halt tests.
        error "Too many attenuator errors, halting tests!"
    }

    # Take error recovery action for attenuator. Show results on main
    # web summary page.
    UTF::Try "set_attn_grp $attngrp step=$step" {

        # Duplicate the error message the brought us here.
        UTF::Message ERROR "" "set_attn_grp: step=$step catch_msg=$catch_msg"

        # Do we have power control for the attenuator?
        if {[info exists ::rvr_attn_power]} {
            # Reboot the attenuator.
            UTF::Message INFO "" "set_attn_grp: found ::rvr_attn_power=$::rvr_attn_power, doing power cycle"
            # Since the Aeroflex object does not yet have a proper power
            # control option, we create a dummy Linux object with the
            # desired power control data just for this occasion.
            set att_pwr_obj [UTF::Linux %AUTO% -power $::rvr_attn_power]
            # puts "att_pwr_obj=$att_pwr_obj"
            $att_pwr_obj power off
            UTF::Sleep 3
            $att_pwr_obj power on
            $att_pwr_obj destroy ;# we are done with the dummy object
            UTF::Sleep 60 "$::localhost" "let attenuator reboot"

            # Will the attenuator respond now?
            set catch_resp [catch {$attngrp attn $step} catch_msg]
            if {$catch_resp == 0} {
                set catch_msg "step=$step attenutor recovered OK"
                UTF::Message INFO "" "set_attn_grp: $catch_msg"
            }

        } else {
            UTF::Message INFO "" "set_attn_grp: no power control found for attenuator, no recovery possible."
        }

        # We always show an error, even if the attenuator was recovered.
        # This ensures the user sees that an issue occurred.
        error "$catch_msg" 
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::setup_build_info] [arg rtrtrx] [arg rtrtag] [arg rtrdate]
    # [arg statag] [arg stadate] [arg noapload] [arg nostaload] [arg summary]
    # [opt btver=null] [opt btdate=null] [opt external=null] [opt stabin=null]
    # [opt btcgr=null] [opt windhdbin=null] [opt stadhd=null] [opt useobjectsasis=null]
    # [opt dhdtag]]

    # Args are taken from the higher level script parsed command line unnamed
    # array $().[para]

    # Sets up build related info in the object -image, -tag and -date options.
    # The command line options will override any options specified in the
    # config file for a specific object. Default values are also added as
    # necessary. Builds based on these options are located and checked against
    # the desired values.[para]

    # If you want to manually tag your objects in your config file, and then have
    # the scripts use those tags as is, specify [opt useobjectsasis]=1. This is 
    # usefull for specifying specific builds/images and then letting the script do
    # the loading of those specific builds/images.[para]

    # The items [arg rtrtrx] [opt stabin] [opt btcgr] [opt windhdbin] [opt stadhd],
    # when specified, are expected to be full pathnames for a private build driver file.[para]

    # The build info is passed back in [arg summary] variable as an html formatted
    # table.[para]

    # If the [arg external]=1, attempts to locate the equivalent external 
    # builds instead of the internal builds.[para]

    # Returns an error list of devices & STAs for builds not found.[para]

    # Sets up global list: ::branch_list[para]

    # Assumes that UTF::setup_testbed_info has already been run.
}

proc UTF::setup_build_info {rtrtrx rtrtag rtrdate statag stadate noapload\
    nostaload summary {btver ""} {btdate ""} {external ""} {stabin ""} {btcgr ""}\
    {windhdbin ""} {stadhd ""} {useobjectsasis ""} {dhdtag ""}} {
    upvar $summary result

    # Sanity checks
    if {![info exists ::router_sta_list] || ![info exists ::endpoint_sta_list]} {
        error "UTF::setup_build_info ERROR: variables ::router_sta_list or\
            ::endpoint_sta_list not defined, please run\
            UTF::setup_testbed_info first!"
    }
    if {$noapload eq "" || ($noapload ne "0" && $noapload ne "1")} {
        set noapload 0
    }
    if {$nostaload eq "" || ($nostaload ne "0" && $nostaload ne "1")} {
        set nostaload 0
    }
    if {$useobjectsasis eq "" || ($useobjectsasis ne "0" && $useobjectsasis ne "1")} {
        set useobjectsasis 0
    }

    # To avoid single digit days accidentally matching double digit days,
    # we sometimes need to add a trailing period to the date. If last
    # character of date isnt period and we dont have the .N build number
    # suffix, then we add a period. This allows for someone specifying a
    # specific build number within a given day and avoids the likes of
    # 2008.1.1 incorrectly matching 2008.1.12.
    if {[regexp {^[\d\.]+$} $rtrdate]} {
        set cnt [llength [split $rtrdate "."]]
        set last_char [string range $rtrdate end end]
        if {$last_char != "." && $cnt < 4} {
            set rtrdate "${rtrdate}\."
        }
        # puts "last_char=$last_char rtrdate=$rtrdate"
    }
    if {[regexp {^[\d\.]+$} $stadate]} {
        set cnt [llength [split $stadate "."]]
        set last_char [string range $stadate end end]
        if {$last_char != "." && $cnt < 4} {
            set stadate "${stadate}\."
        }
        # puts "last_char=$last_char stadate=$stadate"
    }
    if {[regexp {^[\d\.]+$} $btdate]} {
        set cnt [llength [split $btdate "."]]
        set last_char [string range $btdate end end]
        if {$last_char != "." && $cnt < 4} {
            set btdate "${btdate}\."
        }
         puts "last_char=$last_char btdate=$btdate"
    }

    # Add tag/image/date info to Router objects.
    # NB: The config file may have set values for image/tag/date.
    # If so, these will be replaced with the command line option values.
    if {!$noapload && !$useobjectsasis} {
        # puts "adding router image/tag/date info"
        if {$rtrtrx ne ""} {
            # Specific image name takes precedance over date option.
            # We may need -tag info, so update only if non-blank.
            foreach Router $::router_sta_list {
                $Router configure -image $rtrtrx -date {}
                if {$rtrtag ne ""} {
                    $Router configure -tag $rtrtag
                }
            }

        } elseif {$rtrtag ne ""} {
            # Blow away -image name, add tag/date info.
            # NB: Tag name can be modified by date option.
            # EG: We need DIPSY2_BRANCH_4_170 branch, built on 2008.1.31
            foreach Router $::router_sta_list {
                $Router configure -tag $rtrtag -date $rtrdate -image {}
            }

        } elseif {$rtrdate ne ""} {
            # Date option is at the bottom of the option precendance hierarchy.
            # Blow away -image name, but leave -tag alone.
            foreach Router $::router_sta_list {
                $Router configure -date $rtrdate -image {}
            }
        }
    }

    # Add tag/date info to endpoint STA objects. WinBT objects have
    # slightly different options, and are processed separately.
    # NB: The config file may have set values for tag/image/date.
    # If so, these will be replaced with the command line option values.

    # With the advent of p2p_coex.test, we have scenarios where we are 
    # concurrently testing say a Linux NIC with a Linux USB dongle & AP. In
    # cases like this, we need to be selective about which tags are applied
    # to which object. If the user has specified stadhd (.ko) we dont want
    # to apply that to a Linux NIC. We also want to be careful about what
    # filetype (.ko, .bin, .trx, .sys) get applied to what object.

    if {!$nostaload && !$useobjectsasis} {
        # puts "adding stat tag/date info"
        # HND WLAN objects
        if {$stabin ne ""} {
            # Specific image name takes precedance over date option.
            # We may need -tag info, so update only if non-blank.
            foreach item $::endpoint_sta_list {
                set obj_type [UTF::check_host_type $item]
                # puts "item=$item obj_type=$obj_type"
                # When stadhd or windhdbin are present, then stabin applies to specific objects only.
                if {$stadhd ne "" && $obj_type ne "DHD"} {
                    UTF::Message WARN $item "setup_build_info: not tagging $item $obj_type with stabin=$stabin, stadhd=$stadhd was specified"
                    continue
                }
                if {$windhdbin ne "" && $obj_type ne "WinDHD"} {
                    UTF::Message WARN $item "setup_build_info: not tagging $item $obj_type with stabin=$stabin, windhdbin=$windhdbin was specified"
                    continue
                }
                # Use stabin filetype to select appropriate objects.
                if {[regexp -nocase {\.bin|\.trx} $stabin] && $obj_type ne "DHD"} {
                    UTF::Message WARN $item "setup_build_info: not tagging $item $obj_type with stabin=$stabin, wrong filetype"
                    continue
                }
                if {[regexp -nocase {\.ko} $stabin] && $obj_type ne "Linux"} {
                    UTF::Message WARN $item "setup_build_info: not tagging $item $obj_type with stabin=$stabin, wrong filetype"
                    continue
                }
                if {[regexp -nocase {\.sys} $stabin] && $obj_type ne "Cygwin" && $obj_type ne "WinDHD"} {
                    UTF::Message WARN $item "setup_build_info: not tagging $item $obj_type with stabin=s$stabin, wrong filetype"
                    continue
                }
                # We allow other filetypes to be blindly tagged. Hopefully the user knows what they are doing.

                # All the semantic checks have passed, tag the object.
                $item configure -image $stabin -date {}
                if {$statag ne ""} {
                    $item configure -tag $statag
                }
            }

        } elseif {$statag ne ""} {
            # NB: Tag name can be modified by date option.
            # EG: need DIPSY2_BRANCH_4_170 branch, built on 2008.1.31
            foreach item $::endpoint_sta_list {
                # puts "statag $item $statag"
                $item configure -tag $statag -date $stadate -image {}
            }

        } elseif {$stadate ne ""} {
            # Date option is at the bottom of the option precendance hierarchy.
            # Set -image to null, but leave -tag alone.
            foreach item $::endpoint_sta_list {
                # puts "stadate $item $stadate"
                $item configure -date $stadate -image {}
            }
        }

        # HND WinDHD objects only.
        if {$windhdbin ne ""} {
            # dongleimage option applies only to the WinDHD object.
            foreach item $::endpoint_sta_list {
                if {[UTF::check_host_type $item WinDHD]} {
                    $item configure -dongleimage $windhdbin
                }
            }
        }

        # HND DHD objects only. Should we enforce .bin/.trx filetype only?
        if {$stadhd ne ""} {
            # dhd_image option applies only to the DHD objects.
            foreach item $::endpoint_sta_list {
                if {[UTF::check_host_type $item DHD]} {
                    $item configure -dhd_image $stadhd
                }
            }
        }

        # HND DHD objects only.
        if {$dhdtag ne ""} {
            # dhd_tag option applies only to the DHD objects.
            foreach item $::endpoint_sta_list {
                if {[UTF::check_host_type $item DHD]} {
                    $item configure -dhd_tag $dhdtag
                }
            }
        }

        # BlueTooth objects. Currently there is no BT tag as BT is
        # single stream. BT items are in separate list, no need to 
        # check object type.
        if {$btcgr ne ""} {
            # Specific BT image name takes precedance over date option.
            foreach item $::endpoint_sta_bt_list {
                # puts "btdate $item $btcgr"
                $item configure -image $btcgr -date {} -version {}
            }

        } elseif {$btdate ne ""} {
            # Add WinBT date info. Do NOT change the -version!
            foreach item $::endpoint_sta_bt_list {
                # puts "btdate $item $btdate"
                $item configure -date $btdate -image {}
            }

        } elseif {$btver ne ""} {
            # Add WinBT version info.
            foreach item $::endpoint_sta_bt_list {
                # puts "btver $item $btver"
                $item configure -version $btver -date * -image {}
            }
        }
    }

    # Preserve existing branch_list info, if any, from higher level script.
    if {![info exists ::branch_list]} {
        set ::branch_list ""
    }
    UTF::Message INFO "" "setup_build_info: start ::branch_list=$::branch_list"

    # At this point, we have overridden the config file default values,
    # but have not filled in all the blank image/tag/date info. This is the
    # place to collect the initial Router & endpoint STA image/tag info.

    # Dont include TOT unless its the only tag available. The ::branch_list
    # influences the stream used for perfcache data. When there are other
    # choices besides TOT, we avoid using TOT.
    set item_list "" ;# list of STA to findimages for
    if {$noapload} {
        lappend ::branch_list "unknown"
    } else { 
        append item_list " $::router_sta_list"
    }
    if {$nostaload} {
        lappend ::branch_list "unknown"
    } else { 
        append item_list " $::endpoint_sta_list $::endpoint_sta_bt_list"
    }
    set all_tags ""
    foreach item $item_list {

        # If -image is available, save image info.
        set image [$item cget -image]
        set image [string trim $image]
        if {$image != ""} {
            lappend ::branch_list $image
            # puts "item=$item image=$image"
            continue
        }

        # Image wasnt available, so save tag info.
        set tag [$item cget -tag]
        if {$tag eq "NIGHTLY"} {
            set tag "TOT"
        }
        if {$tag != "" && $tag != "*"} {
            lappend all_tags $tag
            if {$tag != "TOT"} {
                lappend ::branch_list $tag
                # puts "item=$item tag=$tag"
            }
        }
        # For WinBT devices, save the version info.
        if {[UTF::check_host_type $item WinBT]} {
            set ver [$item cget -version]
            if {$ver != "" && $ver != "*"} {
                lappend ::branch_list $ver
                # puts "item=$item ver=$ver"
            }
        }
    }

    # Keep just the unique branchs
    set ::branch_list [string trim $::branch_list]
    if {$::branch_list == ""} {
        # The only branch info available may include TOT.
        set ::branch_list $all_tags
    }
    set ::branch_list [lsort -unique $::branch_list]
    UTF::Message INFO "" "::branch_list=$::branch_list" 

    # Check Router / STA have some criteria for picking a build.
    # NB: When objects are created, the default tag is usually NIGHTLY,
    # except WinBT which doesnt have streams.
    if {!$useobjectsasis} {
        foreach item "$item_list" {
            if {[string trim [$item cget -image]] == ""} {
                if {[UTF::check_host_type $item WinBT]} {
                    # Ensure BT date and version have a value.
                    if {[string trim [$item cget -date]] == ""} {
                        $item configure -date *
                    }
                    if {[string trim [$item cget -version]] == ""} {
                        $item configure -version *
                    }
    
                } else {
                    # Ensure WLAN tag & date have values.
                    if {[string trim [$item cget -tag]] == ""} {
                        $item configure -tag NIGHTLY
                    }
                    if {[string trim [$item cget -date]] == ""} {
                        $item configure -date *
                    }
                }
    
            } else {
                # Since a specific image has been specified, it is not
                # useful to have date info. Leave tag info alone.
                $item configure -date ""
            }
        }
    }

    # Initialize results
    set buildfinderror "" ;# collect list of device names we couldnt find builds for
    set result [UTF::setup_report_table];# html summary table
    if {$noapload} {
        append result "   <tr><td colspan=\"8\">Routers not being loaded<td><font color=\"green\"><b>OK</b></font></td></tr>\n"
    }
    if {$nostaload} {
        append result "   <tr><td colspan=\"8\">STAs not being loaded<td><font color=\"green\"><b>OK</b></font></td></tr>\n"
    }
    # puts "\n\n\nitem_list=$item_list \n\nresult=$result"

    # Find images and store in object -image. They will be different for
    # each AP/STA type, mini-PCI vs USB dongle, and OS involved.
    foreach item $item_list {

        # We generally dont want to load Sniffer objects. We usually want to
        # leave them running whatever stable build the user has chosen.
        if {[UTF::check_host_type $item Sniffer]} {
            UTF::Message INFO "" "UTF::setup_build_info: Skipping Sniffer $item"
            continue
        }

        # Get object image/tag/date.
        set image [string trim [$item cget -image]]
        set host [$item cget -host]
        set host [$host cget -name]
        set tag [string trim [$item cget -tag]]
        set date [string toupper [string trim [$item cget -date]]]
        set brand [$item cget -brand]
        set type ""
        catch {set type [$item cget -type]} ;# Router doesnt have -type

        # If image is already specified, check it exists.
        if {$image != ""} {
            if {$tag == ""} {
                set tag "&nbsp;"
            }
            if {$date == ""} {
                set date "&nbsp;"
            }
            if {[file exists $image]} {
                # File was found, extract id.
                set id [UTF::get_build_id $image]
                set notes [UTF::get_build_notes $image]
                set msg "   <tr><td>$item</td><td>&nbsp;</td><td>&nbsp;</td><td>$image</td><td>$tag</td><td>$date</td><td>$id</td><td>$notes</td><td><font color=\"green\"><b>OK</b></font></td></tr>\n"

            } else {
                # File not found.
                set msg "   <tr><td>$item</td><td>&nbsp;</td><td>&nbsp;</td><td>$image</td><td>$tag</td><td>$date</td><td>&nbsp;</td><td>&nbsp;</td><td><font color=\"red\"><b>Not found: $image</b></font></td></tr>\n"
                lappend buildfinderror $host $item
            }

            # Save result, move to next item.
            append result "$msg"
            UTF::Message INFO $item "$msg"
            continue
        }

        # If necessary, switch object brand & type to use external builds.
        # This agorithm will get you to the correct directory. There is no
        # guarantee that the same build exist in both the internal & external
        # directories, especially the 4325 builds.
        if {$external == 1} {
            regsub -nocase {linux-internal-dongle} $brand {linux-external-dongle-sdio} brand
            regsub -nocase {internal} $brand {external} brand
            regsub -nocase {checked} $type {Bcm} type
            regsub -nocase {DriverOnly} $type {Bcm_DriverOnly} type
            $item configure -brand $brand
            catch {$item configure -type $type}
            UTF::Message INFO "$item" "setting brand=$brand type=$type"
        }
        
        # Some objects have supporting image files, which can be specified on
        # the object instance in the config file, such as dhd.ko, 4315.nvm & wl.
        # Since the primary -image is null at this point, we need to set the 
        # supporting image files to null. This ensures that we dont mix and
        # match images from different builds.
        if {!$useobjectsasis} {
            catch {$item configure -dhd_image ""}
            catch {$item configure -nvram_image ""}
            catch {$item configure -wl_image ""}
        }
        
        # Try to find an image for this device. If user requested date=TODAY|CURRENT|*,
        # remove this date. Some of the object specific findimage methods get upset
        # with non-numeric dates when trying to find related dhd, etc files.
        if {$date == "TODAY" || $date == "CURRENT" || $date == "*"} {
            $item configure -date "20*" ;# avoid CVS subdirectories
        }
        # puts "\n\n\n$item findimage..."
        set catch_resp [catch {$item findimages} path]
        puts "path=$path"
        set aux_note ""
        if {[regexp -nocase {no\s+route\s+to\s+host|timeout} $path]} {
            # Reboot host and try findimage again.
            set catch_resp [catch {$item shutdown_reboot} catch_msg]
            if {$catch_resp == 0} {
                # Reboot worked, try findimage again.
                set aux_note "<b><font color=\"red\">Reboot PC <font color=\"green\">OK</b></font>"
                set catch_resp [catch {$item findimages} path]
                if {$catch_resp != 0} {
                    # Still couldnt findimage.
                    set msg "   <tr><td>$item</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>$tag</td><td>$date</td><td>&nbsp;</td><td>$aux_note</td><td><font color=\"red\"><b>$path</b></font></td></tr>\n"
                    append result "$msg"
                    UTF::Message ERROR $item "$msg"
                    lappend buildfinderror $host $item
                    $item configure -date $date
                    continue
                }

            } else {
                # Reboot failed.
                set aux_note "<b><font color=\"red\">Reboot PC Failed</b></font>"
                set msg "   <tr><td>$item</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>$tag</td><td>$date</td><td>&nbsp;</td><td>$aux_note</td><td><font color=\"red\"><b>$path</b></font></td></tr>\n"
                append result "$msg"
                UTF::Message ERROR $item "$msg"
                lappend buildfinderror $host $item
                $item configure -date $date
                continue
            }
        }
        UTF::Message INFO $item "imagepath=$path"

        # Save the image name, extract the build id.
        $item configure -image $path
        set id [UTF::get_build_id $path] ;# rearranged data used for display only!
        set notes "[UTF::get_build_notes $path] $aux_note"
        append result "   <tr><td>$item</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>$tag</td><td>$date</td><td>$id</td><td>$notes</td>"

        # Coex WinBT Ref Boards wont have a image.
        if {[UTF::check_host_type $item WinBT] && $path == ""} {
            append result "<td><font color=\"green\"><b>OK</b></font></td></tr>\n"
            continue
        }

        # Did we really find a file?
        set msg "<td><font color=\"red\"><b>$path</b></font></td></tr>\n"
        if {![file exists "$path"]} {
           append result "$msg"
           lappend buildfinderror $host $item
           UTF::Message ERROR $item "$msg"
           continue
        }
        
        # Check if image matches the desired date attributes.
        # If user requested date=TODAY, this will be translated to todays date.
        # The isssue here is that a build might be done at 11PM and the script
        # might not start running until 1AM, in which case the date check fails.
        # The other variation is clocks not being in sync due to daylight saving
        # time rule variations, causing the script to see the current build as
        # having tomorrows date. If user requested date=CURRENT, the dates
        # allowed will be tomorrows date, todays date, 1 day ago & 2 days ago.
        # This allows testing to handle the above date synchronization issue
        # and the occasional missing build due to compile errors.
        # NB: The build dates dont always have 2 digits for the day (leading
        # zero is suppressed). To avoid incorrect matches, the match pattern
        # includes a trailing "\.", taking advantage of the .N suffix for each
        # build done in a given day.
        if {$date == "" || $date == "*" || [UTF::check_host_type $item WinBT]} {
            # puts "No date checking required."

        } elseif {$date == "TODAY"} {
            if {![string match "*$::todays_date\.*" $path]} {
                set msg "<td><font color=\"red\"><b>Wrong date(1)</b></font></td></tr>\n"
                append result "$msg"
                lappend buildfinderror $host $item
                UTF::Message ERROR $item "$msg"
                continue
            }

        } elseif {$date == "CURRENT"} {
            if {![string match "*$::tomorrows_date\.*" $path] &&\
                ![string match "*$::todays_date\.*" $path] &&\
                ![string match "*$::1_day_ago_date\.*" $path] &&\
                ![string match "*$::2_days_ago_date\.*" $path]} {

                set msg "<td><font color=\"red\"><b>Wrong date(2) tomorrows_date=$::tomorrows_date todays_date=$::todays_date 1_day_ago_date=$::1_day_ago_date 2_days_ago_date=$::2_days_ago_date</b></font></td></tr>\n"
                append result "$msg"
                lappend buildfinderror $host $item
                UTF::Message ERROR $item "$msg"
                continue
            }

        } else {
            # When the rtrdate/stadate were added to objects, the date
            # trailing period issue was taken care of at that point.
# # #             if {![string match "*$date*" $path]} {
# # #                 set msg "<td><font color=\"red\"><b>Wrong date(3)</b></font></td></tr>\n"
# # #                 append result "$msg"
# # #                 lappend buildfinderror $host $item
# # #                 UTF::Message ERROR $item "$msg"
# # #                 continue
# # #             }
# # # do nothing
        }

        # Check if image matches the desired tag attribute
        set tag [string trim $tag]
        set msg "<td><font color=\"red\"><b>Wrong tag</b></font></td></tr>\n"
        if {$tag != "" && ![string match -nocase "*$tag*" $path] &&\
            ![UTF::check_host_type $item WinBT]} {
            append result "$msg"
            lappend buildfinderror $host $item
            UTF::Message ERROR $item "$msg"
            continue
        }

        # All checks passed.
        append result "<td><font color=\"green\"><b>OK</b></font></td></tr>\n"
    }

    # Wrap up the html table.
    append result "</table>\n<p>\n"

    # return build error list
    set buildfinderror [string trim $buildfinderror]
    # puts "\n\n\nresult=$result\n\n\n"
    return $buildfinderror
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::setup_config_testrig]]
    # Sets up global variables ::config_file, ::testrig
}

proc UTF::setup_config_testrig { } {

    # If we have non-blank ::testrig, we are done.
    if {[info exists ::testrig]} {
        set ::testrig [string trim $::testrig]
        if {$::testrig != ""} {
            return
        }
    }

    # Get testrig based on active config file.
    set ::config_file $UTF::args(utfconf)
    set ::testrig [string toupper [file rootname [file tail $::config_file]]]
    return
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::setup_devices]]

    # Used to setup all the devices in a testbed that use SSH.
}

proc UTF::setup_devices { } {

    # NB: WinBT has additional setup items built on top of Cygwin.
    # In order for the WinBT setup items to run, we must call the WinBT
    # objects explicitly. Using the OnAll method results in WinBT
    # objects being setup with the Cygwin setup method instead of
    # the WinBT setup method. Same story for Power::Agilent.
    set sta_list ""
    set catch_resp [catch {set sta_list [UTF::STA info instances]} catch_msg]
    set catch_resp [catch {set power_list [UTF::Power::Agilent info instances]} catch_msg]
    foreach item $sta_list {
        set type [$item hostis]
        if {[lsearch -exact $::ssh_type_list $type] >=0} {
            $item setup
        }
    }
    foreach item $power_list {
        $item setup
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::setup_email_subject] [arg Title]]

    # Sets up email subject in format: UTF Testrig [arg Title] TestName (Branch Names) [para]

    # Title is an optional text string, default is null.[para]

    # UTF::EndSummary will add the (N failures) info.[para]

    # Sets up global variable: ::test_name [para]

    # Assumes that UTF::setup_build_info has already been run.
}

proc UTF::setup_email_subject {{title ""}} {

    # Get test name
    set ::test_name [file tail $::argv0]
    set ::test_name [file rootname $::test_name]

    # Get testrig name
    UTF::setup_config_testrig

    # Shorten branch names.
    if {![info exists ::branch_list]} {
       set ::branch_list ""
    }
    set branches ""
    foreach branch $::branch_list {
        regsub {^[^_]+_[^_]+_} $branch "" branch ;# remove first 2 words
        append branches " $branch"
    }
    set branches [string trim $branches]
    regsub -all { } $branches ", " branches ;# add commas inside string

    # UTF::EndSummary will add the (N failures) info.
    set email_subject "UTF $::testrig $title $::test_name \($branches\)"
    set email_subject [string trim $email_subject]
    regsub -all {\s+} $email_subject " " email_subject ;# get rid of tabs, compress whitespace
    # puts "email_subject=$email_subject"
    return $email_subject
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::setup_report_table]]
    # Returns html string for report table.
}

proc UTF::setup_report_table { } {

    # Sanity checks
    UTF::setup_config_testrig
    if {![info exists ::todays_date]} {
        set ::todays_date ""
    }
    if {![info exists ::todays_time]} {
        set ::todays_time ""
    }

    # Start html table result
    append result "<!-- Build Info -->\n"
    append result "<p>\n<table border=\"2\">\n"
    append result "   <tr><td colspan=\"6\"><b>Testrig $::testrig</b></td>"
    append result "<td colspan=\"3\"><b>Tests started $::todays_date $::todays_time &nbsp;</b></td></tr>\n"
    append result "   <tr><td colspan=\"3\"><b>Device Name / HW Ver / OS Ver</b></td>"
    append result "<td colspan=\"3\"><b>Desired Image / Tag / Date</b></td>"
    append result "<td><b>Found Build ID</b></td>"
    append result "<td><b>Notes</b></td>"
    append result "<td><b>Status</b></td></tr>\n"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::setup_scale_factors] [arg tput]]
    # Used to setup scale factors used for plotting beacon and 
    # asscociation state data as line graphs.
}

proc UTF::setup_scale_factors {tput} {

    # We only set the scale factors once at the start of the tests.
    if {![info exists ::sta_scale_factor]} {
        set ::sta_scale_factor 0
    }
    if {$::sta_scale_factor > 0} {
        return
    }

    # We need two scale factors, for sta & ap. Ideally the state line
    # graphs will show discreetly on the bottom 10% of the graphs.
    if {$tput < 40} {
        set tput 40 ;# range for legacy devices
    }
    set ::sta_scale_factor [expr int($tput * 0.10)]
    set ::ap_scale_factor  [expr int($tput * 0.08)]
    UTF::Message INFO "" "setup_scale_factors: sta_scale_factor=$::sta_scale_factor ap_scale_factor=$::ap_scale_factor"
    return
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::setup_sniffer] [arg sniffer] [arg chanspec]]
    # If [arg sniffer] is not null, initializes [arg sniffer] to
    # channel [arg chanspec] Returns null or throws error.[para]

    # To start collecting packets, use proc UTF::start_sniffer. To
    # stop collecting packets and upload the .pcap file, use proc
    # UTF::stop_sniffer.
}

proc UTF::setup_sniffer {sniffer chanspec} {

    # Sniffer use is optional.
    set name [UTF::get_name $sniffer]
    UTF::Message INFO "$name" "setup_sniffer: sniffer=$sniffer chanspec=$chanspec"
    set sniffer [string trim $sniffer]
    if {$sniffer == ""} {
        return
    }

    set setup 0
    UTF::Try "Setup $sniffer CH=$chanspec" {

        # Validate sniffer name.
        set resp [UTF::check_host_type $sniffer Sniffer]
        if {$resp == 0} {
            error "setup_sniffer ERROR: $sniffer not type Sniffer"
        }

        # Reload sniffer & setup.
        $sniffer reload
        $sniffer setupSniffer $chanspec
        set sniffer_ver [$sniffer wl ver]
        regsub -all {\n} $sniffer_ver " " sniffer_ver
        set setup 1
        return "html: $sniffer: $sniffer_ver"
    }

    # If sniffer wasnt setup correctly, halt the tests
    if {$setup == 0} { 
        error "setup_sniffer got error, halting tests!"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::setup_testbed]]
    # Does setup for testbed only if not already done. Executes users
    # config file code from variable ::UTF::SetupTestBed, saves status
    # in variable ::UTF::doneSetupTestBed.
}

proc UTF::setup_testbed { } {

    # Many testrigs have Aeroflex attenuator initialization strings
    # defined, usually to set all channels to 0 attenuation. If found,
    # setup, and not already done, do the setup.
    # NB: Some people do additional steps in ::UTF::SetupTestBed, which 
    # can include turning off the AP radios!
    if {[info exists ::UTF::SetupTestBed]} {
        # This is an optimization to avoid doing the setup multiple times
        if {[info exists ::UTF::doneSetupTestBed] && $::UTF::doneSetupTestBed == "DONE"} {
            UTF::Message INFO "" "skipping ::UTF::SetupTestBed"
        } else {
            UTF::Try "Setup test bed" {
                # Set the flag before doing setup. Experience has shown there is no
                # value doing the setup multiple times because of an error. Setting
                # the flag first ensures we do the setup only once, regardless of errors.
                set ::UTF::doneSetupTestBed DONE
                eval $::UTF::SetupTestBed
                return
            }
        }
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::setup_testbed_info] [arg email_list] [arg log_dir]]

    # Gets testbed data from the config file. Formats the results as an 
    # html table.[para]

    # Sets up global lists: ::router_device_list, ::router_sta_list,
    # ::endpoint_device_list, ::endpoint_sta_list, ::endpoint_sta_bt_list,
    # ::lan_peer_sta_list, ::wan_peer_sta_list, ::lan_wan_device_list,
    # ::sniffer_sta_list, ::attn_grp_list [para]

    # Sets up global variables: ::config_file, ::localhost, ::home, 
    # ::1_day_ago_date, ::2_days_ago_date, ::testrig, ::todays_date,
    # ::todays_time, ::tomorrows_date, ::username, ::UTF::SummaryDir[para]

    # returns an html formatted string
}

proc UTF::setup_testbed_info {email_list log_dir} {

    # Get selected environment variables
    global env
    if {[info exists env(LOGNAME)]} {
       set ::username $env(LOGNAME)
    } else {
       set ::username unknown
    }
    if {[info exists env(HOME)]} {
       set ::home $env(HOME)
    } else {
       set ::home "/home/$::username"
    }
    if {[info exists env(HOSTNAME)]} {
       set ::localhost $env(HOSTNAME)
    } elseif {[info exists env(HOST)]} {
       set ::localhost $env(HOST)
    } else {
       set ::localhost unknown
    }
    # UTF::Message INFO "" "username=$::username home=$::home localhost=$::localhost"

    # Get config file, testrig.
    UTF::setup_config_testrig
    
    # Check log directory is defined, default to users HOME.
    if {![info exists ::UTF::SummaryDir]} {
        set ::UTF::SummaryDir "$::home"
    }

    # When putting results in the common SIG directory, we need to keep
    # keep results separated by username and at least one other parameter,
    # such as testrig name. If necessary, update the config file setting.
    if {[string match -nocase /projects/hnd_* $::UTF::SummaryDir]} {
        # Get list of subdirectories
        set temp0 [split $::UTF::SummaryDir "/"]
        set temp1 [lindex $temp0 1] ;# projects
        set temp2 [lindex $temp0 2] ;# disk_extention
        set temp3 [lindex $temp0 3] ;# might be userid
        set temp4 [lrange $temp0 4 end] ;# might be testrig
        set temp4 [join $temp4 "/"]
        set temp4 [string trim $temp4]
        # puts "temp0=$temp0 temp1=$temp1 temp2=$temp2 temp3=$temp3 temp4=$temp4"

        # hnd_software/utf is a symbolic link to hnd_sig/utf
        # NB: Dont change hnd_svt volumes!
        if {$temp2 == "hnd_software" && $temp3 == "utf"} {
            set temp2 hnd_sig
        }

        # Build up UTF::SummaryDir
        set ::UTF::SummaryDir "/$temp1/$temp2"

        # Always replace 3rd token with username
        set ::UTF::SummaryDir "$::UTF::SummaryDir/$::username"

        # We need at least one more token.
        if {$temp4 == ""} {
            # Put log files in subdirectory for the specific testbed/config file.
            set temp4 [file tail $::config_file]
            set temp4 [file rootname $temp4]
            regsub -all {\.} $temp4 "" temp4
            set ::UTF::SummaryDir "$UTF::SummaryDir/$temp4"
        } else {
            # Use the existing token(s) to complete the path.
            set ::UTF::SummaryDir "$UTF::SummaryDir/$temp4"
        }
    }

    # If necessary, override the config file log directory setting.
    if {$log_dir != ""} {
        set UTF::SummaryDir $log_dir
    }
    UTF::Message INFO "" "Logfiles located in ::UTF::SummaryDir=$::UTF::SummaryDir"
    set catch_resp [catch {exec mkdir -p $::UTF::SummaryDir} catch_msg]
    if {$catch_resp != 0} {
        UTF::Message WARN "" "setup_testbed_info: mkdir -p $::UTF::SummaryDir: $catch_msg"
    }
    if {! [file isdirectory "$::UTF::SummaryDir"]} {
        error "setup_testbed_info ERROR directory $::UTF::SummaryDir not found!"
    }
    if {! [file writable "$::UTF::SummaryDir"]} {
        error "setup_testbed_info ERROR directory $::UTF::SummaryDir not writable!"
    }

    # Get dates & time.
    set now_sec [clock seconds]
    set 1_day_sec [expr 24 * 60 * 60]
    set tomorrow_sec [expr $now_sec + $1_day_sec]
    set 1_day_ago_sec [expr $now_sec - $1_day_sec]
    set 2_days_ago_sec [expr $now_sec - (2* $1_day_sec)]
    set ::tomorrows_date [join [clock format $tomorrow_sec -format "%Y %N %e"] .] ;# use join command
    set ::todays_date [join [clock format $now_sec -format "%Y %N %e"] .]
    set ::todays_time [clock format $now_sec -format "%T"] ;# obtain proper time code from tcl8.5
    set ::1_day_ago_date [join [clock format $1_day_ago_sec -format "%Y %N %e"] .]
    set ::2_days_ago_date [join [clock format $2_days_ago_sec -format "%Y %N %e"] .]
    # UTF::Message INFO "" "tomorrows_date=$::tomorrows_date todays_date=$::todays_date\
    #    todays_time=$::todays_time 1_day_ago_date=$::1_day_ago_date\
    #    2_days_ago_date=$::2_days_ago_date"

    # Locate all Router STA names. -all option gets the clones.
    set ::router_sta_list ""
    foreach item $::ap_type_list {
        append ::router_sta_list " [UTF::get_names_values UTF::${item} sta names -all]"
    }

    # Locate all Router device names for all the Router STA names.
    # NB: we need 1 hostname per Router STA, regardless of clones.
    set ::router_device_list ""
    foreach item $::router_sta_list {
        set host [$item cget -host]
        set name [$host cget -name]
        # puts "item=$item host=$host name=$name"
        lappend ::router_device_list $name
    }

    # Locate all Endpoint STA names. Some boxes many have multiple STAs.
    # -all option gets the clones.
    set ::endpoint_sta_list ""
    foreach item $::sta_type_list {
        append ::endpoint_sta_list " [UTF::get_names_values UTF::${item} sta names -all]"
    }

    # Workaround to ignore duplicate DHD objects
    set ::endpoint_sta_list [lsort -unique $::endpoint_sta_list]

    # Keep the BlueTooth STA separate from WLAN. -all option gets the clones.
    set ::endpoint_sta_bt_list "[UTF::get_names_values UTF::WinBT sta names -all]"

    # Keep the Sniffer STA separate from WLAN & BT.
    set ::sniffer_sta_list "[UTF::get_names_values UTF::Sniffer sta names -all]"

    # Locate all Endpoint device names for all the STA names.
    # NB: we need 1 hostname per STA, regardless of clones.
    # Sniffer is included for benefit of RebootTestbed.test.
    set ::endpoint_device_list ""
    foreach item "$::endpoint_sta_list $::endpoint_sta_bt_list\
        $::sniffer_sta_list" {
        set host [$item cget -host]
        set name [$host cget -name]
        lappend ::endpoint_device_list $name
    }

    # Locate all Router peer STA names. These are wired connections, not radios.
    # NB: There can be multiple devices named in -lanpeer & -wanpeer.
    set ::lan_peer_sta_list ""
    set ::wan_peer_sta_list ""
    foreach item $::router_sta_list {
        # There are some config files with invalid lan/wan peers.
        # Remove invalid items, if any.
        foreach type "lan wan" {
            set peer_list [$item cget -${type}peer]
            set valid_peer ""
            foreach peer $peer_list {
                set catch_resp [catch {set name [$peer cget -name]} catch_msg]
                if {$catch_resp == 0} {
                    # This lanpeer is valid, save it.
                    lappend valid_peer $peer
                    set var "::${type}_peer_sta_list"
                    lappend $var $peer
                } else {
                    UTF::Message ERROR "" "Removing $item ${type}peer $peer: $catch_msg"
                }
            }

            # Update the router with list of known valid peers.
            $item configure -${type}peer $valid_peer
        }
    }
    set ::lan_peer_sta_list [string trim $::lan_peer_sta_list]
    set ::wan_peer_sta_list [string trim $::wan_peer_sta_list]

    # Remove Router lan/wan peer STA from endpoint device/STA lists, giving
    # endpoint radios only. Get list of all lan / wan device names.
    set ::lan_wan_device_list ""
    foreach item "$::lan_peer_sta_list $::wan_peer_sta_list" {

        # Save name on lan/wan device list
        set host [$item cget -host]
        set name [$host cget -name]
        lappend ::lan_wan_device_list $name

        # Find host name and remove from device list
        # puts "item=$item host=$host name=$name"
        set i [lsearch -exact $::endpoint_device_list $name]
        if {$i >= 0} {
            # UTF::Message INFO "" "Removing lan/wanpeer $name"
            set ::endpoint_device_list [lreplace $::endpoint_device_list $i $i]
        }

        # Remove from sta list
        set i [lsearch -exact $::endpoint_sta_list $item]
        if {$i >= 0} {
            # UTF::Message INFO "" "Removing lan/wanpeer $item"
            set ::endpoint_sta_list [lreplace $::endpoint_sta_list $i $i]
        }

        # Remove from sta BT list
        set i [lsearch -exact $::endpoint_sta_bt_list $item]
        if {$i >= 0} {
            # UTF::Message INFO "" "Removing lan/wanpeer $item"
            set ::endpoint_sta_bt_list [lreplace $::endpoint_sta_bt_list $i $i]
        }
    }

    # Sort the device lists, but leave duplicate items in place. This keeps
    # one host device per (clone) STA.
    set ::endpoint_sta_list [lsort $::endpoint_sta_list]
    set ::endpoint_sta_bt_list [lsort $::endpoint_sta_bt_list]
    set ::endpoint_device_list [lsort $::endpoint_device_list]
    set ::router_device_list [lsort $::router_device_list]
    set ::router_sta_list [lsort $::router_sta_list]

    # Sort & remove duplicates from lan/wan peer lists.
    set ::lan_peer_sta_list [lsort -unique $::lan_peer_sta_list]
    set ::wan_peer_sta_list [lsort -unique $::wan_peer_sta_list]
    set ::lan_wan_device_list [lsort -unique $::lan_wan_device_list]

    if {0} { ;# comment out: not display 'Testbed Info' table
    # Locate all attenuator group names.
    set ::attn_grp_list ""
    foreach type $::attn_grp_type_list {
        append ::attn_grp_list " [UTF::get_names_values $type]" 
    }
    regsub -all {::} $::attn_grp_list "" ::attn_grp_list
    set ::attn_grp_list [lsort -unique $::attn_grp_list]

    # Format data as html table for web pages    
    set result ""
    append result "<!-- Testbed Info -->\n"
    append result "<p><table border=\"2\">\n"
    append result "   <tr><th colspan=\"2\">Testbed Info</tr>\n"
    append result "   <tr><td><b>Item<td><b>Value</tr>\n"
    append result "   <tr><td>username<td>$::username</tr>\n"
    append result "   <tr><td>localhost<td>$::localhost</tr>\n"
    append result "   <tr><td>TCL pwd<td>[pwd]</tr>\n"
    set temp $::auto_path
    regsub -all " " $temp "<br>" temp ;# insert html breaks
    append result "   <tr><td>TCL auto_path<td>$temp</tr>\n"
    append result "   <tr><td>UTF::SummaryDir<td>$UTF::SummaryDir</tr>\n"
    set temp $email_list
    if {$temp == ""} {
        set temp "&nbsp;"
    }
    append result "   <tr><td>Email list<td>$temp</tr>\n"
    append result "   <tr><td>config_file<td>$::config_file</tr>\n"
    append result "   <tr><td>Testrig<td>$::testrig</tr>\n"
    set temp $::argv
    if {$temp == ""} {
        set temp "&nbsp;"
    }
    regsub -all {\-password\s+\S+} $temp "" temp ;# hide passwords
    regsub -all { \-} $temp {<br> -} temp ;# add breaks before each token
    set ::rtr_device_list [lsort -unique $::router_device_list]
    regsub -all {\.\d+} $::router_sta_list {} ::rtr_sta_list
    set ::rtr_sta_list [lsort -unique $::rtr_sta_list]
    append result "   <tr><td>command line args<td>$temp</tr>\n"
    append result "   <tr><td>router_device_list<td>$::rtr_device_list</tr>\n"
    append result "   <tr><td>router_sta_list<td>$::rtr_sta_list</tr>\n"
    append result "   <tr><td>lan_wan_device_list<td>$::lan_wan_device_list</tr>\n"
    append result "   <tr><td>lan_peer_sta_list<td>$::lan_peer_sta_list</tr>\n"
    append result "   <tr><td>wan_peer_sta_list<td>$::wan_peer_sta_list</tr>\n"
    append result "   <tr><td>endpoint_device_list<td>$::endpoint_device_list</tr>\n"
    append result "   <tr><td>endpoint_sta_list<td>$::endpoint_sta_list</tr>\n"
    append result "   <tr><td>endpoint_sta_bt_list<td>$::endpoint_sta_bt_list</tr>\n"
    append result "   <tr><td>sniffer_sta_list<td>$::sniffer_sta_list</tr>\n"
    append result "   <tr><td>attn_grp_list<td>$::attn_grp_list</tr>\n"
    append result "</table>\n"

    # Return html formatted result.
    # UTF::Message INFO "" "$result"
    return $result
    } ;# comment out
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::snifferCtl] [arg cmd] [arg args]]

    # Wrapper to access the UTF Sniffer host and run the [arg cmd] & [arg args][para]

    # The sniffer object to use is expected to be in variable ::utf_sniffer_host. The
    # sniffer interface to use is expected to be in variable ::utf_sniffer_if. If  
    # either variable is undefined or null, then this routine takes this as an 
    # indication that the sniffer either does not physically exist in your testrig
    # or is not to be used for this test. Consequently this routine does nothing
    # and returns.[para]

    # When [arg cmd]=start and [arg args]=yes, a single .pcap sniffer file will
    # be created. When [arg cmd]=start and [arg args]=ring, a series of ring buffer
    # .pcap sniffer files will be created.[para]

    # When [arg cmd]=stop and [arg args]=yes, a single .pcap sniffer file will
    # be retrieved. When [arg cmd]=stop and [arg args]=ring, a series of ring buffer
    # .pcap sniffer files will be retrieved.[para]

    # When [arg cmd]=start and [arg args]=no, .pcap sniffer files are not created.[para]

    # It is assumed that you have created an appropriate subdirectory for the .pcap
    # files by calling Sniffer method setupDir.[para]

    # Returns: response message from underlying methods.
}

proc UTF::snifferCtl {cmd args} {

    # Check if we have a sniffer host to use.
    if {![info exists ::utf_sniffer_host] || ![info exists ::utf_sniffer_if] ||\
        $::utf_sniffer_host == "" || $::utf_sniffer_if == ""} {
        UTF::Message INFO "" "UTF::snifferCtl: config file not have a UTF::Sniffer object or is not being used."
        return    
    }

    # Use the Sniffer method to run cmd & args.
    set cmd [string trim $cmd]
    set args [string trim $args]
    UTF::Message INFO "" "UTF::snifferCtl cmd=$cmd args=$args\
        utf_sniffer_host=$::utf_sniffer_host utf_sniffer_if=$::utf_sniffer_if"
    set catch_resp1 [catch {$::utf_sniffer_host snifferCtl $cmd $args} catch_msg1]

    # When Sniffer is being stopped, we also collect the .pcap file(s).
    set catch_resp2 0
    set catch_msg2 ""
    if {$cmd == "stop"} {
        if {$args == "yes"} {
            set catch_resp2 [catch {$::utf_sniffer_host collectFile} catch_msg2]
 
        } elseif {$args == "ring"} {
            set catch_resp2 [catch {$::utf_sniffer_host collectRingFiles} catch_msg2]
        }
    }

    # Check for errors.
    set catch_msg [string trim "$catch_msg1 $catch_msg2"]
    if {($catch_resp1 != 0 || $catch_resp2 != 0) && $catch_msg != ""} {
        UTF::Message ERROR "" "UTF::sniffer_Ctl: catch_msg=$catch_msg"
        # Maintain compatibility with btampqtp.test
        if {[info exists ::tc_errors]} {
            incr ::tc_errors
            lappend ::error_list $catch_msg
        }
    }
    return $catch_msg
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::sta_ampdu] [arg STA] [opt nohistograms=0]]

    # Queries the [arg STA] ampdu stats if [opt nohistograms] is 0.[para]

    # Returns CSV string with counts for: RX MCS, TX MCS, MPDU,
    # RX MCS SGI, TX MCS SGI, MCS PER, RX VHT, TX VHT, RX VHT SGI, TX VHT SGI, VHT PER,
}

proc UTF::sta_ampdu {STA {nohistograms 0}} {

    # If STA is known to be in trouble or nohistograms!=0, return.
    UTF::Message INFO "$STA" "sta_ampdu: STA=$STA nohistograms=$nohistograms"
    set msg " , , , , , , , , , , , ," ;# Need 1 comma for each 12 fields returned.
    set var "::${STA}_state"
    if {[set $var] != "OK" || $nohistograms != "0"} {
        return $msg
    }

    # Get wl dump ampdu & save device status.
    set catch_resp [catch {$STA wl dump ampdu} catch_msg]
    UTF::save_device_state $STA $catch_msg
    if {$catch_resp != 0 || $catch_msg == "N/A"} {
        UTF::Message ERROR "$STA" "catch_msg=$catch_msg"
        return $msg
    }

    # NB: For SISO devices, max MCS is 7, so there will be only one row of MCS stats!
    # NB: Sometimes there will be a MAC address after these strings.
    # So parsing has to avoid picking up: 01:23:...

    # Look for 1 line or more of RX MCS stats.
    # set catch_msg "RX MCS :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  7(1%)\n\
    #			    :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  14(2%)\n\
    #			    :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  21(3%)\n\
    #			    :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  28(4%)"
    if {![regexp -nocase {RX\s*MCS(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg rx_mcs]} {
        set rx_mcs ""
    }
        
    # Look for 1 line or more of TX MCS stats.
    if {![regexp -nocase {TX\s*MCS(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg tx_mcs]} {
        set tx_mcs ""
    }

    # Look for 1 line or more of MPDUdens stats.
    # NB: Not always multiples of 8! Can be only 1!
    # set catch_msg "MPDUdens:   0 884 123  88 118 154  47 268 1134 341  97  56  18  34  51 326 3474"
    if {![regexp -nocase {MPDUdens(\s*:(\s*\d+\s*\(\d+%\)\s*){1,}){1,}} $catch_msg mpdu_dens]} {
        # Old format is list of space separated digits.
        if {![regexp {MPDUdens\s*:[\d\s]+\n} $catch_msg mpdu_dens]} {
            set mpdu_dens ""
        }
    }

    # Look for 1 line or more of RX MCS SGI stats.
    if {![regexp -nocase {RX\s*MCS\s*SGI(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg rx_mcs_sgi]} {
        set rx_mcs_sgi ""
    }

    # Look for 1 line or more of TX MCS SGI stats.
    # set catch_msg "TX MCS SGI: \n00:90:4c:14:4a:ab: max_pdu 8 release 8" ;# no sgi data followed by mac addr
    if {![regexp -nocase {TX\s*MCS\s*SGI(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg tx_mcs_sgi]} {
        set tx_mcs_sgi ""
    }

    # Look for 1 line or more of MCS PER stats.
    if {![regexp -nocase {MCS\s*PER(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg mcs_per]} {
        set mcs_per ""
    }

    # Look for 1 line or more of RX VHT stats.
    # set catch_msg "RX VHT  :  1(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%) \
    #                        :  0(0%)  2(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  8(0%) \
    #                        :  0(0%)  0(0%)  3(0%)  0(0%)  0(0%)  0(0%)  5470(5%)  8836(93%)  27(0%)  0(0%)"
    set sta_vht_len 0
    if {[regexp -nocase {RX\s*VHT(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg rx_vht]} {
	# The purpose here is to get the number of fields, either 10 or 12.
	regexp -nocase {^RX\s*VHT(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,1}} $rx_vht rx_vht_tmp
	regexp -line {RX\s*VHT\s*:\s*(.*)} $rx_vht_tmp - rx_vht_final
	set sta_vht_len [llength $rx_vht_final]
    } else {
        set rx_vht ""
    }

    # Look for 1 line or more of TX VHT stats.
    # set catch_msg "TX VHT  :  1(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%) \
    #                        :  0(0%)  2(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  8(0%) \
    #                        :  0(0%)  0(0%)  3(0%)  0(0%)  0(0%)  0(0%)  5471(5%)  8833(93%)  290(0%)  0(0%)"
    if {![regexp -nocase {TX\s*VHT(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg tx_vht]} {
        set tx_vht ""
    }

    # Look for 1 line or more of RX VHT SGI stats.
    # set catch_msg "RX VHT SGI :  1(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%) \
    #                           :  0(0%)  2(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  8(0%) \
    #                           :  0(0%)  0(0%)  3(0%)  0(0%)  0(0%)  0(0%)  540(5%)  884(93%)  72(0%)  0(0%)"
    if {![regexp -nocase {RX\s*VHT\s*SGI(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg rx_vht_sgi]} {
        set rx_vht_sgi ""
    }

    # Look for 1 line or more of TX VHT SGI stats.
    # set catch_msg "TX VHT SGI :  1(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%) \
    #                           :  0(0%)  2(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  8(0%) \
    #                           :  0(0%)  0(0%)  3(0%)  0(0%)  0(0%)  0(0%)  5(5%)  8(93%)  300(0%)  0(0%)"
    if {![regexp -nocase {TX\s*VHT\s*SGI(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg tx_vht_sgi]} {
        set tx_vht_sgi ""
    }

    # Look for 1 line or more of VHT PER stats.
    if {![regexp -nocase {VHT\s*PER(\s*:(\s*\d+\s*\(\d+%\)\s*){8,}){1,}} $catch_msg vht_per]} {
        set vht_per ""
    }

    # Cleanup distribution strings, leaving just the raw frame counts.
    set i 1
    set result $sta_vht_len  ;# put it as first element, and will remove it later in rvr1.test. 
    foreach list [list $rx_mcs $tx_mcs $mpdu_dens $rx_mcs_sgi $tx_mcs_sgi $mcs_per $rx_vht $tx_vht $rx_vht_sgi $tx_vht_sgi $vht_per] {
        if {$i == 6 || $i == 11} {
            # For PER data, extract percentage numbers.
            regsub -all {\d+\(} $list "" list   ;# remove '8(' part
            regsub -all {%\)}   $list "" list   ;# remove '%)' part
        } else {
	    # For others, remove percentage numbers.
            regsub -all {\(\d+%\)} $list "" list ;# remove percentage numbers (nn%)
        }
        regsub -all {\n} $list "" list 		;# remove new-lines
        regsub -all {:} $list "" list 		;# remove colons
        regsub -all {[a-zA-Z]} $list "" list	;# remove text
        regsub -all {\s+} $list " " list 	;# compress consecutive whitespace into single space

	# Save a copy.
        if {$i == 2 || $i == 8} {
            set tx_saved $list
        }

	# If there is TX data, use original PER data. Otherwise, replace it with -1.
        if {$i == 6 || $i == 11} {
            set j 0
	    set list_tmp ""
            foreach element $tx_saved {
                if {$element == 0} {
                    append list_tmp "-1 "
                } else {
                    append list_tmp "[lindex $list $j] "
                }
                incr j
            }
            set list $list_tmp
        }

	# Fill in some data to avoid gnuplot errors.
        set list [string trim $list]
        if {$list == "" || $list == "0"} {
            if {$i == 6 || $i == 11} {
                set list "-1 -1 -1 -1 -1 -1 -1 -1"
            } else {
                set list ""
            }
        }

	# Result is CSV formatted.
	append result " ${list},"
	incr i
    }

    # Log & return result.
    UTF::Message INFO "" "sta_ampdu distributions: sta_vht_len,rx_mcs,tx_mcs,mpdu_dens,rx_mcs_sgi,tx_mcs_sgi,mcs_per,rx_vht,tx_vht,rx_vht_sgi,tx_vht_sgi,vht_per,=$result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::sta_chan_info] [arg STA] [opt ctl=0]]

    # Queries the [arg STA] chanspec. Useful for plotting chanspec as
    # an integer to observe any unexpected changes. BW40 channels are
    # shown as a very large integer. If [arg ctl] is 0, data is not
    # collected.[para]

    # Returns CSV string with: staChanSpec, staChanInt,
}

proc UTF::sta_chan_info {STA {ctl 0}} {

    # If STA is known to be in trouble, return.
    set msg " , ,"
    set var "::${STA}_state"
    if {[set $var] != "OK" || $ctl == 0} {
        return $msg
    }

    # Get current STA chanspec
    set catch_resp [catch {$STA wl chanspec} staChanSpec]
    UTF::save_device_state $STA $staChanSpec
    puts "staChanSpec=$staChanSpec"

    # Look for simple channel number, no u or l. 
    if {![regexp {^\s*(\d+)\s*\(} $staChanSpec - chan]} {
        # For BW40 channels, extract 0x.... portion
        if {![regexp {\((\s*0x[0-9,a-f,A-F]+)\s*\)} $staChanSpec - chan]} {
            set chan -1
        }
    }
    set staChanInt [expr int($chan)]
    UTF::Message INFO "" "sta_chan_info: STA=$STA staChanSpec=$staChanSpec chan=$chan staChanInt=$staChanInt"
    return "$staChanSpec, $staChanInt,"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::sta_stats] [arg STA] [arg musta]]

    # Queries the [arg STA] stats.[para]

    # Returns CSV string with: staAssocBeacon, staMcs, staNss, staBw, staRxRate, staState, staTxRate,
}

proc UTF::sta_stats {STA musta} {
    UTF::Message INFO "$STA" "\nCalling sta_stats: STA=$STA $musta\n"

    # If STA is known to be in trouble, return.
    set msg " , , , , , ,"
    set var "::${STA}_state"
    if {[set $var] != "OK"} {
        return $msg
    }

    # Initialize results. gnuplot responds better to 0 than n/a.
    set staAssocBeacon 0
    set staMcs ""
    set staNss ""
    set staBw ""
    set staRxRate 0 ;# right now there is no separate STA RX rate reported
    set staState NULL
    set staTxRate 0

    # Get staTxRate
    set catch_resp1 [catch {$STA wl rate} catch_msg1]
    if {$catch_resp1 != 0} {
        UTF::Message INFO "$STA" "catch_msg1=$catch_msg1"
    }
    UTF::save_device_state $STA $catch_msg1
    if {[regexp -nocase {([\.\d]+)\s*Mbps} $catch_msg1 - rate]} {
        set staTxRate $rate ;# rate is already in Mb/s
    }
    # For musta, only do the calling to have messages in log but not doing any data processing.
    if {$musta != ""} {
	foreach MUSTA $musta {
	    set catch_ret [catch {$MUSTA wl rate} catch_ret]
	}
    }
   
    # Get staState, save as global. staAssocBeacon is a numeric value for staState
    # so it can be plotted as a line graph.
    set catch_resp2 [catch {$STA wl status} catch_msg2]
    UTF::save_device_state $STA $catch_msg2
    if {[regexp -nocase {not.*associated} $catch_msg2]} {
        set staState ROAMING
        set staAssocBeacon 0
    } elseif {[regexp -nocase {BSSID:\s*00:00:00:00:00:00} $catch_msg2]} {
        set staState ASSOCIATED ;# but no beacons
        set staAssocBeacon [expr $::sta_scale_factor / 2]
    } elseif {[regexp -nocase {BSSID:\s*[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}} $catch_msg2]} {
        set staState "ASSOCIATED BEACONS"
        set staAssocBeacon $::sta_scale_factor
    }
    set temp "::${STA}_assoc"
    set $temp $staState
    set temp "::${STA}_msg"
    set $temp $catch_msg2

    # We save either nrate or rate info for benefit of other routines.
    # Info is prefixed with M for MCS rate or L for legacy rate.
    set temp "::${STA}_rate"

    # Get staMcs, staNss and staBw
    if {[set $var] != "OK"} {
        return $msg
    }
    set catch_resp4 [catch {$STA wl nrate} catch_msg4]
    UTF::save_device_state $STA $catch_msg4
    if {[regexp -nocase {mcs\s*index\s*(\d+)} $catch_msg4 - mcs]} {
	# Sample nrate msg for 11n: "mcs index 15 stf mode 3 auto"
	set staMcs $mcs
	#set $temp "M${nrate}"
    } elseif {[regexp -nocase {vht\s*mcs\s*(\d+)\s*Nss\s*(\d+).*bw(\d)} $catch_msg4 - mcs nss bw]} {
	# Sample nrate msg for 11ac: "vht mcs 9 Nss 3 Tx Exp 0 bw80 ldpc sgi auto"
	set staMcs $mcs
	set staNss $nss
	set staBw  $bw
	#set $temp "V${nrate}${nss}"
    } else {
	#set $temp "L${staTxRate}"
    }
    # For musta, only do the calling to have messages in log but not doing any data processing.
    if {$musta != ""} {
        foreach MUSTA $musta {
    	    set catch_ret [catch {$MUSTA wl nrate} catch_ret]
	}
    }

    # Return CSV formatted results.
    UTF::Message INFO "" "sta_stats: staAssocBeacon=$staAssocBeacon\
	staMcs=$staMcs staNss=$staNss staBw=$staBw staRxRate=$staRxRate staState=$staState\
	staTxRate=$staTxRate"
    return "$staAssocBeacon, $staMcs, $staNss, $staBw, $staRxRate, $staState, $staTxRate,"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::start_sniffer] [arg sniffer] [opt ring_file_cnt=null] [opt ring_file_size=null]
    # [opt sniffercapture=null] [opt dir=null] [opt attn=null]]
    # Starts .pcap capture file on [arg sniffer]. [arg ring_file_cnt] null means a single .pcap file
    # will be generated. [arg ring_file_cnt] greater than 0 means use multiple files in a ring buffer
    # of size [arg ring_file_size] Mbytes each. For [arg sniffercapture] description, see 
    # UTF::check_sniffer_capture_steps. [arg dir] is RampDown or RampUp. [arg attn] is the
    # attenuator step. Returns null.[para]

    # Because ring buffer files are always named by date & time, they tend to accumulate
    # until the sniffer local disk is full. For this reason, any existing ring buffer files
    # are deleted before starting a new sniffer trace. We also delete the single fixed name .pcap
    # file. This ensures that you will have maximum room on the sniffer local disk for the
    # current test.
}

proc UTF::start_sniffer {sniffer {ring_file_cnt ""} {ring_file_size ""} {sniffercapture ""}\
    {dir ""} {attn ""}} {

    # Sniffer use is optional.
    set name [UTF::get_name $sniffer]
    UTF::Message INFO "$name" "start_sniffer: sniffer=$sniffer ring_file_cnt=$ring_file_cnt\
        ring_file_size=$ring_file_size sniffercapture=$sniffercapture dir=$dir attn=$attn"
    if {$sniffer == ""} {
        return
    }

    # User may want the sniffer to start/stop at an arbitrary set of attenuation points
    # to create one or more large .pcap files.
    set sniffercapture [string trim $sniffercapture]
    set sniffercapture [string tolower $sniffercapture]
    if {$sniffercapture != ""} {

        # If there are multiple capture steps, make sure the state variables are initialized.
        # For multiple consecutive runs, these variables should be reset by the calling routine.
        if {![info exists ::sniffer_capture_active]} {
            set ::sniffer_capture_active "" ;# on/off sniffer state for large .pcap file
        }
        if {![info exists ::sniffer_capture_index]} {
            set ::sniffer_capture_index "" ;# current index into list of sniffer commands for large .pcap file(s)
        }

        # If sniffer is already active, we are done.
        if {$::sniffer_capture_active == "on"} {
            return
        }

        # Check dir & attn values.
        set dir [string trim $dir]
        set dir [string tolower $dir]
        if {$dir != "rampdown" && $dir != "rampup" && ![regexp {fastrampup\d+} $dir]} {
            error "start_sniffer ERROR: invalid dir=$dir, must be RampDown or RampUp or FastRampUpnn!"
        }
        set attn [string trim $attn]
        if {![regexp {^\d+$} $attn]} {
            error "start_sniffer ERROR: invalid attn=$attn, must be integer!"
        }

        # Format curr_step as: dnnn or fnn or unnn
        if {[regexp {fastrampup(\d+)} $dir - temp]} {
            # For fastrampup, we always use the same attenuator value. To distinguish
            # between these steps, we use the step sequence number. 
            set curr_step "f${temp}"
        } else {
            set temp [string range $dir 4 4]
            set curr_step "${temp}${attn}"
        }

        # Find next start step index. Start steps are the even numbered steps in the list.
        if {$::sniffer_capture_index == ""} {
            set start_index 0
        } else {
            set start_index [expr $::sniffer_capture_index + 1]
        }

        # Sanity check start_index is even number.
        if {[expr $start_index % 2] != 0} {
            error "start_sniffer ERROR: start_index=$start_index NOT an even number,\
                sniffer_capture_index=$::sniffer_capture_index, sniffercapture=$sniffercapture"
        }

        # If we are not at the next start step, we are done.
        set next_start_step [lindex $sniffercapture $start_index]
        if {$curr_step != $next_start_step} {
            return
        }

        # We ARE at the next start step. Save new state info, as the sniffer will be started.
        set ::sniffer_capture_active on
        set ::sniffer_capture_index $start_index
        UTF::Message INFO "$name" "start_sniffer: sniffer_capture_active=$::sniffer_capture_active\
            sniffer_capture_index=$::sniffer_capture_index"
    }

    # Ring buffer files are always named with todays date & time. As a result, we will not
    # automatically overwrite existing ring buffer files the way we would for a single .pcap
    # file that uses the same filename every time. That means that the sniffer local disk will 
    # slowly fill up with ring buffer files until it is full. To ensure we have the maximum amount
    # of disk space for the sniffer trace, we delete any previous ring buffer files before starting
    # the sniffer.
    set catch_resp [catch {$sniffer rexec rm -f /root/ring_*} catch_msg]
    if {$catch_resp == 0} {
        UTF::Message INFO $sniffer "$catch_msg"
    } else {
        UTF::Message ERROR $sniffer "$catch_msg"
    }

    # Also, delete the single fixed name .pcap file as well.
    set catch_resp [catch {$sniffer rexec rm -f /root/captureFile.pcap} catch_msg]
    if {$catch_resp == 0} {
        UTF::Message INFO $sniffer "$catch_msg"
    } else {
        UTF::Message ERROR $sniffer "$catch_msg"
    }

    # Start the sniffer.
    $sniffer rexec date ;# shows sniffer date/time in case NTP not working on sniffer
    set ring_file_cnt [string trim $ring_file_cnt]
    set ring_file_size [string trim $ring_file_size]
    if {$ring_file_cnt != "" && $ring_file_cnt > 1 && $ring_file_size != "" && $ring_file_cnt > 1} {

        # Start sniffer with ring buffer
        set size_kb [expr $ring_file_size * 1000] ;# convert MB to KB
       	set catch_resp [catch {$sniffer start "tshark -b filesize:$size_kb -b files:$ring_file_cnt -i prism0 -w /root/ring"} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR $name "$catch_msg"
        } else {
            UTF::Message INFO $name "Finished sniffer ring buffer setup, $ring_file_cnt files, $ring_file_size MBytes each"
        }

    } else {
        # Start sniffer with single .pcap file
        # Always use the same filename on the sniffer. This ensures the file will be overwritten
        # the next time the sniffer is started up. This policy maximizes the disk space available
        # for sniffer use.
        set catch_resp [catch {$sniffer start "tshark -i prism0 -w captureFile.pcap"} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR $name "$catch_msg"
        } else {
            UTF::Message INFO $name "Finished sniffer single .pcap startup"
        }
    }
    return
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::stop_sniffer] [arg sniffer] [arg result] [arg SummaryLoc]
    # [opt ring_file_cnt] [opt sniffercapture=null] [opt dir=null] [opt attn=null]]
    # Stops the [arg sniffer] trace. For [arg result] 1, copies the .pcap file to
    # the [arg SummaryLoc] directory. To keep .pcap file on local sniffer disk, set
    # [arg SummaryLoc] to /tmp and the file will be stored as /tmp/<host>.<date>.<time>.<testnum>.N.pcap.
    # For [arg result] 0, ignores the .pcap file and returns null. [arg ring_file_cnt] 
    # indicates if a single .pcap file is expected or a series of ring buffer files.
    # For [arg sniffercapture] description, see UTF::check_sniffer_capture_steps.
    # [arg dir] is RampDown or RampUp. [arg attn] is the attenuator step. [opt force=0]
    # set to 1 to force sniffer off.  Returns one or more web links to the .pcap files.[para]

    # The .pcap files are always left on the [arg sniffer] hard drive. The next time
    # the sniffer is started, any existing ring buffer files will be deleted and the existing
    # fixed name single .pcap file will be deleted.
}

proc UTF::stop_sniffer {sniffer result SummaryLoc {ring_file_cnt ""} {sniffercapture ""}\
    {dir ""} {attn ""} {force 0}} {

    # Sniffer use is optional.
    set name [UTF::get_name $sniffer]
    UTF::Message INFO "$name" "stop_sniffer: sniffer=$sniffer result=$result SummaryLoc=$SummaryLoc\
        ring_file_cnt=$ring_file_cnt sniffercapture=$sniffercapture dir=$dir attn=$attn\
        force=$force"
    if {$sniffer == ""} {
        return
    }

    # User may want the sniffer to start/stop at an arbitrary set of attenuation points
    # to create one or more large .pcap files.
    set sniffercapture [string trim $sniffercapture]
    set sniffercapture [string tolower $sniffercapture]
    if {$sniffercapture != ""} {

        # If there are multiple capture steps, make sure the state variables are initialized.
        # For multiple consecutive runs, these variables should be reset by the calling routine.
        if {![info exists ::sniffer_capture_active]} {
            set ::sniffer_capture_active "" ;# on/off sniffer state for large .pcap file
        }
        if {![info exists ::sniffer_capture_index]} {
            set ::sniffer_capture_index "" ;# current index into list of sniffer commands for large .pcap file(s)
        }

        # If sniffer is already off, we are done.
        if {$::sniffer_capture_active == "off" || $::sniffer_capture_active == ""} {
            return
        }

        # Check dir & attn values.
        set dir [string trim $dir]
        set dir [string tolower $dir]
        if {$dir != "rampdown" && $dir != "rampup" && ![regexp {fastrampup\d+} $dir]} {
            error "stop_sniffer ERROR: invalid dir=$dir, must be RampDown or RampUp or FastRampUpnn!"
        }
        set attn [string trim $attn]
        if {![regexp {^\d+$} $attn] && $force == 0} {
            error "stop_sniffer ERROR: invalid attn=$attn, must be integer!"
        }

        # Format curr_step as: dnnn or fnn or unnn
        if {[regexp {fastrampup(\d+)} $dir - temp]} {
            # For fastrampup, we always use the same attenuator value. To distinguish
            # between these steps, we use the step sequence number. 
            set curr_step "f${temp}"
        } else {
            set temp [string range $dir 4 4]
            set curr_step "${temp}${attn}"
        }

        # Find next stop step index. Stop steps are the odd numbered steps in the list.
        if {$::sniffer_capture_index == ""} {
            set stop_index 1
        } else {
            set stop_index [expr $::sniffer_capture_index + 1]
        }

        # Sanity check start_index is odd number.
        if {[expr $stop_index % 2] != 1 && $force == 0} {
            error "stop_sniffer ERROR: stop_index=$stop_index NOT an odd number,\
                sniffer_capture_index=$::sniffer_capture_index, sniffercapture=$sniffercapture"
        }

        # If we are not at the next stop step, we are done.
        # At end of tests, script may force sniffer off, regardless.
        set next_stop_step [lindex $sniffercapture $stop_index]
        # UTF::Message INFO "$name" "curr_step=$curr_step stop_index=$stop_index next_stop_step=$next_stop_step"
        if {$curr_step != $next_stop_step && $force == 0} {
            return
        }

        # We ARE at the next stop step. Save new state info, as the sniffer will be stopped.
        set ::sniffer_capture_active off
        set ::sniffer_capture_index $stop_index
        UTF::Message INFO "$name" "stop_sniffer: sniffer_capture_active=$::sniffer_capture_active\
            sniffer_capture_index=$::sniffer_capture_index"
    }

    # Stop the sniffer.
    set catch_resp [catch {$sniffer stopall} catch_msg]
    if {$catch_resp != 0} {
        UTF::Message ERROR $name "$catch_msg"
    }

    # If controlchart got an error, or we have a series of start/stop steps,
    # save the sniffer .pcap file. The .pcap files are always left on sniffer
    # PC and overwritten when the sniffer is started up again.
    if {$result == 0 && $sniffercapture == "" && $force == 0} {
        return
    }

    # There can be multiple ring buffer .pcap files.
    UTF::Try "Download sniffer .pcap file" {
        set name [UTF::get_name $sniffer]
        $sniffer rexec date ;# shows sniffer date/time in case NTP not working on sniffer
        if {$ring_file_cnt != "" && $ring_file_cnt > 1} {
            # Multiple ring buffer files.
            set src_file_list [$sniffer rexec ls /root/ring_*]
        } else {
            # Single file.
            set src_file_list "/root/captureFile.pcap"
        }
        UTF::Message INFO "$name" "src_file_list=$src_file_list"
        set errors ""
        set i 0
        set msg "html:"
        set num [UTF::get_testnum]
        set timestamp [clock format [clock seconds] -format "%Y%m%d.%H%M%S"]
        foreach src $src_file_list  {

            # Rename files to consistent unique name.
            incr i
            set dest_file "${name}.${timestamp}.${num}.${i}.pcap"
            set dest "$SummaryLoc/$dest_file"

            # Where we store the file detemines how it is handled.
            if {$SummaryLoc == "/tmp"} {
                # User wants files kept on local sniffer disk.
                # MUST use unix cp command to keep file on local sniffer disk!
                # UTF copyfrom will put file on endpoint PC /tmp directory!
                set msg "html: See $name local $SummaryLoc directory for .pcap files"
                set catch_resp [catch {$sniffer rexec cp $src $dest} catch_msg]

            } else {
                # User wants files stored on big file server disk.
                append msg " <a href=\"$dest_file\">${num}.${i}.pcap</a>"
                set catch_resp [catch {$sniffer copyfrom $src $dest} catch_msg]
            }

            # Check for copy errors.
            if {$catch_resp != 0} {
                append errors "ERROR: $catch_msg\n"
            }

            # Set permissions on big file servers only.
            if {$SummaryLoc != "/tmp"} {
                set catch_resp [catch {file attributes $dest -permissions 00644} catch_msg]
                if {$catch_resp != 0} {
                    append errors "ERROR: $catch_msg\n"
                }
            }
        }

        # Return web links
        if {$errors == ""} {
            return "$msg"
        } else {
            error "$msg $errors"
        }
    }

    # Disk utilization check on sniffer to help user manage locally stored .pcap files.
    UTF::check_disk_usage $sniffer
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::terminate_pid] [arg object] [arg pid] [arg verbose]]
    # Used to terminate a single specific process id.
    # [arg object] host object to terminate existing process id on.
    # [arg pid] integer process id to be terminated.
    # [arg verbose] flag controls logging of pid details. Default=0
    # means no logging. Any other value gives full logging.[para]

    # Returns null when process id has been successfully terminated, 
    # even if it took multiple attempts. Returns null if process was
    # not running when termination was requested. Throws an error if
    # multiple attempts to terminate the process id have failed.
}

proc UTF::terminate_pid {object pid {verbose 0}} {

    # Get object host_os, also validates object is defined in the config file.
    set host_os [check_host_os $object]

    # Check pid is integer
    if {![regexp {^\d+$} $pid]} {
        error "UTF::terminate_pid ERROR: pid=$pid is not integer, object=$object"
    }

    # Set cmd according to host_os
    if {$host_os == "WinXP" || $host_os == "Vista" || $host_os == "Win7"} {
        # Windows / Cygwin
        set cmd "/bin/kill -f"
    } else {
        # Linux, MacOS, etc
        set cmd "kill -9"
    }

    # Try multiple times to terminate the pid, verify its gone.
    for {set i 1} {$i <= $::max_tries} {incr i} {

        # Try to terminate the pid
        if {$verbose != 0} {
            UTF::Message INFO "" "UTF::terminate_pid object=$object terminating pid=$pid try #$i"
        } 
        set catch_resp [catch {$object rexec -quiet -silent $cmd $pid} catch_msg]

        # If the process wasnt running to begin with, log this fact and gracefully exit.
        if {[regexp -nocase {no\s+such\s+process} $catch_msg] ||\
            [regexp -nocase {process.*not.*found} $catch_msg]} {
            UTF::Message WARN "" "UTF::terminate_pid object=$object pid=$pid was not running, try #$i"
            return
        }

        # Verify that the pid is really gone.
        set resp [UTF::check_pid $object $pid $verbose]
        if {$resp == 0} {
            # pid is truly gone.
            if {$i != 1 || $verbose != 0} {
                UTF::Message INFO "" "UTF::terminate_pid object=$object pid=$pid was terminated on try #$i"
            }
            return

        } else {
            # Log the failure data
            UTF::Message ERROR "" "UTF::terminate_pid object=$object pid=$pid try #$i ERROR: $catch_msg"
        } 

        # Wait a bit and try again.
        if {$i < $::max_tries} {
            UTF::Sleep 5 "$::localhost"
        }
    }

    # Process wouldnt go away after multiple tries, throw error.
    error "UTF::terminate_pid ERROR: object=$object pid=$pid\
        would not terminate after $::max_tries tries, $catch_msg"
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::terminate_pid_by_name] [arg object] [arg name]
    # [arg verbose]]
    # Used to terminate one or more process that match the specified
    # name. This is like a shotgun and may hit more than you expected, 
    # so be careful!
    # [arg object] host object to terminate existing process on.
    # [arg name] process name to be terminated. Wildcard case insensitive 
    # matching of the process table contents is used.
    # [arg verbose] flag controls logging of pid details. Default=0
    # means no logging. Any other value gives full logging.[para]

    # Returns null when all process that match (wildcard, case insensitive)
    # the specified name are successfully terminated, even if it took
    # multiple attempts. Returns null if no process matched.
    # Throws an error if multiple attempts to terminate the matching
    # process have all failed.
}

proc UTF::terminate_pid_by_name {object name {verbose 0}} {

    # NB: Linux & Cygwin do have the utility pkill to terminate
    # processes by name. BUT Cygwin's pkill does NOT access any of
    # the tasks started by Windows. So it wont do the job needed
    # needed here! Also, Cygwin pkill doesnt support the -W option
    # like the Cygwin ps command does to access the Windows tasks.

    # Get list matching pids from object.
    set pid_list [UTF::check_pid_by_name $object $name $verbose]
    if {$pid_list == ""} {
        return
    }

    # Terminate the pids one at a time.
    set error_cnt 0
    set error_list ""
    UTF::Message INFO "" "UTF::terminate_pid_by_name: $object terminating pid: $pid_list"
    foreach pid $pid_list {
        # Terminate the pid. Store any errors that occur.
        set catch_resp [catch {UTF::terminate_pid $object $pid $verbose} catch_msg]
        if {$catch_resp != 0} {
            incr error_cnt
            append error_list $catch_msg
        }
    }

    # Return status depends on error_list.
    set  error_list [string trim $error_list]
    if {$error_list == ""} {
        return
    } else {
        error "UTF::terminate_pid_by_name got $error_cnt errors,\
            error_list: $error_list"
    }
}


#====================================================================
UTF::doc {
    # [call [cmd UTF::turn_off_radios] [arg AP_list] [arg STA_list]]

    # For each AP in [arg AP_list], turns off the radio. On error, turns
    # off power to the AP. For each STA in [arg STA_list], disassociates
    # from AP, turns off the radio. On error, turns off power to the STA.

    # Returns the stats string, or throws an error.
}

proc UTF::turn_off_radios {AP_list STA_list} {

    # Initialization
    UTF::Message INFO "" "turn_off_radios: AP_list=$AP_list STA_list=$STA_list"
    set cnt [llength "$AP_list $STA_list"]
    set radios_off 0
    set powered_off 0
    set failed 0
    set failed_list ""

    # Turn off every radio in AP_list.
    foreach AP $AP_list {
        set name [UTF::get_name $AP]
        UTF::Message INFO "$name" "\n\n\nTurn off AP $AP radio\n\n\n"
        set catch_resp [catch {UTF::set_ap_nvram $AP wl0_radio=0 wl1_radio=0} catch_msg]
        if {$catch_resp == 0} {
            incr radios_off
        } else {
            UTF::Message ERROR "$name" "Could not turn off AP $AP radio: $catch_msg"
            # OEM AP will probably go here.
            UTF::Message INFO "$name" "Try to power off AP $AP"
            set catch_resp [catch {$AP power off} catch_msg]
            if {$catch_resp == 0} {
                incr powered_off
            } else {
                incr failed
                lappend failed_list $AP
                UTF::Message FAIL "$name" "Could not power off AP $AP radio: $catch_msg"
            }
        }
    }

    # Turn off every radio in STA_list.
    foreach STA $STA_list {
        set name [UTF::get_name $STA]
        UTF::Message INFO "$name" "\n\n\nDisassoc & turn off STA $STA radio\n\n\n"
        set catch_resp [catch {$STA wl disassoc} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message ERROR "$name" "Could not disassoc STA $STA: $catch_msg"
            # Dont count this as an failure, its WBN / good housekeeping.
            # What really matters is the "wl radio off" below.
        }
        set catch_resp [catch {$STA wl radio off} catch_msg]
        if {$catch_resp == 0} {
            incr radios_off
        } else {
            UTF::Message ERROR "$name" "Could not turn off STA $STA radio: $catch_msg"
            UTF::Message INFO "$name" "Try to power off STA $STA"
            set catch_resp [catch {$STA power_sta off} catch_msg]
            if {$catch_resp == 0} {
                incr powered_off
            } else {
                incr failed
                lappend failed_list $STA
                UTF::Message FAIL "$name" "Could not power off STA $STA radio: $catch_msg"
            }
        }
    }

    # Did we get any failures?
    if {$failed == 0} {
        return "radios_off=$radios_off powered_off=$powered_off failed=$failed"
    } else {
        error "radios_off=$radios_off powered_off=$powered_off failed=$failed failed_list: $failed_list"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::update_connection_list] [arg i] [arg conn_list]]

    # Removes items from the specified connection list if they are no
    # longer in the list of items to be tested. This ensures that we
    # dont continue to test a device that couldnt find a build, or
    # failed to load the build, or could not associate to the AP.[para]

    # i is integer count for log messages. [arg conn_list] is expected
    # to be a space separated list of valid UTF STA object names. The 
    # first item in the list is expected to be a Router STA. The second
    # and subsequent items are expected to be endpoint STAs.[para]

    # Returns the updated conn_list
}

proc UTF::update_connection_list {i conn_list} {

     # Sanity checks
     if {![info exists ::router_sta_list] ||
         ![info exists ::endpoint_sta_list]} {
         error "UTF::update_connection_list ERROR: global variables\
         not set, must run UTF::setup_testbed_info"
     }

     # If conn_list is null, return null.
     set conn_list [string trim $conn_list]
     if {$conn_list == ""} {
         return
     }
     UTF::Message INFO "" "(conn$i)=$conn_list"

     # If the router didnt find a build or load, then return null
     # so it doesnt get used.
     set rtr [lindex $conn_list 0]
     if {[lsearch -exact $::router_sta_list $rtr] < 0} {
         UTF::Message INFO "" "Router $rtr didnt find a build/load/associate\
             properly, ignoring connection list: $conn_list"
         return
    }

    # Remove STAs that didnt find builds or load.
    set sta [lrange $conn_list 1 end ]
    foreach item $sta {
        if {[lsearch -exact $::endpoint_sta_list $item] < 0} {
            set conn_list [UTF::remove_item_exact $conn_list $item] ;# remove STA
            UTF::Message INFO "" "STA $item didnt find a build/load/associate\
                properly, updating connection list: $conn_list"
        }
    }

    # Check connection list has 2 or more items.
    if {[llength $conn_list] >= 2} {
        return $conn_list
    } else {
        UTF::Message INFO "" "List must have 2 items or more, ignoring connection list: $conn_list"
        return
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::update_control_chart_data] [arg cc_keys]
    # [arg cc_samples] [arg readonly] [arg stats_array] [arg fail_criteria]
    # [opt STA=""] [opt LAN=""] [opt direction=""] [opt samplesize=5]
    # [opt history=30] [opt title=Throughput] [opt units=Mbit/sec]
    # [opt yaxis=Throughput]]

    # Updates a control chart specified by [arg cc_keys] using the
    # raw sample data in string [arg cc_samples]. Data will not be
    # saved to disk file if [arg readonly] is 1. Control chart stats
    # results are returned in array [arg stats_array]. Optional items,
    # if present, are used to update the cc_keys, etc.[para]

    # If you want failures reported by throwing an error, then specify fail_criteria 
    # such as LOW|WIDE. If fail_criteria is null, no errors will be thrown.[para]

    # NB: Does NOT run iperf! You supply the raw cc_samples data string.[para]

    # Returns html string with thumbnail image & weblink. Array [arg stats_array]
    # is updated. Large .png pathname is in ::uccd_png_path
}

proc UTF::update_control_chart_data {cc_keys cc_samples readonly stats_array\
    fail_criteria {STA ""} {LAN ""} {direction ""} {samplesize 5} {history 30}\
    {title "Throughput"} {units "Mbit/sec"} {yaxis "Throughput"}} {

    UTF::Message INFO "" "update_control_chart_data: cc_keys=$cc_keys\
        cc_samples=$cc_samples readonly=$readonly stats_array=$stats_array\
        fail_criteria=$fail_criteria STA=$STA LAN=$LAN direction=$direction\
        samplesize=$samplesize history=$history title=$title units=$units yaxis=$yaxis"

    # Initialization
    set ::uccd_png_path ""

    # Make stats_array writable.
    upvar ${stats_array}(samples) samples
    upvar ${stats_array}(mmm) cc_mmm
    upvar ${stats_array} stats

    # Create control chart keys
    set cc_keys [string trim $cc_keys]
    set direction [string trim $direction]
    set direction [string tolower $direction]
    set LAN [string trim $LAN]
    set STA [string trim $STA]
    if {$direction != ""} {
        if {$direction == "upstream"} {
            append cc_keys " \{$STA $LAN\}"
        } else {
            append cc_keys " \{$LAN $STA\}"
        }
    }
    set cc_keys [string trim $cc_keys]

    # If we did this right, we can locate the perfcache data file.
	if {[info exists UTF::Perfcache]} {
        set perfcache_dir $UTF::Perfcache
    } else {
        set perfcache_dir [file dirname "$UTF::Logfile"] ;# remove filename
        set perfcache_dir [file dirname "$perfcache_dir"] ;# remove date/time
        set perfcache_dir "$perfcache_dir/perfcache" ;# add standard subdirectory
    }
    set perfcache_file "$perfcache_dir/$cc_keys.data"
    if {[file exists "$perfcache_file"]} {
        UTF::Message INFO "" "update_control_chart_data: found perfcache_file=$perfcache_file OK"
    } else { 
        UTF::Message WARN "" "update_control_chart_data: perfcache_file=$perfcache_file NOT found!"
        # Let calling routine know about perfcache files not found
        lappend ::perfcache_not_found "$cc_keys\n"
    }

    # Save readonly setting. Create control chart object. 
    set saved_readonly $::UTF::ControlChart::readonly
    if {$readonly == "1"} {
        # NB: UTF::ControlChart::readonly may have already been 1.
        # Dont ever force to 0. User may not want to save any of the results.
        set ::UTF::ControlChart::readonly 1
        set ::UTF::MemChart::readonly 1
    }

    # We dynamically choose the chart object type based on sample size
    if {$samplesize > 1} {
        # We prefer ControlChart with many samples.
        UTF::ControlChart CC -key $cc_keys -allowzero 1 -s $samplesize -history $history -title $title -units $units -ylabel $yaxis
    } else {
        # For RvR rejoin test, possibly other tests, there will be only 1 sample.
        # We create the MemChart with size=2, and it uses rolling average.
        # Not supported: -allowzero 1
        UTF::MemChart CC -key $cc_keys -s 2 -history $history -title $title -units $units -ylabel $yaxis
    }

    # Get mean, min, max from samples
    if {$samplesize > 1} {
        set cc_mmm [UTF::MeanMinMax $cc_samples]
    } else {
        set cc_mmm $cc_samples
    }
    puts "cc_samples=$cc_samples cc_mmm=$cc_mmm"

    # Get control chart results, convert to graph.
    set cc_result [CC addsample $cc_mmm]
    set html_result [CC plotcontrolchart $cc_result]

    # Populate stats_array.
    CC stats stats
    set samples $cc_samples ;# for sake of completeness

    # Save .png path
    set ::uccd_png_path [CC plotfile]

    # Take out the trash.
    CC destroy

    # Restore readonly setting. 
    set ::UTF::ControlChart::readonly $saved_readonly
    set ::UTF::MemChart::readonly $saved_readonly

    # Pass back html string with thumbnail & weblink.
    # Pass/Fail criteria are optional.
    set fail_criteria [string trim $fail_criteria]
    if {$fail_criteria == ""} {
        return $html_result
    } elseif {[regexp -nocase "$fail_criteria" $cc_result]} {
        error $html_result
    } else {
        return $html_result
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::update_control_chart_result] [arg result_code]
    # [arg html_string]]

    # Routine used to customize the control chart throughput Pass
    # criteria. The pass criteria from variable ::cc_pass_criteria
    # is used to update the original control chart result.[para]

    # EG: set ::cc_pass_criteria "OK|LOW|WIDE|HIGH|NARROW"[para]

    # NB: This will NOT ignore ping, iperf & other failures.[para]

    # Returns html string with thumbnail image & weblink. 
}

proc UTF::update_control_chart_result {result_code html_string} {

    # Custom throughput PASS criteria are specified below.
    if {![info exists ::cc_pass_criteria]} {
        set ::cc_pass_criteria "OK" ;# safe default
    }
    set ::cc_pass_criteria [string trim $::cc_pass_criteria]
    UTF::Message INFO "" "update_control_chart_result: result_code=$result_code\
        ::cc_pass_criteria=$::cc_pass_criteria"

    # If there is no pass criteria at all (very dangerous) or the
    # tests passed all the controlchart criteria, we are done.
    if {$::cc_pass_criteria == "" || $result_code == 0} {
        return "$html_string"
    }

    # We had a failure according to the UTF contolchart. This can
    # include ping failures, insufficient iperf samples, ZERO, WIDE,
    # LOW, HIGH or NARROW thruput. We want to ignore just the items in
    # the ::cc_pass_criteria, and continue to fail on everything else,
    # such as ping failed & no route to host.
    # Look for "result </a>" at end of html string. Start by getting
    # the last few charactars of html_string.
    if {![regexp {(.{16})$} $html_string - end_str]} {
        set end_str ""
    }
    UTF::Message INFO "" "update_control_chart_result: end_str=$end_str"
    if {[regexp -nocase {</a>\s*$} $end_str]} {
        # Found </a>, now look for ::cc_pass_criteria.
        if {[regexp -nocase $::cc_pass_criteria $end_str]} {
            # cc_pass_criteria are met. Return shows up as PASS on the web page.
            return "$html_string"
        } else {
            # Throwing error shows up as a FAIL on the web page.
            error "$html_string"
        }

    } else {
        error "$html_string"
    }
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::update_report_add] [arg summaryinfo] [arg STA] [arg image]
    # [arg tag] [arg date] [arg id] [arg notes] [arg status]]

    # Routine updates the report [arg summaryinfo] html fragment by adding
    # a new row at the end of the html table with the other argument data.[para]

    # Returns: updates report object and returns updated [arg summaryinfo]
}

proc UTF::update_report_add {summaryinfo STA image tag date id notes status} {

    # Look for first </table> tag, add new table row.
    set done 0
    set summaryinfo [split $summaryinfo "\n"]
    set result ""
    foreach line $summaryinfo {
        if {[regexp {</table>} $line] && $done == 0} {
            # New row starts with STA name, and 2 placeholders for the
            # hw & OS versions, which get added in later on.
            set new_row "   <tr><td>$STA</td><td>&nbsp;</td><td>&nbsp;</td>"
            foreach item [list $image $tag $date $id $notes $status] {
                set item [string trim $item]
                if {$item == ""} {
                    set item "&nbsp;"
                }
                append new_row "<td>$item</td>"
            }
            append new_row "</tr>"
            append result "$new_row\n"
            set done 1
        }
        # Always keep the line, never drop any data from the summary table.
        append result "$line\n"
    }

    # Update report object with updated table, return updated table.
    # puts "\n\n\nresult=$result\n\n\n"
    $UTF::Summary configure -info "$result"
    return $result

}

#====================================================================
UTF::doc {
    # [call [cmd UTF::update_report_hwinfo] [arg summaryinfo]]

    # Routine updates the report [arg summaryinfo] html fragment with
    # the correct OS info & hardware version once devices all have
    # drivers loaded.[para]

    # Returns: updates report object and returns updated [arg summaryinfo]
}

proc UTF::update_report_hwinfo {summaryinfo} {

    # NB: BlueTooth hardware is not updated, as it will not provide
    # any hardware info.

    # Look for STA names in first table column of each line.
    # Update with OS & hw info.
    set summaryinfo [split $summaryinfo "\n"]
    set result ""
    foreach line $summaryinfo {
        set line [UTF::update_report_line $line]
        append result "$line\n"
    }

    # Update report object with updated table, return updated table.
    UTF::Message INFO ""  "\n\n\nupdate_report_hwinfo: result=$result\n\n\n"
    $UTF::Summary configure -info "$result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::update_report_line] [arg line]]

    # Routine updates [arg line] of html with
    # the correct OS info & STA hardware version & country code. [para]

    # Returns: updated [arg line]
}

proc UTF::update_report_line {line} {

    # Extract STA name from line. There are header lines in the summary table
    # that dont contain STA, so its not an error if we dont find a STA.
    # Also, if there arent 2 non-breaking spaces, there is nothing to do.
    # This can occur if a line was updated previously.
    UTF::Message INFO "" "update_report_line: line before: $line"
    if {![regexp {<tr><td>([^<]+)</td><td>&nbsp;</td><td>&nbsp;</td>} $line - STA]} {
        # Always return the line, never drop any data from the summary table.
        return $line
    }
    set STA [string trim $STA]

    # Get host OS. This also checks for valid UTF objects.
    set name [UTF::get_name $STA]
    set catch_resp [catch {set os_ver [UTF::check_host_os $STA]} catch_msg]
    if {$catch_resp != 0} {
        # Always return the line, never drop any data from the summary table.
        UTF::Message ERROR $name "update_report_line: $STA could not get OS: $catch_msg"
        return $line
    }
    set os_ver [string trim $os_ver]

    # Get STA type.
    set catch_resp [catch {set type [UTF::check_host_type $STA]} catch_msg]
    if {$catch_resp != 0} {
        # Always return the line, never drop any data from the summary table.
        UTF::Message ERROR $name "update_report_line: $STA could not get type: $catch_msg"
        return $line
    }

    # BlueTooth items dont provide hardware info.
    if {$type == "WinBT"} {
        # For WinBT objects, add BT & OS to columns to right of STA name.
        UTF::Message INFO $name "update_report_line: STA=$STA os_ver=$os_ver type=$type"
        regsub "&nbsp;" $line "BT" line
        if {$os_ver != ""} {
            regsub "&nbsp;" $line "$os_ver" line
        }
        UTF::Message INFO "" "update_report_line: line after: $line"
        return $line
    }

    # Get hardware info. UTF will often have this data cached.
    set catch_resp [catch {set hw_ver [$STA whatami -notype]} catch_msg]
    if {$catch_resp != 0} {
        # Always return the line, never drop any data from the summary table.
        UTF::Message ERROR $name "update_report_line: $STA could not get hw_ver: $catch_msg"
        return $line
    }

    # Get country code.
    set country ""
    set catch_resp [catch {set country [$STA wl country]} catch_msg]
    UTF::Message INFO $name "update_report_line: STA=$STA os_ver=$os_ver type=$type hw_ver=$hw_ver country=$country"
    set country [lrange $country 0 1] ;# keep 2 words only.

    # Add hw_ver & OS to columns to right of STA name.
    regsub "&nbsp;" $line "$hw_ver" line
    if {$os_ver != ""} {
        regsub "&nbsp;" $line "$os_ver" line
    }
    UTF::Message INFO "" "update_report_line: line after: $line"
    return $line
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::update_report_testtime] [arg summaryinfo]]

    # Routine updates the report [arg summaryinfo] html fragment with
    # the correct test end time when tests are complete.[para]

    # Returns: updates report object and returns updated [arg summaryinfo]
}

proc UTF::update_report_testtime {summaryinfo} {

    # Look for "Tests started: ... &nbsp;" in each line.
    set summaryinfo [split $summaryinfo "\n"]
    set result ""
    foreach line $summaryinfo {
        if {[regexp {Tests started.*&nbsp;} $line]} {
            # Replace &nbsp; with current time.
            set now_sec [clock seconds]
            set time [clock format $now_sec -format "%T"]
            regsub "&nbsp;" $line "ended $time" line
        }
        # Always save the line, never drop any data from the summary table.
        append result "$line\n"
    }

    # Update report object with updated table, return updated table.
    # puts "\n\n\nresult=$result\n\n\n"
    $UTF::Summary configure -info "$result"
    return $result
}

#====================================================================
UTF::doc {
    # [call [cmd UTF::update_report_title] [arg args]]

    # Routine updates the report title with actual build TAGs as the
    # STA drivers are loaded. This eliminate the "??" in the 
    # build info in the report title. [arg args] is list of STA to 
    # check and get updated info from. Returns: null
}

proc UTF::update_report_title {args} {

    # Get current report title
    set title [$UTF::Summary cget -title]
    # set title "test script (5_22_??, 5_100_? 5_90_6)abcd"
    # UTF::Message INFO "" "before title: $title"

    # We need to extract any tag numbers that are already available in the title.
    # That way we dont lose data from previous runs.
    if {![regexp {\(([^\)]*)\)\s*$} $title - temp]} {
        set temp ""
        append title " ()" ;# add place for tag numbers
    }
    regsub -all "," $temp "" temp ;# commas will be reinserted later
    set tagnum_list ""
    foreach item $temp {
        # Ignore items with ? or * or unknown
        if {[regexp {\?|\*|unknown} $item]} {
            # puts "dropping: $item"
        } else {
            # puts "keeping: $item"
            lappend tagnum_list $item
        }
    }

    # Find proper TAG info for each STA being tested.
    foreach STA $args {

        # Get image fullpath from STA object
        set catch_resp [catch {set image [$STA cget -image]} catch_msg]
        if {$catch_resp != 0} {
            UTF::Message WARN "" "update_report_title: $STA cget -image: $catch_msg"
            continue
        } 

        # Extract tag numbers from image path, eg: 5_100_98_39
        regsub -nocase {/.*/build_[^/]+/} $image "" tag ;# remove leading path items
        regsub -all -nocase {PRESERVED/|ARCHIVED/} $tag "" tag ;# handle older builds
        regsub {/.*} $tag "" tag ;# remove trailing items
        regsub {[^_]+_[^_]+_} $tag "" tagnum ;# remove first 2 tokens, eg: KIRIN_REL_
        if {$tagnum == "NIGHTLY"} {
            set tagnum TOT
        }
        UTF::Message INFO "" "update_report_title: STA=$STA tagnum=$tagnum tag=$tag image=$image"
        if {$tagnum != ""} {
            lappend tagnum_list $tagnum
        }
    }

    # Update title string.
    set tagnum_list [lsort -unique $tagnum_list]
    regsub -all " " $tagnum_list ", " tagnum_list
    regsub {\(([^\)]*)\)\s*$} $title "\($tagnum_list\)" title

    # Update report object with new title.
    # UTF::Message INFO "" "after title: $title"
    $UTF::Summary configure -title "$title"
    return
}

#====================================================================
UTF::doc {
    # [list_end]
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

