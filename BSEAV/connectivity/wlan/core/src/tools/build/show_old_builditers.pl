#!/usr/bin/perl
#
# Script called by all build_<platform>.sh scripts. This script scans the
# brand directory for old build iterations that need to be kept or deleted
# as per different age (number of days) criterion. This is used by build
# scripts when "-x <number-of-days-to-keep>" option is selected.
#
# It just shows what to be deleted, but does not do any deletions.
#
# Unlike previous methods of pruning which was purely based on age of the
# build, this new method ensures that at-least one old successful build is
# preserved and also at-least $keep_count days worth of builds are preserved
# (irrespective of when they were built)
#
# Default aging numbers are determined by BUILDS_TO_KEEP_TOB and
# BUILDS_TO_KEEP_TOT variables in build scripts and are as per DiskSpacePolicy
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#
# Usage: show_old_builditers -keep_count <no_of_old_builds_to_keep> -brand_dir <brand_dir>
# Example:
# show_old_builditers \
#    -keep_count 2 \
#    -brand_dir \
#     "/projects/hnd/swbuild/build_linux/HORNET_TWIG_4_212/linux-internal-dongle"
#
# Sample for debuging:
# perl show_old_builditers.pl \
#      -brand_dir /projects/hnd/swbuild/build_linux/NIGHTLY/linux-internal-wl \
#      -keep_count 7 \
#      -verbose'
#
# version=2011.7.29.0; age=6.3 days
# version=2011.7.30.0; age=5.3 days
# .....
# PRESERVED:
# KEEP  :/projects/hnd/swbuild/build_linux/NIGHTLY/linux-external-wl/2011.7.30.0
# .....
# DELETE:/projects/hnd/swbuild/build_linux/NIGHTLY/linux-external-wl/2011.7.29.0

my $verbose, $keep_count, $brand_dir;
my $i, $succeeded_flag, $preserve_iter, $arg, $age, %ages;
my $all_iters, @all_iters, $iter, $iters_found;
my @keep_iters, $keep;
my @delete_iters, $delete;
my @duplicate_iters;
my $ver, $day, %builds;

while ( $ARGV[0] ) {
	$arg = shift @ARGV;
   	if ($arg =~ /^-(x|keep_count)/ ) {
      		$keep_count = shift @ARGV;
   	} elsif ($arg =~ /^-(b|brand_dir)/ ) {
      		$brand_dir = shift @ARGV;
   	} elsif ($arg =~ /^-(v|verbose)/ ) {
      		$verbose = 1;
	} else {
		print STDERR "Usage: $0 -keep_count <n> -brand_dir <dir> [-verbose]\n";
		exit 1;
	}
} # while

# NOTE: Output from this script is processed by build scripts. So do not
# NOTE: place verbose messages unless in verbose mode for debugging
#if (!$keep_count || !$brand_dir) {
#	print STDERR "Usage: $0 -keep_count <num-of-days> -brand_dir <dir> [-verbose]\n";
#	exit 1;
#}

print "\nINFO: Running '$0 -brand_dir $brand_dir -keep_count $keep_count -verbose'\n\n" if ( $verbose );

exit(1) if ( ( $keep_count < 0 ) ||  ( $keep_count !~ /\d+/ ) );
exit(1) if ( ! -d "$brand_dir" );

# Search number of build iterations available
$all_iters = qx(/usr/bin/find "$brand_dir" -maxdepth 1 -mindepth 1 -type d -print);
chomp($all_iters);
@all_iters = split(/\s+/,$all_iters);

# Find age of each build iteration
$iters_found = 0;
%ages        = ();
foreach $iter ( @all_iters ) {
	my $skip=0;

	# Address variants of build destination dirs supported by platform build scripts
	if ( $iter =~ /-private-build/ ) {
		if ( $iter !~ /\/\d{4}\.\d{1,2}\.\d{1,2}\.\d{1,3}-private-build$/ ) {
			$skip=1;
			print "\nINFO: Skipping $iter\n" if ($verbose);
		}
	} else {
		if ( $iter !~ /\/\d{4}\.\d{1,2}\.\d{1,2}\.\d{1,3}$/ ) {
			$skip=1;
			print "\nINFO: Skipping $iter\n" if ($verbose);
		}
	}

	next if ($skip);
	$age = -M $iter;
	$age = int($age * 10)/10;
	$ages{$iter} = $age;
	$iters_found++;
}

# delete_count is number of build iterations to delete
$delete_count   = ${iters_found}-${keep_count};

exit (0) if ( $delete_count < 0 );

@keep_iters = @delete_iters = @duplicate_iters = ();
$succeeded_flag = 0;
foreach $iter ( sort { $ages{$b} <=> $ages{$a} } keys %ages ) {
	( $ver ) = ( $iter =~ m%.*/(.*)$%g );
	( $day ) = ( $ver  =~ m%(\d{4}\.\d{1,2}\.\d{1,2})\.\d{1,3}$%g );
	print "version=$ver; age=$ages{$iter} days\n" if ( $verbose );
	if ( $ages{$iter} > $keep_count ) {
		push(@delete_iters,$iter);
	} else {
		push(@keep_iters,$iter);
		$succeeded_flag = 1 if ( -f "$iter/,succeeded" );
		push(@{$builds{$day}},$iter);
	}
}

# Identify duplicate build iterations from keep list 2 days and prior to it
foreach $day ( keys %builds ) {
	while ( @{$builds{$day}} > 1 ) {
		$iter = shift(@{$builds{$day}});
		if ( $ages{$iter} > 2 ) {
			push(@duplicate_iters,$iter)
		}
	}
}

# Append duplicate build iterations to deletion list
push(@delete_iters,@duplicate_iters);

# Filter out duplicate build iterations from keep_iters list
foreach $iter ( @keep_iters ) {
	push(@keep_iters_new, $iter)
		unless ( grep { /${iter}$/ } @duplicate_iters );
}

# keep_iters_new is now without any duplicate_iters
@keep_iters = @keep_iters_new;

# If none of kept builds have any succeeded builds, then keep one build
# successful iter from @deleted_iters list (if any exist)
$preserve_iter="";

if ( ! $succeeded_flag ) {
	foreach $delete ( reverse @delete_iters ) {
		if ( -f "$delete/,succeeded" ) {
			$preserve_iter = $delete;
			last;
		}
	}
}

print "PRESERVED: $preserved_iter\n" if ( $verbose );

# Filter out preserved_iter from delete list. Preserve_iter must be kept
if ( -d "${preserve_iter}" ) {
	@delete_iters = grep { $_ !~ /^${preserve_iter}$/ } @delete_iters;
	push(@keep_iters,$preserve_iter);
	foreach $keep ( @keep_iters ) {
		if ( ! -f "$keep/,succeeded" ) {
			print "WARN: Keeping Failed $keep\n" if ( $verbose );
		}
	}
}

# Finally print build iterations which are older than keep_count
map { $verbose ? print "KEEP  : $_\n" : print "";     } @keep_iters;
map { $verbose ? print "DELETE: $_\n" : print "$_\n"; } @delete_iters;

exit(0);
