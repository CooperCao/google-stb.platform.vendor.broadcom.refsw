#!/usr/bin/perl

## #####################################################################
##
## Search production build storage location and identify which needs
## to be purged and moved to RO area and generate build aging report
## for pruning/moving purposes.
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## $Id$
##
## #####################################################################

$SCRIPT_VERSION='$Id$';

use     Env;
use     Getopt::Long;

## Command line arguments supported by this program
##
## released_build_list           :   List of builds to keep (see help section for file format)
## release_keep        :   Age in days to keep released builds
## intermediate_keep   :   Age in days to keep intermediate/engg builds
## report_dir          :   Filename to keep the generated reports
## show_disk_usage     :   Not recommended to be used, shows disk consumption per build

%scan_args = (
                'keep_build_list=s'                  => \$released_build_list,
                'released_build_list=s'              => \$released_build_list,
                'intermediate_build_keep_duration=s' => \$intermediate_keep,
                'preserve_program=s'                 => \@keep_programs,
                'release_build_keep_duration=s'      => \$release_keep,
                'report_dir=s'                       => \$report_dir,
                'show_disk_usage'                    => \$show_disk_usage,
                'help'                               => \$help,
             );

# Scan the command line arguments
GetOptions(%scan_args) or &Help;

$uname = `uname -s`; chomp($uname);
if ( $uname !~ m/Linux/gi ) {
   print "ERROR: This script $0 runs only on Linux (xlinux) systems\n";
   exit(1);
}

# hash containing released build information
%released   = ();
# hash containing all physical properties of all builds in SWBUILDDIR
%buildprops = ();
# Array containing builds/files/dir whose age can not be determined
@misc       = ();
# Missing builds (which may be wronly entered in Twiki
@missing    = ();
# Instances (count) of builds found in various locations.
%instances  = ();
# supported platforms
@platforms  = qw(build_linux build_macos build_netbsd build_window);
# skipped build dirs
$skip_bld_re= '/PRESERVED|/TEMP|/ARCHIVED|/SORTED|/BUILD_|/LOGS|/SCRATCH|/RESTORE|/CUSTOM|/\.logs|/sw_archive|/PREBUILD_DONGLE|/DOSTOOL|/BUILD_DELL_';
# nightly/tob builds to skip
$skip_tob_re= '/NIGHTLY|_BRANCH_|_TWIG_';

# get current time
`touch /tmp/.$$.temp`; $currenttime = (stat("/tmp/.$$.temp"))[9]; `rm /tmp/.$$.temp`;
$today              = `date '+%Y.%m.%d'`; chomp($today);
$NULL               = "/dev/null";
$SWBUILDDIR         = "/projects/hnd/swbuild";
$ROBUILDDIR         = "/projects/hnd/sw_archive_ro/swbuild"; ## Currently not used
$ERRORS             = "report_errors_${today}.log";

print "Programs to preserve = @keep_programs\n" if ( @keep_programs );

if ( $release_keep && $release_keep =~ m/\d+/ ) {
   $RELEASE_BUILD_KEEP = $release_keep;
} else {
   $RELEASE_BUILD_KEEP = 180;
   print "Setting default release build life/keep: $RELEASE_BUILD_KEEP\n";
}

if ( $intermediate_keep && $intermediate_keep =~ m/\d+/ ) {
   $INTERMEDIATE_BUILD_KEEP = $intermediate_keep;
} else {
   $INTERMEDIATE_BUILD_KEEP = 45;
   print "Setting default intermediate build life/keep: $INTERMEDIATE_BUILD_KEEP\n";
}

if ( $report_dir && -d $report_dir ) {
   $REPORTDIR = $report_dir;
} else {
   $REPORTDIR = "/tmp";
   print "Setting default reportdir to: $REPORTDIR\n";
}

if ( $show_disk_usage ) {
   print "\n\nWARN: -show_disk_usage option is very very slow!!\n\n";
}

print "\n";
&Help if ( $help );
&Help if ( ! $released_build_list );

