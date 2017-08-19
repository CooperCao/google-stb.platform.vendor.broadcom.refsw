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
#	File Name:	d11regs.pl
#	<<Broadcom-WL-IPTag/Proprietary:>>
#	Abstract:
#		 [similar to d11shm counterpart]
#
#	$History:$
#
#********************************************************************
#
# Auto-generation of the following files:
#       - d11regs_defaults.h         - Contains the default defs of regdefs
#       - d11regs_structs_decl.h     - Contains the feature specific regdefs structures
#       - d11regs_func_decl.h        - Declarations of the above functions
#       - d11regs_declarations.h     - Declarations of regdefs pointers
#       - d11regs_structs_inits.c    - Contains the inits of feature specific regdefs structures
#       - d11regs_main_structs.c     - Functions for switching the regdefs structures
#       - d11regs.h                  - Contains M_* macros used in the code
#       - d11regs_regdefs_t.h        - Contains the main regdefs structure
#
# Usage: perl d11regs.pl = --ucode_type=<ucode type> --d11rev=<d11 rev> \
#                       --named_init=<named init of structs> \
#			--i_d11regs_template=<d11regs template file> \
#			--i_d11regs_count=<number of revisions> \
#			--o_d11regs_defaults=d11regs_defaults.h \
#			--o_d11regs_structs_decl=d11regs_structs_decl.h \
#			--o_d11regs_regdefs_t=d11regs_regdefs_t.h \
#			--o_d11regs_structs_inits=d11regs_structs_inits.c \
#			--o_d11regs_main_structs=d11regs_main_structs.c \
#			--o_d11regs_declarations=d11regs_declarations.h \
#			--o_d11regs_hdr=d11regs.h\
#			--o_d11regs_func_partial=d11regs_func_<ucode type>_partial.c
#

use strict;
use warnings;
use Getopt::Long;

my $ucode_type;
my $d11rev;
my $d11regs_named_init;

my $d11regs_file_template;
my $d11regs_file_defaults;
my $d11regs_file_structs_decl;
my $d11regs_file_regdefs_t;
my $d11regs_file_structs_inits;
my $d11regs_file_main_structs;
my $d11regs_file_declarations;
my $d11regs_file_hdr;
my $d11regs_file_func_partial;
my $d11regs_count;

our $VERBOSE = '';
GetOptions ("verbose=s" => \$VERBOSE,
		"ucode_type=s" => \$ucode_type,
		"d11rev=s" => \$d11rev,
		"named_init=s" => \$d11regs_named_init,
		"i_d11regs_count=s" => \$d11regs_count,
		"i_d11regs_template=s" => \$d11regs_file_template,
		"o_d11regs_defaults=s" => \$d11regs_file_defaults,
		"o_d11regs_structs_decl=s" => \$d11regs_file_structs_decl,
		"o_d11regs_regdefs_t=s" => \$d11regs_file_regdefs_t,
		"o_d11regs_structs_inits=s" => \$d11regs_file_structs_inits,
		"o_d11regs_main_structs=s" => \$d11regs_file_main_structs,
		"o_d11regs_declarations=s" => \$d11regs_file_declarations,
		"o_d11regs_hdr=s" => \$d11regs_file_hdr,
		"o_d11regs_func_partial=s" => \$d11regs_file_func_partial);

my $d11regs_onetime = 1;
my $d11regs_func = join "_", "d11regs_struct", $ucode_type, "d11rev", $d11rev;

if (-e $d11regs_file_structs_inits) {
	$d11regs_onetime = 0;
}

open(FILE_D11REGS_TPL, $d11regs_file_template) ||
	die "$0: Error: $d11regs_file_template: $!";
open(FILE_D11REGS_STRUCTS_INITS, '>'.$d11regs_file_structs_inits) ||
	die "$0: Error: $d11regs_file_structs_inits: $!";
open(FILE_D11REGS_MAIN_STRUCTS, '>>'.$d11regs_file_main_structs) ||
	die "$0: Error: $d11regs_file_main_structs: $!";
open(FILE_D11REGS_FUNC_PARTIAL, '>>'.$d11regs_file_func_partial) ||
	die "$0: Error: $d11regs_file_func_partial: $!";

if ($d11regs_onetime == 1) {
	open(FILE_D11REGS_DEFAULTS, '>'.$d11regs_file_defaults) ||
		die "$0: Error: $d11regs_file_defaults: $!";
	open(FILE_D11REGS_STRUCTS_DECL, '>'.$d11regs_file_structs_decl) ||
		die "$0: Error: $d11regs_file_structs_decl: $!";
	open(FILE_D11REGS_REGDEFS_T, '+>'.$d11regs_file_regdefs_t) ||
		die "$0: Error: $d11regs_file_regdefs_t: $!";
	open(FILE_D11REGS_DECL, '>'.$d11regs_file_declarations) ||
		die "$0: Error: $d11regs_file_declarations: $!";
	open(FILE_D11REGS_HDR, '>'.$d11regs_file_hdr) ||
		die "$0: Error: $d11regs_file_hdr: $!";
}

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

