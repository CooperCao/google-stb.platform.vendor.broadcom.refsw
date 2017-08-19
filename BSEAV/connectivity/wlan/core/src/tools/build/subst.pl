#!/usr/bin/perl

use strict;
use warnings;

my $idre = '([\w$]+)';

my $fh;

sub usage {
    print STDERR "Usage: cat list_of_files | subst subst_list.txt\n";
    exit 1;
}

sub err {
    my $msg = shift;
    print STDERR "subst: $msg\n";
    exit 1;
}

&usage if (@ARGV != 1);

my $subst_list = shift;

# Read substitution file

my %subst = ();

open($fh, $subst_list) or
    err "Could not open $subst_list: $!";

while (<$fh>) {
    chomp;
    s/#.*//;
    s/^\s*//;
    s/\s*$//;
    next if /^$/;
    if (/^${idre}\s*${idre}$/) {
	$subst{$1} = $2;
    } else {
	err "Invalid substitution, $subst_list line $.";
    }
}

close $fh;

# Read and process list of files from stdin

while (<>) {
    chomp;
    my $fn = $_;
    my $data;

    open($fh, $fn) or
	die "Could not open $fn for reading: $!";
    {
	local $/;   # slurp
	$data = <$fh>;
    }
    close $fh;

    map {
	$data =~ s/\b\Q$_\E\b/$subst{$_}/g;
    } keys %subst;

    open($fh, ">$fn") or
	die "Could not open $fn for writing: $!";
    print $fh $data;
    close $fh;
}
