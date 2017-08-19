#
#
# UTF tool to analyze the Phy Channels using data taken
# from wl dump phychanest
#
#
# Written by: Robert J. McMahon February 2015
#
# $Copyright Broadcom Corporation$
#

package require UTF
package require snit
package require math::fourier
package require math::complexnumbers

package provide UTF::PhyChanEst 2.0

snit::type UTF::PhyChanEst {
    typevariable PI [expr {acos(-1)}]
    typevariable GNUPLOT_COMMAND {}
    typevariable EXCEPTATTRIBUTES "-debug -verbose -graphtype -graphsize -matlab"
    typevariable JSDIR "http://www.sj.broadcom.com/projects/hnd_sig_ext4/rmcmahon/gnuplotfiles"
    typemethod config_gnuplot {} {
	if {$GNUPLOT_COMMAND eq {}} {
	    set latestver 0
	    set latestpatch 0
	    set searchpath [list /tools/bin/ /usr/local/bin/ /usr/bin/]
	    foreach searchdir $searchpath {
		set gnuplot [file join $searchdir gnuplot]
		if {[file executable $gnuplot]} {
		    set ret [exec $gnuplot --version]
		    if {![regexp {gnuplot (\d+\.\d+)\spatchlevel\s(\d+)?} $ret -- version patchlevel]} {
			continue
		    }
		    if {($version > $latestver) || ($version == $latestver && $patchlevel > $latestpatch)} {
			set ::env(GNUPLOTVER) "${latestver}.$latestpatch"
			set GNUPLOT_COMMAND $gnuplot
			set ::UTF::Gnuplot $gnuplot
		    }
		}
	    }
	    if {$GNUPLOT_COMMAND eq {}} {
		UTF::_Message ERROR PHYCHAN "$GNUPLOT_COMMAND not executable"
		error "no gnuplot"
	    } else {
		UTF::_Message INFO PHYCHAN "$GNUPLOT_COMMAND [exec $GNUPLOT_COMMAND --version]"
	    }
	}
    }
    option -sta -readonly true
    option -ap -readonly true
    option -name -default "" -readonly true
    option -debug -type boolean -default false
    option -graphtype -default png
    option -graphsize -default "1024,768"
    option -verbose -type boolean -default false
    option -title -default ""
    option -notraffic -type boolean -default false
    option -mode "sta"
    option -store -type boolean -default true
    option -forcephycal -type boolean -default false
    option -maxtemp -type boolean -default false

    # UDP offer rate
    option -b -default "1.2G"

    variable rawsamples -array {}
    variable normalized -array {}
    variable polar -array {}
    variable _subchannels -array {}
    variable _txantennas -array {}
    variable _rxantennas -array {}
    variable _sampleid -1
    variable graphcache
    variable plotgraphcache
    variable utfmsgtag
    variable rxant
    variable sts
    variable lastindex
    variable traffic
    variable cyclicshiftfreq -array {}
    variable _directoryindex
    variable htmllink
    variable rawoutput
    variable matlabformat
    variable bestrate

    constructor {args} {
	$self configurelist $args
	set utfmsgtag "${options(-sta)}-PHY"
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
	UTF::PhyChanEst config_gnuplot
	if {$options(-title) eq ""} {
	    set options(-title) "$options(-sta)"
	}
	if {!$options(-notraffic)} {
	    package require UTF::Streams
	    if {$options(-mode) eq "sta"} {
		set traffic [UTF::stream %AUTO% \
				 -tx $options(-ap) -rx $options(-sta) \
				 -protocol udp -pktsize 1470 -rate $options(-b)]
	    } else {
		set traffic [UTF::stream %AUTO% \
				 -rx $options(-ap) -tx $options(-sta) \
				 -protocol udp -pktsize 1470 -rate $options(-b)]
	    }
	    $traffic id
	}
	array set cyclicshiftfreq [list 0 0 1 [expr {$PI/4}] 2 [expr {$PI/8}] 3 [expr {$PI/(8*3)}]]
	set _directoryindex 0
    }
    destructor {
	catch {$traffic destroy}
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
    method dump {} {
	parray polar
    }
    method mylink {} {
	$self plot
	return $htmllink
    }
    method bestrate {} {
	return $bestrate
    }
    method store {} {
	if {$matlabformat} {
	    set filename [file join $plotgraphcache data.m]
	    set G [open $filename "w"]
	    puts $G $rawoutput
	    close $G
	    UTF::Message INFO $utfmsgtag "Data written to $filename in matlab format"
	}
    }

    # For compatibility with old scripts, stash a copy for the
    # "condition" method to return, since ThumData deleted the
    # original.
    variable conditionlink "<unset>"

    method condition {{text ""}} {
	return "html:$conditionlink $text"
    }
    method plot {{text ""}} {
	set filename [file join $plotgraphcache script.m]
	set G [open $filename "w"]
	puts $G "source $UTF::unittest/etc/acphy_plot_chanest.m;"
	puts $G "cd [file join $plotgraphcache];"
	puts $G {acphy_plot_chanest('data');}
	close $G
	localhost octave -qf --no-window-system $filename
	set htmllink "html:"
	foreach graph "phase magnitude condition" {
	    set imglink [subst {<a href="[UTF::URI $plotgraphcache/${graph}.png]">[UTF::ThumbData $plotgraphcache/${graph}_sm.png]</a>}]
	    if {$graph eq "condition"} {
		set conditionlink $imglink
	    }
	    append htmllink $imglink
	}
    }
    method read_ampdu {args} {
	set output [$options(-$options(-mode)) wl dump ampdu]
	set rxtx "rx"
	set bestrate "UNK"
	switch $rxtx {
	    "tx" {
		set match [regexp {\nTX MCS\s*:(.*)\nTX VHT\s*:\s*(.*)\nTX MCS SGI:} $output - mcstable vhttable]
	    }
	    "rx" {
		set match [regexp {\nRX MCS\s*:(.*)\nRX VHT\s*:\s*(.*)\nRX MCS SGI:} $output - mcstable vhttable]
	    }
	    default "program error: $rxtx"
	}
	if {$match} {
	    set row 1
	    foreach line [split $mcstable "\n"] {
		set line [string trim $line { :}]
		if {$line ne {}} {
		    set mcs($row) $line
		    incr row
		}
	    }
	    set row 1
	    foreach line [split $vhttable "\n"] {
		set line [string trim $line { :}]
		if {$line ne {}} {
		    set vht($row) $line
		    incr row
		}
	    }
	    set best "-1/-1"
	    set maxpackets 0
	    set maxpercent 0
	    if {[array exists mcs]} {
		foreach row [array names mcs] {
		    set column 0
		    foreach entry $mcs($row) {
			if {[regexp {([0-9]+)\(([0-9]+%)} $entry - packets percent]} {
			    if {$packets > $maxpackets} {
				set maxpackets $packets
				set maxpercent $percent
				set best "MCS ${column}x${row}x${percent}"
			    }
			    incr column
			}
		    }
		}
	    }
	    if {[array exists vht]} {
		foreach row [array names vht] {
		    set column 0
		    foreach entry $vht($row) {
			if {[regexp {([0-9]+)\(([0-9]+%)} $entry - packets percent]} {
			    if {$packets > $maxpackets} {
				set maxpackets $packets
				set maxpercent $percent
				set best "VHT ${column}x${row}x${percent}"
			    }
			    incr column
			}
		    }
		}
	    }
	}
	switch $rxtx {
	    "tx" {
		set match [regexp {tot_mpdus\s+([0-9]+)\s+tot_ampdus\s+([0-9]+)} $output - mpdu ampdu]
	    }
	    "rx" {
		set match [regexp {rxampdu\s+([0-9]+)\s+rxmpdu\s+([0-9]+)} $output - ampdu mpdu]
	    }
	}
	if {$match} {
	    set mpduampdu [expr {(10.0 * $mpdu / $ampdu)/10}]
	    if {[expr {$mpduampdu < 10}]} {
		set mpduampdu [format %.1f [expr {(round(10.0 * $mpduampdu)) / 10.0}]]
	    } else {
		set mpduampdu [expr {round($mpduampdu)}]
	    }
	} else {
	    UTF::Message WARN "" "Did not find $rxtx mpdu and ampdu in ampdu dump"
	    set mpduampdu "unk"
	}
	set bestrate "$best (mdense=$mpduampdu)"
	UTF::Message INFO "" "Best rate is $bestrate"
	return
    }

    method sample {args} {
	set errored 0
	incr _sampleid
	set index "sample#${_sampleid}"
	set lastindex $index
	$self set_next_directory
	if {$options(-maxtemp)} {
	    package require UTF::Test::TemperatureConvergence
	    catch {UTF::Test::TemperatureConvergence -ap $options(-ap) -sta $options(-sta)}
	}
	if {$options(-forcephycal)} {
	    #-Nitin
	    # Following are the details reagarding wl phy_percal
	    #	0, disable
	    #	1, enable, singlephase cal only,
	    #	2, enable mutliphase cal allowed
	    #	3, manual (testing mode), blocking all driver initiated
	    #      periodical cal, give phy_forcecal the full control
	    set curr [$options(-$options(-mode)) wl phy_percal]
	    $options(-$options(-mode)) wl -u phy_percal 0
	    $options(-$options(-mode)) wl -u phy_forcecal 1
	    $options(-$options(-mode)) wl -u dump phycal
	    UTF::Sleep 2.0
	}
	$options(-$options(-mode)) wl -u dump_clear_ampdu
	$options(-$options(-mode)) wl -u ampdu_clear_dump
	if {!$options(-notraffic)} {
	    $traffic start
	    if {[catch {$traffic linkcheck -now} err]} {
		$traffic stop
		error $err
	    }
	}
	set tries 5
	while {$tries && [catch {$options(-$options(-mode)) [expr {($options(-verbose) eq "false") ? {-silent} : {}}] wl dump phychanest} rawoutput]} {
	    UTF::Message WARN $utfmsgtag "Retrying wl dump phychanest command $tries"
	    incr tries -1
	    set rawoutput {}
	}
	if {[string index $rawoutput end] ne {;}} {
	    UTF::Message ERROR $utfmsgtag "Couldn't get a complete phychanest dump"
	    set rawoutput [string range $rawoutput 0 [string last \; $rawoutput]]
	}
	if {$rawoutput eq {}} {
	    error "Couldn't get any reasonable phychanest dump"
	}
	$self read_ampdu
	if {$options(-forcephycal)} {
	    $options(-$options(-mode)) wl -u phy_percal $curr
	}
	if {!$options(-notraffic)} {
	    $traffic stop
	}
	set lines [split $rawoutput "\n"]
	if {![catch {$self line2normalized [lindex $lines 0] -1}]} {
	    set matlabformat 1
	    $self store
	    UTF::Message INFO $utfmsgtag "Phychanest output in matlab format: [lindex $lines 0]"
	} else {
	    set matlabformat 0
	    UTF::Message INFO $utfmsgtag "Phychanest output in raw format: [lindex $lines 0]"
	    if {[regexp {num_tones=([0-9]+)} [lindex $lines 0] - num_tones]} {
		set lines [lrange $lines 1 end]
		set index "polar#${_sampleid}"
		set polar(${index},num_tones) $num_tones
		set polar(${index},timestamp) [clock seconds]
		foreach line $lines {
		    if {[regexp {(-?[0-9]+,-?[0-9]+,-?[0-9]+)} $line - cnum]} {
			foreach {real imagine exp} [split $cnum ","] break
			if {$options(-store)} {
			    if {($real eq "0") && ($imagine eq "32") && ($exp eq "0")} {
				set imagine 0
			    }
			    puts $G "chan\([expr {1 + ${rxant}}],[expr {1 + ${sts}}],[expr {1 + ${freq}}]\)=\(${real}+i*$imagine\)*2^${exp}\;"
			    # UTF::Message INFO $utfmsgtag "Raw output written to $filename"
			}
			set polar(${index},${rxant},${sts},${freq}) [list $amplitude $phase]
			incr freq
		    } elseif {[regexp {sts=([0-9]+)} $line - sts]} {
			if {$options(-debug)} {
			    UTF::_Message DEBUG $utfmsgtag "Parsing Spacial Stream=$sts"
			}
			lappend _txantennas($index) $sts
			if {![info exists polar(${index},maxtxcnt)] || $sts > $polar(${index},maxtxcnt)} {
			    set polar(${index},maxtxcnt) $sts
			}
			set freq 0
		    } elseif {[regexp {rx=([0-9]+)} $line - rxant]} {
			if {$options(-debug)} {
			    UTF::_Message DEBUG $utfmsgtag "Parsing RX antenna=$rxant"
			}
			if {![info exists polar(${index},maxrxcnt)] || $rxant > $polar(${index},maxrxcnt)} {
			    set polar(${index},maxrxcnt)  $rxant
			}
		    } elseif {[regexp {re,im,exp} $line]} {
			continue
		    } else {
			UTF::_Message ERROR $utfmsgtag "Parse error for: $line"
		    }
		}
		if {!$errored} {
		    UTF::_Message INFO $utfmsgtag "Phy channel estimate taken succesfully (rx:[expr {$polar(${index},maxrxcnt) + 1}],tx:[expr {$polar(${index},maxtxcnt) + 1}],sc:$polar(${index},num_tones))"
		} else {
		    UTF::_Message INFO $utfmsgtag "Phy channel estimate had errors"
		}
	    } else {
		error "invalid format"
	    }
	}
	if {$options(-store) && !$matlabformat} {
	    close $G
	}
	if {$options(-debug)} {
	    catch {UTF::_Message DEBUG $utfmsgtag "subchannels: $_subchannels($index)"}
	    catch {UTF::_Message DEBUG $utfmsgtag "rx: $_rxantennas($index)"}
	    catch {UTF::_Message DEBUG $utfmsgtag "tx: $_txantennas($index)"}
	}
	return $index
    }
    method set_next_directory {} {
	while {[file isdirectory [file join $graphcache phyest$_directoryindex]]} {
	    incr _directoryindex
	}
	set plotgraphcache [file join $graphcache phyest$_directoryindex]
	if {[catch {file mkdir $plotgraphcache} res]} {
	    error "Graph : unable to make directory $plotgraphcache $res"
	} elseif {![file writable $plotgraphcache]} {
	    error "Graph : directory $plotgraphcache not writeable"
	}
    }
    method tclplot {{sample -1}} {
	if {$sample eq -1} {
	    $self sample
	    set index $lastindex
	    set title "$options(-title)\\n[clock format [clock seconds]]"
	} else {
	    set index $sample
	    set title "$options(-title) $sample\\n[clock format [clock seconds]]"
	}
	if {[regexp {polar#} $index]} {
	    $self plot_polar $index
	} else {
	    $self plot_cartisian $index
	}
    }
    method plot_polar {index} {
	UTF::Message INFO $utfmsgtag "Plotting $index results in $plotgraphcache"
	set G [open [file join $plotgraphcache data.gpc] "w"]
	fconfigure $G -buffering line
	for {set rxix 0} {$rxix <= $polar($index,maxrxcnt)} {incr rxix}  {
	    for {set txix 0} {$txix <= $polar($index,maxtxcnt)} {incr txix}  {
		for {set freqix 0} {$freqix < $polar($index,num_tones)} {incr freqix}  {
		    puts $G "$freqix [lindex $polar($index,$rxix,$txix,$freqix) 0] [lindex $polar($index,$rxix,$txix,$freqix) 1]"
		}
		puts -nonewline $G "\n\n"
	    }
	}
	close $G
	foreach type "Magnitude Phase" {
	    set gnuplotindex 0
	    for {set rxix 0} {$rxix <= $polar($index,maxrxcnt)} {incr rxix}  {
		set G [open [file join $plotgraphcache helper.gpc] w]
		fconfigure $G -buffering line
		puts $G "reset; unset multiplot"
		if {$mode eq {-sta}} {
		    set out [file join $plotgraphcache ${options(-sta)}${type}_rx_ant${rxix}]
		} else {
		    set out [file join $plotgraphcache ${options(-ap)}${type}_rx_ant${rxix}]
		}
		if {$options(-graphtype) eq "png"} {
		    puts $G "set terminal png enhanced size 1024,768"
		    set out "${out}.png"
		} else {
		    puts $G "set terminal svg enhanced size 1024,768"
		    set out "${out}.html"
		}
		puts $G "set output \"${out}\""
		puts $G "set multiplot layout [expr {$polar($index,maxtxcnt) + 1}],1 title \"$options(-title) $type SubChan($polar($index,num_tones)) RX=$rxix\\n[clock format $polar($index,timestamp)]\""
		puts $G "set xtics [expr {$polar($index,num_tones) / 16}]"
		puts $G {set grid}
		if {$type eq "Phase"} {
		    puts $G "set format y \"%.1P pi\""
		    puts $G "set ytics pi/2"
		    set dataoffset 3
		} else {
		    # puts $G "set ytics 0.5"
		    set dataoffset 2
		}
		for {set txix 0} {$txix <= $polar($index,maxtxcnt)} {incr txix}  {
		    puts $G "plot \"[file join $plotgraphcache data.gpc]\" index $gnuplotindex using 1:$dataoffset title \"STS=$txix\" with impulses lt [expr {$dataoffset -1}] lw 2"
		    incr gnuplotindex
		}
		puts $G "unset multiplot"
		close $G
		catch {exec gnuplot [file join $plotgraphcache helper.gpc]}
	    }
	}
    }
    method plot_cartisian {index} {
	set amplitudes {}
	foreach tx $_txantennas($index) {
	    set G [open [file join $plotgraphcache data.gpc] "w"]
	    fconfigure $G -buffering line
	    foreach freq $_subchannels($index) {
		puts -nonewline $G "$freq"
		foreach rx $_rxantennas($index) {
		    set complexvalue $normalized($index,$tx,$rx,$freq)
		    set amplitude [::math::complexnumbers::mod  $complexvalue]
		    puts -nonewline $G " $amplitude "
		}
		puts -nonewline $G "\n"
	    }
	    puts -nonewline $G "\n\n"
	    foreach freq $_subchannels($index) {
		puts -nonewline $G "$freq"
		foreach rx $_rxantennas($index) {
		    set complexvalue $normalized($index,$tx,$rx,$freq)
		    set phase [expr {atan2([::math::complexnumbers::imag $complexvalue], [::math::complexnumbers::real $complexvalue])}]
		    puts -nonewline $G " $phase "
		}
		puts -nonewline $G "\n"
	    }
	    close $G
	    set G [open [file join $plotgraphcache helper.gpc] w]
	    fconfigure $G -buffering line
	    set numrx [llength $_rxantennas($index)]
	    foreach type "magnitude phase" plotix "0 1" {
		if {$options(-graphtype) eq "png"} {
		    puts $G "set terminal png enhanced size 1024,768"
		    set out [file join $plotgraphcache ${type}_tx_ant${tx}.png]
		} else {
		    puts $G "set terminal svg enhanced size 1024,768"
		    set out [file join $plotgraphcache ${type}_tx_ant${tx}.html]
		}
		puts $G "set output \"${out}\""
		puts $G "set xtics [expr {[lindex $_subchannels($index) end] / 16}]"
		puts $G {set grid}
		if {$type eq "phase"} {
		    puts $G "set format y \"%.1P pi\""
		    puts $G "set ytics pi/2"
		} else {
		    puts $G "set ytics 0.5"
		}
		puts $G "set title \"SubChan ($type) (TX=$tx) $title\""
		puts $G "set multiplot layout $numrx,1"
		for {set rxix 2} {[expr {$rxix <= ($numrx + 1)}]} {incr rxix} {
		    puts $G "plot \"[file join $plotgraphcache data.gpc]\" index ${plotix} using 1:$rxix title \"RX=[expr {$rxix - 1}]\" with impulses lt [expr {$plotix + 1}]"
		}
		puts $G "unset multiplot"
	    }
	    close $G
	    exec gnuplot [file join $plotgraphcache helper.gpc]
	}
    }
    method diff {s2 s1} {
	foreach freq2 $_subchannels($s2) {
	    foreach tx2 $_txantennas($s2) tx1 $_txantennas($s2) {
		foreach rx $_rxantennas($index) {
		    set c2 $normalized($s2, $tx,$rx,$freq)
		    set c1 $normalized($s1, $tx,$rx,$freq)
		    set c [::math::complexnumbers::-  $c1 $c2]
		}
		puts -nonewline $G "\n"
	    }
	    puts -nonewline $G "\n\n"
	    foreach freq $_subchannels($index) {
		puts -nonewline $G "$freq"
		foreach rx $_rxantennas($index) {
		    set complexvalue $normalized($tx,$rx,$freq)
		    set phase [expr {atan2([::math::complexnumbers::imag $complexvalue], [::math::complexnumbers::real $complexvalue])}]
		    puts -nonewline $G " $phase "
		}
		puts -nonewline $G ""
	    }
	}
    }
    method line2normalized {line index} {
	if {[regexp {chan\(([0-8]{1,1}),([0-8]{1,1}),([0-9]+)\)=\((-?[0-9]+)\+i\*(-?[0-9]+)\)\*2\^(-?[0-9]+)\;} $line - txantenna rxantenna subchannelindex real imagine exp]} {
	    set cnum [math::complexnumbers::complex $real $imagine]
	    set exponent [list [expr {pow(2,$exp)}] 0]
	    set coefficient [::math::complexnumbers::* $cnum $exponent]
	    if {![info exists _subchannels($index)] || [lsearch $_subchannels($index) $subchannelindex] < 0} {
		lappend _subchannels($index) $subchannelindex
		set _subchannels($index) [lsort -integer -increasing $_subchannels($index)]
	    }
	    if {![info exists _txantennas($index)] || [lsearch $_txantennas($index) $txantenna] < 0} {
		lappend _txantennas($index) $txantenna
		set _txantennas($index) [lsort -integer -increasing $_txantennas($index)]
	    }
	    if {![info exists _rxantennas($index)] || [lsearch $_rxantennas($index) $rxantenna] < 0} {
		lappend _rxantennas($index) $rxantenna
		set $_rxantennas($index) [lsort -integer -increasing $_rxantennas($index)]
	    }
	} else {
	    set coefficient [list 0 0]
	    set txantenna -1
	    set rxantenna -1
	    set subchannelindex -1
	    UTF::_Message WARN $utfmsgtag "Parse fail for $line"
	    error "parse fail"
	}
	set final($index,$txantenna,$rxantenna,$subchannelindex) $coefficient
	return [array get final]
    }
    proc exp2cartesian {amp theta} {
	set real [expr {$amp * cos($theta)}]
	set imag [expr {$amp * sin($theta)}]
	return [math::complexnumbers::complex $real $imag]
    }
}

