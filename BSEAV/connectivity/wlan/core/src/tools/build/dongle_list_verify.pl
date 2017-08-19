#!/usr/bin/perl
#
# This script tracks list of firmware images in release makefiles
# and alerts branch owners if any required images aren't built
# by default in src/dongle/rte/wl/builds folder
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

# use strict; # Uncomment these after converting wl_swengg_lib to a pkg
# use vars;

use Env;
use English;
use Config;
use Getopt::Long;
use File::Path;
use File::Basename;
use Net::SMTP;

$| = 1; # Do not buffer output, spit out asap

##
## Command line arguments supported by this program
##
## branch    :   Branch to query for default firmware images
## brand     :   Use this pre-built branddir to run showtgts
## dbg       :   Enable debug mode
## help      :   Help/Usage info
## ccuser    :   list of users who volunteer to be notified in Cc: list
##

# Initialize
my $branch;          #
my $brand;           #
my $ccusers;         #
my $dbg=0;           #
my $help;            #
my $notify;          #
my $hndrte_brand;    #

my %scan_args = (
	'branch=s'      => \$branch,
	'hndrte_brand=s'=> \$hndrte_brand,
	'ccuser=s'      => \$ccusers,
	'dbg'           => \$dbg,
	'help'          => \$help,
	'mail=s'        => \$notify,
);

# Scan the command line arguments
GetOptions(%scan_args) or Error("GetOptions failed!");

use lib  "/home/hwnbuild/src/tools/build";
require  "wl_swengg_lib.pl";

Help() if ($help || !$branch);
Info3("Running Dongle image list scanner on $thishost for $branch on $nowtimestamp");

# The chipimages mapping below is to map chip image dir to chiprev
my %BranchOwners = (
	"KIRIN_BRANCH_5_100"	=> {
					owner => mzhu,
	},
	"BASS_BRANCH_5_60"	=> {
					owner => mzhu,
	},
	"FALCON_BRANCH_5_90"	=> {
					owner => csm,
	},
	"F16_BRANCH_5_80"	=> {
					owner => csm,
	},
	"F15_BRANCH_5_50"	=> {
					owner => kiranm,
	},
	"ROMTERM2_BRANCH_4_219"	=> {
					owner => hnd-lpsta-list,
	},
	"ROMTERM_BRANCH_4_218"	=> {
					owner => hnd-lpsta-list,
	},
	"RAPTOR2_BRANCH_4_217"	=> {
					owner => prakashd,
	},
	"ALL_BRANCHES"	=> {
					owner => hnd-software-scm-list,
	},
); # %BranchOwners

# Override owner to admin user in debug mode
$BranchOwners{$branch}{owner} = "prakashd" if ($dbg);

my @dbgusers    = qw(hnd-software-scm-list);

$ENV{PATH}      = "/bin:/usr/bin:/usr/local/bin:/tools/bin:/sbin:/usr/sbin:/projects/hnd/tools/linux/bin:$ENV{PATH}";

my $FIND        = "/tools/bin/find";
my $domain      = "broadcom.com";
#my $ccusers     = "hnd-software-scm-list";
my $ccusers     = "";
my $hndrte_brand= defined($hndrte_brand) ? $hndrte_brand : "hndrte-dongle-wl";

my $touch       = "/home/$BUILDOWNER/bin/newtouch";
my $build_linux = "/projects/hnd/swbuild/build_linux";
my $build_admin = "/projects/hnd/swbuild/build_admin";
my $show_fw_mk  = "/home/hwnbuild/src/tools/release/show-dongle-images.mk";
my %missing     = ();
my $TEMP        = "/tmp";
# Output report in html format (currently not used)
my $htmlfile    = qx(mktemp ${TEMP}/msg_${hndrte_brand}_XXXXXX); chomp($htmlfile);
# Output report in text format
my $textfile    = qx(mktemp ${TEMP}/msg_${hndrte_brand}_XXXXXX); chomp($textfile);
   $htmlfile   .= ".htm";
   $textfile   .= ".txt";

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

my $hndrte_dir  = qx($FIND $build_linux/$branch/$hndrte_brand -maxdepth 1 -mindepth 1 -type d -name "*${todayiter}*" 2> $NULL | sort | tail -1);
chomp($hndrte_dir);

my $dhd_brands  = qx(/home/hwnbuild/src/tools/build/build_config.sh -r $branch -q 2> $NULL);
chomp($dhd_brands);

&Info("Searching following dhd build brands for missing firmware images");
&Info("$dhd_brands");
print "\n\n";

##
## Help ( )
##
sub Help {

	Dbg("Run Help()");

	print "

	Usage: $0 -branch <branch> \
		[-dbg ] [ -help ] \
		[-hndrte_brand <hndrte_brand>] \
		[-mail <recipient1> -mail <recipient2> ...]

	Command line arguments supported by this program

	-branch     : Does ROML tracking on this branch (default: TOT)
	-hndrte_brand
                    : Name of build brand that generates firmware (default: hndrte-dongle-wl)
	-ccuser     : List of users to be notified for missing firmwares
	-dbg        : Enable debug mode
	-help       : Show this help screen
	-mail       : Show this help screen

	Example:
	1) $0 -branch KIRIN_BRANCH_5_100
	2) $0 -branch ROMTERM_BRANCH_5_100
