## #####################################################################
##             S-T-A-R-T  O-F  P-E-R-L  S-C-R-I-P-T
## #####################################################################
##
## $ Copyright Broadcom Corporation 2005
##
## Repkg_WinBuild.pl
##   This script is used to perform pc-oem release repackaging
##   In its basic implementation, it can merge trayapp+installer from
##   one build to driver from another to generate new signed Setup.exe
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## $Id$
##
## SVN: $HeadURL$
##
### #####################################################################

if ( -f 'C:/tools/build/bcmretrycmd.exe' ){
	$RETRYCMD = "C:/tools/build/bcmretrycmd.exe";
}
else{
	$RETRYCMD = "Z:/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe";
}

## Check the host on which this script is run. It has to be a windows build server
print "\n\n";
my $thisuname = qx(uname -s); chomp($thisuname);

if ($thisuname !~ /CYGWIN/i) {
	print "  WARN:\n";
	print "  WARN: This script needs to be run on a windows build server\n";
	print "  WARN:\n";
	sleep 3;
}

## Derive oem and winos from given builddir
BEGIN {
	my (@ARGVTMP) = @ARGV;

	while ( $ARGVTMP[0] ) {
		my $arg = shift @ARGVTMP;
		if ($arg =~ /^-oem/ ) {
			$oem = shift @ARGVTMP;
		} elsif ($arg =~ /^-build/ ) {
			$builddir = shift @ARGVTMP;
		}
	} # while

	# Most common OEMs are bcm, hp and dell
	# We only build bcm or dell or hp branded packages
	# But we can package as oem_alias
	# (i.e. bcm may be packaged as Toshiba or Sony)
	# So following validates derived oem name from -builddir value
	if (!$oem && $builddir) {
		($oem) = ($builddir =~ m%[/\\](dell|hp|bcm)%i);
		$oem = lc($oem);
		if (!$oem) {
			print "ERROR:\n";
			print "ERROR: -oem isn't specified or can't be derived\n";
			print "ERROR:\n";
			exit(1);
		} else {
			print "  INFO:\n";
			print "  INFO: Derived OEM=${oem} from -builddir value\n";
			print "  INFO:\n";
		}
	}
}

require  "flush.pl";
use       Getopt::Long;
use       Env;
use       File::Basename;

if (-f "C:/tools/build/wl_swengg_lib.pl") {
	$TOOLSDIRUNIX = "C:/tools";
} elsif (-f "D:/tools/build/wl_swengg_lib.pl") {
	$TOOLSDIRUNIX = "D:/tools";
} elsif (-f "E:/tools/build/wl_swengg_lib.pl") {
	$TOOLSDIRUNIX = "E:/tools";
} elsif (-f "Z:/home/hwnbuild/src/tools/build/wl_swengg_lib.pl") {
	$TOOLSDIRUNIX = "Z:/home/hwnbuild/src/tools";
}

print "\n";

use lib  "${TOOLSDIRUNIX}/build";
push(@INC,"${TOOLSDIRUNIX}/build");

$FIND = "/bin/find"; # Use cygwin find instead of native find from windows

require "wl_swengg_lib.pl";

$TOOLSDIRSRV  = "Z:/projects/hnd_tools/win";
$ISUTILS_DIR  = "Z:/projects/hnd_software/gallery/src/tools/install/app/installshield/utils";
$ismanifest   = "${ISUTILS_DIR}/IsReMan.exe /manifest=${ISUTILS_DIR}/SetupExe.Admin.manifest";

# Some systems don't have enough disk-space on C: drive, so some tools
# have to be moved to D: drive! So check if a required tool has moved
# TODO: We need check for per tool in wl_swengg_lib.pl
if ( -d "C:/tools/verisign" ) {
	$verisigndir="C:/tools/verisign";
} elsif ( -d "D:/tools/verisign" ) {
	$verisigndir="D:/tools/verisign";
} else {
	$verisigndir="${TOOLSDIRSRV}/verisign";
}
$signcert="${verisigndir}/mycredentials.pfx";
$signtool="${RETRYCMD} ${verisigndir}/SignTool.exe";

# Default installshield version for packaging is IS 2009
# Locate release packager utility on local drive followed by Z: drive
if ($isver =~ /IS12/) {
   $releasepkgdir  = "C:/tools/InstallShield12SAB";
   $releasepkgdir  = "${TOOLSDIRSRV}/InstallShield12SAB" if ( ! -d "$releasepkgdir" );
} else {
   $releasepkgdir  = "C:/tools/InstallShield2009SAB";
   $releasepkgdir  = "${TOOLSDIRSRV}/InstallShield2009SAB" if ( ! -d "$releasepkgdir" );
}
$releasepkgr  = "${releasepkgdir}/System/ReleasePackager.exe";

# Use tree cmd to generate contents list
$treecmd      = "${TOOLSDIRUNIX}/utils/tree";
# REPKG_DIR is repackaging work-area, where repackaging happens
$REPKG_DIR    = "c:/temp/repkg"; `mkdir -pv $REPKG_DIR`;

# Command line options supported
# -builddir is mandatory, others are optional or are derived from -builddir
%scan_args = (
	'builddir=s'   => \$builddir, 	# -blddir is for backward compatibility
	'catdir=s'     => \$driverdir, 	# -catdir is for backward compatibility
	'customsrc=s'  => \$customsrc, 	# Custom file to copy
	'customdest=s' => \$customdest, # Path in package for custom file
	'dbg'          => \$dbg,    	# Enable debug mode
	'dellenc=s'    => \$dellenc,	# Dellenc file for dell packaging
	'driverdir=s'  => \$driverdir, 	# Directory containing driver bits and whql cats
	'help'         => \$help,   	# Show help and exit
	'installer=s'  => \$isver,  	# Choose either IS12 or IS2009
	'interactive'  => \$interactive,# After extracting pause for manual
					# or custom tweak to pkg contents!
	'oem=s'        => \$oem,    	# Which oem this repackaging needed for
	'pkgregions'   => \$pkgregions, # For Dell, do region based packaging
	'pkgversion=s' => \$pkgversion, # Location to package version dll
	'pkgtype=s'    => \$pkgtype,    # Type of package (Gold, TestCat, Drop, PreWHQL etc.,)
	'skipdriver'   => \$skipdriver, # Re-use driver from $builddir
	'skipvwldriver'=> \$skipvwldriver, # Re-use VWL driver from $builddir
	'treeprefix=s' => \$treeprefix, # Contents list filename
	'verbose'      => \$dbg,    	# Same as -dbg
	'virtualdir=s' => \$vwldir, 	# Folder containing virtual driver (whql'd) bits
	'winos=s'      => \$winos,  	# Windows os to package for
);

