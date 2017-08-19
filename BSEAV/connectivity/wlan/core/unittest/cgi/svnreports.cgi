#!/usr/bin/perl -w

# UTF cgi script for generating SVN reports

#
# $Id: aa74514dba6493221fa415f73e839b75d3bc17d3 $
# $Copyright Broadcom Corporation$
#

use strict;
use CGI qw(:standard -xhtml -nosticky);
use CGI::Carp qw(fatalsToBrowser warningsToBrowser);
use Date::Parse;
use Time::Local;
use File::Temp;
use File::Basename;
use XML::Simple;
use POSIX qw(strftime);

$CGI::POST_MAX=1024 * 100;  # max 100K posts
$CGI::DISABLE_UPLOADS = 1;  # no uploads

# Prevent cgi forms warnings when run from commandline.
$ENV{QUERY_STRING}='' unless exists $ENV{QUERY_STRING};
$ENV{SHELL}='/bin/bash';

$ENV{SUBVERSIONVER}="1.7.8";

# global constants
my $wlansvn = "http://svn.sj.broadcom.com/svn/wlansvn";
my $svncmd = "/usr/bin/svn";
my $hndcmd = "/usr/local/bin/python2.7 /projects/hnd/tools/GUB/bin/hnd";
my $svnview = "http://svn.sj.broadcom.com/viewvc/wlansvn?view=revision";
my $gnats = "http://gnatsweb.broadcom.com/cgi-bin/gnatsweb.pl?cmd=view%20audit-trail;database=HND_WLAN";
my $rb = "http://wlan-rb.sj.broadcom.com/r";
my $jira = "http://jira.broadcom.com/browse";

my $ucodesvn = "http://svn.sj.broadcom.com/svn/ucodesvn";
my $ucodeview = "http://svn.sj.broadcom.com/viewvc/ucodesvn?view=revision";

###########################
# Tag Dates
###########################


my $datejs;

{
    my $cachefile = "/var/tmp/tagdates";
#    $cachefile = "/projects/hnd_sig/cgi/tagdates";
    my %dates;

    sub readcache {
	my($start) = @_;
	my $cached = $start;
	my $startok = 0;
	if (open(C, $cachefile)) {
	    while(<C>) {
		if (/^START=(.*)$/) {
		    if ($1 eq $start) {
			$startok = 1;
		    } else {
			# Start date changed - invalidate cache
			return $start;
		    }
		} elsif (/^END=(.*)$/) {
		    $cached = $1;
		} elsif ($startok && /^(\S+)\s+(\S+)$/) {
		    foreach my $t (split(/,/,$2)) {
			$dates{$1}{$t} = 1;
		    }
		}
	    }
	    close(C);
	}
	warn "cached=$cached\n";
	return $cached;
    }

    sub writecache {
	my($start, $end) = @_;
	# Using PID for tmp file is good enough since this is a local
	# filesystem and we only have to protect against ourselves.
	open(C, ">$cachefile$$") || die "Can't write tmp tag cache";
	print C "START=$start\n";
	print C "END=$end\n";
	foreach my $k (sort keys %dates) {
	    print C "$k\t".join(",", (sort keys %{$dates{$k}}))."\n";
	}
	close C;
	rename("$cachefile$$", $cachefile) || die "Can't rename tag cache";
	if (-z $cachefile) {
	    die "Zero size cache.\n".`df -h $cachefile`."\n";
	}
    }

    sub fetchtags {
	my($start, $end) = @_;
	my $cmd = "/usr/bin/svn log -v -q --xml -r$start:$end http://svn.sj.broadcom.com/svn/wlansvn proj/tags";

	warn "$cmd\n";
	my $rawxml = `$cmd 2>&1`;
	my $date;
	# Parse XML
	my $struct = XMLin($rawxml, ForceArray=>['path','logentry']);
	foreach my $logentry (@{$struct->{logentry}}) {
	    use Data::Dumper;
	    #print Dumper($logentry)."\n";

	    $date = $logentry->{'date'};
	    my $time = str2time($date);

	    # Reformat date in local time, discarding fractional part.
	    my $fdate = strftime("%F", localtime($time));

	    foreach my $path (@{$logentry->{'paths'}->{'path'}}) {
		my $c = $path->{'content'};
		if ($c =~ m{proj/tags}) {

		    if (exists($path->{'copyfrom-path'})) {
			my $cfp = $path->{'copyfrom-path'};
			my $t = basename($c);
			my $b = basename($path->{'copyfrom-path'});
			$dates{"$fdate,$b"}{$t} = 1;
		    }
		}
	    }
	}
	return "{$date}"
    }

    sub generate_js {

	my $js = '
function ttsetup(date) {
    var b=document.forms["main"]["branch"].value;
    var d=$.datepicker.formatDate("yy-mm-dd", date);
    switch (d + "," + b) {
';

	foreach my $k (sort keys %dates) {
	    $js .= "    case \"$k\":\n";
	    $js .= "        return [1, \"ui-state-error\", \"".
		join("\\n", (sort keys %{$dates{$k}}))."\"];\n";

	}
	$js .= '
    default:
        return [1, "", ""]
    }
}
$(function() {
    $( "#datepicker" ).datepicker({
        dateFormat: "yy.m.d",
        maxDate: "+0D",
        beforeShowDay: ttsetup
    });
});';
	return $js
    }


    my $start = "{2015-01-01}";
    my $end = "HEAD";

    my $cached = readcache($start);
    my $newhead = fetchtags($cached, $end);
    writecache($start, $newhead);
    $datejs = generate_js();

}




