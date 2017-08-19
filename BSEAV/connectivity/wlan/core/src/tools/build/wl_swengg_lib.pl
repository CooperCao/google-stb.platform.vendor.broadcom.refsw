## #########################################################################
##
## $ Copyright Broadcom Corporation 2003 $
##
## This library contains the globals definitions and variables
## for HND sw development environment. This is owned by hnd sw
## SCM group. This will become a WLAN SW perl module, which will
## used by most of the client scripts
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## $Id$
##
## SVN: $HeadURL$#
##
## #########################################################################

use        Env;
require    "flush.pl";

#$dbg = 1;
## program status or exit code variables
$OK            = "0";
$ERROR         = "1";
$IGNORE        = "2";
$SKIP          = "2";

# $SIG{INT}    = 'IGNORE';   ## Ctrl-C
# $SIG{TSTP}   = 'IGNORE';   ## Ctrl-Z
# $SIG{QUIT}   = 'IGNORE';   ## Ctrl-\
# $SIG{INT}      = \&sigHandler;
# $SIG{TSTP}     = \&sigHandler;
# $SIG{QUIT}     = \&sigHandler;

## commonly used echo or print prefixes for pretty printing
$bs_           = "         |";
$is_           = "   INFO> |";
$es_           = " *ERROR> |";
$ds_           = "  DEBUG> |";
$ns_           = "   NOTE> |";
$ws_           = " - WARN> |";

# Program calling this library
$myname        = `basename $0`; chomp($myname); $myname =~ s#\\#/#g;

$uname         = `uname -s`; chomp($uname);

if ( $uname =~ m/cygwin_nt/gi ) {
   $hostenv  = "win32";
   $build_pf = "windows";
} else {
   $hostenv  = "unix";
   $build_pf = "linux";
}

## Tools and Utilities (platform specific)
$CVS            = "/bin/cvs";
$NULL           = "/dev/null";
if ( $hostenv   =~ m/win32/gi ) {
   $RSH         = "C:\\winnt\\system32\\rsh.exe";
  #$USER        =  getpwuid($<);  ## ActiveState Perl does not have this!!!
  #($USER)      = $USER  =~ m/([^*].*)/g;
   $USER        =  $USERNAME;
} else {
   $RSH         = "rsh";
   $USER        =  getpwuid($<);
   ($USER)      = $USER  =~ m/([^*].*)/g;
}

## ADMINTOOLS var is set only on SCM/build servers for "hwnbuild" user.
## This flag can be used to certain system build/specific activities
$ADMINTOOLS    = "$ENV{'ADMINTOOLS'}"; $ADMINTOOLS =~ s#\\#/#g;

$BUILDOWNER    = "hwnbuild";
$BUILDADMIN    = 'hnd-software-scm-list@broadcom.com';
$thishost      = uc(`hostname`); $thishost =~ s#\s+##g; #chomp($thishost);

@now           = localtime(time());
$nowyear       = sprintf("%.4d",$now[5]+1900);
$nowmonth      = sprintf("%.2d",$now[4]+1);
$nowday        = sprintf("%.2d",$now[3]);
$nowtime       = sprintf("%.2d:%.2d:%.2d",$now[2],$now[1],$now[0]);
$nowdate       = sprintf("%d.%.2d.%.2d",$nowyear,$nowmonth,$nowday);
$nowtimestamp  = "$nowdate $nowtime";
$dirtimestamp  = sprintf("%d.%.2d.%.2d_%.2d.%.2d.%.2d",$now[5]+1900,$now[4]+1,$now[3], $now[2], $now[1], $now[0]);

#disabled# push @NIGHTLY_NOTIFY_LIST, "some-build-listzzzzzzz\@broadcom.com";

## These build servers may have non-uniform drives as compared to other servers
$rServerData = {
    "NT-SJCA-0517" => {
         drives                 =>  [qw(C)],
         TOOLSDRIVE             =>  C,
    },
};

