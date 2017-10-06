#!/usr/bin/perl -w

# UTF cgi script for extracting and plotting usage data from
# GridEngine.  This must be executed from a web server that is also a
# GridEngine server.

#
# $Id$
# $Copyright Broadcom Corporation$
#

use strict;
use CGI qw(:standard -xhtml -nosticky);
use CGI::Carp qw(fatalsToBrowser warningsToBrowser);
use Date::Parse;

$CGI::POST_MAX=1024 * 100;  # max 100K posts
$CGI::DISABLE_UPLOADS = 1;  # no uploads

# Prevent cgi forms warnings when run from commandline.
$ENV{QUERY_STRING}='' unless exists $ENV{QUERY_STRING};
$ENV{SHELL}='/bin/bash';

# Standards cleanup
sub Checkbox_group {
    my $t = checkbox_group(@_);
    $t =~ s/<table>/<table summary="checkbox_group">/;
    return $t;
}

my $gnuplot='/tools/bin/gnuplot 2>&1';
my $caption = "Grid Engine usage";

if (!defined(param('Days')) || param('Days') !~ /^\d+$/) {
    param('Days'=>1);
}

if(param('image')) {

    $|=1; # Make sure header gets out first
    
    if(param('Gnuplot Source')) {
	print header(-type=>'text/plain');

    } elsif(param('Gnuplot Output')) {
	print header(-type=>'text/plain');
	open(STDOUT, "|$gnuplot >/dev/null")
	    or die "Can't run $gnuplot: $!\n";
    } else {
	print header(-type=>'image/png');
	open(STDOUT, "|$gnuplot 2>/dev/null")
	    or die "Can't run $gnuplot: $!\n";
    }
    print qq(set terminal png color\n);

    if(param('Size') && param('Size') =~ /^(\d+)x(\d+)$/) {
	print "set size ".($1/640).",".($2/480)."\n";
    }

    my $showqueues = join('|', param("queues"));

    # offset for PST, since gnuplot doesn't understand timezones.
    my $zone = (8-(localtime)[8])*60*60;

   print "set title '$caption'\n";
#    print qq(set ylabel "Throughput (Mbit/sec)"\n);

#    print qq(set ytics nomirror\n);
    print qq(set nokey\n);
    print "set xdata time\n";
    print "set timefmt '%s'\n";
    print "set format x \"%m/%d\\n%H:%M\"\n";
#    print "set xtics rotate\n";
    my @plots = ();
    my %data;
    my $data = "";
    my ($qname, $start, $end);
    my $first = time()-$zone;
    my $last = 0;

    my $days = param('Days');
    
    open(ACCT, "{ . /usr/sge/default/common/settings.sh; qacct -d $days -j; } 2>&1|") || die "$!";
    while(<ACCT>) {
	#print $_;
	if (/^qname\s+(\S+)/) {
	    $qname = $1;
	} elsif (/^start_time\s+(\S.*)/) {
	    $start = str2time($1)-$zone;
	    $data{$qname}.= "$start 1\n";
	    if ($start < $first) {
		$first = $start;
	    }
	    if ($start > $last) {
		$last = $start;
	    }
	} elsif (/^end_time\s+(\S.*)/) {
	    $end = str2time($1)-$zone;
	    $data{$qname}.= "$end 0\n";
	    if ($end < $first) {
		$first = $end;
	    }
	    if ($end > $last) {
		$last = $end;
	    }
	}
    }
    close(ACCT);
    print "# first is ".localtime($first)." last is ".localtime($last)."\n";
    my $i = 0;
    my @ytics;
    foreach $qname (sort keys %data) {
	next if $showqueues && $qname !~ /^$showqueues$/;
	push(@ytics, qq("$qname" $i));
	push(@plots, qq('-' using 1:($i+\$2/2) title '$qname' with steps $i));
	$data .= "$first 0\n$data{$qname}$last 0\ne\n";
	$i++;
    }
    print "set yrange [-0.5:$i]\n";
    print "set ytics (".join(', ', @ytics).")\n";

    print "plot ".(join(', ',@plots))."\n";
    print $data;
    exit;
} else {

    print header();
    print start_html(-title=>$caption,
		     -head=>meta({-http_equiv => 'Content-Type',
				   -content =>
				       'text/html, charset=iso-8859-1'}));
    print h1($caption)."\n";
    warningsToBrowser(1);

    my %queues;

    foreach my $q (`. /usr/sge/default/common/settings.sh; qconf -sql`) {
	chop $q;
	$queues{$q} = 1;
    }
    unless(param("queues")) {
	param(queues=>(keys %queues));
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

    my $cols; # max columns for displaying checkboxes
    my @k = keys %queues;
    if (@k > 1) {
	$cols = int(80 * @k / length(join(', ', @k)));
    } else {
	$cols = 1;
    }

    my $controlpaneltable =
    my $sizetable = 
	table({border=>0, summary=>'Size'},
	      Tr([td(popup_menu(-name=>'Size',
				-default=>'640x480',
				-values=>['320x240',
					  '320x360',
					  '640x420',
					  '640x480',
					  '640x720',
					  '640x960',
					  '800x480',
					  '800x600',
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

    my $recordshowtable = 
	table({border=>1, summary=>'Filter Panel'},
	      Tr((td([Checkbox_group
		      (-name=>"queues",
		       -columns=>$cols,
		       -values=>
		       [sort $numortext keys %queues])]),
		  )));

    my $othertable = 
	table({border=>0, summary=>'Other Options'},
	      Tr([td([a({-href=>$o->self_url,
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
		    td({-colspan=>2}, ' Number of days to show: '.
		       textfield(-name=>'Days', -default=>"1", -size=>5)),
		    td($sizetable),
		    td({-colspan=>2}, 'Uncheck boxes below to '.b('hide').
		       ' records.'),
		    td({-colspan=>2}, $recordshowtable),
		    td($othertable),
		    ]));

    print end_form;

#    print Dump;

#    print pp(\%show);

    print p(a({-href=>"http://tidy.sourceforge.net"},
	      img({src=>"checked_by_tidy.gif",
		   alt=>"Checked by Tidy!", border=>"0",
		   height=>"32", width=>"32", align=>'right'}))),"\n";

    print end_html;

}
exit;

sub pp {
    my ($v) = @_;
    if (ref($v) eq 'HASH') {
	my $l;
	foreach (sort keys %{$v}) {
	    $l .= li("$_ => ".pp($v->{$_}))."\n";
	}
	if ($l) {
	    return ul($l);
	} else {
	    return "";
	}
    } else {
	return "$v\n";
    }
}

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
    $n->param('Size'=>'640x480');
    return a({-href=>$n->self_url},img({src=>$m->self_url,
					alt=>"@_"}));
}


