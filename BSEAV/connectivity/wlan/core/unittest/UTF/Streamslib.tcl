#!/bin/env utf
#
# $Id: debf09ba2058c51916bfa6b77001f6b2700ef54f $
# $Copyright Broadcom Corporation$
#package require snit
package require UTF
package require base64

package provide UTF::Streamslib 2.0

namespace eval UTF::Streamslib {}

proc UTF::Streamslib::force_cleanup {devices} {
    foreach device $devices {
	catch {$device lan iptables -F}
	catch {$device lan pkill iperf}
	catch {$device lan pkill iperf208c64}
	catch {$device lan pkill tcpdump}
	catch {$device lan iperf -v}
	catch {$device lan uname -r}
        catch {$device deinit}
    }
}

proc UTF::Streamslib::graphs_create_utf_htmlpage {graphs} {
    UTF::Getopts {
	graphs.arg ""
	summary ""
    }
    set htmllink "html:"
    foreach graph $graphs {
	set plotgraph($graph) [graph plot]
	append htmlink [string range $graph 5 end]
    }
    if {!$summary} {
	return $htmllink
    }
}

proc UTF::Streamslib::grapharray {data args} {
    UTF::GetKnownopts {
	{title.arg "" "Title for Graph"}
	{outputtype.arg "png" "Output type"}
	{size.arg "1024,768" "Graph size"}
	{thumbsize.arg "64,32" "Thumbnail size"}
	{htmltxt.arg "" "Text for HTML link"}
	{ymin.arg "" "Use this for the default ymin"}
	{style.arg "lines" "Gnuplot style"}
	{ylabel.arg "" "Text for ylabel"}
	{xlabel.arg "" "Text for xlabel"}
	{yrange.arg "0:" "Default yrange"}
	{xrange.arg "" "Default xrange"}
	{append "return an appendable url"}
	{overlay.arg "" "The attenuator overlay function, pass as tcl array"}
	{pointtype.arg "1" "Gnuplot point type"}
	{linetype.arg "1" "Gnuplot line type"}
	{colors.arg "1 2" "Colors to use"}
	{overlaycolors.arg "#969696 #000000" "RGB colors to use"}
	{precision.arg "0" "Precision of x-axis (or decimal places)"}
	{yprecision.arg "0" "Precision of y-number (or decimal places)"}
	{xtics.arg "" "xtics value"}
	{width.arg "2" "Line width to use"}
	{name.arg "" "name to append to file"}
    }
    upvar $data myarray
    if {$(overlay) ne ""} {
	upvar $(overlay) attnarray
	if {![array exists attnarray]} {
	    error "-overlay must be a tcl array"
	}
	set YAXISATTRIBUTES "set y2tics\nset y2range \[0:103\]\nset y2label \"Attn dB\""
    }
    set GNUPLOT_COMMAND [::UTF::streamgraph config_gnuplot]
    set otype $(outputtype)
    if {[info exists ::tcl_interactive] && $::tcl_interactive} {
	set graphcache [file join [exec pwd] graphcache]
    } elseif {[info exists ::UTF::Logdir]} {
	set graphcache [file join $::UTF::Logdir graphcache]
    } else {
	error "Graph: Unable to find directory for graphcache."
    }
    if {![file exists $graphcache]} {
	if {[catch {file mkdir $graphcache} res]} {
	    error "Graph : unable to make directory $graphcache $res"
	}
    } elseif {![file writable $graphcache]} {
	error "Graph : directory $graphcache not writeable"
    }
    if {$(htmltxt) eq ""} {
	set (htmltxt) $data
    }
    set ix 0
    if {$(ymin) ne {}} {
	set ymin $(ymin)
    }
    if {$(style) eq "CDF"} {
	set datafile "CDF"
    } elseif {$(style) eq "CCDF"} {
	set datafile "CCDF_$(name)"
    } else {
	set datafile $data
    }
    while {1} {
	set outfile [file join $graphcache [namespace tail $datafile]]
	if {[file exists ${outfile}_$ix.$otype]} {
	    incr ix
	} else {
	    set datafile "${outfile}_$ix.data"
	    set out ${outfile}_$ix.$otype
	    break
	}
    }
    set fid [open $datafile w]
    if {![file exist $datafile]} {
	set msg "Cannot create gnu plot data file"
	UTF::Message ERROR "" $msg
	error $msg
    } else {
	set total 0
	set xvalues [lsort -real -increasing [array names myarray]]
	if {$(style) eq "CDF" || $(style) eq "CCDF"} {
	    foreach xn $xvalues {
		set total [expr {$total + $myarray($xn)}]
	    }
	    set running 0
	    foreach xn $xvalues {
	        set running [expr {$running + (1.0 * $myarray($xn) / $total)}]
		if {$(style) eq "CCDF"} {
		    puts $fid "$xn [expr {1 -$running}]"
		} else {
		    puts $fid "$xn $running"
		}
	    }
	    set ymax 1
	    set ymin 0
	} else {
	    foreach xn $xvalues {
		foreach y $myarray($xn) {
		    puts $fid "$xn $y"
		    if {![info exists ymax] || $y > $ymax} {
			set ymax $y
		    }
		    if {![info exists ymin] || $y < $ymin} {
			set ymin $y
		    }
		}
	    }
	}
	close $fid
	set ymaxr [expr {$ymax + (($ymax - $ymin) * 0.1)}]
	if {$ymin > 0} {
	    set tmp [expr {$ymin - (($ymax - $ymin) * 0.1)}]
	    if {$tmp > 0} {
		set ymin $tmp
	    }
	}
    }
    set G  [open ${outfile}_$ix.gpc w]
    puts $G "set output \"${out}\""
    if {[UTF::GnuplotVersion] > 4.0} {
	puts $G  "set terminal png size $(size)"
    } else {
	set tmp [split $(size) ,]
	set x [lindex $tmp 0]
	set y [lindex $tmp 1]
	puts $G "set terminal png picsize ${x} ${y}"
    }
    if {$(title) ne ""} {
	set title "$(title)"
    } else {
	set title $data
    }
    if {$(xlabel) ne ""} {
	set xlabel "$(xlabel)\\n[clock format [clock seconds]]"
    } else {
	set xlabel "[clock format [clock seconds]]"
    }
    puts $G {set key bottom}
    puts $G "set title \"$title\""
    puts $G "set xlabel \"$xlabel\""
    if {$(ylabel) ne ""} {
	puts $G "set ylabel \"$(ylabel)\""
    }
    # puts $G {set autoscale fix}

    if {$(style)  eq "CDF" || $(style)  eq "CCDF" ||  $(style) eq "PDF"} {
	switch -exact $(xrange) {
	    {[0:10]} {
		puts $G "set xtics add 0.5"
		puts $G "set format x \"%.1f\""
	    }
	    {[0:20]} {
		puts $G "set xtics add 1"
		puts $G "set format x \"%.0f\""
	    }
	    {[0:40]} {
		puts $G "set xtics add 2"
		puts $G "set format x \"%.0f\""
	    }
	}
    } else {
	puts $G "set format x \"%.${(precision)}s%c\""
    }
    if {$(style)  eq "CDF"  || $(style) eq "CCDF"} {
	puts $G "set ytics add 0.1"
	puts $G "set format y \"%.1f\""
    } else {
	puts $G "set format y \"%.${(yprecision)}s%c\""
    }
    puts $G "set yrange \[$(yrange)\]"
    if {$(xtics) ne ""} {
	puts $G "set xtics $(xtics)"
    } else {
	puts $G {set grid}
    }
    if {$(xrange) ne ""} {
	puts $G "set xrange $(xrange)"
    }
    if {$(overlay) ne ""} {
	puts $G $YAXISATTRIBUTES
    }
    set plottxt {}
    switch $(style) {
	"points" {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with points pointtype $(pointtype) notitle"
	}
	"lines" {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with lines linetype $(linetype) notitle"
	}
	"CDF" -
	"CCDF" {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with lines linetype $(linetype) linewidth 2  notitle"
	}
	"PDF" {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with impulses linetype $(linetype) notitle"
	}
	default {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with $(style) linetype $(linetype) linewidth $(width) notitle"
	}
    }
    if {$(overlay) ne ""} {
	set fid [open $datafile a]
	puts $fid "\n"
        set lt "-1"
	set segment 1
	foreach t [lsort -real -increasing [array names attnarray]] {
	    if {[llength $attnarray($t)] > 1} {
		puts $fid "\n"
		# Set the overlay line width to twice the normal size
		append plottxt ", \"$datafile\" using 1:2 index $segment axes x1y2 notitle with lines lt $lt lw 2 lc rgb \"[lindex $(overlaycolors) [expr {$segment % 2}]]\""
		set xtictxt [lindex $attnarray($t) 1]
		if {$xtictxt ne "-1"} {
		    puts $G "set xtics add \(\"$xtictxt\" $t\)"
		}
		incr segment +1
	    }
	    if {[info exists v0]} {
		puts $fid "$t $v0"
	    }
	    puts $fid "$t [lindex $attnarray($t) 0]"
	    set v0 [lindex $attnarray($t) 0]
	}
	close $fid
	append plottxt ", \"$datafile\" using 1:2 index $segment axes x1y2 notitle with lines lt $lt lw 2 lc rgb \"[lindex $(overlaycolors) [expr {$segment % 2}]]\""
    }
    puts $G $plottxt
    puts $G "reset"
    puts $G "unset multiplot"
    puts $G "set output \"${outfile}_${ix}_thumb.png\""
    if {[UTF::GnuplotVersion] > 4.0} {
	puts $G "set terminal png transparent size $(thumbsize) crop"
    } else {
	set tmp [split $(thumbsize) ,]
	set x [lindex $tmp 0]
	set y [lindex $tmp 1]
	puts $G "set terminal png transparent picsize ${x} ${y}"
    }
    puts $G {unset xtics; unset ytics; unset key; unset xlabel; unset ylabel; unset border; unset grid; unset yzeroaxis; unset xzeroaxis; unset title; set lmargin 0; set rmargin 0; set tmargin 0; set bmargin 0;}
    if {$(ymin) ne {} && $ymin < $ymaxr} {
	puts $G "set yrange \[$ymin:$ymaxr\]"
    }
    if {$(xrange) ne ""} {
	puts $G "set xrange $(xrange)"
    }
    switch $(style) {
	"points" {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with points pointtype $(pointtype) notitle"
	}
	"lines" {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with lines linetype $(linetype) notitle"
	}
	"CDF" -
        "CCDF" {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with lines linetype $(linetype) notitle"
	}
	"PDF" {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with impulses linetype $(linetype) notitle"
	}
	default {
	    set plottxt "plot \"$datafile\" using 1:2 index 0 with $(style) linetype $(linetype) linewidth 2 notitle"
	}
    }
    puts $G "$plottxt \n"
    close $G
    if {[catch {exec $GNUPLOT_COMMAND ${outfile}_$ix.gpc} results]} {
	UTF::Message WARN "" "GNUPLOT catch message: $results"
    }
    UTF::Message INFO "" "Graph done: $out"
    set fd [open "${outfile}_${ix}_thumb.png"]
    fconfigure $fd -translation binary
    set thumbdata [base64::encode -maxlen 0 [read $fd]]
    close $fd
    if {$(htmltxt) eq ""} {
	set text4htmllink "$data $ymax"
    } else {
	set text4htmllink $(htmltxt)
    }
    set res "html:<!--\[if IE]><a href=\"${outfile}_${ix}.$otype\"><img src=\"${outfile}_${ix}.${otype}\" alt=\"url\" /></a><a href=\"${outfile}_${ix}.$otype\">$text4htmllink</a><!\[endif]--><!\[if !IE]><a href=\"${outfile}_${ix}.$otype\"><img src=\"data:image/png;base64,$thumbdata\" alt=\"data\" /></a><a href=\"${outfile}_${ix}.$otype\">$text4htmllink</a>\<!\[endif]>"
    if {$(append)} {
	return " [string range $res 5 end]"
    } else {
	return $res
    }
}
