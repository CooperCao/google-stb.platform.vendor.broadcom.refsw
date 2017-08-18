#!/bin/env utf
# -*-tcl-*-

# Macro to merge multiple RvR directory CVS files into multiple
# composite RvR graphs.

# online help
set x [lindex $argv 0]
set x [string tolower $x]
set x [string trim $x]
set x [string range $x 0 1]
# puts "x=$x"
if {$x == "-h" || $x == "/?"} {
    puts "\nUse: $argv0 \[dir1\] \[dir2\] <dir3> ... <dirN>"
    puts "\nCopies all .csv files from each input directory to"
    puts "pwd. Then matches up .csv files by chanspec and direction"
    puts "info from filenames and merges the .csv data to create"
    puts "composite .png files." 
    exit 1
}

# Need 2 directories
if {$argc < 2} {
    puts "ERROR: need 2 input directories minimum!"
    exit 1
}

# If required, collect all CVS files from input directories.
set csv_list ""
set dir_cnt 0
set nocopy 0 ;# option for future expansion.
if {$nocopy == 0} {
    # Copy CSV files from input directories
    foreach dir $argv {

        # Check directory
        # puts "\n\ndir=$dir"
        if {[file isdirectory "$dir"]} {
            incr dir_cnt
        } else {
            puts "\nERROR: $dir is not a directory!"
            continue
        }

        # Get list of CSV files
        set file_list [glob -nocomplain -directory $dir *.csv]
        # puts "file_list=$file_list"
        set cnt [llength $file_list]
        if {$cnt == 0} {
            puts "\nERROR: no CSV files in $dir"
            incr dir_cnt -1
            continue
        }

        # Copy CSV files adding N_ prefix to ensure files dont overwrite each other.
        foreach file $file_list {
            # puts "\n\nfile=$file"
            set dest [file tail "$file"]
            set dest "./${dir_cnt}_${dest}"
            # puts "dest=$dest"
            set catch_resp [catch "file copy -force \"$file\" \"$dest\"" catch_msg]
            if {$catch_resp == 0} {
                lappend csv_list $dest
            } else {
                puts "\nERROR: copy $dest got: $catch_msg"
            }
        }
    }
} else {
    # Get CSV files from pwd.
    set dir [pwd]
    puts "Looking in $dir for CSV files"
    set csv_list [glob -nocomplain -directory $dir *.csv]
}

# Check we got 2 or more valid directories.
if {$dir_cnt < 2 && $nocopy == 0} {
    puts "\nERROR: $dir_cnt valid directories found, need 2 minimum!"
    exit 1
}

# Check we got 2 or more CSV files.
set cnt [llength $csv_list]
# puts "cnt=$cnt csv_list=$csv_list"
if {$cnt < 2} {
    puts "\nERROR: $cnt CSV files found, need 2 minimum!"
    exit 1
} else {
    puts "\nFound $cnt CSV files in [pwd]." 
    # puts "csv_list=$csv_list"
}

# Select sets of files to merge based on chanspec & direction.
set png_cnt 0
set merge_tool "[file dirname $argv0]/merge_rvr_data.tcl"
foreach chan "24G20 24G40 5G20 5G40 5G80" {
    foreach dir "Upstream Downstream" {

        # Look for files that match chan & dir pair
        set merge_list "" ;# has filename & title in pairs
        foreach file $csv_list {
            # puts "FILE $chan $dir $file"
            if {[regexp -nocase $chan $file] && [regexp -nocase $dir $file]} {
                # puts "MATCH $chan $dir $file"
                lappend merge_list $file
                # Extract sta name from filename, use non-greedy regexp.
                # This info is used as the titles / legend text on the composite graph.
                if {[regexp -nocase {(\d+)_\d+_([^_]+)_rvr} $file - k sta]} {
                    lappend merge_list "${k}_${sta}_${chan}_${dir}"
                } else {
                    lappend merge_list "${chan}_${dir}"
                }
            }
        }

        # Did we get at least 4 files & titles to merge?
        # puts "\nchan=$chan dir=$dir merge_list=$merge_list" 
        if {[llength $merge_list] < 4} {
            puts "\nWARN: not enough CSV files to merge for $chan $dir"
            continue
        }

        puts "\nMerging $chan $dir $merge_list"
        set catch_resp [catch "eval exec $merge_tool $merge_list" catch_msg]
        # set catch_resp 0 ;# test code
        # set catch_msg " 1 1 1 1 1 errorlines 4319win7_24G20_Upstream 4 2 1 1 1 args=cleanedges\n\
            12:38:58  LOG   mc46end1   gnuplot_rvr_lines created /home/brearley/ComboThroughput.png\n\
            All done, 0 errors" ;# test code
        if {$catch_resp == 0} {
            # Parse out .png location
            if {[regexp {gnuplot_rvr_lines\s+created\s+(\S+)\n} $catch_msg - png_path]} {
                # Copy .png to pwd.
                # puts "png_path=$png_path"
                set dest "[pwd]/${chan}_${dir}.png"
                set catch_resp [catch "file copy -force \"$png_path\" \"$dest\"" catch_msg]
                if {$catch_resp == 0} {
                    incr png_cnt
                } else {
                    puts "\nERROR: copy $png_path $dest got: $catch_msg"
                }
            } else {
                puts "ERROR: could not find .png path in: $catch_msg"
            }
        } else {
            puts "ERROR: $merge_tool got: $catch_msg"
        }
    }
}

# All done.
puts "$argv0 all done. Created $png_cnt .png files in [pwd]"
exit 0