###########################
#
###########################

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

my %csets = ();
my @ucodedir;

sub readdeps {
    my($rev, $deps) = @_;
    my $cmd = "$hndcmd -NN co -r '$rev' $wlansvn/$deps";
    warn "$cmd\n";
    my @dir;
    open(D, "$cmd 2>&1|") || warn "Can't parse DEPS: $!";
    while(<D>) {

	chop;
	if (s!.*ucodesvn/!! && !m/@\d+$/) {
	    push @ucodedir, $_;
	} elsif (s!.*wlansvn/!! && !m/@\d+$/) {
	    push @dir, $_;
	} else {
	    warn "$_\n";
	}
    }
    close D;
    push (@dir, $deps); # include deps file itself
    return @dir;
}

sub getlog {
    my($start, $end, @dir) = @_;
    my $interval = "$start:$end";
    my $retry = 1;
    my $table = "";
    my $match = join('|', @dir);
    my $user = param('user');
    my $branchmatch;
    if (param('branch') eq 'trunk') {
	$branchmatch = "proj/trunk";
    } else {
	$branchmatch = "proj/branches/".param('branch');
    }

    my $srvr;
    my $view;
    if ($match =~ "dot11") {
	$srvr = $ucodesvn;
	$view = $ucodeview;
    } else {
	$srvr = $wlansvn;
	$view = $svnview;
    }

    while ($retry) {
	$retry = 0;
	my $cmd = "$svncmd -v log -r '$interval' --xml --with-all-revprops $srvr @dir";

	warn "$cmd\n";
	my $rawxml = `$cmd 2>&1`;

	# If a user-specified path is missing, remove it from the
	# search and try again.
	if ($rawxml =~ m!svn: File not found: .*path '/(.*)'!) {
	    print p($rawxml);
	    my @olddir = @dir;
	    @dir = grep ($_ ne $1, @olddir);
	    if (@dir ne @olddir) {
		$retry = 1;
	    }
	    next;
	}

	# Parse XML
	my $struct = XMLin($rawxml, ForceArray=>['path','logentry']);

	my $cset = "";
      LOGENTRY:	foreach my $logentry (@{$struct->{logentry}}) {

	    #use Data::Dumper;
	    #print "<hr><pre>".Dumper($logentry),"</pre>\n";

	    my $rev = $logentry->{'revision'};
	    my $author = $logentry->{'author'};
	    next if $user ne "" && $author ne $user;

	    my $date = $logentry->{'date'};
	    my $time = str2time($date);

	    # Reformat date in local time, preserving fractional part.
	    my $fdate;
	    if ($date =~ /(\.\d+)Z/) {
		$fdate = strftime("%FT%T$1%z", localtime($time));
		$time += $1;
	    } else {
		$fdate = strftime("%FT%T%z", localtime($time));
	    }
	    my $count = @{$logentry->{'paths'}->{'path'}};
	    my $msg = $logentry->{'msg'};
	    chomp($msg);
	    $cset = "<a href=\"$view;revision=$rev\">r$rev</a> | ";
	    $cset .= "$author | ";
	    $cset .= strong("{$fdate}")." | ";
	    $cset .= "line".($count == 1?'':'s')." $count\n";
	    my @paths = @{$logentry->{'paths'}->{'path'}};

	    @paths = sort { $a->{'content'} cmp $b->{'content'} } @paths;

	    foreach my $path (@paths) {
		my $c = $path->{'content'};

		if ($path->{'kind'} eq "dir") {
		    $c .= "/";
		}
		if ($c =~ m{proj/tags}) {
		    if (exists($path->{'copyfrom-path'})) {
			my $cfp = $path->{'copyfrom-path'};
			next LOGENTRY if $cfp !~ m{^/$branchmatch$};
			my $cfr = $path->{'copyfrom-rev'};
			$c .= " (from $path->{'copyfrom-path'}:<a href=\"$view;revision=$cfr\">r$cfr</a>)";
		    } elsif ($c =~ m{^/(?!$branchmatch)}) {
			next LOGENTRY;
		    }
		} elsif (!param('Show Other Branches') &&
			 $c =~ m{^/(?!$match)}) {
		    next;
		}
		$cset .= "   ". $path->{'action'}. " $c\n";
	    }

	    # Protect special chars
	    $msg = escapeHTML($msg);

	    # Add Links for revisions
	    $msg =~ s!\br(\d{5,6})\b!<a href=\"$view;revision=${1}\">r$1</a>!g;

	    # Add links for PRs
	    $msg =~ s!(PR[ #:]*(\d+))!<a href=\"$gnats;pr=$2\">$1</a>!g;

	    # Add links for Jira
	    $msg =~ s!((?:JIRA|PR)[ #:]*(\w+-\d+))!<a href=\"$jira/$2\">$1</a>!gi;
	    # Add links for ReviewBoard
	    $msg =~ s!(RB[ #:]*(\d+))!<a href=\"$rb/$2/\">$1</a>!g;

	    $cset .= $msg;
	    $csets{$time} = $cset;
	}

    }
}

if (!defined(param('branch')) || param('branch') eq "") {
    param('branch' => "trunk");
}
if (!defined(param('deps'))) {
    param('deps' => "<none>");
}
if (!defined(param('user'))) {
    param('user' => "");
}
if (!defined(param('path'))) {
    param('path' => "");
}
if (!defined(param('days')) || param('days') eq "") {
    param('days' => "1");
}
if (!defined(param('date')) || param('date') eq "") {
    param('date' => yymmddd(time - 24*60*60));
}


print header();

my $caption = "SVN Report: ".param('branch')." ".param('deps');

print start_html(-title=>$caption,
		 -head=>[Link({-rel => 'stylesheet',
			       -type => 'text/css',
			       -href => '//code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css'}),
			 Link({-rel => 'shortcut icon',
			       -type => 'image/x-icon',
			       -href => '/favicon.ico'})],
		 -script => [
		      {-src => '//code.jquery.com/jquery-1.10.2.js'},
		      {-src => '//code.jquery.com/ui/1.11.4/jquery-ui.js'},
		      {-type => 'text/javascript',
		       -code => $datejs}]);

print h1(escapeHTML($caption))."\n";
warningsToBrowser(1);

if (!exists $ENV{LOGNAME}) {
    # Running as a web page
#--password Hrun*100 --config-option servers:global:store-plaintext-passwords=yes
    #$svncmd .= "--username hwnsig  ";
} else {
    # Running under a user account for debugging
    $svncmd .= "--config-option servers:global:store-plaintext-passwords=yes "
}

my $cmd;

if (defined(param('rev'))) {
    # If user enters an svn rev, translate that to a date
    $cmd = "$svncmd info -r ".param('rev')." $wlansvn";
    warn "$cmd\n";
    if (`$cmd 2>&1` =~ m/Last Changed Date: (\d+)-(\d+)-(\d+)/) {
	param('date' => join('.', $1, $2+0, $3+0));
	Delete('rev');
    }
}

my $start=str2time(param('date'));
my $end=$start + (24*param('days')+1)*60*60;
my $startrev='{'.(yymmdd($start)).'}';
my $endrev='{'.(yymmdd($end)).'}';
my $interval='{'.(yymmdd($start)).'}:{'.(yymmdd($end)).'}';

my @branches;

my $ddir = "components/deps/";
if (param('include non-active')) {
    $cmd = "$svncmd ls -r '$endrev' $wlansvn/proj/branches";
    warn "$cmd\n";
    @branches = split("/\n", `$cmd 2>&1`);
} elsif (defined(param('branches')) && defined(param('endrev')) &&
    param('endrev') eq $endrev) {
    @branches = split(' ', param('branches'));
} else {
    $cmd = "$hndcmd -Y '$wlansvn/groups/software/infrastructure/GUB/GUB.yaml\@$endrev' query -s active_branches";
    warn "$cmd\n";
    my $ret = `$cmd 2>&1`;
    if ($?) {
	print pre("$cmd\n$ret");
    }
    @branches = sort(grep($_ ne "trunk", split(' ', $ret)));
    param('branches' => join(' ', @branches));
    param('endrev' => $endrev);
}
unshift(@branches, "trunk");
warn "@branches\n";
if (param('branch') eq 'trunk') {
    $ddir .= "trunk";
} else {
    $ddir .= "branches/".param('branch');
}

$cmd = "$svncmd ls -r '$endrev' $wlansvn/$ddir";
warn "$cmd\n";
my @depslist = split("/\n", `$cmd 2>&1`);
if ($?) {
    @depslist = ();
}

if (param('branch') eq 'trunk') {
    unshift(@depslist, "<infrastructure>");
    unshift(@depslist, "<all>");
}
unshift(@depslist, "<none>");

warn "@depslist\n";

my $nopath = 0;
my $pathbox;
if (param('deps') ne "<none>") {
    $nopath = 1;
    param('path' => "");
    $pathbox = td({-style=>"Color: grey"}, 'Path: '.
		  textfield(-name=>'path', -size=>50,-disabled=>'disabled'));
} else {
    if (param('path') eq "") {
	if (param('branch') eq "trunk") {
	    param('path' => 'proj components');
	} else {
	    param('path' => 'src');
	}
    }
    $pathbox = td('Path: '.textfield(-name=>'path', -size=>50));
}
# Strip leading /
if (param('path') =~ m!^/(.*)!) {
    param('path' => $1);
}
# trim trailing whitespace.
if (param('user') =~ /(\S*)\s+$/) {
    param('user' => $1);
}

print start_form({name=>'main', 'accept-charset'=>'utf8'});
print hidden(-name=>'branches')."\n";
print hidden(-name=>'endrev')."\n";
#print hidden(-name=>'x')."\n";

my $bookmark = new CGI();
$bookmark->delete('branches','endrev');

print table({border=>0, summary=>'Form'},
	    [Tr([td('Branch: '.popup_menu(-name=>'branch', -values=>\@branches)).
		 td([checkbox(-name=>'include non-active')])]),
	     Tr([td('Deps: '.popup_menu(-name=>'deps', -values=>\@depslist,
					-default=>['wl-src'])).
		 td('Date: '.textfield(-name=>'date', id=>'datepicker',
				       -size=>10)).
		 td('or rev: '.textfield(-name=>'rev', -size=>10)).
		 td('+'.textfield(-name=>'days', -size=>1)."day(s)").
		 td('User: '.textfield(-name=>'user', -size=>10)).
		 td(submit)]),
	     Tr([$pathbox.
		 td([checkbox(-name=>'Show Other Branches').
		     checkbox(-name=>'Tags only')])]),
	     Tr([td([a({-href=>$bookmark->self_url}, 'Bookmark')])])]);

print end_form."\n";
#if (param('x')) {
#    gettags($startrev, $endrev);
#}

my @dir = ();
if (param('Tags only')) {
    getlog($startrev, $endrev, @dir);
} elsif (param('deps') eq '<infrastructure>') {
    @dir = ('groups/software/infrastructure');
    getlog($startrev, $endrev, @dir);
} elsif (param('deps') eq '<all>') {
    @dir = ('proj', 'components');
    getlog($startrev, $endrev, @dir);
} elsif (param('deps') eq "<none>") {
    my $dir = "";
    if (param('branch') ne 'trunk') {
	$dir = "proj/branches/".param('branch')."/";
    }
    foreach my $p (split(' ' ,param('path'))) {
	push(@dir, $dir.$p);
    }
    push(@dir, "proj/tags");
    getlog($startrev, $endrev, @dir);
} else {
    my $deps = "$ddir/".param('deps')."/DEPS";
    $cmd = "$svncmd log -q -r '$interval' $wlansvn/$deps";
    warn "$cmd\n";
    open(DEPS, "$cmd 2>&1|") || die "Can't read DEPS log: $!";
    my $e = $endrev;
    @dir = readdeps($startrev, $deps);
    while (<DEPS>) {
	/-{72}\n/ && next;;
	warn "$_";
	if (/^r\d+ \| \w+ \| ([^\(]+) /) {
	    $e = "{$1}";
	    push(@dir, "proj/tags");
	    getlog($startrev, $e, @dir);
	    @dir = readdeps($e, $deps);
	    #	    $startrev = "r". ($e + 1);
	    $startrev = $e;
	}
    }
    push(@dir, "proj/tags");
    getlog($startrev, $endrev, @dir);
    if (@ucodedir) {
	getlog($startrev, $endrev, @ucodedir);
    }
}

# Build table, reverse sorted by numeric time
my $table="";
my $count = 0;
foreach my $time (sort {$b <=> $a} keys %csets) {
    $count++;
    $table.="\n".Tr(td(hr.pre($csets{$time})));
}
print "$count records\n";


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
