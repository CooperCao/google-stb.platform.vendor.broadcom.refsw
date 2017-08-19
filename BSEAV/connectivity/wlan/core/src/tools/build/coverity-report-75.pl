#!/usr/local/bin/perl

## #########################################################################
##
## This program queries the coverity database and generates email
## reports to respective owners of various coverity targets.
## NOTE: Coverity target names specified here should match target
## NOTE: name format used in all-run-coverity.mk makefile
##
## Categories
## 1. The administrators (hnd-software-scm-list)
##     Will want nightly notification of what ran and its status.
## 2. The developers:
##    2.1 STA Group (XP/Vista NIC, Vista USB DHD and dongle)
##    2.2 Embedded Group (Linux/WinXP/CE/WP7 DHD and dongle)
##    2.3 Router (nothing yet)
##
## Author: Prakash Dhavali
##
## $Id: coverity-report.pl 357582 2012-09-19 02:48:47Z prakashd $
##
## SVN: $HeadURL$
##
## #########################################################################
##

use     Env;
use     Getopt::Long;
use     MIME::Lite;
use     File::Basename;

my $fromAddress = "hnd-coverity-list\@broadcom.com";

# TODO. Make all symbols adhere to "use strict"
#use strict 'refs';

use lib  "/home/hwnbuild/src/tools/build";
require  "wl_swengg_lib.pl";

my $reportdate;
my $help;
my $bldadmin="hnd-software-scm-list";
my $mailto;

# -date : Set a different date to produce report for (default: today)
# -dbg  : Enable debug mode (emails don't go to blame list, goes to bldadmin)
my %scan_args = (
		'date=s'        => \$reportdate,
		'dbg'           => \$dbg,
		'help'          => \$help,
		'logdir=s'	=> \$logdir,
		'mailto=s'      => \$mailto,
		'nomail'        => \$nomail,
		'server=s'	=> \$covserver,
		'webapidir=s'   => \$webapidir,
		'webapipw=s'	=> \$webapipw
	);

# Scan the command line arguments
GetOptions(%scan_args) or &Help;

#$dbg=1;

&Help if ($help);

&Info("=======================================================");
my $REPORT_USER   = "hwnbuild"; # user used to access/query db and report results

&Dbg("Debug/Verbose mode enabled");

my $goner          = "_goner_"; # Pseudo name for invalid users
my $newdefect_runs = 0; # Number of runs with new defects
my $numtotal_sched;     # Number of targets scheduled
my $rNightlyRuns   = {};
my $build_config   = "/home/hwnbuild/src/tools/build/build_config.sh";
my $svnwho         = "/projects/hnd_software/gallery/src/tools/misc/svnwho";
my $svncmd         = "/tools/bin/svn --non-interactive";
my $srclist        = "/projects/hnd_software/gallery/src_tree.txt"; # generated daily by update_gallery_svn.sh=>svn_update_session.sh
my $name2path      = {};                                            # hash populated from $srclist
my $coverity_config= "gmake -s -f /home/hwnbuild/src/tools/release/coverity-target-info.mk";
my @ActiveBranches = split(/\s+/,qx($build_config --show_coverity_active)); chomp(@ActiveBranches);

if ( $hostenv =~ /win32/ ) {
	$PATHPFX = "Z:";
	$TEMP    = "C:/temp";
} else {
	$PATHPFX = "";
	$TEMP    = "/tmp";
}

my $COVERITY_VERSION_65 = "6.5";
my $COVERITY_VERSION_75 = "7.5";
my $COVERITY_VERSION = "$COVERITY_VERSION_75";
my $COVPROCESS_URL= "http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/CoverityStaticCodeAnalysis";
my $BLDSERVER_BASE= "/projects/hnd_swbuild";
my $BLDSERVER_URL = "http://home.sj.broadcom.com";
my $COVBUILD_URL  = "${BLDSERVER_URL}${BLDSERVER_BASE}";
my $COVSERVER_65 = "wlansw-cov65.sj.broadcom.com";
my $COVSERVER_75 = "wlansw-cov75.sj.broadcom.com";


if (! $logdir ) {
	$logdir = "${BLDSERVER_BASE}/build_admin/logs/coverity750";
}
&Info("Log Directory \t$logdir");

my $COVSUM_LOGDIR = "$logdir/summary";
my $SVNBLAME_LOGDIR = "$logdir/svnblames";
my $WEBAPI_LOG = "$logdir/webapi.log";

open(WEBAPI_LOG,">>$WEBAPI_LOG") || die "ERROR: Can't open $WEBAPI_LOG";
print WEBAPI_LOG "======= START: $dirtimestamp ======\n";

if ( !$covserver){
	$covserver = "$COVSERVER_75";
}
&Info("Coverity Server \t$covserver:8080");

my $PREVENT_WEBURL="http://$covserver:8080";

# WebService APIs to replace coverity query commands
my $WEBAPI_USER = "hwnbuild";

if (!$webapipw){
	$webapipw = qx(cat /home/${WEBAPI_USER}/.restricted/passwd 2> $NULL);
}
my $WEBAPI_PW = $webapipw;
chomp($WEBAPI_PW);

my $WEBAPI_OPTS           = "--host $covserver --port 8080 --user hwnbuild --password ${WEBAPI_PW}";

if (!$webapidir){
	$webapidir = "/projects/hnd_sw_continuous/wlansvn/groups/software/infrastructure/coverity/webapi/python_api_v5";
}
my $WEBAPI_DIR            = $webapidir;
&Info("WebAPI Directory \t$webapidir\n");

my $GET_SNAPSHOTS_FOR_DAY = "python ${WEBAPI_DIR}/getSnapshotsForDate.py $WEBAPI_OPTS";
my $GET_CIDS_FOR_SNAPSHOT = "python ${WEBAPI_DIR}/getCidsForSnapshotComponent.py $WEBAPI_OPTS";
my $DIFF_RUNS             = "python ${WEBAPI_DIR}/diffRuns.py $WEBAPI_OPTS";
my $GET_PROJECT_FOR_STREAM       = "python ${WEBAPI_DIR}/getProjectURLForStream.py $WEBAPI_OPTS";
my $GET_CID_URL_FOR_STREAM       = "python ${WEBAPI_DIR}/getCIDURL.py $WEBAPI_OPTS";

# Build logs
my $ERRORS        = "";
my $WARNINGS      = "";
my $RLSLOG        = ",release.log";
my $ERRORLOG      = ",build_errors.log";

my %BrandsByPlatform = (
	windows => qw(win-coverity),
	window  => qw(win-coverity),
	windows => qw(win-coverity-65),
	window  => qw(win-coverity-65),	
	windows => qw(win-coverity-75),
	window  => qw(win-coverity-75),	
	linux   => qw(linux-coverity),
	linux   => qw(linux-coverity-65),
	linux   => qw(linux-coverity-75),
	macos   => qw(macos-coverity),
	macos   => qw(macos-coverity-65),
	macos   => qw(macos-coverity-75),
	netbsd  => qw(netbsd-coverity),
);

my %tgtDescMap;    # Mapping between make target and descriptive names