# standalone old gmake-only build servers
$rGmakeServerData = {
	"NT-SJCA-0516" => {
		TOOLSDRIVE          =>  C,
	},
	"NT-SJCA-0517" => {
		TOOLSDRIVE          =>  C,
	},
	"WC-SJCA-EB101" => {
		TOOLSDRIVE          =>  C,
	},
	"WC-SJCA-E104" => {
		TOOLSDRIVE          =>  C,
	},
};

## Currently active emake (parallel) build servers
## Current Test Cluster Windows build launch server WC-SJCA-EB01
## is not used for Windows builds, instead use WC-SJCA-E031
if ( $thishost =~ /WC-SJCA-E031/ ) {
    # test cluster build servers
    $rEmakeServerData = {
        "WC-SJCA-E031" => {
             TOOLSDRIVE             =>  C,
        }
    }
} else {
    # production cluster build servers
    $rEmakeServerData = {
        "WC-SJCA-EB04" => {
             TOOLSDRIVE             =>  C,
        },
        "WC-SJCA-EB12" => {
             TOOLSDRIVE             =>  C,
        },
        "WC-SJCA-EB13" => {
             TOOLSDRIVE             =>  C,
        },
    }
}

# Backup emake build launch servers, in-case primary ones above
# have any issues. These are not live yet. These are backup/standby
# for production right now.
$rEmakeBackupServerData = {
        "WC-SJCA-EB11" => {
             TOOLSDRIVE             =>  C,
        },
};

@aEmakeServersTest   = qw(WC-SJCA-E031);
@aGmakeServers       = (sort keys %{$rGmakeServerData});
@aEmakeServers       = (sort keys %{$rEmakeServerData});
@aEmakeBackupServers = (sort keys %{$rEmakeBackupServerData});
@aValidServers       = (@aGmakeServers,@aEmakeServers,@aEmakeBackupServers);

## Default build duration(mins) if it doesn't appear in $rEmakeBldDuration
$emakeBldDefaultDuration = 10;

## Current emake build times (approx in mins) per brand
$rEmakeBldDuration = {
      win_external_wl                  => 35,
      win_internal_wl                  => 30,
      win_mfgtest_wl                   => 25,
      win8_external_wl                  => 35,
      win8_internal_wl                  => 30,
      win8_mfgtest_wl                   => 25,
      win_tools                        => 5,
      win_bcmdl                        => 5,
      win_mfgtest_dongle_sdio          => 5,
      win_external_dongle_sdio         => 10,
      efi-internal-wl                  => 10,
      efi-external-wl                  => 10,
      nucleus-mfgtest-dongle-sdio      => 20,
      nucleus-external-dongle-sdio     => 20,
      nucleus-internal-dongle-sdio     => 20,
      win_wps_enrollee                 => 5,
};

## Currently active release properties and config data
## This data structure is not used
#disabled# $rBranchData = {
#disabled#     somerelease1    => {
#disabled#          branch_name            =>  somerelease1,
#disabled#          branch_description     =>  "What is this branch about",
#disabled#          branch_status          =>  DISABLE,
#disabled#          branch_heirarchy       =>  1,
#disabled#          release_area           =>  somerelease1,
#disabled#          build_schedule         =>  [qw(Mon Wed Fri)],
#disabled#          tagbuild_notify        =>  [@TAGBUILD_NOTIFY_LIST],
#disabled#          nightlybuild_notify    =>  [@NIGHTLYBUILD_NOTIFY_LIST],
#disabled#          release_notify         =>  [@RELEASE_NOTIFY_LIST],
#disabled#     },
#disabled# };

## Verify if an env var is already set, if not, set it to defaults
sub setVar
{
    ($name, $value) = @_;

    if ( ! ${$name} ) {
       ${$name} = $ENV{$name} = $value;
    } else {
      #print "Preserving \$$name = ${$name}\n";
    }
}

