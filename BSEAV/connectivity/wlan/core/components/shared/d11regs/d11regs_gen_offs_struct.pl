#********************************************************************
#	THIS INFORMATION IS PROPRIETARY TO
#	BROADCOM CORP.
#-------------------------------------------------------------------
#
#			Copyright (c) 2002 Broadcom Corp.
#					ALL RIGHTS RESERVED
#
#********************************************************************
#********************************************************************
#	File Name:	d11regs_gen_offs_struct.pl
#	<<Broadcom-WL-IPTag/Proprietary:>>
#	Abstract:
#		 [similar to d11shm counterpart]
#
#	$History:$
#
#********************************************************************
#
# Auto-generation of the following files: [mka tbd ] 
#       - d11regs_offs_struct.h      - Contains register map structure
#
# Usage: perl d11regs.pl = --named_init=<named init of structs> \
#			--i_d11regs_offs_c=<d11regs offsets file> \
#			--i_d11regs_structs_inits=d11regs_structs_inits.c \
#			--i_d11regs_count=<number of revisions> \
#			--o_d11regs_offs_struct_h=d11regs_offs_struct.h
#
#	- note that this script will execute ONLY if there is a single revision
#

use strict;
use warnings;
use Getopt::Long;

my $d11regs_named_init;

my $d11regs_offs_c;
my $d11regs_offs_struct_h;
my $d11regs_file_structs_inits;
my $d11regs_count;

our $VERBOSE = '';
GetOptions ("named_init=s" => \$d11regs_named_init,
		"i_d11regs_offs_c=s" => \$d11regs_offs_c,
		"i_d11regs_structs_inits=s" => \$d11regs_file_structs_inits,
		"i_d11regs_count=s" => \$d11regs_count,
		"o_d11regs_offs_struct_h=s" => \$d11regs_offs_struct_h);

if ($d11regs_count > 1) {
	print "NOT generating structs for revisions:$d11regs_count being > 1\n";
	exit(0);
}

open(FILE_D11REGS_OFFS_C, $d11regs_offs_c) ||
	die "$0: Error: $d11regs_offs_c: $!";
open(FILE_D11REGS_STRUCTS_INITS, $d11regs_file_structs_inits) ||
	die "$0: Error: $d11regs_file_structs_inits: $!";
open(FILE_D11REGS_OFFS_STRUCT_H, '>'.$d11regs_offs_struct_h) ||
	die "$0: Error: $d11regs_offs_struct_h: $!";

# Trim function - remove leading and trailing whitespace
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}

sub d11print {
	print("DEBUG: $_[0]\n") if $VERBOSE;
}

my @offs_tbl;
my $temp_line;
my $name;
my $offs;
my @sizes;
my @sizes_check_name;
my $idx = 0;

#parse FILE_D11REGS_STRUCTS_INITS line by line to get sizes
while(my $line = <FILE_D11REGS_STRUCTS_INITS>) {
	# Skip the blank lines
	if ($line =~ /^\s*$/) {
		next;
	}

	# match line which contains offsets
	if ($line =~ /\.(\w+)\s=\s(\w+),\s\/\*\s(\w+)\s\*\//) {
		push @sizes, $3;
		push @sizes_check_name, $1;
	}
}

my @macros;
my $sz_macro;
my $reg_name;