";

	Exit($OK);

} # Help()

##
## genReleaseImages ()
## Generate a list of firmware images per brand type
##
sub genReleaseImages {

	foreach $dhd_brand ((split(/\s+/,$dhd_brands))) {
		# Skip internal build brands, as their firmware list is
		# derived with 'showtgts' target
		next if ($dhd_brand =~ /internal/);
		# Skip firmware build brand itself from list
		next if ($dhd_brand =~ /hndrte/);

		# Since some firmware names are derived dynamically, we need
		# cvs workspace to derive that list! So search for an existing
		# tob build for $dhd_brand
		my $dhd_brand_dir  = qx($FIND $build_linux/$branch/$dhd_brand -maxdepth 1 -mindepth 1 -type d -name "*${todayiter}*" 2> $NULL | sort | tail -1);
		chomp($dhd_brand_dir);
		open(RLS,"gmake -s -f $show_fw_mk CVSSRC=$dhd_brand_dir BRANDS=$dhd_brand|");
		while (<RLS>) {
			chomp;
			next if /^\s*$/;
			next if /#/;
			$_ =~ s/\s+//g;
			push(@release,"$dhd_brand:$_");
			&Dbg("Release Image : $dhd_brand => $_");
		}
		close(RLS);
	}

} # genReleaseImages ()

##
## genDefaultImages()
## Generate a list of firmware images that are built by default
##
sub genDefaultImages {
	open(DEFAULT,"gmake -s -C $hndrte_dir/src/dongle/rte/wl showtgts 2> /dev/null|");

	while (<DEFAULT>) {
		chomp;
		next if /^\s*$/;
		next if /#/;
		$_ =~ s/\s+//g;
		push(@default,$_);
		&Dbg("Default Image : $_");
	}

} # genDefaultImages ()

##
## matchImages()
## Search for release images in default list of images
##
sub matchImages {
	my $img;
	my $n=1;
	
	foreach $brand_img (sort grep { !$seen{$_}++ } @release) {
		($dhd_brand,$img) = (split(/:/,$brand_img));
		$img       =~ s/\s+//g;
		$imgmk     = (split(/\//,$img))[0];
		$dhd_brand =~ s/\s+//g;
		if ( ! grep { /^$img$/ } @default ) {
			push(@{$missing{$imgmk}{images}},$img);
			push(@{$missing{$imgmk}{brands}},$dhd_brand);
			print "[$n] MISSING entry for '$img' needed by '$dhd_brand' build in src/dongle/rte/wl/makefiles/${imgmk}.mk\n";
			$n++;
		}
	}

} # genDefaultImages ()
	
##
## genMsg()
## Generate an email message to branch owner
##
sub genMsg {
	my $img;

	open(MSG,">$textfile") || die "Can't open msg file $textfile";

	select MSG;
	print "\nFirmware image mismatch report for $branch ($todayts)\n";
	print "===============================================================\n";
	print "\nNOTE: Following firmware images are needed by release builds\n";
	print "that are not built by default in hndrte-dongle-wl build brand.\n";
	print "To correct this, please do one of the following\n";
	print "  1) If these are NOT needed, remove from release build shown\n";
	print "  OR\n";
	print "  2) If these are needed, update src/dongle/rte/wl/makefiles/<chip>.mk\n";
	print "\nWithout these changes, $branch builds will run slower\n";
	$n = 1;
	foreach $mk (keys %missing) {
		my %seeni=();
		my %seenb=();

		print "\n[$n] Add following firmwares to src/dongle/rte/wl/makefiles/${mk}.mk\n";
		foreach $img (sort grep { !$seeni{$_}++} @{$missing{$mk}{images}}) {
			print "\t$img\n";
		}
		print "\n\tNeeded by following build brands\n";
		foreach $brand (sort grep { !$seenb{$_}++} @{$missing{$mk}{brands}}) {
			print "\t\t$brand\n";
		}
		$n++;
	}
	select STDOUT;
	close(MSG);
	system("unix2dos $textfile > /dev/null 2>&1");
} # genMsg()

##
## sendMsg()
## Search for release images in default list of images
##
sub sendMsg {
	if (!$notify) {
		$notify = $BranchOwners{$branch}{owner} ?  $BranchOwners{$branch}{owner} : $BranchOwners{ALL_BRANCHES}{owner};
	}

	if (keys %missing) {
		print "\nNotifying $notify user to update firmware list\n";
		system("cat $textfile | mail -s 'WARN: Dongle Image List Errors on $branch' $BranchOwners{$branch}{owner} $ccusers");
	} else {
		print "\nNo missing images found for $branch\n";
	}
	unlink($textfile);

} # sendMsg()

## Main
sub Main {

	Dbg("Run Main()");

	genReleaseImages();
	genDefaultImages();
	matchImages();
	genMsg();
	sendMsg();

} # Main()

## Start of program
Main();
