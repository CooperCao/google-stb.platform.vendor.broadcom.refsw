#!/usr/local/bin/perl
#
# Script to report CVS checkin activity on active branches
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$

use strict;
use Getopt::Long;

my $rowcolor;
my %seen;
my $today = qx(date '+%Y-%m-%d'); chomp($today);
my $current_user=getpwuid($<);

# Derive active branches from build_config.sh
my $active_branches=qx(/home/hwnbuild/src/tools/build/build_config.sh -a -q);
my @active_branches=sort split(/\s+/,$active_branches);
my $cvsreports_dir="/projects/hnd_software/cvsreport/Software";
my $branchreports_dir="/projects/hnd/swbuild/build_admin/logs/branch_activity";
my $branch_plans_url="http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BranchPlans";
my %branches;       # Temporary var
my %other_branches; # Branches that haven't been marked as active branch
my @branches;       # Temporary var
my @sub_branches;   # Sub branches found while searching cvsreports_dir
my %processed;      # Processed active branch index
my $report;         # Final output report
my @report_branches;# final list of branches to report on
my $dbg;            # Debug mode flag
my $csv;            # NOT implemented yet, meant to generate excel/csv file
my $output;         # Output report file name
my $branch;         #
my $show_twigs;     # Include or exclude flag for branch=>twig derivation
my @other_branches; # Additional branches to report on
                    # By default active branches from build_config.sh and
                    # their twigs are selected for reporting
my %branch_data;    # Branch data for a given date
my @all_dates=();   # List of all possible dates in last period (1 year)

# Number of days the report needs to run for
# my @periods=qw(1 7 15 21 30 60 90 120 150 180 365); # in days
# my @periods=qw(1 7 15 21 30 60 90); # in days for 3 months
my @periods=qw(1 7 15 21 30 60); # for 2 months
# Map number of days to meaningful names in report
my %period_name=(
	1 	=> "1 Day",
	7 	=> "1 Week",
	15 	=> "2 Weeks",
	21 	=> "3 Weeks",
	30 	=> "1 Month",
	60 	=> "2 Months",
	90 	=> "3 Months",
	120 	=> "4 Months",
	150 	=> "5 Months",
	180 	=> "6 Months",
	365 	=> "1 Year"
);

my %branch_category=(
	# PCOEM categories
	PBR		=> qw(PCOEM),
	BASS		=> qw(PCOEM),
	KIRIN		=> qw(PCOEM),
	# MEDIA categories
	SUZUKI		=> qw(MEDIA),
	DUCATI		=> qw(MEDIA),
	# ROUTER categories
	COMANCHE2	=> qw(ROUTER),
	COMANCHE	=> qw(ROUTER),
	MILLAU		=> qw(ROUTER),
	AKASHI		=> qw(ROUTER),
	# MOBILITY categories
	RAPTOR		=> qw(MOBILITY),
	RAPTOR2		=> qw(MOBILITY),
	RAPTOR3		=> qw(MOBILITY),
	ROMTERM		=> qw(MOBILITY),
	ROMTERM2	=> qw(MOBILITY),
	ROMTERM3	=> qw(MOBILITY),
	F15		=> qw(MOBILITY),
	F16		=> qw(MOBILITY),
	F17		=> qw(MOBILITY),
	FALCON		=> qw(MOBILITY),
);

# Command line options
my %scan_args = (
                'dbg'           	=> \$dbg,
                'csv'           	=> \$csv,
                'output=s'      	=> \$output,
                'report=s'      	=> \$output,
                'twigs'         	=> \$show_twigs,
                'other_branches=s'    	=> \@other_branches,
);

# Scan the command line arguments
GetOptions(%scan_args) or &Error("GetOptions failed!");

$show_twigs=1;   # Include branch=>twig derivation

if (!$output) {

	if ($current_user =~ /hwnbuild/) {
		$report="${branchreports_dir}/${today}.htm"
	} else {
		$report="branch_activity_${today}.htm"
	}
} else {
		$report=$output;
}
rename("$report","${report}.old$$") if ( -s "$report");

# Debug ()
sub Dbg {
        my ($msg) = @_;

        return if (!$dbg);

        print STDOUT "DBG: $msg\n";
}

