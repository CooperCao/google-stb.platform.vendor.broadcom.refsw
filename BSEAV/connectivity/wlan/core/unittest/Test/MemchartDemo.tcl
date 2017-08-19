#!/bin/env utf
# -*-tcl-*-
#
# Script to demo mem chart usage
# Written by: Robert J. McMahon September 2011
# $Copyright Broadcom Corporation$
#
package require UTF
package require UTF::MemChart
package require UTF::ControlChart

set history 300

#
# Must pass
#   -key
#   -values (list)
# Options
#   -title
#   -units
#   -ylabel
#   -format
#   -nsigma
#
proc ccwrapper {args} {
    UTF::Getopts {
	{key.arg ""}
	{title.arg ""}
	{values.arg ""}
	{units.arg " "}
	{history.arg "30"}
	{format.arg {%.2f}}
	{nsigma.arg "3.0"}
	{perfcache.arg ""}
    }
    # Determine where to store persistent values
    if {$(perfcache) eq ""} {
       set (perfcache) [file join c: pts perfcache]
    }
    if {![file exists $(perfcache)] || ![file isdirectory $(perfcache)]} {
	error "Invalid directory $(perfcache)"
    }
    set l [llength $(values)]
    puts $l
    if {[expr {$l > 1}]} {
	set (values) [UTF::MeanMinMax $(values)]
	UTF::ControlChart cc -key $(key) -history $(history) -perfcache $(perfcache)  -title $(title) -units $(units) -s $l -history $(history) -format $(format) -nsigma $(nsigma)
    } elseif {[expr {$l == 1}]} {
	UTF::MemChart cc -key $(key) -history $(history) -perfcache $(perfcache)  -title $(title) -units $(units) -s 2 -history $(history) -format $(format) -nsigma $(nsigma)
    } else {
	error "no samples given"
    }
    set l [llength $(values)]
    set results [cc addsample $(values)]
    set html [cc plotcontrolchart "$results"]
    cc destroy
    if {[regexp {(HIGH|LOW|WIDE|ZERO)} $results - match]} {
	error $html
    } else {
	return $html
    }
}

proc create {{key "this_is_my_key"}} {
    global history
    # Key generation.  Keys > 200 chars will be hashed (via md5)
    set cckey $key
    UTF::MemChart cc -key $cckey -history $history -perfcache "." -title "MemChart Demo" -units "scores"
}

proc populate {mean {count ""}} {
    global history
    if {$count eq ""} {
	set count $history
    }
    for {set ix 0} {$ix < $count} {incr ix} {
	set datapoint [RandomNormal $mean 3]
	set boundsresults [cc addsample $datapoint]
    }
}

proc plot {{msg ""}} {
    set htmllink [cc plotcontrolchart $msg]
}

proc cctest {value} {
    set testresults [cc boundcheck $value]
    if {[regexp {(HIGH|LOW|WIDE|ZERO)} $testresults - match]} {
	UTF::Message ERROR+Red "" $match
    } else {
	UTF::Message PASS+Green "" "OK"
    }
}

# generate a random number that is normally distibuted
# using the central limit theorem
#
# mean -> expected mean of the generated numbers
# sd -> expected standard deviation of the generated numbers
proc RandomNormal {mean sd} {
    
    # number of iterations (the higher, the better, the slower):
    set n 150
    # produce n equally random integers from [0,1]
    set sum 0
    for {set i 0} {$i < $n} {incr i} {
	set sum [expr {$sum + int(rand()*2)}]
    }
    # transform the sum to the interval [0,1] again -> sum/n
    # and then transform to [mean,sd^2]
    #
    return [expr {((($sum/double($n))-0.5) * sqrt($n*12)) * $sd + $mean}]
}
