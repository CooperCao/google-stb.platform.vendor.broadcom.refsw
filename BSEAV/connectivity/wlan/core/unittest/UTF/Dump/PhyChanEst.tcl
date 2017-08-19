#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 7211d31ddf26ec0a47e1b0d1c7c307da711af5a2 $
# $Copyright Broadcom Corporation$
#

package provide UTF::Dump::PhyChanEst 2.0

package require snit
package require UTF::doc

snit::type UTF::Dump::PhyChanEst {

    # config files.
    pragma -canreplace yes


    typemethod capture {STA} {
	$STA wl phy_deaf 1
	try {
	    catch {$STA wl dump phychanest} ret
	    set o [UTF::Dump::PhyChanEst %AUTO% -raw $ret]
	    set n [$o num_tones]
	    UTF::Message LOG "" "num_tones:$n"
	    while {[set count [regexp -all {chan\(1,1,} [$o matlab_data]]] < $n &&
		   ![info exists ::UTF::panic]} {
		UTF::Message LOG "" "tones captured:$count"
		catch {$STA wl dump phychanest} ret
		$o append $ret
	    }
	} finally {
	    $STA wl phy_deaf 0
	}
	set o
    }

    option -raw

    variable matlab_data
    variable num_tones

    method append {raw} {
	append options(-raw) "\n$raw"
	unset matlab_data
    }

    method matlab_data {} {
	if {![info exists matlab_data]} {
	    if {[regexp {re,im,exp} $options(-raw)]} {
		#UTF::Message LOG "" "raw format"
		set matlab_data [raw2matlab $options(-raw)]
	    } else {
		#UTF::Message LOG "" "matlab format"
		# trim partial records
		regsub {;[^;]+$} $options(-raw) {;} matlab_data
	    }
	}
	set matlab_data
    }

    variable meanminmax {0 0 0}

    method plot {{basefile ""}} {
	if {$basefile eq ""} {
	    set basefile [file nativename \
			      [file join [file dir $UTF::Logfile] \
				   [file rootname $UTF::Logfile].phychan]]
	}
	set dir [file dir $basefile]
	set datafile "$basefile.data.m"
	set fname [file rootname [file tail $datafile]]
	set script "$basefile.script.m"

	set data [$self matlab_data]
	set f [open $datafile "w"]
	puts $f "figname='$basefile.'"
	puts $f $data
	close $f

	set G [open [file join $dir $script] "w"]
	puts $G "source $UTF::unittest/etc/acphy_plot_chanest.m;"
	puts $G "cd $dir;"
	puts $G "acphy_plot_chanest('$fname');"
	close $G
	set ret [localhost octave -qf --no-window-system $script]
	if {[regexp {mean/min/max=([\d.]+)/([\d.]+)/([\d.]+)} $ret - mean min max]} {
	    set meanminmax [list $mean $min $max]
	} else {
	    error "MMM not found!"
	}
	set htmllink "html:"
	foreach graph "phase magnitude condition" {
	    set imglink [subst {<a href="[UTF::URI $basefile.${graph}.png]">[UTF::ThumbData $basefile.${graph}_sm.png]</a>}]
	    append htmllink $imglink
	}
	set htmllink
    }

    method meanminmax {} {
	set meanminmax
    }

    method num_tones {} {
	if {![info exists num_tones]} {
	    if {![regexp {num_tones=(\d+)} [$self matlab_data] - num_tones]} {
		UTF::Message LOG "" "num_tones not found"
		set num_tones 0; # force single_dump
	    }
	}
	set num_tones
    }

    ### internal

    proc raw2matlab {raw} {
	set m ""
	set num_tones 0;
	set r 0;
	set sts 0;
	set k 0;
	foreach line [split $raw \n] {
	    if {[regexp {num_tones=(\d+)} $line - ret]} {
		set num_tones $ret
		append m "$line\n"
	    } elseif {[regexp {rx=(\d+)} $line - ret]} {
		set r $ret
		append m "$line\n"
	    } elseif {[regexp {sts=(\d+)} $line - ret]} {
		set t $ret
		set k 0
		append m "$line\n"
	    } elseif {[regexp {(-?\d+),(-?\d+),(-?\d+)} $line - \
			   ch_re ch_im ch_exp]} {
		set fftk [expr {($k < $num_tones/2) ?
				($k + $num_tones/2) : ($k - $num_tones/2)}]
		append m [format "chan(%d,%d,%d)=(%d+i*%d)*2^%d;\n" \
			      [expr {$r + 1}] \
			      [expr {$t + 1}] \
			      [expr {$fftk + 1}] \
			      $ch_re $ch_im $ch_exp]
		incr k
	    } else {
		append m "% $line\n"
	    }
	}
	return $m
    }

}
