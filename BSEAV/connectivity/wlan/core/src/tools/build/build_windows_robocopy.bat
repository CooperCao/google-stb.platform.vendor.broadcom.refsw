@rem = 'Run perl script through windows batch file. Description is below 
@echo off
set scriptname=%0.bat
if exist c:/tools/build/perlexec.pl                        goto CDRVTOOLS
if exist d:/tools/build/perlexec.pl                        goto DDRVTOOLS
if exist z:/projects/hnd/tools/win32/bin/perlexec.pl       goto ZDRVTOOLS
:CDRVTOOLS
set perlexec=c:/tools/build/perlexec.pl                  & goto exec
:DDRVTOOLS
set perlexec=d:/tools/build/perlexec.pl                  & goto exec
:ZDRVTOOLS
set perlexec=z:/projects/hnd/tools/win32/bin/perlexec.pl & goto exec
:exec
if     "%1"==""         perl -S %perlexec% %scriptname%
if NOT "%1"=="" shift & perl -S %perlexec% %scriptname% %*
rem echo "perl -S %perlexec% %scriptname% %*"
goto endofperl
@rem ';

## #####################################################################
##             S-T-A-R-T  O-F  P-E-R-L  S-C-R-I-P-T
## #####################################################################
##
## $ Copyright Broadcom Corporation 2003 
##
## build_window: Script to perform windows platform builds. This is a 
## wrapper around build_brand_brand.sh This implements multi-brand 
## builds by automating resource allocation dynamically, detailed 
## logging, parallelizing multiple build brands, scanning for errors, 
## reporting errors copying builds and cleaning when done
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## $Id: build_windows.bat 384933 2013-02-13 16:00:33Z fedors $
##
## SVN: $HeadURL$
##
## #####################################################################

require  "flush.pl";
use       Getopt::Long;
use       Env;

if ( -f "C:/tools/build/wl_swengg_lib.pl" ) {
   $TOOLSDIRUNIX = "c:/Tools";
} elsif ( -f "D:/tools/build/wl_swengg_lib.pl" ) {
   $TOOLSDIRUNIX = "D:/Tools";
} elsif ( -f "E:/tools/build/wl_swengg_lib.pl" ) {
   $TOOLSDIRUNIX = "E:/Tools";
} else {
   $TOOLSDIRUNIX = "Z:/home/hwnbuild/src/tools";
}

print "\n";

## Check if it unix or windows env
if ( -f "${TOOLSDIRUNIX}/build/wl_swengg_lib.pl" ) {
   use lib  "${TOOLSDIRUNIX}/build";
   push(@INC,"${TOOLSDIRUNIX}/build");
} else {
   use lib  "/home/hwnbuild/src/tools/build";
}

require "wl_swengg_lib.pl";

## -branch or -revision or -tagname are synonyms
## Multiple brands if specified run in serial in gmake and in parallel
## in emake context.
## Command line option names similar to as underlying build_windows_brand.sh script
%scan_args = (
             'all_brands'      => \$all_brands,
             'args=s'          => \$passargs,
             'brand=s'         => \@brands,
             'cvscutofftime=s' => \$cvscutoff,
             'dir=s'           => \$builddir,
             'e=s'             => \$extralogstatus,
             'fix'             => \$respin,
             'flags=s'         => \$gmakeflags,
             'force'           => \$forcebuild,
             'gmake'           => \$gmake,
             'j'               => \$emake,
             'help'            => \$help,
             'localbuild'      => \$localbuild,
             'label=s'         => \$label,
             'mail=s'          => \@notify,
             'nightly'         => \$nightly,
             'nocopy'          => \$skip_server_copy,
             'now'             => \$respin,
             'pvtmk=s'         => \$pvtmk,
             'revision=s'      => \$tagname,
             'serverdir=s'     => \$serverdir,
             'verbose'         => \$dbg,
             'xdays=s'         => \$xdays,
             );

# Scan the command line arguments
GetOptions(%scan_args) or Exit($ERROR);

# WARN: Enable debug.
# $dbg=1;

if ( defined($help) )     { &Help(); &Exit($OK); }

$tagname           = "NIGHTLY" if ( ! defined($tagname) );

## NIGHTLY is not a tag, but a mnemonic used in our build env
$BUILD_LOGS_DIR     = "c:/build_logs/$tagname";
$BUILD_LOGS_DIR_DOS = $BUILD_LOGS_DIR; $BUILD_LOGS_DIR_DOS =~ s#/#\\#g;
if ( ! -d "$BUILD_LOGS_DIR" ) { &Cmd("mkdir -p $BUILD_LOGS_DIR"); }

