#!/usr/local/bin/perl

# script to break up SOCRAM ROM memory init file into a few little files
# because the hardware is that way

$ROMBANKS = 5;
$ROMSIZE = 0x20000;
while ($ARGV[0] =~ /^([\-+])/) {
    $full_arg = $ARGV[0];
    $arg = substr (shift, 1);
    $infile = shift if $arg =~ s/^input//;
    $ROMSIZE = shift if $arg =~ s/^rombanksz//;
    $ROMBANKS = shift if $arg =~ s/^rombanks//;
    $CPU = shift if $arg =~ s/^cpu//;


   die "Unknown option: $full_arg\n" if  $arg;
}

print "infile : $infile, number of rombank: $ROMBANKS, size of rombank: $ROMSIZE, CPU : $CPU\n";

open (INF, $infile) || die "Unable to open $infile. $!\n";
$fCt = 0; # file ct [0-4]
$lCt = 0; # line ct [0-0x3fff]

while (<INF>) {
   next if (/INSTANCE/);
   next if (/RADIX/);
   next if (/ADDRESS/);
   next if (/END/);
   if ($lCt == 0) {
	open (OUT, ">${CPU}_armcr4_rom_$fCt.hex.qt");	
	print "opening file ${CP}U_armcr4_rom_$fCt.hex.qt\n";
	print OUT "\$RADIX   HEX\n\$ADDRESS 0 3fff\n";
# the original usbrdl.qt file has sequencially growing line number, need to null the leading 0's
        $hexval = sprintf("%08X", $lCt);
        s/^......../$hexval/;
	print OUT $_;
	$lCt++;
   } elsif ($lCt == $ROMSIZE -1) {
	s/^...../00000/; 
	print OUT $_;
	print OUT "\$END";
	close (OUT);
	print "close file ${CP}U_armcr4_rom_$fCt.hex.qt\n";
	$fCt++;
	$lCt = 0;
   } else {
        $hexval = sprintf("%08X", $lCt);
        s/^......../$hexval/;
	print OUT $_; 
	$lCt++;
   }
}

if (($fCt == $ROMBANKS) && ($lCt > 0)) {
	print "rom file size > rom size, abort!\n";
	exit 1;
}

# fill in zeros for the rest of the mem files
print "filling in zeros!\n";
print "fCt is $fCt, lCt is $lCt\n";

while ($fCt < $ROMBANKS) {
  if ($lCt == 0) {
	open (OUT, ">${CPU}_armcr4_rom_$fCt.hex.qt");	
	print "opening file ${CPU}_armcr4_rom_$fCt.hex.qt\n";
	print OUT "\$RADIX   HEX\n\$ADDRESS 0 3fff\n";
	printf OUT "%08X    %016X\n", $lCt, 0; 
	$lCt++;
   } elsif ($lCt == $ROMSIZE - 1) {
	printf OUT "%08X    %016X\n", $lCt, 0; 
	print OUT "\$END";
	close (OUT);
	print "close file ${CPU}_armcr4_rom_$fCt.hex.qt\n";
	$fCt++;
	$lCt = 0;
   } else {
	printf OUT "%08X    %016X\n", $lCt, 0; 
	$lCt++;
   }
} 
close (INF);
close (OUT);