## setBuildEnvDirs ( )
## Set all build environment variables. Many of these dirs listed here
## do not exist currently, HND s/w is intended to be organized to provide
## all these details later
sub setBuildEnvDirs
{
    my($branch) = shift;

    $branch             =~ s#\s+##g;
    #disabled# $BRANCH  = $rBranchData->{$branch}{release_area};

    # All these variables also become ENV variables, as we source Env.pm perl module
    $HNDBASE            = "";    #Define this
    $HNDSRC             = "${HNDBASE}/src";
    $DOMAIN             = "broadcom.com";
    $MAILHOST           = "mailhost.$DOMAIN";

    #disabled# $STAGINGSVR         = "brcm-sj";
    #disabled# # Staging Area Primary Directories [future use]
    #disabled# if ( $hostenv =~ m/win32/gi ) {
    #disabled# 	$RELDIR         = "//${STAGINGSVR}/dfs/projects/hnd/swbuild";
    #disabled# } else {
    #disabled# 	$RELDIR         = "/projects/hnd/swbuild";
    #disabled# }
    #disabled# $LOGSDIR            = "${RELDIR}/LOGS";
    #disabled# $BUILDDIR           = "${RELDIR}/BUILD";
    #disabled# $NOTEDIR            = "${RELDIR}/NOTES";
    #disabled# $WIPDIR             = "${RELDIR}/WIP";
    #disabled# $CONFIGDIR          = "${RELDIR}/CONF";

    # Staging Area (secondary) Sub-Directories [future use]
    #disabled# $MASTERLOGSDIR      = "${RELDIR}/LOGS/MasterLogs";
    #disabled# $MAKELOGSDIR        = "${RELDIR}/LOGS/MakeLogs";
    #disabled# $PUBLISHLOGSDIR     = "${RELDIR}/LOGS/PubLogs";
    #disabled# $ENVLOGSDIR         = "${LOGSDIR}/EnvLogs";
    #disabled# $NIGHTLYLOGSDIR     = "${LOGSDIR}/NitelyLogs";
    #disabled# $CBTLOGSDIR         = "${LOGSDIR}/CbtLogs";
    #disabled# $BUGFIXES           = "${WIPDIR}/BugFixes";
    #disabled# $CHECKINS           = "${WIPDIR}/CheckIns";
    #disabled# $NIGHTLYBUILDDIR    = "${RELDIR}/BUILD/NIGHTLY";

    # Compilation env settings [avoid hard-coding env vars with drive letter]
    if ( $hostenv =~ m/win32/gi ) {
       &setVar("BUILD_DRIVE",     "D:");
       &setVar("TOOLSDRIVE",      "$rServerData->{$thishost}{TOOLSDRIVE}");
       $TOOLSDRIVE                = "C" if ( ! $TOOLSDRIVE );

       &setVar("MSDEV",           "${TOOLSDRIVE}:/tools/msdev/studio");
       &setVar("DDK",             "${TOOLSDRIVE}:/tools/msdev/ddk");
       &setVar("MSSDK",           "${TOOLSDRIVE}:/tools/msdev/PlatformSDK");
       &setVar("IS_COMMON",       "${TOOLSDRIVE}:/tools/installshield6/common");
       &setVar("IS_ROOT",         "${TOOLSDRIVE}:/tools/installshield6/root");
       &setVar("IS12_ROOT",       "${TOOLSDRIVE}:/tools/InstallShield12SAB");
       &setVar("IS2009_ROOT",     "${TOOLSDRIVE}:/tools/InstallShield2009SAB");
       &setVar("VC152",           "${TOOLSDRIVE}:/tools/msdev/vc152");
       &setVar("W95DDK",          "${TOOLSDRIVE}:/tools/msdev/95ddk");
       &setVar("W98DDK",          "${TOOLSDRIVE}:/tools/msdev/98ddk");
       &setVar("_WINCEROOT",      "${TOOLSDRIVE}:/tools/msdev/wince420");
       &setVar("_WINCE_BUILD_TOOL_DIR","${TOOLSDRIVE}:\\tools\\msdev\\wince420\\bin\ipaq");
       &setVar("WDMDDK",          "${TOOLSDRIVE}:/tools/msdev/nt5ddk");
       &setVar("WDM2600DDK",      "${TOOLSDRIVE}:/tools/msdev/2600ddk");
       &setVar("VERISIGN_ROOT",   "${TOOLSDRIVE}:/tools/verisign");
       &setVar("VERISIGN_PASSWORD","brcm1");
       &setVar("WDM37900DDK",     "${TOOLSDRIVE}:/tools/msdev/3790ddk");
       # 3790ddk1830 is used only for chkinf validation
       &setVar("WDM37900DDK1830", "${TOOLSDRIVE}:/tools/msdev/3790ddk1830");
       &setVar("NTICE",           "${TOOLSDRIVE}:/tools/msdev/softicent");
       &setVar("VS2003AMD64",     "${TOOLSDRIVE}:/tools/msdev/VS2003AMD64");

       &setVar("ACTIVE_PERL_ROOT","${TOOLSDRIVE}:/tools/ActivePerl/Perl");
       &setVar("HNDEMAKE_DIR",    "${TOOLSDRIVE}:\\tools\\ECloud");
       &setVar("WIND_HOST",       "${TOOLSDRIVE}:/tools/Tornado2.1");
       &setVar("WIND_HOST_TYPE",  "x86-win32");
       &setVar("BUILD_CVSSRC",    "C:/build_cvssrc");
       &setVar("CLEANUP_DIR",     "${BUILD_DRIVE}/tools/build/cleanup");
       &setVar("TEMP",            "C:/temp");
       &setVar("TMP",             "C:/temp");

       $ENV{'3790SDK1830'} = "${TOOLSDRIVE}:/tools/msdev/3790sdk1830";
    }

    # Individual File Names
    # This file should record all the activities being done as hwnbuild user
    # for SCM builds
    #&setVar("ACTIVITY_LOGFILE","${RELDIR}/LOGS/MasterLogs/activity.log");

    # Skew rBranchData to generate rRlsData
    #disabled# &generateReleaseData($rBranchData);
} # end setBuildEnvDirs()

