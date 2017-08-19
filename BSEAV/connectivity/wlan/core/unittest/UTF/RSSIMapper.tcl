#!/bin/env utf
#
# Given a list of STAs build an RSSI map
#
# $Id: 6c25a68d4b872a443c7411146417ca519f8f04ea $
# $Copyright Broadcom Corporation$
#
package require snit
package require UTF

package provide UTF::RSSIMapper 2.0

snit::type UTF::RSSIMapper {
    typevariable DEFAULT_CHANSPEC 3
    option -stas
    option -scaninterval -default 10
    option -scanresultsdelay -default 6
    option -scantype -default "passive"
    option -verbose -type boolean -default 0
    option -debug -type boolean -default 0
    option -channel -default ""
    option -changemessages -type boolean -default 0
    option -errorchecking -type boolean -default 1
    option -dbtolerance -default 5
    option -txant -default {}
    option -rxant -default {}

    variable everyids -array {}
    variable afterids -array {}
    variable scanresults -array {}
    variable previousmap -array {}
    variable currentmap -array {}
    variable othernets -array {}
    variable entrytime -array {}
    variable previousothernets -array {}
    variable wlwatch -array {}
    variable IBSS_STR "__rssimapper__"
    variable IBSS_STR_LEN
    variable runningstate 0
    variable txantvalue 3
    variable operatingchannel -array {}

    constructor {args} {
	$self configurelist $args
	set IBSS_STR_LEN [string length $IBSS_STR]
    }

    destructor {
	$self stop
    }
    method inspect {args} {
	UTF::Getopts {
	    verbose ""
	}

	if {[array exists currentmap]} {
	    parray currentmap
	}
	if {[array exists othernets]} {
	    parray othernets
	}
	if {$(verbose)} {
	    if {[array exists entrytime]} {
		parray entrytime
	    }
	    if {[array exists everyids]} {
		parray everyids
	    }
	    if {[array exists afterids]} {
		parray afterids
	    }
	}
    }
    method start {} {
	if {[array exists currentmap]} {
	    unset currentmap
	}
	if {[array exists previousmap]} {
	    unset previousmap
	}
	if {[array exists othernets]} {
	    unset othernets
	}
	if {[array exists entrytime]} {
	    unset entrytime
	}
	if {$options(-stas) == "all"} {
	    set tmp [UTF::STA info instances]
	    foreach sta $tmp {
		lappend stalist [namespace tail $sta]
	    }
	} elseif {$options(-stas) == {}} {
	    UTF::Message WARN $self "No stas configured. Start request ignored."
	    return
	} else {
	    set stalist $options(-stas)
	}
	UTF::Message INFO $self "starting for $stalist"
	foreach sta $stalist {
	    if {[catch {$sta wl band auto} err]} {
		UTF::Message ERROR $self "wl band auto failed for $sta with: $err"
		set stalist [lsearch -all -not -inline $stalist $sta]
		continue
	    }
	    if {[catch {$sta wl up} err]} {
		UTF::Message ERROR $self "wl up failed for $sta with: $err"
		set stalist [lsearch -all -not -inline $stalist $sta]
		continue
	    }
	    if {![info exists operatingchannel($sta)] || $options(-channel) != $operatingchannel($sta)} {
		if {$options(-channel) != ""} {
		    set chan $options(-channel)
		} else {
		    set chan $DEFAULT_CHANSPEC
		}
		if {[$sta hostis Router]} {
		    package require UTF::Test::APChanspec
		    UTF::Test::APChanspec $sta $chan
		} elseif {[$sta hostis Linux]} {
		    $sta wl chanspec $chan
		    $sta wl join $IBSS_STR$sta imode ibss
		} else {
		    $sta wl chanspec $chan
		    $sta wl wsec 0
		    $sta wl join $IBSS_STR$sta imode ibss amode open
		}
		set operatingchannel($sta) $chan
	    }
	    $sta wl ssid $IBSS_STR$sta
	}
	foreach sta $stalist {
	    set everyids($sta) [UTF::Every $options(-scaninterval) [mymethod __scanner $sta]]
	}
	if {[$sta hostis Router]} {
	    UTF::Message DEBUG+red $self  "RJM FIX THIS"
	} else {
	    if {$options(-txant) == ""} {
		set txantvalue 3
	    } else {
		set txantvalue $options(-txant)
	    }
	    foreach sta $stalist {
		$sta wl txant $txantvalue
	    }
	    if {$options(-rxant) == ""} {
		set rxantvalue 3
	    } else {
		set rxantvalue $options(-rxant)
	    }
	    foreach sta $stalist {
		$sta wl antdiv $rxantvalue
	    }
	}
	set runningstate 1
    }
    method stop {} {
	set runningstate 0
	if {[array exists everyids]} {
	    foreach eid [array names everyids] {
		# UTF::Message DEBUG $self "canceling $eid"
		UTF::Every cancel $everyids($eid)
	    }
	    unset everyids
	}
	if {[array exists afterids]} {
	    foreach aid [array names afterids] {
		# UTF::Message DEBUG $self "canceling $afterids($aid)"
		after cancel $afterids($aid)
	    }
	    unset afterids
	}
    }
    method __collapse_table {} {
	set indices [array names currentmap]
	set results {}
	array set searchmap [array get currentmap]
	foreach indice [array names currentmap] {
	    set tmp [split $indice ,]
	    if {[llength $tmp] == 2} {
		set sta_a [lindex $tmp 0]
		set sta_b [lindex $tmp 1]
		if {[info exists searchmap($sta_a,$sta_b)] && [info exists searchmap($sta_b,$sta_a)]} {
		    unset searchmap($sta_b,$sta_a)
		}
	    } else {
		unset searchmap($indice)
	    }
	}
	foreach indice [array names searchmap] {
	    set tmp [split $indice ,]
	    set sta_a [lindex $tmp 0]
	    set sta_b [lindex $tmp 1]
	    set a $currentmap($sta_a,$sta_b)
	    if {!$currentmap($sta_a,$sta_b,active)} {
		append a *
	    }
	    if {[info exists currentmap($sta_b,$sta_a)]} {
		set b $currentmap($sta_b,$sta_a)
		if {!$currentmap($sta_b,$sta_a,active)} {
		    append b *
		}
	    } else {
		set b {}
	    }
	    lappend results $indice "$a/$b"
	}
	return $results
    }
    method lookup {args} {
	UTF::Getopts {
	    sta.arg ""
	}
	if {$(sta) == {}} {
	    array set results [$self __collapse_table]
	    # parray results
	    foreach indice [array names results] {
		UTF::Message INFO $self "map($indice) = $results($indice)"
	    }
	} elseif {[llength $(sta)] == 1} {
	} elseif {[llength $(sta)] == 2} {
	    set sta_a [lindex $(sta) 0]
	    set sta_b [lindex $(sta) 1]
	    set a {} ; set b {}
	    if {[info exists currentmap($sta_a,$sta_b)]} {
		set a $currentmap($sta_a,$sta_b)
		if {!$currentmap($sta_a,$sta_b,active)} {
		    append a *
		}
	    }
	    if {[info exists currentmap($sta_b,$sta_a)]} {
		set b $currentmap($sta_b,$sta_a)
		if {!$currentmap($sta_b,$sta_a,active)} {
		    append b *
		}
	    }
	    if {$a != {}  || $b != {}} {
		return $a/$b
	    } else {
		return
	    }
	} else {
	    error "invalid sta list of $(sta)"
	}
    }
    method __scanner {sta} {
	if {[info exists afterids($sta)]} {
	    return
	}
	set wlwatch($sta) 0
	set watchid($sta) [after 10000 [list set [myvar wlwatch($sta)] 1]]
	if {$options(-channel) != ""} {
	    set scancmd "$sta wl scan -c $options(-channel)"
	} else {
	    set scancmd "$sta wl scan -t passive"
	}
	while {[catch {eval $scancmd}] && !$wlwatch($sta)} {
	    UTF::Sleep 1.0
	}
	if {!$wlwatch($sta)} {
	    after cancel $watchid($sta)
	    # Use flag to ignore latent every cancel
	    if {$runningstate} {
		set afterids($sta) [after [expr {$options(-scanresultsdelay) * 1000}] [mymethod __pollscanresults $sta]]
	    }
	}
    }
    method __pollscanresults {sta} {
	if {$options(-verbose)} {
	    set cmd "$sta wl scanresults"
	} else {
	    set cmd "$sta wl -silent scanresults"
	}
	if {$options(-channel) != ""} {
	    append cmd " -c $options(-channel)"
	}
	set wlwatch($sta) 0
	set watchid($sta) [after 10000 [list set [myvar wlwatch($sta)] 1]]
	while {[catch {eval $cmd} output] && !$wlwatch($sta)} {
	    UTF::Sleep 1.0
	}
	if {$wlwatch($sta)} {
	    UTF::Message WARN $self "Timeout on polling scanresults for $sta"
	} else {
	    after cancel $watchid($sta)
	    $self __parsescanresults $sta $output
	}
	unset afterids($sta)
    }
    method __parsescanresults {sta buf} {
	#    UTF::Message DEBUG+blue $sta $buf
	set mode "PARSE_SSID"
	set entrytime($sta) [clock seconds]
	foreach line [split $buf "\n"] {
	    switch -exact $mode {
		"PARSE_SSID" {
		    if {[regexp {^SSID: "(\w+)"} $line - ssid]} {
			if {[string first $IBSS_STR $ssid] == 0} {
			    set peersta [string range $ssid $IBSS_STR_LEN end]
			    set currentmap($sta,$peersta) "rssi parse err"
			    set mode "PARSE_MAPPER_RSSI"
			} else {
			    set othernets($sta) $ssid
			    set mode "PARSE_OTHER_RSSI"
			}
		    }
		}
		"PARSE_MAPPER_RSSI" {
		    if {[regexp {Mode: (Ad Hoc|Managed)\s+RSSI: (-)?(\d+) dBm.+Flags: (FromBcn|RSSI on-channel)?} $line - mode sign rssi flags]} {
			# UTF::Message DEBUG $self "flags: = $flags"
			if {($options(-channel) == "" && $flags == "FromBcn") || ($options(-channel) != "" && $flags == "RSSI on-channel")} {
			    set currentmap($sta,$peersta) "$sign$rssi"
			    set currentmap($sta,$peersta,time) "$entrytime($sta)"
			    if {$options(-errorchecking)} {
				$self __rssi_symmetry_check $sta $peersta
			    }
			}
		    }
		    set mode "PARSE_SSID"
		}
		"PARSE_OTHER_RSSI" {
		    if {[regexp {Mode: (Ad Hoc|Managed)\s+RSSI: (-)?(\d+) dBm} $line - mode sign rssi]} {
			lappend othernets($sta) "$sign$rssi"
			lappend othernets($sta) $entrytime($sta)
		    }
		    set mode "PARSE_SSID"
		}
		default {
		    error "program error"
		}
	    }
	}
	foreach index [array names currentmap] {
	    set lkey [split $index ,]
	    if {[lindex $lkey 0] == $sta && [lindex $lkey end] == "time"} {
		set key [lrange $lkey 0 end-1]
		if {$currentmap($index) == $entrytime($sta)} {
		    set currentmap([join $key ,],active) 1
		} else {
		    set currentmap([join $key ,],active) 0
		}
	    }
	}
    }
    method __rssi_symmetry_check {sta_a sta_b} {
	if {[info exists currentmap($sta_a,$sta_b)] && [info exists currentmap($sta_b,$sta_a)]} {
	    set a $currentmap($sta_a,$sta_b)
	    set b $currentmap($sta_b,$sta_a)
	    if {![string is integer $a] || ![string is integer $b]} {
		UTF::Message WARN $self "symmetry check called with non integer: $a or $b"
		return
	    }
	    if {[expr {abs($a - $b)}] > $options(-dbtolerance)} {
		UTF::Message WARN+red "$sta_a" "Symmetry mismatch for ${sta_a}/${sta_b} ${a}/${b}"
	    } elseif {$options(-verbose)} {
		UTF::Message WARN+green "$sta_a" "Symmetry match for ${sta_a}/${sta_b} ${a}/${b}"
	    }
	}
    }
}