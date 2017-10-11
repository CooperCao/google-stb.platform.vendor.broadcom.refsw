#!/bin/env utf

# UTF Framework Object Definition for a ControlChart using n>2 sample
# size clustering.  For data which cannot be logically clustered, such
# as memory tests, see MemChart.tcl
#
# With a little work on the cache file format it should be possible to
# merge MemChart.tcl back into this.
#
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::ControlChart 2.0

package require snit
package require UTF::doc

UTF::doc {
    # [manpage_begin UTF::ControlChart n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF ControlChart}]
    # [copyright {2008 Broadcom Corporation}]
    # [require UTF::ControlChart]
    # [description]
    # [para]

    # UTF::ControlChart takes performance data (usually from iperf)
    # and performs a Control Chart stability analysis on the new data
    # compared with cached historical data.[para]

    # Normally another script will create a new object and then use
    # method addsample to get updated results.[para]

    # ControlChart can optionally use gnuplot to provide graphical
    # plots for visual reporting in addition to the autimated
    # analysis.

    # ControlChart is currently called by Test/controlchart.test.
    # There is a related self test utility Test/randomchart.test. See
    # either of these scripts for practical examples of setting up the
    # appropriate object and calling the addsample method.

    # [list_begin definitions]
}

snit::type UTF::ControlChart {

    UTF::doc {
        # [call [cmd UTF::ControlChart] [arg name]
        #       [lb][option -perfcache] [arg pathname] [rb]
        #       [lb][option -key] [arg string][rb]
        #       [lb][option -s] [arg integer][rb]
        #       [lb][option -nsigma] [arg number][rb]
        #       [lb][option -history] [arg integer][rb]
        #       [lb][option -format] [arg format][rb]]

        # Create a new ControlChart object.
        # [list_begin options]

        # [opt_def [option -perfcache] [arg pathname]]

        # Pathname of directory used to store historical performance
        # data.  If -perfcache is not specified, variables
        # UTF::Perfcache and UTF::SummaryDir will be checked, and if
        # present, will be used to determine as the default location.

        # Note: relative paths will be relative to the current working
        # directory, which is the same as the directory the script was
        # launched in, not the SummaryDir.

        # [opt_def [option -key] [arg string]]

        # Arbitrary text string used to which history should be used.
        # It should uniquely describe all configuration options
        # expected to affect the performance being examined, including
        # the names of the endpoints.  It will be used as a file name
        # (after translating '/' into '.').  If the key is too large
        # to be used directly, an md5 hash of the key will be used
        # instead.  Default is null.[para]

        # NB: The keys need to be sufficiently unique in order to keep
        # the data for each test separate.  If not, you will get
        # performance data from different tests mixed together
        # resulting multimodal behaviour and meaningless results, ie
        # the faster test will always report HIGH and the slower test
        # will always report LOW.

	# For multi-driver hosts, it is recommended that you use
	# static configurations so that the STA names uniquely define
	# the driver type.  If you manually switch driver, eg by
	# loading with explicit -brand or -type options, then the STA
	# name will not be sifficient and you will need to add the
	# additional driver identification to the key in some other
	# way.

        # [opt_def [option -oldkey] [arg string]]

	# Alternate key to be used if no cache file is found for the
	# primary key.  Used for migrating between keys.

	# [opt_def [option -title] [arg string]]

	# Test title (will have "ControlChart" appended)

	# [opt_def [option -ylabel] [arg string]]

	# Y-axis label.  Defaults to the same as [option -title]

	# [opt_def [option -s] [arg integer]]

        # Specifies the number of data points in each sample. Integer
        # value, range is 2 to 25. Default is 5.  Values other than 5
        # should work, but have not been extensively tested.  For
        # tests that only provide a single datapoint per sample, see
        # [uri APdoc.cgi?UTF::MemChart.tcl UTF::MemChart]

        # [opt_def [option -nsigma] [arg number]]

        # Specifies the number of sigmas variation used for the
        # control chart control limits.  Default is 3.0.

        # [opt_def [option -history] [arg integer]]

        # Specifies how many data samples are stored in the history
        # disk file.  Integer value greater than 0.  Default is 30.

	# [opt_def [option -format] [arg format]]

	# Specifies a sprintf-style reporting format for the data.
	# Default is "%.2f".  For integer data such as memory limits,
	# [option -format] [arg "%.0f"] may be useful.

	# [opt_def [option -allowzero] [arg boolean]]

	# By default, zero values are assumed to be an immediate
	# failure.  They are recorded in the performance cache for the
	# historical record, but not included in the means
	# calculation.  Setting [option -allowzero] [arg true]
	# disables this and allows zero to be treated normally.

        # [list_end]

        # [para]
        # ControlChart objects have the following methods:
        # [list_begin definitions]
    }

    pragma -canreplace yes
    option -perfcache -readonly true
    option -key -readonly true -configuremethod CheckKey
    option -oldkey -readonly true -configuremethod CheckKey
    option -s 5
    option -nsigma 3.0
    option -history 30
    option -format "%.2f"
    option -allowzero -type snit::boolean -default false
    option -norangecheck -type snit::boolean -default false
    option -units "Mbit/sec"
    option -title "Throughput"
    option -ylabel ""
    option -overridehistoricalmean ""

    variable A2
    variable D3
    variable D4
    variable E2
    variable G ""
    variable mmmlist  {}
    variable Rlist    {}
    variable Rmean    0
    variable meanmean 0
    variable group    0
    variable Ux 0
    variable Lx 0
    variable UR 0
    variable LR 0
    variable plotfile ""
    variable plotfile_sm ""


    typevariable readonly 0

    method CheckKey {key val} {
	if {[regexp {^/} $val]} {
	    error "ControlChart $key must not start with '/': $val"
	}
	# Sanitize key for use as a filename
	# / cannot be used in a filename is it is a directory seperator
	# <> are legal in filenames (but very confusing), but they will
	# mess up any URL's that point to them.
	regsub -all {[/<>]} $val "." val

	# If key is too large, use a hash instead
	if {[string length $val] > 200} {
	    package require md5
	    set val [::md5::md5 -hex $val]
	}
	set options($key) $val
    }

    constructor {args} {
	$self configurelist $args

	# Control Chart Constants
	# http://www.qimacros.com/formulas.html
	switch $options(-s) {
	    2  {set A2 1.880; set D3 0.000; set D4 3.267; set E2 2.660;}
	    3  {set A2 1.023; set D3 0.000; set D4 2.575; set E2 1.772;}
	    4  {set A2 0.729; set D3 0.000; set D4 2.282; set E2 1.457;}
	    5  {set A2 0.577; set D3 0.000; set D4 2.115; set E2 1.290;}
	    6  {set A2 0.483; set D3 0.000; set D4 2.004; set E2 1.184;}
	    7  {set A2 0.419; set D3 0.076; set D4 1.924; set E2 1.109;}
	    8  {set A2 0.373; set D3 0.136; set D4 1.864; set E2 1.054;}
	    9  {set A2 0.337; set D3 0.184; set D4 1.816; set E2 1.010;}
	    10 {set A2 0.308; set D3 0.223; set D4 1.777; set E2 0.975;}
	    11 {set A2 0.285; set D3 0.256; set D4 1.744;}
	    12 {set A2 0.266; set D3 0.283; set D4 1.717;}
	    13 {set A2 0.249; set D3 0.307; set D4 1.693;}
	    14 {set A2 0.235; set D3 0.328; set D4 1.672;}
	    15 {set A2 0.223; set D3 0.347; set D4 1.653;}
	    16 {set A2 0.212; set D3 0.363; set D4 1.637;}
	    17 {set A2 0.203; set D3 0.378; set D4 1.622;}
	    18 {set A2 0.194; set D3 0.391; set D4 1.608;}
	    19 {set A2 0.187; set D3 0.403; set D4 1.597;}
	    20 {set A2 0.180; set D3 0.415; set D4 1.585;}
	    21 {set A2 0.173; set D3 0.425; set D4 1.575;}
	    22 {set A2 0.167; set D3 0.434; set D4 1.566;}
	    23 {set A2 0.162; set D3 0.443; set D4 1.557;}
	    24 {set A2 0.157; set D3 0.410; set D4 1.548;}
	    25 {set A2 0.153; set D3 0.459; set D4 1.541;}
	    default {
		error "Bad sample size: $options(-s)"
	    }
	}

	if {$options(-nsigma) ne 3.0} {
	    # Convert to N sigma
	    # The tables above are all based on 3-sigma, since that's
	    # the industry standard, but changing it is straight forward.
	    # http://www.itl.nist.gov/div898/handbook/pmc/section3/pmc321.htm

	    set A2 [expr {$options(-nsigma)/3.0*$A2}]
	    if {$D3 > 0} {
		set D3 [expr {1+$options(-nsigma)/3.0*($D3-1)}]
	    }
	    set D4 [expr {1+$options(-nsigma)/3.0*($D4-1)}]
	}

        # Setup default Perfcache directory.
	if {$options(-perfcache) eq ""} {
	    if {[info exists UTF::Perfcache]} {
		set options(-perfcache) $UTF::Perfcache
	    } elseif {[info exists UTF::SummaryDir]} {
		# [FIXME] Shouldn't really be using SummaryDir here,
		# since it was intended to be just an argument to
		# WrapSummary, but it is getting used as a global
		# elsewhere.  Fix as part of Web report reorg.
		set options(-perfcache) [file join $UTF::SummaryDir perfcache]
	    } else {
		error "ControlChart: Unable to find default for -perfcache.  Please use -perfcache or set UTF::Perfcache or UTF::SummaryDir"
	    }
	}

	# Default ylabel is the same as title
	if {$options(-ylabel) eq ""} {
	    set options(-ylabel) $options(-title)
	}

	# Load existing data from disk file into mmmlist.
	set mmmlist [$self readcache]
    }

    UTF::doc {
        # [call [arg name] [method plotcontrolchart] [arg msg]]

        # Plot controlchart history data and limits.  Three files are
        # created, a full size PNG file, a PNT thumbnail, and the
        # gnuplot source file used to generate them.  The result is an
        # HTML annotated string which can be used to display the
        # resulting images.

	# [list_begin options]

        # [opt_def [arg link_text]]

	Link text for the resulting URL.

	# [list_end]
    }

    method plotcontrolchart {msg} {

 	if {![llength $mmmlist]} {
 	    UTF::Message WARN plotcontrolchart "No Data"
	    return $msg
  	} elseif {![llength $Rlist]} {
	    # We have data, but no stats.  This is probably a dummy
	    # run.  Run the controlchart on the cached data anyway.
	    $self calccontrolchart
	}

	# nativename is needed to avoid confusing gnuplot with ~ and
	# other special path components.
	set f [file nativename \
		   [file join [file dir $UTF::Logfile] $options(-key)]]

	# Make unique files in case we're looping in the same test run.
	for {set file $f; set i 1} {[file exists "$file.png"]} {incr i} {
	    set file "${f}_$i"
	}
	set plotfile "${file}.png"
	set plotfile_sm "${file}_sm.png"

	set num $options(-history)

	# small graph can use shorter limits.
	set num_s [expr {$num - 1}]

	# Offset so that the data is right-aligned.
	set ibase [expr {$num - ([llength $mmmlist] / 4)}]

        # Reformat meanlist to be readable by gnuplot.
	set i $ibase
	foreach {- mean max min} $mmmlist {
	    append mmm "$i $mean $max $min\n"
	    incr i
	}
	# Reformat Rlist to be readable by gnuplot.
	set Rs ""
	set i $ibase
	foreach R $Rlist {
	    append Rs "$i $R\n"
	    incr i
	}

        # Put gnuplot command sequence into file: $file.plot
	# Note: requires Gnuplot v4
	set t [lindex [time {
	    set G [open "$file.plot" w]
	    if {[UTF::GnuplotVersion] >= 5.0} {
		puts $G {set colors classic}
	    }
	    if {[UTF::GnuplotVersion] > 4.0} {
		puts $G {set terminal png transparent size 62,13}
		puts $G {set tmargin 0; set bmargin 0}
		puts $G {set lmargin 0; set rmargin 0.15}
	    } else {
		puts $G {set terminal png transparent; set size 0.15,0.08}
		# Gnuplot 4.0 loops if margins are zero - use mogrify below
	    }
	    puts $G {unset y2tics}
	    puts $G {unset ytics}
	    puts $G {unset xtics}
	    puts $G {unset key}
	    puts $G {unset border}
	    puts $G "set output \"$plotfile_sm\""

	    puts $G {plot \
			 "-" using 1:2:3:4 axes x1y1 title "x" with errorlines lt 1, \
			 "-" axes x1y1 notitle with lines lt 2, \
			 "-" axes x1y1 notitle with lines lt 2 \
		     }
	    puts $G "${mmm}e"
	    puts $G "0 $Ux\n$num_s $Ux\ne"
	    puts $G "0 $Lx\n$num_s $Lx\ne"

	    # For long performance tests, make the graph wider
	    set xwidth [expr $options(-history) * 0.02]
	    if {$xwidth < 1} {
		set xwidth 1
	    }
	    if {$xwidth > 4} {
		set xwidth 4
	    }

	    if {[UTF::GnuplotVersion] > 4.0} {
		puts $G "set terminal png size [expr {640*$xwidth}],480 notransparent medium"
		puts $G {set tmargin -1; set bmargin -1}
		puts $G {set lmargin -1; set rmargin -1}
	    } else {
		puts $G "set terminal png notransparent"
		puts $G "set size $xwidth,1"
	    }

	    puts $G "set title \"$options(-title) Control Chart\""
	    if {$options(-units) eq ""} {
		puts $G "set ylabel \"$options(-ylabel)\""
		puts $G "set y2label \"R\""
	    } else {
		puts $G "set ylabel \"$options(-ylabel) ($options(-units))\""
		puts $G "set y2label \"R ($options(-units))\""
	    }
	    puts $G {set y2tics}
	    puts $G {set ytics nomirror}
	    if {$options(-overridehistoricalmean) ne ""} {
		puts $G "set ytics add ([format $options(-format) $options(-overridehistoricalmean)])"
	    }
	    puts $G {set key below}
	    puts $G {set border}
	    puts $G "set output \"$plotfile\""

	    if {$UR == 0} {
		# need something to plot
		set UR 1
	    }
	    puts $G "set y2range \[0:[expr {$UR*4}]\]"
	    puts $G "set offsets 0, 0, 0, $UR"
	    puts $G {plot \
			 "-" using 1:2:3:4 axes x1y1 title "x" with errorlines lt 1, \
			 "-" axes x1y1 notitle with lines lt 5, \
			 "-" axes x1y1 notitle with lines lt 2, \
			 "-" axes x1y1 notitle with lines lt 2, \
			 "-" axes x1y2 title "R" with linespoints lt 4, \
			 "-" axes x1y2 notitle with lines lt 5, \
			 "-" axes x1y2 notitle with lines lt 3, \
			 "-" axes x1y2 notitle with lines lt 3 \
		     }
	    puts $G "${mmm}e"
	    puts $G "0 $meanmean\n$num $meanmean\ne"
	    puts $G "0 $Ux\n$num $Ux\ne"
	    puts $G "0 $Lx\n$num $Lx\ne"
	    puts $G "${Rs}e"
	    puts $G "0 $Rmean\n$num $Rmean\ne"
	    puts $G "0 $UR\n$num $UR\ne"
	    puts $G "0 $LR\n$num $LR\ne"
	    close $G
	}] 0]
	if {$t > 1000000} {
	    UTF::Message WARN "" "gnuplot script took [expr {$t / 1000000.0}]s"
	}

        # Run gnuplot on plot file.  Catch is needed because gnuplot
        # often writes to stderr.  Report any output even if we think
        # it was ok.
	set t [lindex [time {catch {exec $::UTF::Gnuplot "$file.plot"} ret}] 0]
	UTF::Message WARN "gnuplot" $ret
	if {$t > 1000000} {
	    UTF::Message WARN "" "gnuplot exec took [expr {$t / 1000000.0}]s"
	}

        # Add thumb-nail & full-size png files to results msg.

	if {[UTF::GnuplotVersion] <= 4.0} {
	    # Trim white space from the thumnail to make it as small
	    # as possible.  Not needed for recent Gnuplots.
	    if {[catch {exec /usr/bin/mogrify -transparent white \
			    -colors 3 -trim $plotfile_sm} ret]} {
		UTF::Message WARN "mogrify" $ret
	    }
	}

	# If we got this far gnuplot worked, so we can save a little
	# bit of space by removing the source file.
	file delete "$file.plot"

	subst {html:<a href="[UTF::URI $plotfile]">[UTF::ThumbData $plotfile_sm] $msg</a>}

    }

    UTF::doc {
        # [call [arg name] [method plotfile]]

        # Retrieve the path to the last full sice plotted image.
    }

    method plotfile {} {
	return $plotfile
    }

    UTF::doc {
        # [call [arg name] [method plotfile_sm]]

        # Retrieve the path to the last small plotted image.
    }

    method plotfile_sm {} {
	return $plotfile_sm
    }

    UTF::doc {
        # [call [arg name] [method readcache]]

        # Retrieve the saved history data from a disk file and stores
        # it in the object variable mmmlist.
    }

    method readcache {} {
	set mmmlist {}; # instance variable
	set cache $options(-perfcache)/$options(-key).data
	if {[catch {open $cache r} f]} {
	    if {![regexp {no such file or directory} $f]} {
		error $f
	    }
	    # Cache file is missing, check to see if we have an
	    # upgrade path

	    # ::UTF::PerfCacheMigrate is a global mapping list
	    # {oldsubkey newsubkey ...}.  It overrides the -oldkey
	    # option
	    if {[info exists ::UTF::PerfCacheMigrate]} {
		set options(-oldkey) \
		    [string map [lreverse $UTF::PerfCacheMigrate] \
			 $options(-key)]
	    }
	    if {$options(-oldkey) ne ""} {
		set cache $options(-perfcache)/$options(-oldkey).data
		if {[catch {open $cache r} f]} {
		    if {![regexp {no such file or directory} $f]} {
			error $f
		    }
		    UTF::Message INFO "" \
			"No old cache to migrate: $options(-oldkey)"
		    return $mmmlist
		}
		UTF::Message INFO "" \
		    "Migrating cache: $options(-oldkey) -> $options(-key)"
	    } else {
		UTF::Message INFO "" "Creating new cache file: $options(-key)"
		return $mmmlist
	    }
	}
	while {[gets $f line] >= 0} {
	    if {[llength $line] != 4} {
		UTF::Message WARN "" "Discarding bad cache line: $line"
		continue
	    }
	    set mmmlist [concat $mmmlist $line]
	}
	close $f
	return $mmmlist
    }

    UTF::doc {
        # [call [arg name] [method writecache]]

        # Store the session mean min max data in a disk file for
        # future use.  This is the save history cache method.

	# Samples outside the permitted range are discarded as invalid
	# to prevent accidental cache poisoning by configuration
	# errors.
    }

    method writecache {} {

	# Ok to record nulls.  They may be ignored later, depending on -allowzero
	set RANGEMIN -1
	# Don't record loopback accidents
	set RANGEMAX 4000

	# norangecheck overrides all range checks.

	set cache $options(-perfcache)/$options(-key).data
	file mkdir $options(-perfcache)
	catch {file attributes $options(-perfcache) -permissions 02777}
	set f [open $cache w]
	catch {file attributes $cache -permissions 00666}
	foreach {session mean min max} $mmmlist {
	    # Record if within cache plausibility limits
	    if {$options(-norangecheck) ||
		($max < $RANGEMAX && $min > $RANGEMIN)} {
		puts $f "$session $mean $min $max"
	    }
	}
	close $f
    }

    UTF::doc {
        # [call [arg name] [method calccontrolchart]]

        # Calculate Control Chart, based on object variable mmmlist,
        # which is a flat list of session mean min max.  session is
        # not used.

	# This method returns key/value pairs from an array of
	# computations.
    }

    method calccontrolchart {} {

	# Clear object vars
	set Rlist {}
	set Rmean 0
	set meanmean 0

	# local vars
	set Rsum 0
	set group 0
	set meansum 0
	set meannum 0

        # Compute mean of means, mean of ranges and list of range values
	foreach {- mean min max} $mmmlist {
	    if {($max == 0 && !$options(-allowzero)) || $max eq "Inf"} {
		# Skip nulls
		continue
	    }
	    set meansum [expr {$meansum + $mean}]
	    set R [expr {$max - $min}]
	    set Rsum [expr {$Rsum + $R}]
	    append Rlist "$R\n"
	    incr group
	}
	if {!$group} {
	    # No data - nothing to do.
	    return
	}
	if {$options(-overridehistoricalmean) ne ""} {
	    set meanmean $options(-overridehistoricalmean)
	} else {
	    set meanmean [expr {$meansum/$group}]
	}
	set Rmean [expr {$Rsum/$group}]

	# Calculate X-bar and R limits
	# http://www.itl.nist.gov/div898/handbook/pmc/section3/pmc321.htm
	set Ux [expr {$meanmean + $A2*$Rmean}]
	set Lx [expr {$meanmean - $A2*$Rmean}]
	if {!$options(-norangecheck) && $Lx < 0} {
	    set Lx 0
	}
	set UR [expr {$D4*$Rmean}]
	set LR [expr {$D3*$Rmean}]

	set f $options(-format)
	UTF::Message INFO "" \
	    [format "%d samples: \[$f - $f\], \[$f - $f\]" \
		 $group $Lx $Ux $LR $UR]

        # bounds are stored in instance variables
	return
    }

    UTF::doc {
        # [call [arg name] [method boundcheck] [arg mmm]]

	# Compare the current "mean min max" triplet against the
	# controlchart bounds determined by the the historical data.

	# The comparison reports HIGH, LOW, WIDE and NARROW but does
	# not itself determine pass/fail because those criteria may
	# vary depending on the test context.  The calling test is
	# expected to make that determination.

        # If image files were plotted, links to those files are added
        # to the results.[para]
    }

    method boundcheck {mmm} {
	set mean [lindex $mmm 0]
	set min [lindex $mmm 1]
	set max [lindex $mmm 2]
	set R [expr {$max - $min}]

	# Calculate chart.  Limits are stated as instance vars
	$self calccontrolchart
	set f $options(-format)

        # Compose results text msg.
	if {$LR > 0} {
	    set msg [format \
			 "$f \[$f - $f\], range $f \[$f - $f\]" \
			 $mean $Lx $Ux $R $LR $UR]
	} else {
	    set msg [format \
			 "$f \[$f - $f\], range $f \[$f]" \
			 $mean $Lx $Ux $R $UR]
	}

        # Append control limit analysis results msg.  Actual pass/fail
        # criteria will be determined by outer test.
	if {$mean == 0 && !$options(-allowzero)} {
	    append msg " ZERO"
	} elseif {$mean < $Lx} {
	    append msg " LOW"
	} elseif {$mean > $Ux} {
	    append msg " HIGH"
	} elseif {$R > $UR} {
	    append msg " WIDE"
	} elseif {$R < $LR} {
	    append msg " NARROW"
	} else {
	    append msg " OK"
	}

	UTF::Message LOG "" $msg
	return $msg
    }

    UTF::doc {
        # [call [arg name] [method addsample] [arg mmm]]

        # Main entry point from other scripts.  This method takes the
        # current sample of performance data, merges the new data with
        # the cached historical data, saves the data back to disk and
        # then calls other methods that compute statistical results
        # and limit criteria.[para]

        # This method returns a string containing the statistical
        # analysis and a hyperlink to the graphic files, if any.[para]

        # [arg mmm] is flat list of numeric values in the order: mean
        # min max [para]

        # Typically the raw performance data (from iperf) is run
        # through the method UTF::MeanMinMax to get the data into the
        # desired "mean min max" format.
    }

    method addsample {mmm} {

	# Add new sample to values from cache.
	foreach {mean min max} $mmm {
	    lappend mmmlist $UTF::SessionID $mean $min $max
	}

        # Remove earlier values, keeping last (-history) datapoints.
	set mmmlist [lreplace $mmmlist 0 end-[expr {4*$options(-history)}]]

        # Save data to perfcache disk file.
	if {!$readonly} {
	    $self writecache
	} else {
	    UTF::Message LOG "" "Skipping cache update"
	}

        # Pass data to statistical analysis
	$self boundcheck $mmm
    }

    UTF::doc {
        # [call [arg name] [method stats] [arg var]]

	# Retrieve stats information from the ControlChart object and
	# return it in the array pointed to by [arg var].  Returned
	# data includes means and limits, but not the original sample
	# data.
    }
    method stats {stats} {
	foreach k {meanmean Rmean Ux Lx UR LR group} {
	    upvar $stats ret
	    eval set ret(\$k) \$$k
	}
    }

    UTF::doc {
	# [list_end]
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::ControlChart man]

UTF::doc {
    # [list_end]
}


UTF::doc {
    # [section WARNING]
    # This API is experimental and is changing rapidly.
    # [section REFERENCES]
    # Ishikawa, Kaoru (1990); (Translator: J. H. Loftus); Introduction to Quality Control; 448 p; ISBN 4-906224-61-X [uri http://worldcat.org/oclc/61341428 {OCLC 61341428}]
    # [para]
    # NIST/SEMATECH e-Handbook of Statistical Methods [uri http://www.itl.nist.gov/div898/handbook/]
    # [para]
    # QI Macros SPC Formulas [uri http://www.qimacros.com/formulas.html]

    # [see_also [uri APdoc.cgi?UTF::MemChart.tcl UTF::MemChart]]
    # [see_also [file Test/random.test]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
