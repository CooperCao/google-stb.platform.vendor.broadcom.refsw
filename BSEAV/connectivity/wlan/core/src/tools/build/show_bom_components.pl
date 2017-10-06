#!/usr/local/bin/perl
#
# Given a hndcvs bom and inventory list module_list.mk for any given tag/branch
# show component information (expanded contents). Output can be generated in tree
# or text or csv format.
#
# This requires a src/tools/release/components exists on the branch where
# module_list.mk is pointing.
#
# One way of using this tool is to run inside a nightly/release build brand's
# src/tools/release directory.
#
# $Copyright (C) 2004 Broadcom Corporation
#
# # $Id$
#
# SVN: $HeadURL$
#
# Author: Prakash Dhavali (2010/08/31)
# Contact: hnd-software-scm-list
#

#use strict;

use       Getopt::Long;

my $bom         = "linux-router-bom.mk";
my $module_list = "module_list.mk";
my $dbg         = 0;
my $format      = "tree"; # Default is tree, valid values=text or csv

# Makefile that shows bom components and their baseline tags
my $show_bom_components="/home/hwnbuild/src/tools/release/show_bom_components.mk";
# Makefile that expands components into physical dirs and files
my $show_component_contents="/home/hwnbuild/src/tools/release/show_component_contents.mk";
my $CVSROOT="/projects/cvsroot";

&Dbg("BOM                   = $bom");
&Dbg("MODULE_LIST INVENTORY = $module_list");

# Show debug info
sub Dbg { print "DBG> @_\n" if ($dbg); }

# Process cmd line args
%scan_args = (
             'bom=s'           => \$bom,
             'brand_dir=s'     => \$brand_dir,
             'contents'        => \$contents,
             'dbg'             => \$dbg,
             'details'         => \$contents,
             'format=s'        => \$format,
             'module_list=s'   => \$module_list,
             'verbose'         => \$dbg,
             );

# Scan the command line arguments
GetOptions(%scan_args) or die "Get Options Failed";

if ($bom !~ /\.mk/) { $bom="$bom".".mk"; }

if ($format !~ /text|tree|csv/) {
	print "ERROR: -format needs to be one ether 'text' or 'tree' or 'csv'\n";
	exit 1;
}

if (! -f "$bom" || ! -f "$module_list") {
	print "ERROR: Can't continue without bom:$bom OR\n";
	print "ERROR: module_list:$module_list\n\n";
	exit 1;
}

if (! -f "$show_bom_components" || ! -f "$show_component_contents") {
	print "ERROR: Can't continue without $show_bom_components show_bom_components makefile OR\n";
	print "ERROR: $show_component_contents show_component_contents makefile\n\n";
	exit 1;
}

# Process given bom to filter its relevant components and baselines
my $bomopts="BOM=$bom";
my $components=qx(make -s -f $show_bom_components $bomopts);
my @components=split(/\s+/,$components);

#&Dbg("COMP_CMD=make -s -f $show_bom_components $bomopts");
#&Dbg("COMPONENTS = $components");

# Foreach component found, create data structure that maps component to
# a baseline tag (to check it out from)
foreach my $comp (@components) {
	chomp($comp);
	($compname,$baseline)=(split(/\.\.\./,$comp))[0,1];
	$baselines{$compname} = $baseline;
	&Dbg("FOUND C=$compname; B=$baseline");
}

# Show component details. Distiguish between dir and files
sub showCompDetails {
	my ($component,$modtype,@modules) = @_;

	&Dbg("showCompDetails() C=$component, T=$modtype, M=@{modules}");

	return if exists $processed{$component};
	return unless (@modules);

	my $opts = ($modtype =~ /non.*recursive/ ? " [non-recursive]" : "");

	if ($format =~ /tree/) {
		print "BOM=$bom; COMPONENT=$component\n";
		print "\t|\n";
	} elsif ($format =~ /text/) {
		print "BOM=$bom; COMPONENT=$component\n";
	} elsif ($format =~ /csv/) {
		print "$component,";
	}

	foreach $module (@modules) {
		next if ($module =~ /^\s*$/);
		my $extmod=$module;
		$extmod   =~ s%src/%%g;
		if ( -d "$CVSROOT/$module" ) {
			print "\t+--> $module/$opts\n" if ($format =~ /tree/);
			print "\tdir=$module/$opts\n" if ($format =~ /text/);
			print "$module/$opts;" if ($format =~ /csv/);
		} elsif ( -f "$CVSROOT/${module},v" ) {
			print "\t+--> $module\n" if ($format =~ /tree/);
			print "\tfile=$module\n" if ($format =~ /text/);
			print "$module;" if ($format =~ /csv/);
		} else {
			print "\t+--> ERROR($module)\n" if ($format =~ /tree/);
			print "\tERROR($module)\n" if ($format =~ /text/);
			print "ERROR($module);" if ($format =~ /csv/);
		}
		$processed{$component}=1;
	}
	print "\n";
}

# 1) Process each component to fetch a list of recursive (sources) modules
# and non-recursive (local_sources) from its <component>.mk definition file
foreach $component (sort keys %baselines) {
	my $sources=qx(make -s -f $show_component_contents COMPONENT=$component show_sources); chomp($sources);
	my $local_sources=qx(make -s -f $show_component_contents COMPONENT=$component show_local_sources); chomp($local_sources);
	my @sources = split(/\s+/,$sources);
	my @local_sources = split(/\s+/,$local_sources);

	&Dbg("C=$component; R=$sources; L=$local_sources");

	if ($contents) {
		showCompDetails($component,"recursive",@sources);
		showCompDetails($component,"nonrecursive",@local_sources);
	} else {
		print "BOM=$bom; COMPONENT=$component\n";
	}
}