my %cidAuthorsByFile;  # File Author of a CID
my %cidAuthorsByLine;  # Line Author of a CID
my %cidGonersByFile;   # File Author of a CID if author is $goner
my %cidGonersByLine;   # Line Author of a CID if author is $goner
my %cidTargets;        # Targets for a CID
my %cidBranches;       # Code Branches for a CID
my $numGoners = 0;     # Number of Goners reported

my $ndindex = 0;# Number of defects

my $querydate     = `date '+%Y%m%d'`; chomp($querydate);
my $rundate       = $nowdate;

if (( $reportdate) && ($reportdate !~ /(\d{4}).(\d{2}).(\d{2}):(\d{2}):(\d{2})/)) {
	&Error3("Date=$reportdate wrong specs, expecting 'YYYY.mm.dd:HH:MM'");
	$ERRORS .= "Date=$reportdate wrong specs, expecting 'YYYY.mm.dd:HH:MM'\n";
	&Exit($ERROR);
}

# nowhour, nowminute are the default query times for the reports. Often this is the time
# that the report script runs but not always. These variables can be overriden by command line options
# if we are debugging.
# There needs to be a better linkage between the report times and the end of the set of Coverity builds.
# See http://jira.broadcom.com/browse/SWSCM-40
my $nowhour   = 11;
my $nowminute = 0;
my $svn_start_range = `date --date="yesterday 00:00:00" "+%Y-%m-%d %H:%M:%S"`; chomp($svn_start_range);
my $svn_stop_range = `date --date="yesterday 23:59:59" "+%Y-%m-%d %H:%M:%S"`; chomp($svn_stop_range);

## Set rundate (which is used in report email) to custom date if specified
if ($reportdate =~ /(\d{4}).(\d{2}).(\d{2}):(\d{2}):(\d{2})/) {
	$querydate   = "$1$2$3";
	$querytime   = "$3$4";
	$rundate     = $reportdate;

	# If reportdate is specified on cmd line, override now{year,month,day}
	# These are used in WebAPI calls
	$nowyear   = $1;
	$nowmonth  = $2;
	$nowday    = $3;
	$nowhour   = $4;
	$nowminute = $5;

	$svn_start_range = `date --date="$nowyear-$nowmonth-$nowday 1 day ago 00:00:00" "+%Y-%m-%d %H:%M:%S"`; chomp($svn_start_range);
	$svn_stop_range = `date --date="$nowyear-$nowmonth-$nowday 1 day ago 23:59:59" "+%Y-%m-%d %H:%M:%S"`; chomp($svn_stop_range);
}
print WEBAPI_LOG "Today's Runs Cmd: $GET_SNAPSHOTS_FOR_DAY --year $nowyear --month $nowmonth --day $nowday --hour $nowhour --minute $nowminute \n";

&Info("Run Date=$rundate TimeStamp=$nowtimestamp");
&Dbg("nowyear=$nowyear; nowmonth=$nowmonth; nowday=$nowday; nowhour=$nowhour; nowminute=$nowminute") if ($reportdate);

my $today      = `date '+%a'`;     chomp($today);
my $blddate    = `date '+%Y.%-m.%-d.{?,??}'`; chomp($blddate);

# Define functional modules in coverity and their owners
# These are referred as "categories" in this programs
$rCatInfo = {
	admin   => {
		owners  => [($bldadmin)],
	},
	sta     => {
		owners  => [qw(hnd-coverity-list@broadcom.com)],
	},
	router  => {
		owners  => [qw(hnd-coverity-list@broadcom.com)],
	},
}; # rCatInfo

# Defects are now stored in a global table rather than in the per-stream rCatinfo tables
my $globalDefectTable = "";

# Email subroutine to send email
sub email {

    my $fromAddress = $_[0];
    my $toAddress = $_[1];
    my $ccAddress = $_[2];
    my $subject = $_[3];
    my $data = $_[4];

    my $message = MIME::Lite->new(
    From => $fromAddress,
    To => $toAddress,
    Cc => $ccAddress,
    Subject => $subject,
	Type => 'text/html',
	Data => $data
	);
    $message->send('smtp','mail.broadcom.com');

}

# 7.5
my $HTML_DRIVER_URL = "http://${COVSERVER_75}:8080/query/defects.htm?project=DRIVER&outstanding=true";
my $HTML_WL_EXE_URL = "http://${COVSERVER_75}:8080/query/defects.htm?project=WL-EXE&outstanding=true";
my $HTML_ROUTER_URL = "http://${COVSERVER_75}:8080/query/defects.htm?project=ROUTER&outstanding=true";
my $HTML_IHV_URL    = "http://${COVSERVER_75}:8080/query/defects.htm?project=IHV&outstanding=true";

my $HTML_NIGHTLY_HEADER = <<'END';
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <title>Coverity Nightly Run Summary</title>
</head>
<body>
END

my $HTML_NIGHTLY_TRAILER = <<'END';
</body>
</html>
END

my $HTML_DEFECT_HEADER = <<'END';
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <title>Coverity Defect Report</title>
</head>
<body>
END

my $HTML_DEFECT_TRAILER = <<'END';
</body>
</html>
END

my $HTML_TABLE_HEADER = <<'END';

<table width="100%">
END

my $HTML_CID_TABLE_CAPTION_START = <<END;
<table border="1" cellspacing="1" cellpadding="5">
  <caption>
END

my $HTML_CID_TABLE_CAPTION_END = <<END;
  </caption>
END

my $HTML_CID_TABLE_HEADER_ITEMS = <<END;
<tr align="left">
  <th scope="col">CID</th>
  <th scope="col">Checker</th>
  <th scope="col">User</th>
  <th scope="col">File[Line]</th>
  <th scope="col">Code Branches</th>
  <th scope="col">Function</th>
</tr>
END

my $HTML_TABLE_ROW_START = <<'END';
<tr>
END

my $HTML_TABLE_ROW_END = <<'END';
</tr>
END

my $HTML_TABLE_CELL_START = <<'END';
<td>
END

my $HTML_TABLE_CELL_END = <<'END';
</td>
END

my $HTML_TABLE_TRAILER = <<'END';
</table>
END

my $HTML_BR         = '<br>';
my $HTML_START_PRE  = '<pre>';
my $HTML_END_PRE    = '</pre>';

#
# Given Target Description, identify its build OS and categorize
# to broadcom functional group. This will be help keep the reports
# going specific groups for faster closure
#
sub CategorizeTargets {
	my ($tgtType, $tgt, $tgtDesc) = @_;

	# print "CategorizeTargets(): Processing type=$tgtType, tgt=$tgt, desc=$tgtDesc\n";

	if ( $tgtDesc =~ /Win(XP|7|Vista).*WL.*(NIC|BMac).*Driver/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /Win(XP|7|Vista).*DHD.*Driver/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /Win(XP|7|Vista).*WL.*Utility/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /WinCE|WinMobile/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /prefast/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /Linux.*WL.*(NIC|BMac).*Driver/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Linux.*DHD.*Driver/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Linux.*(WL|DHD).*Utility/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /(43...*bmac\/|Dongle)/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /(43...*\/|Dongle)/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Router/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(router)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Linux/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Win/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /VxWorks/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(router)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /MacOS/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "macos";
	} else {
		&Warn("'$tgtDesc' couldn't be categorized as sta");
		&Warn("'$tgtDesc' is assigned to sta category");
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	}

	&Info("Categorize '$tgtDesc' as '@{$tgtType->{$tgtDesc}{categories}}' target");

	# Mark all builds in admin category too
	push(@{$tgtType->{$tgtDesc}{categories}},"admin");

} # CategorizeTargets

