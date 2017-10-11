#!/usr/bin/perl -w

# UTF cgi script for extracting and plotting performance data from UTF
# log files.

#
# $Id$
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

my %LEGACY = (1 => -12,
	      2 => -11,
	      5.5 => -10,
	      6 => -9,
	      9 => -8,
	      11 => -7,
	      12 => -6,
	      18 => -5,
	      24 => -4,
	      36 => -3,
	      48 => -2,
	      54 => -1);

my %CHANNEL = (36 => 12,
	       40 => 13,
	       44 => 14,
	       48 => 15,
	       52 => 16,
	       56 => 17,
	       60 => 18,
	       64 => 19,
	       149 => 20,
	       153 => 21,
	       157 => 22,
	       161 => 23,
	       165 => 24);


# Work around CGI.pm bugs
# get rid of annoying empty name submit warning
sub Submit { local $^W=0; submit(@_) }

# Standards cleanup
# hidden fails XHTML tests
sub Hidden { CGI::_textfield('hidden', @_) }
sub Checkbox_group {
    my $t = checkbox_group(@_);
    $t =~ s/<table>/<table summary="checkbox_group">/;
    return $t;
}
sub Radio_group {
    my $t = radio_group(@_);
    $t =~ s/<table>/<table summary="radio_group">/;
    return $t;
}

my $gnuplot='/tools/bin/gnuplot 2>&1';
my $caption = "Performance";

# List of paths to search for log files.
# First check to see if we left a cookie, otherwise set defaults
my $DefaultFileList = cookie(-name=>'FileList') ||
    '/home/tima/src/unittest/performance/*/test*.log
/home/tima/src/unittest/test.log';

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
    $p->{in_speed} = $p->{$p->{in_if}."_speed"} if
	exists $p->{in_if} && exists $p->{$p->{in_if}."_speed"};
    $p->{out_speed} = $p->{$p->{out_if}."_speed"} if
	exists $p->{out_if} && exists $p->{$p->{out_if}."_speed"};
    delete $p->{eth0_speed};
    delete $p->{eth1_speed};
    delete $p->{eth2_speed};
    delete $p->{eth3_speed};
    delete $p->{eth4_speed};
    # strip SB speed
#    ($p->{_clkfreq}=$p->{clkfreq}) =~ s/,.*// if exists $p->{clkfreq}; 
    if(exists $p->{clkfreq}) {
	my @c = split(',', $p->{clkfreq});
	if (@c) {
	    $c[1] = 133 unless @c > 1;
	    $c[2] = 33 unless @c > 2;
	    $p->{_clkfreq} = $c[0]/360 + $c[1]/180 + $c[2]/66;
	}
    }

    if ($p->{wlemu_rx} || $p->{wlemu_tx}) {
	if ($p->{wlemu_if} eq $p->{in_if}) {
	    $p->{wlemu_if} = 'in';
	} elsif ($p->{wlemu_if} eq $p->{out_if}) {
	    $p->{wlemu_if} = 'out';
	}
    }
    delete $p->{in_if};
    delete $p->{out_if};
    
    if ($p->{rate} eq "") {
	if($p->{legacy}) {
	    $p->{rate} = "($p->{legacy})";
	} elsif($p->{mcs} == -1) {
	    $p->{rate} = "auto";
	} else {
	    $p->{rate} = $p->{mcs};
	}
    }
    delete $p->{legacy};
    delete $p->{mcs};

    if($p->{rate} =~ /\(([.\d]+)\)/) {
	$p->{_rate} = $LEGACY{$1};
    } elsif($p->{rate} =~ /auto/) {
	$p->{_rate} = 16;
    } else {
	$p->{_rate} = $p->{rate};
    }

    if($p->{chanspec} =~ /(\d+)\w(\w*)/) {
	$p->{_chanspec} = $CHANNEL{$1}||$1;
	$p->{_chanspec} += 0.2 if $2 eq 'l';
	$p->{_chanspec} -= 0.2 if $2 eq 'u';
    }
    
    if(exists $p->{wsec}) {
	my @sec;
	push(@sec, "WEP") if $p->{wsec} & 1;
	push(@sec, "TKIP") if $p->{wsec} & 2;
	push(@sec, "AES") if $p->{wsec} & 4;
	push(@sec, "Software") if $p->{wsec} & 8;
	push(@sec, 'FIPS') if $p->{wsec} & 0x80;
	push(@sec, '') unless @sec;
	$p->{wsec} = join('|', @sec);
    }

    if($p->{wlver} =~ /(\d+)\.(\d+)\.(\d+)\.(\d+)/) {
	$p->{_wlver} = (($1*10+$2)*100+$3)*10+$4;
    }

    unless ($p->{path}) {
	if (exists $p->{in_pci} && exists $p->{out_pci}) {
	    $p->{path} = join('-',(delete $p->{board},
				   delete $p->{in_pci},
				   delete $p->{out_pci}));
	} else {
	    $p->{path} = delete $p->{board};
	}
    }
}

