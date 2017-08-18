#!/usr/local/bin/perl

# Script to convert 4 byte width to 8 byte width for qt files.
#


use strict;

sub usage {
    print STDERR "Usage: qtmem_4to8 <fname.qt>\n";
    exit 1;
}

&usage if $#ARGV < 0;

my $fname = $ARGV[0];
open (INF, $fname) || die "Unable to open $fname. $!\n";

printf "\$INSTANCE\n";
printf "\$RADIX   HEX\n";

while (<INF>) {
	next if (/INSTANCE/);
	next if (/RADIX/);
	if (/ADDRESS/) {
		my ($ignore, $addr_str, $data_str) = split(/\s+/);	
		printf "\$ADDRESS %x %x\n", (hex $addr_str)/2, (hex $data_str)/2;
		next;
	}

	next if (/END/);

	my $newline;
	my ($addr_str_low, $data_str_low) = split(/\s+/);
	my ($addr_str_high, $data_str_high);
	if (hex($addr_str_low) %2 != 0) {
		my $addr = ((hex $addr_str_low) - 1)/2;
		my $data_low = 0;
		my $data_high = hex $data_str_low;
		printf "%08X    %08X%08X\n", $addr,$data_high, $data_low;
		next;
	}

	$newline = <INF>;
	if($newline =~/END/) {
		$data_str_high = 0;
	}
	else {
		($addr_str_high, $data_str_high) = split(/\s+/,$newline);
	}

	my $addr = (hex $addr_str_low)/2;
	my $data_low = hex $data_str_low;
	my $data_high = hex $data_str_high;

	printf "%08X    %08X%08X\n", $addr,$data_high, $data_low;

}

close INF;

print "\$END\n";