local $prevloghandle = "$LOGFILEHANDLE";
if ( $gmakeflags =~ /COTOOL=.*gclient/ ) {
   $CURRENTLOGFILE = "${BUILD_LOGS_DIR}/build_gclient.log";
} else {
   $CURRENTLOGFILE = "${BUILD_LOGS_DIR}/build.log";
}

&Info("Begin logfile: $CURRENTLOGFILE");
$rc = &BeginLogging("$CURRENTLOGFILE");

# If any cmd line args can not be processed, then report error
if ( @ARGV ) {
   print "\n";
   &Error3("Invalid cmd line argument: @ARGV");
   &Info("Usage:");
   &Usage();
   &Exit($ERROR);
}

# If P: or Z: drives are missing, try to map them explicitly
${HOMEDRIVE} = ( $HOME =~ /cygdrive\/(.*)/ ? "$1".":" : "$HOME" );
${HOMEDRIVE} =~ s%\\%/%g;
&Dbg("HOMEDRIVE = $HOMEDRIVE");

if ( ! -d "${HOMEDRIVE}/" ) {
   &Cmd("echo Y | net use ${HOMEDRIVE} /delete");
   &Warn("Mapping missing ${HOMEDRIVE} from \\\\brcm-sj\\dfs\\home\\${BUILDOWNER}");
   &Cmd("echo Y | net use ${HOMEDRIVE} \\\\\\\\brcm-sj\\\\dfs\\\\home\\\\${BUILDOWNER} /persistent:yes /yes");
}

if ( ! -d "Z:/home/${BUILDOWNER}" || ! -d "Z:/projects/hnd" ) {
   &Cmd("echo Y | net use Z: /delete");
   &Warn("Mapping missing Z: from \\\\brcm-sj\\dfs");
   &Cmd("echo Y | net use Z: \\\\\\\\brcm-sj\\\\dfs /persistent:yes /yes");
}

($SYSTEMDRIVELETTER) =  ( $SYSTEMDRIVE =~ m/([a-zA-Z])\:/g );

