#!/usr/bin/env perl
#
# Show list build brand per active branch per platform in csv format
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$

# Timestamp
my $timestamp = qx(date '+%Y/%d/%M %H:%M:%S'); chomp($timestamp);

# Central build config file
my $build_config = "/home/hwnbuild/src/tools/build/build_config.sh";

# List of platforms
my @platforms = qw(linux macos netbsd windows);

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
print "BRANCH,PLATFORM,BUILD_BRAND\n" if (keys %branchdetails);

# Foreach branch generate platform specific brand details
foreach my $branch (sort keys %branchdetails) {
	foreach my $platform (sort keys %{$branchdetails{$branch}}) {
		foreach my $brand (sort keys %{$branchdetails{$branch}{$platform}}) {
			print "$branch,$platform,$brand\n";
		}
	}
	
}
