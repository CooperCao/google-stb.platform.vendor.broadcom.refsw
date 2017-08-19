#!/usr/bin/env perl
#
# For all active build brands, search the bom used and check if corresponding
# SVN .sparse file exists. The generated report is in csv format
#
# Intended to be used for SVN transition effort
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#
#

# Timestamp
my $timestamp = qx(date '+%Y/%d/%M %H:%M:%S'); chomp($timestamp);

# SVN URL
# my $SVNROOT = "http://engcm-sj1-vm13.sj.broadcom.com/wlan_demo/2010-12-21_syd/proj";
my $SVNROOT = "http://engcm-sj1-vm13.sj.broadcom.com/wlan_demo/2011-01-20_sj/proj";

# Central build config file
my $build_config = "/home/hwnbuild/src/tools/build/build_config.sh";

# List of platforms
my @platforms = qw(linux macos netbsd window);

# First populate branchdetails hash by reading branch specific data from
# build_config
foreach my $branch (split(/\s+/, qx($build_config -a))) {
        foreach my $platform (@platforms) {
		my @brands=split(/\s+/, qx($build_config -r $branch -p $platform));
		foreach my $brand (@brands) {
			$branchdetails{$branch}{$platform}{$brand}=1;
        	}
        }
}

# First populate branchdetails hash by reading branch specific data from
# build_config
print "# Automatically generated on $timestamp\n";

# Print header
print "#,BRANCH,PLATFORM,BUILD_BRAND,HNDCVS BOM,LEGACY CVS MODULE,SVN SPARSE\n" if (keys %branchdetails);

# Foreach branch generate platform specific brand details
foreach my $branch (sort keys %branchdetails) {
	my $n=1;
	foreach my $platform (sort keys %{$branchdetails{$branch}}) {
		foreach my $brand (sort keys %{$branchdetails{$branch}{$platform}}) {
			my $rlslog = `find /projects/hnd/swbuild/build_$platform/$branch/$brand/* -maxdepth 1 -mindepth 1 -name '*,release.log' 2> /dev/null | tail -1`; chomp($rlslog);
			my $rlsdir = qx(dirname $rlslog 2> /dev/null); chomp($rlsdir);
			my $sparse_url;
			if ( -s "$rlslog" ) {
				# Find legacy cvs modules if any
				my $cvsmodules_found = qx(egrep -h 'CVSMODULES.*=' $rlsdir/*.mk 2> /dev/null | fmt -1 | egrep -v "CVSMODULES|=" | xargs echo); chomp($cvsmodules_found);
				# Find hndcvs bom references
				my $bom_cmd = qx(egrep -A 3 'hndcvs .*\\\\\|hndcvs .*-bom\|hndcvs_frontend .*\\\\\|hndcvs_frontend .*-bom' $rlslog | grep -v "hndcvs.*version"); chomp($bom_cmd);
				my @bom_cmds = split(/\s+/, $bom_cmd);
				my ($bom_found) = grep { /-bom/ } @bom_cmds;
				my $sparse_derived = $bom_found;
				$sparse_derived =~ s/-bom/.sparse/g;
				if ($branch == "NIGHTLY") {
					$sparse_url = "$SVNROOT/trunk/$sparse_derived";
				} else {
					$sparse_url = "$SVNROOT/branches/$branch/$sparse_derived";
				}
				my $sparse_found = qx(/tools/bin/svn info $sparse_url 2> /dev/null | grep "Name: "); chomp($sparse_found);
				$sparse_found =~ s/Name: //g;
				if ($bom_found && $sparse_found) {
					print "$n,$branch,$platform,$brand,$bom_found,,$sparse_found,\n";
				} elsif ($bom_found && !$sparse_found) {
					print "$n,$branch,$platform,$brand,$bom_found,,SPARSE_MISSING,\n";
				} elsif (!$bom_found && !$sparse_found) {
					print "$n,$branch,$platform,$brand,,$cvsmodules_found,SPARSE_MISSING,\n";
				}
			} else {
				print "$n,$branch,$platform,$brand,,,\n";
			}
		}
		$n++;
	}
	print ",,,,,\n";
}
