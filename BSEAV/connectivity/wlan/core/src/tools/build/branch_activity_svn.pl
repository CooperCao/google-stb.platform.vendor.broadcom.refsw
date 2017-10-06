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
my $SVN="/tools/bin/svn";
my $SVNROOT="http://svn.sj.broadcom.com/svn/wlansvn/proj";

# Derive active branches from build_config.sh
my $active_branches  = qx(/home/hwnbuild/src/tools/build/build_config.sh -a -q);
my @active_branches  = sort split(/\s+/,$active_branches);
my $branchreports_dir="/projects/hnd/swbuild/build_admin/logs/branch_activity_svn";
my $branch_plans_url ="http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BranchPlans";
my @platforms        = qw(linux macos netbsd windows);
my @built_branches; # List of branches for which builds exist

my %branches;       # Temporary var
my @branches;       # Temporary var
my @sub_branches;   # Sub branches found
my %processed;      # Processed active branch index
my $report;         # Final output report_
my @report_branches;# final list of branches to report on
my $dbg;            # Debug mode flag
my $csv;            # NOT implemented yet, meant to generate excel/csv file
my $output;         # Output report file name
my $branch;         #
my $show_twigs;     # Include or exclude flag for branch=>twig derivation
my @all_dates=();   # List of all possible dates in last period (1 year)
my %all_checkins;
my %found_checkins;
my %branch_info;

# Number of days the report needs to run for
# my @requested_periods=qw(1 7 15 21 30 60 90 120 150 180 365); # in days
# my @requested_periods=qw(1 7 15 21 30 60 90 120 150 180); # for last 6 months
# my @requested_periods=qw(1 7 15 21 30 60 90); # in days for 3 months
# my @requested_periods=qw(1 7 15 21 30 60); # for 2 months
# my @requested_periods=qw(1 15 30);
# my @requested_periods=qw(1 15 30 60 90); # in days for 3 months
# my @requested_periods=qw(1 15 30 60 90 120 180); # in days for 6 months
my @requested_periods=qw(15 30 60 90 180); # in days for 6 months
my $longest_period=$requested_periods[$#requested_periods];
print "longest_period = $longest_period\n";
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
	AARDVARK	=> qw(PCOEM),
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
	RT2TWIG46	=> qw(MOBILITY),
	F15		=> qw(MOBILITY),
	F16		=> qw(MOBILITY),
	F17		=> qw(MOBILITY),
	FALCON		=> qw(MOBILITY),
	FALCONPHY	=> qw(MOBILITY),
	PHOENIX		=> qw(MOBILITY),
	PHOENIX2	=> qw(MOBILITY),
);