my $regs;
my $regs_id;
my $regs_line;
my $regs_type;
my $in_feature_blk = 0;
my $feature;
my $feature_t;
my $feature_ptr;
my $feature_struct;
my $config;
my $config_present = 0;

print FILE_D11REGS_MAIN_STRUCTS "\n\/\* regdefs structure for $ucode_type - $d11rev \*\/\n";
print FILE_D11REGS_MAIN_STRUCTS "static const regdefs_t $d11regs_func = \n\{\n";

print FILE_D11REGS_FUNC_PARTIAL "\tcase $d11rev\:\n";
print FILE_D11REGS_FUNC_PARTIAL "\t\t*regdefs = &$d11regs_func;\n";
print FILE_D11REGS_FUNC_PARTIAL "\t\tbreak;\n";

if ($d11regs_onetime == 1) {
	print FILE_D11REGS_DEFAULTS "#ifndef _D11REGS_DEFAULTS_H\n";
	print FILE_D11REGS_DEFAULTS "#define _D11REGS_DEFAULTS_H\n\n";

	print FILE_D11REGS_STRUCTS_DECL "#ifndef _D11REGS_STRUCTS_DECL_H\n";
	print FILE_D11REGS_STRUCTS_DECL "#define _D11REGS_STRUCTS_DECL_H\n\n";
	print FILE_D11REGS_STRUCTS_DECL "#include <typedefs.h>\n\n";

	print FILE_D11REGS_DECL "#ifndef _D11REGS_DECLARATIONS_H\n";
	print FILE_D11REGS_DECL "#define _D11REGS_DECLARATIONS_H\n\n";
	print FILE_D11REGS_DECL "#include \"d11regs_structs_decl.h\"\n";
	print FILE_D11REGS_DECL "#include \"d11regs_func_decl.h\"\n\n";

	print FILE_D11REGS_HDR "#ifndef _D11REGS_HDR_H\n";
	print FILE_D11REGS_HDR "#define _D11REGS_HDR_H\n\n";
	print FILE_D11REGS_HDR "#include \"d11regs_declarations.h\"\n\n";
	print "GENERATING FOR $d11regs_count corerevs\n";
	# for dongle build which has only 1 revision, there is a special processing to
	# generate reg structure and corresponding macros
	if ($d11regs_count == 1) {
		print FILE_D11REGS_HDR "#ifdef DONGLEBUILD\n\n";
		print FILE_D11REGS_HDR "#include \"d11regs_offs_struct.h\"\n\n";
		print FILE_D11REGS_HDR "#else /* DONGLEBUILD */\n\n";
	}
	print FILE_D11REGS_HDR "typedef volatile uint8 d11regs_t;\n\n";
}


print FILE_D11REGS_STRUCTS_INITS "#include \"d11regs_structs_decl.h\"\n";
print FILE_D11REGS_STRUCTS_INITS "#include \"d11regs_defaults.h\"\n\n";

if ($d11regs_onetime == 1) {
	print FILE_D11REGS_REGDEFS_T "typedef struct regdefs_struct {\n";
}

