#!/usr/bin/perl

##############################################################
# Script to filter patch from CVS diff, eliminating files in
# directories that are not already in the target checkout.
#
# Copyright (C) 2004 Broadcom Corporation
#
# $Id$
#
# Usage: cvs diff -Nu <srcdir> | ovpatch.pl <srcdir>
##############################################################

##############################################################
# Argument is path to the source tree where the diff output
# (input to ovpatch) was generated.  Assumes current directory
# is the equivalent location in the target tree.
#
# Look at Index lines to determine file names; make relative
# to target (current directory) by removing <srcdir> and
# changing repository top based on <srcdir>/CVS/Repository.
#
# Check for containing target subdirs; pass lines for files
# whose directories exist, discard others.
##############################################################

# Will use basename and dirname
use File::Basename;

# Check input arg
if (! scalar @ARGV) {
    $progname = basename($0);
    printf STDERR "Usage: $progname <srcdir>\n";
    printf STDERR "  Assumes stdin is the output from:\n";
    printf STDERR "    cvs diff -Nu <srcdir>\n";
    printf STDERR "  Makes context-diff file names repository-relative.\n";
    printf STDERR "  Removes files w/o corresponding target subdirs.\n";
    printf STDERR "Note: <srcdir> must be and absolute path.\n";
    exit 1;
}
$srctree = $ARGV[0];
$srctree =~ m:^/: || die "Not an absolute path: $srctree\n";

# Get repository information from srctree
open(RepFile, "$srctree/CVS/Repository") ||
    die "Can't find repository file in $srctree, stopped";
($repdir=<RepFile> and $repdir) ||
    die "Can't get repository info in $srctree, stopped";
close(RepFile);

# Parse/check srctree as per repository root
chomp $repdir;
($rephead,$reptail) = ($repdir =~ m:([^/]+)(/.+)?:);
($srctree,$srchead) = ($srctree =~ m:(.+)/([^/]+)$reptail:);
printf STDERR "Using $srctree/$srchead as repository: $rephead\n";

# Start in discard mode with no header
$xform=0;
$header="";
$discard=1;

# Initialize file counts
$goodfiles=0;
$badfiles=0;

while (<STDIN>) {

    if (!/^Index: /) {
	# Discard or passthru
	$discard && next;
	!$header && print && next;

	# Convert filenames and add to header
	/^(---|\+\+\+) / && ++$xform &&
	    s{^(---|\+\+\+) $srctree/$srchead/(.+)}{$1 $rephead/$2};
	$header .= $_;

	# If done both file lines, dump modified header
	if ($xform == 2) {
	    print $header;
	    $header=""; $xform=0;
	}

	# If it's a binary file, skip it
	if (/^Binary.*differ$/) {
	    $header = "";
	    next;
	}

	next;
    }

    # New index line: make sure we're ready for it
    $header && die "New index in file header at stdin line $., stopped";

    # Get relative target file and directory relative to repository
    chomp;
    ($tfile) = /^Index: (.*)/;
    $tfile =~ s{^$srctree/$srchead/(.+)}{$1} ||
	die "Not in $srctree/$srchead ($_), stopped";
    $tdir = dirname($tfile);
    $tdir ne $lastdir && printf STDERR "Target: $tdir\n";
    $lastdir = $tdir;

    # Set discard mode based on existence in target repository
    $tdir = "$rephead/$tdir";
    (-e $tdir && ! -d $tdir) &&
	die "Target $tdir is not a directory, stopped";
    $discard = ! -e $tdir;
    $discard && printf STDERR "Skipping $tfile (no target subdir)\n";

    # If not discarding, start header with this Index line
    $discard || ($header = "$_\n");

    # Count files discarded or passed
    $discard && $badfiles++;
    $discard || $goodfiles++;
}

printf STDERR "Discarded $badfiles, passed $goodfiles.\n";
$goodfiles || print STDERR "ERROR: no files changed!!\n";

# Error exit if all files discarded
exit ($goodfiles == 0);
