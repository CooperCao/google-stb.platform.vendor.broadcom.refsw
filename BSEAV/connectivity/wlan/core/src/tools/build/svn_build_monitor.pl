#!/usr/local/bin/perl
#
# Generate SVN Build Status dash board until svn transition for all
# branches
#
# $Id$

use strict;
use Getopt::Long;

my $rowcolor;
my %seen;
my $fordate; # In format yyyy.m.d
my $today     = qx(date '+%Y.%-m.%-d'); chomp($today);
my $timestamp = qx(date '+%Y.%m.%d_%H:%M:%S'); chomp($timestamp);
my $current_user=getpwuid($<);

my $BUILD_CONFIG=qq(/projects/hnd_software/gallery/src/tools/build/build_config.sh);
my $SVNREPORTS_DIR=qq(/projects/hnd/swbuild/build_admin/logs/svnreports);

# Derive active svn branches from build_config.sh
my @platforms = qw(linux macos netbsd window);
my $active_branches=qx($BUILD_CONFIG -s -q);
my @active_branches=sort split(/\s+/,$active_branches);
my @report_branches;# Subset of branches to produce report on
my $dbg;            # Debug mode flag
my $csv;            # NOT implemented yet, meant to generate excel/csv file
my $output;         # Output report file name
my $branch;         #
my %branch_data;    # Branch data for a given date
my $report;	    # Final report
my $report_new;	    # New report that is getting generated
my $errors_only;    # Produce failure only report
my $pass_only;      # Produce success only report
my $report_suffix;  # report prefix

my $report_index  = "index.htm";  # main report
my $report_errors = "errors.htm";  # errors report
my $report_pass   = "pass.htm";   # pass report

my %SVN_DIR=(
   'svn' => {
   	'linux' => qw(/projects/hnd/swbuild/build_linux/SVN_TRANSITION/SVN),
   	'macos' => qw(/projects/hnd/swbuild/build_macos/SVN_TRANSITION/SVN),
   	'netbsd' => qw(/projects/hnd/swbuild/build_netbsd/SVN_TRANSITION/SVN),
   	'window' => qw(/projects/hnd/swbuild/build_window/SVN_TRANSITION/SVN),
   },
   'cvs' => {
   	'linux' => qw(/projects/hnd/swbuild/build_linux/SVN_TRANSITION/CVS),
   	'macos' => qw(/projects/hnd/swbuild/build_macos/SVN_TRANSITION/CVS),
   	'netbsd' => qw(/projects/hnd/swbuild/build_netbsd/SVN_TRANSITION/CVS),
   	'window' => qw(/projects/hnd/swbuild/build_window/SVN_TRANSITION/CVS),
   }
);


# Command line options
my %scan_args = (
                'csv'           	=> \$csv,
                'dbg'           	=> \$dbg,
                'debug'           	=> \$dbg,
                'errors_only'           => \$errors_only,
                'output=s'      	=> \$output,
                'pass_only'             => \$pass_only,
                'report=s'      	=> \$output,
                'date=s'      	        => \$fordate,
);

# Scan the command line arguments
GetOptions(%scan_args) or &Error("GetOptions failed!");

if ($errors_only && $pass_only) {
	print "ERROR: \n";
	print "ERROR: -pass and -error are mutually exclusive switches\n";
	print "ERROR: Choose either one of them\n";
	print "ERROR: \n";
	exit 1;
}

# If debug flag is specified, don't overwrite main status pages
if ($dbg) {
	$report_suffix = "_debug";
	$report_index  = "index$report_suffix.htm";  # debug main report
	$report_errors = "error$report_suffix.htm";  # debug errors report
	$report_pass   = "pass$report_suffix.htm";   # debug pass report
}

# Append _errors or _pass to generated reports for errors or pass cmd lines
if ($errors_only) {
	$report_suffix .= "_errors";
} elsif ($pass_only) {
	$report_suffix .= "_pass";
}

if (!$output) {

	if ($current_user =~ /hwnbuild/) {
		system("mkdir -pv $SVNREPORTS_DIR/${today}");
		$report="${SVNREPORTS_DIR}/${today}${report_suffix}.htm";
		$report_new="${SVNREPORTS_DIR}/${today}${report_suffix}_new.htm";
	} else {
		$report="svn_status_${today}${report_suffix}.htm";
		$report_new="svn_status_${today}${report_suffix}_new.htm";
	}
} else {
		$report=$output;
		$report_new=$output;
}

# Debug ()
sub Dbg {
        my ($msg) = @_;

        return if (!$dbg);

        print STDOUT "DBG: $msg\n";
}

#
# Print HTML page header
#
sub HtmlHeader {
	# Output HTML page header
	print "<html xml:lang=\"en\" lang=\"en\">
	<head>
	 <title> SVN Build Status </title>
	 <meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\" />
	</head>
	<body bgcolor=\"#ffffff\">\n";

} # HtmlHeader()

