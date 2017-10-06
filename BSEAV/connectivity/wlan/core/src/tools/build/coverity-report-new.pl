#!/usr/bin/perl

## #########################################################################
##
## $ Copyright Broadcom Corporation 2003 $
##
## This program queries the coverity database and generates email
## reports to respective owners of various coverity targets.
## NOTE: Coverity target names specified here should match target
## NOTE: name format used in all-run-coverity.mk makefile
##
## Current Coverity stakeholders:
##
## 1. The administrators (Prakashd/Ray):
##     Will want nightly notification of what ran and its status.
## 2. The developers:
##    2.1 STA (XP/Vista NIC, Vista USB DHD and dongle) - Manoj/Ray
##    2.2 Embedded (Linux SDIO DHD and dongle, WinCE SDIO DHD and dongle, Vista USB DHD and dongle)- JQ/Ray
##    2.3 Router (nothing yet) - Trask/Ray
## 3. There is a weekly run of a all targets on TOT
##     The administrators will also get weekly consolidated status
##
## $Id$
##
## SVN: $HeadURL$
##
## #########################################################################
##

use     Env;
use     Getopt::Long;

# TODO. Make all symbols adhere to "use strict"
#use strict 'refs';

use lib  "/home/hwnbuild/src/tools/build";
require  "wl_swengg_lib.pl";

# Default release tag is TOT, if not specified explicitly
my $releasetag = "TOT";

my $reportdate;
my $gen_runcidmap;
my $help;
my $releasetag;
my $respin;
my $shownewdefects;
my $showrundiffs;
my $weeklybld;
my $bldadmin="hnd-software-scm-list";

## TODO: Description needed
my %scan_args = (
                'date=s'        => \$reportdate,
                'dbg'           => \$dbg,
                'gen_runcidmap' => \$gen_runcidmap,
                'help'          => \$help,
                'releasetag=s'  => \$releasetag,
                'spin'          => \$respin,
                'shownewdefects'=> \$shownewdefects,
                'showrundiffs'  => \$showrundiffs,
                'weeklybld'     => \$weeklybld,
             );

use lib  "/home/hwnbuild/src/tools/build";
require  "wl_swengg_lib.pl";

# Scan the command line arguments
GetOptions(%scan_args) or &Help;

&Dbg("Debug/Verbose mode enabled");

my $shownewdefects = 1; # if new defects can be found, show them
my $showrundiffs   = 1; # diff current run with previous run for a target run
my $newdefect_runs = 0; # Number of runs with new defects
my $rNightlyRuns   = {};
my $rWeeklyRuns    = {};
my $build_config   = "/home/hwnbuild/src/tools/build/build_config.sh";
my $svnwho         = "/projects/hnd_software/gallery/src/tools/misc/svnwho";
my $svncmd         = "/tools/bin/svn --non-interactive";
my $srctree        = "/projects/hnd_software/gallery/src_tree.txt";
my @srctree        = qx(cat $srctree); chomp(@srctree);
my $coverity_config= "gmake -s -f /home/hwnbuild/src/tools/release/coverity-target-info.mk";
my @ActiveBranches = split(/\s+/,qx($build_config --show_coverity_active)); chomp(@ActiveBranches);

# TODO: Move the common sections to a coverity common library implementation
if ( $hostenv =~ /win32/ ) {
   $PATHPFX = "Z:";
   $TEMP    = "C:/temp";
} else {
   $PATHPFX = "";
   $TEMP    = "/tmp";
}

my $PREVENT_DIR   = "${PATHPFX}/projects/hnd_tools/linux/Coverity/CurrentX86";
# Newly upgraded data to 4.5.1. Datadir name is misleading!
my $HNDWLANDB_DIR = "/var/lib/coverity/coverity-data";
my $QUERY_DB      = "${PREVENT_DIR}/bin/cov-query-db";
my $DB_USER       = "hwnbuild"; # user used to access/query db
my $COVPROCESS_URL= "http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/CoverityStaticCodeAnalysis";
my $PREVENT_WEBURL= "http://cov-sj1-06.sj.broadcom.com:8080";
my $PREVENT_RUNURL= "${PREVENT_WEBURL}/cov.cgi?run=RUN_ID&t=14&v=1";
my $PREVENT_NEWURL= "${PREVENT_WEBURL}/cov.cgi?c=AAAAAADA7g&hstate=1&q=6&runs=RUN_ID&t=6&v=1";
my $PREVENT_CMPURL= "${PREVENT_WEBURL}/cov.cgi?new=NEW_RUN_ID&old=OLD_RUN_ID&sort=b62&t=11&v=1";
my $BLDSERVER_BASE= "/projects/hnd_swbuild";
my $BLDSERVER_URL = "http://home.sj.broadcom.com";
my $COVBUILD_URL  = "${BLDSERVER_URL}${BLDSERVER_BASE}";
my $COVSUM_LOGDIR = "${BLDSERVER_BASE}/build_admin/logs/coverity/summary";
my $ERRORS        = "";
my $WARNINGS      = "";
my $RLSLOG        = ",release.log";
my $ERRORLOG      = ",build_errors.log";

my %BrandsByPlatform = (
	windows => qw(win-coverity),
	window  => qw(win-coverity),
	linux   => qw(linux-coverity),
	macos   => qw(macos-coverity),
	netbnsd => qw(netbsd-coverity),
);