$DELETE_ACTION_STRING   = "DELETE_after_partial_preserve";
$RECENT_ACTION_STRING   = "Recent_KEEP_${INTERMEDIATE_BUILD_KEEP}_days";
$RELEASED_ACTION_STRING = "Released_KEEP_${RELEASE_BUILD_KEEP}_days";
$ARCHIVE_ACTION_STRING  = "Released_ARCHIVE_to_readonly";

##
## Help
##
sub Help
{
   print "\n\nUsage:\n";
   print " $0\n";
   print "    -k,   -released_build_list  <input file with releases to keep. See below>\n";
   print "    -rel, -release_build_keep_duration \n";
   print "                           [no_of_days to keep release builds]\n";
   print "                           default is $RELEASE_BUILD_KEEP days\n";
   print "    -i,   -intermediate_build_keep_duration\n";
   print "                           [no_of_days to keep intermediate builds]\n";
   print "                           default is $INTERMEDIATE_BUILD_KEEP days\n";
   print "    -rep, -report_dir      [dir to store generated reports]\n";
   print "    -s,   -show_disk_usage [Display disk-usge per build (very very slow)]\n";
   print "    -h,   -help\n\n";
   print "    All arguments except '-k' are optional. Default values are as specified in twiki\n";
   print "        http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/DiskSpacePolicy\n\n";
   print "Input (released_build_list) file format (example):\n";
   print "# Warriors\n";
   print "build_linux:    D11_REL_3_11_RC48\n";
   print "build_window:   D11_REL_3_11_RC50\n";
   print "\n\n";

   exit(0);

} # Help()

##
## Identify action for released builds based on how old are released builds
##
sub rlsBuildMove
{
   my($platform,$build) = @_;
   my($keep_period)     = $RELEASE_BUILD_KEEP;
   my($action, $age, $creation, $brand, @brands, @sorted_brands, %mtimes);

   #print "Checking existance: ${SWBUILDDIR}/$platform/$build\n";
   if (( -d "${SWBUILDDIR}/$platform/$build" ) || ( -l "${SWBUILDDIR}/$platform/$build" )) {

     chdir("${SWBUILDDIR}/$platform/$build");
     @brands = qx(find * -maxdepth 1 -mindepth 1 -type d);
     chomp(@brands);
     %mtimes=();
     foreach $brand ( @brands ) {
        $mtimes{$brand} = (stat("$brand/,release.log"))[9] if ( -s "$brand/,release.log" );
     }

     @sorted_brands = sort { $mtimes{$b} <=> $mtimes{$a} } keys %mtimes;
     $latestbrand   = shift(@sorted_brands);
     $rloglatest    = "$latestbrand/,release.log";
     #$pwd = qx(pwd); chomp($pwd);
     #print "release rloglatest  = $pwd/$rloglatest\n";
     if ( -s "$rloglatest" ) {
        $mtime = (stat($rloglatest))[9];
        @creation = localtime($mtime);
        $creation = sprintf("%d-%.2d-%.2d", $creation[5]+1900, $creation[4]+1, $creation[3]);
        $age = $currenttime - $mtime;
        $age = int(${age}/(24*60*60));
     } else {
        print "WARN: [rls] Missing $platform/$build/$rloglatest\n";
        print ERRORS "${SWBUILDDIR}/$platform/$build\n";
        push(@misc, "$platform/$build");
     }
     $buildprops{$platform}{$build}{age}      = $age;
     $buildprops{$platform}{$build}{creation} = $creation;
     $buildprops{$platform}{$build}{type}     = "* Released";

     $action = ( $age > $keep_period ) ? "$ARCHIVE_ACTION_STRING" : "$RELEASED_ACTION_STRING";
     $buildprops{$platform}{$build}{action}  = ( $instances{$build} > 1 ) ? "? ASK ?" : "$action";
    #$buildprops{$platform}{$build}{action} .= $action;

   } else {

     $buildprops{$platform}{$build}{age}      = 0;
     $buildprops{$platform}{$build}{creation} = "0.0.0";
     $buildprops{$platform}{$build}{type}     = "* MISSING_RELEASED";
     $action = ( $age > $keep_period ) ? "$ARCHIVE_ACTION_STRING" : "$RELEASED_ACTION_STRING";
     $buildprops{$platform}{$build}{action}  = ( $instances{$build} > 1 ) ? "? ASK ?" : "$action";
     print "WARN: [rls] Missing $platform/$build/$rloglatest\n";
     print ERRORS "${SWBUILDDIR}/$platform/$build\n";
     push(@misc, "$platform/$build");

   }

   if ( $show_disk_usage ) {
        $buildprops{$platform}{$build}{usage} = qx(du -sm . | awk '{print $1}');
   }

} # rlsBuildMove()