#
# Generate a list of possible date strings in the reporting period
#
sub genDates
{
	my $tmonth;
	my $tday;
	my $tdate;
	my $cur_year=qx(date '+%Y'); chomp($cur_year);
	my $prev_year=$cur_year - 1;
	my $cur_month=qx(date '+%m'); chomp($cur_month);
	my $prev_month=$cur_month;
	my $cur_day=qx(date '+%d'); chomp($cur_day);
	my $prev_day=$cur_day;
	my $cur_date="${cur_year}_${cur_month}_${cur_day}";
	my $prev_date="${prev_year}_${cur_month}_${cur_day}";
	my $start_period=0;
	my $end_period=0;

	## Generate a list of all valid dates between start and end dates
	for (my $y=$prev_year; $y<=$cur_year; $y++) {
		for (my $m=1; $m<=12; $m++) {
			for (my $d=1; $d<=31; $d++) {
				$tmonth = ( $m < 10 ) ? "0$m" : "$m";
				$tday   = ( $d < 10 ) ? "0$d" : "$d";
				$tdate  = "${y}_${tmonth}_${tday}";
				$start_period=1 if ($tdate eq $prev_date);
				$end_period=1 if ($tdate eq $cur_date);
				last if ($end_period==1);

				if ($start_period && !$end_period) {
					unshift(@all_dates, $tdate);
				} else {
					next;
				}
			}
		}
	}
	unshift(@all_dates, $cur_date);
	#foreach (@all_dates) { print "$_\n"; }
}

#
# Find twigs from given active branches
#
sub SearchTwigs {
	my $syear;

	# Search all available branches/twigs and group them by branch prefix
	if (opendir(BRANCHES,"$cvsreports_dir")) {
		my $cvsbranch;
		my $cvsdate;
		my $branch_prefix;
		foreach my $CvsReport (readdir(BRANCHES)) {
			next unless ($CvsReport =~ /_BRANCH_|_TWIG_/);
			($cvsdate,$cvsbranch) = ($CvsReport =~ m%(\d{4}_\d{2}_\d{2})_([^.]*).html$%);
			next if ($cvsbranch =~ /^(t|b)/);
			$branch_data{$cvsbranch}{$cvsdate}=$CvsReport;
			$branch_prefix=(split(/_/,$cvsbranch))[0];
			next if (exists $processed{$cvsbranch});
			push(@{$branches{$branch_prefix}},$cvsbranch);
			$processed{$cvsbranch}=1;
			# TODO: This will include COMPONENT branches or any misc branches
			# TODO: that managers may want filtered out
	 		if (grep { ! /^${cvsbranch}$/ } @active_branches) {
				push(@other_branches,$cvsbranch);
	 		}
		}
	}

	# Process Active branches first
	foreach my $branch (@active_branches) {
	 	my $branch_prefix;
	 	($branch_prefix)=(split(/_/,$branch))[0];
	 	if (grep { /^${branch_prefix}$/ } keys %branches) {
	 		push(@sub_branches,@{$branches{$branch_prefix}});
	 		Dbg("Full Project=$branch_prefix; Found branches=@{$branches{$branch_prefix}}");
	 	}
	}

	# Mark sub-branches also as activie. Actual filtering will take out branches
	# that don't have checkins in the reporting period
	push(@active_branches,@sub_branches);
	push(@active_branches,@other_branches) if (@other_branches);

	# Show branch Vs. found cvs report on a given date
	#disabled# foreach my $branch (sort keys %branch_data) {
	#disabled#  	foreach my $date (reverse sort keys %{$branch_data{$branch}}) {
	#disabled#  		Dbg("$branch = $date [$branch_data{$branch}{$date}]");
	#disabled# 	}
	#disabled# }

} # SearchTwigs()

#
# Search cvs reports and collect branch checkin activity numbers
#
sub CollectBranchData {
	foreach my $branch (@report_branches) {
		chomp($branch);

		next if ($branch =~ /NIGHTLY/);

		Dbg("==== START: PROCESSING BRANCH=$branch =====");
		foreach my $period (@periods) {
	
			my @cvsreports_period=();
			my $num_checkins=0;

			Dbg("---- START: BRANCH=$branch; PERIOD=$period ----");
			for (my $day=1; $day <= $period; $day++) {
				my $cvsreport_day="$cvsreports_dir/$all_dates[$day]_${branch}.html";
				# Dbg("Searching [$branch]: $cvsreport_day");
				if ( -s "$cvsreport_day" ) {
					Dbg("Found [branch=$branch][period=$period]: report=$all_dates[$day]_${branch}.html");
					push(@cvsreports_period,$cvsreport_day);
					
				}
			}
			if (scalar(@cvsreports_period)) {
				$num_checkins=qx(/home/prakashd/bld/report_cvs_checkins -r $branch -n $period @cvsreports_period);
				chomp($num_checkins);
				my $num_of_reports=scalar(@cvsreports_period);
				Dbg("Number of Reports [branch=$branch][period=$period][num_of_checkins=$num_checkins]: $num_of_reports");
			}
	
			$branch_data{$branch}{$period}=$num_checkins;
			# If number of checkins in any given period is greater than 0, then include the branch/twig for reporting
			if($num_checkins > 0) {
				$branch_data{$branch}{include_in_report}=1;
			}
			Dbg("---- END  : BRANCH=$branch; PERIOD=$period ----");
		}
		Dbg("==== END  : PROCESSING BRANCH=$branch =====");
		Dbg("");
	}

} # CollectBranchData()

