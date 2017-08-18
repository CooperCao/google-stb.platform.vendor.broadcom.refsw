#!/bin/env utf
#
# Routines to support numerical analysis of data sets.
# Written mostly to support UTF::streams
#
# Author: Robert McMahon
# Date: 1/4/2013
#
# $Id: 10089f3f51146951df9760c6d2f2b4c1d818ab86 $
# $Copyright Broadcom Corporation$
#
package require UTF
package require math
package require math::statistics
package require math::fourier
package require math::complexnumbers
package require math::linearalgebra
package require snit

package provide UTF::math 2.0

# Object based moving average, supports running moving average
snit::type UTF::math::obj::moving_avg {
    option -taps -type integer -default 5
    variable vals
    variable idx
    constructor {args} {
	set vals {}
	$self configurelist $args
    }
    method _val {x} {
	lappend vals [expr {1.0 * $x}]
	set idx end-[expr {$options(-taps) - 1}]
	set vals [lrange $vals $idx end]
	set res 0
	foreach v $vals {
	    set res [expr {$res + $v}]
	}
	set res [expr {$res / [llength $vals]}]
	return $res
    }
    method val {args} {
	foreach x $args {
	    lappend r [$self _val $x]
	}
	return $r
    }
}

snit::type UTF::math::covar_matrix {
    constructor {args} {
	$self configurelist $args
    }
}
namespace eval UTF::math {
    proc modified_moving_average {values} {
	# MAt = MAt-1 + (1/n * (Pt - (MAt-1)))
	set mma 0.0
	set mmacount 0
	foreach value $values {
	    incr mmacount +1
	    if {![info exists mmaprev]} {
		set mmaprev [expr {$value * 1.0}]
		continue
	    }
	    set mma [expr {$mmaprev + (($value - $mmaprev) / $mmacount)}]
	    set mmaprev $mma
	}
	return $mma
    }
    proc outliers {samples {multiplier 3}} {
	set outliers {}
	set samples [lsort -increasing -real $samples]
	set cnt [llength $samples]
	if {$cnt < 4} {
	    error "insufficient samples : $samples"
	}
	set ix1q [expr {int(floor($cnt/4.0))}]
	set LQ [lindex $samples $ix1q]
	set UQ [lindex $samples end-$ix1q]
	set EIQR [expr {$multiplier * [expr {$UQ - $LQ}]}]
	set lowerfence [expr {$LQ - $EIQR}]
	set upperfence [expr {$UQ + $EIQR}]
	UTF::Message STATS "" "EIQR=$EIQR LF=$lowerfence UF=$upperfence  \[samples: $samples\]"
	foreach s [lsort [lrange $samples 0 $ix1q]] {
	    if {$s < $lowerfence} {
		lappend outliers $s
	    } else {
		break
	    }
	}
	foreach s [lrange $samples end-$ix1q end] {
	    if {$s > $upperfence} {
		lappend outliers $s
	    }
	}
	return $outliers
    }
}
#
# Filters to be applied to lists of data
#
namespace eval UTF::math::filters {
    # basic moving average calculation returns a list
    proc moving_average {args} {
	UTF::Getopts {
	    {values.arg "" "list of real number values"}
	    {taps.arg "5" "number of samples used for the moving average"}
	}
	set ma [::UTF::math::obj::moving_avg %AUTO% -taps $(taps)]
	set res [eval [concat $ma val [lrange $(values) 0 end]]]
	$ma destroy
	return $res
    }
    #
    # Fourier based implementation - FIR moving average
    # Reference: Understanding Digital Signal Processing by Richard Lyons
    #
    proc fir_moving_average {args} {
	UTF::Getopts {
	    {values.arg "" "list of real number values"}
	    {count.arg "5" "number of samples used for the moving average"}
	}
	set $(values) [lrange $(values) 0 end]
	set size [llength $(values)]
	# FIR filter
	for {set ix 0} {$ix < $(count)} {incr ix} {
	    lappend h [list [expr {1.0 / $(count)}] 0.0]
	}
	for {set ix $(count)} {$ix < $size} {incr ix} {
	    lappend h [list 0.0 0.0]
	}
	# Work in the frequency domain
	set ht [math::fourier::dft $h]
	set in_t [math::fourier::dft $(values)]
	set ix 0
	foreach z1 $in_t {
	    set z2 [lindex $ht $ix]
	    incr ix
	    lappend r_t [math::complexnumbers::* $z1 $z2]
	}
	# Convert back to time domain and real numbers
	set r [math::fourier::inverse_dft $r_t]
	foreach z1 $r {
	    lappend res [math::complexnumbers::mod $z1]
	}
	return $res
    }

    #  Low pass filter per TCL's math::fourier package
    #  Filters out high frequencies in a weighted manner
    #  Code looks like (args: in_data cutoff)
    #    utf> namespace eval math::fourier info body lowpass
    #    package require math::complexnumbers
    #    set res    [list]
    #    set cutoff [list $cutoff 0.0]
    #    set f      0.0
    #    foreach a $in_data {
    #       set an [::math::complexnumbers::/ $a  [::math::complexnumbers::+ {1.0 0.0}  [::math::complexnumbers::/ [list 0.0 $f] $cutoff]]]
    #       lappend res $an
    #       set f [expr {$f+1.0}]
    #    }
    #    return $res
    proc tcl_lowpass {args} {
	UTF::Getopts {
	    {values.arg "" "list of values"}
	    {cutoff.arg "10" "cut-off frequency"}
	    {complex "return the list as complex numbers, else Real"}
	}
	set c [math::fourier::inverse_dft [math::fourier::lowpass $(cutoff) [math::fourier::dft $(values)]]]
	if {!$(complex)} {
	    foreach n $c {
		lappend res [math::complexnumbers::mod $n]
	    }
	} else {
	    set res $c
	}
	return $res
    }
}