sub drawline {
    my($plots, $start, $end, $val, $axes, $title, $i) = @_;
    push(@{$plots}, qq('-' using 1:2 axes $axes title "$title" with l $i));
    return "$start $val\n$end $val\ne\n";
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
	print qq(set terminal png color\n);
    }

    my %hide;
    foreach my $name (param()) {
	if ($name =~ /^Hide_(.*)/) {
	    map($hide{$1}{$_} = 1, param($name));
	}
    }
    my $ymax = param('Y-Max');
    my $ymin = param('Y-Min');
    my $sort;
    if (param('X-Axis') eq 'time') {
	$sort = sub {$a cmp $b};
    } else {
	$sort = sub {$a <=> $b};
    }

    my %params;
    my %values;

    my @plots;
    my $i = 1;

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
		if(($time, $ap, $parms, $tput) = 
		   m!(\d\d:\d\d:\d\d)\s+XPASS (?:UTF|::)?([-\w]+) (.*): ([\d.]+) Mbit/sec$!) {
		    my @parms = $parms =~ /(\{[^\}]*\}|\S+)/g;
		    map (s/^{(.*)}$/$1/, @parms);
		    my %p = @parms;
		    $p{board}=$ap;
		    $p{time}=$time;
		    $p{tput}=$tput;
		    push(@entries, \%p);
		    map($params{$_}=1, keys %p);
		}
	    }
	    close FILE;
	}
    }

    my $key;
  PASS:
    foreach my $p (@entries) {
	# Fill in all the blanks
	foreach (keys %params) {
	    $p->{$_} = '' unless defined $p->{$_};
	}
	parm_translate($p);
	foreach (keys %{$p}) {
	    next PASS if $hide{$_}{$p->{$_}};
	}
	my $tput = delete $p->{tput};
	next unless defined $tput && $tput ne "";
	next if $ymax ne "" && $p->{tput} > $ymax;
	next if $ymin ne "" && $p->{tput} < $ymin;
	my $x;
	if(param('X-Axis') eq 'time') {
	    if (param('Start Hour') != 0) {
		my @x = split(':', $p->{time}, 3);
		$x[0]+=24 if $x[0] < param('Start Hour');
		$x = join(':', @x);
	    } else {
		$x = $p->{time};
	    }
	} elsif(param('X-Axis') eq 'idle') {
	    $x = delete $p->{idle};
	    $x /= 1000;
	} elsif(exists $p->{"_".param('X-Axis')}) {
	    # Invoke shadow params
	    $x = delete $p->{"_".param('X-Axis')};
	    $xtics{$x} = delete $p->{param('X-Axis')};
	} else {
	    $x = delete $p->{param('X-Axis')};
	}
	foreach (keys %{$p}) {
	    if (/^_|^time$/) {
		# Strip time and shadow keys
		delete $p->{$_};
		next;
	    }
	    $values{$_}{$p->{$_}} = 1;
	}
	$key = '';
	$key .= "$_$;$p->{$_}$;" foreach sort keys %{$p};
	push(@{$stream{$key}->{$x}}, $tput);
	$sum{$key}->{$x} += $tput;
	$num{$key}->{$x} += 1;
    }
    
    

    # Check which parameters varied so we can exclude them from the
    # plot name
    my @keys;
    my @titlekeys;
    foreach my $k (sort keys %values) {
	next if grep($_ eq $k, (param('Ignore')));

	my $n = keys %{$values{$k}};
	my $v = join('',keys %{$values{$k}});
	if ($n == 1 && $v ne '') {
	    push(@titlekeys, "$k=$v");
	} elsif ($n > 1) {
	    push(@keys, $k);
	}
    }
    my $Rsmean=0;
    my $xmean;
    foreach my $s (sort keys %stream) {
	print "# $s\n";
	my %p = split($;, $s);
	my $average = "";
	my $title = join('', map("$_=".(exists $p{$_}?$p{$_}:'').' ', @keys));
#	$title =~ s/(.{80}).*/$1/;
	push(@plots, qq('-' using 1:2 title "$title" with lp $i));
	my $lp;
	my $needdummy = 1;
	my $Rs;
	my $Rssum = 0;
	my $Rsnum = 0;
	my $Rsdata = "";
	my $start;
	my $end;
	my $xsum;
	my $xnum;
	foreach my $c (sort $sort keys %{$stream{$s}}) {
	    if (defined $start) {
		$end = $c;
	    } else {
		$start = $c;
	    }
	    foreach my $p (@{$stream{$s}->{$c}}) {
		if ($needdummy) {
		    $data .= "$c $p\ne\n"; # Dummy for title
		    $needdummy = 0;
		}
		$data .= "$c $p\n";
		if (param('X-Axis') ne 'time') {
		    $xtics{$c} = $c unless exists $xtics{$c};
		}
		if (defined $lp) {
		    $Rs = abs($p - $lp);
		    $Rssum+= $Rs;
		    $Rsnum++;
		    $Rsdata .= "$c $Rs\n";
		}
		$xsum += $p;
		$xnum ++;

		$lp = $p;
	    }
	    $average.= "$c ".(($sum{$s}->{$c})/($num{$s}->{$c}))."\n";
	}
	push(@plots, qq('-' using 1:2 notitle with p $i));
	push(@plots, qq('-' using 1:2 notitle with l $i));
	
	$data .= "e\n${average}e\n";

	$i++;

	push(@plots, qq('-' using 1:2 axes x1y2 notitle with l $i));
	$data .= "${Rsdata}e\n";

	$i++;

	$Rsmean = $Rssum/$Rsnum;
	$data .= drawline(\@plots, $start, $end, $Rsmean, 'x1y2', "Rs mean", $i++); 

	$xmean = $xsum/$xnum;
	$data .= drawline(\@plots, $start, $end, $xmean, 'x1y1', "x mean", $i++);

	$data .= drawline(\@plots, $start, $end, $xmean + (2.66*$Rsmean), 'x1y1', "3sigma", $i);
	$data .= drawline(\@plots, $start, $end, $xmean - (2.66*$Rsmean), 'x1y1', "3sigma", $i++);

 	$data .= drawline(\@plots, $start, $end, 3.267*$Rsmean, 'x1y2', "UCL", $i++);

    }



    print "set title \"".join("\\n", @titlekeys)."\"\n";
    print qq(set ylabel "Throughput (Mbit/sec)"\n);
    print qq(set y2label "Rs"\n);
    print qq(set y2tics\n);
    print qq(set ytics nomirror\n);
    print "set y2range [0:".($Rsmean*10)."]\n";
    print "set yrange [".($xmean - 10*$Rsmean).":]\n";
    if(param('X-Axis') eq 'time') {
	print qq(set xlabel "Time"\n);
	print qq(set xdata time\n);
	print qq(set timefmt "%H:%M:%S"\n);
	print qq(set format x "%H:%M"\n);
    } elsif(param('X-Axis') eq 'idle') {
	print qq(set xlabel "Idle Cycles (x1000)"\n);
    } elsif(param('X-Axis') eq 'clkfreq') {
	print qq(set xlabel "CPU Speed (MHz)"\n);
    } else {
	print "set xlabel '".param('X-Axis')."'\n";
    }
