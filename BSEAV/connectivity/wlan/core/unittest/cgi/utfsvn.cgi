#!/usr/bin/perl -w

# UTF cgi script for generating SVN reports

#
# $Id$
# $Copyright Broadcom Corporation$
#

use strict;
use CGI qw(:standard -xhtml -nosticky);
use CGI::Carp qw(fatalsToBrowser warningsToBrowser);
use Date::Parse;
use Time::Local;

$CGI::POST_MAX=1024 * 100;  # max 100K posts
$CGI::DISABLE_UPLOADS = 1;  # no uploads

# Prevent cgi forms warnings when run from commandline.
$ENV{QUERY_STRING}='' unless exists $ENV{QUERY_STRING};
$ENV{SHELL}='/bin/bash';

my $caption = "UTF SVN";
my $svn = "http://svn.sj.broadcom.com/svn/wlsigsvn/";
my $svnview = "http://svn.sj.broadcom.com/viewvc/wlsigsvn?view=revision";
my $gnats = "http://gnatsweb.broadcom.com/cgi-bin/gnatsweb.pl?cmd=view%20audit-trail;database=HND_WLAN";
my $rb = "http://wlan-rb.sj.broadcom.com/r";
my $jira = "http://jira.broadcom.com/browse";

if (!defined(param('path'))) {
    param('path' => "");
}
if (!defined(param('user'))) {
    param('user' => "");
}
if (!defined(param('days')) || param('days') eq "") {
    param('days' => "1");
}
if (!defined(param('date')) || param('date') eq "") {
    param('date' => yymmddd(time - 24*60*60));
}

print header();

my $url;
my $dir;
$url = "$svn/trunk/unittest/";
$dir .= param('path');


print start_html(-title=>$caption,
		 -head=>[Link({-rel => 'stylesheet',
			       -type => 'text/css',
			       -href => '//code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css'}),
			 Link({-rel => 'stylesheet',
			       -type => 'text/css',
			       -href => '/resources/demos/style.css'})],
		 -script => [
		      {-src => '//code.jquery.com/jquery-1.10.2.js'},
		      {-src => '//code.jquery.com/ui/1.11.4/jquery-ui.js'},
		      {-type => 'text/javascript',
		       -code => '
      $(function() {
        $( "#datepicker" ).datepicker({
          dateFormat: "yy.m.d",
          maxDate: "+0D"
        });
      });'
		 }]);

print h1($caption)."\n";
warningsToBrowser(1);


print start_form;
print table({border=>0, summary=>'Form'},
	    [Tr([td('Date: '.textfield(-name=>'date', id=>'datepicker',
				       -size=>10)).
		 td('+'.textfield(-name=>'days', -size=>1)."day(s)").
		 td('User: '.textfield(-name=>'user', -size=>10)).
		 td(submit)]),
	     Tr([td('Path: /trunk/unittest/'.
		    textfield(-name=>'path', -size=>50).
		    checkbox(-name=>'Hide Index'))]),
	     Tr([td([a({-href=>self_url}, 'Bookmark')])])]);

print end_form."\n";

sub yymmdd {
    my($t) = @_;
    my($y, $m, $d) = (localtime($t))[5,4,3];
    return sprintf("%02d%02d%02d", $y+1900, $m+1, $d);
}

sub yymmddd {
    # dotted build format
    my($t) = @_;
    my($y, $m, $d) = (localtime($t))[5,4,3];
    return ($y+1900).'.'.($m+1).'.'.$d; 
}

my $rev="";
my $start=str2time(param('date'));
my $end=$start + (24*param('days')+1)*60*60;
$rev='{'.(yymmdd($start)).'}:{'.(yymmdd($end)).'}';
my $cmd = "/usr/bin/svn -v ";
if (!exists $ENV{LOGNAME}) {
    # Running as a web page
    $cmd .= "--username hwnsig --password Hrun*100 --no-auth-cache ";
} else {
    # Running under a user account for debugging
    $cmd .= "--config-option servers:global:store-plaintext-passwords=yes "
}
$cmd .= "log -r '$rev' $url $dir";

warn "$cmd\n";

open(LOG, "$cmd 2>&1|") || die "svn failed";

my $table = "";
my $cset = "";
my $user = param('user');
my $skip = 0;

while (<LOG>) {
#    warn "$_\n";
    next if /^\s*$/;
    next if /^Changed paths:\n/;

    s!/trunk/unittest/!!;

    # Filter cset
    if ($cset eq "" && $user ne "" && !/^r\d+\s+\|\s+$user\s+\|/) {
	$skip = 1;
    }

    # Protect special chars
    s/&/&amp;/g;
    s/</&lt;/g;
    s/>/&gt;/g;
    
    

    # Add Links for revisions
    s!\br(\d+)\b \|!<a href=\"$svnview;revision=${1}\">r$1</a> \|!g;

    # Add links for PRs
    s!(PR[ #:]*(\d+))!<a href=\"$gnats;pr=$2\">$1</a>!g;

    # Add links for Jira
    s!((?:JIRA|PR)[ #:]*(\w+-\d+))!<a href=\"$jira/$2\">$1</a>!gi;

    # Add links for ReviewBoard
    s!(RB[ #:]*(\d+))!<a href=\"$rb/$2/\">$1</a>!g;

    if (/-{72}\n/) {
	chomp $cset;
	if ($cset ne "" && !$skip &&
	    !(param('Hide Index') && 
	      $cset =~ m!\| 1 line\n   M pkgIndex.tcl\nversions$!)) {

	    # Build up the table in reverse order
	    $table="\n".Tr(td(pre($cset))).$table;

	}
	$skip = 0;
	$cset = "";
    } else {
	$cset .= $_;
    }
}
if ($cset ne "") {
    # If there is remaining output outside of a cset, print it.  It
    # may be an error message.
    print p($cset)."\n";
}
close LOG;

if ($table ne "") {
    $table .= "\n";
    print table({summary=>'table', border=>0, cellpadding=>0, cellspacing=>0},
		$table)."\n";
}

#print Dump;

print p(a({-href=>"http://tidy.sourceforge.net"},
	  img({src=>"checked_by_tidy.gif",
	       alt=>"Checked by Tidy!", border=>"0",
	       height=>"32", width=>"32", align=>'right'})));

print end_html;

exit;