while(my $line = <FILE_D11REGS_TPL>) {
	# Skip the blank lines
	if ($line =~ /^\s*$/) {
		next;
	}

	# Search of begin of feature block
	if ($line =~ /FEATURE_BEGIN/) {
		if ($in_feature_blk == 1) {
			die "\nFEATURE_END: nested blocks not allowed!\n";
		}

		# Extracting name of feature block
		$line =~ m/FEATURE_BEGIN\[(.*)\]/;
		$feature = trim($1);
		$feature_t = join "", "d11regs_", $feature, "_t";
		$feature_ptr = join "", $feature, "_ptr";
		$feature_struct = join "_", $feature, $ucode_type, $d11rev;

		d11print("Found begin of feature block:[$feature]");
		$in_feature_blk = 1;

		if ($d11regs_onetime == 1) {
			print FILE_D11REGS_STRUCTS_DECL "typedef struct {\n";
		}

		# Search for config string of the feature block
		$line = <FILE_D11REGS_TPL>;

		if ($line =~ m/IF\[.*\]/) {
			$line =~ m/IF\[(.*)\]/;
			$config = $1;
			d11print("Found config for this block: $config");

			print FILE_D11REGS_STRUCTS_INITS "#if $config\n";
			print FILE_D11REGS_STRUCTS_INITS "static const $feature_t $feature_struct = {\n";
			$config_present = 1;
			next;
		} else {
			print FILE_D11REGS_STRUCTS_INITS "static const $feature_t $feature_struct = {\n";
			$config_present = 0
		}
	}

	# Search for end of feature block
	if ($line =~ /FEATURE_END/) {
		if ($in_feature_blk == 0) {
			die "\nFEATURE_END: nested blocks not allowed!\n";
		}

		if ($d11regs_onetime == 1) {
			print FILE_D11REGS_STRUCTS_DECL "} $feature_t;\n\n";
			print FILE_D11REGS_REGDEFS_T "\tconst $feature_t *$feature;\n";
		}

		print FILE_D11REGS_STRUCTS_INITS "};\n";
		if ($config_present == 1) {
			print FILE_D11REGS_STRUCTS_INITS "#endif\n";
			if ($d11regs_onetime == 1) {
				print FILE_D11REGS_STRUCTS_INITS "const $feature_t *$feature_ptr =\n";
				print FILE_D11REGS_STRUCTS_INITS "#if $config\n";
				print FILE_D11REGS_STRUCTS_INITS "&$feature_struct;\n";
				print FILE_D11REGS_STRUCTS_INITS "#else\n";
				print FILE_D11REGS_STRUCTS_INITS "NULL;\n";
				print FILE_D11REGS_STRUCTS_INITS "#endif\n\n\n";
			}

			if ($d11regs_named_init eq "1") {
				print FILE_D11REGS_MAIN_STRUCTS "\t.$feature = \n";
			}
			print FILE_D11REGS_MAIN_STRUCTS "\t#if $config\n";
			print FILE_D11REGS_MAIN_STRUCTS "\t&$feature_struct,\n";
			print FILE_D11REGS_MAIN_STRUCTS "\t#else\n";
			print FILE_D11REGS_MAIN_STRUCTS "\tNULL,\n";
			print FILE_D11REGS_MAIN_STRUCTS "\t#endif\n";
		} else {
			if ($d11regs_onetime == 1) {
				print FILE_D11REGS_STRUCTS_INITS "const $feature_t *$feature_ptr = &$feature_struct;\n\n\n";
			}

			if ($d11regs_named_init eq "1") {
				print FILE_D11REGS_MAIN_STRUCTS "\t.$feature = &$feature_struct,\n";
			} else {
				print FILE_D11REGS_MAIN_STRUCTS "\t&$feature_struct,\n";
			}
		}

		d11print("Found end of feature block");
		$in_feature_blk = 0;
		$config_present = 0;
		next;
	}

	# Skip the lines outside the feature block
	if ($in_feature_blk == 0) {
	       next;
	}

	# Strip out the comments and white spaces
	$regs_line = $line;
	$regs_line =~ s/\#.*//;
	$regs_line = trim($regs_line);

	if ($regs_line =~ /(\w+).(\w+)/) {
		$regs = $2;
		$regs_type = $1;
	}
	$regs_id = join "", $regs, "_ID";
	#$regs_id = "D11_$regs";

	d11print(" \tFound regs: $regs");

	if ($d11regs_onetime == 1) {
		print FILE_D11REGS_STRUCTS_DECL "\tuint16 $regs_id;\n";
	}

	if ($d11regs_named_init eq "1") {
		print FILE_D11REGS_STRUCTS_INITS "\t.$regs_id = $regs, /* $regs_type */\n";
	} else {
		print FILE_D11REGS_STRUCTS_INITS "\t$regs,\n";
	}

	if ($d11regs_onetime == 1) {
		print FILE_D11REGS_HDR "#define D11_$regs(ptr) ((volatile $regs_type *)(((volatile uint8*) ((ptr)->regs)) + ((ptr)->regoffsets->$feature->$regs_id)))\n";
		print FILE_D11REGS_HDR "#define D11_${regs}_ALTBASE(base, offstbl) ((volatile $regs_type *)(((volatile uint8*) (base)) + (offstbl->$feature->$regs_id)))\n";
		print FILE_D11REGS_HDR "#define D11_${regs}_OFFSET(ptr) ((ptr)->regoffsets->$feature->$regs_id)\n";

		print FILE_D11REGS_DEFAULTS "#ifndef $regs\n";
		print FILE_D11REGS_DEFAULTS "#define $regs INVALID\n";
		print FILE_D11REGS_DEFAULTS "#endif\n";
	}
}

print FILE_D11REGS_MAIN_STRUCTS "\};\n";

if ($d11regs_onetime == 1) {
	print FILE_D11REGS_REGDEFS_T "} regdefs_t;\n";

	# Copy regdefs_t structure to d11regs_structs_decl.h
	seek FILE_D11REGS_REGDEFS_T, 0, 0;
	foreach(<FILE_D11REGS_REGDEFS_T>){
		 print FILE_D11REGS_STRUCTS_DECL $_;
	}
}

if ($d11regs_onetime == 1) {
	print FILE_D11REGS_DEFAULTS "\n#endif\n";
	print FILE_D11REGS_STRUCTS_DECL "\n#endif\n";
	print FILE_D11REGS_DECL "\n#endif\n";
	if ($d11regs_count == 1) {
		print FILE_D11REGS_HDR "#endif /* else-DONGLEBUILD */\n\n";
	}
	print FILE_D11REGS_HDR "\n#endif\n";

	close FILE_D11REGS_DEFAULTS;
	close FILE_D11REGS_DECL;
	close FILE_D11REGS_HDR;

	close FILE_D11REGS_REGDEFS_T;
}

close FILE_D11REGS_TPL;
close FILE_D11REGS_STRUCTS_INITS;
close FILE_D11REGS_FUNC_PARTIAL;
