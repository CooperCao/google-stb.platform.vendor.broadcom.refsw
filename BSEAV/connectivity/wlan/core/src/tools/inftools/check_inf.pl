##
## Script to do inf check and analyze results and exit if there are any errors
##
## $Id: check_inf.pl,v 1.3 2009-07-29 19:35:19 prakashd Exp $

use File::Basename;

my($verbose, $ignore_error, @ignored_errors, $show_warning);
my $exitcode = 0;

sub Dbg {
	my($msg) = shift;

	return unless $dbg;
	print "[DBG]: $msg";
}

while ($ARGV[0]) {
	my $arg = shift @ARGV;
	if ($arg =~ /^-inf/ ) {
		$inf_path = shift @ARGV;
		Dbg("inf_path = $inf_path");
	} elsif ($arg =~ /^-ignore/ ) {
		$ignore_error = shift @ARGV;
		push(@ignored_errors,$ignore_error);
		Dbg("Ignore Error = $ignore_error");
	} elsif ($arg =~ /^-chkinf/ ) {
		$chkinf = shift @ARGV;
		Dbg("chkinf = $chkinf");
	} elsif ($arg =~ /^-show_warning/ ) {
		$show_warning = 1;
	} elsif ($arg =~ /^-verbose/ ) {
		print "INFO: Setting verbose flag\n";
		$verbose = 1;
	} elsif ($arg =~ /^-dbg/ ) {
		print "INFO: Setting debug flag\n";
		$dbg = 1;
	}
} # while

if (! -f "$inf_path" || ! -f "$chkinf") {
	print "\nERROR: Missing some cmd line arguments\n\n";
	print "Usage: \n\n";
	print " $0 -inf_path '<inf_path>' -chkinf <chkinf_utility_to_use\n";
	print " \t[-ignore <ignore_error1> -ignore <ignore_error2> .. [-verbose]\n";
	print "\n";
	exit(1);
}

my $inf_file          = basename($inf_path);
(my $inf_file_dos     = $inf_file) =~ s%/%\\\\%g;
my $chkinf_log        = "${inf_file_dos}.log";
my $ignored_errors    = join('|',@ignored_errors);

my $inf_dir = dirname($inf_path);
chdir($inf_dir) || die "$0: Error: $inf_dir: $!\n";
system("rm -rfv htm utf8 $chkinf_log");

my $chkinf_dir = dirname($chkinf);
my $cmd = "$chkinf_dir/perl -I$chkinf_dir $chkinf /L $chkinf_log $inf_file_dos";

print $cmd, "\n";
system($cmd);

open(LOG, $chkinf_log) || die "$0: Error: $chkinf_log: $!\n";
while(<LOG>) {
	next if /^\s*$/g;
	Dbg("Processing: \"$_\"");
	if (/${inf_file}: /) {
		print $_;
		next;
	}
	if (/(Total Lines:\s+\d+)\s+/) {
		print "$1\n";
		next;
	}
	if (/(Total Errors:\s+\d+)\s+/) {
		print "$1\n";
		next;
	}
	if (/(Total Warnings:\s+\d+)\s+/) {
		print "$1\n";
		next;
	}
	if ($show_warning && /: WARNING: /) {
		print "$_";
		next;
	}
	# TODO: Similar looking errors can be combined
	if (/: ERROR: /) {
		if ($ignored_errors && /$ignored_errors/) {
			Dbg("Ignored Error: $_");
			# print "Ignored Error: $_";
			next;
		} else {
			print $_;
			$exitcode = 1;
			next;
		}
	}
}
close(LOG);

#system("unix2dos $chkinf_log");

print "NOTE: INF Check logfile: $chkinf_log\n";

# If there are real errors, then build should exit with an error
exit($exitcode);
