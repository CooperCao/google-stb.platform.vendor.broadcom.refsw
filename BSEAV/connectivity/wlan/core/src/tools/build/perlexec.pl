######################################################################
##
## Run non batch scripts as though they are batch scripts
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## $Id$
##
######################################################################

#
# ARGV[0] contains the current script name
# Change either ".bat.bat" -> ".bat" and ".bat.pl" -> ".pl"
#

$ARGV[0] =~ s/\.bat\.bat$/.bat/gi;
$ARGV[0] =~ s/\.bat\.pl$/.pl/gi;
$TMPPATH = $ENV{PATH};

if ( $TMPPATH =~ m%:/|/cyg%g ) {
   $TMPPATH = join (";", map { "\"$_\"" } split(':',$TMPPATH));
}

# Locate the first arg (which is the script name)
if (! -f $ARGV[0]) {

    local ($dirname);
    local (@path) = split(';', $TMPPATH);
    local ($found) = 0;

    foreach $dirname (@path) {
        local $file = "$dirname/$ARGV[0]";
        $file =~ s/"//g;
        #print "Searching  $file\n";
        if (-f "$file" ) {
            $ARGV[0] = $file;
	    $found = 1;
            #print "Found $file\n";
            last;
        }
    }

    if (!$found) {
	die "Can not find script: $ARGV[0]";
    }
}

#
# Run the located script now
#
$_       = $ARGV[0];
$ARGV[0] =~ s#\\#/#g;
#print "Running script = $ARGV[0]\n";
do (shift @ARGV);

## End of script