$nowts            = $dirtimestamp; $nowts =~ s#\.##g;
# mimicking missing mktemp() on non-linux platforms
$random           = join('', map { chr($_+97) } split //,sprintf("%04d",rand(10000)));
# To keep temporary scheduler task .bat files
$wintasks_dir     = "Z:/projects/hnd/swbuild/build_admin/logs/build-tasks/wintasks";
$build_script     = "${SYSTEMDRIVE}/tools/build/build_windows_brand_robocopy.sh";
# Local instance of windows build script, to allow seemless svn updates
$build_script_name= "temp_build_windows_brand_robocopy_${nowdate}.sh";
$build_script_loc = "${SYSTEMDRIVE}/temp/${build_script_name}";
$build_config_net = "Z:/projects/hnd/software/gallery/src/tools/build/build_config.sh";
$build_config_loc = "${SYSTEMDRIVE}/tools/build/build_config.sh";
$serverdir_default= "z:/projects/hnd/swbuild/build_window";
%sched            = ();
$default_bldserver= "wc-sjca-eb04";

# Pick build_config.sh from gallery first
if ( -f "${build_config_net}" ) {
   $build_config     = "${build_config_net}";
} else {
   $build_config     = "${build_config_loc}";
}

$cfgec = 0;
if ( $gmakeflags =~ /DAILY_BUILD=1/ ) {
   $default_brands = qx(bash ${build_config} -r DAILY -p windows 2> $NULL);
   $cfgec = $?;
   $daily_build    = 1;
   $gmakeflags =~ s/DAILY_BUILD=1//g;
   $gmakeflags =~ s/\s+//g;
   &Info("Daily TOT BRANDS = $default_brands");
} else {
   my $tagname_orig = ($tagname =~ /^NIGHTLY$/) ? "TRUNK" : "$tagname";
   $default_brands = qx(bash ${build_config} -r ${tagname_orig} -p windows 2> $NULL);
   $cfgec = $?;
}

if ( !@brands && ("$cfgec" != "0") ) {
   $cfgerror=`bash ${build_config} -r DAILY -p windows 2>&1`;
   &Error("$cfgerror");
   &Error("build_config at $build_config had non-zero exit code ($cfgec)");
   &Error("Can't continue. Exiting");
   exit($cfgec);
}
&Dbg("Derived   BRANDS = $default_brands");
&Dbg("Requested BRANDS_REQUESTED = @brands");

@default_brands = split(/\s+/,$default_brands);

# Reset pseudo tags to their original values
$tagname="NIGHTLY" if ( $tagname  =~ /TOTUTF/ );

# Warning and Error messages to notify BUILDADMIN
$warnmsgs = "";
$errmsgs  = "";

&checkCmdLineArgs();

print "\n";

if ( ! -d "${wintasks_dir}" ) {
   &Error3("Missing $wintasks_dir");
   my $mailsubject="ERROR: On $thishost $wintasks_dir not accessible";
   &CmdQL("echo '$mailsubject' | blat - -mailfrom hwnbuild\@broadcom.com -t '${BUILDADMIN}' -s '$mailsubject'",$CURRENTLOGFILE);
   &Exit($ERROR);
}

## in k-bytes
$mindiskspace = "4000000";

# Warn if a build is launched on a debug server
if ( grep { m/^$thishost$/gi } @aEmakeServers ) {
   $emake = 1 unless $gmake;
   if ( grep { m/^$thishost$/gi } @aEmakeServersTest ) {
        &Warn("");
        &Warn("Server '$thishost' is a test/debug server. Do not use it to launch");
        &Warn("your builds. Refer to HndBuild twiki to login to other");
        &Warn("valid build servers or contact ${BUILDADMIN}");
        &Exit($ERROR) if ( ! $forcebuild );
        print "\n";
   }
};

# Warn if a build is launched on old standalone servers
if ( grep { m/^$thishost$/gi } @aGmakeServers ) {
   $gmake = 1;
   if (( ! $forcebuild ) && ($thishost !~ /wc-sjca-e/i)) {
       &Warn("");
       &Warn("'$thishost' is an old/slower standalone build server");
       &Warn("Please use new faster WC-SJCA-EB02 or WC-SJCA-EB04");
       &Warn("cluster server to launch your windows builds");
       print "\n\nDo you want to continue? [Yes|No|Quit]: ";
       $ans = <STDIN>; chomp($ans);
       &Exit($ERROR) if ( $ans =~ m/n|q/gi );
   }
}

# Warn if build is launched on a server other than @aValidServers
if ( ! grep { m/^$thishost$/gi } @aValidServers ) {
   $gmake = 1;
   &Warn("");
   &Warn("'$thishost' is not a valid windows build server!");
   &Warn("Build may not succeed");
}

## array to hold any windows scheduler errors in parallel cluster context
@sched_errors = ();

#
# checkCmdLineArgs ( );
#
sub checkCmdLineArgs
{
   my($brandcheck) = "$OK";
   my(%seent,$ans,%seenm);
 
   if ( defined($help) )     { &Help(); &Exit($OK); }
   if ( defined($nightly) && ( $tagname != "NIGHTLY" ) ) {
      &Error3("-nightly and -r flags are mutually exclusive");
      &Exit($OK);
   }

   if ( defined($cvscutoff) ) {
      &Info("Using '$cvscutoff' time for cvs checkouts");
   } elsif ( defined($nightly) ) {
      $cvscutoff="nightly";
      &Info("Setting '$cvscutoff' cutoff date for cvs checkouts");
   } elsif ( defined($respin) ) {
      $cvscutoff="now";
      &Info("Setting '$cvscutoff' cutoff time for cvs checkouts");
   }

   if ( defined($cvscutoff) && ( $tagname !~ m/_BRANCH_|_TWIG_|NIGHTLY/ )) {
      &Error3("-cvscutoff and -r flags are mutually exclusive");
      &Exit($OK);
   }

   $builddir  =~ s#\\#/#g if ( $builddir );
   $serverdir =~ s#\\#/#g if ( $serverdir );

   unless ( $builddir && ( -d $builddir ) ) {
         $builddrive= &assignBuildDrive;
         $builddir  = "${builddrive}:/build";
         &Dbg("Assigned build drive: $builddrive");
   }
   &Info("Setting Serverdir = $serverdir") if ( $serverdir );
   $builddir   =~ s#\\#/#g;

   ## Select default brands to build for tagged/tob, if user specified none
   if ( ! @brands ) {
      if ( $nightly ) {
         &Info("No brands specified. Picking nightly default brands list");
         @brands = @default_brands;
         &Info("@brands");
      } elsif ( $all_brands ) {
         @brands = @default_brands;
         &Warn("Following all brands will be built for ${tagname}");
         &Info("It is preferable to exclude brands that are already built");
         &Info("@brands");
         if ( ! $forcebuild ) {
            print "\n\nDo you want to continue? [Y|N]: ";
            $ans = <STDIN>; chomp($ans);
            &Exit($ERROR) if ( $ans =~ m/n/gi );
         }
      } elsif ( $daily_build ) {
         @brands = @default_brands;
         &Warn("Following daily brands will be built for ${tagname}");
         &Info("@brands");
         if ( ! $forcebuild ) {
            print "\n\nDo you want to continue? [Y|N]: ";
            $ans = <STDIN>; chomp($ans);
            &Exit($ERROR) if ( $ans =~ m/n/gi );
         }
      } else {
         @brands = @default_brands;
         print "\n";
         &Info("No brands specified with '-brand <brandname>'");
         if ( ${tagname} =~ m/^NIGHTLY$/g ) {
            &Warn("For TOT, provide at least one '-brand <brandname>'");
            &Exit($ERROR);
         } else {
            &Info("Following brands will be built by default for ${tagname}");
            &Info("@brands");
            if ( ! $forcebuild ) {
               print "\n\nDo you want to continue? [Y|N]: ";
               $ans = <STDIN>; chomp($ans);
               &Exit($ERROR) if ( $ans =~ m/n/gi );
            }
         }
      }
   } else {
      # Try to auto-correct if user specifies all brands as one argument
      foreach $brand ( @brands) {
        if ( $brand =~ /\s+/ ) {
           $brand =~ s/"|'//g;
           &Warn3("Will treat each word as a build brand from '$brand'");
           push(@valid_brands, split(/\s+/,$brand)); 
        } else {
          push(@valid_brands,$brand); 
        }
      }
      @brands = grep { !$seent{$_}++ } @valid_brands;
      foreach $brand ( @brands ) {
        unless ( grep { /^$brand$/ } @default_brands ) {
          $brandcheck = "$ERROR";
          &Warn("Non-default brand: '$brand' specified. Will try to build it");
          &Dbg("build_config.sh needs to be updated to include this $brand");
        }
        &Dbg("Processed brand = $brand");
     }
   }

   if ( ! defined($notify[0]) ) {
      &Warn(" -mail not specified. Notification will go to hnd-build-list");
     @notify = "hnd-build-list\@broadcom.com ";
   }

   ## If an email address is specified without a @broadcom.com, fix it
   if ( scalar(@notify) ) {
      # Microsoft scheduler does not like long build-command paths especially
      # when we have long tag and brand names. So trim @$DOMAIN for notify list
      $notify = join(',',sort grep { !$seenm{$_}++ } @notify);
      $notify =~ s/\@$DOMAIN//g;
      $notify =~ s/\s+/,/g;
      $notify = "$notify";
   } else {
      $notify = "";
   }
} # checkCmdLineArgs ( )

#
# Usage ( )
#
sub Usage
{
   print "\n";
   print "\tcd ${TOOLSDIRUNIX}/build\n\n";
   print "\tbuild_windows.bat -revision <tagname|branch>                    
             [-brand <brand1> -brand <brand2> ...]
             [-dir <builddir>] [-serverdir <serverdir>]
             [-mail <user1> -mail <user2> ...]
             [-help]
   ";
   print "\n\tFor more help run 'build_windows.bat -help'\n\n";

} # Usage ( )

#
# Help ( )
#
sub Help
{
    &Usage();
    print "

    * You may not want to use <optional> cmd line switches below other than
      what are shown in 'Usage:' above (or as in usage examples below)

    -al,  -all_brands,      Force build all build brands

    -ar,  -args \"<args>\"  Any additional command line options to be passed
                            to build_windows or build_linux (optional)

    -b,   -brand <brand>    Brand to build. Brands are built in the order
                            they are specified in gmake and in parallel
                            across available parallel build servers in emake
                     *note: Multiple -brand  options may be specified
                     *note: Current list of brands: @default_brands;
                            (REQUIRED)

    -c,   -cvscutoff  <time> cvs cutoff time for TOT (yyyy-mm-dd HH:MM:SS)
                            (optional)

    -d,   -dir              Where to build locally (optional, default C:/build)

    -fi,  -fix              Respin TOT builds (optional)

    -fl,  -flags            Pass optional gmake flags to release make (optional)

    -fo,  -forcebuild       For tagged/tob builds, force default brand build
                            (optional)

    -g,   -gmake            Force a gmake build on emake build m/c (optional)
                            On non-emake m/c gmake is turned on by default
                            [Intended for build debugging only]

    -h,   -help             Show this help screen (optional)

    -j,   -jobs             Use emake instead of gmake (default)

    -l,   -localbuild       Force emake build to local host (optional)
                            (used for admin/debug purposes only)

    -m,   -mail <user>      Recipients you want notified 
                            (optional, default hnd-build-list)
                     *note: Multiple -mail options may be specified

    -ni,  -nightly          Build all default nightly builds (optional)
                            (used for admin/debug cronjobs only)

    -no,  -nocopy           Skip copying local build to server directory
                            (optional)

    -p,   -pvtmk <mkfile>   Use <mkfile> as top-level makefile

    -r,   -revision <tag>   Cvstagname or branch to build
                            (optional)

    -s,   -serverdir        Where to copy on server (optional, 
                            default z:/projects/hnd/swbuild/build_window for
                            tag/tob builds and c:/build_window for tot)

    -x,   -xdays <#days>    Number of days to keep old builds


   Examples (default email goes to hnd-build-list):
   1. Build win_external_wl and mail to prakashd for PO_REL_4_100_10 tag:
      build_window.bat -revision PO_REL_4_100_10 -brand win_external_wl -mail prakashd
   2. Build win_external_wl win_internal_wl brands for PO_BRANCH_4_100_9 tag:
      build_window.bat -revision PO_BRANCH_4_100_9 -brand win_external_wl -brand win_internal_wl -mail prakashd
   3. Build TOT nightly for win_external_wl win_internal_wl brands (takes current cutoff time by default):
      build_window.bat -brand win_external_wl -brand win_internal_wl
   4. Respin TOT nightly for win_external_wl and win_internal_wl brands:
      build_window.bat -brand win_external_wl -brand win_internal_wl -fix
   5. Force respin ALL TOT build brands (Use this ONLY if you really need this)
      build_window.bat -all_brands
   6. For tagged builds, respin DHD build and force dongle image/firmware 
      to rebuild:
      build_window.bat -brand win_external_dongle_sdio -flags 'FORCE_HNDRTE_BUILD=1'
    \n\n";

} # Help ( )

#
# assignBuildDrive ( )
#
sub assignBuildDrive
{
    my($avail, $df, $drive, $builddrive);
    my($buildserver) = 0;

    foreach $drive ( @{$rServerData->{$thishost}{drives}} ) {
      #&Info("Verifying $drive for available disk-space now");
       `touch ${drive}:/touch1 2> $NULL`;
       if ( ! $? ) {
	       `rm -f ${drive}:/touch1`;
	       $df = `df ${drive}: | grep -i ${drive}:`; chomp($df);
               $avail = (split(/\s+/,$df))[3];
	       $builddrive = $drive;
	       if ( $avail > $mindiskspace ) {
	          &Info("Build Drive: ${builddrive}:");
                  $buildserver = 1;
	          last;
	       }
       }
    }
    return $buildserver ? $builddrive : C;

} # assignBuildDrive ( )

#
# doBuild ( )
#
sub doBuild
{
   my($brand, $bldSvr, $delay) = @_;
   my($result) = 0;
   my($build_cmd, $bld_start_time, $hr, $min, $sec, @now);
   my($jobid, $jobname);
   my($standalonebuild);

   $standalonebuild = 1 if ($brand =~ /win8_/);

   $build_cmd  = "";
   $build_cmd .= "$build_script_loc";

   $build_cmd .= " $passargs"        if ( $passargs );
   $build_cmd .= " -f \"$gmakeflags\""   if ( $gmakeflags );
   $build_cmd .= " -b $brand";
   # for automatically scheduled builds escape the cutoff date correctly
   # $standalonebuild variable isn't set by anyone, if a brand needs
   # it for any reason, it can be set via a conditional for BRAND value
   $build_cmd .= " -c \"$cvscutoff\""  if ( $cvscutoff );
   $build_cmd .= " -e \"$extralogstatus\""        if ( $extralogstatus );
   $build_cmd .= " -d $builddir"     if ( $builddir );
   $build_cmd .= " -m \"$notify\""   if ( scalar(@notify) );
   $build_cmd .= " -p $pvtmk"        if ( $pvtmk );
   $build_cmd .= " -r $tagname"      if ( $tagname !~ /^NIGHTLY$/ );
   $build_cmd .= " -s $serverdir"    if ( $serverdir );
   $build_cmd .= " -x $xdays"        if ( $xdays );
   $build_cmd .= " -k"               if ( $forcebuild ); #force
   $build_cmd .= " -g"               if ( $gmake || $standalonebuild ); # default on non ec nodes
   $build_cmd .= " -n"               if ( $skip_server_copy );
   $build_cmd .= " -l \"$label\""    if ( $label && !$gmake ); #build label
   #$build_cmd .= " -j"               if ( $emake ); # This is default

   ## If this script is invoked on emake enabled build server,
   ## then schedule multiple builds parallely.
   ## Currently this does not query servers to see if they are
   ## busy with other tagged builds.
   if ( $emake ) {
      if ( -s "${HOMEDRIVE}/.restricted/passwd" ) {
         $password=`cat ${HOMEDRIVE}/.restricted/passwd`;
      } elsif ( -s "z:/home/${BUILDOWNER}/.restricted/passwd" ) {
         $password=`cat z:/home/${BUILDOWNER}/.restricted/passwd`;
      } else {
         $password="";
         &Warn3("${BUILDOWNER} credentials missing. Enter ${BUILDOWNER} password when prompted");
         $warnmsgs .= "WARN: ${BUILDOWNER} credentials missing\n";
         # &Exit($ERROR);
      }
      $password =~ s/\s+//g;
      @now = localtime(time());
      ($hr, $min, $sec) = ($now[2], $now[1], $now[0]);
      ## Insert artificial delay of 1min for any time diff across servers
      ## When automatic remote task querying is implemented, this delay
      ## need to be adjusted to ensure that builds are queued, instead
      ## invoking right now
      $min += $delay;

      while (  int($min/60) > 0 ) {
          # print "min = $min; hr = $hr\n";
          $min -= 60; $hr++;
          $hr = 00 if ( $hr >= 24 );
      }

      $hr  = ( $hr  < 10 ) ? "0$hr"  : $hr;
      $min = ( $min < 10 ) ? "0$min" : $min;
      $sec = ( $sec < 10 ) ? "0$sec" : $sec;
      $bld_start_time = "${hr}:${min}:${sec}";
      $SCHSCRIPT =  "spawn_${tagname}_${brand}.bat";

      `schtasks /query /s $bldSvr /fo csv > $NULL 2>&1`;
      if ( "$?" != "0" ) {
         &Warn3("Emake build server : $bldSvr is not accepting schedule jobs, defaulting to $default_bldserver host");
         $bldSvr = $default_bldserver;
         $warnmsgs .= "WARN: Build Server $bldSvr scheduler problems found\n";
      }

      if ( $gmakeflags =~ /COTOOL=.*gclient/ ) {
         $jobid="GCBLD_${tagname}_${brand}_${nowts}";
      } else {
         $jobid="BLD_${tagname}_${brand}_${nowts}";
      }

      $jobname="${wintasks_dir}/${jobid}.bat";
      $jobpath=$jobname; $jobpath =~ s%/%\\%g;

      my $brand_log="${BUILD_LOGS_DIR}/${brand}.log";
      my $temp_brand_log="${BUILD_LOGS_DIR}/${jobid}.log";
      my $temp_script="//$bldSvr/${SYSTEMDRIVELETTER}\$/temp/${build_script_name}";
      if ( ! -s "$temp_script") {
         &CmdQL("cp -v $build_script $temp_script");
         if ( ! -f "$temp_script" ) {
            # This assumes that default_bldserver is at-least up, if not alert
            # admin users
            $errmsgs .= "ERROR: Temp $temp_script couldn't be created\n";
            $errmsgs .= "ERROR: Ensure that //$bldSvr/${SYSTEMDRIVELETTER} is accessible from $thishost\n";
            if ( $bldSvr ne $default_bldserver) {
               &Warn("Temp $temp_script couldn't be created on $bldSvr. Defaulting to $default_bldserver");
               $bldSvr = $default_bldserver;
               my $temp_script="//$bldSvr/${SYSTEMDRIVELETTER}\$/temp/${build_script_name}";
               &CmdQL("cp -v $build_script $temp_script");
            }
         }
      } else {
         &Info("Local build script instance $temp_script be re-used");
      }

      open(BLDCMD, ">${jobname}") || die "Can't create ${jobname}";
      print BLDCMD "\@echo off\n";
      print BLDCMD "\@REM Auto-generated at $nowts\n";
      print BLDCMD "\@REM on $thishost and scheduled on $bldSvr\n";
      print BLDCMD "set tmplog=${temp_brand_log}\n";
      print BLDCMD "chmod -v 755 $build_script_loc                  >> %tmplog%\n";
      # See the permission on local instance of build script
      print BLDCMD "ls -l $build_script_loc                         >> %tmplog%\n";
      print BLDCMD "echo [%date% %time%] Build Command = $build_cmd\n";
      print BLDCMD "echo [%date% %time%] Build Command = $build_cmd >> %tmplog%\n";
      print BLDCMD "bash -c '${build_cmd}'                          >> %tmplog% 2>&1\n";
      print BLDCMD "echo [%date% %time%] Exit Code = %errorlevel%  >> %tmplog%\n";
      print BLDCMD "echo ========================================== >> %tmplog%\n";
      print BLDCMD "if ERRORLEVEL 0 sleep 10\n";
      print BLDCMD "exit";
      close(BLDCMD);

      sleep 2; # Artificial delay for schtask to succeed
      open(SCHCMD, ">${SYSTEMDRIVE}/temp/${SCHSCRIPT}") || die "Can't create ${SYSTEMDRIVE}/temp/${SCHSCRIPT}";
      print SCHCMD "\@echo off\n";
      print SCHCMD "schtasks /create /f /Z";
      print SCHCMD " /tn \"${jobid}\"";
      print SCHCMD " /tr \"${jobpath}\"";
      print SCHCMD " /sc once /st ${bld_start_time}";
      if ( $bldSvr =~ m/$thishost/gi ) {
         print SCHCMD " /ru broadcom\\${BUILDOWNER}";
         print SCHCMD " /rp ${password}" if ( $password !~ /^$/ );
      } else {
         print SCHCMD " /s $bldSvr /u broadcom\\${BUILDOWNER}";
         print SCHCMD " /p ${password}" if ( $password !~ /^$/ );
      }
      print SCHCMD "\n";
      close(SCHCMD);

      &Info("Scheduling $brand on ${bldSvr} to run at $bld_start_time ...");
      &Dbg("Console log for $brand recorded in following file on $bldSvr");
      &Dbg("  ${BUILD_LOGS_DIR}/${brand}.log");

      $sched{$brand}="$bldSvr $nowdate $bld_start_time";
   
      &Cmd("cat ${SYSTEMDRIVE}/temp/${SCHSCRIPT}") if ( $dbg );
      if ( &Cmd("cmd /c ${SYSTEMDRIVE}\\\\temp\\\\${SCHSCRIPT}","verbose")) {
         &Warn3("Could not schedule build '$brand' on $bldSvr. Retrying in 15sec!!");
         sleep(15);
         if ( &Cmd("cmd /c ${SYSTEMDRIVE}\\\\temp\\\\${SCHSCRIPT}","verbose")) {
            push(@sched_errors, $brand);
            &Error3("Verify ${SYSTEMDRIVE}/temp/${SCHSCRIPT} on $thishost");
            # Error message from windows scheduler: "Value for '/tr' option 
            # cannot be more than 261 character(s)"
            $errmsgs .= "ERROR: Build for '$brand' couldn't be scheduled on $bldSvr\n";
            $errmsgs .= "ERROR: Check ${BUILD_LOGS_DIR}/build.log on $thishost\n";
         }
         # TODO: else needs a scheduler query
      } else {
         `mkdir -p //$bldSvr/${SYSTEMDRIVELETTER}\$/build_logs/$tagname`;
         sleep 2; `rm -f ${SYSTEMDRIVE}/temp/${SCHSCRIPT}`;
      }
   } else {
     # gmake builds are run in sequence
      print "\n";
      &Info("Building $tagname -> $brand gmake build locally ...");
      &Info("bash -c '$build_cmd 2>&1 | tee -a ${BUILD_LOGS_DIR}/${brand}.log'");
      system("bash -c '$build_cmd 2>&1 | tee -a ${BUILD_LOGS_DIR}/${brand}.log'");
      $buildrc=$?;

      if ( $buildrc ) {
         &Error("Brand $brand FAILED");
      } else {
         &Info("Brand $brand SUCCEEDED");
      }
   } # emake = 1

   if (( ! $emake ) && ( ! $skip_server_copy )) {
      # build_windows_brand.sh can create either ${brand} or ${brand}_0 or
      # ${brand}_1 and so on, in local builddir. So clean all instances
      # on local builddir, if build has already been copied to serverdir
      $buildfailed = 0;
      $date = `${TOOLSDIRUNIX}/win32/bin/date '+%y%m%d'`; chomp($date);
      if ( "$tagname" == "NIGHTLY" ) {
         $localdir="$builddir/$tagname/${brand}_${date}";
      } else {
         $localdir="$builddir/$tagname/${brand}";
      }
      foreach $iter ( 0 .. 20 ) {
        $iter = 20 - $iter;
        $localdir_iter="${localdir}_${iter}";
        if ( -d "$localdir_iter" ) {
           if ( -d "$serverdir_default/$tagname/$brand" ) {
              &Cmd("rm -rf ${localdir_iter}");
           } else {
              &Error("Serverdir=$serverdir_default/$tagname/$brand not accessible");
              &Error("Retaining local copy of build at");
              &Error("${localdir_iter} temporarily");
              &Error("Manually copy it to server from $thishost");
              $errmsgs .= "ERROR: $serverdir_default/$tagname/$brand dir not found\n";
              $errmsgs .= "ERROR: Skipping server copying\n";
              $buildfailed = 1;
           }
        }
      }
      if ( ( $buildfailed != "1" ) && ( -d "${localdir}" ) ) {
           if ( -d "$serverdir_default/$tagname/$brand" ) {
              &Cmd("rm -rf ${localdir}");
           } else {
              &Error("Serverdir=$serverdir_default/$tagname/$brand not accessible");
              &Error("Retaining local copy of build at");
              &Error("${localdir} temporarily");
              &Error("Manually copy it to server from $thishost");
              $errmsgs .= "ERROR: $serverdir_default/$tagname/$brand dir not found\n";
              $errmsgs .= "ERROR: Skipping server copying\n";
           }
      }
   }

   return($result);

} # doBuild ( )

#
# Main ( )
#
sub Main
{
   &Info3("Starting Build for $tagname");
   print "\n";

   # Temporarily wc-sjca-eb13 has some ssh issues with cygwin
   # It can take a build request, but it can't actually launch the build
   # Until that is resolved, skip that server for brand builds.
   foreach my $srv (@aEmakeServers) {
	push(@aEmakeServersTemp, $srv) unless ($srv=~/wc-sjca-eb13/i);
   }

   if ( $emake ) {
      $no_of_servers = @aEmakeServersTemp;

      # Randomly choose different build server combinations for parallel builds
      $rand = int(rand($no_of_servers));
      foreach $brand ( @brands ) {
         $psrv{$brand} = $aEmakeServersTemp[$rand];
         &Dbg("rand = $rand t=$brand s=$aEmakeServersTemp[$rand]");
         $rand = ( $rand > 0 ) ? $rand - 1 : $no_of_servers - 1 ;
      }
   }

   # Win8 builds go to standalone servers.
   my @win8ServersWdk8161=("WC-SJCA-EB101","WC-SJCA-EB110","WC-SJCA-E104");
   my @win8ServersWdk9200=("WC-SJCA-E105");
   my @tagsWdk8161=();
   my %tagsWdk8161Hash = map { $_ => 1 } @tagsWdk8161;
   foreach $brand ( grep { /win8_/ } @brands ) {
      my @win8ServersTemp = ( ( $brand =~ /win8_slate_/ ) || exists ( $tagsWdk8161Hash{$tagname} ) ) ? @win8ServersWdk8161 : @win8ServersWdk9200;
      $psrv{$brand} = $win8ServersTemp[int(rand(@win8ServersTemp))];
      $rand = ( $rand > 0 ) ? $rand - 1 : @win8ServersTemp - 1 ;
   }

   ## Small artificial delay (in mins) introduced for emake builds.
   ## Parallel builds are scheduled in batches, with batch-size being
   ## same as number of parallel build launch servers(wc-sjca-eb02/03/04)
   foreach $srv ( @aEmakeServersTemp ) {
     $delay{$srv} = 2;
   }

   $ldelay = 2;
   foreach $brand ( @brands ) {
      next if ( $brand =~ /^\s+$/ );
      if ( $localbuild ) {
         &Info("Launching $brand build on $thishost. Other brand builds will follow");
         &doBuild($brand, $thishost, $ldelay);
         $ldelay += 15;
      } else {
         $delay   = $delay{$psrv{$brand}}+$ldelay;
         $ldelay += 2;
         if ( exists $rEmakeBldDuration->{$brand} ) {
            &Dbg("Adding $rEmakeBldDuration->{$brand} min delay to subsequent builds on $psrv{$brand} after $brand");
            $delay{$psrv{$brand}} += $rEmakeBldDuration->{$brand};
         } else {
            &Dbg("Adding $rEmakeBldDefaultDuration default delay to subsequent builds on $psrv{$brand} after $brand");
            $delay{$psrv{$brand}} += $emakeBldDefaultDuration;
         }
         &doBuild($brand, $psrv{$brand}, $delay);
      }
   }

   if ( keys %sched ) {
      @now = localtime(time());
      ($hr, $min, $sec) = ($now[2], $now[1], $now[0]);
      &Info3("[${hr}:${min}:${sec}] Final Brand Build Schedule Details:");
      &Info("BUILD-SERVER DATE       STARTTIME => BRAND");
      &Info("---------------------------------------------------------------");
      foreach $brand ( sort { $sched{$a} cmp $sched{$b} } keys %sched ) {
         next if grep { m/^$brand$/ } @sched_errors;
         &Info("$sched{$brand}  => $brand") if ( ! $localbuild );
         &Info("$thishost $sched{$brand}  => $brand") if ( $localbuild );
      }
      print "\n";
   }

   if ( scalar(@sched_errors) ) {
      &Error3("Build brands scheduling failed for: @sched_errors");
   }

   if ( "$warnmsgs" || "$errmsgs" ) {
      $warnmsgs =~ s/('|")/\\$1/g;
      $errmsgs  =~ s/('|")/\\$1/g;

      &Warn("Sending build errors and warnings alert to '${BUILDADMIN}'");
      $mailsubject="Build Errors/Warnings for ${tagname} build on $thishost at $nowtimestamp";
      &Cmd("echo '$errmsgs$warnmsgs' | blat - -mailfrom hwnbuild\@broadcom.com -t '${BUILDADMIN}' -s '$mailsubject'",$CURRENTLOGFILE);
   }

} # Main ( )

&Main();

&Info("End logfile: $CURRENTLOGFILE");
&EndLogging("$CURRENTLOGFILE");
#&Cmd("chmod ugo-w $CURRENTLOGFILE");
$LOGFILEHANDLE = "$prevloghandle";

__END__
:endofperl
