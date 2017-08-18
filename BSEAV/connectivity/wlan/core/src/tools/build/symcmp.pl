#!/usr/local/bin/perl

#
# Takes two symsize files and returns a comparison showing what changed
#
# $Copyright (C) 2005 Broadcom Corporation$
# $Id$

use strict 'vars';

use Getopt::Long;

use constant LINESIZE => 256;	# max acceptable lin size
use constant FALSE => 0;
use constant TRUE => (!FALSE);

my $bMatch = FALSE;
my $bCSV = FALSE;
my $uiLimit = 0;

sub getaline {

	my ($buf,$fp) = @_;

	if (!($buf = <$fp>)) {
		return (-1);
	}

	if (length($buf) < 1) {
		return (-1);
	}
	chomp($buf);

	return (len);
}

sub pr {
	my ($output, $func, $size1, $size2) = @_;

	my $delta = $size2 - $size1;
	
	my $comment = "";
	SWITCH: {
		($size1 == 0) && do {
			$comment = "(added)";
			last SWITCH;
		};
		($size2 == 0) && do {
			$comment = "(removed)";
			last SWITCH;
		};
	}

	if ($bCSV) {
		printf $output "%s,%d,%d,%d\n",
			$func, $size1, $size2, $delta;
	} else {
		printf $output "%-24s\t%6d\t%6d\t%+6d\t%s\n",
			$func, $size1, $size2, $delta, $comment;
	}
}

sub printNotCSV {
	my $output = shift(@_);
	if (!$bCSV) {
		print $output @_;
	}
}

sub printfNotCSV {
	my $output = shift(@_);
	if (!$bCSV) {
		printf $output @_;
	}
}

sub smatch {
	my ($name1,$name2) = @_;

	my $baselen = index($name1, ".");

	if (($baselen == index($name2, ".")) &&
		(substr($name1,0,$baselen) eq substr($name2,0,$baselen)) &&
		(substr($name1,$baselen) =~ /^[.0123456789]*$/) &&
		(substr($name2,$baselen) =~ /^[.0123456789]*$/)) {
		return 1;
	} else {
		return 0;
	}
}