# Scan the command line arguments
GetOptions(%scan_args) or Exit($ERROR);

$version = '$Id$';
&Info("Program Version: $version");


# Following paths are prefixed with Z:\projects\hnd_svt_certs\WHQL\Submission
%other_whql_drivers = (
	"5.60.48"  => {
			vista => {
				vwl   => "BASS_REL_5_60_48_35/VWL/Vista/whql",
			},
			win7 => {
				vwl   => "BASS_REL_5_60_48_35/VWL/Win7/whql",
			},
			win8 => {
				vwl   => "BASS_REL_5_60_48_35/VWL/Win8/whql",
			},
	},
	"5.60.350"  => {
			vista => {
				vwl   => "BASS_REL_5_60_48_35/VWL/Vista/whql",
			},
			win7 => {
				vwl   => "BASS_REL_5_60_48_35/VWL/Win7/whql",
			},
			win8 => {
				vwl   => "BASS_REL_5_60_48_35/VWL/Win8/whql",
			},
	},
	"5.100.9"  => {
			vista => {
				vwl   => "KIRIN_REL_5_100_9_14/VWL/Vista/whql",
			},
			win7 => {
				vwl   => "BASS_REL_5_60_48_35/VWL/Win7/whql",
			},
			win8 => {
				vwl   => "BASS_REL_5_60_48_35/VWL/Win8/whql",
			},
	},
);

# List of branches or releases that need vwl driver is derived
# from above data-structure
foreach my $tag (sort keys %other_whql_drivers) {
	foreach my $os (sort keys %{$other_whql_drivers{$tag}}) {
		if (exists $other_whql_drivers{$tag}{$os}{vwl}) {
			push(@need_vwl,$tag);
		}
	}
}
&Dbg("Need VWL Driver Tags/Branches=@need_vwl");

# If -driverdir isn't specified, it is implicit the driver comes from the
# same package as rest of build.
# If -driverdir is specified, then don't skip driver copy step
# So if skipdriver cmd line switch isn't set
# - Set it to "no" if -driverdir is specified
# - Rename 1=>yes 0=>no
$skipdriver = ($driverdir) ? "no" : "yes" unless ($skipdriver);
$skipdriver =~ s/1/yes/;
$skipdriver =~ s/0/no/;

# If -virtualdir isn't specified, it is implicit the virtual driver comes
# from the same package as rest of build.
# If -virtualdir is specified, then don't skip virtual driver copy step
# So if skipvwldriver cmd line switch isn't set
# - Set it to "no" if -virtualdir is specified
# - Rename 1=>yes 0=>no
#$skipvwldriver= ($vwldir) ? "no" : "yes" unless ($skipvwldriver);
$skipvwldriver = "no" unless ($skipvwldriver);
$skipvwldriver =~ s/1/yes/;
$skipvwldriver =~ s/0/no/;

if (! -s "$FIND") {
	&Error3("Can't find 'find.exe'!");
	&Exit($ERROR) unless $help;
}

$date_suffix  = `sh -c "date '+%Y_%m_%d'"`;      chomp($date_suffix);

# Most common scenario is to mark a new package as "Gold", but can be
# overriden with -pkgtype on command line (e.g
$pkgtype = "Gold" unless ($pkgtype);

$winos  = lc($winos); $winos =~ s/\s+//g;

# Interpret WinXP package dirname as xp (similarly for others)
%winos_alias = (
	winxp		=> xp,
	winvista	=> vista,
	win7		=> win7,
	win8		=> win8,
);

# WinOS packaging driver dir alias
%winos_dir_alias = (
	winxp		=> WinXP,
	winvista	=> WinVista,
	win7		=> Win7,
	win8		=> Win8,
	xp		=> WinXP,
	vista		=> WinVista,
	7		=> Win7,
	8		=> Win8,
);

$oem    = lc($oem);   $oem   =~ s/\s+//g;

# Common OEM aliases. Main oem is still either bcm or hp or dell
# but they can repckaged and given to retail vendors (like acer/samsung)
%oem_alias = (
	acer		=> bcm,
	asus		=> bcm,
	bcm		=> bcm,
	dell		=> dell,
	delllowtouch	=> dell,
	hp		=> hp,
	hpdesktop	=> hp,
	lenovo		=> bcm,
	nec		=> bcm,
	samsung		=> bcm,
	sony		=> bcm,
	toshiba		=> bcm,
);

# If a random -oem value is specified that is not recognized by known
# real oem or oem_aliases, then confirm with user
if (!exists $oem_alias{$oem} && !$help) {
	&Warn3("oem = '$oem' specified isn't a commonly known oem vendor");
	$rc = &askUser("Do you want to continue with oem '$oem' packaging",1);
	&Exit($ERROR) if ($rc =~ /no|quit/);
	# Push new oem onto %oem_alias structure
	$oem_alias{$oem} = "bcm";
}

# OEM package prefix
%oem_pkg_alias = (
	bcm	=> Bcm,
	dell	=> Dell,
	hp	=> HP,
);

if (!exists $oem_pkg_alias{$oem}) {
	$oem_pkg_alias{$oem} = ucfirst($oem);
}

# Package directory derived from oem name, type of package and date
$oem_dest{$oem} = "$oem_pkg_alias{$oem}_${pkgtype}_${date_suffix}";

# Additional oem specific extra driver files. Right now they are same,
# but they may change per oem
%oem_extra_xp_driver_files = (
	bcm  =>
	[qw(
		bcmwlcoi.dll
		bcmwlcoi64.dll
	)],
	dell =>
	[qw(
		bcmwlcoi.dll
		bcmwlcoi64.dll
	)],
	hp   =>
	[qw(
		bcmwlcoi.dll
		bcmwlcoi64.dll
	)],
);

# WinXP Driver contents, that must be present in WHQL cat dirs
%xp_driver_files = (
	nic =>
	[qw(
		bcm43xx.cat
		bcm43xx64.cat
		bcmwl5.inf
		bcmwl5.sys
		bcmwl564.sys
	),
	(
		@{$oem_extra_xp_driver_files{$oem}}
	)],
	bmac =>
	[qw(
		bcmh43xx.cat
		bcmh43xx64.cat
		bcmwlhigh5.inf
		bcmwlhigh5.sys
		bcmwlhigh564.sys
	),
	(
		@{$oem_extra_xp_driver_files{$oem}}
	)],
);

