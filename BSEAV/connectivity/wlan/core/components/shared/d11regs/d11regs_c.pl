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
#	File Name:	d11regs_c.pl
#	<<Broadcom-WL-IPTag/Proprietary:>>
#	Abstract:
#
#
#	$History:$
#
#********************************************************************
#
# Auto-generation of the following files:
#       - d11regs.c
#		 [similar to d11shm counterpart]
# Usage: perl d11regs_c.pl = --ucode_type=<ucode type> --d11rev=<d11 rev> \
#                           --i_d11regs_partial=d11regs_partial.c \
#                           --o_d11regs_c=d11regs.c
#

use strict;
use warnings;

use Getopt::Long;

my $ucode_type;
my $d11rev;
my $code_found = 0;
my $d11regs_onetime = 1;

my $d11regs_file_partial;
my $d11regs_file_c;

our $VERBOSE = '';

GetOptions ("verbose=s" => \$VERBOSE,
		"ucode_type=s" => \$ucode_type,
		"d11rev=s" => \$d11rev,
		"i_d11regs_partial=s" => \$d11regs_file_partial,
		"o_d11regs_c=s" => \$d11regs_file_c);

if (-e $d11regs_file_c) {
	$d11regs_onetime = 0;
}

open(FILE_D11REGS_C_PARTIAL, $d11regs_file_partial) ||
	die "$0: Error: $d11regs_file_partial: $!";
open(FILE_D11REGS_C, '>>'.$d11regs_file_c) ||
	die "$0: Error: $d11regs_file_c: $!";

sub d11print {
	print("DEBUG: $_[0]\n") if $VERBOSE;
}

if ($d11regs_onetime == 1) {
	print FILE_D11REGS_C "#include <typedefs.h>\n";
	print FILE_D11REGS_C "#include <bcmdefs.h>\n";
	print FILE_D11REGS_C "#include \"d11regs_declarations.h\"\n\n";
	print FILE_D11REGS_C "#define INVALID 0xFFFF\n\n";
}

print FILE_D11REGS_C "\n\/\* $ucode_type d11rev$d11rev related structures \*\/\n";

while (my $line = <FILE_D11REGS_C_PARTIAL>) {
	my $text = $line;

	# Search for begin of code to copy
	if ($code_found == 0) {
		if ($text =~ /static const d11regs/) {
			d11print("code found!");
			$code_found = 1;
		} else {
			next;
		}
	}

	print FILE_D11REGS_C $line;
}

close FILE_D11REGS_C_PARTIAL;
close FILE_D11REGS_C;
