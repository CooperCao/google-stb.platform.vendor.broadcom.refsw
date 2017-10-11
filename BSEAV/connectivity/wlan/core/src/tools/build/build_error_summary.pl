#!/usr/bin/perl
##
## Script to scan build summary emails and generate a consolidate
## report.
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##

use     Env;
use     Getopt::Long;

##
## Command line arguments supported by this program. Start and enddates
## provide a date range to generate report on
##
## startdate    :   Start date in yyyy.m.d format
## enddate      :   End   date in yyyy.m.d format
## output       :   Output file name (default: STDOUT)
##

%scan_args = (
                'branch=s'      => \$branch,
                'startdate=s'   => \$startdate,
                'enddate=s'     => \$enddate,
                'output=s'      => \$output,
                'dbg'           => \$dbg,
                'help'          => \$help,
             );

# Scan the command line arguments
GetOptions(%scan_args) or &Error("GetOptions failed!");

use lib  "/home/hwnbuild/src/tools/build";
require  "wl_swengg_lib.pl";

$summarylogdir = "/projects/hnd/swbuild/build_admin/logs/summary";
$branch = "NIGHTLY" if (!$branch);

if ($branch =~ /NIGHTLY/) {
	$branch_alias = "TOT";
} else {
	$branch_alias = $branch;
}

if ( ! $startdate || ! $enddate ) {
   &Error3("-startdate or -enddate option is missing!!");
   &Error3("NOTE: Use yyyy.m.d format for start or end date");
   &Error3("NOTE: e.g: 2006.1.15 or 2006.10.18 are all valid dates");
   &Exit($ERROR);
}

$startdate =~ s/\s+//g; $enddate   =~ s/\s+//g;

if ( $startdate !~ /(\d{4})\.(\d{1,2})\.(\d{1,2})$/ ) {
   &Error3("Use -startdate yyyy.m.d format");
   &Exit($ERROR);
} else {
   $syear=$1; $smonth=$2; $sday=$3;
}

if ( $enddate !~ /(\d{4})\.(\d{1,2})\.(\d{1,2})$/ ) {
   &Error3("Use -enddate yyyy.m.d format");
   &Exit($ERROR);
} else {
   $eyear=$1; $emonth=$2; $eday=$3;
}

$startyear     = $syear;
$startmonth    = ( $smonth < 10 ) ? "0$smonth" : "$smonth";
$startday      = ( $sday   < 10 ) ? "0$sday"   : "$sday";
$startdatetemp = "${startyear}${startmonth}${startday}";

$endyear     = $eyear;
$endmonth    = ( $emonth < 10 ) ? "0$emonth" : "$emonth";
$endday      = ( $eday   < 10 ) ? "0$eday"   : "$eday";
$enddatetemp = "${endyear}${endmonth}${endday}";

if ( $enddatetemp < $startdatetemp ) {
   &Error3("Use enddate should be later than startdate");
   &Exit($ERROR);
}

&Dbg("start date = $startdatetemp");
&Dbg("end date   = $enddatetemp");
&Dbg("START : $syear,$smonth,$sday");
&Dbg("END   : $eyear,$emonth,$eday");

$allsummaries="SUMMARY_${branch}_${startdate}_${enddate}.txt";

## Generate a list of all valid dates between start and end dates
$sflag = $eflag = 0;
@dates     = ();
for ( $y=$syear; $y<=$eyear; $y++ ) {
    for ( $m=1; $m<=12; $m++ ) {
        for ( $d=1; $d<=31; $d++ ) {
            $tmonth = ( $m < 10 ) ? "0$m" : "$m";
            $tday   = ( $d < 10 ) ? "0$d" : "$d";
            $tdate  = "${y}.${m}.${d}";
            $tdate2 = "${y}${tmonth}${tday}";
            if ( "$tdate2" == "$startdatetemp" ) { $sflag = 1; $eflag = 0; }
            if ( $sflag && !$eflag ) { push(@dates, $tdate); }
            if ( "$tdate2" == "$enddatetemp" )   { $eflag = 1; $sflag = 0 }
        }
    }
}

system("rm -fv $allsummaries");
foreach $date ( @dates ) {
  if ( -f "${summarylogdir}/${branch}_${date}.0.txt" ) {
     system("cat ${summarylogdir}/${branch}_${date}.0.txt >> $allsummaries");
  }
}

if ( ! $output ) { 
   $OP = "STDOUT";
} else {
   open (REPORT,">$output") || die "Cann't open output = $output file";
   $OP = "REPORT";
}

print $OP "HND WLAN SOFTWARE, BUILD SUMMARY REPORT for $branch\n\n";
print $OP "START-DATE: $startdate, END-DATE: $enddate\n\n";
open (ALL_SUMMARIES,"$allsummaries") || die "Can not open generated $allsummaries file!";

%months = ( 
             Jan      => "01",
             January  => "01",
             Feb      => "02",
             February => "02",
             Mar      => "03",
             March    => "03",
             Apr      => "04",
             April    => "04",
             May      => "05",
             Jun      => "06",
             June     => "06",
             Jul      => "07",
             July     => "07",
             Aug      => "08",
             August   => "08",
             Sep      => "09",
             September=> "09",
             Oct      => "10",
             October  => "10",
             Nov      => "11",
             November => "11",
             Dec      => "12",
             December => "12",
);