# Additional oem specific extra driver files
%oem_extra_vista_driver_files = (
	bcm  => [qw()],
	dell => [qw()],
	hp   => [qw()],
);

# WinVista and Win7 Driver contents, that must be present in WHQL cat dirs
%vista_driver_files = (
	nic =>
	[qw(
		bcm43xx.cat
		bcm43xx64.cat
		bcmihvsrv.dll
		bcmihvsrv64.dll
		bcmihvui.dll
		bcmihvui64.dll
		bcmwl6.inf
		bcmwl6.sys
		bcmwl664.sys
		bcmwlcoi.dll
		bcmwlcoi64.dll
		bcmwl63.inf
		bcmwl63a.inf
		bcmwl63.sys
		bcmwl63a.sys
	),
	(
		@{$oem_extra_vista_driver_files{$oem}}
	)],
	bmac =>
	[qw(
		WdfCoInstaller01009.dll
		WdfCoInstaller0100964.dll
		bcmh43xx.cat
		bcmh43xx64.cat
		bcmihvsrv.dll
		bcmihvsrv64.dll
		bcmihvui.dll
		bcmihvui64.dll
		bcmwlcoi.dll
		bcmwlcoi64.dll
		bcmwlhigh6.inf
		bcmwlhigh6.sys
		bcmwlhigh664.sys
	),
	(
		@{$oem_extra_vista_driver_files{$oem}}
	)],
);

%vwl_driver_files = (
	nic =>
	[qw(
		README.txt
		bcmvwl.inf
		bcmvwl32.cat
		bcmvwl32.sys
		bcmvwl64.cat
		bcmvwl64.sys
	)],
);
		
		
# Windows 7 inherits most of the config/things from vista
%win7_driver_files = %vista_driver_files;

# WinVista and Win7 Driver contents, that must be present in WHQL cat dirs
%win8_driver_files = (
	nic =>
	[qw(
		bcm43xx.cat
		bcm43xx64.cat
		bcmihvsrv.dll
		bcmihvsrv64.dll
		bcmihvui.dll
		bcmihvui64.dll
		bcmwl63.inf
		bcmwl63.sys
		bcmwl63a.sys
	),
	(
		@{$oem_extra_vista_driver_files{$oem}}
	)],
	bmac =>
	[qw(
		bcmh43xx.cat
		bcmh43xx64.cat
		bcmihvsrv.dll
		bcmihvsrv64.dll
		bcmihvui.dll
		bcmihvui64.dll
		bcmwlhigh63.inf
		bcmwlhigh63.sys
		bcmwlhigh63a.sys
	),
	(
		@{$oem_extra_vista_driver_files{$oem}}
	)],
);

%vwl_driver_files = (
	nic =>
	[qw(
		README.txt
		bcmvwl.inf
		bcmvwl32.cat
		bcmvwl32.sys
		bcmvwl64.cat
		bcmvwl64.sys
	)],
);
	
# List of regions dell packages support (multi-sku)
# This used to be old Dell packaging style in early 2010 and before
@dellregions  = ("US","JPN","ROW");
# Name of dell system-id file
$dellencfile  = "DellInst.enc";
# Name of package version dll
$pkgverfile   = "PackageVersion.dll";

# Validate input parameters and exit if there are any issues
if (&validateCmdArgs) {
	&Exit($ERROR);
}

# We can only repackage InstallShield or InstallShield_Driver
# InstallShield_BMac or InstallShield_Driver_BMac.
# pkgname is one of these package variant
$pkgname = qx(basename $builddir); chomp($pkgname);


# Unique logfile for each repackaging session. This needs to be
# preserved for debugging or to trace the roots of different
# contents
$logf    = "${REPKG_DIR}/repkg_${oem}_${winos}_${pkgname}_${date_suffix}.log";

# If a logfile from previous session exists (very unlikely), rename it
# instead of clobbering it
if ( -f "$logf" ) {
   my $ts =  qx(date '+%H_%M_%S');
   my $logfold =  "$logf";

   $logfold =~ s/\.log/old_${ts}.log/;
   &Warn("Logfile with name: $logf already exists");
   &Warn("Renaming it to $logfold");
   rename("$logf","$logfold");
}

&Info3("All detailed log messages redirected to $logf");
&BeginLogging("$logf");
		