if ( ( $reportdate ) && ( $reportdate !~ /(\d{4}).(\d{2}).(\d{2})/ ) ) {
   &Error3("Date=$reportdate wrong specifications, expecting 'YYYY.mm.dd'");
   $ERRORS .= "Date=$reportdate wrong specifications, expecting 'YYYY.mm.dd'\n";
   &Exit($ERROR);
}
$querydate     = `date '+%Y%m%d'`; chomp($querydate);
$rundate       = $nowdate;
## Set rundate (which is used in report email) to custom date if specified
if ( $reportdate =~ /(\d{4}).(\d{2}).(\d{2})/ ) {
  $querydate   = "$1$2$3";
  $rundate     = $reportdate;
}

&Info3("Date=$rundate");

$today         = `date '+%a'`;     chomp($today);
$weeklybld     = ( $today eq 'Sun' ) ? 1 : 0 if ( ! defined($weeklybld) );
$blddate       = `date '+%Y.%-m.%-d.{?,??}'`; chomp($blddate);

if ( $USER !~ /$DB_USER/ ) {
   &Error3("'$USER' user does not have coverity query database access");
   $ERRORS .= "'$USER' user does not have query coverity database access\n";
   &Exit($ERROR);
}

## Define functional modules in coverity and their owners
## These are referred as "categories" in this programs
$rCatInfo = {
  admin   => {
               owners  => [qw($bldadmin hayes)],
             },
  sta     => {
               owners  => [qw(hnd-coverity-list)],
             },
  esta    => {
               owners  => [qw(hnd-coverity-list)],
             },
  router  => {
               owners  => [qw(hnd-coverity-list)],
             },
}; # rCatInfo

