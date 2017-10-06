#!/bin/env utf
# -*-tcl-*-

# Routine to merge multiple RvR CSV files into one CSV file and plot
# a composite RvR comparison graph.
# $Id$
# $Copyright Broadcom Corporation$
#

# Keep pkg_mkIndex happy
# puts "__package_orig=[info command __package_orig]"
if {[info command __package_orig] != ""} {
    set ::argv ""
}

package require UTF
package require UTF::utils

#==================== dump_graph ==================================
# Routine uses gnuplot_rvr_lines to plot composite graph from the
# output .CSV file.
#
# Calling parameters: none
# Returns: graph pathname
#==================================================================
proc dump_graph { } {

    # Build list of data columns to be graphed.
    # .CSV file is formated: loss, mean1, min1, max1, ... meanN, minN, maxN,
    set list ""
    for {set i 1} {$i <= $::files_cnt} {incr i} {
        # Retrieve title & column data from stats_array
        set title $::stats_array($i,title)
        set mean_col $::stats_array($i,mean_col)
        set min_col  $::stats_array($i,min_col)
        set max_col  $::stats_array($i,max_col)

        # If we have min & max data, use errorlines.
        set style linespoints
        if {$min_col >= 0 && $max_col >= 0} {
            set style errorlines
        }

        # Loss column is: 0
        # Mean data columns are at: 1, 4, 7, ...
        set data_col [expr ($i * 3) - 2]
        set lt $i
        set lw 1
        set pt 1
        set ps 1
        append list " $style \"$title\" $data_col $lt $lw $pt $ps"
    }

    # Set fastrampup option for common graph routine
    if {$::fastrampup} {
        set fr fastrampup
    } else {
        set fr ""
    }

    # Call common graph routine.
    if {[info exists ::testrig]} {
        set title "$::testrig Composite $::devicenames"
    } else {
        set title "Composite $::devicenames"
    }
    UTF::Message DEBUG "" "MergeRvrData: gnuplot_rvr_lines cmdline: $::out_file $title $::transportname $::rvr_pathloss $list $fr"
    set graph [UTF::gnuplot_rvr_lines $::out_file "ComboThroughput" "$title"\
        "-" "Throughput $::transportname (Mbit/sec)" "$::rvr_pathloss" $list cleanedges $fr]
    UTF::Message INFO "" "MergeRvrData: gnuplot_rvr_lines returned: $graph"

    if {[regexp {no_data} $graph]} {
	UTF::Message ERROR "" "MergeRvrData: Graph not generated: $graph data=$::out_file"
        incr ::errors
    }

    UTF::Message INFO "" "MergeRvrData: returning to caller: $graph"
    return $graph
}

#==================== dump_stats_array ============================
# Routine that dumps the stats_array data on the output .CSV file.
#
# Calling parameters: none
# Returns: OK
#==================================================================
proc dump_stats_array { } {

    # Create header line, adding 3 columns per input file.
    puts $::out "Merged RvR data"
    set header "Loss,"
    for {set i 1} {$i <=$::files_cnt} {incr i} {
        append header " $::stats_array($i,title) mean, min, max,"
    }
    puts $::out "$header"

    # Loop thru stats_array, collecting all data for a given loss.
    # First access data in down / ascending loss order, then access
    # data by up / descending loss order.
    # NB: Not all elements of the array will be defined, so access carefully.
    # NB: Each element in the array can be a string of values, so we need to
    # generate multiple lines as necessary.
    set out_cnt 0
    foreach {dir lim1 lim2 step} "down 0 $::max_loss 1 up $::max_loss 0 -1" {
        for {set j $lim1} {($dir == "down" && $j <= $lim2) || ($dir == "up" && $lim2 <= $j)} {incr j $step} {
            # Each file can have a different number of data points stored.
            # Find the maximum number of mean data points at this loss.
            # The number of min/max data points will be the same number.
            set max_k 1
            for {set i 1} {$i <=$::files_cnt} {incr i} {
                if {[info exists ::stats_array($i,$j,$dir,mean)]} {
                    set temp $::stats_array($i,$j,$dir,mean)
                    set cnt [llength $temp]
                    if {$cnt > $max_k} {
                        set max_k $cnt
                    }
                }
            }

            # Generate an output line for each data sample at this loss
            for {set k 0} {$k < $max_k} {incr k} {
                set line "$j,"
                set found_data no
            
                for {set i 1} {$i <=$::files_cnt} {incr i} {
                    if {[info exists ::stats_array($i,$j,$dir,mean)]} {
                        set mean $::stats_array($i,$j,$dir,mean)
                        set mean [lindex $mean $k] ;# get k'th value of string
                        set found_data yes
                    } else {
                        set mean "0"
                    }
                    if {[info exists ::stats_array($i,$j,$dir,min)]} {
                        set min $::stats_array($i,$j,$dir,min)
                        set min [lindex $min $k] ;# get k'th value of string
                        set found_data yes
                    } else {
                        set min "0"
                    }
                    if {[info exists ::stats_array($i,$j,$dir,max)]} {
                        set max $::stats_array($i,$j,$dir,max)
                        set max [lindex $max $k] ;# get k'th value of string
                        set found_data yes
                    } else {
                        set max "0"
                    }
                    append line " $mean, $min, $max,"
                }

                if {$found_data == "yes"} {
                    puts $::out "$line"
                    incr out_cnt
                }
            }
        }
    }

    # Clean up.
    catch {flush $::out}
    catch {close $::out}

    return OK
}