$failed = 0;
$errors = 0;
$begin  = 0;
$end    = 0;
while (<ALL_SUMMARIES>) {
  chomp;
  next if ( /^\s+$/ );

  # TOT build summary (3 of 46 FAILED; 2 in progress)[Saturday 01/01/2011]
  if ( $_ =~  m/^\s*${branch_alias}\s+build\s+summary\s+\((\d+)\s+of\s+(\d+)\s+FAILED[\)\;]/gi ) {
     &Dbg("Start new FAILED summary : $_");
     $failedbuilds = $1; $total = $2;
     $failed = 1; $errors = 0;
     $failedbrand =""; $failedbrandtmp = "";

     ($date) = ($_ =~ /${branch_alias}\s+build\s+summary\s+.*\[\w+\s+(.*?)\]/);
     ($month,$day,$year) = ($date =~ /(\d+)\/(\d+)\/(\d+)/);
     $daystr = "${year}${month}${day}";
     &Dbg("date=$date; month=$month; day=$day; year=$year;");

     next;
  } elsif ( $_=~ m/^\s*${branch_alias}\s+build\s+summary\s+\(all\s+(\d+)\s+passed\)/gi ) {
     &Dbg("Start new PASSED summary : $_");
     $failedbuilds = 0; $total = $1;
     $failed = 0; $errors = 0;
     $failedbrand =""; $failedbrandtmp = "";

     ($date) = ($_ =~ /${branch_alias}\s+build\s+summary\s+.*\[\w+\s+(.*?)\]/);
     ($month,$day,$year) = ($date =~ /(\d+)\/(\d+)\/(\d+)/);
     $daystr = "${year}${month}${day}";
     &Dbg("date=$date; month=$month; day=$day; year=$year;");

     next;
  }

  if ( $failed && /:\s*FAILED/ ) {
     if ( /,release.log does not exist/ ) {
       if (  /\/projects\/hnd/ ) {
          ($failedbrandtmp) = (split(/\//))[6];
       } else {
          ($failedbrandtmp) = (split(/\s+/))[0];
       }
       if ( $failedbrandtmp =~ /${branch}|\d{4}\.\d{1,2}\.\d{1,2}/ ) {
          &Dbg("Bad brand $failedbrandtmp found on $daystr. Skipping it");
          #$summary{$daystr}{total}        -= 1 if ( exists($summary{$daystr}{total}) );
          #$summary{$daystr}{failedbuilds} -= 1 if ( exists($summary{$daystr}{failedbuilds}) );
          $failedbrandtmp = "BAD_BRAND($failedbrandtmp)";
       }
       $errors = X;
     } else {
       ($failedbrandtmp,$errors) = (split(/\s+/))[0,5];
     }
     $failedbrandtmp =~ s/://g;
     $failedbrand .= "${failedbrandtmp}($errors) ";
     $failurecount{$failedbrandtmp} += 1;
     &Dbg("failedbrand = $failedbrand");
  }

  if ( ! exists($summary{$daystr}{total}) ) {
     $summary{$daystr}{total} = $total ? $total : 0;
  } else {
     # &Dbg("summary{$daystr}{total} exists");
  }

  if ( ! exists($summary{$daystr}{failedbuilds}) ) {
     $summary{$daystr}{failedbuilds} = $failedbuilds ? $failedbuilds : 0;
  } else {
     # &Dbg("summary{$daystr}{failedbuilds} exists");
  }

  $summary{$daystr}{failedbrand}  = $failedbrand  if ( $failedbrand );
} # ALL_SUMMARIES

printf $OP "%-30.30s, %-14.14s, %-14.14s, %s\n","DATE","TOTAL BUILDS","FAILED BUILDS","FAILED BRANDS DETAILS (errors)";

foreach $day ( sort { $b <=> $a } keys %summary ) {
    next if ( $day =~ /^\s*$/ );
    $day =~ m/(\d\d\d\d)(\d\d)(\d\d)/g;
    $nday = "$2/$3/$1";
    printf $OP "%-30.30s, %2d, %2d, %s\n",$nday, $summary{$day}{total}, $summary{$day}{failedbuilds}, $summary{$day}{failedbrand};
}

foreach $d ( keys %summary ) { $sumtotal += $summary{$d}{total};        }
foreach $d ( keys %summary ) { $sumfails += $summary{$d}{failedbuilds}; }

printf $OP "%-30.30s, %d, %d, %s (Failure Rate = %-2.2f%)\n","TOTALS", $sumtotal, $sumfails, "FOR ALL BRANDS", $sumfails*100/$sumtotal;

print $OP "\n\nBRAND-NAME, NUMBER OF TOTAL FAILURES\n";
foreach $brand ( sort { $failurecount{$b} <=> $failurecount{$a} } keys %failurecount ) {
   print $OP "$brand, $failurecount{$brand}\n";
}
close(ALL_SUMMARIES);
close($OP);

&Info();
&Info("Build summary output generated in $output");
&Info("Now you can open $output file in MS Excel");
&Info();

sub Usage {
   &Info();
   &Info("$0 -startdate <start-date> -enddate <end-date> [-output <outputfile> -help -dbg -branch <branch-name>]");
   &Info("NOTE: Output generated is stored in excel readable .csv format");
   &Info("NOTE: Use yyyy.m.d format for start or end date");
   &Info("NOTE: e.g: perl build_error_summary.pl -start 2006.1.15 -end 2006.10.18");
   &Info();
   &Exit($OK);
}

&Exit($OK);