##
## Identify action for intermediate builds
##
sub intermediateBuildDelete
{
   my($platform,$build, $preserve) = @_;
   my($keep_period)     = $INTERMEDIATE_BUILD_KEEP;
   my($action, $age, $creation, $brand, @brands);

   #print "Checking existance: ${SWBUILDDIR}/$platform/$build\n";
   if (( -d "${SWBUILDDIR}/$platform/$build" ) || ( -l "${SWBUILDDIR}/$platform/$build" )) {

     chdir("${SWBUILDDIR}/$platform/$build");
     @brands = qx(find * -maxdepth 1 -mindepth 1 -type d);
     chomp(@brands);
     %mtimes=();
     foreach $brand ( @brands ) {
        $mtimes{$brand} = (stat("$brand/,release.log"))[9] if ( -s "$brand/,release.log" );
     }

     @sorted_brands = sort { $mtimes{$b} <=> $mtimes{$a} } keys %mtimes;
     $latestbrand   = shift(@sorted_brands);
     $rloglatest    = "$latestbrand/,release.log";
     #$pwd = qx(pwd); chomp($pwd);
     #print "intermediate rloglatest  = $pwd/$rloglatest\n";
     if ( -s "$rloglatest" ) {
        $mtime = (stat($rloglatest))[9];
        @creation = localtime($mtime);
        $creation = sprintf("%d-%.2d-%.2d", $creation[5]+1900, $creation[4]+1, $creation[3]);
        $age = $currenttime - $mtime;
        $age = int(${age}/(24*60*60));
     } else {
        print "WARN: [tag] Missing ,release.log $platform/$build/$rloglatest!\n";
        print ERRORS "${SWBUILDDIR}/$platform/$build\n";
        push(@misc, "$platform/$build");
     }
     $buildprops{$platform}{$build}{age}      = $age;
     $buildprops{$platform}{$build}{creation} = $creation;
     $buildprops{$platform}{$build}{type}     = "Intermediate";

     $action = ( $age > $keep_period ) ? "$DELETE_ACTION_STRING" : "$RECENT_ACTION_STRING";
     print "Marking $platform/$build to PRESERVE\n" if ( $preserve );
     $action = $preserve if ( $preserve );
     $buildprops{$platform}{$build}{action}  = ( $instances{$build} > 1 ) ? "? ASK ?" : "$action";
    #$buildprops{$platform}{$build}{action} .= $action;

   } else {

     $buildprops{$platform}{$build}{age}      = 0;
     $buildprops{$platform}{$build}{creation} = "0.0.0";
     $buildprops{$platform}{$build}{type}     = "* MISSING_INTERMEDIATE";
     $action = ( $age > $keep_period ) ? "$ARCHIVE_ACTION_STRING" : "$RECENT_ACTION_STRING";
     $buildprops{$platform}{$build}{action}  = ( $instances{$build} > 1 ) ? "? ASK ?" : "$action";
     print "WARN: [rls] Missing $platform/$build/$rloglatest\n";
     print ERRORS "${SWBUILDDIR}/$platform/$build\n";
     push(@misc, "$platform/$build");

   }

   if ( $show_disk_usage ) {
        $buildprops{$platform}{$build}{usage} = qx(du -sm . | awk '{print $1}');
   }

} # intermediateBuildDelete()

