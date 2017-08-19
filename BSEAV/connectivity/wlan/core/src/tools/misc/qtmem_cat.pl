#!/usr/local/bin/perl

# Script to concatenate SOCRAM ROM memory init file(s)
#
# If -baseaddr is specified, ROM is relocated to new base address
# Assumes concatenated ROM < 1M

use strict;

sub usage {
    print STDERR "Usage: qtmem_cat [-baseaddr 0xN] <fname.qt> [...]\n";
    exit 1;
}

&usage if $#ARGV < 0;

my $width = 4;
my $baseaddr = 0;
my $arg;
my $fname;
while ($ARGV[0] =~ /^([\-+])/) {
    $arg = shift;
    if ($arg eq '-baseaddr') {
        $baseaddr = shift;
	$baseaddr = hex($baseaddr)/4; # convert to 32-bit words
    } elsif ($arg eq '-width') {
	    $width = shift;
    }else {
        die "Unknown option: $arg\n";
    }
}

my %memory = ();

my $romsize = 0xfffff;
my $addr_min = 0xffffffff;
my $addr_max = -1;
my $addr;
# Read the input files into a hash of memory address => data.
# Memory addresses are word addresses for 32-bit words

foreach $fname (@ARGV) {
    open (INF, $fname) || die "Unable to open $fname. $!\n";

    while (<INF>) {
	next if (/INSTANCE/);
	next if (/RADIX/);
	next if (/ADDRESS/);
	next if (/END/);

	my ($addr_str, $data_str) = split(/\s+/);
	$addr = hex $addr_str;
	$addr &= $romsize if ($baseaddr);
	my $data;
	if ($width eq 8) {
		$data = $data_str;
	} else {
		$data = hex $data_str;
	}

	$addr_min = $addr if $addr < $addr_min;
	$addr_max = $addr if $addr > $addr_max;

	$memory{$addr} = $data;
    }

    close INF;
}


$baseaddr = 0 if (!$baseaddr);

# Write the output file
printf "\$INSTANCE\n";
printf "\$RADIX   HEX\n";
printf "\$ADDRESS %x %x\n", $baseaddr + $addr_min, $baseaddr + ($addr_max + 1);

for ($addr = $addr_min; $addr <= $addr_max; $addr++) {
    if($width eq 8) {
		# printf is dependent on the word size of the machine, so in 64 bit case 
		# use print instead.
		printf "%08X", $addr + $baseaddr;
		print "    ";
		print "$memory{$addr}\n";
    } else {
	    printf "%08X    %08X\n", $addr + $baseaddr, $memory{$addr};
    }
}
print "\$END\n";
