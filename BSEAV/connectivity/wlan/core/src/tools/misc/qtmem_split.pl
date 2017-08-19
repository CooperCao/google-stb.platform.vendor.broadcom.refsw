#!/usr/local/bin/perl

# Script to break up SOCRAM ROM/RAM memory init file into one file per bank.

sub usage {
    print STDERR "Usage: qtmem_split -infile <fname.qt> [-banks <n>] [-banksize <bytes>] 
    [-memsize <bytes>] [-width <bits>] [-qtfilepfx <filename prefix>]\n";
    exit 1;
}

&usage if $#ARGV < 0;

#default values
$memsize = 0;
$width = 4;
$out_file = 'soc_rom_';

while ($ARGV[0] =~ /^([\-+])/) {
    $arg = shift;

    if ($arg eq '-banks') {
	$banks = shift;
    } elsif ($arg eq '-input' || $arg eq '-infile') {
	$infile = shift;
    } elsif ($arg eq '-banksize') {
	$banksize = shift;
    } elsif ($arg eq '-memsize') {
	$memsize = shift;
    } elsif ($arg eq '-qtfilepfx') {
	$out_file = shift;
    } elsif ($arg eq '-width') {
	$width = shift;
    } else {
	die "Unknown option: $arg\n";
    }
}

$banksize = 16384 if ($banksize eq ""); # 16KB default
$bankwords = $banksize / $width;

# Read the input file into a hash of memory address => data.
# Memory addresses are word addresses; only the lowest 20 bits are used.

open (INF, $infile) || die "Unable to open $infile. $!\n";

%memory = ();

$addr_min = 0x100000;
$addr_max = -1;

while (<INF>) {
   next if (/INSTANCE/);
   next if (/RADIX/);
   next if (/ADDRESS/);
   next if (/END/);

   my ($addr_str, $data_str) = split(/\s+/);
   my $addr = hex $addr_str & 0xfffff;
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

# Compute number of banks if not specified
if ($banks eq "") {
    $banks = int(($addr_max + $bankwords - 1) / $bankwords);
}

# Compute memsize if not specified
if ($memsize eq 0) {
    $memsize = $banks * $banksize;
}

# Write the output files
my $wordaddr = 0;

for ($of = 0; $of < $banks; $of++) {
    open (OUT, ">$out_file$of.hex.qt");
    print STDOUT "qtmem_split.pl: writing file $out_file$of.hex.qt...\n";
    if (($of+1) * $banksize > $memsize){
	$bankwords = ($memsize - ($of * $banksize))/$width;
    }
    printf OUT "\$RADIX   HEX\n\$ADDRESS 0 %x\n", $bankwords - 1;
    for ($w = 0; $w < $bankwords; $w++) {
	if ($width eq 8) {
	    if ($memory{$wordaddr} eq "") {
		$memory{$wordaddr} = "0000000000000000";
	    }
	    printf OUT "%08X", $w;
	    print OUT "    ";
	    print OUT "$memory{$wordaddr++}\n";
	} else {
	    printf OUT "%08X    %08X\n", $w, $memory{$wordaddr++};
	}
    }
    print OUT "\$END\n";
    close OUT;
}