# Command line options
my %scan_args = (
                'dbg'           	=> \$dbg,
                'csv'           	=> \$csv,
                'output=s'      	=> \$output,
                'report=s'      	=> \$output,
                'twigs'         	=> \$show_twigs,
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
	my $cur_date="${cur_year}-${cur_month}-${cur_day}";
	my $prev_date="${prev_year}-${cur_month}-${cur_day}";
	my $start_period=0;
	my $end_period=0;

	## Generate a list of all valid dates between start and end dates
	for (my $y=$prev_year; $y<=$cur_year; $y++) {
		for (my $m=1; $m<=12; $m++) {
			for (my $d=1; $d<=31; $d++) {
				$tmonth = ( $m < 10 ) ? "0$m" : "$m";
				$tday   = ( $d < 10 ) ? "0$d" : "$d";
				$tdate  = "${y}-${tmonth}-${tday}";
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
	# for (my $i=0; $i<$all_dates_num; $i++) { print "Date = $all_dates[$i]\n"; }
}

#
# Find twigs from given active branches
#
sub SearchTwigs {
	my $syear;

	my (@SVN_BRANCHES) = map { $_ =~ s%/%%g; $_ } split(/\s+/,qx($SVN list ${SVNROOT}/branches));

	foreach my $branch (sort @SVN_BRANCHES) {
		my $branch_prefix=(split(/_/,$branch))[0];
		my $svninfo = qx($SVN info ${SVNROOT}/branches/$branch); chomp($svninfo);
		my ($author,$rev,$update) = ($svninfo =~ /Last Changed Author: (\w+).*Last Changed Rev: (\d+).*Last Changed Date: (\d{4}-\d{2}-\d{2})/msg);
		print "Processed: $branch\n";
		print "Rev    = $rev\n";
		print "Author = $author\n";
		print "Update = $update\n";

		next if ($rev =~ /^\s*$/);

		$branch_info{$branch}{last_rev}    = $rev;
		$branch_info{$branch}{last_update} = $update;
		$branch_info{$branch}{last_author} = $author;

		push(@{$branches{$branch_prefix}},$branch);
		$processed{$branch}=1;
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

	# Push sub branches (aka twigs) to active branches to report on
	push(@active_branches,@sub_branches);

	# Find the branches that have builds in progress
	foreach my $platform (@platforms) {
        	my $build_dir = "/projects/hnd/swbuild/build_$platform";

        	# Search branches and twigs that have builds launched recently
        	if (opendir(BUILDS,"$build_dir")) {
                	foreach my $tag (readdir(BUILDS)) {
				next unless ($tag =~ /_BRANCH_|_TWIG_/);
				next if (exists $processed{$tag});
				push(@built_branches,$tag);
				$processed{$tag}=1;
			}
		}
	}
	# Push built branches (and twigs) to active branches to report on
	push(@active_branches,@built_branches);


} # SearchTwigs()

#
# Search cvs reports and collect branch checkin activity numbers
#
sub CollectBranchData {
	foreach my $branch (@report_branches) {
		chomp($branch);

		next if ($branch =~ /NIGHTLY/);

		Dbg("==== START: PROCESSING BRANCH=$branch =====");
	
		my $num_checkins=0;

		my $start_date = $today;
		my $end_date   = $all_dates[$longest_period];
		print "start:end = {$start_date}:{$end_date}\n";
		open(SVNLOG,"$SVN log -r {$start_date}:{$end_date} ${SVNROOT}/branches/$branch|") || die "Can't run svn log";
		my $n=1;
		while(<SVNLOG>) {
			if (/^r(\d+)\s+\|\s+(\w+)\s+\|\s+([^ ]*)/) {
				my $log_rev=$1;
				my $log_author=$2;
				my $log_date=$3;
				$all_checkins{$branch}{$log_date}{$log_rev} = $log_author;
				print "[$n] LOG Entry: $branch : $log_rev; $log_author; $log_date\n";
				$n++;
			}
		}
		
		foreach my $this_period (@requested_periods) {
			Dbg("---- START: BRANCH=$branch; PERIOD=$this_period ----");
			my $this_period_cutoff = $all_dates[$this_period];
			my $this_period_count= 0;
			my $cutoff_num = $this_period_cutoff;

			$cutoff_num =~ s/-//g;

			print "[$branch] Period=$this_period Cutoff=$this_period_cutoff\n";
			foreach my $date (sort keys %{$all_checkins{$branch}}) {
				my $date_num   = $date;
				my @revs_found = sort keys %{$all_checkins{$branch}{$date}};
				my $revs_found = $#revs_found + 1;
				$date_num =~ s/-//g;
				if ( $date_num >= $cutoff_num ) {
					$this_period_count += $revs_found
				}
				print "\t[ci_count=$this_period_count] $date revs[$revs_found]: @revs_found\n";
			}
			$found_checkins{$branch}{$this_period} = $this_period_count;
			print "[$branch] FINAL Period=$this_period Checkins=$this_period_count\n\n";
			Dbg("---- END  : BRANCH=$branch; PERIOD=$this_period ----");
			if ($found_checkins{$branch}{$this_period} > 0) {
				$found_checkins{$branch}{report} = 1;
			}
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
	 <title> SVN Branch Checkin Activity </title>
	 <meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\" />
	</head>
	<body bgcolor=\"#ffffff\">\n";

} # HtmlHeader()

#
# Print HtML table header
#
sub TableHeader {
	print "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\"> \n";
	print "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b><u>WLAN S/W Branch Checkin Activity in Subversion</u></b></td></tr>\n";
	print "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b><u>Cumulative data generated on $today. This shows number of commits over different time periods.</u></b></td></tr>\n";
	print "<tr style=\"background-color: yellow; font-size: 12pt\"><td><b><i>Repository: $SVNROOT</i></b></td></tr>\n";
	print "</table>\n";
	print "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\">\n";

	# START: Header/Title Line
	print "<b><tr style=\"font-size: 10pt; background-color: black; color: #ffffff\">\n";
	print "<td style=\"font-size: 12pt\"><b>Branch Name (twiki link)</b></td>\n";
	print "<td style=\"font-size: 12pt\"><b>Segment</b></td>\n";
	print "<td style=\"font-size: 12pt\"><b>Last Update Info</b></td>\n";

	foreach my $period (@requested_periods) {
		print "<td style=\"font-size: 12pt\"><b>$period_name{$period} commits</b></td>\n";
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
		my $branch_prefix=(split(/_/,$branch))[0];

		next if ($branch =~ /NIGHTLY/);
		next unless (defined($found_checkins{$branch}{report}));
		next unless (defined($branch_info{$branch}{last_rev}));

		my $branch_info = "Last rev $branch_info{$branch}{last_rev}; $branch_info{$branch}{last_author}; $branch_info{$branch}{last_update}";

		# Toggle row color
		$rowcolor = $rowcolor ? "background-color\: #FFFFCC" : "";
		print "<tr style=\"$rowcolor; font-size: 12pt\">\n";
		if ($branch =~ /_BRANCH_/) {
			print "<td style=\"font-size: 12pt\"><a href=\"${branch_plans_url}#$branch\">${branch}</a></td>\n";
		} else {
			print "<td style=\"font-size: 8pt\"><a href=\"${branch_plans_url}#$branch\">${branch}</a></td>\n";
		}
		print "<td style=\"font-size: 12pt\">$branch_category{$branch_prefix}</a></td>\n";
		print "<td style=\"font-size: 10pt\">$branch_info</td>\n";

		foreach my $period (@requested_periods) {
			if ($found_checkins{$branch}{$period} > 0 ) {
				print "<td style=\"font-size: 12pt\">$found_checkins{$branch}{$period}</td>\n";
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
