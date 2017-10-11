#!/usr/local/bin/perl
# ------------------------------------------------------------------------------
# $Id: codefile2qt.pl,v 12.3 2011-01-18 20:53:43 lut Exp $
# Usage: codefile2qt.pl < pac > tb
# Description: This script will take .codefile format on standard input
#              and convert it to .qt format on standard output
# ------------------------------------------------------------------------------

sub usage {
    print STDERR "Usage: codefile2qt -maxaddr <hexval>\n";
    exit 1;
}

&usage if $#ARGV < 1;

while ($ARGV[0] =~ /^([\-+])/) {
    $arg = shift;

    if ($arg eq '-maxaddr') {
	$maxaddr = shift;
    } else {
	die "Unknown option: $arg\n";
    }
}

$line = 0;

print "\$RADIX   HEX\n";
print "\$ADDRESS 0 $maxaddr\n";

while (<>) {
    chomp();
    $hex_value = bin_2_hex($_);
    $hex_line = sprintf("%08x",$line);
    print "$hex_line    $hex_value\n";
    $line++
}

print "\$END\n";

sub bin_2_hex
{
    my ($bin_num) = @_;
	
    my $hex_num = "";
    my $ctr;
    my $hex_bit;
    my @char;
    my $tt = 0;
    my $bin_val = 0;

    @char 	= split(//,$bin_num);

    $num_bits = $#char + 1;
    if ($num_bits % 4) {
      $tt = 4 - ($num_bits % 4);
    }
    $num_bits += $tt;
    $bin_val = 0 x $tt.$bin_num;
    @char 	= split(//,$bin_val);

    # convert from bin to hex
    for($ctr = 0; $ctr < $num_bits/4; $ctr = $ctr + 1)
    {

	$hex_bit[$ctr] = $char[$ctr * 4].$char[$ctr * 4 + 1].$char[$ctr * 4 + 2].$char[$ctr * 4+3];

	$hex_bit[$ctr]	=~ s/0000/0/;
        $hex_bit[$ctr]	=~ s/0001/1/;
	$hex_bit[$ctr]	=~ s/0010/2/;
	$hex_bit[$ctr]	=~ s/0011/3/;
	$hex_bit[$ctr]	=~ s/0100/4/;
	$hex_bit[$ctr]	=~ s/0101/5/;
	$hex_bit[$ctr]	=~ s/0110/6/;
	$hex_bit[$ctr]	=~ s/0111/7/;
	$hex_bit[$ctr]	=~ s/1000/8/;
	$hex_bit[$ctr]	=~ s/1001/9/;
	$hex_bit[$ctr]	=~ s/1010/a/;
	$hex_bit[$ctr]	=~ s/1011/b/;
	$hex_bit[$ctr]	=~ s/1100/c/;
	$hex_bit[$ctr]	=~ s/1101/d/;
	$hex_bit[$ctr]	=~ s/1110/e/;
	$hex_bit[$ctr]	=~ s/1111/f/;
	$hex_num 	= $hex_num.$hex_bit[$ctr];
    }
    return ($hex_num);
}
# ------------------------------------------------------------------------------