# Validate cmd line args
sub validateCmdArgs
{
	my ($error_flag) = 0;

	&Info("Validating command line arguments");

	if ($help) {
		&Usage($help);
		&Exit($OK);
	}

	# -builddir is mandatory (that points to previous InstallShield* pkg)
	if (!$builddir) {
		&Error3("-builddir <build-dir> needs to be specified");
		&Usage();
		$error_flag=1;
	} else {
		#$builddir =~ s/\s+//g;
		$builddir =~ s%\\%/%g;
	}

	# Derive oem name from builddir if not specified
	if (!$oem && $builddir) {
		($oem) = ($builddir =~ m%[/\\](dell|hp|bcm)%i);
		$oem = lc($oem);
		&Info3("Derived OEM=${oem} from -builddir option");
	}

	# One final check (may be redundant) to validate oem against oem_alias
	if (!grep {/^${oem}$/} keys %oem_alias) {
		&Error("oem=$oem is not recognized");
		&Error("Don't know how to re-package for oem=$oem");
		&Error("You can specify '-oem <oem>' cmd line switch");
		&Error("Valid oem values are: bcm,dell,hp");
		&Usage();
		$error_flag=1;
	}

	if ($dellenc && ($oem !~ /dell/i)) {
		&Error("oem=$oem is invalid when -dellenc is specified");
		$error_flag=1;
	} elsif ((-s "$dellenc") && ($oem =~ /dell/i)) {
		&Info("dellenc file $dellenc found");
	}

	# Validate existance of specified PackageVersion.dll file
	$pkgversion =~ s%\\%/%g if ($pkgversion);

	# HP needs PackageVersion.dll that HP gives us
	if (($oem =~ /hp/) && (! -s "$pkgversion")) {
		&Error("OEM=$oem needs a custom PackageVersion.dll file");
		$error_flag=1;
	}

	if ($pkgversion && ( ! -s "$pkgversion")) {
		&Error("Specified PackageVersion file '$pkgversion' empty or missing");
		$error_flag=1;
	}

	# Derive winos name from builddir
	if (!$winos && $builddir) {
		($winos) = ($builddir =~ m%/(WinVista|WinXP|Win7|Win8)/%i);
		$winos = lc($winos);
		$winos = $winos_alias{$winos} if (exists $winos_alias{$winos});
		&Info3("Derived winos=${winos} from -builddir option");
	}

	if (!$winos || !(grep {/$winos/} values %winos_alias)) {
		&Error("winos=winos is invalid. Valid values are xp,vista,win7,win8");
		&Error("You can specify '-winos <win os>' cmd line switch");
		&Usage();
		$error_flag=1;
	}

	if ($hostenv !~ /win32/) {
		&Error3("$0 needs to be run windows system");
		$error_flag=1;
	}

	if ($USER ne "hwnbuild") {
		&Error3("Login as hwnbuild and retry");
		$error_flag=1;
	}

	if ("$builddir" eq "$driverdir") {
		&Error3("builddir=$builddir and driverdir=$driverdir CAN NOT BE SAME!");
		$error_flag=1;
	}

	if ($driverdir && (! -d "$driverdir")) {
		&Error3("Specified driverdir=$driverdir is not found or doesn't exist");
		$error_flag=1;
	}

	if ($vwldir && (! -d "$vwldir")) {
		&Error3("Specified virtualdir=$vwldir is not found or doesn't exist");
		$error_flag=1;
	}

	if ($blddir && (! -d "$blddir")) {
		&Error3("Specified builddir=$blddir is not found or doesn't exist");
		$error_flag=1;
	}

	# We need virtual driver WHQL cat files for
	# @need_vwl builds for vista/win7 for all OEM brands
	if ($driverdir) {
		#$driverdir =~ s/\s+//g;
		$driverdir =~ s%\\%/%g;
		# For BASS twig builds, we need Virtual driver also merged
		my $need_vwl_regex = join('|', @need_vwl);
		if ($driverdir =~ /($need_vwl_regex)/) {
			&Info("Checking if virtual driver is needed");
			&Info("(for winos=$winos;oem=$oem;virtualdir=$vwldir;skipvwl=$skipvwldriver)");
			# But only for win7 or vista and bcm or dell oems
			if ($winos =~ /vista|win7|win8/) {
				# only if virtual dir isn't specified
				# or skipvwldriver cmd line switch isn't set
				if (!$vwldir && ($skipvwldriver eq "no")) {
					&Error("");
					&Error("This merge needs you to specify directory");
					&Error("containing virtual driver WHQL cat files");
					&Error("with -virtualdir <virtual-driver-dir> cmd line switch");
					&Error("");
					&Info3("If you don't want virtual driver, pass -skipvwldriver cmd line switch");
					&Exit($ERROR);
				} else {
					&Info("Virtual driver is needed");
				}
			} else {
				&Info("Virtual driver is optional");
			}
		}
	}

	if ( "$customsrc" && ! -f "$customsrc" ) {
		&Error("Specified custom file doesn't exist");
		&Error("at: $customsrc");
		&Exit($ERROR);
	}
	$customsrc =~ s%\\%/%g;

	if ( -f "$customsrc" && ! "$customdest" ) {
		&Warn("Destination file name not specified to copy $customsrc");
		$customdest = basename($customsrc);
		&Warn("It will be copied as $customdest in the package");
	}
	$customdest =~ s%\\%/%g;

	if ((grep { m/_tree.txt/gi } @bldfiles) && ($treeprefix)) {
		&Warn("'$treelist' will override '${treeprefix}_tree.txt'");
	}

	return($error_flag);

} # validateCmdArgs()

#
# copyFromBuild ()
# Copy all files from InstallShield folder (Setup.exe etc.,)
#
sub copyFromBuild
{
	if (! $setupexe) {
		&Error("No Setup.exe found in $builddir");
		&Exit($ERROR);
	}

	if (! $treelist) {
		&Error("No previous tree file found in $builddir");
		&Error("Specify '-treeprefix <some-tree-prefix>' cmd line switch");
		&Exit($ERROR);
	}
	# Correct tree filenames if needed
	if ($winos =~ /vista/) { $oldtreelist=$treelist; $treelist =~ s/Win7/Vista/i };
	if ($winos =~ /win7/) { $oldtreelist=$treelist; $treelist =~ s/Vista/Win7/i };
	if ($winos =~ /win8/) { $oldtreelist=$treelist; $treelist =~ s/Vista/Win8/i };

	&Info("Removing previous local/temp repackaging dirs:");
	&Cmd("rm -rf $setup_dir");
	&Cmd("rm -rf $extract_dir");
	&Cmd("rm -rf $repkg_dir");
	if (-d "$setup_dir" || -d "$extract_dir" || -d "$repkg_dir") {
		&Error3("Previous temporary directories couldn't be deleted");
		&Exit($ERROR);
	}
	&Info("Copying build contents from $builddir into $setup_dir",$logf);
	&CmdQL("mkdir -p $setup_dir",$logf);
	&CmdQL("cp -pv $builddir/* $setup_dir",$logf);
	$origexe = (stat("$setup_dir/$setupexe"))[7]/1000000;

} # copyFromBuild

#
# extractFromBuild ()
# Unzip/extract Setup.exe
#
sub extractFromBuild
{
	chdir($setup_dir);
	&Info("Extracting from $setupexe");
	&CmdQL("mkdir -pv $extract_dir",$logf);
	&CmdQL("cmd /c '$setupexe -extract_all:$extract_dir'",$logf);
	chdir($extract_dir);
	$origbom = qx($FIND Disk1 -type f -print | wc -l); chomp($origbom);
	&CmdQL("$treecmd -a -s -D -f Disk1",$logf);
	&CmdQL("rm -f $extract_dir/$oldtreelist $extract_dir/$treelist $extract_dir/$setupexe",$logf);

} # extractFromBuild()

#
# copyDellEnc ()
# For Dell OEM, if -dellenc is specified, copy it over
#
sub copyDellEnc
{
	chdir($extract_dir);
	if ($oem =~ /dell/i) {
		if (-s "$dellenc" ) {
			&Info("Copying $dellenc");
			&CmdQL("rm -fv Disk1/$dellencfile",$logf);
			&CmdQL("cp -pv '$dellenc' Disk1/$dellencfile",$logf);
			if (! -s "Disk1/$dellencfile") {
				&Error("Failed to copy '$dellenc' to 'Disk1/$dellencfile'");
				&Exit($ERROR);
			}
		}
	}
} # copyDellEnc()