#==================== get_line ====================================
# Routine reads a line of data from the current input file.
#
# Calling parameters: none
# Returns: OK, ERROR
#==================================================================
proc get_line { } {

    # Get line of data.
    incr ::line_cnt ;# file line number
    set line ""
    set catch_resp [catch {gets $::in ::line} catch_msg]
    if {$catch_resp != 0} {
	UTF::Message ERROR "" "MergeRvrData: Reading input ::in=$::in ::line_cnt=$::line_cnt catch_msg=$catch_msg"
        incr ::errors
        return ERROR
    }
    set ::line [string trim $::line]
    return OK
}

#==================== help ========================================
# Routine that gives online help.
#
# Calling parameters: args
# Returns: OK or exits script.
#==================================================================
proc help {args} {

    # Check if help was requested or not.
    set x [lindex $args 0]
    set x [string tolower $x]
    set x [string range $x 0 1]
    if {[string compare $x "-h"] != 0 && [string compare $x "/?"] != 0} {
       return OK
    }

    # Give help
    puts "Basic usage: tclsh $::argv0 \[file1 title1 file2 title2\] ... <fileN titleN> <options>"
    puts " "
    puts "options:"
    puts "-fastrampup      Handle fastrampup test"
    puts "-nominmax        Show only mean data, hide min/max data"
    puts "-rampdownonly    Show only rampdown data, hide rampup data"
    puts " "
    puts "Merges two or more .CSV files with RvR data into a composite file and"
    puts "plots the composite throughtput RvR graph. Titles are text used on"
    puts "the graph legend."
    exit 1
}

#==================== Parse_Line_Store_Stats ======================
# Routine that parses a line of data from the input file and stores
# the stats in stats_array.
#
# Calling parameters: none
# Returns: OK, ERROR, STOP
# Sets various global variables.
#==================================================================
proc parse_line_store_stats { } {

    # Get line of data.
    set resp [get_line]
    if {$resp == "ERROR"} {
        return ERROR
    }

    # Ignore blank lines, symmetry data
    if {$::line == "" || [regexp {rvr\s*symmetry\s*errors} $::line]} {
        return OK
    }

    # Get desired fields from line
    set fields [split $::line ","]
    set loss [lindex $fields 0]
    set loss [string trim $loss]
    if {$loss == ""} {
        incr ::errors
        return ERROR
    }
    if {$loss > $::max_loss} {
        set ::max_loss $loss
    }

    # Have we changed from rampdown to rampup data?
    if {$::direction == "down" && $loss <= $::previous_loss} {
        if {$::rampdownonly == 1} {
            return STOP
        }
        set ::direction up
    }
    set ::previous_loss $loss

    # With the advent of the fastrampup test, there can be be multiple 
    # values at the same loss. We store the a string of space separated
    # values for each loss, instead of a single value.
    set mean [lindex $fields $::tput_mean_col]
    append ::stats_array($::files_cnt,$loss,$::direction,mean) " $mean"
    if {$::tput_min_col >= 0} {
        set min  [lindex $fields $::tput_min_col]
        append ::stats_array($::files_cnt,$loss,$::direction,min) " $min"
    }
    if {$::tput_max_col >= 0} {
        set max  [lindex $fields $::tput_max_col]
        append ::stats_array($::files_cnt,$loss,$::direction,max) " $max"
    }

    # Store data in stats_array
    return OK
}

#==================== setup_basics ================================
# Routine that does basic setup, parses command line tokens, opens
# the running output .CSV file.
#
# Calling parameters: args
# Returns: OK or exits script.
#==================================================================
proc setup_basics {args} {

    # Initialize counters
    set ::errors 0
    set ::files_cnt 0
    set ::max_loss 0
    set ::warnings 0
    set ::rvr_pathloss "Relative"

    # Check two pairs of file names & titles.
    set args [join $args " "]
    set len [llength $args]
    if {$len < 4} {
	UTF::Message ERROR "" "MergeRvrData: You must specify 2 input files & 2 titles minimum"
	exit 1
    }

    # Create temp output file at local /tmp. Filetype is .csv so Excel will read the file without any arguing.
    # A file with name like /tmp/tmp.j6kXYjTyCc.csv will be created.
    set catch_resp [catch {exec mktemp --suffix .csv} ::out_file]

    # Open output file. Existing file, if any, is blown away.
    set catch_resp [catch {set ::out [open $::out_file w]} catch_msg]
    if {$catch_resp != 0} {
	UTF::Message ERROR "" "MergeRvrData: Could not open $::out_file: catch_resp=$catch_resp catch_msg=$catch_msg"
	# Remove temp before existing
	if {[file exists "$::out_file"]} {
	    catch {file delete $::out_file}
 	}
	exit 1
    }

    # Clean out ::stats_array, in case data was left from a previous run.
    # This can happen if a higher level routine sources this script and runs it more than once.
    set names [array names ::stats_array]
    foreach item $names {
	unset ::stats_array($item)
    }
    return OK
}