#
# Go through active branches and find out scheduled coverity runs on a
# nightly basis as well as weekly basis
#
sub findTargets {

	foreach my $branch (@ActiveBranches) {
		my @branchNightlyTargets;

		# Starting with Coverity 5.3.x, we use TRUNK for NIGHTLY
		$branch =~ s/NIGHTLY/TRUNK/g;

		@branchNightlyTargets = split(/\s+/, qx($coverity_config TAG=$branch show_coverity_targets | fmt -1));

		next unless ($#branchNightlyTargets > 0);

		&Info("-------------------------------------");
		&Info("branchNightlyTargets[$#branchNightlyTargets] = @branchNightlyTargets");

		# Categorize nightly targets
		foreach my $tgt (@branchNightlyTargets) {
			my ($tmptgt);
			my ($tmptgtDesc);

			next if $tgt =~ /^\s*$/g;

			my $tgtDesc = qx($coverity_config $tgt);
			chomp($tgtDesc);

			$tmptgt     = $tgt;     $tmptgt     =~ s/\//_/g;
			$tmptgtDesc = $tgtDesc; $tmptgtDesc =~ s/\//_/g;

			# Reverse map, used for stream/project names
			$tgtDescMap{$tmptgtDesc} = $tmptgt;

			# Prefix Preco to identify preco targets from
			# similar nightly targets. Needed in URL query below
			if ($tgtDesc =~ /preco/i) {
				$tgtDesc =~ s/preco |preco_//gi;
				$tgtDesc = "Preco $branch $tgtDesc";
			} else {
				$tgtDesc = "$branch $tgtDesc";
			}
			&CategorizeTargets($rNightlyRuns,$tgt,$tgtDesc);
		}

		# Target name to description name mapping
		&Info("Target description mapping (cumulative across branches)");
		foreach my $tgtdesc (sort keys %tgtDescMap) {
			&Info("Map '$tgtdesc' => '$tgtDescMap{$tgtdesc}'");
		}

	} # @ActiveBranches

	# Populate a new rAllRuns hash from rNightlyRuns and rWeeklyRuns
	# Note: Weekly targets are suppressed for now

	foreach my $run ( keys %$rNightlyRuns ) {

		next if ($run =~ /prefast/);

		$nrun = $run;
		@{$rAllRuns->{$nrun}{categories}} = @{$rNightlyRuns->{$run}{categories}};
		$rAllRuns->{$nrun}{schedule}      = $rNightlyRuns->{$run}{schedule};
		$rAllRuns->{$nrun}{buildos}       = $rNightlyRuns->{$run}{buildos};
	}

	# Total number of scheduled coverity targets
	$numtotal_sched = keys %$rAllRuns;

	foreach my $schedrun ( sort keys %$rAllRuns ) {
		&Dbg("Scheduled target = $schedrun");
	}

} # findTargets()

#
# For all targets found in findTargets, search for Coverity snapshots
#
sub getSnapshots {

	# Extract a list of targets that actually ran for ${querydate}
	# TODO: currently this implicitly means "for the 24 hours prior to $querydate". This is what getSnapshotsForDay.py
	# returns. There should be a more exact mechanism...
	# Examples from following query
	# Run ID,Product,Description,Date,Hist. New Defects,Outstanding Defects,PARSE_ERROR,# Files,# Components,LOC
	# 100030,hndwlan,FALCON_BRANCH_5_90 4330b2-roml/sdio-g 20110416_1444 ,2011-04-16 14:05:50.000163,5,87,0,0,0,0

	my $curtime=qx(date '+%Y/%m/%d %H:%M:%S'); chomp($curtime);
	&Info("[$curtime] Getting list of snapshots as of $nowmonth.$nowday.$nowyear:$nowhour:$nowminute");
	# The following call will get snapshots committed since 12:01 AM on this date up to the "nowhour/nowminute" specified
	&Info("$GET_SNAPSHOTS_FOR_DAY --year $nowyear --month $nowmonth --day $nowday --hour $nowhour --minute $nowminute");

	@todays_runs = qx($GET_SNAPSHOTS_FOR_DAY --year $nowyear --month $nowmonth --day $nowday --hour $nowhour --minute $nowminute); $webapi_ec=$?;
	chomp(@todays_runs);

	$curtime=qx(date '+%Y/%m/%d %H:%M:%S'); chomp($curtime);
	&Info("[$curtime] Finished getting list of snapshots for $nowmonth.$nowday.$nowyear:$nowhour:$nowminute");

	# Take out header row and double spaces
	@todays_runs = grep { !/Run ID,/ } @todays_runs;
	@todays_runs = map { $_ =~ s/\s+/ /g; "$_" } @todays_runs;

	# If @todays_runs is empty, then it is an error
	if (($webapi_ec != 0 ) || ( ! @todays_runs )) {
		&Error3("Coverity snapshot query error ($webapi_ec)");
		$ERRORS .= "\nCoverity snapshot query error ($webapi_ec)\n";
		$ERRORS .= "\nLook up stdout log on system where this ran\n";
		# When this happens, notify admin users and set dbg
		$dbg = 1;
	}

	my $i=1;
	foreach my $run (@todays_runs) {
		&Info("[$i] Snapshot Found: $run");
		$i++;
	}

}; # getSnapshots ()

## slap cell header/trailer on string
sub insertHTMLCell
{
	my($cell_string) = @_;
	my($retval);

	$retval = "";
	$retval .= "${HTML_TABLE_CELL_START}";
	$retval .= "$cell_string";
	$retval .= "${HTML_TABLE_CELL_END}";

	return ("$retval");
};  # insertHTMLCell ()

# Find a file in the source tree map.
# Arguments: $here   = top level build dir containing file
#            $tries  = hash map for file
#            $svnurl = top level svn URL to use for diff
# Helper for storeNewDefects.
sub finddiff {
    my($svnurl, $here, $tries) = @_;
    return undef unless %$tries;
    my %counts;
    for my $try (keys %$tries) {
	return $try if keys %$tries == 1;
	my @diffs = qx(svn cat '$svnurl/$try' | diff -w '$here' -);
	$counts{$#diffs} = $try;
    }
    my($best) = sort {$a <=> $b} keys %counts;
    return $counts{$best};
}

sub do_file_based_owner
{

	my($line_owner,$file_owner,@file_based_information) = @_;
	my %hash = map { $_ => 1} @file_based_information;
	my @unique = keys %hash;

	# Empty file based owners
	return "unknown" if ( $#unique < 0 );

	# Only one (unique) file based owner found
	return $unique[0] if ( $#unique == 0 );

	# Line based owner found in file based owners array
	my $found = grep /$line_owner/, @unique;
	return $line_owner if ( $found > 0);

	return "unknown";

}

## Save a defect in a hash along with blame, branch and target info
sub storeNewDefects
{
	my($myrun_id,$schedrun,$streamname,$component) = @_;
	my($ndef,@new_defects, $linkmsg);
	my($cid,$checker,$class,$func,$file);
	my($myrun_newurl);
	my($branch, $branchdir,$ver_author);

	my $buildos;
	my $COVBUILD_BRAND;
	my $blddir;
	my $bldsvnrev;
	my $svnurl;
	my $fileurl;
	my $schedruntmp= $schedrun;
	my $prevbldsvndate;
	my @file_based_info = ();
	my @file_based_owners = ();
	my $component_arg;

	&Dbg("storeNewDefects('$myrun_id','$schedrun','$component')");

	$schedruntmp =~ s/Preco\s+//gi;
	($branch) = $schedruntmp =~ /^(\w+\b)/;

	$buildos = $rAllRuns->{$schedrun}{buildos};
	$COVBUILD_BRAND= $BrandsByPlatform{$buildos};

	$ndefmsg = "";

	# Get New Defects.
	$component_arg = "--component=router.router_other" if ( $component =~ m/router/i );
	$component_arg = "--component=ihv.ihv_other" if ( $component =~ m/ihv/i );
	print WEBAPI_LOG "Get New Defects Cmd: $GET_CIDS_FOR_SNAPSHOT --only-new --snapshot $myrun_id $component_arg\n";
	&Info("Get New Defects Cmd: $GET_CIDS_FOR_SNAPSHOT --only-new --snapshot $myrun_id $component_arg");
	@new_defects = qx($GET_CIDS_FOR_SNAPSHOT --only-new --snapshot $myrun_id $component_arg);
	$webapi_ec=$?;

	if ($webapi_ec != 0) {
		&Warn("Couldn't get defects");
		return("");
	}

	chomp(@new_defects);

	# Filter out only defects marked as New
	@new_defects = sort grep { /,New,/ } @new_defects;

	# If no new defects are found, no work to do here
	# Recurred defects, are reported as recurred by API, even though
	# UI may show them as New
	return("") if ( $#new_defects < 0 );

	$ndindex=0;
	foreach my $defect (@new_defects) {
		$ndindex++;
		&Dbg("[$ndindex] FOUND NDEFECT: $defect");
	}

	if (@new_defects) {
		$branchdir= ($branch =~ /TRUNK/) ? "NIGHTLY" : $branch;
		$bldsvnrev = 0;
		$blddir = "";

		my @svn_revision_files = glob("${BLDSERVER_BASE}/build_${buildos}/$branchdir/${COVBUILD_BRAND}/*/_SUBVERSION_REVISION");
		foreach my $svn_revision_file (@svn_revision_files) {
			next if (! -r $svn_revision_file);
			open my $in, '<', $svn_revision_file or die "Open fail on $svn_revision_file: $!\n";
			my $svn_rev = <$in>;
			$svn_rev =~ s/\s+//g;
			close $in;

			if ($svn_rev > $bldsvnrev) {
				$bldsvnrev = $svn_rev;
				$blddir = dirname($svn_revision_file);
			}

		}

		#
		# Get the previous build svn revision. Builds are getting removed early so consulting the previous Coverity
		# build tree is unreliable. Use a date string for the previous svn revision.
		# Future enhancement will maintain the build revisions within a file in SVN.
		#
		$prevbldsvndate = "{'${svn_start_range}'}";

		if ($branch =~ /NIGHTLY|TRUNK/) {
			$svnurl      = "${SVNROOT}/trunk";
		} else {
			$svnurl      = "${SVNROOT}/branches/$branch";
		}
	}
	$ndindex=0;
	foreach my $ndef ( @new_defects ) {
		my $curtime;
		my $filelinkpath;
		my $cid_key;

		($cid,$checker,$func,$filefullpath,$line) = (split(/,/,$ndef))[0,1,7,8,9];

		# Increase new defect index
		$ndindex++;

		# If the coverity path, is a symlink, dereference it
		$filelinkpath=qx(readlink -q $filefullpath);
		chomp($filelinkpath);
		&Info("Readlink path=$filelinkpath");

		$curtime=qx(date '+%Y/%m/%d %H:%M:%S'); chomp($curtime);
		&Info("[$curtime] Blame[$ndindex]: $ndef");

		$filepath = ($filelinkpath ? $filelinkpath : $filefullpath);
		$filepath =~ s%\\%/%g;
		# Remove leading absolute path until 'src'
		my (@filepaths) = split(/\//, $filepath);
		my $skip_path;
		foreach my $fpath (@filepaths) {
		        last if ($fpath =~ /^src$/);
        		$skip_path.="$fpath/";
		}

		$filepath =~ s/$skip_path//g;
		&Info("Effective filepath to search = $filepath");

		$file = qx(basename $filefullpath); chomp($file);

		&Dbg("=== filepath is $filepath, file is $file, filefullpath is $filefullpath ====");

		# Create a unique CID key, so that blame list generation
		# can re-use an identified user id across different targets
		$cid_key="${cid}__${checker}__${func}__${file}__${line}";
		$cid_key=~ s/\(\)//g;

		if (!exists($cidAuthorsByLine{$cid_key})) {
			&Info("Getting blame info for $cid_key");

			# Find svn path for file. Might need to be mapped because
			# some files are copied from their svn location to a different
			# location before they are built
			$fileurl = "${svnurl}/$filepath";
			my $svnls=`$svncmd ls $fileurl 2>&1`;

			if ( "$?" != "0" ) {
				&Dbg("Looking for $file in source tree map");

				# $filepath example: src/wl/sys/wdm/buildwin7/wlc_phy_cmn.c
				# $blddir example: /projects/hnd_swbuild/build_window/AARDVARK_BRANCH_6_30/win-coverity-65/2013.2.11.0
				$bldpath = $blddir . "/" . $filepath;
				$svnfilepath = &finddiff(${svnurl}, ${bldpath}, $name2path->{$file});
				$filepath = $svnfilepath;

				&Info("Recomputed filepath=$filepath");
				$fileurl = "${svnurl}/$filepath";
			} else {
				&Dbg("No mapping required for $file");
			}

			$curtime=qx(date '+%Y/%m/%d %H:%M:%S'); chomp($curtime);
			&Info("[$curtime] start svnwho, info $cid_key");

			if ( $filepath ) {
				# tell svnwho to cache annotations
				$cachedir="$blddir/misc/svn_annotate_cache";

				# Find the line author
				my $cmd = "$svnwho -c $cachedir -r $bldsvnrev $fileurl $line";
				Info($cmd);
				$def_author = qx($cmd 2> $NULL);
				chomp($def_author);

				# Note the last Changed Author in case line-based says "goner"
				&Info("svncmd info -r $bldsvnrev $fileurl");
				$ver_author = qx($svncmd info -r $bldsvnrev $fileurl | grep "^Last Changed Author:" | awk '{printf "%s", \$NF}');
				$ver_author =~ s/\s+//g;
				chomp($ver_author);

				# Validate if the identified user valid or not
				`id $def_author > $NULL 2>&1`;
				if ( "$?" != "0" ) {
					$cidGonersByLine{$cid_key} = $def_author;
					$def_author = $goner;
				}
				`id $ver_author > $NULL 2>&1`;
				if ( "$?" != "0" ) {
					$cidGonersByFile{$cid_key} = $ver_author;
					$ver_author = $goner;
				}

				# Get all of the file revisions inclusive of the last run
				@file_info = qx($svncmd log -r ${prevbldsvndate}:${bldsvnrev} $fileurl | egrep "^r[0-9]+");
				@file_based_info = ();
				@file_based_owners = ();
				foreach my $finfo ( @file_info ) {
					my ($file_version,$file_version_author,$file_version_time) = (split(/\|/, $finfo))[0,1,2];
					$file_version =~ s/\s+//g;
					$file_version_author =~ s/\s+//g;
					$file_version_time =~ s/^\s+|\s+$//g;
					push(@file_based_info, "$file_version | $file_version_time | $file_version_author");
					push(@file_based_owners, "$file_version_author");
				}

			} else {
				$def_author = "unknown";
				$ver_author = "unknown";
			}

			&Info("blddir=$blddir");
			&Info("bldsvnrev=$bldsvnrev");
			&Info("filelinkpath=$filelinkpath");
			&Info("filefullpath=$filefullpath");
			&Info("filepath=$filepath");
			&Info("bldsvnrev=$bldsvnrev");
			&Info("prevbldsvndate=$prevbldsvndate");
			&Info("defect_author=$def_author");
			&Info("version_author=$ver_author");
			&Info("file based deliveries between revisions $prevbldsvndate and $bldsvnrev");
			foreach my $file_stuff ( @file_based_info ) {
				&Info("   $file_stuff");
			}

			$file_based_owner = &do_file_based_owner( $def_author, $ver_author, @file_based_owners );

			&Info("Setting $cid_key file based author to $file_based_owner ");
			&Info("Setting $cid_key line author to $def_author ");
			&Info("Setting $cid_key file author to $ver_author ");
			&Info("Setting $cid_key target to $schedrun ");

			# Set owner to file based
			$def_author = $file_based_owner;

			# Populate processed blames, to share with other targets
			$cidAuthorsByFile{$cid_key} = $ver_author if ($ver_author);
			$cidAuthorsByLine{$cid_key} = $def_author if ($def_author);
			push(@{$cidTargets{$cid_key}}, $streamname);
			push(@{$cidBranches{$cid_key}}, $branch);

		} else {
			#else (!exists($cidAuthorsByLine{$cid_key}))
			&Info("Already found: $cid_key = $cidAuthorsByLine{$cid_key}");
			$def_author = $cidAuthorsByLine{$cid_key};
			$ver_author = $cidAuthorsByFile{$cid_key};

			&Dbg("== adding target $streamname ===");
			push(@{$cidTargets{$cid_key}}, $streamname);
			push(@{$cidBranches{$cid_key}}, $branch);
		}  # cidAuthorsByLine

		$curtime=qx(date '+%Y/%m/%d %H:%M:%S'); chomp($curtime);
		&Info("[$curtime] end svnwho, info $cid_key");

		foreach my $category ( @{$rAllRuns->{$schedrun}{categories}} ) {
			if ($def_author !~ /unknown|$goner/) {
				&Info("Will skip $def_author from email notification list");
			}
			if ( $def_author ne "" ) {
				&Dbg ("======= pushing def_author $def_author ===============");
				push(@{$rCatInfo->{$category}{notifylist}},$def_author) unless (grep {/^$def_author$/} @{$rCatInfo->{$category}{notifylist}});
			}
		}
	} # @new_defects

	return();

}; # storeNewDefects ()

## Go through the cidAuthorsByLine hash and report the results
## return a string containing the HTML table
sub showNewDefects
{
	my($table_caption) = @_;
	my($ndefmsg);
	my($streamname);
	my($html_branch_cell);
	my(%cids_seen);

	&Dbg("showNewDefects");

	$ndefmsg = "";
	$ndefmsg .= "$HTML_TABLE_HEADER";

	$ndefmsg .= "$HTML_CID_TABLE_CAPTION_START";
	$ndefmsg .= "$table_caption";
	$ndefmsg .= "$HTML_CID_TABLE_CAPTION_END";

	$ndefmsg .= "$HTML_CID_TABLE_HEADER_ITEMS";

	for my $cidkey ( reverse sort keys %cidAuthorsByLine ) {
		($cid, $checker, $func, $file, $line) = split ("__", $cidkey);

		# if we've already seen this cid don't duplicate a row for it
		if (exists($cids_seen{$cid})){
			next;
		}
		$cids_seen{$cid} = "True";

		&Dbg("=== Author $cidkey: $cidAuthorsByLine{$cidkey}");
		&Dbg("=== Targets:");
		foreach my $target (@{$cidTargets{$cidkey}} ) {
			&Dbg("$target");
		}

		#	($cid, $checker, $func, $file, $line, $branch, $target) = split('__', "$cidkey");
		#	&Dbg("\tcid $cid, checker $checker, function $func file $file, line $line");

		# pick first stream name in list. For the URL it doesn't matter which target.
		$streamname = $cidTargets{$cidkey}[0];

		# Provide a URL pointing directly to the CID
		print WEBAPI_LOG "$GET_CID_URL_FOR_STREAM --stream $streamname\n";

		&Info("$GET_CID_URL_FOR_STREAM --cid $cid --stream $streamname");
		$fcid_url = qx($GET_CID_URL_FOR_STREAM --cid $cid --stream $streamname);
		$webapi_ec=$?;
		chomp($fcid_url);

		if ($webapi_ec == 0) {
			$linkmsg = "<a href=\"$fcid_url\">$cid</a>";
		} else {
			&Warn("Couldn't get CID URL");
			$linkmsg = "";
		}

		$checker = substr($checker, 0, 16);

		$ndefmsg .= "${HTML_TABLE_ROW_START}";
		$ndefmsg .= &insertHTMLCell($linkmsg);
		$ndefmsg .= &insertHTMLCell($checker);
		
		$blamee = $cidAuthorsByLine{$cidkey};
		if ( $blamee eq $goner ) {
			$ndefmsg .= &insertHTMLCell("$cidGonersByLine{$cidkey}*");
			$numGoners += 1;
		} else {
			$ndefmsg .= &insertHTMLCell($blamee);
		}
		$ndefmsg .= &insertHTMLCell("$file\[$line\]");

		$html_branch_cell = "";
		%seen=(); @unique = grep { ! $seen{$_} ++ } @{$cidBranches{$cidkey}};
		foreach my $branch (sort @unique ) {
			$html_branch_cell .= "$branch<br>";
		}
		$ndefmsg .= &insertHTMLCell($html_branch_cell);

		$ndefmsg .= &insertHTMLCell($func);

		$ndefmsg .= "${HTML_TABLE_ROW_END}";
	} # all cidAuthorsByLine

	# Append table trailer
	$ndefmsg .= "${HTML_TABLE_TRAILER}";

	&Dbg("=== showNewDefects: returning $ndefmsg");
	return("$ndefmsg");
}; # showNewDefects ()

## Scan runs in rAllRuns hash
sub scanRuns
{
	## Initialize counters
	$numpassed = $numfailed = 0;

	## Now check the status of each coverity target and generate messages
	foreach my $schedrun ( sort keys %$rAllRuns ) {
		my @fruns      = ();
		my @runstrings = ();
		my $msg        = "";
		my $ndefmsg    = "";
		my $branch     = "";
		my $branddir   = "";
		my $branchdir  = "";
		my $bldtarget  = "";
		my $streamname = "";
		my $schedruntmp= $schedrun;
		my $table_caption = "";

		my $runstring = "$schedrun";
		my $curtime   = qx(date '+%Y/%m/%d %H:%M:%S'); chomp($curtime);

		&Info("[$curtime] Scanning Run: $runstring");

		push(@runstrings,"$runstring");

		$schedruntmp =~ s/Preco\s+//g;
		($branch,$bldtarget) = ($schedruntmp =~ /^(\w+\b)\s*(.*)$/);

		&Info("Derived from snapshot branch=$branch; bldtarget=$bldtarget");

		$bldtarget =~ s/\//_/g;

		# Construct project/stream name from build target name
		# e.g: "FALCON_BRANCH_5_90 WinXP x86 DHD USB Driver" becomes
		#       "FALCON_BRANCH_5_90__WinXP_x86_DHD_USB_Driver"
		if (($bldtarget =~ /preco_/) || ($runstring =~ /preco/i)) {
			# Get short target name from descriptive name for stream
			# Prefix Preco to fetch correct bld target name
			$bldtarget = $tgtDescMap{"Preco $bldtarget"};
			# Match preco coverity baseline convention preco_<branch>_<target>
			$bldtarget =~s/preco_//g;
			$streamname  = "preco_${branch}__${bldtarget}";
		} else {
			# Get short target name from descriptive name for stream
			$bldtarget = $tgtDescMap{$bldtarget};
			$streamname  = "${branch}__${bldtarget}";
		}

		&Info("Coverity stream derived for query: $streamname");

		foreach my $runstring ( @runstrings ) {
			my $msg_header = "";
			my $msg        = "";

			$runstrings =~ s/\s+/ /g;
			&Dbg("=== scanRuns: runstring $runstring");

			# Search for matching snapshot in today's
			# snapshots doesn't mix preco and nightly streams
			# So prefix with product hndwlan in query
			@frunstring = grep { m/(hndwlan|router|ihv)\s*,\s*$runstring\s/i } @todays_runs;
			&Dbg("=== scanRuns: frunstring @frunstring");

			if (@frunstring) {
				&Info("Found matching snapshot: @frunstring");
			} else {
				&Warn("No matching snapshot found for $runstring");
			}

			# See if the new stream that is created in different product component
			@frunstring_bad = grep { m/Default\s*,\s*$runstring/i } @todays_runs;
			if (@frunstring_bad) {
				&Error("WRONG product component map 'Default' for $runstring");
				&Error("Login to Coverity UI Config and update this stream to have hndwlan as component");
			}

			# Run ID,Product,Description,Date,Hist. New Defects,Outstanding Defects,PARSE_ERROR,# Files,# Components,LOC
			if ( @frunstring ) {

				push(@fruns, @frunstring);

				foreach my $frun ( @frunstring ) {
					@frun_details     = split(/,/,$frun);
					$frun_id          = @frun_details[0];
					$frun_component   = @frun_details[1];
					$frun_newdefects  = $frun_details[4];
					$frun_newdefects  =~ s/^\s+|\s+$//g;
					@frun_branch_targ = split(' ',$frun_details[2]);
					$frun_branch      = $frun_branch_targ[0];
					@frun_target      = @frun_branch_targ[1 .. $#frun_branch_targ - 1];
					$frun_date        = $frun_branch_targ[-1];
					$frun_desc        = $frun_details[2];
					$frun_newdefects  = $frun_details[4];
					$frun_newdefects  =~ s/^\s+|\s+$//g;
					$frun_outstanding = $frun_details[5];
					$frun_outstanding =~ s/^\s+|\s+$//g;
					$frun_files       = $frun_details[6];

					&Dbg("== $frun_desc ==");
					if ( $frun_newdefects > 0 ) {
						# If new defects are found, tag it with '*' char
						$msg_header  = "* $frun_desc";
					} else {
						$msg_header  = "  $frun_desc";
					}

					$msg_header .= "\n";
					$msg_header .= "    $frun_newdefects new defects\n";

					if ( $frun_newdefects > 0 ) {
						# Store new defects in hashes for rendering later in showNewDefects()
						&storeNewDefects($frun_id,"$schedrun","$streamname","$frun_component");
						$newdefect_runs++;
					}

					foreach my $category ( @{$rAllRuns->{$schedrun}{categories}}) {
						$rCatInfo->{$category}{pass} .= "$msg_header $msg\n";
						if ( $frun_newdefects > 0 ) {
							$rCatInfo->{$category}{newdefects} .= "Defects detected\n";
							my $branch_pfx;
							($branch_pfx) = (split(/_/,$branch))[0];
							# If $branch has a new defect, include branch_pfx
							# to include in email notification subject line
							push(@{$rCatInfo->{$category}{newdefect_branches}},$branch_pfx) unless (grep { /^$branch_pfx$/ } @{$rCatInfo->{$category}{newdefect_branches}});
						}
					}
					$numpassed++;
				} # for $frun in @frunstring

			} else {

				# !@frunstring
				my $buildos= $rAllRuns->{$schedrun}{buildos};
				my $COVBUILD_BRAND= $BrandsByPlatform{$buildos};

				&Dbg("buildos = $buildos COVBUILD_BRAND = $COVBUILD_BRAND");

				$branchdir= ($branch =~ /TRUNK/) ? "NIGHTLY" : $branch;
				$branddir = "${BLDSERVER_BASE}/build_${buildos}/$branchdir/${COVBUILD_BRAND}";
				$bldsvnrev = 0;

				my @svn_revision_files = glob("${branddir}/*/_SUBVERSION_REVISION");
				foreach my $svn_revision_file (@svn_revision_files) {
					next if (! -r $svn_revision_file);
					open my $in, '<', $svn_revision_file or die "Open fail on $svn_revision_file: $!\n";
					my $svn_rev = <$in>;
					$svn_rev =~ s/\s+//g;
					close $in;

					if ($svn_rev > $bldsvnrev) {
						$bldsvnrev = $svn_rev;
						$blddir = dirname($svn_revision_file);
					}

				}

				&Info("Found Build Dir = $blddir");
				&Info("Found Build ERROR LOG=$blddir/$ERRORLOG") if ( -f "$blddir/$ERRORLOG" );

				if (( -d "${blddir}" ) && ( -s "${blddir}/${ERRORLOG}" )) {
					# Match build errors/warnings to build_summary format
					$ignorewarnings='has modification time in the future|clock skew detected';
					$blderrors   = qx(grep "Error [0-9]\\+[[:space:]]*\$" ${blddir}/${RLSLOG} |  wc -l | xargs printf "%d"); chomp($blderrors);
					$bldwarnings = qx(grep "warning" ${blddir}/${RLSLOG} | egrep -v -i "${ignorewarnings}"  | wc -l | xargs printf "%d"); chomp($bldwarnings);
					&Dbg("blderrors=$blderrors; bldwarnigns=$bldwarnings");
					$msg  = "  $runstring FAILED";
					$msg .= "  ($blderrors errors, $bldwarnings warnings)\n";
					$msg .= "    Errors: ${BLDSERVER_URL}${blddir}/${ERRORLOG}\n";
					$msg .= "    Log   : ${BLDSERVER_URL}${blddir}/${RLSLOG}\n";
				} else {

					$msg  = "  $runstring FAILED (missing build)\n";
					$msg .= "    Build : ${COVBUILD_URL}/build_$rAllRuns->{$schedrun}{buildos}/$branch/${COVBUILD_BRAND}/\n";
				}

				foreach my $category ( @{$rAllRuns->{$schedrun}{categories}}) {
					$rCatInfo->{$category}{fail} .= "$msg\n";
				}
				$numfailed++;
			} # frunstring
		} # foreach $runstring in @runstrings
	} # foreach $schedrun in %$rAllRuns
	# All defects have been scanned, append the table
	if ($ndindex > 0) {
		$globalDefectTable = &showNewDefects("New Defects");
	}
}; # scanRuns ()

## Notify admins overall coverity summary (and also module level messages)
sub notifyCategoryAdmins
{
	my ($category) = shift;
	my ($catname,$catfh,$catmsg);

	$catname = uc($category);
	$CATFH   = uc($category);

	$catmsg = qx(mktemp ${TEMP}/cov${category}.XXXXXX);
	chomp($catmsg);
	chmod 0644, $catmsg;
	open($CATFH, ">$catmsg")  || die "Can't open $catmsg";
	print $CATFH "${HTML_NIGHTLY_HEADER}";

	if ( $newdefect_runs > 0 ) {
		print $CATFH "Coverity Run Status: ${numtotal_sched} targets were scheduled, ${numpassed} ran, ${numfailed} failed and $newdefect_runs have new defects.\n\n";
	} else {
		print $CATFH "Coverity Run Status: ${numtotal_sched} targets were scheduled, ${numpassed} ran, ${numfailed} failed.\n\n";
	}
	print $CATFH $HTML_BR;
	print $CATFH $HTML_BR;

	print $CATFH $HTML_START_PRE;
	if ( $ERRORS ne '' ) {
		print $CATFH "=== COVERITY REPORTING ERRORS ============================\n\n";
		print $CATFH "$ERRORS\n";
	}
	if ( $WARNINGS ne '' ) {
		print $CATFH "=== COVERITY REPORTING WARNINGS ==========================\n\n";
		print $CATFH "$WARNINGS\n";
	}
	print $CATFH $HTML_END_PRE;

	if ( $globalDefectTable ne '' ) {
		&Dbg("notifyCategoryAdmins: Dumping Global Defect Table:");
		print $CATFH "$globalDefectTable\n";
		print $CATFH $HTML_BR;

		&Dbg("=== Number of Goners: $numGoners ===");
		if ( $numGoners > 0 ) {
			print $CATFH "* Goner (Author is not an active user)";
			print $CATFH $HTML_BR;
			print $CATFH $HTML_BR;
		}
	}
	print $CATFH $HTML_START_PRE;

	if ( defined($rCatInfo->{$category}{fail}) ) {
		&Dbg("$rCatInfo->{$category}{fail}");
		print $CATFH "=== COVERITY TARGETS THAT FAILED TO RUN =====================\n\n";
		print $CATFH "$rCatInfo->{$category}{fail}\n";
	}
	if ( defined($rCatInfo->{$category}{pass}) ) {
		&Dbg("$rCatInfo->{$category}{pass}");
		print $CATFH "=== COVERITY TARGETS THAT RAN SUCCESSFULLY ==================\n\n";
		print $CATFH "$rCatInfo->{$category}{pass}\n";
	}
	print $CATFH $HTML_END_PRE;

	print $CATFH "Coverity DRIVER Project: ";
	print $CATFH "$HTML_DRIVER_URL $HTML_BR";
	print $CATFH "Coverity WL-EXE Project: ";
	print $CATFH "$HTML_WL_EXE_URL $HTML_BR";
	print $CATFH "Coverity ROUTER Project: ";
	print $CATFH "$HTML_ROUTER_URL $HTML_BR";
	print $CATFH "Coverity IHV Project: ";
	print $CATFH "$HTML_IHV_URL $HTML_BR";

	print $CATFH "${HTML_NIGHTLY_TRAILER}";
	close($CATFH);
	&Info("Notifying $catname summary to '@{$rCatInfo->{$category}{owners}}' now");
	$subject  = "";
	$subject .= "COVERITY ERROR:"    if ( $ERRORS   ne '' );
	$subject .= "COVERITY WARNINGS:" if ( $WARNINGS ne '' );
	$subject .= "Coverity Run Summary for DRIVER, WL-EXE, ROUTER and IHV projects";
	if ( $numfailed > 0 ) {
		if ( $newdefect_runs > 0 ) {
			$subject .= "($numfailed of ${numtotal_sched} FAILED, found $newdefect_runs with new defects) for ${rundate}";
		} else {
			$subject .= "($numfailed of ${numtotal_sched} FAILED) for ${rundate}";
		}
	} else {
		if ( $newdefect_runs > 0 ) {
			$subject .= "(all ${numtotal_sched} ran, found $newdefect_runs with new defects) for ${rundate}";
		} else {
			$subject .= "(all ${numtotal_sched} ran) for ${rundate}";
		}
	}
	if ( $USER =~ /$REPORT_USER/ ) {
		my(@tolist,@bcclist);

		push(@tolist, @{$rCatInfo->{$category}{owners}});

		if ($dbg) {
			&Dbg("Reset tolist=bcclist=$bldadmin");
			@tolist = @bcclist = ($bldadmin);
		}

		if ($mailto) {
			&Dbg("Reset tolist=bcclist=$mailto");
			@tolist = @bcclist = ($mailto);
		}

		# Add @broadcom.com to email addresses
		for ( @tolist ) {
			$_ .= '@broadcom.com' unless /@/;
		}
		for ( @bcclist ) {
			$_ .= '@broadcom.com' unless /@/;
		}

		&Info("Notifying $catname defects to To:'@tolist' Cc:'@bcclist' now");
		my $toAddress = join(',', @tolist);
		my $ccAddress = join(',', @bcclist);
		open(my $CATFH, "<", $catmsg) or die "Can't open $catmsg: $!";
		my $data = do {local $/; <$CATFH> };
		close($CATFH);
		email($fromAddress, $toAddress, $ccAddress, $subject, $data);
		sleep 5;
		system("cp -v $catmsg ${COVSUM_LOGDIR}/${category}_${dirtimestamp}.txt");
		system("unix2dos ${COVSUM_LOGDIR}/${category}_${dirtimestamp}.txt");
		system("rm -fv $catmsg");
	} else {
		&Info("Email notification disabled. Check '$category' report '$catmsg'");
	}
}; # notifyCategoryAdmins ()

## Notify individual module owners only if there are new defects
sub notifyCategoryOwners
{
	my ($category) = shift;
	my ($catname,$catfh,$catmsg);
	my ($notifyusers);
	my (%seen_users);

	$catname = uc($category);
	$CATFH   = uc($category);

	if ( ! defined($rCatInfo->{$category}{newdefects}) ) {
		&Info("No new defects found in $catname category");
		return;
	}

	&Dbg("==nofityOwners: notify list is @{$rCatInfo->{$category}{notifylist}}");
	$notifyusers = join(',',grep { !$seen_users{$_}++ } sort @{$rCatInfo->{$category}{notifylist}});

	$catmsg  = qx(mktemp ${TEMP}/cov${category}.XXXXXX);
	chomp($catmsg);
	chmod 0644, $catmsg;
	open($CATFH,  ">$catmsg")   || die "Can't open $catmsg";
	print $CATFH "${HTML_DEFECT_PRE_HEADER}";

	if ( "$ERRORS" ne '' ) {
		print $CATFH "\n";
		print $CATFH "#################### WARN ##################\n";
		print $CATFH "* ERRORS : Coverity DB Query API Errors Found\n";
		print $CATFH "* ERRORS : Lookup console log for details\n";
		print $CATFH "* ERRORS : BLAME EMAIL SUPPRESSED\n";
		print $CATFH "#################### WARN ##################\n";
		print $CATFH "\n";
	}

	print $CATFH "* New Defects Found in $catname targets for $rundate$HTML_BR";
	print $CATFH "* Defects From    : $notifyusers$HTML_BR";
	print $CATFH "* Coverity  UI : $PREVENT_WEBURL$HTML_BR";
	print $CATFH "* Read This Now. You MUST follow this process to address this issue:$HTML_BR";
	print $CATFH "*                 : $COVPROCESS_URL$HTML_BR";
	print $CATFH $HTML_BR;

	&Dbg("notifyCategoryOwners: Dumping Global Defect Table:");
	if ( $globalDefectTable ne '' ) {
		print $CATFH "$globalDefectTable\n";
		print $CATFH $HTML_BR;
		&Dbg("=== Number of Goners: $numGoners ===");
		if ( $numGoners > 0 ) {
			print $CATFH "* Goner (Author is not an active user)";
			print $CATFH $HTML_BR;
			print $CATFH $HTML_BR;
		}
	}

	print $CATFH "Coverity DRIVER Project: ";
	print $CATFH "$HTML_DRIVER_URL $HTML_BR";
	print $CATFH "Coverity WL-EXE Project: ";
	print $CATFH "$HTML_WL_EXE_URL $HTML_BR";
	print $CATFH "Coverity ROUTER Project: ";
	print $CATFH "$HTML_ROUTER_URL $HTML_BR";
	print $CATFH "Coverity IHV Project: ";
	print $CATFH "$HTML_IHV_URL $HTML_BR";

	print $CATFH "${HTML_DEFECT_TRAILER}";
	close($CATFH);
	if ( $USER =~ /$REPORT_USER/ ) {
		my(@tolist,@bcclist);

		push(@tolist, @{$rCatInfo->{$category}{owners}});
		push(@tolist, @{$rCatInfo->{$category}{notifylist}});
		push(@bcclist, @{$rCatInfo->{$category}{bcclist}});

		# Take out goner from tolist and bcclist for email purposes
		# But still show in subject line
		@bcclist = grep { ! /^$goner$/ } @bcclist;
		@tolist = grep { ! /^$goner$/ } @tolist;

		if ($dbg) {
			&Dbg("Reset $notifyusers=tolist=bcclist=$bldadmin");
			@tolist = @bcclist = ($bldadmin);
		}

		if ($mailto) {
			&Dbg("Reset tolist=bcclist=$mailto");
			@tolist = @bcclist = ($mailto);
		}

		my @newdefect_branches = sort @{$rCatInfo->{$category}{newdefect_branches}};

		my $subject;
		if ( "$ERRORS" ne '' ) {
			$subject = "COVERITY QUERY ERRORS FOUND: Coverity New Defects on DRIVER project, targets @newdefect_branches by $notifyusers on $rundate";
		} else {
			$subject = "Coverity New Defects on DRIVER project by $notifyusers on $rundate";
		}

		# Add @broadcom.com to email addresses
		for ( @tolist ) {
			$_ .= '@broadcom.com' unless /@/;
		}
		for ( @bcclist ) {
			$_ .= '@broadcom.com' unless /@/;
		}

		&Info("Notifying $catname defects to To:'@tolist' Bcc:'@bcclist' now");
		my $toAddress = join(',', @tolist);
		my $ccAddress = join(',', @bcclist);
		open(my $CATFH, "<", $catmsg) or die "Can't open $catmsg: $!";
		my $data = do {local $/; <$CATFH> };
		close($CATFH);
		my $num_notify = scalar(grep {defined $_} @{$rCatInfo->{$category}{notifylist}});
		email($fromAddress, $toAddress, $ccAddress, $subject, $data) if ($num_notify > 0);
		sleep 5;
		system("cp -v $catmsg ${COVSUM_LOGDIR}/${category}_${dirtimestamp}.txt");
		system("unix2dos ${COVSUM_LOGDIR}/${category}_${dirtimestamp}.txt");
		system("rm -fv $catmsg");
	} else {
		&Info("Email notification disabled. Check '$category' report '$catmsg'");
	}

}; #notifyCategoryOwners ()

# Certain files are copied from their svn location to another directory to build.
# At report time we need to map the path of these files back to their
# paths in the source tree. This function initializes the map.
sub initializeSourceMaps {
	open(SRCLIST, $srclist) || die "$0: Error: $srclist: $!\n";
	for my $path (<SRCLIST>) {
	    chomp $path;
	    #print "mapping $path\n";
	    my $name = basename($path);
	    $name2path->{$name}->{$path} = 1;
	}
	# insert entries manually here if not found in source tree map
	#$name2path->{"wlc_offloads.c"}->{$path} = 1;
close SRCLIST;
}

#
# Main ()
#
sub Main {
	&Info3("STEP0: Initialize source tree map.");
	&initializeSourceMaps();

	&Info3("STEP1: Finding list of all scheduled Nightly and Preco Targets");
	&findTargets;

	&Info3("STEP2: Finding coverity snapshots for Nightly and Preco Targets");
	&getSnapshots;

	&Info3("STEP3: Scan snapshots for new defects introduced");
	&scanRuns;

	# If coverity query db returns any errors, notify admin users
	if (( "$ERRORS" ne '' ) || (defined($dbg))) {
		$rdbgowner=[($bldadmin)];
		foreach my $category ( keys %$rCatInfo ) {
			$rCatInfo->{$category}{owners} = $rdbgowner;
		};
	}

	if ( $nomail ) {
		&Info3("STEP4: Defect Report NOT mailed");

	} else {
		Info3("STEP4: Notify functional groups on new defects found");
		&notifyCategoryAdmins("admin");
		&notifyCategoryOwners("sta");
		#&notifyCategoryOwners(router);
	}
}

# Show help
sub Help
{
	print "
        This script produces Coverity build status report and detects any
	new defects across all branches and targets.
        This script needs to be run as $REPORT_USER by build-team.
        Coverity database is access as read-only through webapi.

        Usage: $0 \\
          -date yyyy.mm.dd:HH:MM (optional) Coverity reports as of yyyy.mm.dd:HH:MM:00    \\
          -dbg             (optional) Debug/Verbose mode. Emails go to admins.            \\
          -help            (optional) Show this help screen.                              \\
          -logdir          (optional) Log script output to this directory                 \\
          -mailto <rcpt>   (optional) Force emails to <rcpt>.                             \\
          -nomail          (optional) Do not send email at all.                           \\
          -server          (optional) DNS name of Coverity server. Assumes port 8080.     \\
          -webapidir       (optional) Location of Coverity/Broadcom Python WebAPI scripts.\\
          -webapipw        (required if -server) build account password.                  \\
        \n";

	exit 0;
}

&Main;

print WEBAPI_LOG "======= END  : $dirtimestamp ======\n";
close(WEBAPI_LOG);
