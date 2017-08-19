#!/usr/bin/perl -w

# UTF cgi script for extracting and plotting performance data from UTF
# log files.

#
# $Id: 1a7c92af1c01f01e06a6f489efa7d3d8b4f06f80 $
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
my $caption = "Control Charts";

if (!defined(param('nsigma')) || param('nsigma')!~/^[\d.]+$/) {
    param('nsigma'=>'3.0');
}

my $dir = url(-absolute=>1);
$dir =~ s![^/]*$!!;
$dir =~ s!/~!/home/!;

my @files;

if (opendir(DIR, $dir)) {
    foreach (sort readdir(DIR)) {
	if (/(.*)\.data$/) {
	    push(@files, $1);
	}
    }
}

my $files = @files;

if(param('image')) {

    $|=1; # Make sure header gets out first
    
    if(param('Gnuplot Source')) {
	print header(-type=>'text/plain');
    } elsif(param('Gnuplot Output')) {
	print header(-type=>'text/plain');
	open(STDOUT, "|$gnuplot >/dev/null")
	    or die "Can't run $gnuplot: $!\n";
    } else {
	if(param('Use SVG')) {
	    print header(-type=>'image/svg+xml');
	} else {
	    print header(-type=>'image/png', -Refresh=>'10');
	}
	open(STDOUT, "|$gnuplot 2>/dev/null")
	    or die "Can't run $gnuplot: $!\n";
    }
    if(param('Use SVG')) {
	print qq(set terminal svg\n);
    } else {
	print qq(set terminal png color\n);
    }

    my $name = param('File');
    $name =~ s!.*/!!;
    $name =~ s!\.data$!!;

    my $showtext = 1;
    if(param('Size') && param('Size') =~ /^(\d+)x(\d+)$/) {
	print "set size ".($1/640).",".($2/480)."\n";
	$showtext = 0 if $1 < 320; # Too small for text
    }

    if ($showtext) {
	unless(param('Hide Title')) {
	    print "set title 'Throughput Control Chart $name'\n";
	    print qq(set ylabel "Throughput (Mbit/sec)"\n);
	}
	unless(param('Hide R')) {
	    print qq(set y2label "R (Mbit/sec)"\n);
	    print qq(set y2tics\n);
	}
	print qq(set ytics nomirror\n);
	print qq(set key outside\n);
    } else {
	print qq(unset key\n);
	print qq(unset xtics\n);
	print qq(unset ytics\n);
	print qq(unset border\n);
    }

    # Fixed Sample size 5
    my $A2 = 0.577;
    my $D3 = 0.000;
    my $D4 = 2.115;

    {
	# Convert to N sigma
	my $s = param('nsigma')/4.0;
	$A2 = $A2 * $s;
	$D3 = 0;
	$D4 = 1 + $s*($D4 - 1);
    }

    if(open(FILE, "$dir/".param('File').".data")) {
	my ($min, $max, $R);
	my $mean = 0;
	my $meansum = 0;
	my $Rsum = 0;
	my $group = 0;
	my $mmmlist = "";
	my $Rlist = "";
	while(<FILE>) {
	    if(($mean, $min, $max) = /^([\d.]+)\s+([\d.]+)\s+([\d.]+)$/) {
		$meansum += $mean;
		$R = $max - $min;
		$Rsum += $R;
		$group++;
		
		$mmmlist.="$mean $min $max\n";
		$Rlist.= "$R\n";
		
	    }
	}
	next unless $group;
	my $meanmean = $meansum/$group;
	my $Rmean = $Rsum/$group;
	my $ux = $meanmean + $A2 * $Rmean;
	my $lx = $meanmean - $A2 * $Rmean;
	my $ur = $D4 * $Rmean;
	my $lr = $D3 * $Rmean;
	
	print "set y2range [0:".($ur*4)."]\n";
	print "set offsets 0, 0, 0, $ur\n";

	my @plots = ();
	my $data = "";

	push(@plots, qq('-' using 0:1:2:3 axes x1y1 title 'x' with errorlines 1));
	$data .= "${mmmlist}e\n";

	unless(param('Hide Limits')) {

	    push(@plots, qq('-' axes x1y1 notitle with l 5));
	    $data .= "0 $meanmean\n$group $meanmean\ne\n";

	    push(@plots, qq('-' axes x1y1 notitle with l 2));
	    $data .= "0 $ux\n$group $ux\ne\n";

	    push(@plots, qq('-' axes x1y1 notitle with l 2));
	    $data .= "0 $lx\n$group $lx\ne\n";
	}

	unless(param('Hide R')) {
	    push(@plots, qq('-' axes x1y2 title 'R' with lp 4));
	    $data .= "${Rlist}e\n";
	    push(@plots, qq('-' axes x1y2 notitle with l 5));
	    $data .= "0 $Rmean\n$group $Rmean\ne\n";
	    push(@plots, qq('-' axes x1y2 notitle with l 3));
	    $data .= "0 $ur\n$group $ur\ne\n";
	    push(@plots, qq('-' axes x1y2 notitle with l 3));
	    $data .= "0 $lr\n$group $lr\ne\n";
	}	    
	print "plot ".(join(', ',@plots))."\n";
	print $data;
    }
    exit;
} else {

    print header();
    print start_html(-title=>$caption,
		     -head=>meta({-http_equiv => 'Content-Type',
				   -content =>
				       'text/html, charset=iso-8859-1'}));
    print h1($caption)."\n";
    warningsToBrowser(1);

    my $s = new CGI;
    $s->param(image=>1);
    $s->param('Use SVG'=>1);
    my $o = new CGI;
    $o->param(image=>1);
    $o->param('Gnuplot Output'=>1);
    my $g = new CGI;
    $g->param(image=>1);
    $g->param('Gnuplot Source'=>1);

    my %show;
    foreach my $name (param()) {
	if ($name =~ /^Show_(.*)/) {
	    map($show{$1}{$_} = 1, param($name));
	}
    }

    my %parmlist;
    my %showing;
    foreach my $f (@files) {
	my $field = 0;
	my @fields = $f =~ /(\{[^\}]*\}|\S+)/g;
	foreach (@fields) {
	    $parmlist{$field}{$_}=1;
	    $showing{$f} = 1 if exists $show{$field}{$_};
	    $field++;
	}
    }

    my $sizetable = 
	table({border=>0, summary=>'Size'},
	      Tr([th($files.
		     "&nbsp;log&nbsp;file".($files!=1?'s':'')),
		  td(submit('Redraw')),
		  ]));

    my $filetable = 
	table({border=>1, summary=>'Files'},
	      (map {
		  Tr(th([$_,
			 $showing{$_}?graph(File=>$_, Size=>'256x128'):'']));
	      } @files));
	      

    my $numortext = sub {
	if($a =~ /^\d+$/ && $b =~ /^\d+$/) {
	    $a <=> $b;
	} else {
	    $a cmp $b;
	}
    };

    my $recordshowtable = "";
    if (keys %parmlist) {
	$recordshowtable = 
	    table({border=>1, summary=>'Filter Panel'},
		  Tr([map
		      (th($_).
		       td([checkbox_group
			   (-name=>"Show_$_",
			    -values=>
			    [sort $numortext keys %{$parmlist{$_}}])]),
		       sort keys %parmlist),
		      ]));
    }

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
		Tr([
		    td("Dir: $dir").td($sizetable),
		    td('N sigma: '.
		       textfield(-name=>'nsigma', -size=>4)),
		    td(checkbox(-name=>'Hide Title')),
		    td(checkbox(-name=>'Hide Limits')),
		    td(checkbox(-name=>'Hide R')),
		    td({-colspan=>2}, 'Check boxes below to '.b('show').
		       ' records.'),
		    td({-colspan=>2}, $recordshowtable),
		    td($filetable),
		    td({-colspan=>2}, $othertable)
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
    $m->param('Use SVG'=>0);
    $n->param('Size'=>'640x480');
    return a({-href=>$n->self_url},img({src=>$m->self_url,
					alt=>"@_"}));
}