#
# updateDellProperty ()
# For Dell OEM, change Setup.ini property PLATFORM from Win7 to Vista
#
sub updateDellProperty
{
	return unless (($winos =~ /vista/i) && ($oem =~ /dell/i));

	chdir("${extract_dir}/Disk1");

	# If this routine is called directly, release packager may
	# be unpacking the contents from Setup.exe still. So wait few secs
	while ( -z "Setup.ini") {
		&Warn("Extracted Setup.ini is zero size. Wait 30sec");
		sleep(30);
	}

	open(SETUPINI,"<Setup.ini") || die "ERROR: Can't read Setup.ini";
	open(SETUPINI_OUT,">Setup_Vista.ini") || \
		die "ERROR: Can't write to Setup.ini";

	while(<SETUPINI>) {
		if (/PLATFORM\s*=\s*Windows7/) {
			&Info("Resetting PLATFORM=Vista in Setup.ini");
			s/Windows7/Vista/g;
		}
		print SETUPINI_OUT "$_";
	}

	close(SETUPINI_OUT);
	close(SETUPINI);

   	rename("Setup_Vista.ini","Setup.ini");

	if ( -z "Setup.ini") {
		&Error3("Generated Setup.ini has zero size");
		&Exit($ERROR);
	}
	&CmdQL("unix2dos Setup.ini");

} # updateDellProperty()

#
# copyPackageVersion ()
# If PackageVersion.dll is provided, include it in new package
#
sub copyPackageVersion
{
	chdir($extract_dir);

	$pkgversion =~ s%\\%/%g;

	if (-s "$pkgversion" ) {
		&Info("Copying $pkgversion");
		&CmdQL("rm -fv Disk1/$pkgverfile",$logf);
		&CmdQL("cp -pv '$pkgversion' Disk1/$pkgverfile",$logf);
		if (! -s "Disk1/$pkgverfile") {
			&Error("Failed to copy '$pkgversion' to 'Disk1/$pkgverfile'");
			&Exit($ERROR);
		}
	} else {
		&Error3("Specified $pkgverfile at '$pkgversion' isn't found");
		&Exit($ERROR);
	}

} # copyPackageVersion()

#
# copyWLDriver ()
# If an alterate driver is specified with -driverdir, then copy
# driver bits
#
sub copyWLDriver
{
	my $driver_files_copied=0;
	my $driver_files_to_copy=0;
	my $indir; # input path to copy driver from
	my $outdir;# Output path to copy driver to

	chdir($extract_dir);
	&Info("Copying WHQL driver files to $extract_dir");

	# If pkgregions is set, allow for region specific packaging
	# for Dell PC-OEM customer. This is needed for older packages
	# in early 2010 and 2009 Dell releases
	if (($oem =~ /dell/) && $pkgregions) {
		foreach $region (@dellregions) {
			$driver_files_copied=0;
			$driver_files_to_copy=scalar(@{$driver_files{$drvtype}});
			&Info("Driver files to copy [$region] [$driver_files_to_copy]: @{$driver_files{$drvtype}}");

			$indir="$driverdir/DRIVER_$region";
			$outdir="Disk1/DRIVER_$region";

			foreach $drvfile (@{$driver_files{$drvtype}}) {
				if (-f "$indir/$drvfile") {
					&CmdQL("rm -fv $outdir/$drvfile",$logf);
					&CmdQL("cp -p '$indir/$drvfile' $outdir/$drvfile",$logf);
					if (! -s "$outdir/$drvfile") {
						&Error("Failed to copy '$indir/$drvfile' to '$outdir/$drvfile'");
						&Exit($ERROR);
					} else {
						$driver_files_copied++;
					}
				} else {
					&Error("Missing cat file dir '$indir/$drvfile'");
					&Exit($ERROR);
				}
			} # foreach
			# Ensure that number of driver files copied match required list
			if ( $driver_files_to_copy != $driver_files_copied ) {
				&Error("Copied only $driver_files_copied of ${driver_files_to_copy} driver files");
				&Exit($ERROR);
			}
		} # foreach
	} else {
		$driver_files_to_copy=scalar(@{$driver_files{$drvtype}});
		&Info("Driver files to copy [$driver_files_to_copy]: @{$driver_files{$drvtype}}");
		$driver_files_copied=0;

		$indir="$driverdir";

		# Newer PC-OEM packaging has new driver directory structure
		# Look if that structure exists and make packaging compatible
		# with old and new structures
		if ( -d "Disk1/Drivers/$winos_dir_alias{$winos}/WL" ) {
			$outdir="Disk1/Drivers/$winos_dir_alias{$winos}/WL";
		} else {
			$outdir="Disk1";
		}

		foreach $drvfile (@{$driver_files{$drvtype}}) {
			if (-f "$indir/$drvfile") {
				&CmdQL("rm -fv $outdir/$drvfile",$logf);
				&CmdQL("cp -pv '$indir/$drvfile' $outdir/$drvfile",$logf);
				if (! -s "$outdir/$drvfile") {
					&Error("Failed to copy '$indir/$drvfile' to '$outdir/$drvfile'");
					&Exit($ERROR);
				} else {
					$driver_files_copied++;
				}
			} else {
				&Error("Missing drvfile file '$indir/$drvfile'");
				&Exit($ERROR);
			}
		} # foreach
		# Ensure that number of driver files copied match required list
		if ( $driver_files_to_copy != $driver_files_copied ) {
			&Error("Copied only $driver_files_copied of $driver_files_to_copy driver files");
			&Exit($ERROR);
		}
	}

	# If we add any new oem, ensure that list of driver files are defined
	if ( $driver_files_copied == 0 ) {
		&Error("No driver files found to copy for oem=$oem");
		&Exit($ERROR);
	}
	
} # copyWLDriver()


sub copyOtherDriver
{
	my ($drvtype,$drvdir,@drvfiles)=@_;
	my $indir; # input path to copy driver from
	my $outdir;# Output path to copy driver to

	$indir = "$drvdir";
	# Newer PC-OEM packaging has new driver directory structure
	# Look if that structure exists and make packaging compatible
	# with old and new structures
	if ( -d "Disk1/Drivers/$winos_dir_alias{$winos}/$drvtype" ) {
		$outdir="Disk1/Drivers/$winos_dir_alias{$winos}/$drvtype";
	} else {
		$outdir="Disk1/$drvtype";
	}

	if (! -d "$indir") {
		&Info3("$drvtype driver dir not found or not needed for this step");
		sleep 3;
		return;
	}

	chdir($extract_dir);
	&Info("Copying WHQL $drvtype driver files to $extract_dir");

	foreach my $drvfile (@drvfiles) {
		if (-f "$indir/$drvfile") {
			&CmdQL("rm -fv $outdir/$drvfile",$logf);
			&CmdQL("cp -pv '$indir/$drvfile' $outdir/$drvfile",$logf);
			if (! -s "$outdir/$drvfile") {
				&Error("Failed to copy '$indir/$drvfile' to '$outdir/$drvfile'");
				&Exit($ERROR);
			}
		} else {
			&Error("Missing $drvtype driver file '$indir/$drvfile'");
			&Exit($ERROR);
		}
	} # foreach
} # copyOtherDriver ()

