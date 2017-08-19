#!/usr/bin/perl
#
# This script tracks ROM library changes for different chips. ROM library
# data is recorded by roml build process.
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#
# SVN: $HeadURL$
#

#use strict; # Uncomment these after converting wl_swengg_lib to a pkg
#use vars;

use Env;
use English;
use Config;
use Getopt::Long;
use File::Path;
use File::Basename;
use Net::SMTP;

#BEGIN {
#  unshift(@INC, "/home/prakashd/dist/perl5lib/lib");
#}
#use HTML::TextToHTML;

$| = 1; # Do not buffer output, spit out asap

##
## Command line arguments supported by this program
##
## branch    :   Use this if any TOB image fails size check (default: TOT)
## brand     :   Check for failed builds for this brand (default: hndrte)
##               (other brands, like locator etc., can be used. But not
##               tested still)
## chiprevs  :   Chip revisions for roml tracking
## dbg       :   Enable debug mode
## help      :   Help/Usage info
## ccuser    :   list of users who volunteer to be notified in Cc: list
## timewindow:   Specify a time-window of cvs checkins in hours (default: 24hrs)
##

local $branch;          #
local $brand;           #
local $build_dir;       # build_dir to look for roml logs
local @ccusers;         # list of users to be CC'd
local @chiprevs;        #
local $report_date;     # Use this day to report on instead of today
local $dbg=0;           #
local $help;            #
local @notify;          #
local $skip_cvs_query;  #
local $timewindow;      #
local @wl_components;   #

my %scan_args = (
	'branch=s'      => \$branch,
	'brand=s'       => \$brand,
	'build_dir=s'   => \$build_dir,
	'ccuser=s'      => \@ccusers,
	'chiprevs=s'    => \@chiprevs,
	'date=s'        => \$report_date,
	'dbg'           => \$dbg,
	'help'          => \$help,
	'mail=s'        => \@notify,
	'skip_cvs_query'=> \$skip_cvs_query,
	'timewindow=s'  => \$timewindow,
	'wl_components=s' => \@wl_components,
);

# Scan the command line arguments
GetOptions(%scan_args) or Error("GetOptions failed!");

use lib  "/home/hwnbuild/src/tools/build";
require  "wl_swengg_lib.pl";

Help() if ($help);
Info3("Running ROML Tracker on $thishost at $nowtimestamp");

# romldir/<chiprev> => chipimages/<chip> dir mapping.
# These are searched in chipimagesdir "$hndrte_dir/src/dongle/rte/chipimages";
# Each embedded has a different set of chip revs to track and
# may have a different owner to monitor and manager roml deviations
# The chipimages mapping below is to map chip image dir to chiprev
#
my %ChipImagesByBranch = (
	"FALCON_BRANCH_5_90"    => {
					chipimages => {
						'4336b1'  => '4336b1',
						'4330b1'  => '4330b1',
						'4330b2'  => '4330b2',
						'43362a0'  => '43362a0',
						'43362a2'  => '43362a2',
						'43236b1'  => '43236b1',
						'43237a0'  => '43237a0',
						'43239a0'  => '43239a0',
					},
					owner => 'kiranm',
	},
	"ROMTERM_BRANCH_4_218"	=> {
					chipimages => {
						'4329b1' => '4329b1',
						'4319b0' => '4319b0',
						'4319b1' => '4319b1',
					},
					owner => 'tanlu',
	},
	"ROMTERM3_BRANCH_4_220"	=> {
					chipimages => {
						'4329c0' => '4329c0',
					},
					owner => 'sushant',
	},
	"PHOENIX_BRANCH_5_120"	=> {
					chipimages => {
						'4334a0' => '4334a0',
					},
					owner => 'lut',
	},
	"PHOENIX2_BRANCH_6_10"	=> {
					chipimages => {
						'4324a0' => '4324a0',
						'4334b1' => '4334b1',
						'43143a0' => '43143a0',
					},
					owner => ['lut', 'pieterpg'],
	},
); # %ChipImagesByBranch

$ENV{PATH}   = "/bin:/usr/bin:/usr/local/bin:/tools/bin:/sbin:/usr/sbin:/projects/hnd/tools/linux/bin:$ENV{PATH}";

