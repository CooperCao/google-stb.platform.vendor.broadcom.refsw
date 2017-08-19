##
## Little script to scan current build environment and exclude certain
## env variables (by value or by name) based on arguments provided
## 
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## $Id$
#
# SVN: $HeadURL$
#
local $exclude_by_values = "";
local @exclude_by_values = ();
local $exclude_by_names  = "";
local @exclude_by_names  = ();
local @exclude_list      = ();
local %seen1             = ();
local %seen2             = ();

while ($ARGV[0] =~ /^-(.)(.*)$/) {
	$arg = shift @ARGV;
	if ( $arg =~ /^-exclude_by_values/ ) {
	    $exclude_by_values = shift @ARGV;
	    if ( ! $exclude_by_values || $exclude_by_values =~ /^-/ ) {
	       print "ERROR: -exclude_by_values needs env values to strip\n"; \
	       print "ERROR: e.g: -exclude_by_values \"msdev installshield\"\n";
	       exit 1;
	    } else {
	       @exclude_by_values = sort grep { !$seen1{$_}++ } split(/\s+/, $exclude_by_values);
	       #print "exclude_by_values = @exclude_by_values\n";
	    }
	}
	if ( $arg =~ /^-exclude_by_names/ ) {
	    $exclude_by_names = shift @ARGV;
	    if ( ! $exclude_by_names || $exclude_by_names =~ /^-/ ) {
	       print "ERROR: -exclude_by_names needs var names to strip\n"; \
	       print "ERROR: e.g: -exclude_by_names \"msdev installshield\"\n";
	       exit 1;
	    } else {
	       @exclude_by_names = sort grep { !$seen2{$_}++ } split(/\s+/, $exclude_by_names);
	       #print "exclude_by_names = @exclude_by_names\n";
	    }
	}
}

foreach $var ( keys %ENV ) {
   push(@exclude_list,$var) if grep { m/$var/gi } @exclude_by_names;
   foreach $value ( @exclude_by_values ) {
      if ( $ENV{$var} =~ m/$value/gi ) {
         push(@exclude_list,$var);
      }
   }
}

foreach $var ( @exclude_list ) {
   print "$var\n";
}
