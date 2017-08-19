#!/usr/bin/perl -i
#
# Filters out filenames from stdin based on rulesets. The ruleset
# should be a newline-separated list of Perl regexps. If a regexp is
# prefaced with '~', then the file is included if the regexp matches,
# otherwise it is excluded. If multiple regexps match, the last rule
# to match takes precedence.
#
# When run with -v command line switch, inverse of above happens
# entries with '~' are excluded and others are included.
#
# If a wildcard (directory rule) exists and if it followed by '~'
# rules that are inside a mogrified-out conditional, then '~' lines have
# no effect (as mogrifier strips out rules you want excluded). So
# avoid wildcard entries as much as possible in filelists
#
# Copyright (C) 2002 Broadcom Corporation
#
# $Id$
#
# SVN: $HeadURL$
#

# invert the sense of matching
$invert = 0;
$exact = 0;
@rules  = ();
@nrules = ();

# process command line
foreach $file (@ARGV) {
    # process options
    if ($file eq "-v") {
	$invert = 1;
	next;
    }
    if ($file eq "-e") {
	$exact = 1;
	next;
    }
    # append ruleset
    !open(INPUT, "<$file") && die "Error: Can't open input filelist '$file'";
    push(@rules, <INPUT>);
    close(INPUT);
}

# Cleanup rulesets before stdin processed
foreach $rule ( @rules ) {
        chomp($rule);
	# Remove trailing whitespace
	$rule =~ s/\s+$//g;
	# skip comments and whitespace
	next if     ( $rule =~ /^#/ );
	next unless ( $rule =~ /\S/ );
	# avoid accidental substring matches by escaping "."
	if ( $rule !~ /\.\*/ ) { $rule =~ s/\./\\\./ };
	# if exact filename match insert ^ and $ anchors in rule/pattern
        if ($exact)  {
		$rule =~ s/(~?)(.*)$/$1^$2\$/
	}
        push(@nrules,$rule);
}

# apply rulesets to stdin
while ($name=<STDIN>) {
    # chomp;
    $name   =~ s/[\r\n]+$//;
    $accept = 1;
    next if ($name =~ m%/CVS([/]*)%g);
    foreach $rule (@nrules) {
	# wildcard rules are discouraged in *-filelists as they create
	# weird problems, like $rule matching as a substring in middle of $name
	# reject if the rule matches...
	if ( $name =~ /^\s*$rule/ ) {
	    $accept = 0;
	}
	# ...unless the rule starts with '~'
	elsif ( ( $rule =~ /^~(.*)/ ) && ( $name =~ /$1/ ) ) {
	    $accept = 1;
	}
    }
    if ($accept ^ $invert) {
	print "$name\n";
    }
}