sub convert {
	my ($i1, $i2, $o) = @_;

	# fill hashes
	my %hSize1;
	my %hSize2;
	my $NewSize = 0;
	while (<$i1>) {
		if (/(\S+)\s+([0-9]+)/) {
#			if (exists($hSize1{$1}) && ($hSize1{$1} != $2)) {
#				die("Error: multiple entries in input file 1 for $1 with different sizes ($hSize1{$1}, $2)\n");
#			}
			my $func = $1;
			my $size = $2;
			# skip unused memory after reclaim section
			if ($func !~ /^_rend[12]$/) {
				$hSize1{$func} += $size;
			}
		} else {
			printNotCSV($o,"ERROR: $_ doesn't match\n");
		}
	}
	my $bReclaim=FALSE;
	my $ReclaimSize=0;
	while (<$i2>) {
		if (/(\S+)\s+([0-9]+)/) {
#			if (exists($hSize2{$1}) && ($hSize2{$1} != $2)) {
#				die("Error: multiple entries in input file 2 for $1 with different sizes ($hSize2{$1}, $2)\n");
#			}
			my $func = $1;
			my $size = $2;

			# skip unused memory after reclaim section
			if ($func !~ /^_rend[12]$/) {
				$hSize2{$func} += $size;
				$NewSize += $size;
			}

			# calculate size of reclaim section
			if ($func =~ /^_rstart[12]$/) {
				$bReclaim=TRUE;
			} elsif ($func =~ /^_rend[12]$/) {
				$bReclaim=FALSE;
			}
			if ($bReclaim) {
				$ReclaimSize += $size;
			}

		} else {
			printNotCSV($o,"ERROR: $_ doesn't match\n");
		}
	}

	# eliminate grouped entries (*.#####) with common sizes
	my %hGroupSize1;
	my %hGroupSize2;
	my %hhaGroup1;
	for my $func1 (sort(keys(%hSize1))) {
		if ($func1 =~ /([^.]+)\.[.0-9]+$/) {
			my $Group = $1;
			$hGroupSize1{$Group} += $hSize1{$func1};
			push(@{$hhaGroup1{$Group}{$hSize1{$func1}}},$func1);
		}
	}
	for my $func2 (sort(keys(%hSize2))) {
		if ($func2 =~ /([^.]+)\.[.0-9]+$/) {
			my $Group = $1;
			$hGroupSize2{$Group} += $hSize2{$func2};
			if ($bMatch && exists($hhaGroup1{$Group}{$hSize2{$func2}}) &&
				(scalar(@{$hhaGroup1{$Group}{$hSize2{$func2}}}) > 0)) {
				# we found a match - throw them both out of the future comparison
				my $func1 = shift(@{$hhaGroup1{$Group}{$hSize2{$func2}}});
				delete($hSize1{$func1});
				delete($hSize2{$func2});
			}
		}
	}

	my $reduced = 0;
	my $added = 0;
	my %hhDeltas;
	# check for text/data which changed size or was added
	for my $func (sort(keys(%hSize1))) {
		if (exists($hSize2{$func})) {
			my $diff = $hSize2{$func} - $hSize1{$func};
			if ($diff > 0) {
				$added += $diff;
			} else {
				$reduced -= $diff;
			}

			if ($diff != 0) {
				$hhDeltas{$diff}{$func}{oldsize} = $hSize1{$func};
				$hhDeltas{$diff}{$func}{newsize} = $hSize2{$func};
			}
		} else {
			# symbol removed
			$reduced += $hSize1{$func};
			$hhDeltas{-$hSize1{$func}}{$func}{oldsize} = $hSize1{$func};
			$hhDeltas{-$hSize1{$func}}{$func}{newsize} = 0;
		}
	}

	for my $func (sort(keys(%hSize2))) {
		if (!exists($hSize1{$func})) {
			# symbol added (new)
			$added += $hSize2{$func};
			$hhDeltas{$hSize2{$func}}{$func}{oldsize} = 0;
			$hhDeltas{$hSize2{$func}}{$func}{newsize} = $hSize2{$func};
		}
	}

	printNotCSV($o,"SYMBOL COMPARISON\n");
	printNotCSV($o,"-----------------\n");

	printNotCSV($o,"Unique Symbols:");	
	if ($uiLimit) {
		printNotCSV($o," (limited to $uiLimit symbols)\n");
	} else {
		printNotCSV($o,"\n");
	}

	my $UniqueAdded = 0;
	my $UniqueReduced = 0;

	# print the non-grouped symbols in declining order of "added"
	my $index = 0;
	for my $diff (sort{$b <=> $a}(keys(%hhDeltas))) {
		for my $func (sort(keys(%{$hhDeltas{$diff}}))) {
			if ($func !~ /([^.]+)\.[.0-9]+$/) {
				if (!$uiLimit || ($index++ < $uiLimit)) {
					pr ($o, $func, $hhDeltas{$diff}{$func}{oldsize}, $hhDeltas{$diff}{$func}{newsize});
				}
				if ($diff > 0) {
					$UniqueAdded += $diff;
				} else {
					$UniqueReduced -= $diff;
				}
				delete($hhDeltas{$diff}{$func});
			}
		}
	}

	printfNotCSV($o,"\nUnique diff %+d\tadded %+d\treduced %+d\n",
		($UniqueAdded - $UniqueReduced), $UniqueAdded, -$UniqueReduced);

	# process group diffs
	if (scalar(keys(%hhDeltas)) > 0) {
		printNotCSV($o,"\n\nGroup Symbols:");
	}
	if ($uiLimit) {
		printNotCSV($o," (limited to $uiLimit symbols)\n");
	} else {
		printNotCSV($o,"\n");
	}

	
	$index = 0;
	# print the grouped symbols in declining order of "added"
	for my $diff (sort{$b <=> $a}(keys(%hhDeltas))) {
		for my $func (sort(keys(%{$hhDeltas{$diff}}))) {
			if ($func =~ /([^.]+)\.[.0-9]+$/) {
				if ((!$uiLimit || ($index++ < $uiLimit)) && !$bCSV) {
					pr ($o, $func, $hhDeltas{$diff}{$func}{oldsize}, $hhDeltas{$diff}{$func}{newsize});
				}
			}
		}
	}

	$index = 0;
	if ((scalar(keys(%hGroupSize1)) > 0) || (scalar(keys(%hGroupSize2)) > 0)) {
		printNotCSV($o,"\n\nGroup Diffs:");
		if ($uiLimit) {
			printNotCSV($o," (limited to $uiLimit groups)\n");
		} else {
			printNotCSV($o,"\n");
		}

		my $GroupAdded = 0;
		my $GroupReduced = 0;	
		# groups in both files
		for my $Group (sort(keys(%hGroupSize1))) {
			if (exists($hGroupSize2{$Group})) {
				my $diff = $hGroupSize2{$Group} - $hGroupSize1{$Group};
				if ($diff != 0) {
					if ((!$uiLimit || ($index++ < $uiLimit)) && !$bCSV) {
						pr($o, $Group, $hGroupSize1{$Group}, $hGroupSize2{$Group}, $diff, "");
					}
				}
				if ($diff > 0) {
					$GroupAdded += $diff;
				} else {
					$GroupReduced -= $diff;
				}
				delete($hGroupSize1{$Group});
				delete($hGroupSize2{$Group});
			}
		}
	
		# groups only in first file
		for my $Group (sort(keys(%hGroupSize1))) {
			if ((!$uiLimit || ($index++ < $uiLimit)) && !$bCSV) {
				pr($o, $Group, $hGroupSize1{$Group}, 0, -$hGroupSize1{$Group}, "(removed)");
			}
			$GroupReduced += $hGroupSize1{$Group};
		}
	
	
		# groups only in second file
		for my $Group (sort(keys(%hGroupSize2))) {
			if ((!$uiLimit || ($index++ < $uiLimit)) && !$bCSV) {
				pr($o, $Group, 0, $hGroupSize2{$Group}, $hGroupSize2{$Group}, "(added)");
			}
			$GroupAdded += $hGroupSize2{$Group};
		}
	
		printfNotCSV($o,"\nGroup diff %+d\tadded %+d\treduced %+d\n",
			($GroupAdded - $GroupReduced), $GroupAdded, -$GroupReduced);
	}

	printfNotCSV($o,"\n\nFile diff %+d\tadded %+d\treduced %+d\n",
		($added - $reduced), $added, -$reduced);
#	printf $o "Reclaim: %+d\n", $ReclaimSize;
}