my $FIND        = "/tools/bin/find";
my(%commits);
my $domain      = "broadcom.com";
# If $reldir isn't specified with '-r' switch, then TOT ROML data is tracked!
# TOT doesn't have ROML code merge or builds though still
my $reldir      = $branch ? $branch : "NIGHTLY";
my @tousers     = scalar(@notify) ? @notify : qw(hnd-lpsta-list);
#my @ccusers    = scalar(@ccusers) ? @ccusers : qw(prakashd);
my @adminusers  = qw(hnd-software-scm-list);
# Build that generates roml related logfiles for this script's processing
my $brand       = defined($brand) ? $brand : "hndrte";

# By default compare previous (24 hours earlier) roml logs
$timewindow     = 24 unless ($timewindow);
AlertAdmin("specify -timewindow only in hours") unless ($timewindow =~ /^(\d+)$/);

my $urlprefix   = "http://home.sj.broadcom.com";
my $cvsreport   = "/home/$BUILDOWNER/src/tools/build/cvsreport";
my $touch       = "/home/$BUILDOWNER/bin/newtouch";
my $build_linux = "/projects/hnd/swbuild/build_linux";
my $build_admin = "/projects/hnd/swbuild/build_admin";
my $TEMP        = "/tmp";
my $todayts     = "";
my $prevts      = "";

# Only build user can run this report
if ($USER !~ /$BUILDOWNER/){
	Warn("USER=$USER is not build owner ($BUILDOWNER)");
	Warn("Symbol size generation and comparison can not be run");
	Exit($ERROR);
}

# Generate roml track report for this $report_date other than $today
if ($report_date && $report_date =~ /^\d{4}-\d{2}-\d{2}$/){
	$todayts = "$report_date";
}

# Set to $today to today's calender day
if (! $todayts){
	$todayts = qx(date '+%Y-%m-%d');
	chomp($todayts);
}

# Compute build iteration number (iteration dirname) from $today
my @todayiter   = split(/-/,$todayts);
   $todayiter[1]=~ s/0(\d)/$1/;
   $todayiter[2]=~ s/0(\d)/$1/;
my $todayiter   = "$todayiter[0].$todayiter[1].$todayiter[2]";

Dbg("Processing brand($brand) build $build_linux/$reldir/$brand");

# ROML tracking logs are generated in $brand build, so get the latest build
my $hndrte_dir  = qx($FIND $build_linux/$reldir/$brand -maxdepth 1 -mindepth 1 -type d -name "*${todayiter}*" 2> $NULL | sort | tail -1);
chomp($hndrte_dir);

Dbg("$todayts hndrte_dir=$hndrte_dir");

# Override build iteration via cmd line if -build is specified
$hndrte_dir  = $build_dir if ($build_dir && -d "$build_dir");

Dbg("Given hndrte_dir=$hndrte_dir") if ($build_dir);

# Exit on error if given hndrte_dir isn't found
if (! -d $hndrte_dir){
	Error("Build directory hndrte_dir = $hndrte_dir not found for $todayts");
	Error("Did the build fail at $build_linux/$reldir/$brand for $todayts?");
	Exit($ERROR);
}

# Default recipient for new branches is SCM/build admins
my @dbgusers      = @adminusers;
if ($ChipImagesByBranch{$branch}{owner}) {
   @dbgusers      = ($ChipImagesByBranch{$branch}{owner});
}

my $todayromldir  = "$hndrte_dir/src/dongle/rte/roml";
my $chipimagesdir = "$hndrte_dir/src/dongle/rte/chipimages";
my $prevduration  = $timewindow * 60 * 60; # in seconds to previous roml data/run
my $prevmaxdays   = 14; # Go upto two weeks back to search for previous symsize file
my $prevtmp       = qx(mktemp $TEMP/prevts_XXXXXX); chomp($prevtmp);
my $prevts        = qx($touch -B $prevduration $prevtmp; stat -c "%y" $prevtmp);
		  chomp($prevts); unlink $prevtmp;
   $prevts        = (split(/\s+/,$prevts))[0];

# This is where daily ROML tracking reports are archived
my $todaylogdir   = "$build_admin/logs/romltrack/$reldir/$brand/$todayts";
my $prevlogdir    = "$todaylogdir";
   $prevlogdir    =~ s/$todayts/$prevts/g;

