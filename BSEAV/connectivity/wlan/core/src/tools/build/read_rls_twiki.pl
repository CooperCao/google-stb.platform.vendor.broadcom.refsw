#
# Script to convert release twiki table to "build_<platform> : <tag>" format
# This is in-turn used as an input file to build report generation
#
# $Id$
#
# Author: Prakash Dhavali (2011/02/20)
# Contact: hnd-software-scm-list
#

$TIMESTAMP            = qx(date '+%Y%m%d_%H%M%S'); chomp($TIMESTAMP);
$RELEASE_TWIKI_HTML   = "http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/HndSwReleases";
$RELEASE_TWIKI_FILE   = "HndSwReleases";
$RELEASE_BUILDS_HTML  = "Temp_SwRelease_${TIMESTAMP}.html";
$RELEASE_BUILDS_TEXT  = "Released_Builds_${TIMESTAMP}.txt";
$AUTH                 = qx(cat /home/hwnbuild/.restricted/passwd); chomp($AUTH);

# Globals
my @released; # List of releases found in twiki
my $topic_revision;

sub getHtmlTwiki {
	system("wget --user hwnbuild --password $AUTH $RELEASE_TWIKI_HTML");
	system("egrep 'th.*rowspan|build_window|build_linux|build_macos|build_netbsd' HndSwReleases > $RELEASE_BUILDS_HTML");

	# Extract topic revision for this report:
	# Topic revision: r166 - 24 Feb 2011 - 17:00:17 - Main.xiwang
	$topic_revision=qx(grep "Topic revision: " ${RELEASE_TWIKI_FILE}* | tail -1);
	#print "rev1 = $topic_revision\n";

	($topic_revision) = ($topic_revision =~ />(Topic revision:[^<]*)/msg);
	#print "rev2 = $topic_revision\n";
} # getHtmlTwiki

sub convHtml2Text {

	## Process text files
	open(INPUT, "$RELEASE_BUILDS_HTML") || die "Can't open Input file $RELEASE_BUILDS_HTML";

	# First convert an existing html file back to a text format
	while (<INPUT>) {
	      my $line = $_;
	
	      if ($flag==1){
	         # remove all until the first >
	         next if ($line !~ s/[^>]*>//);
	         # if we didn't do next, it succeeded
	         $flag=0;
	      }

	      while ($line =~ s/<[^>]*>//g) {};

	      if ($line =~ s/<.*$//){
	         $flag=1;
	      }

	      # push converted txt line into txtrpt array
	      #print "$line";
	      next if ($line =~ /http:.*projects/);
	      push(@released, $line);
	}

	close(INPUT);
	system("rm -fv $RELEASE_BUILDS_HTML");
	system("rm -fv $RELEASE_TWIKI_FILE");

} # convHtml2Text

sub printOutput {
	open (OUTPUT, ">$RELEASE_BUILDS_TEXT") || die "Can't open output file $RELEASE_BUILDS_TEXT";

	print OUTPUT "#\n# Report Generated at: $TIMESTAMP\n#\n";
	print OUTPUT "#\n# $topic_revision\n#\n";
	foreach $build (@released) {
		$build =~ s/^\s+//g;
		if ($build =~ /^build_(linux|window|macos|netbsd)\//) {
			@tmp = split(/\s+/,$build);
			chomp(@tmp);
			foreach my $rls (@tmp) {
				$rls =~ s%/% : %g;
				print OUTPUT "$rls\n";
			}
		} else {
			print OUTPUT "# $build";
		}
	}
	print "\n\nOutput is stored is in: $RELEASE_BUILDS_TEXT\n\n";
	close (OUTPUT);
} # printOutput

sub Main ()
{
	getHtmlTwiki();	
	convHtml2Text();
	printOutput();
}

# Run the main program now
&Main();
