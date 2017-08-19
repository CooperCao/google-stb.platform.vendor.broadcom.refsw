#!/usr/bin/perl
#
# This script processes the image size violations in dongle image builds
# done as part of hndrte-dongle-wl build brand. If it finds any image size
# exceeded instances, then it queries cvs to get list of potential checkins
# and alerts the users who checked those changes in.
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#
# SVN: $HeadUrl$
#

use     Env;
use     Getopt::Long;

# When TOT owner changes, update this TOT_OWNER to new owner
# branches have their own owners in %PRDetails hash
my $TOT_OWNER = "rtrask";

##
## Command line arguments supported by this program
##
## branch    :   Use this if any TOB image fails size check (default: TOT)
## brand     :   Check for failed builds for this brand (default: hndrte)
##               (other brands, like locator etc., can be used. But not
##               tested still)
## dbg       :   Enable debug mode
## help      :   Help/Usage info
## ccuser    :   list of users who volunteer to be notified in Cc: list
## timewindow:   Specify a time-window of cvs checkins in hours (default: 24hrs)
##

%scan_args = (
                'branch=s'      => \$branch,
                'brand=s'       => \$brand,
                'brand_dir=s'   => \$brand_dir,
                'ccuser=s'      => \@ccusers,
                'date=s'        => \$report_date,
                'dbg'           => \$dbg,
                'forcesymcmp'   => \$forcesymcmp,
                'gen_prtypes'   => \@gen_prtypes,
                'help'          => \$help,
                'skip_cvs_query'=> \$skip_cvs_query,
                'timewindow=s'  => \$timewindow,
		'wl_components=s' => \@wl_components,
             );

use lib  "/home/hwnbuild/src/tools/build";
require  "wl_swengg_lib.pl";

my $SYMCMP = "/home/hwnbuild/src/tools/build/symcmp.pl";

$ENV{PATH}   = "/bin:/usr/bin:/usr/local/bin:/tools/bin:/sbin:/usr/sbin:/projects/hnd/tools/linux/bin:$ENV{PATH}";

# Scan the command line arguments
GetOptions(%scan_args) or &Error("GetOptions failed!");
&checkCmdLineArgs();

$svncmd      = "/tools/bin/svn";
$reldir      = $branch ? $branch : "NIGHTLY";
$brand       = "hndrte-dongle-wl" unless ( $brand );
$SVNROOT     = "http://svn.sj.broadcom.com/svn/wlansvn/proj";

$urlprefix   = "http://home.sj.broadcom.com";
$cvsreport   = "/home/hwnbuild/src/tools/build/cvsreport";
$touch       = "/home/hwnbuild/bin/newtouch";
# List of modules impacting hndrte image size failures
if (! @wl_components) {
    if (open(WLAN_USF, "src/tools/release/WLAN.usf")) {
	@wl_components = map { s%.*WLAN_COMPONENT_PATHS=%%; split ' ' }
			 grep { /WLAN_COMPONENT_PATHS=/ }
			 <WLAN_USF>;
	close(WLAN_USF);
	chomp @wl_components;
    }
    if (! @wl_components) {
	@wl_components = qw(src/shared/bcmwifi components/clm-api);
    }
}
@srcmodules  = (qw(src/include/ src/wl/sys/ components/phy/old src/usbdev/dongle/ src/shared/ src/dongle/rte/ src/bcmcrypto/),
    map { ("$_/include/", "$_/src/") } @wl_components);
$mod_regexp  = "^(".join("|",@srcmodules).")";
$TEMP        = "/tmp";
@dbgusers    = qw(hnd-software-scm-list);
push(@gen_prtypes,"hardlimit"); # Always create PRs for hardlimit violations
@gen_prtypes = map {lc} @gen_prtypes;

if ( $report_date && $report_date =~ /^\d{4}-\d{2}-\d{2}$/ ) {
   $todayts = "$report_date";
} elsif ( $report_date && $report_date =~ /^\d{4}-\d{2}-\d{2}$/ ) {
   &Error3("Wrong specification of '-date $report_date' option");
   &Exit($ERROR);
}
if ( ! $todayts ) {
   $todayts     = qx(date '+%Y-%m-%d'); chomp($todayts);
}
@todayiter   =split(/-/,$todayts);
$todayiter[1]=~ s/0(\d)/$1/;
$todayiter[2]=~ s/0(\d)/$1/;
$todayiter   = "$todayiter[0].$todayiter[1].$todayiter[2]";

$hndrte_dir  = qx(find /projects/hnd/swbuild/build_linux/${reldir}/${brand} -type d -maxdepth 1 -mindepth 1 -name "*${todayiter}*" | sort | tail -1);
chomp($hndrte_dir);
$hndrte_dir  = $brand_dir if ( $brand_dir && -d "$brand_dir" );
#&Dbg("hndrte_dir = $hndrte_dir");
$todayimgstat= "${hndrte_dir}/src/dongle/rte/wl/imgstat.log";
$prevduration= $timewindow * 60 * 60;
$prevmaxdays = 14; # Go upto two weeks back to search for previous symsize file
$prevtmp     = qx(mktemp ${TEMP}/prevts_XXXXXX); chomp($prevtmp);
$prevts      = qx($touch -B $prevduration $prevtmp; stat -c "%y" $prevtmp); chomp($prevts);
$prevts      = (split(/\s+/,$prevts))[0];
&Dbg("Original prevts=$prevts");
&CmdQ("rm -f $prevtmp");
$todaylogdir = "/projects/hnd/swbuild/build_admin/logs/donglesize";
$todaylogdir.= "/$reldir/$brand/$todayts";
if ( ( $reldir =~ /BRANCH|TWIG/ ) && ( ! -d "$todaylogdir" ) ) {
   &Cmd("mkdir -pv $todaylogdir");
}
$prevlogdir  = "$todaylogdir";
$prevlogdir  =~ s/$todayts/$prevts/g;
$previmgstat = "${prevlogdir}/imgstat.log";
$threshold   = "2048";
@today_passed= ();
@today_failed= ();
@today_warned= ();
@prev_passed = ();
@prev_failed = ();
@prev_warned = ();

