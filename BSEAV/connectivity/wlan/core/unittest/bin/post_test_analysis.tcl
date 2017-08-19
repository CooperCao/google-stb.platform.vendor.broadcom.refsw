#! /usr/bin/tclsh

# Sample script to do selective post test analysis of log files.
# This script is expected to be customized by each user, possibly
# for each testrig.

# Calling parameters will log directory, name of running script,
# option pairs used by the running script formated as: name1=value1
# name2=value2 ... nameN=valueN
# There may be embedded spaces in these pairs, eg: {sta=a1 b2 c3}
# puts "argv=$argv"
set log_dir [lindex $argv 0]
set script  [lindex $argv 1]

# If you want the failure report for the tests just run, use:
# set fail_report "$log_dir/fail.html"
# if {[file exists "$fail_report"]} {
#     puts "found $fail_report OK."
# } else {
#     puts "$fail_report NOT found!"
# }

# Selectively run the log2graphs tool for 15 minute stress tests.
# Put the results on stdout, so calling script can see the results.
if {[regexp {15min=1|15min20=1} $argv]} {
    set resp [exec ~/utf_dev/src/tools/unittest/Test/log2graphs.test $log_dir]
    puts "$resp"
}