##
## Scan for released build locations from SoftwareRelease twiki page information
## Identify any duplicate locations across build_<platform> in production
## and read-only archive area
##
sub scanReleasedBuilds {
    my(@all_builds) = ();

    my($instanceslog) = "${REPORTDIR}/build_instances_${today}.txt";
    system("rm -f $instanceslog");
    foreach $pf ( @platforms ) { push(@all_builds, @{$released{pf}}) }
    foreach $build ( @all_builds ) {
       $instances{$build}  = 0;
       foreach $pf ( @platforms ) {
          $instances{$build} += 1 if ( -d "${SWBUILDDIR}/$pf/$build" );
       }
       push (@missing, $build) if ( ! $instances{$build} );
       if ( $instances{$build} > 1 ) {
          system("ls -1d ${SWBUILDDIR}/build_*/$build >> $instanceslog 2> /dev/null");
          system("ls -1d ${ROBUILDDIR}/build_*/$build >> $instanceslog 2> /dev/null");
          system("echo '' >> $instanceslog");
       }
    }

} # scanReleasedBuilds()

##
## Isolate released and intermediate builds from SoftwareRelease twiki page information
##
sub scanBuilds {
   my ($platform) = shift;
   my ($build, $builddir);

   print "Scanning $platform for released builds now\n";
   chdir($SWBUILDDIR);
   foreach $build ( @all_builds ) {
     $build =~ s/\s+//g;
     if ( $build =~ /$skip_tob_re/ ) {
        push(@misc, "$build"); next;
     }

     $buildpfx = (split(/_/,$build))[0];
     $builddir = "$SWBUILDDIR/$platform/$build";
     if ( -d "${builddir}" ) {
        if ( $builddir =~ m%$skip_bld_re%g ) {
           push(@misc,"$build"); next;
        }
        if ( grep { /^${build}$/i } @all_releases ) {
           &rlsBuildMove($platform,$build)
        } elsif ( grep { /${buildpfx}/i } @keep_programs ) {
           &intermediateBuildDelete($platform,$build,"PRESERVE")
        } else {
           &intermediateBuildDelete($platform,$build)
        }
     }
   }

} # scanBuilds()