if (! @wl_components) {
    my $usf = "$hndrte_dir/src/tools/release/WLAN.usf";
    open(USF, $usf) || die "$0: Error: $usf: $!\n";
    my($line) = grep { /^WLAN_COMPONENT_PATHS=/ } <USF>;
    close(USF);
    chomp $line;
    $line =~ s%^.*?=%%;
    for (split ' ', $line) {
	s%^src/%%;
	push(@wl_components, "$_/src/");
    }
    chomp @wl_components;
}

# If getCvsCommits() is enabled, search for cvs checkins in these folders
my @srcmodules    = (qw(wl/sys/ usbdev/dongle/ shared/ dongle/rte/ bcmcrypto/), @wl_components);
my $mod_regexp    = "^(".join("|",@srcmodules).")";
# $brand build generates these log file that are processed in this script
my @romlfiles     = qw(romdiff-detail.log romdiff-stats.log);
my $romlmd5sign   = "romtable.md5sign";
#push(@romlfiles,qw(romrefs.log romtable.S  romtable.md5sign  romtable.xref));
my %romldiffdata  = {};
if (scalar(@chiprevs) == 0) {
	Warn("Chip revisions NOT specified on command line");
	Warn("Generating chip-revs list from build directory");
	@chiprevs=qx(find $todayromldir -maxdepth 1 -mindepth 1 -type d 2> $NULL);
	chomp(@chiprevs);
	@chiprevs = map { basename($_) } sort @chiprevs;
	Warn("Will process default chips: @chiprevs");
	if (scalar(@chiprevs) == 0) {
		Error("Chiprevs list empty, can't continue with roml tracking");
		Exit($ERROR);
	}
	sleep(2);
}

# When a chiprev is specified on command line, ensure that
# it is present in the build or cvs directory
foreach my $chiprev (sort @chiprevs){
	if (! -d "$todayromldir/$chiprev"){
		Error("Chip rev directory '$chiprev' not found");
		Error("for $reldir build on $todayts");
		Error("at: $todayromldir");
		Exit($ERROR);
	}
}

# Go upto two weeks back to search for previous rom logdir
if (! -d $prevlogdir){
	for (my $prevday=1; $prevday < $prevmaxdays; $prevday++){
		my $tmptimewindow  = $timewindow + ($prevday * 24);
		my $tmpprevduration= $tmptimewindow * 60 * 60;
		my $tmpprevtmp     = qx(mktemp ${TEMP}/tmpprevts_XXXXXX);
		chomp($tmpprevtmp);
		`$touch -B $tmpprevduration $tmpprevtmp`;
		my $tmpprevts      = qx(stat -c "%y" $tmpprevtmp);
		chomp($tmpprevts);
		$tmpprevts      = (split(/\s+/,$tmpprevts))[0];
		my $tmpprevlogdir  = $todaylogdir;
		my $tmpprevlogdir  =~ s/$todayts/$tmpprevts/g;
		#Info("ts=$tmpprevts ld=$tmpprevlogdir");
		if (! -d "$tmpprevlogdir"){
			Dbg("Searching old build directories");
			$tmpprevlogdir = "$todayromldir";
			$tmpprevlogdir =~ s/$todayts/$prevts/g;
		}
		CmdQ("rm -f $tmpprevtmp");
		if (-d "$tmpprevlogdir"){
			Dbg("Found last roml at $tmpprevts");
			$prevlogdir  = "$tmpprevlogdir";
			$prevts      = "$tmpprevts";
			$prevlogdir = "$tmpprevlogdir";
			last;
		} else {
			Warn("$tmpprevts logdir not found. Skipping $prevday");
		}
	} # for prevday
} # ! -d prevlogdir

Dbg("Comparisons to be done with $prevts day");

my $todaytscmp  = $todayts; $todaytscmp =~ s/-//g;
my $prevtscmp   = $prevts;  $prevtscmp  =~ s/-//g;

if (!$report_date && ($prevtscmp >= $todaytscmp)) {
	Error("prevts=$prevts can't be greater than or equal to $todayts");
	Error("If you specified '-date <report_date>', then ensure that");
	Error("'-timewindow <hours>' specified referring a day previous to");
	Error("<report_date>");
	Exit($ERROR);
}

if ($report_date){
	Info("Setting  todayts   = $todayts");
	Info("Computed todayiter = $todayiter");
	Info("Computed hndrte_dir= $hndrte_dir");
	Info("Computed prevts    = $prevts");
}

