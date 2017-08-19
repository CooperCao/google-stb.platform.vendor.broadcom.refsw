#!/usr/bin/perl
#
# This script sends the result to STDOUT based on the second parameter:
#
#	_bldtgts_ - build targets, the first column of .mf file
#	others - copy targets, the second column of .mf file
#
# $Id$

use warnings;

my $fn = "nil";
if (@ARGV > 0) {
    $fn = $ARGV[0];
}

my $tgt = "_bldtgts_";
if (@ARGV > 1) {
    $tgt = $ARGV[1];
}

if ($fn eq "nil") {
    die "Usage: perl mf-parser.pl <.mf-file-name> <target>";
}

open($fh, "<$fn") or die "Unable to open file $fn: $!\n";
while (<$fh>) {
    s/^\s+//g;
    s/#.*$//g;
    next if /^$/;
    my($c1, $c2) = split(/\s+/);
    if ($tgt eq "_bldtgts_") {
	printf("$c1\n");
    }
    elsif ($tgt eq $c1) {
	printf("$c2\n");
    }
}
close($fh);