# Go upto two weeks back to search for previous dongle image status file
if ( ! -s "$previmgstat" ) {
   for ( $prevday=1; $prevday < $prevmaxdays; $prevday++ ) {
       $tmptimewindow  = $timewindow + ( $prevday * 24);
       $tmpprevduration= $tmptimewindow * 60 * 60;
       $tmpprevtmp     = qx(mktemp ${TEMP}/tmpprevts_XXXXXX); chomp($tmpprevtmp);
                         `$touch -B $tmpprevduration $tmpprevtmp`;
       $tmpprevts      = qx(stat -c "%y" $tmpprevtmp); chomp($tmpprevts);
       $tmpprevts      = (split(/\s+/,$tmpprevts))[0];
       $tmpprevlogdir  = $todaylogdir;
       $tmpprevlogdir  =~ s/$todayts/$tmpprevts/g;
       $tmpprevimgstat = "${tmpprevlogdir}/imgstat.log";
       #&Info("ts=$tmpprevts ld=$tmpprevlogdir");
       if ( ! -s "$tmpprevimgstat" ) {
          &Warn("Previous imgstat empty! Searching in old build directories");
          $tmpprevimgstat = "$todayimgstat";
          $tmpprevimgstat =~ s/$todayts/$prevts/g;
       }
       &CmdQ("rm -f $tmpprevtmp");
       if ( -s "$tmpprevimgstat" ) {
          &Info("Found last good imgstat.log $prevday days back on '$tmpprevts'");
          $prevlogdir  = "$tmpprevlogdir";
          $prevts      = "$tmpprevts";
          $previmgstat = "$tmpprevimgstat";
          last;
       } else {
          &Warn("$tmpprevts/imgstat.log not found. Skipping back ${prevday} days");
       }
   };
}

&Info("Symbol/size comparisons to be done with $prevts day");

$todaytscmp  = $todayts; $todaytscmp =~ s/-//g;
$prevtscmp   = $prevts;  $prevtscmp  =~ s/-//g;

# Owners for automatically filed PRs for different branches.
# TODO: If UTF needs these values, then following structure needs
# TODO: to be stored a central place and parsed here. For now they
# TODO: are hardcoded here
# NOTE: if a branch wants to set priority/severity values different
# NOTE: from DEFAULT values, then override their value similar to NIGHTLY
# NOTE: One can override only those values that are diferent from default
%PRDetails = (
   "DEFAULT"              => {
                               pr_gen   => 'yes',
                               owner    => 'prakashd',
                               priority => {
                                               hardlimit => 'high',
                                               softlimit => 'medium',
                                               warnlimit => 'low',
                                             },
                               severity => {
                                               hardlimit => 'critical',
                                               softlimit => 'serious',
                                               warnlimit => 'non-critical',
                                             },
                             },
   "NIGHTLY"              => {
                               # TOT owner
                               owner    => $TOT_OWNER,
                               priority => {
                                               warnlimit => 'low',
                                             },
                               severity => {
                                               warnlimit => 'non-critical',
                                             },
                             },
   "KIRIN_BRANCH_5_100"   => {
                               pr_gen   => qq(no),
                               owner    => 'mzhu',
                             },
   "BASS_BRANCH_5_60"     => {
                               pr_gen   => qq(no),
                               owner    => 'mzhu',
                             },
   "PBR_BRANCH_5_10"      => {
                               owner    => 'mzhu',
                             },
   "F15_BRANCH_5_50"      => {
                               owner    => 'hnd-software-scm-list',
                             },
   "F16_BRANCH_5_80"      => {
                               owner    => 'hnd-software-scm-list',
                             },
   "FALCON_BRANCH_5_90"   => {
                               owner    => 'hnd-software-scm-list',
                             },
   "FALCONPHY_BRANCH_5_92"   => {
                               owner    => 'hnd-software-scm-list',
                             },
   "RAPTOR_BRANCH_4_216"  => {
                               owner    => 'hnd-software-scm-list',
                             },
   "RAPTOR2_BRANCH_4_217" => {
                               owner    => 'hharte',
                             },
   "ROMTERM2_BRANCH_4_219"=> {
                               owner    => 'eichang',
                             },
   "ROMTERM_BRANCH_4_218" => {
                               owner    => 'sridhara',
                             },
   "ROMTERM3_BRANCH_4_220"=> {
                               owner    => 'prakashd',
                             },
   "RAPTOR3_BRANCH_4_230" => {
                               owner    => 'prakashd',
                             },
   "RT2TWIG46_BRANCH_4_221" => {
                               owner    => 'jsiegel',
                             },
   "WH2_TWIG_4_172"       => {
                               owner    => 'jsiegel',
                             },
);

if ( $prevtscmp >= $todaytscmp ) {
   &Error("prevts=$prevts can't be greater than or equal to $todayts");
   &Error("If you specified '-date <report_date>', then ensure that");
   &Error("'-timewindow <hours>' specified referring a day previous to");
   &Error("<report_date>");
   &Exit($ERROR);
}

if ( $report_date ) {
   &Info("Setting  todayts   = $todayts");
   &Info("Computed todayiter = $todayiter");
   &Info("Computed hndrte_dir= $hndrte_dir");
   &Info("Computed prevts    = $prevts");
}

##
## check command line args, set defaults if not specified
##
sub checkCmdLineArgs {

    $CVSROOT   = "/projects/cvsroot" unless ( $CVSROOT );
    $domain    = "broadcom.com";

    if ( $timewindow ) {
       ($timewindow ) = ( $timewindow =~ m/(\d+)/g );
       die "specify -timewindow only in hours" unless ( $timewindow );
    } else {
       $timewindow = 24;
    }

    &Help() if ( $help );

} # checkCmdLineArgs()