# Output report in html format (currently not used)
my $htmlfile  = qx(mktemp ${TEMP}/msg_${brand}_XXXXXX); chomp($htmlfile);
# Output report in text format
my $textfile  = qx(mktemp ${TEMP}/msg_${brand}_XXXXXX); chomp($textfile);
   $htmlfile .= ".htm";
   $textfile .= ".txt";

##
## getCvsCommits ( )
## generate a list of cvs commits
## NOTE: Currently CVS history or reports is not searched for roml
## NOTE: deviations
##
sub getCvsCommits {
	my($cvsreportcmd,$user,$date,$time,$ts,$tz,$file,$version);
	my $CVSROOT = "/projects/cvsroot" unless ($CVSROOT);

	Dbg("Run getCvsCommits()");
	return if $skip_cvs_query;

	# TODO: now-timewidow time period needs to be adjusted to match
	# TODO: the actual hndrte build time
	$cvsreportcmd  = "$cvsreport -a -d $CVSROOT -f 'now-${timewindow}hours'";
	$cvsreportcmd .= $branch ? "-b '$branch'" : "-b ''";
    	Info("Running '$cvsreportcmd'");
    	open(COMMITS, "$cvsreportcmd |") || AlertAdmin("cvs report $cvsreport error");

	# TOT: Commit from brichter (2008-04-02 11:15 PDT)
	# TOB: Commit from jqliu on branch HORNET_BRANCH_4_210 (2008-04-02 10:47 PDT)
	while (<COMMITS>){
       		chomp();
		# If cvsreport for TOT returns any branch results, skip them and vice-a-versa
		next if /^$/;
		next if /^\s+$/;
		next if (/^Commit\s+from\s+\w+\s+on\s+branch\s+/ && ! $branch);
		next if (($_ !~ m/^Commit\s+from\s+\w+\s+on\s+branch\s+/)&& $branch);
		if (/^Commit\s+from\s+\w+\s+/){
			Dbg("Processing '$_' now");
			if ($branch){
				($user,$date,$time,$tz) = (split(/\s+/,$_))[2,6,7,8];
			} else {
				($user,$date,$time,$tz) = (split(/\s+/,$_))[2,3,4,5];
			}
			$ts = "$date $time";
			$ts =~ s%\(|\)%%g;
			next;
		}
		if (/^\s+src\s+(\S+[^ ]*)(.*)/){
			$file    = $1;
			$version = $2; $version =~ s/\s+//g;
			next if ($file !~ m/$mod_regexp/g);
			next if ($file !~ m/\.c|\.cpp|\.cc|\.h/gi);
			if (! grep { /$file/ } @{$commits{$user}}){
				Dbg("Found user=$user; file=$file; version=$version");
				push (@{$commits{$user}}, "$file ($ts; ver $version)");
			} else {
				Dbg("Skipping $file");
			}
		}
	}
	return;

} # getCvsCommits()

##
## Generate HTML ROML track/comparison report
## NOTE: This is not used currently
## genHtmlReport()
##
sub genHtmlReport {

	Dbg("Run genHtmlReport()");
	open(HTMLFILE, ">${htmlfile}") || AlertAdmin("Can't open ${htmlfile}!");

	select(HTMLFILE);
	print "<html xml:lang=\"en\" lang=\"en\">
	<head>
	<title> Checkins potentially breaking roml in $reldir</title>
	<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\" />
	</head>
	<body bgcolor=\"#ffffff\">\n";

	print "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\"> \n";
	print "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b><u>Checkins potentially breaking roml in $reldir</u></b></td></tr>\n";
	print "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b2><u>Please ensure your recent checkins do not break roml</u></b2></td></tr>\n";
	print "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b2><u>Be sure your change is configured out correctly</u></b2></td></tr>\n";
	print "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b2><u>Relevant CVS commits in last $days days on ${reldir}</u></b2></td></tr>\n";
	print "</table>\n";
	print "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\">\n";
	print "<b><tr style=\"font-size: 10pt; background-color: black; color: #ffffff\">\n";
	print "<td style=\"font-size: 12pt\">#</td>\n";
	print "<td>AUTHOR</td>\n";
	print "<td style=\"font-size: 12pt\">FILE NAME (date and revision)</td>\n";
	print "</tr></b>\n";

	my $listno = 1; my $rowcolor = 0;
	foreach my $user (sort keys %commits){
		foreach my $file (sort @{$commits{$user}}){
			$rowcolor = $rowcolor ? "background-color\: #FFFFCC" : "";
			print "<tr style=\"$rowcolor; font-size: 10pt\">\n";
			print "<td style=\"font-size: 12pt\">${listno}</td>\n";
			print "<td>$user</td>\n";
			print "<td style=\"font-size: 12pt\">$file</td>\n";
			print "</tr>\n";
			$rowcolor = $rowcolor ? "" : "background-color\: #FFFFCC";
			$listno++;
		}
	}
	print "</table>\n";
	print "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\">\n";
	print "<tr></tr>\n";
	print "<tr><td><b>Additional Information:</b></td></tr>\n";
	print "<tr><td><b>CVS Report</b>: ${urlprefix}/projects/hnd/software/cvsreport/</td></tr>\n";
	print "<tr><td><b>Build Log:</b> ${urlprefix}${hndrte_dir}/,release.log</td></tr>\n";
	print "</table>\n";
	print "</body>\n</html>\n";
	close(HTMLFILE);
	# End of html report
} # genHtmlReport