##
## Generate a output report (in excel readable format)
##
sub genReport
{
    my($item, $platform, $output, $build);
    my($deleteout, $archiveout, $recentout, $releasedout);

    print "\n";
    system("mkdir -pv ${REPORTDIR}/delete");
    system("mkdir -pv ${REPORTDIR}/archive");
    system("mkdir -pv ${REPORTDIR}/recent");
    system("mkdir -pv ${REPORTDIR}/released");

    foreach $platform ( keys %buildprops ) {
       print "Generating Disk-Cleanup Report for : $platform\n";

       $output      = "${REPORTDIR}/${platform}_cleanup_${today}.csv";
       $deleteout   = "${REPORTDIR}/delete/${platform}_partial_preserve_${today}.txt";
       $archiveout  = "${REPORTDIR}/archive/${platform}_archive_${today}.txt";
       $recentout   = "${REPORTDIR}/recent/${platform}_recent_builds_${today}.txt";
       $releasedout = "${REPORTDIR}/released/${platform}_released_builds_${today}.txt";
       open(OUTPUT,     ">$output")      || die "can't open output: $output!!";
       open(DELETEOUT,  ">$deleteout")   || die "can't open output: $deleteout!!";
       open(ARCHIVEOUT, ">$archiveout")  || die "can't open output: $archiveout!!";
       open(RECENTOUT,  ">$recentout")   || die "can't open output: $recentout!!";
       open(RELEASEDOUT,">$releasedout") || die "can't open output: $releasedout!!";
       print OUTPUT uc($platform),", CLEANUP ACTIONS, (keep days are from build creation date)\n";
       print OUTPUT "Overall Policy:,http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/DiskSpacePolicy\n";
       print OUTPUT "Partial Policy:,http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/HwnBuildPreserveBits\n";
       print OUTPUT "ReleasedBuilds:,http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/HndSwReleases#Released_Builds\n";
       print OUTPUT "Date:, $today, (automatically generated report)\n";
       # print OUTPUT "VERIFY_/MULTIPLE/ASK prefixes, Managers pls comment, See the attached build_instances_${today} file\n\n";
       print OUTPUT "\n";

       if ( $show_disk_usage ) {
          printf OUTPUT "%-12.12s, %-32.32s, %-36.36s, %-9.9s, %-14.14s, %s\n","BUILD_TYPE","ACTION_NEEDED","BUILD_TAG","AGE(days)","CREATION(date)","DISK_USAGE(MB)";
       } else {
          printf OUTPUT "%-12.12s, %-32.32s, %-36.36s, %-9.9s, %-14.14s\n","BUILD_TYPE","ACTION_NEEDED","BUILD_TAG","AGE(days)","CREATION(date)";
       }
       # printf OUTPUT "%-12.12s, %-32.32s, %-36.36s, %-9.9s, %-14.14s\n","-"x15, "-"x20, "-"x30, "-"x9, "-"x14;
       foreach $build ( sort { $buildprops{$platform}{$a} <=> $buildprops{$platform}{$b} } keys %{$buildprops{$platform}} ) {

          $sorted = 0;
          $curaction = $buildprops{$platform}{$build}{action};
          if ( $show_disk_usage ) {
            printf OUTPUT "%-12.12s, %-32.32s, %36.36s, %9.9s, %14.14s, %s\n",
                         $buildprops{$platform}{$build}{type},
                         $buildprops{$platform}{$build}{action},
                         $build,
                         $buildprops{$platform}{$build}{age},
                         $buildprops{$platform}{$build}{creation},
                         $buildprops{$platform}{$build}{usage};
          } else {
            printf OUTPUT "%-12.12s, %-32.32s, %36.36s, %9.9s, %14.14s\n",
                         $buildprops{$platform}{$build}{type},
                         $buildprops{$platform}{$build}{action},
                         $build,
                         $buildprops{$platform}{$build}{age},
                         $buildprops{$platform}{$build}{creation},

          };
          if ( $curaction =~ /$DELETE_ACTION_STRING/ )   {
             print DELETEOUT   "$platform/$build\n"; $sorted = 1;
          }
          if ( $curaction =~ /$ARCHIVE_ACTION_STRING/ )  {
             print ARCHIVEOUT  "$platform/$build\n"; $sorted = 1;
          }
          if ( $curaction =~ /$RECENT_ACTION_STRING/ )   {
             print RECENTOUT   "$platform/$build\n"; $sorted = 1;
          }
          if ( $curaction =~ /$RELEASED_ACTION_STRING/ ) {
             print RELEASEDOUT "$platform/$build\n"; $sorted = 1;
          }
          if ( ! $sorted ) {
             print "ERROR: UNCATEGORIZED: $platform/$build\n";
             exit(1);
          }
       }

       # printf OUTPUT "%-12.12s, %-32.32s, %-36.36s, %3.3s, %-10.10s\n","-"x15, "-"x20, "-"x30, "-"x15, "-"x25;

       print OUTPUT "MISSING, BUILDS (MAY BE TYPO IN TWIKI):\n" if ( @missing );
       foreach $build ( sort @missing ) {
         print OUTPUT "* MISSING, $build,\n";
       }

       print OUTPUT "\n";
       chdir("${SWBUILDDIR}");
       print OUTPUT "Miscellenious, Builds/Files Taking up Disk-Space:\n";
       foreach $item ( sort grep { /$platform/ } grep { -d } @misc ) {
         $item =~ s#$platform/##g;
         print OUTPUT "DIRECTORY, $item,\n";
       }
       foreach $item ( sort grep { /$platform/ } grep { -l } @misc ) {
         $item =~ s#$platform/##g;
         print OUTPUT "SYMLINK, $item,\n";
       }
       foreach $item ( sort grep { /$platform/ } grep { -f } @misc ) {
         $item =~ s#$platform/##g;
         print OUTPUT "FILE, $item,\n";
       }
       close(OUTPUT);
       close(DELETEOUT);
       close(ARCHIVEOUT);
       close(RELEASEDOUT);
       close(RECENTOUT);
       print "$output [Excel format]\n";
       print "Done Generating Disk-Cleanup Report for : $platform\n";
   }

} # genReport()

