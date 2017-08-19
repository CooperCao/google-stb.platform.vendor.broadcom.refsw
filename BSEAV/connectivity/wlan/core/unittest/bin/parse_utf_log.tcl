#!/usr/bin/tclsh

# Written by John Brearley 2008

# Routine to parse a UTF log file for selected data and error messages
# and reformat data into Excel CSV format file to create pretty Excel charts.

#==================== Script Data Definitions =====================
# Define the default information to be used by this script.
#
# Please feel free to change the data definitions in this section
# of the script.
#==================================================================

# Define the error message strings to look for. Add as many as
# necessary to arry msg. Each msg creates a new line on the graph,
# so dont go overboard or the graph will be unreadable. Case
# insensitive wildcard matching is done for the error messages.

set msg(0) "send_bar"
set msg(1) "rx.*fifo.*overflow"
set msg(2) "transaction.*mismatch"
set msg(3) "ampdu_recv.*off"

#==================== End of Data Definitions =====================
# Do NOT change any anything below here!!
#==================================================================


#==================== Global Variables ============================
# Define the global variables that are set & used elsewhere in the
# script. 
#==================================================================
set errors 0  ;# overall error counter
set in "" ;# file handle for input file
set line "" ;# input buffer for data read from input file
set out "" ;# file handle for output CSV file
set out_file "" ;# name of output CSV file
set previous_malloc_fail "" ;# keep track of malloc failures
set null_results 0 ;# counter for lines that have no result data
set stats_array_max 0 ;# number of rows used in stats array
set warnings 0 ;# overall warning counter


#==================== Cleanup =====================================
# Routine that cleans up when script is done. 
#
# Calling parameters: none
# Returns: exits the script.
#==================================================================
proc cleanup { } {
   global out out_file errors warnings 

   # Flush & close the output file
   catch "flush $out"
   catch "close $out"
   puts "All done, $errors errors, $warnings warnings,  see stats in file: $out_file"
   exit $errors
}


#==================== Dump_Graph ==================================
# Routine that dumps the data needed for the graphs of the current
# input file.
#
# Calling parameters: none
# Returns: OK
#==================================================================
proc dump_graph { } {
   global chart_title in_file msg out stats_array stats_array_max 

   # Add header info
   puts $out "\n=========================================================================\n"
   puts $out "Chart Title: $chart_title" 
   puts $out "Chart Tab Name: [file rootname $in_file]"
   puts $out "Chart Type: xlLine\n"

   # Create list of keys to access stats array. Order here determines order shown
   # on Excel graph.
   set key_list "tput_max tput_mean tput_min iperf_samples malloc_fail nrate\
      rate rssi txmpduperampdu rxmpduperampdu tx_mcs rx_mcs"
   set i 0
   while { 1 } {
      if {[info exists msg($i)]} {
         append key_list " msg$i"
         incr i
      } else {
         break
      }
   }
   # puts "key_list=$key_list"

   # Trace code - dump sorted list of indices
   # set indices [array names stats_array]
   # set indices [lsort $indices]
   # puts "indices=$indices"

   # Dump all the stats array contents except last row, which is normally not
   # populated with any data.
   foreach key $key_list {
      set key [string trim $key]
      set out_str ""
      for {set i 0} {$i < $stats_array_max} {incr i} { 
         set out_str "${out_str} $stats_array($i,$key),"
      }
      puts $out "$out_str"
   }
   return "OK"
}


#==================== Get_Line ====================================
# Routine reads a line of data from the current input file.
#
# Calling parameters: none
# Returns: OK, ERROR
#==================================================================
proc get_line { } {
   global errors in in_file line line_cnt

   # Get line of data.
   incr line_cnt ;# file line number
   set line ""
   set catch_resp [catch "gets $in line" catch_msg]
   if {$catch_resp != 0} {
      puts "ERROR: reading input in_file=$in_file line_cnt=$line_cnt catch_msg=$catch_msg"
      incr errors
      return ERROR
   } 
   # puts "line_cnt=$line_cnt line=$line"
   return OK
}