##
## genTextReport ()
##
sub genTextReport {
	my $header = "";

	Dbg("Run genTextReport()");
	# Start of text file
	open(TXT_FILE, ">${textfile}") || AlertAdmin("Can't open ${textfile}!");
	select(TXT_FILE);

	my $notified = "romdiff-stats.log";
	my @requested= keys %{$ChipImagesByBranch{$branch}{chipimages}};

	print "# ----------------------------------------------\n";
	print "# ROML Info/Twiki: http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BcmRomDiff\n";
	print "# ROML Report for: ", join(',',sort keys %{$romldiffdata{$todayts}}),"\n";
	print "# Brand: $brand; Branch/Tag: $reldir; Date: $nowtimestamp\n\n";
	print "# Chipimages dir: $todayromldir\n";
	if (@requested) {
	   print "# Requested Chiprevs: @requested\n";
	} else {
	   print "# ERROR: Requested Chiprevs list is empty\n";
	   print "# ERROR: Ensure that chiprev is added to src/tools/build/roml_report.p on trunk\n";
	   print "# ERROR: in %ChipImagesByBranch data structure\n";
	}
	print "# Available chip images: @chiprevs\n";
	print "\n";

	my $chiprevs_missing=0;
	foreach my $chiprev (@requested) {
		if (!grep {/${chiprev}$/} @chiprevs) {
			print "ERROR: $chiprev image missing at $todayromldir\n";
			$chiprevs_missing++;
		}
	}
	print "\n";
	if ($chiprevs_missing > 0) {
		print "INFO: Either correct src/tools/build/roml_tracking.pl\n";
		print "INFO: OR  create missing chip images at above paths\n";
	}
	print "\n";

	my $index = 1;
	foreach my $chiprev (sort keys %{$romldiffdata{$todayts}}) {
		my $chipimagerev = $romldiffdata{$todayts}{$chiprev}{chipimagerev};
		if (-f "$romldiffdata{$todayts}{$chiprev}{$notified}") {
			open(ROMLFILE,"$romldiffdata{$todayts}{$chiprev}{$notified}");
			print "=" x "70","\n";
			print "$index) [$chiprev $notified] DETAILS: \n";
			print "\nCompare with '$chipimagerev' $chiprev ROM Symbol Signatures\n\n";
			print "=" x "70","\n";
			while(<ROMLFILE>) {
				next if (/Full Diff:/);
				print $_;
			}
			close(ROMLFILE);
			print "\n";
			$index++;
		}
	}

	print "\n---- Links to archived reports: -----\n";
	my $index = 1;
	foreach my $chiprev (sort keys %{$romldiffdata{$todayts}}){
		foreach my $file (sort keys %{$romldiffdata{$todayts}{$chiprev}}){
			next if ($file =~ /chipimagerev/);
			Dbg("Found: $file = $romldiffdata{$todayts}{$chiprev}{$file}");
			print "$index) [$chiprev $file] $urlprefix/$romldiffdata{$todayts}{$chiprev}{$file}\n";
			$index++;
		}
		print "\n";
	}
	close(TXT_FILE);
	system("unix2dos -q $textfile");
	# End of text report

} # genTextReport

##
## genMsg ( )
##
sub genMsg {
	my $days = int($timewindow/24);

	Dbg("Run genMsg()");

	#disabled# genHtmlReport();
	genTextReport();

} # genMsg()