#parse FILE_D11REGS_OFFS_C line by line to make an array with offsets
while(my $line = <FILE_D11REGS_OFFS_C>) {
	# Skip the blank lines
	if ($line =~ /^\s*$/) {
		next;
	}

	# match line which contains offsets
	if ($line =~ /\s\.(\w+)\s=\s\((\w+)\)/) {
		$name = $1;
		$offs = sprintf('%4x', hex($2));
		d11print "$1 -> $2  $sizes[$idx] $sizes_check_name[$idx]\n";
		# create tbls with format: $offs $name $sizes[$idx] $sizes_check_name[$idx]
		$temp_line = "$offs $name $sizes[$idx] $sizes_check_name[$idx]\n";
		push @offs_tbl, $temp_line;
		# double check
		if ($name ne $sizes_check_name[$idx]) {
			die "ERROR:d11regs:name_mismatch: $name, $sizes_check_name[$idx]!\n";
		}
		$sz_macro = $sizes[$idx];
		$reg_name = substr($name, 0, length($name)-3); # remove _ID from end
		# write macros
		push @macros, "#define D11_$reg_name(ptr) ((volatile $sz_macro *)&((ptr)->regs->$name))\n";
		push @macros, "#define D11_${reg_name}_ALTBASE(base, offstbl) ((volatile $sz_macro *)(((volatile uint8*) (base)) + OFFSETOF(d11regs_t, $name)))\n";
		push @macros, "#define D11_${reg_name}_OFFSET(ptr) (OFFSETOF(d11regs_t, $name))\n";
		$idx = $idx + 1;
	}
	# match invalid's to increase idx
	if ($line =~ /\s\.(\w+)\s=\sINVALID/) {
		$reg_name = substr($1, 0, length($1)-3); # remove _ID from end;
		$sz_macro = "uint16";
		# write macros
		push @macros, "#define D11_$reg_name(ptr) ((volatile $sz_macro *)&((ptr)->regs->INVALID_ID))\n";
		push @macros, "#define D11_${reg_name}_ALTBASE(base, offstbl) ((volatile $sz_macro *)(((volatile uint8*) (base)) + OFFSETOF(d11regs_t, INVALID_ID)))\n";
		push @macros, "#define D11_${reg_name}_OFFSET(ptr) (OFFSETOF(d11regs_t, INVALID_ID))\n";
		$idx = $idx + 1;
	}
}
# write invalid field corr to invalid offset [0xffff] to structure
$offs = sprintf('%4x', hex("0xfffc"));
$temp_line = "$offs INVALID_ID uint16 INVALID_ID\n";
push @offs_tbl, $temp_line;

# sort tbl
@offs_tbl = sort @offs_tbl;
$offs = 0;
my $offs_hex = 0;
my $prev_offs_hex = 0;
my $prev_sz = 0;
my $cur_offs_pad_str;
my $cur_offs_str;

# start writing into file
print FILE_D11REGS_OFFS_STRUCT_H "#ifndef _D11REGS_OFFS_STRUCT_H\n";
print FILE_D11REGS_OFFS_STRUCT_H "#define _D11REGS_OFFS_STRUCT_H\n\n";
print FILE_D11REGS_OFFS_STRUCT_H "typedef volatile struct _d11regs {\n";

foreach my $line (@offs_tbl) {
	# format: $offs $name $sizes[$idx] $sizes_check_name[$idx]
	if ($line =~ /(\w+)\s(\w+)\s(\w+)\s(\w+)/) {
		$offs_hex = hex($1);
		d11print "sorted:$1 $2 $3 $4\n";
		my $name = $2; # DONOT substr($2, 0, length($2)-3); # remove _ID from end
		my $sz_to_pad = ($offs_hex - $prev_offs_hex) - $prev_sz;
		# check if sz_to_pad is > 0 and even bytes
		$sz_to_pad = $sz_to_pad/2;
		$cur_offs_pad_str = sprintf('0x%04x', $prev_offs_hex + $prev_sz);
		# check if padding required
		if ($sz_to_pad > 0) {
			print FILE_D11REGS_OFFS_STRUCT_H "\tuint16 PAD[$sz_to_pad]; /* $cur_offs_pad_str */\n";
		}
		# put the struct element
		$cur_offs_str = sprintf('0x%04x', $offs_hex);
		print FILE_D11REGS_OFFS_STRUCT_H "\t$3 $name; /* $cur_offs_str */\n";
		$prev_sz = ($3 eq "uint16") ? 2 : (
				($3 eq "uint32") ? 4 : 0);
		if ($prev_sz == 0) {
			die "ERROR:d11regs:size_error for: $name!\n";
		}
		$prev_offs_hex = $offs_hex;
	}
}
print FILE_D11REGS_OFFS_STRUCT_H "} d11regs_t;\n";

# write macros
print FILE_D11REGS_OFFS_STRUCT_H @macros;

print FILE_D11REGS_OFFS_STRUCT_H "\n#endif /* _D11REGS_OFFS_STRUCT_H */\n";

close FILE_D11REGS_STRUCTS_INITS;
close FILE_D11REGS_OFFS_STRUCT_H;
close FILE_D11REGS_OFFS_C;