##
## genImageInfo ( )
## scan imgstat file and populate imginfo hash/data structure
##
sub genImageInfo {
    my($imgstat) = @_;
    my($image)   = "";
    my(%imginfo) = ();

    $rimginfo = \%imginfo;

    if ( -s "$imgstat" ) {
       open(IMGSTAT,"${imgstat}") || die "Can not open ${imgstat}";
       &Info("Processing $imgstat now");
       while (<IMGSTAT>) {
          chomp();
          next if ( /text\s+data/ );
          next if ( /\s+rtecdc.*/ );
          next if ( /=================|-------------------/ );
          if ( /INFO:\s+Image\s+(.*?)\s+details/ ) {
             $image = "$1";
             $rimginfo->{$image}{status}="PASS";
             next;
          } elsif ( /ERROR:\s+Image\s+(.*?)\s+exceeds\s+limit/ ) {
             $image = "$1";
             $rimginfo->{$image}{status}="FAIL";
             next;
          } elsif ( /WARN:\s+Image\s+(.*?)\s+exceeds\s+warnlimit/ ) {
             $image = "$1";
             $rimginfo->{$image}{status}="WARN";
             next;
          }
          &Dbg("Populating [$image]") if ( $image );
          next unless ( $image );
          if ( /size:\s+(\d+);\s+limit:\s+(\d+);\s+diff:\s+([-]*\d+);/ ) {
             $rimginfo->{$image}{size}      = $1;
             $rimginfo->{$image}{hardlimit} = $2;
             $rimginfo->{$image}{softlimit} = $2 - $threshold;
             $rimginfo->{$image}{diff}      = $3;
             &Dbg("Size : $rimginfo->{$image}{size}; Hard: $rimginfo->{$image}{hardlimit}; Soft: $rimginfo->{$image}{softlimit}; Diff: $rimginfo->{$image}{diff}");
             next;
          } elsif ( /reclaim:\s+(\d+);\s+warnlimit:\s+(\d+);/ ) {
             $rimginfo->{$image}{reclaim}   = $1;
             $rimginfo->{$image}{warnlimit} = $2;
             $rimginfo->{$image}{warnsize}  = $rimginfo->{$image}{size} - $rimginfo->{$image}{reclaim};
             &Dbg("Reclaim: $rimginfo->{$image}{reclaim}; Warn: $rimginfo->{$image}{warnlimit}");
             next;
          }
       } # while <IMGSTAT>
       close(IMGSTAT);
    }
    return($rimginfo);

} # genImageInfo()