##
## sendMsg ( )
##
sub sendMsg {
	my (%seenu, @cvsusers);
	my $romldiffdata_found = 0;
	my $smtp;
	my $mfrom    = "hwnbuild\@$domain";
	my $mdate    = $nowdate; $mdate =~ s/\./\//g;
	my $msubject = "$reldir $brand ROML report [$mdate]";
	my $mbody    = qx(cat $textfile); chomp($mbody);
	my @requested= keys %{$ChipImagesByBranch{$branch}{chipimages}};

	if (!@requested) {
		$msubject = "ERROR: Missing info for $msubject";
	}

	Dbg("Run sendMsg()");
	#my $conv = new HTML::TextToHTML();
	#$conv->txt2html([*options*]);

	@cvsusers  = map { /\@$domain/gi ? "$_" : "$_\@$domain" }
		     sort grep { !$seenu{$_}++ } keys %commits;
	@tousers   = map { /\@$domain/gi ? "$_" : "$_\@$domain" } sort @tousers;
	@ccusers   = map { /\@$domain/gi ? "$_" : "$_\@$domain" } sort @ccusers;
	@dbgusers  = map { /\@$domain/gi ? "$_" : "$_\@$domain" } sort @dbgusers;

	push(@tousers,@cvsusers);

	foreach my $chiprev (@chiprevs) {
		if (exists $romldiffdata{$todayts}{$chiprev}) {
			Info("Including '$chiprev' data in ROML report");
			$romldiffdata_found=1;
		}
	}
	if (!$romldiffdata_found) {
		Warn("No ROML data to report to @tousers");
		Warn("@dbgusers will be alerted instead");
	}

	# If debug is enabled or no romdiff data files are found, alert admin users only.
	if ($dbg || $romldiffdata_found==0) {
		Dbg("Alerting only debug/admin users instead of @tousers");
		@tousers = @dbgusers;
		@ccusers = @dbgusers;
	}

	$mtousers = join(',',@tousers);
	$mccusers = join(',',@ccusers);

	Info("Email Report To  : $mtousers") if (@tousers);
	Info("Email Report Cc  : $mccusers") if (@ccusers);
	Info("Email Body: $textfile") if (-f ${textfile});
	Dbg("Email Host: $thishost");

	my $smtphost = "smtphost.broadcom.com";
	$smtp = Net::SMTP->new($smtphost, Debug=>$dbg) || AlertAdmin("Email Failed $textfile");
	$smtp->mail($mfrom);
	foreach $mto (@tousers) { $smtp->to("$mto"); }
	foreach $mcc (@ccusers) { $smtp->cc("$mcc"); }
	$smtp->data();
	$smtp->datasend("To: $mtousers\n");
	$smtp->datasend("Cc: $mccusers\n") if (@ccusers);
	$smtp->datasend("From: $mfrom\n");
	$smtp->datasend("Date: $mdate\n");
	$smtp->datasend("Subject: $msubject\n");
	$smtp->datasend("MIME-Version: 1.0\n");
	#TODO: Convert text report to HTML format before enabling this text/htm
	#$smtp->datasend("Content-type: text/html\n");
	$smtp->datasend("Content-type: text\n");
	$smtp->datasend("Content-Transfer-Encoding: 7bit\n");
	$smtp->datasend("\n");
	$smtp->datasend("$mbody\n\n");
	$smtp->dataend();
	$smtp->quit();

	sleep(2);
	system('mailq');
	if ($USER =~ /$BUILDOWNER/){
		#Cmd("cp $htmlfile $hndrte_dir/,${brand}_roml_report.htm");
		#Cmd("rm -f $htmlfile");
		CmdQ("cp $textfile $hndrte_dir/,${brand}_roml_report.txt");
		CmdQ("rm -f $textfile");
	} else {
		Warn("USER=$USER is not build owner ($BUILDOWNER)");
		Warn("$htmlfile not copied to $hndrte_dir");
		#CmdQ("rm -f $htmlfile");
	}
	CmdQ("rm -f $textfile") if (-f $textfile);

} # sendMsg()

