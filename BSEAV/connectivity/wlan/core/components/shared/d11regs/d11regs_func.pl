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
#	File Name:	d11regs_func.pl
#	<<Broadcom-WL-IPTag/Proprietary:>>
#	Abstract:
#
#
#	$History:$
#
#********************************************************************
#
# Auto-generation of the d11regs functions
#	 [similar to d11shm counterpart]
# Usage: perl d11regs_func.pl = --ucode_type=<ucode type> --d11rev=<d11 rev> \
#                              --ucode_type=ucode_<ucode type>\
#                              --i_d11regs_func_partial=d11regs_func_ucode_<ucode type>_partial.c \
#                              --o_d11regs_main_functions=d11regs_main_functions.c \
#                              --o_d11regs_func_decl=d11regs_func_decl.h
#

use strict;
use warnings;

use Getopt::Long;

my $line;

my $ucode_type;
my $code_found;

my $d11regs_file_main_functions;
my $d11regs_file_func_decl;
my $d11regs_file_func_partial_ucode_type;

our $VERBOSE = 'true';

GetOptions ("verbose=s" => \$VERBOSE,
		"ucode_type=s" => \$ucode_type,
		"i_d11regs_func_partial=s" => \$d11regs_file_func_partial_ucode_type,
		"o_d11regs_main_functions=s" => \$d11regs_file_main_functions,
		"o_d11regs_func_decl=s" => \$d11regs_file_func_decl);

open(FILE_D11REGS_FUNC_MAIN,">>".$d11regs_file_main_functions) ||
	die "$0: Error: $d11regs_file_main_functions: $!";
open(FILE_D11REGS_FUNC_DECL, '>>'.$d11regs_file_func_decl) ||
	die "$0: Error: $d11regs_file_func_decl: $!";
open(FILE_D11REGS_C_FUNC_PARTIAL_UCODE_TYPE, $d11regs_file_func_partial_ucode_type) ||
	die "$0: Error: $d11regs_file_func_partial_ucode_type: $!";

sub d11print {
	print("DEBUG: $_[0]\n") if $VERBOSE;
}

print FILE_D11REGS_FUNC_MAIN "\n\/\* $ucode_type regdefs selection function \*\/\n";
print FILE_D11REGS_FUNC_MAIN "int BCMRAMFN(d11regs_select_offsets_tbl)(const regdefs_t **regdefs, int d11rev)\n";
print FILE_D11REGS_FUNC_MAIN "{\n";
print FILE_D11REGS_FUNC_MAIN "\tint ret = 0;\n";
print FILE_D11REGS_FUNC_MAIN "#ifndef DONGLEBUILD\n";
print FILE_D11REGS_FUNC_MAIN "\tswitch(d11rev) {\n";

print FILE_D11REGS_FUNC_DECL "extern void d11regs_select_regdefs(int d11rev);\n";
print FILE_D11REGS_FUNC_DECL "extern int d11regs_select_offsets_tbl(const regdefs_t **regdefs, int d11rev);\n";

while ($line = <FILE_D11REGS_C_FUNC_PARTIAL_UCODE_TYPE>) {
	print FILE_D11REGS_FUNC_MAIN $line;
}

print FILE_D11REGS_FUNC_MAIN "\t\tdefault:\n\t\tret = -1;";
print FILE_D11REGS_FUNC_MAIN "\t}\n";
print FILE_D11REGS_FUNC_MAIN "#endif /* not-DONGLEBUILD */\n";
print FILE_D11REGS_FUNC_MAIN "\treturn ret;\n";
print FILE_D11REGS_FUNC_MAIN "}\n";

close FILE_D11REGS_C_FUNC_PARTIAL_UCODE_TYPE;
close FILE_D11REGS_FUNC_MAIN;
close FILE_D11REGS_FUNC_DECL;
