#!/usr/bin/perl

#use strict;
use warnings;

sub usage
{
    print STDERR << "EOF";
preprocessor_includes [-h] [-I include_paths] [-r] [-d] [-i] [-f] file_list
    -h help
    -I include path(s)
    -r output any recursive includes encountered
    -d output defines/macros used in if/elif/ifdef/ifndef's
    -i indent each output line based on nesting
    -f output original file

By default this program will output the list of directly and indirectly included
files for the input file list.  A line is output as each include file is read.  No
attempt is made to consolidate the list so use "sort -u" on the output to reduce.
Note the program doesn't evaluate preprocessor conditional logic (e.g. #if) so the
includes are those might be used.  The intended purpose of this command is understand
dependencies baseed on source code so this behavior is not problematic.

-r can be used to list recursive includes encountered which are otherwise ignored.
-d will list the macros used in if/elif/ifdef/ifndef statements.  An attempt is made to
ignore the common usage of the filename or something similar in an initial ifndef in a file.
EOF
exit
}

$opt_h=0;
$opt_r=0;
$opt_d=0;
$opt_i=0;
$opt_f=0;

use Getopt::Long;

$Getopt::Long::ignorecase = 0;
usage() if ( !GetOptions("I=s@", "h", "r", "d", "f", "i") || $opt_h );

# Add trailing / if search path options is missing one
foreach (@opt_I)
{
    $_ .= "/" if !m{/$};
}

$indent_leader = "";
if ($opt_i)
{
    $indent_leader = "  ";
}

#print "Include path options: @opt_I\n";

@file_stack = ();

foreach $file (@ARGV)
{
    &process_file($file,'fh00',"C", 0);
}

sub process_file
{
    my($input_filename, $input_handle, $type, $depth) = @_;
    my($path,$filename,$line);

    if (grep /$input_filename/, @file_stack)
    {
        if ($opt_r)
        {
            #print "$type* Recursive includes: ", join(", ", @file_stack[0..$#file_stack], $input_filename), "\n";
            print "$type* Recursive includes: ", join(", ", @file_stack[1..$#file_stack], $input_filename), "\n";
        }
        return;
    }

    $input_handle++;
    unless (open $input_handle, $input_filename)
    {
        #print "$type* File open failure on $input_filename because ", lc $!, "\n";
        return;
    }

    #print $indent_leader x $depth, "$input_filename\n" if $type eq 'I';
    if ($depth > 0 || $opt_f)
    {
        print $indent_leader x $depth, "$input_filename\n";
    }
    push @file_stack, $input_filename;

    #
    # Parse out path and filename.
    #
    ($path, $filename) = $input_filename =~ m{(^.*/|^)(.*)};
    #print "input_filename $input_filename, path $path, filename $filename\n";

while ($line = <$input_handle>)
{
    chomp($line);
    #
    # While trailing \ plus optional whitespace at end of line
    # then remove \ plus whitespace and concatente next line
    #
    while ( $line =~ s/\\\s*$// )
    {
        $line .= <$input_handle>;
        chomp($line);
    }
    #
    # Remove comments, both // and /* ... */ (those don't nest)
    #
    $line =~ s=(/\*.*?\*/|//.*$)==g;
    #print $line, "\n";

    #
    # Process one if, ifdef, ifndef and elif lines.
    # Allow whitespace before and after initial #.
    #
    if ($opt_d && $line =~ s/^\s*?#\s*?(if|ifdef|ifndef|elif)\s// )
    {
        my ($directive, $filename_macro, $id);

        $directive = $1;                          # Remove and save # directive/verb
        #
        # Create filename based macro to ignore: e.g. _wlc_phy_c_
        #
        $filename_macro = "_" . $filename . "_";        # added surrounding _'s
        $filename_macro =~ s/\./_/g;                    # change any .'s to _'s

        #
        # find id's: a word starting with a non digit.  Amazingly perl remembers the last
        # position when specifying a pattern match with a "g" modifier.
        #
	while ($line =~ /(\b[A-Za-z_]\w*\b)/g)
	{
            $id = $1;
            next if $id eq "defined";            # defined is a special keyword for the preprocessor
            next if $id =~ /^$filename_macro$/i; # ignore filename_macro ... case insensitive
            if ($depth > 0 || $opt_f)
            {
                print $indent_leader x $depth, $input_filename, ": ", $1, "\n";
            }
        }
        #print "\n";
    }
    #
    # Process a #include file recursively invoking this subroutine.  "" and <> are allowed.
    # Allow whitespace before and after initial #.
    #
    if (    $line =~ /^\s*?#\s*?include "(.*)"/
         || $line =~ /^\s*?#\s*?include <(.*)>/ )
    {
        $include_filename = $1;                     # Save include file name

        #
        # if include file name is not absolute then use the include search path to find
        # an existing regular file
        #
        if ( $include_filename !~ m{^/} )
        {
            #
            # Search current input file directory, current working directory and then -I options paths
            #
            my (@include_search_paths);
            @include_search_paths = ($path, "", @opt_I);
            foreach $p (@include_search_paths)
            {
                #print "... trying ", $p . $include_filename, "\n";
                if ( -f ($p . $include_filename) )
                {
                    $include_filename = $p . $include_filename;
                    last;
                }
            }
        }
	#print "Begin processing #include ", $include_filename, "\n";
        &process_file($include_filename, $input_handle, "I", $depth+1);
        #print "End processing #include ", $include_filename, "\n";
    }
}

    close $input_handle;
    pop @file_stack;
    return;
}
