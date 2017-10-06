#!/bin/env utf
#
# Stressor object for Olympic System Tesing
# Author Robert McMahon April 2015
#
#
package require snit
package require UTF
package require UTF::Streams
package require UTF::Test::ConnectAPSTA
package require UTF::Test::APChanspec
package require md5

package provide UTF::Stressors 2.0

snit::type UTF::Stressor::Traffic {
    typevariable EXCEPTATTRIBUTES "-name"

    option -ap -validatemethod __validateap
    option -sta -validatemethod __validatesta
    option -ssid -default -1
    option -chanspec -default -1
    option -attn -default -1
    option -name -default {}
    option -protocol -default "TCP"
    option -rate -default -1
    option -nosetup -default 0 -type boolean
    option -verbose -default 0 -type boolean
    option -debug -default 0 -type boolean
    option -type -default -1

    variable mystate
    variable myid
    variable utfmsgtag
    variable errorcode

    component mystream -inherit true
    constructor {args} {
	set errorstate ""
	if {$options(-name) eq ""} {
	    set utfmsgtag "[namespace tail $self]"
	    set options(-name) $utfmsgtag
	} else {
	    set utfmsgtag "[namespace tail $self]"
	}
	set options(-protocol) [string toupper $options(-protocol)]
	$self configurelist $args
	set mystream [UTF::stream %AUTO% -tx $options(-sta) -rx $options(-ap) -protocol $options(-protocol) -rate $options(-rate) -reportinterval 0.01]
	$mystream configure -name "${options(-name)}-[namespace tail $mystream]"
	$mystream id
	set mystate "CONFIGURED"
    }
    destructor {
	catch {$mystream destroy}
    }
    method inspect {} {
	puts -nonewline "\nMy State is:"
	puts "\t$mystate"
	if {[UTF::stream info instances $mystream] eq $mystream} {
	    puts -nonewline "Stream is:"
	    puts "\t[$mystream cget -name]\t$mystream"
	}
    }
    method id {args} {
	set key $args
	foreach n [lsort [array names options]] {
	    if {[lsearch -exact $EXCEPTATTRIBUTES $n] == -1} {
		lappend key $n $options($n)
	    }
	}
	regsub -all {[/<>]} $key "." key
	set myid [::md5::md5 -hex $key]
	return $myid
    }
    method whatami {} {
	return "UTF::Stressor::Traffic"
    }
    method __validatesta {option value} {
	if {$option eq "-sta" && [llength $value] != 1} {
	    error $utfmsgtag "STA option requires a single value"
	}
	if {[$value hostis != Linux] && [$value hostis != MacOS]} {
	    error $utfmsgtag "STA needs to be Linux or MacOS"
	}
    }
    method __validateap {option value} {
	if {$option eq "-ap" && [llength $value] != 1} {
	    error $utfmsgtag "AP option requires a single value"
	}
	if {[$value hostis != Router] && [$value hostis != Linux]} {
	    error $utfmsgtag "AP needs to be Router or SoftAP (Linux)"
	}
	if {[$value lan] eq {}} {
	    error $utfmsgtag "AP needs a lan device"
	}
	if {[$value lan hostis != Linux] && [$value lan hostis != MacOS]} {
	    error $utfmsgtag "LAN host needs to be Linux or MacOS"
	}
    }
    method setup {} {
	if {$options(-chanspec) ne "-1"} {
	    UTF::Test::APChanspec $options(-ap) $options(-chanspec)
	}
	if {$options(-ssid) ne "-1"} {
	    $options(-ap) wl ssid $options(-ssid)
	}
	if {$options(-chanspec) ne "-1"} {
	    UTF::Test::APChanspec $options(-ap) $options(-chanspec)
	}
	if {[catch {$options(-sta) wl bssid} bssid] ||  $bssid ne [$options(-ap) wl bssid] } {
	    UTF::Test::ConnectAPSTA $options(-ap) $options(-sta)
	}
	if {[catch {$options(-sta) wl bssid} bssid] ||  $bssid ne [$options(-ap) wl bssid] } {
	    error "Failed setup connect apsta"
	}
	set mystate "SETUPCOMPLETE"
    }
    method start {} {
	if {$mystate eq "RUNNING"} {
	    return
	}
	if {$mystate eq "CONFIGURED"} {
	    if {!$options(-nosetup)} {
		$self setup
	    } else {
		set mystate "SETUPCOMPLETE"
	    }
	}
	if {$mystate eq "SETUPCOMPLETE"} {
	    if {![catch {$mystream start}]} {
		set mystate "RUNNING"
		UTF::Message INFO $utfmsgtag "Stressor succesfully started"
	    } else {
		UTF::Message WARN $utfmsgtag "Stressor failed start"
		set errorcode "START FAILED"
	    }
	}
    }
    method stop {} {
	if {$mystate ne "RUNNING"} {
	    return
	}
	if {[catch {$mystream stop} err]} {
	    UTF::Message ERROR $utfmsgtag $err
	} else {
	    set mystate "STOPPED"
	}
    }
    method status {} {
	if {$options(-protocol) eq "UDP"} {
	    $self linkcheck -now -txstrict
	} else {
	    $self linkcheck -now
	}
    }
}