#
# Print HTML page header
#
sub HtmlHeader {
	# Output HTML page header
	print "<html xml:lang=\"en\" lang=\"en\">
	<head>
	 <title> CVS Branch Activity </title>
	 <meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\" />
	</head>
	<body bgcolor=\"#ffffff\">\n";

} # HtmlHeader()

#
# Print HtML table header
#
sub TableHeader {
	print "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\"> \n";
	print "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b><u>WLAN S/W Branch Checkin Activity (Cumulative data generated on $today) </u></b></td></tr>\n";
	print "</table>\n";
	print "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\">\n";

	# START: Header/Title Line
	print "<b><tr style=\"font-size: 10pt; background-color: black; color: #ffffff\">\n";
	print "<td style=\"font-size: 12pt\"><b>Branch Name</b></td>\n";
	print "<td style=\"font-size: 12pt\"><b>Category</b></td>\n";

	foreach my $period (@periods) {
		print "<td style=\"font-size: 12pt\"><b>$period_name{$period}</b></td>\n";
	}
	print "</tr></b>\n";
	# END: Header/Title Line

} # TableHeader()

#
# Search cvs reports and report branch activity numbers
#
sub HtmlBranchData {
	foreach my $branch (@report_branches) {
		chomp($branch);

		next if ($branch =~ /NIGHTLY/);
		next unless (exists $branch_data{$branch}{include_in_report});
		my $branch_prefix=(split(/_/,$branch))[0];

		# Toggle row color
		$rowcolor = $rowcolor ? "background-color\: #FFFFCC" : "";
		print "<tr style=\"$rowcolor; font-size: 12pt\">\n";
		if ($branch =~ /_BRANCH_/) {
			print "<td style=\"font-size: 12pt\"><a href=\"${branch_plans_url}#$branch\">${branch}</a></td>\n";
		} else {
			print "<td style=\"font-size: 8pt\"><a href=\"${branch_plans_url}#$branch\">${branch}</a></td>\n";
		}
		print "<td style=\"font-size: 12pt\">$branch_category{$branch_prefix}</a></td>\n";

		foreach my $period (@periods) {
			if ($branch_data{$branch}{$period} > 0 ) {
				print "<td style=\"font-size: 12pt\">$branch_data{$branch}{$period}</td>\n";
			} else {
				print "<td style=\"font-size: 12pt\">-</td>\n";
			}
		}
		print "</tr>\n";
		$rowcolor = $rowcolor ? "" : "background-color\: #FFFFCC";
	}

} # HtmlBranchData()

#
# Html tail page
#
sub HtmlTail {
	print "</table>\n</body>\n</html>\n";

} # HtmlTail()

#
# Html tail page
#
sub EndReport {
	if ($current_user =~ /hwnbuild/) {
		chdir("$branchreports_dir");
		unlink("index.htm");
		print "\nSym-linking ${report} as main index.htm\n";
		system("ln -s ${report} index.htm");
	} else {
		print "\nGenerated report is at $report file\n";
	}

} # EndReport()

sub Main {

	print "\n";
	Dbg("Branch Activity Reporting for $today");
	print "\n";

	genDates;

	SearchTwigs();

	@report_branches = sort grep { !$seen{$_}++ } @active_branches;

	CollectBranchData();

	open(OUTPUT, ">$report") || die "Can't create report file $report!!\n";
	select(OUTPUT);
	HtmlHeader();
	TableHeader();
	HtmlBranchData();
	HtmlTail();
	close(OUTPUT);

	select(STDOUT);
	EndReport();


} # Main()

&Main();