#
# Print HtML table header
#
sub TableHeader {
	#print "<table width=\"100%\" border=\"0\" cellpadding=\"3\" cellspacing=\"3\"> \n";
	# Place header info in the center of page
	print "<center>\n";
	print "<table border=\"1\" cellpadding=\"3\" cellspacing=\"3\"> \n";
	print "<tr style=\"background-color: yellow; font-size: 14pt\"><td><b>WLAN S/W Recent Subversion Build Status</b><br><i>(Page Last Updated at $timestamp)</td></tr>\n";
	print "<tr style=\"background-color: yellow; font-size: 12pt\"><td>SVN Builds at: /projects/hnd/swbuild/build_[platform]/SVN_TRANSITION/SVN</td></tr>\n";
	print "<tr style=\"background-color: yellow; font-size: 12pt\"><td>CVS Builds at: /projects/hnd/swbuild/build_[platform]/SVN_TRANSITION/CVS</td></tr>\n";
	print "</table>\n";
	print "</center>\n";

	#print "<table width=\"100%\" border=\"5\" cellpadding=\"1\" cellspacing=\"1\">\n";
	print "<table border=\"1\" cellpadding=\"3\" cellspacing=\"3\"> \n";

	# START: Header/Title Line
	print "<b><tr style=\"font-size: 10pt; background-color: black; color: #ffffff\">\n";
	print "<td style=\"font-size: 12pt\"><b>#</b></td>\n";
	print "<td style=\"font-size: 12pt\"><b>Branch Name</b></td>\n";
	print "<td style=\"font-size: 12pt\"><b>Platform : Build Brand</b></td>\n";

	foreach my $type ("CVS","SVN") {
		print "<td style=\"font-size: 12pt\"><b>$type BUILD STATUS</b></td>\n";
		print "<td style=\"font-size: 12pt\"><b>$type BUILD LOG</b></td>\n";
	}
	print "</tr></b>\n";
	# END: Header/Title Line

} # TableHeader()

# TODO: Embed build errors in generated htm report and add expand button with CSS
sub GenBranchData {
	my ($branch,$bldtype) = @_;

	Dbg("GenBranchData($branch,$bldtype)");

	$ENV{"VCTOOL"}="svn" if ($bldtype =~ /svn/);
	$ENV{"CVSROOT"}="CVS_SNAPSHOT" if ($bldtype =~ /cvs/);

	Dbg("VCTOOL = $ENV{'VCTOOL'}");
	Dbg("CVSROOT = $ENV{'CVSROOT'}");

	foreach my $platform (@platforms) {
		my $build_dir = $SVN_DIR{$bldtype}{$platform};
		Dbg("bldtype=$bldtype; pf=$platform; build_dir = $build_dir");
		my @brands = split(/\s+/, qx($BUILD_CONFIG -r $branch -p $platform));
		for my $brand (@brands) {
			my $brand_dir="$build_dir/$branch/$brand";
			my (@iters)= grep { /\d{4}\.\d{1,2}\.\d{1,2}/ } split(/\s+/, qx(ls -1tr $brand_dir));

			(@iters)=grep { /${fordate}\./ } @iters if ($fordate);

			my ($iter) = pop(@iters);
			my $contents = qx(ls -ltr $brand_dir/$iter); chomp($contents);
				
			Dbg("Populating SVN branch=$branch; brand=$brand; bldtype=$bldtype; iter=$iter");
			Dbg("Contents: $contents");
			if (-f "$brand_dir/$iter/,succeeded") {
				$branch_data{$branch}{$platform}{$brand}{$bldtype} = "PASS $iter";
			} elsif (-f "$brand_dir/$iter/_WARNING_WRONG_CHECKOUT_FOUND") {
				$branch_data{$branch}{$platform}{$brand}{$bldtype} = "WARN $iter";
			} elsif (-f "$brand_dir/$iter/,build_errors.log") {
				$branch_data{$branch}{$platform}{$brand}{$bldtype} = "FAIL $iter";
			} elsif (-d "$brand_dir/$iter") {
				$branch_data{$branch}{$platform}{$brand}{$bldtype} = "IN_PROGRESS $iter";
			} else {
				$branch_data{$branch}{$platform}{$brand}{$bldtype} = "MISSING $iter";
			}
		}
	}
}

sub ShowLastModifiedTime {
	my ($file) = shift;
	my (@stat, $mtime);

	Dbg("ShowLastModifiedTime() File = $file");

	return("") if (! -f $file);

	$mtime = (stat($file))[9];
	@stat=localtime($mtime);
	# $mtime = sprintf("%d/%.2d/%.2d %.2d:%.2d",$stat[5]+1900,$stat[4]+1,$stat[3], $stat[2], $stat[1], $stat[0]);
	$mtime = sprintf("%d/%.2d/%.2d %.2d:%.2d",$stat[5]+1900,$stat[4]+1,$stat[3], $stat[2], $stat[1]);
	return($mtime);
}