&Dbg("Setting build environment variables");
&setBuildEnvDirs($branch);

##
## Function Definitions Follow
##

## swap branch name with release name to generate %rRlsData
## i.e. given a release name, how do you gets properties
#disabled# sub generateReleaseData
#disabled# {
#disabled#     foreach $br ( sort keys %$rBranchData ) {
#disabled#        foreach $rls_key ( sort keys %{$rBranchData->{$br}} ) {
#disabled#          if ( ref($rBranchData->{$br}{$rls_key}) eq "ARRAY" ) {
#disabled#             @{$rRlsData->{$rBranchData->{$br}{release_area}}{$rls_key}} =
#disabled#                                  @{$rBranchData->{$br}{$rls_key}};
#disabled#          } elsif ( ref($rBranchData->{$br}{$rls_key}) eq "HASH" ) {
#disabled#             %{$rRlsData->{$rBranchData->{$br}{release_area}}{$rls_key}} =
#disabled#                                  %{$rBranchData->{$br}{$rls_key}};
#disabled#          } else {
#disabled#             $rRlsData->{$rBranchData->{$br}{release_area}}{$rls_key} =
#disabled#                               $rBranchData->{$br}{$rls_key};
#disabled#          }
#disabled#       }
#disabled#     }
#disabled# }