#==================== setup_file ==================================
# Opens the current input file, sets up variables, parses out
# throughput column numbers.
#
# Calling parameters: file title
# Returns: ERROR, OK
#==================================================================
proc setup_file {file title} {

    # Because UTF only supports Linux, there is no point having
    # support for Windows pathnames with backslash in here.

    # Check input file exists.
    if {![file exists "$file"]} {
	UTF::Message ERROR "" "MergeRvrData: file not found: $file"
	incr ::errors
	return ERROR
    }

    # Open input file.
    set catch_resp [catch {set ::in [open $file r]} catch_msg]
    if {$catch_resp != 0} {
	UTF::Message ERROR "" "MergeRvrData: could not open $file catch_msg=$catch_msg"
        incr ::errors
        return ERROR
    }

    # Setup global variables, stats array and first data row for this input file.
    # Read up to 10 lines looking for header column names.
    set found_header no
    set ::direction down
    set ::line_cnt 0
    set ::previous_loss ""
    set ::tput_mean_col -1
    set ::tput_min_col -1
    set ::tput_max_col -1
    for {set j 0} {$j < 10} {incr j} {
        get_line
        set fields [split $::line ","]
        set fields [string tolower $fields]
        set i -1
        foreach field $fields {
            incr i
            if {[regexp {tput\s*mean|endpoint.*average} $field]} {
                set ::tput_mean_col $i
                set found_header yes
                continue
            }
            if {[regexp {tput\s*min|endpoint.*min} $field]} {
                set ::tput_min_col $i
                set found_header yes
                continue
            }
            if {[regexp {tput\s*max|endpoint.*max} $field]} {
                set ::tput_max_col $i
                set found_header yes
                continue
            }

            # Watch for Total/Estimated Path Loss
            if {[regexp -nocase {(total|estimated)\s+path\s+loss} $field]} {
                set ::rvr_pathloss "Estimated"
                continue
            }
        }
        if {$found_header == "yes"} {
            break
        }
    }

    # If necessary, use old format column 14.
    if {$::tput_mean_col < 0} {
        set ::tput_mean_col 14
    }

    # Save data in stats_array.
    incr ::files_cnt
    set ::stats_array($::files_cnt,title) $title
    set ::stats_array($::files_cnt,mean_col) $::tput_mean_col
    if {$::nominmax == 1} {
        # Dont collect min/max data.
        set ::tput_min_col -1
        set ::tput_max_col -1
    }
    set ::stats_array($::files_cnt,min_col) $::tput_min_col
    set ::stats_array($::files_cnt,max_col) $::tput_max_col
    return OK
}

#==================== Main Program ================================
# This is the main test program. 
#==================================================================
UTF::Test MergeRvrData {args} {

    # Check if online help was requested
    help $args

    # Extract -rampdownonly, -nominmax & -fastrampup options.
    set new_args ""
    set ::fastrampup 0
    set ::nominmax 0
    set ::rampdownonly 0
    set ::devicenames "" 
    foreach arg $args {
        if {[regexp {^-} $arg]} {
            if {$arg == "-fastrampup"} {
                set ::fastrampup 1
            } elseif {$arg == "-nominmax"} {
                set ::nominmax 1
            } elseif {$arg == "-rampdownonly"} {
                set ::rampdownonly 1
            } else {
                error "ERROR: invalid option: $arg"
            }
	} elseif {[regexp {^devicenames:(.*)} $arg - sub]} {
	    set ::devicenames $sub
	} elseif {[regexp {^transport:(.*)} $arg - sub]} {
	    set ::transportname $sub
	} elseif {$arg == ""} {
	    continue
        } else {
            lappend new_args $arg
        }
    }
    set args $new_args

    # Initialization routine
    setup_basics $args

    # Process each file from command line tokens
    foreach {file title} $args {
        # Setup to read this file.
        set resp [setup_file $file $title]
        if {$resp == "ERROR"} {
            continue
        }

        # Now we process this input file data one line at a time.
        while {![eof $::in]} {
           # Read a line of input file, parse it, store stats
           set response [parse_line_store_stats]
           if {$response == "ERROR" || $response == "STOP"} {
               break
           }
        }

        # All done for this file.
        catch {close $::in}
    }

    # Dump stats_array to output .CSV file
    dump_stats_array

    # Generate composite graph.
    set graph [dump_graph]
    UTF::Message DEBUG "" "MergeRvrData: returned from dump_graph: $graph"

    # Thats it!
    UTF::Message INFO "" "MergeRvrData: removing temp file $::out_file"
    catch {file delete $::out_file}
    #puts "All done, $::errors errors"
    return $graph
}