#
# repkgNow ()
# Now the new merged dir is ready to be zipped back again and signed
#
sub repkgNow
{
	my ($mytreelist) = "$treelist";
	my $ec;

	&Info("Re-packaging into single $setupexe");
	chdir($extract_dir);

	if ( -f "$customsrc" ) {
		&Info("Copying custom file $customsrc into $extract_dir/Disk1/$customdest");
		&CmdQL("cp -pv '$customsrc' Disk1/$customdest",$logf);
	}

	&CmdQL("mkdir -p $repkg_dir",$logf);
	$ec = &CmdQL("cmd /c '$releasepkgr $extract_dir_dos\\Disk1 $repkg_dir_dos\\$setupexe'",$logf);
	&Info("Release Packager exit code: $ec");
	if ( "$ec" != "0" ) {
		&Error("Release Packager exited with non-zero error code");
		&Error("Check $logf for any errors. Removing bad Setup.exe");
		&CmdQL("rm -fv $repkg_dir/$setupexe",$logf);
		&Exit($ERROR);
	}

	$ec = &CmdQL("cmd /c '$ismanifest $repkg_dir/$setupexe'",$logf);
	&Info("Setup.exe manifest step exit code: $ec");
	if ( "$ec" != "0" ) {
		&Error("Setup.exe manifesting failed");
		&Error("Check $logf for any errors. Removing bad Setup.exe");
		&CmdQL("rm -fv $repkg_dir/$setupexe",$logf);
		&Exit($ERROR);
	}
	&CmdQL("rm -f $repkg_dir/Setup.exe.bak",$logf);

	foreach $otherf (@bldfiles) {
		next if ($otherf =~ /$treelist/);
		next if ($otherf =~ /$setupexe/);
		&CmdQL("cp -p $setup_dir/$otherf $repkg_dir/",$logf);
	}

	$newexe = (stat("$repkg_dir/$setupexe"))[7]/1000000;
	$newbom = qx($FIND Disk1 -type f -print | wc -l); chomp($newbom);

	&Info("Signing $setupexe");
	&CmdQL("$signtool sign /f $signcert /p brcm1 /t 'http://timestamp.verisign.com/scripts/timstamp.dll' $repkg_dir/$setupexe",$logf);

	&Info("Generating treelist");
	if ($mytreelist !~ /_repkg/) {
		$mytreelist =~ s/_tree\.txt/_repkg1_tree.txt/;
	} else {
		$mytreelist =~ m/_repkg(\d+)_tree.txt/g;
		$mytreeiter =  $1;
		$mytreeitern=  $1 + 1;
		$mytreelist =~ s/_repkg${mytreeiter}_/_repkg${mytreeitern}_/;
	}
	&CmdQL("$treecmd -a -s -D -f -o $repkg_dir/$mytreelist Disk1",$logf);
	`echo ''                                          >> $repkg_dir/$mytreelist`;
	`echo '	NOTE: To unpack with no install run:' >> $repkg_dir/$mytreelist`;
	`echo '	       Setup.exe -extract_all:<path>' >> $repkg_dir/$mytreelist`;
	&CmdQL("unix2dos $repkg_dir/$mytreelist",$logf);
	&Info("Verifying the newly repackaged $setupexe");
	&Info("Orig $setupexe size : $origexe Mbytes");
	&Info("New  $setupexe size : $newexe Mbytes");

	if ( $origbom != $newbom) {
		&Warn("Original number of files in package: $origbom do NOT match");
		&Warn("New number of files in newly created package: $newbom");
		&Warn("");
		&Warn("Please validate repackage logfile and new package contents");
		&Warn("");
		&Info("Detailed repackaging logfile is at: $logf");
		&Info("Repackaged installer is at: $repkg_dir");
		&Warn("");
		&Warn("Hit enter to continue");
		$ans = <STDIN>; chomp($ans);
	} else {
		&Info("Orig number of files: $origbom");
		&Info("New  number of files: $newbom");
	}

	chdir($repkg_dir);

} # repkgNow()

## Show the script help text
sub Usage
{
	my($help)=shift;

	print "\nUsage:\n\n";
	print "$0 \\
	-builddir  	(required) Build to use for tray and installer
	-driverdir 	(optional) Build to use for driver
	-help      	(optional) Show help text
	-installer 	(optional) Which installshield version to use
	-interactive	(optional) Interactively perform packaging steps to allow
			manual package contents tweaking (if needed)
	-oem		(optional) One of <hp|dell|bcm|nec|asus|samsung|acer|lenovo|sony|toshiba>
	-pkgversion	(optional) Specify custom PackageVersion.dll to merge
			NOTE: Derived from -builddir if not specified
	-pkgregions	(optional) If dell needs region specific multi-sku
			use this option
	-skipdriver	(optional) Skip driver bits merge, reuse existing
	-skipvwldriver	(optional) Skip vwldriver bits merge, reuse existing
	-treepfx	(optional) Filename prefix for the package contents
	-virtualdir 	(optional) Folder containing virtual driver files
	-winos		(optiona)  One of <xp|vista|win7>]
			NOTE: Derived from -builddir if not specified
	\n\n";

	return if (!$help);
	print "
    Examples:
    --------:
    1. Repackage Vista Dell installer (5.60.18.8) with WHQLd driver (5.60.18.8):
	 perl C:/tools/build/repkg_winbuild.pl -builddir \"Z:\\projects\\hnd\\swbuild\\build_window\\BASS_REL_5_60_18_8\\win_external_wl\\2009.8.21.0\\release\\Win7\\Dell\\Dell_InstallShield\" -driverdir \"Z:\\projects\\hnd_svt\\Certifications\\WHQL\\Submission\\BASS_REL_5_60_18_8\\43224\\vista-dell\\whql\" -winos vista

    2. Repackage Win7 Dell installer with WHQLd driver (custom tree list name):
	perl C:/tools/build/repkg_winbuild.pl -builddir \"Z:\\projects\\hnd\\swbuild\\build_window\\BASS_REL_5_60_18_8\\win_external_wl\\2009.8.21.0\\release\\Win7\\Dell\\Dell_InstallShield\" -driverdir \"Z:\\projects\\hnd_svt\\Certifications\\WHQL\\Submission\\BASS_REL_5_60_18_8\\43224\\vista-dell\\whql\" -tree Dell_Vista_Custom

    3. Repackage 5.60.18.9 driver with 5.60.18.21 trayapp/installer
	perl c:/tools/build/repkg_winbuild.pl -driverdir \"Z:\\projects\\hnd\\swbuild\\build_window\\BASS_REL_5_60_18_9\\win_external_wl\\2009.8.25.0\\release\\WinXP\\Dell\\Dell_DriverOnly\" -builddir \"Z:\\projects\\hnd\\swbuild\\build_window\\BASS_REL_5_60_18_21\\win_external_wl\\2009.9.24.0\\release\\WinXP\\Dell\\Dell_InstallShield\"

    4. Use old InstallShield 12 to repackage instead of default IS 2009 version (installshield developers must tell you when to use old installer):
	perl c:/tools/build/repkg_winbuild.pl -installer IS12 <other-options-as-shown-in-example-1-and-2-above>

    5. Package with a gold copy of dell .enc file:
	Copy new DellInst.enc to c:\\temp\\DellInst.enc
	perl c:/tools/build/repkg_winbuild.pl -dellenc \"c:/temp/DellInst.enc\" <other-options-as-shown-in-example-1-and-2-above>

    6. Package with WHQL Virtual driver files
	perl c:/tools/build/repkg_winbuild.pl -virtualdir \"<folder-containing-virtual-driver-bits>\" <other-options-as-shown-in-example-1-and-2-above>

    7. Interactively package (used for custom needs. Allows one to manually tweak the package contents with manual steps):
	perl c:/tools/build/repkg_winbuild.pl -interactive <other-options-as-shown-in-example-1-and-2-above>
    ";

} # Usage()