# Given Target Description, identify its build OS and categorize
# to broadcom functional group. This will be help keep the reports
# going specific groups for faster closure
sub CategorizeTargets {
	my ($tgtType, $tgt, $tgtDesc) = @_;

	if ( $tgtDesc =~ /Win(XP|7|Vista).*WL.*(NIC|BMac).*Driver/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /Win(XP|7|Vista).*DHD.*Driver/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(esta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /Win(XP|7|Vista).*WL.*Utility/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /WinCE|WinMobile/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(esta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /prefast/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "window";
	} elsif ( $tgtDesc =~ /Linux.*WL.*(NIC|BMac).*Driver/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Linux.*DHD.*Driver/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(esta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Linux.*(WL|DHD).*Utility/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(esta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /(43...*bmac\/|Dongle)/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /(43...*\/|Dongle)/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(esta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Router/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(router)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Linux/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta esta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /Win/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta esta)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /VxWorks/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(router)];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	} elsif ( $tgtDesc =~ /MacOS/i ) {
		$tgtType->{$tgtDesc}{categories} = [qw(sta)];
		$tgtType->{$tgtDesc}{buildos}    = "macos";
	} else {
		&Warn3("$tgtDesc couldnot be categorized");
		$tgtType->{$tgtDesc}{categories} = [qw()];
		$tgtType->{$tgtDesc}{buildos}    = "linux";
	}

	# Mark all builds in admin category too
	push(@{$tgtType->{$tgtDesc}{categories}},"admin");
	&Info("Categories [@{$tgtType->{$tgtDesc}{categories}}] for $tgtDesc");
}

# Go through active branches and find out scheduled coverity runs on a
# nightly basis as well as weekly basis

foreach my $branch (@ActiveBranches) {
	my @branchNightlyTargets;
	my @branchWeeklyTargets;
	my @branchWeeklyTargetsOnly;

	@branchNightlyTargets = split(/\s+/, qx($coverity_config TAG=$branch show_coverity_targets));
	next unless ($#branchNightlyTargets > 0);
	&Dbg("$coverity_config TAG=$branch show_coverity_targets");
	&Info("branchNightlyTargets[$#branchNightlyTargets] = @branchNightlyTargets");

	if ($weeklybld) {
		&Info("Weekly Build Flag specified: $weeklybld");
		@branchWeeklyTargets  = split(/\s+/, qx($coverity_config WEEKLYBLD=1 TAG=$branch show_coverity_targets));
		foreach my $tgt (@branchWeeklyTargets) {
			push(@branchWeeklyTargetsOnly,$tgt)
			unless grep { m/$tgt/i } @branchNightlyTargets;
		}
		@branchWeeklyTargets  = @branchWeeklyTargetsOnly;
		&Dbg("$coverity_config WEEKLYBLD=1 TAG=$branch show_coverity_targets");
		&Info("branchWeeklyTargets[$#branchWeeklyTargets] = @branchWeeklyTargets");
	}
	
	# Categorize nightly targets
	foreach my $tgt (@branchNightlyTargets) {
		next if $tgt =~ /^\s*$/g;

		my $tgtDesc = qx($coverity_config $tgt);
		chomp($tgtDesc);

		$tgtDesc = "$branch $tgtDesc";
		&CategorizeTargets($rNightlyRuns,$tgt,$tgtDesc);
	}

	# Categorize weekly targets
	foreach my $tgt (@branchWeeklyTargets) {
		next if $tgt =~ /^\s*$/g;

		my $tgtDesc = qx($coverity_config $tgt);
		chomp($tgtDesc);

		$tgtDesc = "$branch $tgtDesc";
		&CategorizeTargets($rWeeklyRuns,$tgt,$tgtDesc);
	}
}

## Populate a new rAllRuns hash from rNightlyRuns and rWeeklyRuns
&Info("New run descriptions generated for report:") if ( $releasetag );
foreach my $run ( keys %$rNightlyRuns ) {
   next if ($run =~ /prefast/);
   # if releasetag is provided, use releasetag to replace hardcoded tagname
   $nrun = $run;
   $nrun =~ s/^(\w+\b)/$releasetag/g if ( $releasetag );
   &Info(" $nrun") if ( $releasetag );
   @{$rAllRuns->{$nrun}{categories}} = @{$rNightlyRuns->{$run}{categories}};
   $rAllRuns->{$nrun}{schedule}      = $rNightlyRuns->{$run}{schedule};
   $rAllRuns->{$nrun}{buildos}       = $rNightlyRuns->{$run}{buildos};
}

if ( $weeklybld ) {
   foreach my $run ( keys %$rWeeklyRuns ) {
      next if ($run =~ /prefast/);
      if ( defined $rAllRuns->{$run}{buildos} ) {
         &Error("$run is repeated between nightly and weekly schedules");
         &Error("Correct rNightlyRuns or rWeeklyRuns data");
         $ERRORS .= "$run is repeated between nightly and weekly schedules\n";
         $ERRORS .= "Correct rNightlyRuns or rWeeklyRuns data\n";
         &Exit($ERROR);
      }
      # if releasetag is provided, use releasetag to replace hardcoded tagname
      $nrun = $run;
      $nrun =~ s/^(\w+\b)/$releasetag/g if ( $releasetag );
      &Info(" $nrun") if ( $releasetag );
      @{$rAllRuns->{$nrun}{categories}} = @{$rWeeklyRuns->{$run}{categories}};
      $rAllRuns->{$nrun}{schedule}      = $rWeeklyRuns->{$run}{schedule};
      $rAllRuns->{$nrun}{buildos}       = $rWeeklyRuns->{$run}{buildos};
   }
}; # weeklybld

## Total number of scheduled coverity targets
$numtotal_sched = keys %$rAllRuns;

## Extract a list of targets that actually ran for ${querydate}
@todays_runs = qx(${QUERY_DB} -d ${HNDWLANDB_DIR} --mode runs --description \.\*${querydate}\.\*);
$db_exitcode=$?;

if ( ( $db_exitcode!= 0 ) || ( ! @todays_runs ) ) {
   &Error3("Coverity query_db exited with an error code");
   &Error("Error report will be sent to coverity admin user");
   $ERRORS .= "Coverity query_db exited with an error code ($db_exitcode)\n";
   $dbg = 1;
}

chomp(@todays_runs);
@todays_runs = map { $_ =~ s/\s+/ /g; "$_" } @todays_runs;

## In debug mode, email only to admin user
if ( defined($dbg) ) {
   $rdbgowner=[qw($bldadmin)];
   foreach my $category ( keys %$rCatInfo ) {
     $rCatInfo->{$category}{owners} = $rdbgowner;
   };
};

## Generate Run Vs CID mapping for all targets run on a day or a tag
sub genRunCidMap
{
   my ($cid,$defect,$run,$found_run,@all_defects,@all_found_cids);
   my (@failed_runs,@all_found_runs,$outstandingdefs,$uniquerun,$nruns);
   my ($seencid,%seenucid);

   my ($rRunCidMap);     # Run Vs CID matrix intersected by # of CID occurances
   my ($rCidCheckerMap); # Given a CID, show its associated CheckerName
   my ($rRunCidCount);   # How many times a current CID occurs in a run
   my ($rRunIdMap);      # Given a Run name, show its CID number
   my ($rRunOSDefsMap);  # Given a Run name, show outstanding defects in it
   my ($rUniqueCidMap);  # Collect number of CIDs per given run

   foreach my $run ( sort keys %$rAllRuns ) {
     $run =~ s/\s+/ /g;
     ($found_run) = grep {m/$run/i} @todays_runs;
     if ( ! $found_run ) { push(@failed_runs,$run); next; }
     $runid = (split(/,/,$found_run))[0];
     $outstandingdefs = (split(/,/,$found_run))[5];
     $rRunOSDefsMap->{$run} = $outstandingdefs;
     @all_defects  = qx(${QUERY_DB} -d ${HNDWLANDB_DIR} --mode defects --runid $runid | grep -v "^CID,"); chomp(@all_defects);
     ## CID,Checker,Classification,Owner,Severity,Action,Function,File
     foreach my $defect ( @all_defects ) {
        $cid = (split(/,/,$defect))[0];
        $rRunCidMap->{$cid}{$run}++;
        # Alternately all_found_cids can be a hash with func names intersecting
        # run vs cid (i.e. print func name instead of 1 or 0 in excel sheet)
        push(@all_found_cids,$cid);
        $rCidCheckerMap->{$cid} = (split(/,/,$defect))[1]; # map to checker
     }
     push(@all_found_runs,$run);
     $rRunIdMap->{$run} = $runid;
   }

   if ( @all_found_cids ) {
      open(OUT,">/tmp/runcid.csv") || die "Output CSV mapfile could not created";

      print OUT "\nCoverity,Run Vs CID Map,for date:,$rundate\n\n";
      # Print target run-id
      print OUT ",,";
      print OUT map { "RunID: $rRunIdMap->{$_}," } sort @all_found_runs;
      print OUT "\n";
      # Print branch names
      print OUT ",,";
      print OUT map { $_ =~ m/^(\w+\b)\s+/g; "$1," } sort @all_found_runs;
      print OUT "\n";
      # Print target/run name
      print OUT "CID,Checker,";
      print OUT map { $_ =~ /^(\w+\b\s+)(.*)/; "$2," } sort @all_found_runs;
      print OUT "\n";
      foreach my $cid ( sort { $b <=> $a } grep { !$seencid{$_}++ } @all_found_cids ) {
        print OUT "$cid,$rCidCheckerMap->{$cid},";
        foreach my $run ( sort @all_found_runs ) {
          # If a run has a defect with $cid, mark it as 1 else 0
          if ( defined $rRunCidMap->{$cid}{$run} ) {
             print OUT "$rRunCidMap->{$cid}{$run},";
             $rRunCidCount->{$run} += $rRunCidMap->{$cid}{$run};
          } else {
             print OUT "0,";
          }
          # Find out unique CIDs per run if any
          $nruns = keys %{$rRunCidMap->{$cid}};
          if ( $nruns == 1 ) {
             ($uniquerun) = (%{$rRunCidMap->{$cid}});
             # show info as "CIDnumber(CheckerName)"
             push(@{$rUniqueCidMap{$uniquerun}},"${cid}($rCidCheckerMap->{$cid})");
          }
        }
        print OUT "\n";
      }
      # Print outstanding (uninspected) defects
      print OUT "\n";
      print OUT ",Outstanding Defects:,";
      print OUT map { "$rRunOSDefsMap->{$_}," } sort @all_found_runs;
      # Print total defects
      print OUT "\n";
      print OUT ",Total Defects:,";
      print OUT map { "$rRunCidCount->{$_}," } sort @all_found_runs;
      # Print target run-id
      print OUT "\n";
      print OUT ",RunID:,";
      print OUT map { "(RunID: $rRunIdMap->{$_})," } sort @all_found_runs;
      # Print branch names
      print OUT "\n";
      print OUT ",Branch/Tag:,";
      print OUT map { $_ =~ /^(\w+\b)\s+/; "$1," } sort @all_found_runs;
      # Print target/run name
      print OUT "\n";
      print OUT ",Run/Target:,";
      print OUT map { $_ =~ /^(\w+\b\s+)(.*)/; "$2," } sort @all_found_runs;
      # Draw a separator line
      print OUT "\n";
      print OUT "-----,------------------,";
      print OUT map { "------------------," } sort @all_found_runs;
      # Print unique CID map
      print OUT "\n\n";
      print OUT ",Unique CIDs:";
      print OUT "\n";
      foreach my $run ( @all_found_runs ) {
         $nrun = $run;
         $nrun =~ s/^\w+\b\s+//g;
         print OUT ",$nrun,";
         print OUT map { "$_," } sort { $a <=> $b } grep { !$seenucid{$_}++ } @{$rUniqueCidMap{$run}};
         print OUT "\n";
      }
      # Print missed/failed build targets
      if ( @failed_runs ) {
         print OUT "\n";
         print OUT "* NOTE:,Missed Targets:\n";
         print OUT map { $_ =~ m/(^\w+\b)\s+(.*)/g; ",,$1 ->,$2,\n" } @failed_runs;
         print OUT "\n";
      }
      close(OUT);
   } else {
     &Warn3("CID list couldn't be extracted from '$rundate' runs");
     $WARNINGS .= "CIDs list couldn't be extracted from '$rundate' runs";
   }
}; #genRunCidMap()

## If new coverity defects are detected, then this function
## searches the coverity database for checkers that produced
## the new defect
sub showNewDefects
{
   my($myrun_id,$schedrun) = @_;
   my($ndefmsg,$ndef,@new_defects);
   my($cid,$checker,$class,$func,$file);
   my($myrun_newurl);
   my($branch, $def_author, $ver_author);
   my($filerev, $filerevtmp);

   my $buildos;
   my $COVBUILD_BRAND;
   my $blddir;
   my $bldsvnrev;
   my $svnurl;
   my $fileurl;

   &Dbg("showNewDefects('$myrun_id','$schedrun')");
   ($branch) = $schedrun =~ /^(\w+\b)/;

   $buildos= $rAllRuns->{$schedrun}{buildos};
   $COVBUILD_BRAND= $BrandsByPlatform{$buildos};

   $ndefmsg    = "";
   $myrun_newurl = $PREVENT_NEWURL;
   $myrun_newurl =~ s/RUN_ID/$myrun_id/;

   # Get New Defects.
   # Filter-out 3rd party components which have their own component name
   # different from "Other". In Coverity query-db default component that
   # corresponds to broadcom sources is called "Other", so only generate
   # for these Broadcom sources
   &Dbg("Get New Defects Cmd: ${QUERY_DB} -d ${HNDWLANDB_DIR} --mode defects --runid $myrun_id --show historical_state --show line --component Other | grep ',New,'");
   @new_defects = qx(${QUERY_DB} -d ${HNDWLANDB_DIR} --mode defects --runid $myrun_id --show historical_state --show line --component Other | grep ",New," | sort -u); chomp(@new_defects);
   if (@new_defects) {
      $blddir   = qx(find ${BLDSERVER_BASE}/build_${buildos}/$branch/${COVBUILD_BRAND}/$blddate -maxdepth 0 -mindepth 0 2> ${NULL} | xargs ls -1td); chomp($blddir);
      $bldsvnrev   = qx(cat $blddir/_SUBVERSION_REVISION);
      $bldsvnrev   =~ s/\s+//g;
      if ($branch =~ /NIGHTLY/) {
         $svnurl      = "${SVNROOT}/trunk";
      } else {
         $svnurl      = "${SVNROOT}/branches/$branch";
      }
   }

   # Massage the output to filter out fields that are really needed
   ## CID,Checker,Classification,Historical State,Owner,Severity,Action,Function,File,Line
   $ndefmsg  .= sprintf("    ----new defects in run-id $myrun_id----------------------\n");
   $ndefmsg  .= sprintf("    %-4s %-15.15s  %-8s  %-20s  %-s\n","CID","Checker","Author","File[Line]", "Function");
   $ndefmsg  .= sprintf("    ------------------------------------------------------------------\n");
   foreach my $ndef ( @new_defects ) {
      ($cid,$checker,$func,$filefullpath,$line) = (split(/,/,$ndef))[0,1,7,8,9];

      my $filelinkpath=qx(readlink -q $filefullpath); chomp($filelinkpath);

      $filepath= ($filelinkpath ? $filelinkpath : $filefullpath);
      # Remove leading absolute path
      $filepath=~ s%.*/src/%src/%g;

      $file    = qx(basename $filefullpath); chomp($file);

      # Extra sv version number from $file. If it can't be
      # extracted successfully, set filerev to branch or HEAD for TOT
      $fileurl   = "${svnurl}/$filepath";
      `$svncmd ls $fileurl > $NULL 2>&1`;
      if ($? != "0") {
         ($filepath) = grep { /\/${file}$/ } @srctree;
         $fileurl   = "${svnurl}/$filepath";
         &Info("Recomputed filepath=$filepath");
      }

      ($filerev) = (split(/\s+/,qx($svncmd info $fileurl | grep "^Revision: ")))[1];
      $filerev   =~ s/\s+//g;
      # If filerev couldn't be derived, set it to build SVN rev
      if ($filerev !~ /^\d+$/) {
         $filerev = $bldsvnrev;
      }

      # First check annotate runs on path found by build, if not go to svn path
      if ( $filepath ) {
         $def_author = qx($svnwho -r $filerev $fileurl $line 2> $NULL);
         chomp($def_author);
         $ver_author_line = qx($svncmd log -q -r $filerev $fileurl | grep "$filerev");
         ($ver_author) = (split(/\|/, $ver_author_line))[1];
         $ver_author =~ s/\s+//g;
         chomp($ver_author);

         # Validate if the identified user still valid user or not
         `id $def_author > $NULL 2>&1`;
         if ( "$?" != "0" ) { $def_author = "*goner*"; }
         `id $ver_author > $NULL 2>&1`;
         if ( "$?" != "0" ) { $ver_author = "*goner*"; }

      } else {
         $def_author = "unknown";
      }

      system("$svnwho -r $filerev $fileurl $line");
      &Info("blddir=$blddir");
      &Info("bldsvnrev=$bldsvnrev");
      &Info("$svnwho -r $filerev $fileurl $line");
      &Info("filelinkpath=$filelinkpath");
      &Info("filefullpath=$filefullpath");
      &Info("filepath=$filepath");
      &Info("filerev=$filerev");
      &Info("bldsvnrev=$bldsvnrev");
      &Info("defect_author=$def_author");
      &Info("version_author=$ver_author");
      $ndefmsg.= sprintf("    %-4s %-15.15s  %-8s  %-20s  %-s()\n",$cid,$checker,$def_author,"$file\[$line\]",$func);
      push(@{$svnCheckins{$branch}},$file);

      foreach my $category ( @{$rAllRuns->{$schedrun}{categories}} ) {
         push(@{$rCatInfo->{$category}{notifylist}},$def_author) unless ($def_author =~ /unknown|goner/);
         push(@{$rCatInfo->{$category}{cclist}},$ver_author) unless ($ver_author =~ /unknown|goner/);
      }
   }
   $ndefmsg  .= sprintf("    URL: $myrun_newurl\n");
   $ndefmsg  .= sprintf("    File last checked in by: $ver_author\n");
   $ndefmsg  .= sprintf("    -------------------------------------------------------\n");

   if ( @new_defects ) {
      return("$ndefmsg");
   } else {
      # &Warn("No new defects were found for $myrun_id");
      return("");
   }

}; # showNewDefects

## If new coverity defects are detected, then this function
## searches the coverity database for checkers that actually
## failed in new run as compared to similar previous target run
## This is heuristic approach to show the diffs
sub showRunDiffs
{
   my($myrun_id,$schedrun) = @_;
   my($myrdiffmsg,$run,$cur_runid,$prev_runid,@all_sched_runs);
   my(@run_diffs,$checker,$diff,$cid1,$cid2);
   my($old,$new,$checker,$class,$func,$file);
   my($myrun_cmpurl);
   my($branch,$schedrunregex);

   &Info("showRunDiffs('$myrun_id','$schedrun')");

   ($branch) = $schedrun =~ /^(\w+\b)/;
   $schedrunregex = $schedrun;
   $schedrunregex =~ s/\s+/\.\*/g;
   $myrdiffmsg  = "";
   $cur_runid   = $prev_runid = 0;
   $rdiffsfound = 0;

   $myrun_cmpurl = ${PREVENT_CMPURL};

   &Info("Fetch previous runs for $schedrun");
   # Query all runs similar to failed target run (e.g. match xp nic to xp nic)
   @all_sched_runs = qx(${QUERY_DB} -d ${HNDWLANDB_DIR} --mode runs --description "\.\*${schedrunregex} \.\*" | grep -v "^Old,"); chomp(@all_sched_runs);

   # Find current and previous run id and branch prefix
   # Don't want to compare same target from one branch to other
   foreach my $run ( @all_sched_runs ) {

      &Dbg("run=$cur_runid");
      if ( $cur_runid ) {
         $prev_runid     = (split(/,/,$run))[0];
         $prev_rundesc   = (split(/,/,$run))[2];
         $prev_runbranch = (split(/\s+/,$prev_rundesc))[0];
         $prev_runbrpfx  = (split(/_/,$prev_runbranch))[0];
         last;
      }

      next if ( $run !~ /$querydate/ ); # skip until we get to ${querydate}'s run

      $cur_runid      = (split(/,/,$run))[0];
      $cur_rundesc    = (split(/,/,$run))[2];
      $cur_runbranch  = (split(/\s+/,$cur_rundesc))[0];
      $cur_runbrpfx   = (split(/_/,$cur_runbranch))[0];
      next;
   }

   if ( $cur_runid != $myrun_id ) {
      &Warn("Query db didn't yield the same run-id as supplied to this routine");
      &Warn("cur_runid=$cur_runid; run_id=$myrun_id");
      $WARNINGS .= "Query db didn't yield the same run-id as supplied to this routine\n";
      $WARNINGS .= "cur_runid=$cur_runid; run_id=$myrun_id\n";
      return("");
   }

   if ( ( $cur_runid == $prev_runid ) || ( ! $cur_runid || ! $prev_runid ) ) {
      &Warn("Failed: Fetch current and previous run-id values for '$schedrun'")
      &Warn("Failed: cur_runid = $cur_runid; prev_runid = $prev_runid");
      $WARNINGS .= "Failed: Fetch current and previous run-id values for '$schedrun'\n";
      $WARNINGS .= "Failed: cur_runid = $cur_runid; prev_runid = $prev_runid\n";
      return("");
   }

   if ( $cur_runbrpfx != $prev_runbrpfx ) {
      &Warn("Current and previous runs are not on same branch");
      &Warn("cur_runbrpfx = $cur_runbrpfx; prev_runbrpfx = $prev_runbrpfx");
      &Warn("Skipping comparison of target with previous run");
      $WARNINGS .= "Current and previous runs are not on same branch\n";
      $WARNINGS .= "cur_runbrpfx = $cur_runbrpfx; prev_runbrpfx = $prev_runbrpfx\n";
      $WARNINGS .= "Skipping comparison of target with previous run\n";
      return("");
   }

   # Diff current and previous run ids
   @run_diffs = qx(${QUERY_DB} -d ${HNDWLANDB_DIR} --mode defects --runid $cur_runid --diffrunid $prev_runid | grep -v "Old,New"); chomp(@run_diffs);

   # Massage the output to filter out fields that are really needed
   ## Old,New,Checker,Classification,Owner,Severity,Action,Function,File
   $myrdiffmsg .= sprintf("    ----diff of coverity runs $cur_runid(current) and ${prev_runid}(previous)---------\n");
   $myrun_cmpurl =~ s/NEW_RUN_ID/${cur_runid}/g;
   $myrun_cmpurl =~ s/OLD_RUN_ID/${prev_runid}/g;
   $myrdiffmsg  .= sprintf("    %-6s %-6s %-18s %-18s  %-s\n","Old","New","Checker","File","Function");
   $myrdiffmsg  .= sprintf("    ------------------------------------------------------------------\n");
   foreach my $diff ( @run_diffs ) {
      $cid1=0; $cid2=0;
      ($cid1,$cid2) = (split(/,/,$diff))[0,1];
      next if ( "$cid1" eq "$cid2" );
      ($old,$new,$checker,$func,$file) = (split(/,/,$diff))[0,1,2,7,8];
      $file = `basename $file`; chomp($file);
      $myrdiffmsg .= sprintf("    %-6s %-6s %-18s %-18s  %-s()\n",$old,$new,$checker,$file,$func);
      $rdiffsfound = 1;
      push(@{$svnCheckins{$branch}},$file);
   }
   $myrdiffmsg .= sprintf("    url: $myrun_cmpurl\n");
   $myrdiffmsg .= sprintf("    -------------------------------------------------------\n");

   if ( $rdiffsfound ) {
      return("$myrdiffmsg");
   } else {
      # &Warn("No diffs were found between $cur_runid and $prev_runid");
      return("");
   }

}; # showRunDiffs

## Scan runs in rAllRuns hash
sub scanRuns
{
  ## Initialize counters
  $numpassed = $numfailed = 0;

  ## Now check the status of each coverity target and generate messages
  foreach my $schedrun ( sort keys %$rAllRuns ) {
   @fruns      = ();
   @runstrings = ();
   $msg        = "";
   $ndefmsg    = "";
   $rdiffmsg   = "";

   $runstring = "$schedrun $querydate";
   &Dbg("ScanRuns()");
   &Dbg("Scanning Run: $runstring");
   push(@runstrings,"$runstring");
   ($branch) = $schedrun =~ /^(\w+\b)/;

   foreach my $runstring ( @runstrings ) {
     my $msg_header = "";
     my $msg        = "";

     $runstrings =~ s/\s+/ /g;
     @frunstring = grep { m/$runstring/i } @todays_runs;
     if ( @frunstring ) {
        push(@fruns, @frunstring);
        foreach my $frun ( @frunstring ) {
          @frun_details     = split(/,/,$frun);
          $frun_id          = $frun_details[0];
          $frun_desc        = $frun_details[2];
          $frun_newdefects  = $frun_details[4];
          $frun_outstanding = $frun_details[5];
          $frun_files       = $frun_details[6];
          $frun_runurl      = $PREVENT_RUNURL;
          $frun_runurl      =~ s/RUN_ID/$frun_id/;

          &Dbg("============== $frun_desc ==========================");
          if ( $frun_newdefects > 0 ) {
             $msg_header  = "* $frun_desc";
          } else {
             $msg_header  = "  $frun_desc";
          }
          $msg_header .= " [weekly-run]" if ( grep { /$frun_desc/ } keys %$rWeeklyRuns );
          $msg_header .= "\n";
          $msg_header .= "    $frun_newdefects new defects, ";
          $msg_header .= "$frun_outstanding outstanding defects, ";
          $msg_header .= "$frun_files files analyzed\n";
          $msg_header .= "    ID=$frun_id, Run Details: $frun_runurl\n";
          if ( $frun_newdefects > 0 ) {
             # Show new defects in run-id. Sometimes this output may be null
             # due to a bug in coverity query database utility.
             $ndefmsg  = &showNewDefects($frun_id,"$schedrun") if ( $shownewdefects );
             $newdefect_runs++;
          }
          # Just run the heuristic diffs between two sequential
          # coverity build targets
          $rdiffmsg = &showRunDiffs($frun_id,"$schedrun","$frun_desc") if ( $showrundiffs );
          foreach my $category ( @{$rAllRuns->{$schedrun}{categories}}) {
            $rCatInfo->{$category}{pass}      .= "$msg_header $msg\n";
            if ( $frun_newdefects > 0 ) {
               $rCatInfo->{$category}{newdefects} .= "$msg_header $msg\n";
               $rCatInfo->{$category}{newdefects} .= "$ndefmsg\n" if ( $ndefmsg );
            }
            $rCatInfo->{$category}{pass} .= "$rdiffmsg\n" if ( $rdiffmsg );
          }

          $numpassed++;
        }
     } else {
	my $buildos= $rAllRuns->{$schedrun}{buildos};
	my $COVBUILD_BRAND= $BrandsByPlatform{$buildos};

        &Dbg("buildos = $buildos COVBUILD_BRAND = $COVBUILD_BRAND");

        $blddir   = qx(find ${BLDSERVER_BASE}/build_${buildos}/$branch/${COVBUILD_BRAND}/$blddate -maxdepth 0 -mindepth 0 2> ${NULL} | xargs ls -1td); chomp($blddir);
        &Dbg("ERRORLOG=$blddir/$ERRORLOG");
        if ( -s "${blddir}/${ERRORLOG}" ) {
           # Match build error and warning calculations to build_summary.sh
           # This also ensures that one can cut and paste the cmd to run it
           $ignorewarnings='has modification time in the future|clock skew detected';
           $blderrors   = qx(grep "Error [0-9]\\+[[:space:]]*\$" ${blddir}/${RLSLOG} |  wc -l | xargs printf "%d"); chomp($blderrors);
           $bldwarnings = qx(grep "warning" ${blddir}/${RLSLOG} | egrep -v -i "${ignorewarnings}"  | wc -l | xargs printf "%d"); chomp($bldwarnings);
           &Dbg("blderrors=$blderrors; bldwarnigns=$bldwarnings");
           $msg  = "  $runstring FAILED";
           $msg .= "  ($blderrors errors, $bldwarnings warnings)\n";
           $msg .= "    Errors: ${BLDSERVER_URL}/${blddir}/${ERRORLOG}\n";
           $msg .= "    Log   : ${BLDSERVER_URL}/${blddir}/${RLSLOG}\n";
        } else {
           $msg      = "  $runstring FAILED (missing build)\n";
           $msg .= "    Build : ${COVBUILD_URL}/build_$rAllRuns->{$schedrun}{buildos}/$branch/${COVBUILD_BRAND}/\n";
        }
        foreach my $category ( @{$rAllRuns->{$schedrun}{categories}}) {
          $rCatInfo->{$category}{fail} .= "$msg\n";
        }
        $numfailed++;
     }
   }
  }
}; # scanRuns

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
  if ( $weeklybld ) {
    print $CATFH "Coverity *Weekly* Run Summary of ALL targets for ${rundate}\n\n";
  } else {
    print $CATFH "Coverity Nightly Run Summary of ALL targets for ${rundate}\n\n";
  }
  print $CATFH "Release Tag         : $releasetag\n" if ( $releasetag );
  print $CATFH "Report Time         : ${nowtimestamp}\n";
  if ( $newdefect_runs > 0 ) {
    print $CATFH "Coverity Run Status : ${numtotal_sched} scheduled, ${numpassed} ran, ${numfailed} failed, $newdefect_runs with new defects\n\n";
  } else {
    print $CATFH "Coverity Run Status : ${numtotal_sched} scheduled, ${numpassed} ran, ${numfailed} failed\n\n";
  }
  if ( $ERRORS ne '' ) {
    print $CATFH "=== COVERITY REPORTING ERRORS ============================\n";
    print $CATFH "$ERRORS\n";
  }

  if ( $WARNINGS ne '' ) {
    print $CATFH "=== COVERITY REPORTING WARNIGNS ==========================\n";
    print $CATFH "$WARNINGS\n";
  }
  if ( defined($rCatInfo->{$category}{fail}) ) {
     &Dbg("$rCatInfo->{$category}{fail}");
     print $CATFH "=== COVERITY TARGETS FAILED TO RUN =====================\n";
     print $CATFH "$rCatInfo->{$category}{fail}\n";
  }
  if ( defined($rCatInfo->{$category}{newdefects}) ) {
     &Dbg("$rCatInfo->{$category}{newdefects}");
     print $CATFH "=== COVERITY TARGETS WITH NEW DEFECTS ==================\n";
     print $CATFH "$rCatInfo->{$category}{newdefects}\n";
  }
  if ( defined($rCatInfo->{$category}{svncheckins}) ) {
     &Dbg("$rCatInfo->{$category}{svncheckins}");
     print $CATFH "=== RELEVANT SVN CHECKINS ==================\n";
     print $CATFH "$rCatInfo->{$category}{svncheckins}\n";
  }
  if ( defined($rCatInfo->{$category}{pass}) ) {
     &Dbg("$rCatInfo->{$category}{pass}");
     print $CATFH "=== COVERITY TARGETS SUCCESSFULLY RAN ==================\n";
     print $CATFH "$rCatInfo->{$category}{pass}\n";
  }
  close($CATFH);
  &Info("Notifying $catname summary to '@{$rCatInfo->{$category}{owners}}' now");
  $subject  = "";
  $subject .= "COVERITY ERROR:"    if ( $ERRORS   ne '' );
  $subject .= "COVERITY WARNINGS:" if ( $WARNINGS ne '' );
  if ( $numfailed > 0 ) {
     if ( $weeklybld ) {
       $subject .= "Coverity WEEKLY Run Summary ";
     } else {
       $subject .= "Coverity Run Summary ";
     }
     if ( $newdefect_runs > 0 ) {
        $subject .= "($numfailed of ${numtotal_sched} FAILED, found $newdefect_runs with new defects) for ${rundate}";
     } else {
        $subject .= "($numfailed of ${numtotal_sched} FAILED) for ${rundate}";
     }
  } else {
     if ( $weeklybld ) {
       $subject .= "Coverity WEEKLY Run Summary ";
     } else {
       $subject .= "Coverity Run Summary ";
     }
     if ( $newdefect_runs > 0 ) {
        $subject .= "(all ${numtotal_sched} ran, found $newdefect_runs with new defects) for ${rundate}";
     } else {
        $subject .= "(all ${numtotal_sched} ran) for ${rundate}";
     }
  }
  $subject .= "[RESPIN]" if ( $respin );
  $subject .= "[TAG:$releasetag]" if ( $releasetag );
  if ( $USER =~ /$DB_USER/ ) {
     my(@tolist,@cclist);

     push(@tolist, @{$rCatInfo->{$category}{owners}});

     if ($dbg) {
        &Dbg("Reset tolist=cclist=$bldadmin");
        @tolist = @cclist = qw($bldadmin);
     }

     &Info("Notifying $catname summary To:'@tolist' now");
     `cat $catmsg   | mail -s '$subject' @tolist`;
     sleep 5;
     if ( ! -s "${COVSUM_LOGDIR}/${category}_${rundate}.txt") {
        system("cp -v $catmsg ${COVSUM_LOGDIR}/${category}_${rundate}.txt");
        system("unix2dos ${COVSUM_LOGDIR}/${category}_${rundate}.txt");
     }
     system("rm -fv $catmsg");
   } else {
     &Info("Email notification disabled. Check '$category' report '$catmsg'");
   }
}; # notifyCategoryAdmins

## Notify individual module owners only if there are new defects
sub notifyCategoryOwners
{
  my ($category) = shift;
  my ($catname,$catfh,$catmsg);
  my ($notifyusers);

  $catname = uc($category);
  $CATFH   = uc($category);

  if ( ! defined($rCatInfo->{$category}{newdefects}) ) {
     &Info("No new defects found in $catname category");
     return;
  }

  $notifyusers = join(',',@{$rCatInfo->{$category}{notifylist}});

  $catmsg  = qx(mktemp ${TEMP}/cov${category}.XXXXXX);
  chomp($catmsg);
  chmod 0644, $catmsg;
  open($CATFH,  ">$catmsg")   || die "Can't open $catmsg";

  print $CATFH "Coverity: New Defects Found in $catname for $rundate\n";
  print $CATFH "Coverity: New Defects For: $notifyusers\n";
  print $CATFH "Coverity: Resolve Process: $COVPROCESS_URL\n";
  print $CATFH "\n\n";

  if ( defined($rCatInfo->{$category}{newdefects}) ) {
     print $CATFH "=== COVERITY TARGETS WITH NEW DEFECTS ==================\n";
     print $CATFH "$rCatInfo->{$category}{newdefects}\n";
  }
  if ( defined($rCatInfo->{$category}{svncheckins}) ) {
     &Dbg("$rCatInfo->{$category}{svncheckins}");
     print $CATFH "=== RELEVANT SVN CHECKINS ==================\n";
     print $CATFH "$rCatInfo->{$category}{svncheckins}\n";
  }
  if ( defined($rCatInfo->{$category}{pass}) ) {
     print $CATFH "=== COVERITY TARGETS SUCCESSFULLY RAN ==================\n";
     print $CATFH "$rCatInfo->{$category}{pass}\n";
  }
  close($CATFH);
  if ( $USER =~ /$DB_USER/ ) {
     my(@tolist,@cclist);

     push(@tolist, @{$rCatInfo->{$category}{owners}});
     push(@tolist, @{$rCatInfo->{$category}{notifylist}});
     push(@cclist, @{$rCatInfo->{$category}{cclist}});

     if ($dbg) {
        &Dbg("Reset tolist=cclist=$bldadmin");
        @tolist = @cclist = qw($bldadmin);
     }

     &Info("Notifying $catname defects to To:'@tolist' Cc:'$cclist' now");
     `cat $catmsg    | mail -s 'Coverity New Defects Found in $catname targets for ${rundate}' -c "@cclist" @tolist`;
     sleep 5;
     if ( ! -s "${COVSUM_LOGDIR}/${category}_${rundate}.txt") {
       system("cp -v $catmsg ${COVSUM_LOGDIR}/${category}_${rundate}.txt");
        system("unix2dos ${COVSUM_LOGDIR}/${category}_${rundate}.txt");
     }
     system("rm -fv $catmsg");
   } else {
     &Info("Email notification disabled. Check '$category' report '$catmsg'");
   }

}; #notifyCategoryOwners

sub Main
{
  if ( $gen_runcidmap ) {
     &genRunCidMap;
  } else {
     &scanRuns;

     # If coverity query db returns any errors, notify admin users
     if ( "$ERRORS" ne '' ) {
        $rdbgowner=[qw($bldadmin)];
        foreach my $category ( keys %$rCatInfo ) {
          $rCatInfo->{$category}{owners} = $rdbgowner;
        };
     }
     &notifyCategoryAdmins("admin");
     &notifyCategoryOwners("esta");
     &notifyCategoryOwners("sta");
     #&notifyCategoryOwners(router);
  }
}

sub Help
{
	print "Usage: $0 -\n";
}

&Main;
