#!/usr/bin/perl -w

# UTF cgi script for extracting and plotting calibration data from UTF
# log files.

#
# $Id: 8f85ad7106c73ade698f6600022a24c4c752a9d5 $
# $Copyright Broadcom Corporation$
#

use strict;
use CGI qw(:standard -xhtml -nosticky);
use CGI::Carp qw(fatalsToBrowser warningsToBrowser);

$CGI::POST_MAX=1024 * 100;  # max 100K posts
$CGI::DISABLE_UPLOADS = 1;  # no uploads

# Prevent cgi forms warnings when run from commandline.
$ENV{QUERY_STRING}='' unless exists $ENV{QUERY_STRING};
$ENV{SHELL}='/bin/bash';

my $gnuplot='/tools/bin/gnuplot 2>&1';
my $caption = "Calibration";

# List of paths to search for log files.
# First check to see if we left a cookie, otherwise set defaults
my $DefaultFileList = cookie(-name=>'FileList') ||
    '/home/tima/src/unittest/test*.log';

# Set up cookie to be saved for next time.
my $filelistcookie = cookie(-name=>'FileList',
			    -value=>[param('FileList')],
			    -path=>url(-absolute=>1),
			    -expires=>'+2y');

# Can't use glob() since it fails if there are any unreadable files in
# a wildcarded parent directory.
my @files;
{
    my @f = split(' ', param('FileList'));
    my $f;
    if(@f > 1) {
	$f =  '{'.join(',', split(' ', param('FileList'))).'}';
    } else {
	$f = $f[0];
    }
    @files = split(' ', `bash -c 'echo $f'`);
}
# Parameters to be totally ignored
my @ignore = ();

sub parm_translate {
    my($p) = @_;
}

