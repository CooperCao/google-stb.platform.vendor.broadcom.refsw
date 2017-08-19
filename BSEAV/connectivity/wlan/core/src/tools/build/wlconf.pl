#!/usr/bin/perl
#
# generate wlconf.h from predefined linux-style configuration file
#
# Copyright (C) 2004 Broadcom Corporation
#
# $Id$

if ($#ARGV !=1) {
   print "Usage: wlconf.pl input_file output_file\n";
   exit(0);
}

$input=$ARGV[0];
$output=$ARGV[1];

open(FIN, "$input") || die "can't open $input file\n";
open (FOUT,">$output") || die "can't open $output file\n";

while ($line = <FIN>) {

	chomp($line);
 	
	if ( $line =~ /^\s*#.*$/ ) {
		next;
	}

	$line =~ s/^\s*(\S*)\s*=\s*(\S*)\s*$/#define	$1	$2/;
	print FOUT "$line\n";
}

close (FIN);
close (FOUT);