##
## Copy build generated romdiff stats and details to central archive
##
## copyTodayLogs ( )
##
sub copyTodayLogs {
	my $chipimagerev="";

	# If a branch's %romldiffdata doesn't have a key for <chiprev>, then that chiprev can't
	# have its roml data tracked on that branch
	Dbg("Run copyTodayLogs()");
	File::Path::mkpath($todaylogdir);
	foreach my $chiprev (@chiprevs){
		foreach my $file (@romlfiles) {
			if (-f "$todayromldir/$chiprev/$file"){
				CmdQ("mkdir -pv $todaylogdir/$chiprev");
				if (! -f "$todaylogdir/$chiprev/$file"){
					CmdQ("cp -pv $todayromldir/$chiprev/$file $todaylogdir/$chiprev/$file");
				}
				$romldiffdata{$todayts}{$chiprev}{$file} = "$todaylogdir/$chiprev/$file";
				#chipimagesdir=xxx/src/dongle/rte/chipimages
				# my $cur_romlmd5sign="$chipimagesdir/${chipimagestable{$chiprev}}/$romlmd5sign";
				my $cur_romlmd5sign="$chipimagesdir/${ChipImagesByBranch{$branch}{chipimages}{$chiprev}}/$romlmd5sign";
				if ( ! -f "$cur_romlmd5sign" ) {
				   # Some times chip images
				   &Warn("-- WARN -- WARN -- WARN -- WARN --");
				   &Warn("");
				   &Warn("\$chipimages{$chiprev} mapping missing");
				   &Warn("Assuming chip images dir matches chip revision number");
				   &Warn("");
				   &Warn("-- WARN -- WARN -- WARN -- WARN --");
				   $cur_romlmd5sign="$chipimagesdir/$chiprev/$romlmd5sign";
				   $ChipImagesByBranch{$branch}{chipimages}{$chiprev} = "$chiprev";
				}
				open(CHIPREV, "$cur_romlmd5sign") ||
					AlertAdmin("$cur_romlmd5sign can't be opened");
				while (<CHIPREV>) {
					if (/#\s*CVS Tag:\s*(.*)$/) {
						$chipimagerev=$1;
						last;
					}
				}
				close(CHIPREV);
				$romldiffdata{$todayts}{$chiprev}{chipimagerev} =
					$chipimagerev ? $chipimagerev : "UNKNOWN";
			} else {
				Dbg("Missing $chiprev -> $file");
			}
		} # foreacho @romlfiles
		if (-f "$cur_romlmd5sign") {
			CmdQ("cp -pv $cur_romlmd5sign $todaylogdir/$chiprev/");
		}
	}
} # copyTodayLogs()

##
## Help ( )
##
sub Help {

	Dbg("Run Help()");

	print "

	Usage: $0 [-d/bg] [-h/elp] \
		[-branch <branch>] \
		[-brand <brand>] \
		[-build_dir <brand_iteration_dir>] \
		[-chiprev <chiprev1> -chiprev <chiprev2> ...] \
		[-date <report_date>] \
		[-mail <recipient1> -mail <recipient2> ...] \
		[-timewindow <timewindow>] \

	This script scans the $brand release.log file to track ROM library changes
	and reports to @tousers	

	Command line arguments supported by this program

	-branch     : Does ROML tracking on this branch (default: TOT)
	-brand      : Name of build brand that generates ROM logs (default: hndrte)
	-build_dir  : OR specify the path where $brand dir instance exists
			(This can be used to report on ROML changes in private build)
	-ccuser     : List of users to be notified of ROML changes
	-chiprevs   : Chip revisions to track for given branch/brand
	-date       : Generate ROML change report for some other day than $todayts
	-dbg        : Enable debug mode
	-help       : Show this help screen
	-mail       : Show this help screen
	-skip_cvs_query:
			Turn of cvs query to get a list of possible offenders
	-timewindow : If specified (in hours), ROML reports from $timewindow hours
			will be used if any comparison report is needed.
			This is not implemented still

	Example:
	1) To report ROML changes on TOT and mail to hnd-lpsta-list
		$0 -m hnd-build-list
	2) To report ROML changes on ROMTERM_BRANCH_4_218 and mail to prakashd and lut
		$0 -m prakashd -m lut -branch ROMTERM_BRANCH_4_218
";

	Exit($OK);

} # Help()

## Main
sub Main {

	Dbg("Run Main()");

	copyTodayLogs();
	#disabled# getCvsCommits();
	genMsg();
	sendMsg();
	Exit($OK);

} # Main()


## Start of program
Main();