## In interactive mode, different steps pause to get confirmation from
## user (mainly to allow for manual/custom tweaking of package contents)
sub askUser {
	my ($question) = shift;
	my ($opt)      = shift;
	my ($rc);

	# return("yes") if ($help);

	if ( $interactive || $opt ) {
		&Echo("");
		&Echo("QUESTION> $question (default: yes)");
		print " [Yes|No|Quit|Skip]: ";
		$ans = <STDIN>; chomp($ans);
		$ans =~ s/\s+//g;
		if ($ans =~ /^s/i) {
			$rc="skip";
		} elsif ($ans =~ /^y/i) {
			$rc="yes";
		} elsif ($ans =~ /^n/i) {
			$rc="no";
		} elsif ($ans =~ /^q/i) {
			$rc="quit";
		}
	}
	return($rc);
} # askUser()

#
# createReadMe ()
# create a Sample Readme from specified command line options
#
sub createReadMe
{
	my ($readme) = shift;

	my ($appbuild)    = grep { /_REL_/} split(/\/|\\/, $builddir);
	my ($drvbuild)    = grep { /_REL_/} split(/\/|\\/, $driverdir);
	my ($vwldrvbuild) = grep { /_REL_/} split(/\/|\\/, $vwldir);

	# -driverdir is optional, so if no driver dir is specified
	# driver is re-used from application build itself in -builddir
	$drvbuild = $builddir if (! $appbuild);

	if ( -s "$readme" ) {
		open(README, ">>$readme") || &Warn3("$readme couldn't be updated");
		print README "============= EXTRA README ==============\n\n";
	} else {
		open(README, ">$readme") || &Warn3("$readme couldn't be created");
	}

	print README "Name : Repackage $appbuild app build with $drvbuild driver\n";
	print README "     : and with $vwldrvbuild virtual driver\n" if ($vwldir);
	print README "     : and with $drvbuild virtual driver\n" if (!$vwldir);
	print README "OEM  : $oem (from $oem_alias{$oem} package)\n";
	print README "Date : $date_suffix\n";
	print README "MO   : <managers who requested this>\n";
	print README "DO   : <developers whose fixes are in>\n";
	print README "SVT  : <who whqld>\n";
	print README "REPKG: $USER for <who repackaged>\n";
	print README "\n";
	print README "Synopsis:\n";
	print README "---------\n";
	print README "\n";
	print README "PR: NNNNN : <synopsis|subject>\n";
	print README "\n\n";
	print README "Description:\n";
	print README "-----------\n";
	print README "\n\n";
	print README "Solution Description:\n";
	print README "---------------------\n";
	print README "Packaged ".uc($winos)." $oem_dest{$oem}\n";
	print README "Packaged on: $thishost\n";
	print README "Package Log: $logf\n";
	print README "Package contents manually touched. Refer to above PR" if ($interactive);
	print README "Package Details: \n";
	print README "  App Tag   : $appbuild\n" 	if ($appbuild);
	print README "  Driver Tag: $drvbuild\n" 	if ($drvbuild);
	print README "  VWL Tag   : $vwldrvbuild\n" 	if ($vwldrvbuild);
	print README "  Build Dir : $builddir\n" 	if ($builddir);
	print README "  Driver Dir: $driverdir\n" 	if ($driverdir);
	print README "  VWL Dir   : $vwldir\n" 	        if ($vwldir);
	print README "  SkipDriver: $skipdriver\n" 	if ($skipdriver);
	print README "  SkipVirtualDriver: $skipvwldriver\n" 	if ($skipvwldriver);
	print README "  PatchSrc:   $customsrc\n" 	if ($customsrc);
	print README "  PatchDest:  $customdest\n" 	if ($customdest);
	print README "  DellEnc:    $dellenc\n" 	if ($dellenc);
	print README "  Installer:  $installer\n" 	if ($installer);
	print README "\n\n";
	print README "\n\n";

	# Since we repackage sometimes multiple times, keep history of old
	# readme files that may contain critical information
	my $oldreadme = "$builddir/../README.txt";
	if ( -s "$oldreadme" ) {
		print README "\n============= PREVIOUS READMES ==============\n";
		open(OLDREADME, "$oldreadme") || &Warn3("$oldreadme can't read");
		print README <OLDREADME>;
		close(OLDREADME);
	}
	close(README);
	&CmdQL("unix2dos $readme");

} # createReadMe()

