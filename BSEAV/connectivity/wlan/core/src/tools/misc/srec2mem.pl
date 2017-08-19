#!/usr/bin/perl

# This script converts the objcopy generated .srec file format to the .mem (=.mif) format.
# The generated .mem file presents the data from the .srec in big endian order.

use strict;
use Math::BigInt;

sub usage {

	print "perl srec2mem.pl  <.srec file>\n";
}

if (@ARGV < 1) {
	&usage;
}
my $show_header;
my $show_footer;

my @args = @ARGV;
my $fname = shift (@args);
foreach (@args) {
	if($_ eq '-h'){
		$show_header = 1;
	} elsif ($_ eq '-f') {
		$show_footer = 1;
	} elsif ($_ eq '32') {
		print "ERROR:\n";
		print "ERROR : This Script is only for 64 bit mem file generation\n";
		print "ERROR : use src/tools/misc/srec2mem executable instead for 32 bit mem file generation\n";
		print "ERROR :\n";
		exit ;
	} elsif ($_ eq '-b') {
		print "WARNING :\n";
		print "WARNING : The script picks up Address from the srec file so explicit mention of \" base address\" is not required \n";
		print "WARNING :\n";
	}else {
		print "ERROR:Invalid Argument\n";
		exit ;
	}
}

my $fhin;

open $fhin, "<$fname";

################################## srec file format ##############################
#  
#  |type(2)|count(2)|address(8)|data(count - 8 -2) | checksum (2)   ##############
#
#  ###############################################################################

my $type;
my $count;
my $data_count;
my $address;
my $data;
my $checksum;
my $next_address;
my $last_address;
my %srec_data;
my $data_lower;
my $data_upper;
my $next_data;

while(<$fhin>) {
	chomp;
	$type = substr($_, 0, 2);
	$count = substr($_, 2, 2);
	# count is a hex value
	$count = "0x" . $count;      
	$count = Math::BigInt->new($count);
	# count  is no of bytes. To get the no of nibbles multiply it with 2.
	$count = $count * 2;
	if ($type eq "S0") {
		$address = substr($_, 4, 4);
		die "$address for S0 record is not \"0000\"" if($address ne "0000");
		$data = substr($_, 12, ($count - 8 - 2));
		$checksum = substr($_, -2, 2);
		#subtract the length of checksum
		$count = $count - 2;
		# subtract the length of the address field
		$data_count = $count - 4;
		my $module_name = substr($_, 8, 20);
		$module_name = hex2ascii($module_name);
		
		my $ver = substr($_, 28, 2);
		$ver = hex2ascii($ver);
		
		my $rev = substr($_, 30, 2);
		$rev = hex2ascii($rev);
		
		$data_count = $data_count - 24;
		my $description;
		if($data_count) {
			$description  = substr($_, 32, $data_count);
			$description = hex2ascii($description);
		}
		if($show_header == 1) {
			print "module_name 	: $module_name\n";
			print "module_ver  	: $ver\n";
			print "module_rev	: $rev\n";
			print "module_desc	: $description\n";
		}
	}
	
	my $no_data_record; 
	my $execution_start_address;
	if($type eq "S5") {
		#subtract the checksum length
		$count = $count - 2;
		$no_data_record = Math::BigInt->new("0x" . substr($_, 4, $count));
		if ($show_footer == 1) {
			print "No Of Data Record : $no_data_record\n";
		}
	}
	
	if($type eq "S7") {
		#subtract the checksum length
		$count = $count - 2;
		$execution_start_address = substr($_, 4, $count);
		if ($show_footer == 1) {
			printf "Execution Start Address : 0x%08X\n", $execution_start_address;
		}
	}
	
	if($type eq "S3") {
		$address = substr($_, 4, 8);
		$address = "0x" . $address;
		$address = Math::BigInt->new($address);
		$data = substr($_, 12, ($count - 8 - 2));
		$checksum = substr($_, -2, 2);
		$count = $count - 2;
		
		# subtract the length of the address field
		$data_count = $count - 8;
		
		for (my $i = 0; $i < $data_count; $i = $i + 16) {
			$next_address = $address + ($i/2);
			$next_data = substr($data, $i, 16);
		
			if((length $next_data) < 16 ) {
				my $length = (length $next_data);
				$data_lower = substr($next_data, 8 , $length);
				$data_lower = unpack ("H*", pack "H8", $data_lower);
				$data_upper = substr($next_data, 0, 8);
			} else {
				$data_lower = substr($next_data, 8, 16);
				$data_upper = substr($next_data, 0, 8);
			}
			$next_data = $data_lower . $data_upper;
			$srec_data{$next_address} = $next_data;
		}
		$last_address = $next_address;
	}
}

close $fhin;

#generate the .mem file

my $output_file;

($output_file = $fname) =~ s/\.srec/\.mem/;

my $fhout;

open $fhout, "> $output_file";

for(my $j = 0; $j <= ($last_address); $j = ($j + 8)) {
	printf $fhout "%x/",($j/8);
	print $fhout "$srec_data{$j};\n";
}

close $fhout;


sub hex2ascii {
	my $hex_string = shift;
	my $ascii;
	my @hex = split /(.{2})/, $hex_string;
	foreach(@hex){
		$ascii .= chr(hex($_));
	}
	return $ascii;
}