#
# ScanArgs ( )
# Generate a listing of specified command line args [used for
# logging and debugging purposes]
#
#disabled# sub ScanArgs
#disabled# {
#disabled#     %scan_args = @_;
#disabled#     $args      = "";
#disabled#
#disabled#     foreach $key ( sort keys %scan_args ) {
#disabled#       if ( ref($scan_args{$key}) eq "SCALAR" )
#disabled#       {
#disabled#          $val    =  ${$scan_args{$key}};
#disabled#          $key    =~ s/=s//g;
#disabled#          $args  .= "$key=${val}; " if ( $val );
#disabled#       }
#disabled#       elsif (    ref($scan_args{$key}) eq "ARRAY" )
#disabled#       {
#disabled#          @val    =  @{$scan_args{$key}};
#disabled#          $key    =~ s/=s//g;
#disabled#          $args  .= "$key={@val}; " if ( $#val ge "0" );
#disabled#       }
#disabled#       elsif (    ref($scan_args{$key}) eq "HASH"    )
#disabled#       {
#disabled#          %val    = %{$scan_args{$key}};
#disabled#          $key    =~ s/=s//g;
#disabled#          $args  .= "$key={%val}; ";
#disabled#       }
#disabled#     }
#disabled#     return "$args";
#disabled# }

sub sigHandler {
    my($signaltype) = shift;

    &Warn("Got a terminal interrupt ($signaltype) !!! Ignoring");
    #&cleanUP;
    #exit;
}

#
# LogActivity ( )
# Log all the build activities in a master log
#
#disabled# sub LogActivity
#disabled# {
#disabled#     my ($package,$thisfn,$line);
#disabled#     my ($progargs,$logmsg);
#disabled#
#disabled#     ($package,$thisfn,$line) = caller;
#disabled#
#disabled#     $thisfn   =~ s#\\#/#g;
#disabled#     $progname =  `basename $thisfn`;    chomp($progname);
#disabled#     $progargs =  &ScanArgs(%scan_args);
#disabled#
#disabled#     $logmsg   = "($nowtimestamp): ($USER@${thishost}) (prog=$progname) (args=$progargs)";
#disabled#
#disabled#     open(ACTIVITYLOGFILE,">>${ACTIVITY_LOG}");
#disabled#     print ACTIVITYLOGFILE "$logmsg\n";
#disabled#     close(ACTIVITYLOGFILE);
#disabled# }

# hndPrintNF ( )
# Print to filehandle/stdout but do not flush the buffer.
#
sub hndPrintNF
{
    my($ts,@now);

    if ( $LOGFLAG ) {
       @now = localtime(time());
       $ts = sprintf("%d-%.2d-%.2d %.2d:%.2d:%.2d",$now[5]+1900, $now[4]+1, $now[3], $now[2], $now[1], $now[0]);
       print $LOGFILEHANDLE "\[$ts\]: @_\n";
       flush($LOGFILEHANDLE);
    }

    print STDOUT "@_\n"; flush(STDOUT);    flush(STDERR);
}

#
# hndPrint ( )
# Print to filehandle/stdout but do flush the buffer.
#
sub hndPrint
{
    my($ts,@now);

    if ( $LOGFLAG ) {
       @now = localtime(time());
       $ts = sprintf("%d-%.2d-%.2d %.2d:%.2d:%.2d",$now[5]+1900, $now[4]+1, $now[3], $now[2], $now[1], $now[0]);
       print $LOGFILEHANDLE "\[$ts\]: @_\n";
       flush($LOGFILEHANDLE);
    }

    print STDOUT "@_\n"; flush(STDOUT);    flush(STDERR);
}

#
# Print ( )
#
sub Print  { &hndPrint("@_"); }

#
# Cmd ( )
# Echo the command being executed before execution.
#
sub Cmd {
    my($cmd)=shift;
    my($verbose)=shift;
    my $ec;

    &Print("$cmd");
    $rc=`$cmd 2>&1`; $ec=$?;
    $verbose ?  &hndPrint("exitcode=$ec; cmdoutput=$rc") : &hndPrint("$rc");
    return $ec;
}

#
# CmdQ ( )
# Do not echo the command
#
sub CmdQ  {
    my($cmd)=shift;
    my $ec;

    &Dbg("'$cmd' exitcode=$?");
    `$cmd`; $ec = $?;
    return $ec;
}

