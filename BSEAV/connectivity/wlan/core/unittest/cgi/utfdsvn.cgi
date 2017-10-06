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

my $caption = "UTFD SVN Dashboard";
my $svn = "http://svn.sj.broadcom.com/svn/wlansvn/proj";
my $svnview = "http://svn.sj.broadcom.com/viewvc/wlansvn?view=revision";
my $gnats = "http://gnatsweb.broadcom.com/cgi-bin/gnatsweb.pl?cmd=view%20audit-trail;database=HND_WLAN";
my $rb = "http://wlan-rb.sj.broadcom.com/r";
my $jira = "http://jira.broadcom.com/browse";
my $q = CGI->new();
my $rigname = $q->param('rig');
my $scriptname = $q->param('script');
my $dutnames = $q->param('dut');
my $branchname = $q->param('branch');
my $utfdurl = "http";
#if ($ENV{HTTPS} = "on") {
#    $utfdurl .= "s";
#}
$utfdurl .= "://";
#$utfdurl .= $ENV{SERVER_NAME};
$utfdurl .= "rjm-sfast-utf";

#if ($ENV{SERVER_PORT} != "80") {
#    $utfdurl .= $ENV{SERVER_NAME}.":".$ENV{SERVER_PORT};
#} else {
#    $utfdurl .= $ENV{SERVER_NAME};
#}
my $utfdsvnquery = "http://".$ENV{HOSTNAME}.":8080/cgi/svnlauncher.cgi";
my $utfdschedule = $utfdurl."/utfdschedulescript.cgi";

if (param('ARM Kernel')) {
    $svn = "http://svn.sj.broadcom.com/svn/wlansvn/components/opensource/linux/linux-2.6.36";
    param('branch', "");
    param('path', "");
} elsif (param('Infrastructure')) {
    $svn = "http://svn.sj.broadcom.com/svn/wlansvn/groups/software/infrastructure";
    param('branch', "");
    param('path', "");    
} elsif (!defined(param('path')) || param('path') eq "") {
    param('path' => "src");
}
if (!defined(param('days')) || param('days') eq "") {
    param('days' => "1");
}
if (!defined(param('date')) || param('date') eq "") {
    param('date' => yymmddd(time - 24*60*60));
}

# print header();

my $url;
if ($svn =~ m!/proj$!) {
    if (!defined(param('branch')) || param('branch') eq "") {
	$url = "$svn/trunk/";
    } else {
	$url = "$svn/branches/".param('branch')."/";
	$caption.=" ".param('branch');
    }
} else {
    $url = "$svn/"
}
$url .= param('path');

while ($url =~ m!/\.\./!) {
    $url =~ s!/[^/]*/\.\./!/!;
}


print start_html(-title=>$caption);

print h1($caption)."\n";
warningsToBrowser(1);

print "This HTTP-server's name : $ENV{SERVER_NAME}<br>\n"; 

print start_form(-action =>$utfdsvnquery);
print table({border=>0, summary=>'Form'},
	    [Tr([td('Rig: '.textfield(-name=>'testrig', -size=>40, -default=>$rigname)).
		 td('DUT(s): '.textfield(-name=>'DUT', -size=>40, -default=>$dutnames))]),
	     Tr([td('Script: '.textfield(-name=>'script', -size=>240, -default=>$scriptname))]),
	     Tr([td('SVN rev: '.textfield(-name=>'r1', -size=>10)).
	         td(submit('scriptline','Submit to UTFD'))]), 
	     Tr([td('Branch: '.textfield(-name=>'branch', -size=>30, -default=>$branchname)).
		 td('Date: '.textfield(-name=>'date', -size=>10)).
		 td('+'.textfield(-name=>'days', -size=>1)."day(s)").
		 td(submit)]),
	     Tr([td('Path: '.textfield(-name=>'path', -size=>50)).
		 td([checkbox(-name=>'Hide Other Branches')]).
		 td([checkbox(-name=>'ARM Kernel')]).
		 td([checkbox(-name=>'Infrastructure')])]),
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
my $which_one;
if ($which_one = param('scriptline')) {
    open(LOG, "$utfdschedule 2>&1|") || die "utfd launch failed";
}
    
my $rev="";
my $start=str2time(param('date'));
my $end=$start + (24*param('days')+1)*60*60;
$rev='{'.(yymmdd($start)).'}:{'.(yymmdd($end)).'}';
my $cmd = "/usr/bin/svn -v ";
$cmd .= "--username hwnsig --password Hrun*100 --no-auth-cache ";
$cmd .= "log -r '$rev' $url";

warn "$cmd\n";

open(LOG, "$cmd 2>&1|") || die "svn failed";

my $table = "";
my $cset = "";
while (<LOG>) {
     warn "$_\n";
    next if /^\s*$/;

    if (param('Hide Other Branches')) {
	if (param('branch') eq '') {
	    next if m![AMR] /proj/branches/!;
	} else {
	    next if m![AMR] /proj/trunk/!;
	    next if m![AMR] /proj/branches/([^/]+)/! && $1 ne param('branch');
	}
    }

    # Protect special chars
    s/&/&amp;/g;
    s/</&lt;/g;
    s/>/&gt;/g;
    
    # Add Links for revisions
    s!\br(\d{6,})\b!<a href=\"$svnview;revision=${1}\">r$1</a>!g;

    # Add links for PRs    s!(PR[ #:]*(\d+))!<a href=\"$gnats;pr=$2\">$1</a>!g;

    # Add links for Jira
    s!((?:JIRA|PR)[ #:]*(\w+-\d+))!<a href=\"$jira/$2\">$1</a>!gi;

    # Add links for ReviewBoard
    s!(RB[ #:]*(\d+))!<a href=\"$rb/$2/\">$1</a>!g;


    if (/----------------------*\n/) {
	# Build up the table in reverse order
	$table=Tr(td(pre($cset))).$table if $cset ne "";
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
    print table({summary=>'table', border=>0},$table);
}

#print Dump;

print p(a({-href=>"http://tidy.sourceforge.net"},
	  img({src=>"checked_by_tidy.gif",
	       alt=>"Checked by Tidy!", border=>"0",
	       height=>"32", width=>"32", align=>'right'}))),"\n";

print end_html;

exit;