##
## Main Program
##
sub Main {
    my ($rls, $platform, $build, $pf, $pfstr, %seenb, %seenr);
    my ($n) = 0;

    print "\n\nRunning SCRIPT_VERSION = $SCRIPT_VERSION\n\n";
    open(ERRORS,">$ERRORS") || die "Can't open ERRORS for writing. Exiting";

    foreach $pf ( @platforms ) {
       $pfstr .= "${pf}|"
    }
    $pfstr =~ s#\|$##g;
   #print "pfstr = $pfstr\n";

    ## rls_builds.txt contains a list of builds which are in Twiki Page
    open(RLSBUILDS, "$released_build_list") || die "Can't open release keep list input file: $released_build_list!!";
    while ( $rls = <RLSBUILDS> ) {
         next if ( $rls =~ /^#/ );
         next if ( $rls =~ /^\s*$/ );
         ($platform, $build) = split(/:/, $rls);
         $platform =~ s/\s+//g; lc($platform);
         $build    =~ s/\s+//g;
         if ( $platform !~ m/^($pfstr)$/g ) {
            print "\nERROR: Input File ($released_build_list) format error!\n";
            print "         platform: $platform  build: $build\n\n";
            print "Run '$0 -h' for more help\n\n";
            exit(1);
         }
         push(@{$released{$platform}}, $build);
        #print "$n: RELEASED: $platform/$build\n";
         $n = $n + 1;
    }
    close(RLSBUILDS);

    # Datastructures used in this program
    # released = {
    #    build_linux  => [array of linux  builds],
    #    build_macos  => [array of macos  builds],
    #    build_netbsd => [array of netbsd builds],
    #    build_window => [array of window builds],
    # }
    #
    # buildprops = {
    #    build_window => {
    #          <build> => {
    #                     type     = <released or engg build>,
    #                     creation = <creation-date>,
    #                     age      = <age>,
    #                     action   = <action>,
    #          }
    #    }
    # }
    # e.g $buildprops{$platform}{$build}{age}      = $age;

    #foreach $platform ( keys %released ) {
    #   #print "platform => $platform\n";
    #   foreach $build ( @{$released{$platform}} ) {
    #   #   print "$platform => $build\n";
    #   }
    #}

    # Assertain release locations
    &scanReleasedBuilds();

    chdir("${SWBUILDDIR}");
    foreach $pf ( @platforms ) {
       print "INFO: Searching for existance all builds for $pf\n";
       push(@all_builds, map { s#$pf/##g; $_} qx(find $pf -type d -follow -maxdepth 1 -mindepth 1 2> ${NULL}));
       system("find ${SWBUILDDIR}/$pf -type d -follow -maxdepth 1 -mindepth 1 2>> /tmp/${ERRORS}_find$$ > ${NULL}");
      push(@all_releases, @{$released{$pf}});
    }
    @all_builds   = sort grep { !$seenb{$_}++ } @all_builds;
    chomp(@all_builds);
    @all_releases = sort grep { !$seenr{$_}++ } @all_releases;

    #$n=1;
    #foreach $b ( @all_builds )   { printf "%4.4s) build    = %s\n",$n++,$b; }
    #$n=1;
    #foreach $r ( @all_releases ) { printf "%4.4s) released = %s\n",$n++,$r; }
    #exit(0);

    # Process builds now
    foreach $pf ( @platforms ) { &scanBuilds("$pf") }

    # Generate report from %buildprops hash
    &genReport();
    print "\n";
    print ERRORS `cat /tmp/${ERRORS}_find$$`; `rm -f /tmp/${ERRORS}_find$$`;
    close(ERRORS);

} # Main()

## Start of program
&Main();

## End of program