#==================== Help ========================================
# Routine that gives online help.
#
# Calling parameters: none
# Returns: OK or exits script.
#==================================================================
proc help { } {
   global argv0 argv msg

   # Check if help was requested or not.
   set x [lindex $argv 0]
   set x [string tolower $x]
   set x [string range $x 0 1]
   if {[string compare $x "-h"] != 0 && [string compare $x "/?"] != 0} {
      return OK
   }

   # Give help
   puts "Basic usage: tclsh $argv0 \[file1\] <file2> ... <fileN>"
   puts " "
   puts "Parses the specified UTF log file(s) to generate statistics." 
   puts "The output file will be called file_name.csv. You can use"
   puts "creat_excel_charts.pl to have Excel graph the data."
   puts " "
   puts "The throughput, rssi, nrate, rate, malloc_failures and"
   puts "counts for user defined error messages are extracted from"
   puts "the log files, if they are present:"
   set i 0
   while { 1 } {
      if {[info exists msg($i)]} {
         puts "msg($i): $msg($i)"
         incr i
      } else {
         break
      }
   }
   puts " "
   puts "Feel free to edit the header of this script and change the"
   puts "user defined error messages that are tracked.  You may have"
   puts "instrument controlchart.test to add trace data for items like"
   puts "rssi, malloc failures, etc."
   exit 1
}