#
# Generate individual build status info
#
sub HtmlBranchData {

	foreach my $branch (@report_branches) {
		chomp($branch);
		Dbg("Processing $branch data now");
		&GenBranchData($branch,"svn");
		&GenBranchData($branch,"cvs");

	}

	my $index=1;
	foreach my $branch (sort keys %branch_data) {
		foreach my $platform (sort keys %{$branch_data{$branch}}) {
			my $PLATFORM = uc($platform);
			foreach my $brand (sort keys %{$branch_data{$branch}{$platform}}) {
				my $cvs_result = $branch_data{$branch}{$platform}{$brand}{cvs};
				my $svn_result = $branch_data{$branch}{$platform}{$brand}{svn};
				if ($errors_only && ($cvs_result =~ /PASS/) && ($svn_result =~ /PASS/)) {
					next;
				}
				if ($pass_only && ($cvs_result !~ /PASS/) && ($svn_result !~ /PASS/)) {
					next;
				}
				$rowcolor = $rowcolor ? "" : "background-color\: #FFFFCC";
				print "<tr style=\"$rowcolor; font-size: 10pt\">\n";
				print "<td style=\"font-size: 10pt\">$index</td>\n";
				print "<td style=\"font-size: 10pt\">$branch</td>\n";
				print "<td style=\"font-size: 10pt\">$PLATFORM : $brand </td>\n";
				foreach my $bldtype (sort keys %{$branch_data{$branch}{$platform}{$brand}}) {
					my $BLDTYPE  = uc($bldtype);
					my $branch_url="http://home.sj.broadcom.com/projects/hnd/swbuild/build_$platform/SVN_TRANSITION/$BLDTYPE/$branch";
					my $brand_url="$branch_url/$brand";
					my $brand_result = $branch_data{$branch}{$platform}{$brand}{$bldtype};
					my ($iter) = grep { /\d{4}\.\d{1,2}\.\d{1,2}/ } split(/\s+/, $brand_result);
					my ($iter_url, $iter_mtime);
					my $iter_dir= "$SVN_DIR{$bldtype}{$platform}/${branch}/${brand}/${iter}";

					$brand_result =~ s/\s*$iter\s*//g;

					Dbg("brand_url = $brand_url");
					Dbg("iter      = $iter");
					my $fontcolor = "blue"; # default
					my $brand_msg = "LOG";  # default
					if ($brand_result =~ /PASS/) {
						$iter_url="$brand_url/$iter/,release.log";
						$iter_mtime=ShowLastModifiedTime("$iter_dir/,release.log");
						$fontcolor = "green";
						$brand_msg = "LOG";
					} elsif ($brand_result =~ /FAIL/) {
						$iter_url="$brand_url/$iter/,build_errors.log";
						$iter_mtime=ShowLastModifiedTime("$iter_dir/,build_errors.log");
						$fontcolor = "red";
						$brand_msg = "ERRORS";
						#$brand_result="<b>$brand_result</b>";
					} elsif ($brand_result =~ /IN_PROGRESS/) {
						$iter_url="$brand_url/$iter/,release.log";
						$iter_mtime=ShowLastModifiedTime("$iter_dir/,release.log");
						$fontcolor = "blue";
						$brand_msg = "STATUS";
					} elsif ($brand_result =~ /WARN/) {
						$iter_url="$brand_url/$iter/,release.log";
						$iter_mtime=ShowLastModifiedTime("$iter_dir/,release.log");
						$fontcolor = "orange";
						$brand_msg = "WRONG_CHECKOUT";
						#$brand_result="<b>$brand_result</b>";
					} else {
						$iter_url="$brand_url";
						$iter_mtime="";
						$fontcolor = "orange";
						$brand_msg = "MISSING";
						#$brand_result="<b>$brand_result</b>";
					}
					print "<td style=\"font-size: 10pt\"><font color=\"$fontcolor\">$bldtype : ${brand_result}</font></a></td>\n";
					print "<td style=\"font-size: 10pt\"><font color=\"$fontcolor\"><a href=\"$iter_url\">$bldtype : $brand_msg</font></a><i><br>($iter_mtime)</br></i></td>\n";
				} # bldtype
				$rowcolor = $rowcolor ? "background-color\: #FFFFCC" : "";
				print "</tr>\n";
				$index++;
			} # brand
		} # platform
		print "<tr><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td></tr>\n";
	} # branch
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
		chdir("$SVNREPORTS_DIR");
		rename("$report","${today}/${timestamp}.htm") if ( -s "$report");
		rename("$report_new","${report}") if ( -s "$report_new");
		if ($errors_only) {
			print "\nSym-linking ${report} as main $report_errors\n";
			unlink("$report_errors");
			system("ln -s ${report} $report_errors");
		} elsif ($pass_only) {
			print "\nSym-linking ${report} as main $report_pass\n";
			unlink("$report_pass");
			system("ln -s ${report} $report_pass");
		} else {
			print "\nSym-linking ${report} as main $report_index\n";
			unlink("$report_index");
			system("ln -s ${report} $report_index");
		}
		print "\nGenerated report is at $report file\n";
	} else {
		print "\nGenerated report is at $report file\n";
	}

} # EndReport()

sub Main {

	print "\n";
	Dbg("SVN Build Status Reporting for $fordate");
	print "\n";

	@report_branches = sort grep { !$seen{$_}++ } @active_branches;
	Dbg("Report Branches = @report_branches");

	open(OUTPUT, ">${report_new}") || die "Can't create report file $report_new!!\n";
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