if(param('image')) {

    $|=1; # Make sure header gets out first
    
    if(param('Gnuplot Source')) {
	print header(-type=>'text/plain',
		     -Refresh=>'10');
    } elsif(param('Gnuplot Output')) {
	print header(-type=>'text/plain');
	open(STDOUT, "|$gnuplot >/dev/null")
	    or die "Can't run $gnuplot: $!\n";
    } else {
	if(param('Use SVG')) {
	    print header(-type=>'image/svg+xml');
	} else {
	    print header(-type=>'image/png',
			 -Refresh=>'10');
	}
	open(STDOUT, "|$gnuplot 2>/dev/null")
	    or die "Can't run $gnuplot: $!\n";
    }
    if(param('Use SVG')) {
	print qq(set terminal svg\n);
    } else {
	print qq(set terminal png\n);
    }

    my %hide;
    foreach my $name (param()) {
	if ($name =~ /^Hide_(.*)/) {
	    map($hide{$1}{$_} = 1, param($name));
	}
    }
    my $sort = sub {$a cmp $b};

    my %params;
    my %values;

    my @plots;
    my $i = 2;

    my %xtics;
    my %ytics;
    my %cpu;
    my ($m, $s);
    my %mgrid;
    my %sgrid;
    my $data = '';
    my %stream;
    my %sum;
    my %num;

    my @entries;
    foreach my $file (@files) {
	my $c = 0;
	my $p = '';
	my %datum;
	my $title;
	my $time;
	my $r = 0;
	my $d;
	my $ap;
	my $parms;
	my $tput;
	if(open(FILE, $file)) {
	    while(<FILE>) {
		if(($time, $ap, $parms, $tput) = m!(\d\d:\d\d:\d\d)\s+XPASS (?:UTF|::)?([-/\w]+) (.*): ([\d.]+)!) {
		    my @parms = $parms =~ /(\{[^\}]*\}|\S+)/g;
		    map (s/^{(.*)}$/$1/, @parms);
		    my %p = @parms;
		    $p{time} = $time;
		    $p{tput} = $tput;
		    push(@entries, \%p);
		} elsif(($time, $ap, $parms) = 
		   m!(\d\d:\d\d:\d\d)\s+XPASS ([-\w]+) (.*):!) {
		    my @parms = split(' ', $parms);
		    my %p = @parms;
		    $p{time}=$time;
		    $p{tput}=$tput;
		    push(@entries, \%p);
		}
	    }
	    close FILE;
	}
    }

  PASS:
    foreach my $p (@entries) {
	parm_translate($p);
	
	my $x;
	if (param('Start Hour') ne "0") {
	    my @x = split(':', $p->{time}, 3);
	    $x[0]+=24 if $x[0] < param('Start Hour');
	    $x = join(':', @x);
	} else {
	    $x = $p->{time};
	}
	delete $p->{time};
	foreach my $key (keys %{$p}) {
	    push(@{$stream{$key}->{$x}}, $p->{$key});
	    $sum{$key}->{$x} += $p->{$key};
	    $num{$key}->{$x} += 1;
	}
    }
    
    my $needy = 0;
    foreach my $s (sort keys %stream) {
	next if grep($_ eq $s, (param('Ignore')));
	print "# $s\n";
	my $average = "";
	my $title = $s;
	my $a = 'x1y2';
	my $w = 'p';
	if ($s eq "tput") {
	    $i = 1;
	    $needy = 1;
	    $a = 'x1y1';
	    $w = 'lp';
	}
	push(@plots, qq('-' using 1:2 axes $a title "$title" with lp $i));

	my $needdummy = 1;
	foreach my $c (sort $sort keys %{$stream{$s}}) {
	    foreach my $p (@{$stream{$s}->{$c}}) {
		if ($needdummy) {
		    $data .= "$c $p\ne\n"; # Dummy for title
		    $needdummy = 0;
		}
		$data .= "$c $p\n";
	    }
	    $average.= "$c ".(($sum{$s}->{$c})/($num{$s}->{$c}))."\n";
	}
	push(@plots, qq('-' using 1:2 axes $a notitle with p $i));
	push(@plots, qq('-' using 1:2 axes $a notitle with l $i));
	$data .= "e\n${average}e\n";
	$i++;
    }
    
    print qq(set xlabel "Time"\n);
    if ($needy) {
	print qq(set ylabel "Throughput"\n);
    }
    print qq(set ytics nomirror\n);
    print qq(set y2label "Calibration"\n);
    print qq(set xdata time\n);
    print qq(set timefmt "%H:%M:%S"\n);
    print qq(set format x "%H:%M"\n);
    print qq(set y2tics\n);
    print qq(set key outside\n);
    
    if(param('Size') && param('Size') =~ /^(\d+)x(\d+)$/) {
	print "set size ".($1/640).",".($2/480)."\n";
    }

    print "plot ".join(', ', @plots)."\n";
    print "$data\n";

    print "show label\n";
    print "set output\n";
    print "exit\n";
    exit;
} else {

    print header(-cookie=>$filelistcookie);
    print start_html(-title=>$caption);
    print h1($caption)."\n";
    warningsToBrowser(1);

    my %params;
    my %parmlist;
    my $records = 0;
    my $files = 0;
    my @entries;
    foreach my $file (@files) {
	my $ap;
	my $parms;
	my $tput;
	if(open(FILE, $file)) {
	    $files++;
	    while(<FILE>) {
		if(($ap, $parms, $tput) = m!XPASS (?:UTF|::)?([-\w]+) (.*): ([\d.]+)!) {
		    my @parms = $parms =~ /(\{[^\}]*\}|\S+)/g;
		    map (s/^{(.*)}$/$1/, @parms);		    
		    my %p = @parms;
		    $p{tput} = $tput;
		    parm_translate(\%p);		    
		    push(@entries, \%p);
		}
	    }
	    close $file;
	}
    }

    # Fill in all the blanks
    foreach my $p (@entries) {
	foreach (keys %params) {
	    $p->{$_} = '' unless defined $p->{$_};
	}
	map($parmlist{$_}{$p->{$_}} = 1, keys %{$p});
	$records++;
    }

    my $s = new CGI;
    $s->param(image=>1);
    $s->param('Use SVG'=>1);
    my $o = new CGI;
    $o->param(image=>1);
    $o->param('Gnuplot Output'=>1);
    my $g = new CGI;
    $g->param(image=>1);
    $g->param('Gnuplot Source'=>1);

    my %cols; # max columns for displaying checkboxes
    foreach (sort keys %parmlist) {
	my @k = keys %{$parmlist{$_}};
	if (@k > 1) {
	    $cols{$_} = int(80 * @k / length(join(', ', @k)));
	} else {
	    $cols{$_} = 1;
	}
    }

    my $controlpaneltable =
	table({border=>1, summary=>'Control Panel'},
	      Tr(th("Files").
		 td(textarea(-name=>'FileList',
			     -default=>$DefaultFileList,
			     -rows=>10,
			     -columns=>80,
			     ))).
	      Tr(td(' Start Hour: '.
		    popup_menu(-name=>'Start Hour', -values=>[0..23])))
	      );

    my $paramhidetable = "";
    if (keys %parmlist) {
	$paramhidetable = 
	    table({border=>1, summary=>'Display Panel'},
		  Tr([td([checkbox_group
			  (-name=>"Ignore",
			   -columns=>8,
			   -values=>
			   [sort keys %parmlist])])]));
    }

    my $sizetable = 
	table({border=>0, summary=>'Size'},
	      Tr([th($files.
		     "&nbsp;log&nbsp;file".($files!=1?'s':'')),
		  th($records.
		     "&nbsp;record".($records!=1?'s':'')),
		  td(popup_menu(-name=>'Size',
				-values=>['320x240',
					  '320x360',
					  '640x420',
					  '640x480',
					  '640x720',
					  '640x960',
					  '800x480',
					  '1024x480',
					  '1024x768',
					  ])),
		  td(submit('Redraw')),
		  ]));
    my $numortext = sub {
	if($a =~ /^\d+$/ && $b =~ /^\d+$/) {
	    $a <=> $b;
	} else {
	    $a cmp $b;
	}
    };

    my $othertable = 
	table({border=>0, summary=>'Other Options'},
	      Tr([td([a({-href=>$s->self_url}, 'SVG'),
		      a({-href=>$o->self_url,
			 -target=>'Output'}, 'Gnuplot Output'),
		      ]),
		  td([a({-href=>self_url}, 'Bookmark'),
		      defaults,
		      a({-href=>$g->self_url,
			 -target=>'Source'}, 'Gnuplot Source'),
		      ]),
		  ]));
    
    print start_form;
    print table({border=>0, summary=>'Layout'},
		Tr([td({-colspan=>2}, graph()),
		    td({-valign=>'top'}, [$controlpaneltable, $sizetable]),
		    td({-colspan=>2}, 'Check items below to '.b('hide').
		       ' parameters from display.'),
		    td({-colspan=>2}, $paramhidetable),
		    td({-colspan=>2}, $othertable)
		    ]));

    print end_form;
    
#    print Dump;
   
    # Display validator stamp.  Make sure you validate the page after any
    # change to this script!
    print p(a({-href=>"http://tidy.sourceforge.net"},
	      img({src=>"checked_by_tidy.gif",
		   alt=>"Checked by Tidy!", border=>"0",
		   height=>"32", width=>"32", align=>'right'}))),"\n";

    print end_html;
}
exit;

sub graph {
    # Clone CGI object so we can modify it for use in links. 
    my $n = new CGI();
    $n->param(image=>1);
    while (@_ > 1) {
	my $k = shift;
	my $v = shift;
	$n->param($k => $v);
    }
    my $m = new CGI($n);
    $m->param('Use SVG'=>0);
    return a({-href=>$n->self_url},img({src=>$m->self_url,
					alt=>"@_"}));
}