##
## createProblemReport ( )
## generate a gnats PR when softlimit or warnlimits are reached
##
sub createProblemReport {
    my($pr_type,$blddir,$image,$todayts,$prevts,$symcmpPR) = @_;

    my($pr_from_user)      = "prakashd";
    my($pr_gnats_address)  = "gnats4-hnd-wlan\@broadcom.com";
    my($pr_notify_list)    = "";
    my($pr_responsible)    = "";
    my($pr_reply_to_user)  = "";
    my($pr_priority)       = "";
    my($pr_severity)       = "";
    my($pr_date)           = `date`; chomp($pr_date);
    my($imgloc,$prfile);

    if ( ! exists $PRDetails{$reldir} ) {
       &Warn3("PR Details for $reldir not found. Using DEFAULT params");
       `echo "PR details missing for $reldir. Update %PRDetails struct" | \
                mail -s 'WARN: $reldir PR details missing. Update %PRDetails' \
                $pr_from_user`;
    }

    # If $reldir specific values are missing use DEFAULT values
    $pr_gen       = ( exists $PRDetails{$reldir}{pr_gen} ) ?
                         $PRDetails{$reldir}{pr_gen} :
                         $PRDetails{DEFAULT}{pr_gen};
    $pr_responsible   = ( exists $PRDetails{$reldir}{owner} ) ?
                         $PRDetails{$reldir}{owner} :
                         $PRDetails{DEFAULT}{owner};
    $pr_reply_to_user = $pr_responsible;
    $pr_priority      = ( exists $PRDetails{$reldir}{priority}{$pr_type} ) ?
                         $PRDetails{$reldir}{priority}{$pr_type} :
                         $PRDetails{DEFAULT}{priority}{$pr_type};
    $pr_severity      = ( exists $PRDetails{$reldir}{severity}{$pr_type} ) ?
                        $PRDetails{$reldir}{severity}{$pr_type} :
                        $PRDetails{DEFAULT}{severity}{$pr_type};

    &CmdQ("mkdir -p ${todaylogdir}/problem_reports");
    &CmdQ("mkdir -p ${todaylogdir}/problem_reports/skipped");

    ($imgname) = $image =~ m%/(.*)$%g;
    $imgloc    = "$image";
    $imgloc    =~ s%/%--%g;
    $prfile    = "${todaylogdir}/problem_reports/${pr_type}--$imgloc";

    $symsizefile = "$todaylogdir/${imgloc}-${todayts}.symsize";
    $symcmpfile  = "$todaylogdir/${imgloc}-${prevts}-to-${todayts}.symcmp";
    $symmapfile  = "$todaylogdir/${imgloc}-${todayts}.maptxt";

    if ( -s "$prfile" ) {
       &Warn(" Previous GNATS problem report exists for image");
       &Warn(" '$image' at '$prfile'");
       &Warn(" Skipping duplicate GNATS PR generation");
       return();
    }

    if ( -s "${prevlogdir}/problem_reports/${pr_type}--$imgloc" ) {
          &Warn(" $pr_type check failed on previous day ($prevts) as well");
          &Warn(" Skipping duplicate PR generation");
          if ( grep {/^$pr_type$/} @gen_prtypes ) {
             `echo "Previous ($prevts) $pr_type PR detected for $reldir $image" | \
                mail -s 'WARN: Skip duplicate PR generation for $reldir $image on $todayts' $pr_from_user $pr_responsible`;
          }
          open(SKIP,">${todaylogdir}/problem_reports/skipped/${pr_type}--$imgloc");
          print SKIP "$pr_type check failed on previous day ($prevts) as well\n";
          print SKIP "Skipping duplicate PR generation\n";
          print SKIP "Previous problem report location:\n";
          print SKIP " ${prevlogdir}/problem_reports/${pr_type}--$imgloc\n";
          close(SKIP);
          return();
    }
    if ($pr_gen =~ /no/i) {
          &Warn(" GNATS PR generation disabled for $reldir");
          return();
    }

    &Info(" Creating GNATS Problem Report on $thishost");
    &Info(" Responsible=$pr_responsible; Priority=$pr_priority; Severity=$pr_severity");
    open(PR,">$prfile") || die "Can't create $prfile";
    select(PR);

    print "Date:        $pr_date\n";
    print "From:        $pr_from_user\n";
    print "Reply-To:    $pr_reply_to_user\n";
    print "To:          $pr_gnats_address\n";
    print "Subject:     $reldir dongle $image $pr_type check failed on $todayts\n";
    print "\n";
    print ">Notify-list:  $pr_notify_list\n";
    print ">Category:     bcm43xx_sw-dongle-SIZE\n";
    print ">Synopsis:     $reldir dongle $image $pr_type check failed on $todayts\n";
    print ">Confidential: no\n";
    print ">Severity:     $pr_severity\n";
    print ">Priority:     $pr_priority\n";
    print ">Responsible:  $pr_responsible\n";
    print ">State:        new\n";
    print ">Class:        sw-bug\n";
    print ">Submitter-Id: ${pr_from_user}\@broadcom.com\n";
    print ">Arrival-Date: $pr_date\n";
    print ">Originator:   $pr_from_user (Prakash Dhavali)\n";
    print ">Organization: Broadcom Corporation\n";
    print ">Release-Found-in: $reldir\n";
    print ">Customer: Broadcom\n";
    print ">Environment:\n";
    print "\n";
    print ">Description:\n\n";
    print "NOTE: This PR is automatically generated on $pr_date\n\n";
    print "NOTE: Image Softlimit is '<hardlimit>-$threshold' bytes\n";
    print "NOTE: Image Warnsize  is '<actual_size>-reclaim'\n";
    print "\n";
    print "Dongle image size check failed:\n\n";
    print "    Image         : $image\n";
    print "    Failure Type  : $pr_type exceeded\n";
    print "    Branch/Tag    : $reldir\n";
    print "    Fail Date     : $todayts\n";
    print "    Compare with  : $prevts\n";

    if ( $pr_type =~ m/warnlimit/gi ) {
       print "    Warn Limit    : $rtodayimginfo->{$image}{warnlimit}\n";
       print "    New Warn Size : $rtodayimginfo->{$image}{warnsize}\n";
       print "    Old Warn Size : $rprevimginfo->{$image}{warnsize}\n";
    } elsif ( $pr_type =~ m/softlimit/gi ) {
       print "    Hard Limit    : $rtodayimginfo->{$image}{hardlimit}\n";
       print "    Actual Size   : $rtodayimginfo->{$image}{size}\n";
       print "    Soft Limit    : $rtodayimginfo->{$image}{softlimit}\n";
    } elsif ( $pr_type =~ m/hardlimit/gi ) {
       print "    Hard Limit    : $rtodayimginfo->{$image}{hardlimit}\n";
       print "    Actual Size   : $rtodayimginfo->{$image}{size}\n";
    }
    print "\n";
    print "    Build Directory : ${urlprefix}$blddir\n";
    print "    Symbol Size Info: ${urlprefix}$symsizefile\n" if ( -f "$symsizefile" );
    print "    Symbol Map  File: ${urlprefix}$symmapfile\n" if ( -f "$symmappfile" );
    print "    Symbol Compare  : ${urlprefix}$symcmpfile\n" if ( -f "$symcmpfile" );
    print "\n";
    if ( -f "${symcmpfile}.notdone" ) {
       print "    WARN: NO symbol comparison available\n";
       print "    Symcmp Status   : ${urlprefix}${symcmpfile}.notdone\n";
    } else {
       print "$symcmpPR\n";
    }
    print "NOTE: Hardlimit is maxsize an image can take, beyond which it won't work\n";
    print "NOTE: Warnlimit is daily delta jump in image size\n";
    print "NOTE: These limits are set by <image>-maxsize and <image>-warnlimit\n";
    print "NOTE: variables in src/dongle/rte/wl/makefiles/<chip> makefile\n";
    print "\n";
    print `cat $todaylogdir/imgstat.log`,"\n";
    print ">How-To-Repeat:\n";
    print " Try to rebuild failed dongle image $image\n";
    print ">Fix:\n";

    close(PR);
    select(STDOUT);

    # As requested by branch owners, we do not generate PRs for all types
    # of violations, although we still track image size for these types
    # By default, we generate only PRs for hardlimit failures, but we
    # still record softlimit and warnlimit size violations in dongle size
    # archive for NDT to report (if needed)
    if ( grep {/^$pr_type$/} @gen_prtypes ) {
       # &Info("Disabled emailing of $prfile to gnats on $thishost");
       # TODO: Currently sendpr is not routing PRs to gnats database
       # TODO: So instead submit via email to gnats db directly
       &Cmd("/usr/lib/sendmail -oi -t < $prfile");
       &Info("Checking mail-queue");
       &CmdQ("mailq");
       # Preserve prfile to ensure that subsequent runs do not create duplicate PRs
       # &Cmd("rm -fv $prfile");
    }

} # createProblemReport ( )