#==================== Parse_Line_Store_Stats ======================
# Routine that parses a line of data from the input file and stores
# the stats in stats_array.
#
# Calling parameters: none
# Returns: OK, ERROR
# Sets various global variables.
#==================================================================
proc parse_line_store_stats { } {
   global chart_title errors in in_file line line_cnt null_results\
      msg previous_malloc_fail stats_array stats_array_max warnings

   # Get line of data.
   set resp [get_line]
   if {$resp == "ERROR"} {
      return ERROR
   }

   # The controlchart sample set line determines the end of one sample
   # set and the start of another. We ignore sample set 0 as it is the
   # start of the data.
   if {[regexp {controlchart\s*getting\s*sample\s*set\s*(\d+)} $line - x1]} {
      # Setup a new data row for the next data sample.
      # puts "x1=$x1 line=$line"
      if {$x1 != "0"} {
         setup_stats_array_row
      }
      return OK
   }

   # Process "Total Results" line in the log file.
   if {[regexp -nocase {total.*results:(.*)} $line - x1]} {

      # The first results line usually has no data, as it is a calibration
      # run. If there is more than one line like this, something is wrong.
      # set x1 "" ;# test code
      set x1 [string trim $x1]
      # puts "line_cnt=$line_cnt x1=$x1"
      if {$x1 == ""} {
         incr null_results
         if {$null_results > 1} {
            puts "ERROR: got null results, null_results=$null_results line_cnt=$line_cnt"
            incr errors
            return ERROR
         }
         return OK
      }

      # Save the results data
      set resp [store_mean_min_max $x1]
      if {$resp == "ERROR"} {
         return ERROR
      }
   }

   # Watch for line that can be used for chart title
   if {$chart_title == ""} {
      if {[regexp -nocase {info.*controlchart(.*)} $line - x1]} {
         set chart_title $x1
         return OK
      }
   }

   # Watch for response to nrate command. 
   if {[regexp -nocase {mcs index (\d+).*mode} $line - x1]} {
      set stats_array($stats_array_max,nrate) $x1
      return OK
   }

   # Watch for response to rate command.
   if {[regexp -nocase {(\d+) mbps$} $line - x1]} {
      set stats_array($stats_array_max,rate) $x1
      return OK
   }

   # Watch for response to rssi command. 
   # Looks like:16:45:16  LOG   pb4atst3   -41
   # NB: Store absolute value to simplify the graphs.
   if {[regexp -nocase {^\d\d:\d\d:\d\d\s+log\s+\w+\s+(-\d+)$} $line - x1]} {
      set stats_array($stats_array_max,rssi) [expr abs($x1)]
      return OK
   }

   # Watch for malloc failures in response to mu command
   if {[regexp -nocase {malloc failure count:\s*(\d+)} $line - x1]} {
      # puts "malloc_fail=$x1, line_cnt=$line_cnt previous_malloc_fail=$previous_malloc_fail"

      # Depending on the logfile being parsed, the malloc fail count
      # may already be greater than 0. Normally the device driver remains
      # active for a series of tests and is not reset between each test.
      # So the malloc fail counter is usually cummulative for the complete
      # sequence of tests.
      if {$previous_malloc_fail == ""} {
         set previous_malloc_fail $x1
      }

      # Watch for counter wrap and/or new values out of sequence from 
      # previous value. This warning can be triggered by log files being
      # processed out of original sequence.
      # set x1 [expr $previous_malloc_fail - 2] ;# test code
      if {$x1 < $previous_malloc_fail} {
         puts "WARNING malloc_fail=$x1, previous_malloc_fail=$previous_malloc_fail\
            line_cnt=$line_cnt"
         incr warnings
         set previous_malloc_fail 0
      }
  
      # Store delta from last malloc check. The malloc count belongs to 
      # the previous sample set of data, as the mu command that is of
      # interest is the one after the iperf results. 
      set malloc_delta [expr $x1 - $previous_malloc_fail]
      set previous_malloc_fail $x1
      set row [expr $stats_array_max - 1]
      # puts "stats_array_max=$stats_array_max row=$row malloc_delta=$malloc_delta"
      if {$row > 0} {
         set stats_array($row,malloc_fail) $malloc_delta
      }
      return OK
   }

   # Look for txmpduperampdu ratio
   if {[regexp -nocase {txmpduperampdu\s*(\d+)} $line - x1]} {
      set stats_array($stats_array_max,txmpduperampdu) $x1
      return OK
   }

   # Look for rxmpduperampdu ratio
   if {[regexp -nocase {rxmpduperampdu\s*(\d+)} $line - x1]} {
      set stats_array($stats_array_max,rxmpduperampdu) $x1
      return OK
   }

   # Look for TX MCS distribution. For the time being, we want
   # only the data from the last cell in the table. Unfortuneately,
   # this is on the following line of data.
   if {[regexp -nocase {TX\s*MCS} $line - x1]} {
      # Get line of data.
      set resp [get_line]
      if {$resp == "ERROR"} {
         return ERROR
      }
      # puts "TX MCS line=$line"
      if {[regexp {(\d+)\s*\([^(]+\)$}  $line - x1]} {
         # puts "x1=$x1"
         set stats_array($stats_array_max,tx_mcs) [expr int($x1/100)]
      }
      return OK
   }

   # Look for RX MCS distribution. For the time being, we want
   # only the data from the last cell in the table. Unfortuneately,
   # this is on the following line of data.
   if {[regexp -nocase {RX\s*MCS} $line - x1]} {
      # Get line of data.
      set resp [get_line]
      if {$resp == "ERROR"} {
         return ERROR
      }
      # puts "RX MCS line=$line"
      if {[regexp {(\d+)\s*\([^(]+\)$}  $line - x1]} {
         # puts "x1=$x1"
         set stats_array($stats_array_max,rx_mcs) [expr int($x1/100)]
      }
      return OK
   }

   # Look for one of the user defined msgs and increment the 
   # appropriate counter
   set i 0
   while { 1 } {
      if {[info exists msg($i)]} {
         set pattern $msg($i)
         if {[regexp -nocase $pattern $line]} {
            # puts "matched i=$i pattern=$pattern line_cnt=$line_cnt line=$line"

            # Artificially raise the counts for ampdu msgs so they will be
            # visible on the graphs
            if {[string match -nocase *ampdu* $line]} {
                set stats_array($stats_array_max,msg$i) [expr $stats_array($stats_array_max,msg$i) + 5]

            # Overflow msgs have a count value that we need to extract
            } elseif {[string match -nocase *overflow* $line]} {
                regexp -nocase {:\s+(\d+)\s+} $line - x1
                if {![info exists x1] || $x1 == ""} {
                   puts "WARNING could not extract overflow count, line_cnt=$line_cnt"
                   incr warnings
                } else {
                   # puts "overflow x=$x1 line_cnt=$line_cnt"
                   if {$x1 > 100} {
                      set x1 100 ;# limit so graph y-axis wont become huge
                   }
                   set stats_array($stats_array_max,msg$i) [expr $stats_array($stats_array_max,msg$i) + $x1]
                }

            } else {
               incr stats_array($stats_array_max,msg$i)
            }
            return OK
         }
         incr i
      } else {
         break
      }
   }
   return OK
}


#==================== Setup_Basics ================================
# Routine that does basic setup, parses command line tokens, opens
# the input file and running output logfile.
#
# Calling parameters: none
# Returns: OK or exits script.
#==================================================================
proc setup_basics { } {
   global argc argv0 argv earliest_date out out_file

   # Check one or more input file name(s) were specified on the 
   # command line tokens.
   if {$argc == 0} {
      puts "ERROR: You must specify one or more input file(s). \nFor more info, type: tclsh $argv0 -h"
      exit 1
   }

   # Create output file name. Filetype is .csv so Excel will read
   # the file without any arguing.
   set out_file [file rootname $argv0]
   set out_file "${out_file}.csv"

   # Open output file. Existing file, if any, is blown away.
   set catch_resp [catch "set out \[open \"$out_file\" w\]" catch_msg]
   if {$catch_resp != 0} {
      puts "ERROR: could not open $out_file catch_msg=$catch_msg"
      exit 1
   }
   # puts "out_file=$out_file out=$out"
   return OK
}


#==================== Setup_Stats_Array ===========================
# Routine that initializes the main stats array.
#
# Calling parameters: none
# Returns: OK
#==================================================================
proc setup_stats_array { } {
   global msg stats_array stats_array_max

   # Data from the previous input file is blown away each time we 
   # intialize an new row in the array.
 
   # Setup fixed title info in stats_array row 0.
   set stats_array_max 0
   set stats_array(0,tput_min) Tput_Min
   set stats_array(0,tput_mean) Tput_Mean
   set stats_array(0,tput_max) Tput_Max
   set stats_array(0,iperf_samples) Iperf_Samples
   set stats_array(0,malloc_fail) Malloc_Fail
   set stats_array(0,nrate) nrate
   set stats_array(0,rate) rate
   set stats_array(0,rssi) rssi
   set stats_array(0,txmpduperampdu) txmpduperampdu
   set stats_array(0,rxmpduperampdu) rxmpduperampdu
   set stats_array(0,tx_mcs) tx_mcs
   set stats_array(0,rx_mcs) rx_mcs

   # Setup user defined title info in stats_array row 0.
   set i 0
   while { 1 } {
      if {[info exists msg($i)]} {
         set stats_array(0,msg$i) $msg($i)
         incr i
      } else {
         break
      }
   }

   # Trace code - dump sorted list of indices
   # set indices [array names stats_array]
   # set indices [lsort $indices]
   # puts "indices=$indices"
   return OK
}

#==================== Setup_Stats_Array_Row =======================
# Routine that initializes the active row for collecting stats in
# the main stats array.
#
# Calling parameters: none
# Returns: OK
#==================================================================
proc setup_stats_array_row { } {
   global msg stats_array stats_array_max line_cnt

   # NB: Excel doenst like rows of items set to null, so always
   # initialize to 0. 

   # Initialize new row in stats array for fixed items.
   incr stats_array_max
   # puts "line_cnt=$line_cnt initializing stats_array row=$stats_array_max"
   set stats_array($stats_array_max,tput_min) 0
   set stats_array($stats_array_max,tput_mean) 0
   set stats_array($stats_array_max,tput_max) 0
   set stats_array($stats_array_max,iperf_samples) 0
   set stats_array($stats_array_max,malloc_fail) 0
   set stats_array($stats_array_max,nrate) 0
   set stats_array($stats_array_max,rate) 0
   set stats_array($stats_array_max,rssi) 0
   set stats_array($stats_array_max,txmpduperampdu) 0
   set stats_array($stats_array_max,rxmpduperampdu) 0
   set stats_array($stats_array_max,tx_mcs) 0
   set stats_array($stats_array_max,rx_mcs) 0

   # Initialize user defined items in new row.
   set i 0
   while { 1 } {
      if {[info exists msg($i)]} {
         set stats_array(${stats_array_max},msg$i) 0
         incr i
      } else {
         break
      }
   }
   return OK
}

#==================== Store_Mean_Min_Max ==========================
# Routine that gives online help.
#
# Calling parameters: space separated list of numeric samples
# Returns: OK, ERROR
#==================================================================
proc store_mean_min_max { samples } {
   global errors in_file line_cnt stats_array stats_array_max 


   # Compute mean, min & max for samples
   set cnt 0
   set min ""
   set max ""
   set total 0.0
   foreach point $samples {

      if {![regexp {^[\.\d]+$} $point]} {
         puts "ERROR: non-numeric data, samples=$samples, in_file=$in_file line_cnt=$line_cnt"
         incr errors
         return ERROR
      }

      incr cnt
      set total [expr {$total + $point}]

      if {$min == ""} {
         set min $point
      } elseif {$point < $min} {
         set min $point
      }

      if {$max == ""} {
         set max $point
      } elseif {$point > $max} {
         set max $point
      }
   }

   if {$cnt == 0} {
      set mean 0
   } else {
      set mean [expr $total / $cnt]
   }

   # Store results in stats array
   # puts "line_cnt=$line_cnt samples=$samples mean=$mean min=$min max=$max"
   set stats_array($stats_array_max,tput_mean) $mean
   set stats_array($stats_array_max,tput_min) $min
   set stats_array($stats_array_max,tput_max) $max
   set stats_array($stats_array_max,iperf_samples) $cnt
   return OK
}


#==================== Main Program ================================
# This is the main program. 
#==================================================================

# Check if online help was requested
help

# Initialization routine
setup_basics

# Process each file from command line tokens
foreach in_file $argv {

   # Save current errors counter. This lets us detect if any errors
   # occurred for this specific input file.
   set saved_errors $errors

   # Convert \ to / in file path.
   set in_file [split $in_file \\]
   set in_file [join $in_file /]

   # Check input file exists.
   if {![file exists "$in_file"]} {
      puts "ERROR: file $in_file not found!"
      incr errors
      continue
   }

   # Open input file.
   set catch_resp [catch "set in \[open \"$in_file\" r\]" catch_msg]
   if {$catch_resp != 0} {
      puts "ERROR: could not open $in_file catch_msg=$catch_msg"
      incr errors
      continue
   }
   puts "in_file=$in_file in=$in"

   # Allow for binary msgs in log file
   fconfigure $in -translation binary

   # Setup global variables, stats array and first data row for this
   # input file.
   set chart_title ""
   set line_cnt 0
   set null_results 0
   setup_stats_array
   setup_stats_array_row

   # Now we process this input file data one line at a time.
   while {![eof $in]} {

      # Read a line of input file, parse it, store stats
      set response [parse_line_store_stats]
      if {$response == "ERROR"} {
         break
      }
   }

   # If this file had 0 errors, dump the graph data for this
   # input file. There are files that dont contain control
   # charts, which are filtered out by lack of a title.
   # incr errors ;#test code
   if {$saved_errors == $errors} {
      if {$chart_title != ""} {
         dump_graph
      }
   } else { 
      puts "ERRORS occurred for $in_file, graphs NOT generated for $in_file"
   }

   # We are done with this input file.
   catch "close $in"
}

# Need closing line to keep perl script happy.
puts $out "\n=========================================================================\n"

# Thats it!
cleanup