#    if (param('nokey')) {
#	print qq(set nokey\n);
#    } else {
#	print qq(set key below\n);
#    }
    if(param('Size') && param('Size') =~ /^(\d+)x(\d+)$/) {
	print "set size ".($1/640).",".(($2+2*@plots)/480)."\n";
    }

#    print qq(set format "%.1s%c"\n); 
    print "set xtics rotate (".(join(',',
				       map("'".$xtics{$_}."' ".$_,
					   sort keys %xtics))).")\n" if %xtics;
    print "plot ".join(', ', @plots)."\n";
    print "$data\n";

    print "show label\n";
    print "set output\n";
    print "exit\n";
    exit;
} else {

    print header(-cookie=>$filelistcookie);
    print start_html(-title=>$caption,
		     -head=>meta({-http_equiv => 'Content-Type',
				   -content =>
				       'text/html, charset=iso-8859-1'}));
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
	if(open(FILE, $file)) {
	    $files++;
	    while(<FILE>) {
		if(($ap, $parms) = 
		   m!XPASS (?:UTF|::)?([-\w]+) (.*): [\d.]+ Mbit/sec$!) {
		    my @parms = $parms =~ /(\{[^\}]*\}|\S+)/g;
		    map (s/^{(.*)}$/$1/, @parms);		    
		    my %p = @parms;
		    $p{board}=$ap;
		    parm_translate(\%p);
		    push(@entries, \%p);
		    map($params{$_}=1, keys %p);
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

    my %Xvars = (time=>1);
    my %cols; # max columns for displaying checkboxes
    foreach (sort keys %parmlist) {
	my @k = keys %{$parmlist{$_}};
	# List numeric variables, ignoring '_' for translated fields
	$Xvars{$1}=1 if grep(/^-?\d+(:?\.\d+)?$/, @k) > 1 && /^_?(.*)/;
	if (@k > 1) {
	    $cols{$_} = int(80 * @k / length(join(', ', @k)));
	} else {
	    $cols{$_} = 1;
	}
	# hide shadow params from selectors
	delete $parmlist{$_} if /^_/;
    }

    my $controlpaneltable =
	table({border=>1, summary=>'Control Panel'},
	      Tr(th("Files").
		 td(textarea(-name=>'FileList',
			     -default=>$DefaultFileList,
			     -rows=>10,
			     -columns=>80,
			     ))).
	      Tr(th('X-Axis').
		 td([Radio_group
		     (-name=>'X-Axis',
		      -values=>[sort keys %Xvars],
		      )])).
	      Tr(th('Y Limits').
		 td(' Min: '.
		    textfield(-name=>'Y-Min', -size=>5).
		    ' Max: '.
		    textfield(-name=>'Y-Max', -size=>5).
		    ' Start Hour: '.
		    popup_menu(-name=>'Start Hour', -values=>[0..23])))
	      );

    my $paramhidetable =
	table({border=>1, summary=>'Display Panel'},
	      Tr([td([Checkbox_group
		      (-name=>"Ignore",
		       -columns=>8,
		       -values=>
		       [sort keys %parmlist])])]));

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
					  '1024x768',
					  ])),
		  td(Submit('Redraw')),
		  ]));
    my $numortext = sub {
	if($a =~ /^\d+$/ && $b =~ /^\d+$/) {
	    $a <=> $b;
	} else {
	    $a cmp $b;
	}
    };

    my $recordhidetable = 
	table({border=>1, summary=>'Filter Panel'},
	      Tr([map
		  (th($_).
		   td([Checkbox_group
		       (-name=>"Hide_$_",
			-columns=>$cols{$_},
			-values=>
			[sort $numortext keys %{$parmlist{$_}}])]),
		   sort keys %parmlist),
		  ]));

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
		    td({-colspan=>2}, 'Check boxes below to '.b('hide').
		       ' records.'),
		    td({-colspan=>2}, $recordhidetable),
		    td({-colspan=>2}, $othertable)
#		    .td({-colspan=>1},
#		       a({-href=>"../performance/UTFGigELab.png"},
#			 img({src=>"../performance/UTFGigELab128.png",
#			      alt=>"Lab Network", border=>0})))
		    ]));

    print end_form;
    
#    print Dump;
   
    # Display validator stamp.  Make sure you validate the page after any
    # change to this script!
    print p(a({-href=>"http://tidy.sourceforge.net"},
	      img({src=>"checked_by_tidy.gif",
		   alt=>"Checked by Tidy!", border=>"0",
		   height=>"32", width=>"32", align=>'right'}))),"\n";
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