#
# Script executation starts from Main function
# Main ( )
#
sub Main
{
	if ( $interactive ) {
		&Info("");
		&Info("Interactive packaging enabled");
		&Info("Answer Yes or No or Quit to prompted questions");
		sleep(5);
	}

	print "==============================================================\n";
	&Info("Performing PC-OEM re-packaging");
	&Info("Command Line options specified or derived:");
	&Info("===========================================");
	&Info(" builddir    = $builddir")	if ($builddir);
	&Info(" driverdir   = $driverdir")	if ($driverdir);
	&Info(" vwldir      = $vwldir")		if ($vwldir);
	&Info(" customsrc   = $customsrc")	if ($customsrc);
	&Info(" customdest  = $customdest")	if ($customdest);
	&Info(" installer   = $isver")		if ($isver);
	&Info(" interactive = $interactive")	if ($interactive);
	&Info(" oem         = $oem")		if ($oem);
	&Info(" dellenc     = $dellenc")	if ($dellenc);
	&Info(" winos       = $winos")		if ($winos);
	&Info(" treeprefix  = $treeprefix")	if ($treeprefix);
	&Info("===========================================");

	chdir($builddir) || die "Can't change dir to build-dir $builddir";

	$bldfiles  = qx($FIND * -maxdepth 0 -mindepth 0 -type f -print);

	chdir($REPKG_DIR);
	@bldfiles  = split(/\s+/,$bldfiles); &Dbg("bldfiles = $bldfiles");
	($treelist)= grep { m/_tree.txt/gi } @bldfiles;
	$treelist  = "${treeprefix}_tree.txt" if ($treeprefix && ! $treelist);
	$treelist  =~ s/vista/win7/i if ($winos =~ /win7/);
	$treelist  =~ s/vista/win8/i if ($winos =~ /win8/);
	($setupexe)= grep { m/etup.exe/gi  } @bldfiles;

	$setup_dir       = "${REPKG_DIR}/orig-setup-pcoem/$oem/$winos/$pkgname";
	$extract_dir     = "${REPKG_DIR}/extract-pcoem/$oem/$winos/$pkgname";
	$repkg_dir       = "${REPKG_DIR}/repkg-pcoem/$oem/$winos/$oem_dest{$oem}/$pkgname";
	$setup_dir_dos   =  $setup_dir;
	$setup_dir_dos   =~ s%/%\\%g;
	$extract_dir_dos =  $extract_dir;
	$extract_dir_dos =~ s%/%\\%g;
	$repkg_dir_dos   =  $repkg_dir;
	$repkg_dir_dos   =~ s%/%\\%g;

	$drvtype = ($builddir =~ /bmac/i) ? "bmac" : "nic";

	if ($winos =~ /xp/) {
		%driver_files = %xp_driver_files;
	} elsif ($winos =~ /vista/) {
		%driver_files = %vista_driver_files;
	} elsif ($winos =~ /win7/) {
		%driver_files = %win7_driver_files;
	} elsif ($winos =~ /win8/) {
		%driver_files = %win8_driver_files;
	}
	%driver_files = %${winos}_driver_files;

	# Copy Setup.exe from network dir to local packaging dir
	$rc=&askUser("Do you want to copy Setup.exe etc., from $builddir");
	&Exit($ERROR) if ($rc =~ /no|quit/);
	if ($rc =~ /skip/i) {
		&Info("Skipping copying of Setup.exe from $builddir");
	} else {
		&copyFromBuild();
	}
	
	# Extract from Setup.exe
	$rc=&askUser("Do you want to decompress Setup.exe");
	&Exit($ERROR) if ($rc =~ /no|quit/);
	if ($rc =~ /skip/i) {
		&Info("Skipping decompress of Setup.exe");
	} else {
		&extractFromBuild();
		&Info("Extracted Setup.exe contents are at:");
		&Info(" $extract_dir");
	}

	# Main WL driver copying
	$rc=&askUser("Do you want to copy driver+cat files from $driverdir");
	&Exit($ERROR) if ($rc =~ /no|quit/);
	if (($skipdriver =~ /yes/) || ($rc =~ /skip/i)) {
		&Warn("WL Driver bits will not be replaced");
		&Warn("They will come from $builddir");
		sleep(5);
	} else {
		&copyWLDriver();
	}

	# VWL driver copying
	if ($winos =~ /vista|win7|win8/) {
		$rc=&askUser("Do you want to copy VIRTUAL driver+cat files from $vwldir");
		&Exit($ERROR) if ($rc =~ /no|quit/);
		if (($skipvwldriver =~ /yes/) || ($rc =~ /skip/i)) {
			&Warn("Virtual Driver bits will not be replaced");
			&Warn("They will come from $builddir");
			sleep(5);
		} else {
			&copyOtherDriver(VWL,$vwldir,@{$vwl_driver_files{$drvtype}});
		}
	}

	# Copy Dell ENC/platform file
	&copyDellEnc() if ($oem =~ /dell/);
	&updateDellProperty() if (($oem =~ /dell/) && ($winos =~ /vista/));
	&copyPackageVersion() if ($pkgversion);

	if ($interactive) {
		print "\n\n";
		&Info("YOU CAN MANUALLY PATCH FILES IN $extract_dir NOW");
		&Info("WITH CUSTOM UPDATES/FILES");
		print "\n\n";
		sleep(5);
	}

	# All copying is done, compress and sign the new package now
	$rc=&askUser("Do you want to compress and sign everything back to Setup.exe");
	&Exit($ERROR) if ($rc =~ /no|quit/);
	if ($rc =~ /skip/i) {
		&Info("Skipping repackaging into setup.exe and signing");
		&Info("Nothing was repackaged and nothing needs to be copied");
		&Exit("$OK");
	} else {
		&repkgNow();
		&CmdQL("rm -rf $setup_dir",$logf);
		&CmdQL("rm -rf $extract_dir",$logf);
	}

	chdir($extract_dir);

	# Generate contents list
	&CmdQL("$treecmd -a -s -D -f Disk1",$logf);

	# Generate README from template. Old README if any from builddir is
	# appended to this README. If more than one sub-package is needed
	# (e.g <oem>_InstallShield and <oem>_InstallShield_Driver, then
	# both are concatanateb
	&createReadMe("$repkg_dir/../README.txt");

	print "\n";
	&Info("Detailed logfile is at: $logf");
	&Info("Repackaged installer is at: $repkg_dir");
	print "\n";
	&Info("You need to copy $repkg_dir as $oem_dest{$oem} on Z:");
	&Info("You need to copy $logf inside $oem_dest{$oem} on Z:");
	&Info("");

} # Main()

## Start of script
&Main();

chdir("$REPKG_DIR");
`unix2dos $logf > $NULL 2>&1`; sleep 2;
&EndLogging("$logf");