#
# CmdQL ( )
# Do not echo the command, but print to logfile
sub CmdQL  {
    my($cmd)=shift;
    my($log)=shift;
    my $ec;

    $log = $NULL if ( ! $log );
    `echo "$cmd" >> $log`;
    `$cmd >> $log 2>&1`; $ec=$?;
    &Dbg("'$cmd' exitcode=$ec");
    return $ec;
}

#
# Echo ( )
# Print the indentation
#
sub Echo { $msg = shift(@_); &hndPrint("$cs_ $msg"); }

#
# Cont3 ( )
#
sub Cont3 { $msg = shift(@_); &hndPrint("${cs_}\n${cs_} ${msg}\n${cs_}");  }

#
# Info ( )
#
sub Info { $msg = shift(@_); &hndPrint("$is_ $msg"); }

#
# Info3 ( )
#
sub Info3
{ $msg = shift(@_); &hndPrint("${is_}\n${is_} ${msg}\n${is_}"); }

#
# Note ( )
#
sub Note { $msg    = shift(@_); &hndPrint("$ns_ $msg"); }

#
# Note3 ( )
#
sub Note3
{ $msg = shift(@_); &hndPrint("${ns_}\n${ns_} ${msg}\n${ns_}"); }

#
# Warn ( )
#
sub Warn  { $msg = shift(@_); &hndPrint("$ws_ $msg"); }

#
# Warn3 ( )
#
sub Warn3
{ $msg = shift(@_); &hndPrint("${ws_}\n${ws_} ${msg}\n${ws_}"); }

#
# Error ( )
#
sub Error     { $msg    = shift(@_); &hndPrint("$es_ $msg\a"); }

#
# Error3 ( )
#
sub Error3 { $msg    = shift(@_); &hndPrint("${es_}\n${es_} ${msg}\a\n${es_}"); }

#
# Exit ( )
#
sub Exit { my($exitcode)=shift; &Print("Exiting: $exitcode"); exit($exitcode); }

#
# Dbg ( )
#
sub Dbg
{
    my($msg,$line,$thisfn,$package);

    if ( $dbg > 0 && $dbg <= 3 )
    {
       $msg = shift(@_);
       ($package,$thisfn,$line)    = caller;
       &hndPrint("$ds_ \[$line\]: $msg");
    }
    elsif ( $dbg > 4 && $dbg <= 7 )
    {
       $msg = shift(@_);
       ($package,$thisfn,$line)    = caller;
       &hndPrint("$ds_ \[$thisfn() $line\]: $msg");
    }
    elsif ( $dbg > 8 && $dbg <= 10 )
    {
       $msg = shift(@_);
       ($package,$thisfn,$line)    = caller;
       &hndPrint("$ds_ \[${package}\:\:${thisfn}() $line\]: $msg");
    }
}

# Alert admin users when some important script step can't continue
sub AlertAdmin {
        my $msg = shift;

        &Error("Alerting Admin Users: $BUILDADMIN");
        &Error("$0: $msg");
        system("echo '$msg' | mail -s 'ERROR: $0 exited' $BUILDADMIN");
        &Exit($ERROR);
}