sub usage {
	printf STDERR "usage: %s [ -m ] symsizefile1 symsizefile2 [ outfile ]\n", $0;
	printf STDERR "       -m        match symbols <name>.[<dot-digits>] of the same size\n";
	printf STDERR "       -limit n  show up to \"n\" symbols in each category\n";
	printf STDERR "       -csv      output symbols in spreadsheet (.csv) format\n";
	exit(1);
}

#if ((scalar(@ARGV) > 1) && ($ARGV[0] eq "-m")) {
#	$bMatch = TRUE;
#	shift(@ARGV);
#}

GetOptions("csv" => \$bCSV,
		"match" => \$bMatch,
		"limit=i" => \$uiLimit);

if ((scalar(@ARGV) < 2) || (scalar(@ARGV) > 3)) {
	usage();
}
open(INFILE1,"<$ARGV[0]") or die ("ERROR:Cannot open $ARGV[0] for input\n");
open(INFILE2,"<$ARGV[1]") or die ("ERROR:Cannot open $ARGV[1] for input\n");

if (scalar(@ARGV) == 2) {
	convert(\*INFILE1, \*INFILE2, \*STDOUT);
} else {
	open(OUTFILE,">$ARGV[2]") or die ("ERROR:Cannot open $ARGV[2] for output\n");
	convert(\*INFILE1, \*INFILE2, \*OUTFILE);
	close(OUTFILE);
}

close(INFILE1);
close(INFILE2);

exit (0);

