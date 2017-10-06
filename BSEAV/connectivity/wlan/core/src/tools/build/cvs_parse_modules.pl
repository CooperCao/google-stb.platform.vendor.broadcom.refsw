#!/usr/local/bin/perl

##
## Script to parse through CVS modules file and collapse the listing
## into final list of dirs and files in alphabetical order
##
## $Id$
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## SVN: $HeadURL$
##
###

use Env;
use File::Basename;
use strict;

my @all_modules;
my $modstart_re;
my $module;
my %contents;
my %nonrecurse;
my %cleaned;
my $arg;
my $out;
my $verbose;
my $dbg;
my $help;
my @cvs_modules;     # Requested list of CVS modules
my $keep_duplicates; 	# After all the folders are expanded, leave children
			# modules, even if their parent dirs exist
my @levels=qw(1 2 3 4 5); # Parse modules five times to expand all submodules
my $tag; # Optional cvs tag to apply

# Process command line switches
while  ($ARGV[0]) {
        $arg = shift @ARGV;
        if ($arg =~ /^-(cvsroot|c)/) {
                $CVSROOT = shift @ARGV;
        } elsif ($arg =~ /^-(debug|d)/) {
                $dbg = 1;
        } elsif ($arg =~ /^-(keep_duplicates|k)/) {
                $keep_duplicates = 1;
        } elsif ($arg =~ /^-(module|m)/) {
                push(@cvs_modules,shift @ARGV);
        } elsif ($arg =~ /^-(out|o)/) {
                $out = shift @ARGV;
#        } elsif ($arg =~ /^-(rtag|r)/) {
#                $tag = shift @ARGV;
        } elsif ($arg =~ /^-(verbose|v)/) {
                $verbose = 1;
        } elsif ($arg =~ /^-(help|h)/) {
                $help = 1;
        }
} # while

usage() if ($help);

if (!defined($out) && scalar(@cvs_modules)==0) {
	print "\nERROR: Missing -out or -module cmd line switch\n";
	usage();
}

# Query cvs modules database and place it in @all_modules
my $CVSROOT = "/projects/cvsroot";
my $bsubcmd = "bsub -q sj-hnd -R r64bit cvs rtag TAGNAME";
system("cvs -d $CVSROOT co -c > /tmp/mod.log");

open(CVSMODULESORIG, "/tmp/mod.log");
push(@all_modules, <CVSMODULESORIG>);
chomp(@all_modules);
close(CVSMODULESORIG);
unlink("/tmp/mod.log");

sub Dbg {
	print "Dbg: @_\n" if ($dbg);
}