#
# substPathSep ( )
#
sub substPathSep
{ foreach $var ( keys %ENV ) { $ENV{$var} =~ s#\\#/#g; } }

#
# BeginLogging ( )
# Log all the session log activities
#
sub BeginLogging
{
    $LOGFILE       = shift;
    my (@now, $nowtime, $nowdate, $nowtimestamp);

    $LOGFLAG       = 1;
    @now           = localtime(time());
    $nowtime       = sprintf("%.2d:%.2d:%.2d",$now[2],$now[1],$now[0]);
    $nowdate       = sprintf("%d.%.2d.%.2d",$now[5]+1900,$now[4]+1,$now[3]);
    $nowtimestamp  = "$nowdate $nowtime";

    ($package,$thisfn,$line) = caller;

    ##print "\nSetting LOGFLAG = $LOGFLAG\n";
    ##print "Setting LOGFILE = $LOGFILE\n\n";

    $pwd           = `pwd`;
    $LOGF_BASENAME = `basename $LOGFILE`;         chomp($LOGF_BASENAME);
    $LOGFILEHANDLE = uc("$LOGF_BASENAME");

    `chmod ugo+w $LOGFILE` if ( -f "$LOGFILE" );
    open($LOGFILEHANDLE,">>$LOGFILE");
    print $LOGFILEHANDLE "\n","="x"78","\n";
    print $LOGFILEHANDLE "BEGIN LOGGING:\n";
    print $LOGFILEHANDLE "**************\n";
    print $LOGFILEHANDLE "  BUILD        : $cvstag\n";
    print $LOGFILEHANDLE "  LOGGED_FUNC  : $thisfn\[$line\]\n";
    print $LOGFILEHANDLE "  LOGGING_TIME : $nowtimestamp\n";
    print $LOGFILEHANDLE "  LOG_FILE     : $LOGFILE\n";
    print $LOGFILEHANDLE "="x"78","\n\n";

    flush(STDOUT); flush(STDERR); flush($LOGFILEHANDLE);
}

#
# EndLogging ( )
# Finish loggging and close the logfilehandle
#
sub EndLogging
{
    $LOGFILE       = shift;
    my (@now, $nowtime, $nowdate, $nowtimestamp);

    $LOGFLAG       = 0;
    @now           = localtime(time());
    $nowtime       = sprintf("%.2d:%.2d:%.2d",$now[2],$now[1],$now[0]);
    $nowdate       = sprintf("%d.%.2d.%.2d",$now[5]+1900,$now[4]+1,$now[3]);
    $nowtimestamp  = "$nowdate $nowtime";

    ($package,$thisfn,$line) = caller;

    ##print "Setting LOGFLAG = $LOGFLAG\n";
    ##print "Setting LOGFILE = $LOGFILE\n\n";

    $LOGF_BASENAME = `basename $LOGFILE`;         chomp($LOGF_BASENAME);
    $LOGFILEHANDLE = uc("$LOGF_BASENAME");

    print $LOGFILEHANDLE "\n";
    print $LOGFILEHANDLE "END LOGGING:\n";
    print $LOGFILEHANDLE "************\n";
    print $LOGFILEHANDLE "  BUILD        : $cvstag\n";
    print $LOGFILEHANDLE "  LOGGED_PROG  : $thisfn\[$line\]\n";
    print $LOGFILEHANDLE "  LOGGING_TIME : $nowtimestamp\n";
    print $LOGFILEHANDLE "  LOG_FILE     : $LOGFILE\n";
    print $LOGFILEHANDLE "="x"78","\n\n";

    flush(STDOUT); flush(STDERR); flush($LOGFILEHANDLE);
    close($LOGFILEHANDLE);
    `chmod ugo-w $LOGFILE` if ( -f "$LOGFILE" );
}

#
# printEnv() can be called to print the
# current target environment settings.
#
#disabled# sub printEnv
#disabled# {
#disabled#
#disabled#        &Info("
#disabled#          Current Env Settings...
#disabled#
#disabled#          HNDBASE   = $ENV{'HNDBASE'},
#disabled#          HNDSRC    = $ENV{'HNDSRC'},
#disabled#         ");
#disabled# }

# setenv_win_tools ( )
#
#disabled# sub setenv_win_tools
#disabled# {
#disabled#     my($echoflag) = shift;
#disabled#
#disabled#     $ENV{"BLAHBLAH"} = $BLAHBLAH = "BLAHBLAH";
#disabled#
#disabled#     if ( $echoflag ne "noecho" ) { &printEnv();    }
#disabled# }

## This library should return 1

1;
