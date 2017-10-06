#!/usr/local/bin/perl
# $Id: hnd_bin2hex.pl,v 168.3 2003/03/04 22:45:38 venkatk Exp a
# convert mem file to binary file for BOOTROOM
#
# Example:
# hnd_hex2bin.pl H0E_L65RM4096X32R32VTSS.mem -rom_size 81920 -convert_2_little
# hnd_hex2bin.pl H0E_L65RM8192X32R32VTSS.mem -rom_size 122880 -rom_bank_size 32768 -convert_2_little
#
($script = $0) =~ s@^.*/@@;

use Getopt::Long;
use POSIX;
Getopt::Long::Configure("pass_through");

# size is in bytes
$rom_size = 16384;
$rom_bank_size = 16384;
$width    = 4;
$convert_2_little = 0;
$fn_lead_zero = "";
$no_max_size_err = 0;

GetOptions ( 
    'rom_size=i'        => \$rom_size,
    'rom_bank_size=i'   => \$rom_bank_size,
    'width=i'           => \$width,
    'fn_lead_zero=i'    => \$fn_lead_zero,
    'convert_2_little'  => \$convert_2_little,
    'no_max_size_err=i' => \$no_max_size_err );

die "$script: Error:fn_lead_zero must be 2!\n" if ($fn_lead_zero != 2);

$rom_size = ($rom_size==1)? $ARGV[1]:$rom_size;
$word_size = $width;
$rom_words = $rom_size / $word_size;
$rom_bank_words = $rom_bank_size / $word_size;

$num_banks = POSIX::ceil($rom_size/$rom_bank_size);
$last_rom_bank_size = $rom_size - ($num_banks - 1) * $rom_bank_size;
$last_rom_bank_words = $last_rom_bank_size / $word_size;

$hex_file = shift @ARGV;
die "$script: Error: input hex file not found" if !defined($hex_file);
print "Hex file $hex_file\n";
print "ROM SIZE=$rom_size Bytes $rom_words Words\n";
print "ROM BANK SIZE=$rom_bank_size Bytes $rom_bank_words Words\n";
print "ROM LAST BANK SIZE=$last_rom_bank_size Bytes $last_rom_bank_words Words\n";
print "NUM BANKS=$num_banks\n";

for ($i = 0; $i < $num_banks; $i++) {
    if ($i < 10) {
        $fn_lead_zero = "00";
    } else {
        $fn_lead_zero = "0";
    }
    ($out_file[$i] = $hex_file) =~ s/\.mem/_${fn_lead_zero}$i\.codefile/;
}
$i--;
$out_file[$i] =~ s/$rom_bank_words/$last_rom_bank_words/;

open (INFILE,"$hex_file") || die "$script: Error: cannot open $hex_file\n";
for ($i = 0; $i < $num_banks; $i++) {
    open ($OUTFILE[$i],">$out_file[$i]") || die "$script: Error: cannot open $out_file[$i]\n";
}

$count = 0;

for ($i = 0; $i < $rom_words; $i++) {
    if($width eq 4) {
	$data_array[$i] = "00000000000000000000000000000000";  
    } else {
	  $data_array[$i] = "0000000000000000000000000000000000000000000000000000000000000000";  
    }
}

$mask = 0x800000000;
while (!($rom_size & $mask)) {
   $mask = $mask >> 1;
}

if ($width eq 4) {
    $mask = ($mask >> 1) - 1;
} else {
    $mask = ($mask >> 2) - 1;
}

$seg = sprintf("0x%0b",10);
while (<INFILE>) {
 	chop();
        s/;//;
        ($addr, $data) = split('/');
        $address = hex($addr);
        $address = $address & $mask;

	# Error check to ensure data fits in ROM.
	if ($address >= $rom_words) {
		my $err_str = "The address 0x$address is too big to fit into ROM ($rom_size bytes)\n";
		if ($no_max_size_err) {
			print "$script: Warning: " . $err_str;
			last;
		}
		else {
			die "$script: Error: " . $err_str;
		}
	}

	if($convert_2_little) {
		$data_array[$address] = big_2_little($data);
        }
	else {
        if($width eq 4) {
  		$bin_data = sprintf("%032b",hex($data));
        } else {
  		  $bin_data = sprintf("%064b",hex($data));
        }
		$data_array[$address] = $bin_data;
        }
}

for ($i=0; $i < $rom_bank_words ; $i++) {
        for ($j = 0; $j < $num_banks-1; $j++) {
                print  {$OUTFILE[$j]} "$data_array[$i + $j * $rom_bank_words]\n";
        }
}
for ($i=0; $i < $last_rom_bank_words ; $i++) {
        print  {$OUTFILE[$j]} "$data_array[$i + $j * $rom_bank_words]\n";
}


close (INFILE);
for ($i = 0; $i < $num_banks; $i++) {
    close ($OUTFILE[$i]);
}


sub big_2_little {
  my ($org_data,$junk) = @_; 
 
	$big_data = $org_data;
 
	$little_data = 0;
	$bin_data = 0;
 
	if ($width eq 4) {
		$big_data = hex($org_data);
		$little_data = ($big_data & 0xff) << 24 | $little_data;
		$little_data = ($big_data & 0xff00) << 8 | $little_data;
		$little_data = ($big_data & 0xff0000) >> 8 | $little_data;
		$little_data = ($big_data & 0xff000000) >> 24 | $little_data;
		$bin_data = sprintf("%032b",$little_data);
	} elsif ($width eq 8) {
		 $little_data_lower = unpack("L", pack("H8", substr($big_data, 0, 8)));
		 $little_data_upper = unpack("L", pack("H8", substr($big_data, 8, 16)));
		 $bin_data_lower = sprintf("%032b", $little_data_lower);
		 $bin_data_upper = sprintf("%032b", $little_data_upper);
		 $bin_data = $bin_data_lower . $bin_data_upper;
	}else {
		die "$script: Error: No correct width provided. Width can be either 4 or 8 \n";
  }

  return $bin_data;
}