##
## recordDongleSizeInfo ( )
## generate a list of cvs commits
##
sub recordDongleSizeInfo {
    my ($hndrte_dir) = @_;
    my ($prevsymsize,$todaysymsize,$todaysymcmp,$symcmpdone,$prevmaxdays);
    my ($blddir,$symfilename,$mapfile,%seeni);
    my ($tmptimewindow,$prevday,$tmpprevduration,$tmpprevtmp,$tmpprevts,$tmpprevlogdir);
    my ($tmpprevsymsize,$imagenum);

    if ( $USER !~ /$BUILDOWNER/ ) {
       &Warn("USER=$USER is not build owner ($BUILDOWNER)");
       &Warn("Symbol size generation and comparison can not be run");
       return();
    }

    if ( -d "$todaylogdir" ) {
       $symcmps=qx(find $todaylogdir -type f -name "*.symcmp" -print);
       if ( ( $symcmps ) && ( -s "$todaylogdir/imgstat.log" )) {
          if ( ! $forcesymcmp ) {
             &Warn("Previously recorded image sizes exist at:");
             &Warn("$todaylogdir");
             &Warn("New sizes will not be recorded");
             return();
          }
       } else {
          &Info("Previously created $todaylogdir exists. Reusing its contents");
          &Cmd("cp $todayimgstat $todaylogdir");
       }
    } else {
       &Cmd("mkdir -p $todaylogdir");
       &Cmd("cp $todayimgstat $todaylogdir");
    };

    @allimages = sort grep { !$seeni{$_}++ } keys %$rtodayimginfo;

    $imagenum = 1;
    foreach $image ( @allimages ) {

       print "================================================\n";
       &Info("PROCESSING IMAGE[$imagenum] = '$image'");

       $warncmp     = 0;
       $blddir      = "$hndrte_dir/src/dongle/rte/wl/builds/$image";
       $todaysymsize= "${image}-${todayts}.symsize";
       $todaysymsize=~ s%/%--%g;
       $todaymapfile= "${image}-${todayts}.map";
       $todaymapfile=~ s%/%--%g;
       $todaysymcmp = "${image}-${prevts}-to-${todayts}.symcmp";
       $todaysymcmp =~ s%/%--%g;
       $prevsymsize = "${image}-${prevts}.symsize";
       $prevsymsize =~ s%/%--%g;
       $warncmp     = $rtodayimginfo->{$image}{warnsize} -
                      $rprevimginfo->{$image}{warnsize} if
                      (( $rtodayimginfo->{$image}{warnsize} > 0 ) &&
                      ( exists $rprevimginfo->{$image}{warnsize}) &&
                      ( $rprevimginfo->{$image}{warnsize} > 0 ));

       $mapfile = qx(find $blddir -type f -name "rte*.map"); chomp($mapfile);
       if ( ! -f "$mapfile" ) {
          &Warn(" Mapfile not found at $mapfile");
          &Warn(" Skipping symbol info generation");
          next;
       }
       $mapfile_bn = $mapfile;
       $mapfile_bn =~ s%${hndrte_dir}/src/dongle/rte/wl/builds/%%;
       &Info(" Creating $todaysymsize");
       &CmdQ("symsize $mapfile > $todaylogdir/$todaysymsize");
       &CmdQ("cp $mapfile $todaylogdir/${todaymapfile}txt");

       $symcmpdone=0;
       my $symcount = "20";
       my $symcmpPR = "";
       # If build for $image failed on previous day, go back $prevmaxdays days
       if ( -s "$prevlogdir/$prevsymsize" ) {
          &Info(" Creating $todaysymcmp");
          &CmdQ("$SYMCMP -m $prevlogdir/$prevsymsize $todaylogdir/$todaysymsize > $todaylogdir/$todaysymcmp");
          $symcmpPR = `$SYMCMP -limit $symcount -m $prevlogdir/$prevsymsize $todaylogdir/$todaysymsize`;
          $symcmpdone=1;
       } else {
          for ( $prevday=1; $prevday < $prevmaxdays; $prevday++ ) {
              $tmptimewindow  = $timewindow + ( $prevday * 24);
              $tmpprevduration= $tmptimewindow * 60 * 60;
              $tmpprevtmp     = qx(mktemp ${TEMP}/tmpprevts_XXXXXX); chomp($tmpprevtmp);
                                `$touch -B $tmpprevduration $tmpprevtmp`;
              $tmpprevts      = qx(stat -c "%y" $tmpprevtmp); chomp($tmpprevts);
              $tmpprevts      = (split(/\s+/,$tmpprevts))[0];
              $tmpprevlogdir  =~ s/$todayts/$tmpprevts/g;
              $tmpprevsymsize = "${image}-${tmpprevts}.symsize";
              $tmpprevsymsize =~ s%/%--%g;
              &CmdQ("rm -f $tmpprevtmp");
              if ( -s "$tmpprevlogdir/$tmpprevsymsize" ) {
                 &Info(" Scaling back $prevday days to fetch previous symsize");
                 &CmdQ("$SYMCMP -m $tmpprevlogdir/$tmpprevsymsize $todaylogdir/$todaysymsize > $todaylogdir/$todaysymcmp");
                 $symcmpPR = `$SYMCMP -limit $symcount -m $tmpprevlogdir/$tmpprevsymsize $todaylogdir/$todaysymsize`;
                 $symcmpdone=1;
                 last;
              }
          }
          if ( ! $symcmpdone ) {
             &Warn(" Previous '$prevsymsize' not found for symcmp");
             &CmdQ(" Creating ${todaysymcmp}.notdone");
             &CmdQ(" echo '$prevsymsize NOT found for symcmp' > $todaylogdir/${todaysymcmp}.notdone");
          }
       }

       # Check if hardlimit was exceeded
       &Dbg(" Checking if hardlimit was exceeded");
       if ( grep {/^$image$/} @today_failed ) {
          &Error(" Hardlimit exceeded");
          &Error(" Hardlimit=$rtodayimginfo->{$image}{hardlimit}; size=$rtodayimginfo->{$image}{size}");
          #disabled# &createProblemReport("hardlimit",$blddir,$image,$todayts,$prevts,$symcmpPR);
          next; # Don't generate soft/warnlimit alerts when hardlimit exceeds
       } else {
          &Dbg(" Hardlimit did not reach");
       }

       # Check if warnlimit was exceeded
       &Dbg(" Checking if warnlimit was exceeded");
      if ( $warncmp > $rtodayimginfo->{$image}{warnlimit} ) {
          if ( $rtodayimginfo->{$image}{status} =~ /FAIL/ ) {
            &Warn(" Skipping softlimit alert as hardlimit already exceeded");
          } else {
            &Warn(" Warnlimit exceeded");
            &Warn(" Warnlimit=$rtodayimginfo->{$image}{warnlimit}; warncmp=$warncmp");
            #disabled# &createProblemReport("warnlimit",$blddir,$image,$todayts,$prevts,$symcmpPR);
          }
       } else {
          &Dbg(" Warnlimit did not reach");
       }

       # Check if softlimit was exceeded
       &Dbg(" Checking if softlimit was exceeded");
       if ( $rtodayimginfo->{$image}{size} > $rtodayimginfo->{$image}{softlimit} ) {
          if ( $rtodayimginfo->{$image}{status} =~ /FAIL/ ) {
             &Warn(" Skipping softlimit alert as hardlimit already exceeded");
          } else {
             &Error(" Softlimit exceeded");
             &Error(" Softlimit=$rtodayimginfo->{$image}{softlimit}; size=$rtodayimginfo->{$image}{size}");
             #disabled# &createProblemReport("softlimit",$blddir,$image,$todayts,$prevts,$symcmpPR);
          }
       } else {
          &Dbg(" Softlimit did not reach");
       }
       $imagenum++;
    }
    print "================================================\n";
    qx(unix2dos $todaylogdir/* > $NULL 2>&1);

} # recordDongleSizeInfo()

##
## getCommits ( )
## generate a list of cvs commits
## TO-DO: This needs to be converted to SVN (Manoj working on this)
##
sub getCommits {
    my($user,$date,$time,$ts,$file,$version);

    if (!$tag || ($tag =~ /trunk/i)) {
       $svnurl="$SVNROOT/trunk";
    } elsif ($tag =~ /_REL_/) {
       my $tagprefix=(split(/_/,$tag))[0];
       $svnurl="$SVNROOT/tags/$tagprefix/$tag";
    } elsif ($tag =~ /_BRANCH_|_TWIG_/) {
       $svnurl="$SVNROOT/branches/$tag";
    }
    # Time lapse in seconds for requested number of days
    $lastseconds = $timewindow * 60 * 60;

    # Local timezone
    $tz  = qx(date '+%z'); chomp($tz);

    $nowtime = strftime("%Y-%m-%d %H:%M:%S",localtime(time));
    $nowtime = "$nowtime $tz";

    $prevtime = strftime("%Y-%m-%d %H:%M:%S",localtime(time()-$lastseconds));
    $prevtime = "$prevtime $tz";

    #New Sample Command# my $svnlogcmd = "svn-commit-info.pl -url $svnurl -start 2011-05-24 -end 2011-05-23 -module mod1 -module mod2 ....";

    #old one# my $svnlogcmd = "$svncmd log -q -r{'$nowtime'}:{'$prevtime'} $svnurl";

    &Info("Running '$svnlogcmd'");
    open(COMMITS, "$svnlogcmd |") || die "svn log error!!";

    while ( <COMMITS> ) {
       chomp();
       # If cvsreport for TOT returns any branch results, skip them and vice-a-versa
       next if /^$/;
       next if /^\s+$/;
       # TO-DO: Create %commits hash with $commits{$user} for each file
       if ( /^\s+src\s+(\S+[^ ]*)(.*)/ ) {
          $file    = $1;
          # version is SVN revision
          $version = $2; $version =~ s/\s+//g;
          next if ( $file !~ m/$mod_regexp/g );
          next if ( $file !~ m/\.c|\.cpp|\.cc|\.h/gi );
          if ( ! grep { /$file/ } @{$commits{$user}} ) {
             &Dbg("Found user=$user; file=$file; version=$version");
             my $fileinfo  = sprintf("%-30.30s (%-16.16s v%s)",$file,$ts,$version);
             push (@{$commits{$user}}, "$fileinfo");
          } else {
             &Dbg("Skipping $file");
          }
       }
    }
    # Uncomment following for loop after debugging
    foreach $user ( sort keys %commits ) {
       foreach $file ( sort @{$commits{$user}} ) {
          printf "%3.3s %10.10s %s\n", "$listno", "$user", "  $file";
          $listno++;
       }
    }

    return(\%commits);

} # getCommits()

##
## genMsg ( )
##
sub genMsg {
    my ($htmlfile, $textfile, $rcommits, $hndrte_dir) = @_;
    my (%commits);
    my ($failedimage);

    $days = int($timewindow/24);

    open(HTMLFILE, ">${htmlfile}") || die "Can't open ${htmlfile}!";
    %commits = %${rcommits};

    print HTMLFILE "<html xml:lang=\"en\" lang=\"en\">
    <head>
     <title> Checkins potentially breaking dongle image sizes in $reldir</title>
     <meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\" />
    </head>
    <body bgcolor=\"#ffffff\">\n";

    print HTMLFILE "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\"> \n";
    print HTMLFILE "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b><u>Checkins potentially breaking dongle image size in $reldir</u></b></td></tr>\n";
    foreach $failedimage ( @today_failed ) {
      print HTMLFILE "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b><u>Failed Image: $failedimage</u></b></td></tr>\n";
    }
    print HTMLFILE "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b><u>Images Details: ${urlprefix}/$todaylogdir/imgstat.log</u></b2></td></tr>\n";

    print HTMLFILE "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b2><u>Please ensure your recent checkins do not break dongle build</u></b2></td></tr>\n";
    print HTMLFILE "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b2><u>Be sure your change is configured out correctly, so it doesn't take up space in the dongle if its not used in the dongle.</u></b2></td></tr>\n";
    print HTMLFILE "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b2><u>Relevant CVS commits in last $days days ($timewindow hours) on ${reldir}</u></b2></td></tr>\n";
    print HTMLFILE "</table>\n";
    print HTMLFILE "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\">\n";

    print HTMLFILE "<b><tr style=\"font-size: 10pt; background-color: black; color: #ffffff\">\n";
    print HTMLFILE "<td style=\"font-size: 12pt\">#</td>\n";
    print HTMLFILE "<td>AUTHOR</td>\n";
    print HTMLFILE "<td style=\"font-size: 12pt\">FILE NAME (date and revision)</td>\n";
    print HTMLFILE "</tr></b>\n";

    $listno = 1; $rowcolor = 0;
    foreach $user ( sort keys %commits ) {
       foreach $file ( sort @{$commits{$user}} ) {
          $rowcolor = $rowcolor ? "background-color\: #FFFFCC" : "";
          print HTMLFILE "<tr style=\"$rowcolor; font-size: 10pt\">\n";
          print HTMLFILE "<td style=\"font-size: 12pt\">${listno}</td>\n";
          print HTMLFILE "<td>$user</td>\n";
          print HTMLFILE "<td style=\"font-size: 12pt\">$file</td>\n";
          print HTMLFILE "</tr>\n";
          $rowcolor = $rowcolor ? "" : "background-color\: #FFFFCC";
          $listno++;
       }
    }
    print HTMLFILE "</table>\n";
    print HTMLFILE "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\">\n";
    print HTMLFILE "<tr></tr>\n";
    print HTMLFILE "<tr><td><b>Additional Information:</b></td></tr>\n";
    print HTMLFILE "<tr><td><b>Symbol Info</b>: ${urlprefix}/$todaylogdir/</td></tr>\n";
    print HTMLFILE "<tr><td><b>CVS Report</b>: ${urlprefix}/projects/hnd/software/cvsreport/</td></tr>\n";
    print HTMLFILE "<tr><td><b>Dongle Report:</b> ${urlprefix}/projects/hnd_svt/IntraWeb/NIGHTLY/NDT/ndt_debug.html</td></tr>\n";
    print HTMLFILE "<tr><td><b>Build Log:</b> ${urlprefix}${hndrte_dir}/,release.log</td></tr>\n";
    print HTMLFILE "</table>\n";
    print HTMLFILE "</body>\n</html>\n";
    close(HTMLFILE);
    # End of html report

    # Start of text file
    open(TXT_FILE, ">${textfile}") || die "Can't open ${textfile}!";
    print TXT_FILE "\nCheckins potentially breaking dongle image size in $reldir.\n";
    print TXT_FILE "This report generated at ${nowtimestamp}.\n\n";
    if ( -s "${todayimgstat}" ) {
       open(IMGSTAT,"${todayimgstat}") || die "Can't open $todayimgstat";
       while (<IMGSTAT>) { print TXT_FILE "$_" if (/ERROR: /); }
       close(IMGSTAT);
    }
    print TXT_FILE "\n";
    print TXT_FILE "Please ensure your recent checkins do not break dongle build.\n\n";
    print TXT_FILE "Be sure your change is configured out correctly, so it \n";
    print TXT_FILE "doesn't take up space in the dongle if its not used in \n";
    print TXT_FILE "the dongle.\n\n";
    print TXT_FILE "Relevant CVS commits in last $days days (or $timewindow hours) on ${reldir}\n";
    print TXT_FILE "\n";
    print TXT_FILE "-" x "75", "\n";
    printf TXT_FILE "%3.3s %10.10s %s\n", "\#", "AUTHOR", "  FILE NAME (date and revision)";
    print TXT_FILE "-" x "75", "\n";

    $listno = 1; $rowcolor = 0;
    foreach $user ( sort keys %commits ) {
       foreach $file ( sort @{$commits{$user}} ) {
          printf TXT_FILE "%3.3s %10.10s %s\n", "$listno", "$user", "  $file";
          $listno++;
       }
    }

    print TXT_FILE "\nAdditional Information:\n";
    print TXT_FILE "Symbol Info : ${urlprefix}/$todaylogdir\n";
    print TXT_FILE "CVS Report: ${urlprefix}/projects/hnd/software/cvsreport/\n";
    print TXT_FILE "Dongle Report: ${urlprefix}/projects/hnd_svt/IntraWeb/NIGHTLY/NDT/ndt_debug.html\n";
    print TXT_FILE "Build Log: ${urlprefix}${hndrte_dir}/,release.log\n";
    if ( -s "${todayimgstat}" ) {
       open(IMGSTAT,"${todayimgstat}") || die "Can't open $todayimgstat";
       while (<IMGSTAT>) { print TXT_FILE "$_"; }
       close(IMGSTAT);
    }
    print TXT_FILE "\n";
    close(TXT_FILE);
    # End of text report

} # genMsg()

##
## sendMsg ( )
##
sub sendMsg {
    my ($htmlfile, $textfile, $hndrte_dir, $rcommits) = @_;
    my (@tousers, %commits);

    %commits = %${rcommits};

    @cvsusers  = map { $_ =~ m/\@$domain/gi ? "$_" : "$_\@$domain" } sort grep { !$seenu{$_}++ } keys %commits;
    @ccusers   = map { $_ =~ m/\@$domain/gi ? "$_" : "-c $_\@$domain" } sort @ccusers;
    @dbgusers  = map { $_ =~ m/\@$domain/gi ? "$_" : "$_\@$domain" } sort @dbgusers;

    @tousers = $dbg ? @dbgusers : @cvsusers;
    @ccusers = $dbg ? @dbgusers : @ccusers;

    &Info("Email To  : @tousers") if ( @tousers );
    &Info("Email Cc  : @ccusers") if ( @ccusers );
    &Info("Email Body: $textfile") if ( -f ${textfile} );
    &Info("Email Host: $thishost");

    system("mail -s '$reldir ${brand} SIZE CHECK FAILED!. Please check your CVS commits' @ccusers @tousers < ${textfile}") if ( @tousers );
    if ( $USER =~ /$BUILDOWNER/ ) {
       &Cmd("cp $htmlfile $hndrte_dir/,${brand}_sizecheck_commits.htm");
       &Cmd("rm -f $htmlfile");
    } else {
       &Warn("USER=$USER is not build owner ($BUILDOWNER)");
       &Warn("$htmlfile not copied to $hndrte_dir");
       #&Cmd("rm -f $htmlfile");
    }

} # sendMsg()


## showImageInfo
sub showImageInfo {
    my($ts, $rimginfo) = @_;

    foreach $img ( sort keys %{$rimginfo} ) {
      &Dbg("[$ts] IMAGE = $img");
      foreach $size ( sort keys %{$rimginfo->{$img}}  ) {
        &Dbg("[$ts] $size = $rimginfo->{$img}{$size}");
      }
      &Dbg("----------------------------");
    }

} # sub showImageInfo()

## Main
sub Main {
    my($status, $img);

    &Info3("Running Dongle Image Size check on $thishost at $nowtimestamp");

    &checkCmdLineArgs();

    if ( ! -d ${hndrte_dir} ) {
       &Error3("hndrte_dir = $hndrte_dir not found");
       &Exit($ERROR);
    }

    &Info("Checking for existance of '$todayimgstat' and '$previmgstat'");

    if ( -s "$todayimgstat" && -s "$previmgstat" ) {

       ($rtodayimginfo) =  &genImageInfo($todayimgstat);
       foreach $img ( sort keys %{$rtodayimginfo} ) {
          if ( exists $rtodayimginfo->{$img}{status} ) {
             $status = $rtodayimginfo->{$img}{status};
             push(@today_passed,$img) if ( $status =~ m/pass/i );
             push(@today_failed,$img) if ( $status =~ m/fail/i );
             push(@today_warned,$img) if ( $status =~ m/warn/i );
          }
       }

       &showImageInfo($todayts, $rtodayimginfo);

       ($rprevimginfo)   =  &genImageInfo($previmgstat);

       foreach $img ( sort keys %{$rprevimginfo} ) {
          if ( exists $rprevimginfo->{$img}{status} ) {
             $status = $rprevimginfo->{$img}{status};
             push(@prev_passed,$img) if ( $status =~ m/pass/i );
             push(@prev_failed,$img) if ( $status =~ m/fail/i );
             push(@prev_warned,$img) if ( $status =~ m/warn/i );
          }
       }

       &showImageInfo($prevts, $rprevimginfo);

       # Record information on size bloat (if any) to a central location

       &recordDongleSizeInfo($hndrte_dir);

    } else {

       my $todayimgstaturl="$urlprefix/$todayimgstat";
       my $previmgstaturl ="$urlprefix/$previmgstat";
       my $adminmsg = qx(mktemp ${TEMP}/adminmsg_${brand}_XXXXXX);
	
       chomp($adminmsg);

       &Error("Following imgstat.log files are empty");
       &Error("- today    $todayimgstat") if ( ! -s "$todayimgstat" );
       &Error("- previous $previmgstat")  if ( ! -s "$previmgstat" );
       # If the build passed and if imgstat.log file is empty, alert admins
       if ( -f "$hndrte_dir/,succeeded" ) {
          open(MSG,">$adminmsg");
          print MSG "ERROR: Empty $todayimgstaturl\n" if ( ! -s "$todayimgstat" );
          print MSG "ERROR: Empty $previmgstaturl\n\n" if ( ! -s "$previmgstat" );
          print MSG "FULL  LOG: $urlprefix/$hndrte_dir/,release.log\n";
          close(MSG);
          system("unix2dos $adminmsg");
          system("cat $adminmsg 2>&1 | mail -s 'ERROR: $reldir empty dongle imgstat.log found' @dbgusers");;
          system("sleep 2; rm -fv $adminmsg");
       }
    }

    if ( $report_date || $skip_cvs_query ) {
       &Warn("Skipping cvs query");
       &Exit($OK);
    }

    if ( @today_failed ) {
       &Dbg("failedimages = @today_failed");
       &Warn3("HNDRTE image size check failed. Querying for cvs commits");
    } else {
       &Info3("HNDRTE image size check did not fail or $todayimgstat missing");
       &Exit($OK);
    }

    # Get cvs commits that might have caused size bloat
    # TODO: getCommits can be computed before dongle image size checking
    # TODO: done and the relevant users who checked in that day can be
    # TODO: copied as "CC users" to automatically generated PRs.
    # TODO: Only caveat is getCommits() speed. It takes few minutes to complete
    #disabled# $rcommits = &getCommits();

    @modflags = map { "--module $_" } @srcmodules;
    system("perl svn-commit-info.pl @modflags");

    $htmlfile  = qx(mktemp ${TEMP}/msg_${brand}_XXXXXX); chomp($htmlfile);
    $textfile  = qx(mktemp ${TEMP}/msg_${brand}_XXXXXX); chomp($textfile);
    $htmlfile .= ".htm";
    $textfile .= ".txt";

    # Generate a .txt and .htm message. .txt is emailed out and .htm is
    # archived in hndrte_dir
    #disabled# &genMsg($htmlfile, $textfile, $rcommits, $hndrte_dir);
    #disabled# &sendMsg($htmlfile, $textfile, $hndrte_dir, $rcommits);

} # Main()

##
## Help ( )
##
sub Help {

print "

Usage: $0 [-d/bg] [-h/elp] [-branch <branch>] [-brand <brand>] [-timewindow <timewindow>] [-brand_dir <brand_iteration_dir>] [-date <report_date>]

   This script scans the hndrte-dongle-wl  release.log file to check if
   dongle size test failed. If so, then it queries cvs to get
   list of potential causes.

   Command line arguments supported by this program

   -branch     : Use this if any TOB image fails size check (default: TOT)
   -brand      : Check for failed builds for this brand (default: hndrte-dongle-wl)
                 (other brands, like locator etc., can be used. But not
                 tested still)
   -brand_dir  : Run image size against this brand_iteration_dir instead of
                 default hndrte_dir '$hndrte_dir'
   -ccuser     : List of users who will be notified on hardlimit violations
               : with a list of cvs checked in files
   -date       : Generate imagestat report for some other day, than today
               : You need to adjust -timewindow <hours> to point to previous
               : day
   -dbg        : Enable debug mode
   -forcesymcmp: Force generation or overwriting of dongle symbol comparisons
   -gen_prtypes: Create gnats PRs for these dongle size violation types
               : Supply either 'softlimit' or 'warnlimit' or both
   -help       : Show this help screen
   -timewindow : Specify a time-window of cvs checkins in hours (default: 24hrs)
   -skip_cvs_query: Turn of cvs query to get a list of possible offenders

   Example:
   1) To generate dongle sizecheck report for TOT
      $0 -c hnd-build-list
   2) To generate dongle sizecheck report for RAPTOR_BRANCH_4_216
      $0 -c hnd-build-list -branch RAPTOR_BRANCH_4_216
   3) To generate dongle sizecheck report for TOT to a previous day than today
      (if today is 2008-06-08 and you want report for 2008-06-06)
      $0 -c hnd-build-list -date 2008-06-06 -timewindow 72

";

    &Exit($OK);

} # Help()

## Start of program
&Main();
