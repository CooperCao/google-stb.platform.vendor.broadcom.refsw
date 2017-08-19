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
#	File Name:	d11regs_offsets.pl
#	<<Broadcom-WL-IPTag/Proprietary:>>
#	Abstract:
#
#
#	$History:$
#
#********************************************************************
#
# Auto-generation of regsdefs offsets file
#	 [similar to d11shm counterpart]
# Usage: perl d11regs_offsets.pl --ucode_type=<ucode type> \
#                               --i_regsdefs=<regsdefs file> \
#                               --o_regsdefs_offsets=<regsdefs offsets file>
#

use strict;
use warnings;

use Getopt::Long;

my $text;
my $regs;
my $arg_count;
my $ucode_type;

my $d11regs_file_regsdefs;
my $d11regs_file_regsdefs_offsets;

our $VERBOSE = 'true';
GetOptions ("verbose=s" => \$VERBOSE,
		"ucode_type=s" => \$ucode_type,
		"i_regsdefs=s" => \$d11regs_file_regsdefs,
		"o_regsdefs_offsets=s" => \$d11regs_file_regsdefs_offsets);

open(FILE_D11regsDEFS, $d11regs_file_regsdefs) ||
	die "$0: Error: $d11regs_file_regsdefs: $!";
open(FILE_D11regsDEFS_OFFSETS, '>'.$d11regs_file_regsdefs_offsets) ||
	die "$0: Error: $d11regs_file_regsdefs_offsets: $!";

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

print FILE_D11regsDEFS_OFFSETS "#ifndef _D11regsDEFS_OFFSETS_H\n";
print FILE_D11regsDEFS_OFFSETS "#define _D11regsDEFS_OFFSETS_H\n\n";

while (my $line = <FILE_D11regsDEFS>) {
	if ($line =~ /D11_REV\ \=\=\ /) {
		print FILE_D11regsDEFS_OFFSETS $line;
	}


	if ($line =~ /\(\(/) {
		# Extract regs name
		$line =~ m/#define\s+(.*)\s+\(/;
		$regs = trim($1);

		# Extract regs offset
		$line =~ m/\(\(.*\+\((.*)\)\)\*2\)*/;
		$text = join "", "#define\t", "$regs", "_OFFSET\t", "($1 * 2)";

		print FILE_D11regsDEFS_OFFSETS "$text\n";
	}

	if ($line =~ /#endif/) {
		print FILE_D11regsDEFS_OFFSETS $line;
	}
}

print FILE_D11regsDEFS_OFFSETS "\n#endif\n";

close FILE_D11regsDEFS;
close FILE_D11regsDEFS_OFFSETS;