# If a parent or grandparent dir is part of a module, then remove their children
sub remove_duplicates {
	my ($module) = shift; # Get module to cleanup
	my @uniq;             # Cleaned list of physical dirs
	my @submodules;
	my %smdata={};
	my %seen;

	@submodules = sort grep { !/^\s*$/ } split(/\s+/, $cleaned{$module});
	push(@submodules, $cleaned{$module}) if (scalar(@submodules)==0);
	foreach my $submodule (@submodules) {
		$smdata{$module}{$submodule} = 1;
	}
	foreach my $submodule (@submodules) {
		my $found_parent=0;
		my $parent = $submodule;
		while ($parent = dirname($parent)) {
			if (exists($smdata{$module}{$parent})) {
				$found_parent=1;
				last;
			}
			last if ($parent !~ /\\|\//);
		}
		push(@uniq, ($found_parent==1) ?  $parent : $submodule);
	}
	foreach my $entry (@uniq) {
	}
	@uniq = grep {!$seen{$_}++} sort @uniq;

	return join(' ',@uniq);
} # remove_duplicates

# TODO: Regular modules (non-aliases) with no options specified
# TODO: are not supported. They are not used currently in out env
# TODO: Regular module has "module-name [options] dir [files]" syntax

$modstart_re = '\s+(-a|-d|-l)\s+';

# Collapse module entries from multiple lines to single line and
# populate contents hash
# TODO: Convert module contents to associative arrays
foreach my $line (@all_modules) {
	if ($line =~ /^(\S+)(${modstart_re}.*)$/) {
		$module=$1;
		$module=~s/\s+//g;
		$contents{$module} = " $2 ";
		Dbg("PARSE MODULE($module): $contents{$module}");
	} else {
		$contents{$module} .= " $line ";
	}
}

# Cleanup the module entries from following module options
# Three different line formats are valid in modules file:
#       key     -a      aliases...
#       key [options]   directory
#       key [options]   directory files...
#
# Where "options" are composed of:
#       -i prog         Run "prog" on "cvs commit" from top-level of module.
#       -o prog         Run "prog" on "cvs checkout" of module.
#       -e prog         Run "prog" on "cvs export" of module.
#       -t prog         Run "prog" on "cvs rtag" of module.
#       -u prog         Run "prog" on "cvs update" of module.
#       -d dir          Place module in directory "dir" instead of module name.
#       -l              Top-level directory only -- do not recurse.

foreach my $module (sort keys %contents) {
	my $dir;
	my @files;
	my $expanded;

	$contents{$module} =~ s/\t/ /g;        # Tabs to space
	$contents{$module} =~ s/\s+/ /g;       # Multiple spaces to one space
	$contents{$module} =~ s/\&//g;         # Handle ampersand
	$contents{$module} =~ s/^\s+-a\s+/ /g; # Filter out alias keyword

	# NOTE: Since module contents are recursively expanded
	# NOTE: Entries that have -l -d(i.e. no recurse), are
	# NOTE: not expanded
	if ($contents{$module} =~ /^\s+-l\s+-d\s+([^ ]*)\s+/) {
	   $contents{$module} =~ s/^\s+-l\s+-d\s+([^ ]*)\s+/ /g;
	   $nonrecurse{$module} = $module;
	   Dbg("Non-recursive module '$module' not expanded further");
	   next;
	}

	# When -d is specified with "files", prefix the dir portion to them
	# Otherwise skip dest dir and place actual file
	# Process <module> -d <dest> <mod1> <mod2> <dir> <dir> <mod3>
	# Process <module> -d <dest> <mod1> <mod2> <dir> <file1> <file2>
	if ($contents{$module} =~ /^\s+-d\s+([^ ]*)\s+/) {
		my $found_files=0;
		$contents{$module} =~ s/^\s+-d\s+([^ ]*)\s+/ /g;
		Dbg("Redirect Module: $contents{$module}");
		foreach my $submodule (grep { !/^\s*$/ } split(/\s+/,$contents{$module})) {
			next if ($submodule =~ /\!/);
			if (exists($contents{$submodule})) {
				$expanded .= " $submodule ";
				$contents{$module} =~ s/ $submodule / /g;
				next;
			}
		}
		($dir,@files) = grep { !/^\s*$/ } split(/\s+/,$contents{$module});
		Dbg("Expanded=$expanded; Dir=$dir; Files=@files");
		if (scalar(@files) == 0) {
			$expanded .= " $dir ";
		} else {
			# Convert relative source files to absolute paths
			for my $f (@files) {
				if (exists($contents{$f})) {
					$expanded .= " $f ";
				} elsif ($f =~ /\/|\\/) {
					$expanded .= " $f ";
				} elsif ($f =~ /\.(c|h|cpp|s|mk)/) {
					$expanded .= " $dir/$f ";
					$found_files=1;
				}
			}
			$expanded .= $dir unless $found_files;
		}
		Dbg("Contents Final : $expanded");
		$contents{$module} = $expanded;
	}
	# Remove extra white-space
	$contents{$module} =~ s/\s+/ /g;
	# Place the cleaned module in cleaned hash
	$cleaned{$module} = $contents{$module};
}

# Go through cleaned list and expand submodules that are cvs modules themselves
foreach my $level (@levels) {
	foreach my $module (sort keys %cleaned) {
		my %seen;
		my $expanded;
		my @submodules;
		my $submodule;
		
		@submodules = grep { /^\S+$/ } split(/\s+/,$cleaned{$module});

		while (my $submodule=shift @submodules) {
			next if ($submodule =~ /\!/);
			if (exists($cleaned{$submodule})) {
				$expanded .= $cleaned{$submodule};
				# Filter out expanded module key
				$expanded =~ s/\s+$submodule\s+/ /g;
			} else {
				# List each submodule only once
				if ($expanded !~ / $submodule /) {
					$expanded .= " $submodule ";
				}
			}
		}
	
		# Remove any duplicates and sort the cleaned submodules
		$expanded = join(' ',sort grep {!$seen{$_}++} split(/\s+/,$expanded));
		$expanded =~ s/\s+/ /g;
		$cleaned{$module} = $expanded;
	}
}

open(CVSMODULESEXPANDED,">$out") if (defined($out));
foreach my $module (sort keys %cleaned) {
	if (!$keep_duplicates) {
		$cleaned{$module} = remove_duplicates($module);
	}
	if (defined($out)) {
		print CVSMODULESEXPANDED "$module: $cleaned{$module}\n";
		print CVSMODULESEXPANDED "====================================\n";
	}
}
close(CVSMODULESEXPANDED);

# Show expanded module info user requested cvs modules
foreach my $cvs_module (@cvs_modules) {
	my $submodule;
	my @submodules;

	Dbg("CLEANED MODULES: $cleaned{$cvs_module}");

	@submodules = grep { /^\S+$/ } split(/\s+/,$cleaned{$cvs_module});
	foreach my $submodule (@submodules) {
		&Dbg("Reporting Submodule: $submodule");
		if (-f "$CVSROOT/${submodule},v" ) {
			print "File: $submodule\n";
		} else {
			if (exists $nonrecurse{$submodule}) {
				#print "$bsubcmd ${submodule}\n";
				$contents{$submodule} =~ s/^\s+//g;
				$contents{$submodule} =~ s/\s+$//g;
				if ( -d "$CVSROOT/$contents{$submodule}" ) {
					print "Non-recursive: $contents{$submodule}\n";
				} else {
					print "[MISSING] Non-recursive: x$CVSROOT/$contents{$submodule}y\n";
				}
			} else {
				#print "$bsubcmd -l $contents{$submodule}\n";
				if ( -d "$CVSROOT/${submodule}" ) {
					print "Recursive: ${submodule}\n";
				} else {
					print "[MISSING] Recursive: $CVSROOT/${submodule}\n";
				}
			}
		} # Dir
	} # submodules
}

sub usage {
	my $thisscript=qx(basename $0); chomp($thisscript);
	print <<_EOF_

Usage: $thisscript \\
	[-module <cvsmodule1> -module <cvsmodule2> ...] \\
	[-out <parsed_cvs_module_outfile>] \\
	[-debug] \\
	[-help]

	This script parses cvs administrative 'modules' file 
	and iteratively expands all sub-module references and 
	removes duplicates. It doesn't update 'modules' file.
	Expanded result can be stored in a outfile (-out swich).
	If a particular cvsmodule is queried via -module switch
	its expanded output is shown on stdout.

	Examples:
		1. $thisscript -module wl-build	
		2. $thisscript -module wl-build -module hndrte
		2. $thisscript -out /home/$USER/tmp/expanded_cvs_modules

_EOF_

}
