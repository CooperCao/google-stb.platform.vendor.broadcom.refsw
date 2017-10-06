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
use Time::Local;

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

# Standards cleanup
sub Checkbox_group {
    my $t = checkbox_group(@_);
    $t =~ s/<table>/<table summary="checkbox_group">/;
    return $t;
}

my $gnuplot='/tools/bin/gnuplot 2>&1';
my $caption = "Performance";
if(defined(param('Title')) && param('Title') eq "Memory") {
    $caption = 'Memory';
}

# List of paths to search for log files.
# First check to see if we left a cookie, otherwise set defaults
my $DefaultFileList = cookie(-name=>'FileList') || '../test*.log';

# Set up cookie to be saved for next time.
my $filelistcookie = cookie(-name=>'FileList',
			    -value=>[param('FileList')],
			    -path=>url(-absolute=>1),
			    -expires=>'+2y');

# Can't use glob() since it fails if there are any unreadable files in
# a wildcarded parent directory.

if (!defined(param('FileList'))) {
    param('FileList'=>'../test*.log');
}
if (defined(param('Size')) && param('Size') eq 'dynamic') {
    param('Use SVG' => 'on');
}

my @files;
{
    my @f = split(' ', param('FileList'));
    my $f;
    if(@f > 1) {
	$f =  '{'.join(',', split(' ', param('FileList'))).'}';
    } else {
	$f = $f[0];
    }
    @files = split(' ', `bash -c 'echo $f\{,.gz}'`);
}
# Parameters to be totally ignored
my @ignore = ();
if (!defined(param('AutoRate')) || param('AutoRate') eq "") {
    param(AutoRate=>16);
}

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

    if (exists $p->{rate}) {
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

	if($p->{rate} =~ /auto/) {
	    $p->{rate} = "(auto)";
	    $p->{_rate} = param('AutoRate');
	    if(param('WrapMCS')) {
		$p->{txs} = 1+int((param('AutoRate')-1) / 8);
		$p->{_rate} = 1 + ((param('AutoRate')-1) % 8);
	    }
	} elsif($p->{rate} =~ /\((\S+)\)/) {
	    $p->{_rate} = $LEGACY{$1};
	} elsif($p->{rate} =~ /32/) {
	    $p->{_rate} = -0.5;
	} elsif(param('WrapMCS')) {
	    $p->{_rate} = $p->{rate} % 8;
	    $p->{txs} = 1+int($p->{rate} / 8);
	} elsif($p->{rate} =~ /(\d+)\*/) {
	    $p->{_rate} = $p->{rate} + 0.5;
	} elsif($p->{rate} =~ /(\d+)sgi/) {
	    $p->{_rate} = $1 + 0.5;
	} elsif($p->{rate} =~ /(\d+)stbc/) {
	    $p->{_rate} = $1 + 0.25;
	} elsif($p->{rate} =~ /(\d+)x(\d+)/) {
	    $p->{_rate} = 15 + $1 + 12*$2;
	} else {
	    $p->{_rate} = $p->{rate};
	}
    }

    if(exists $p->{chanspec}) {
	if ($p->{chanspec} =~ m.(\d+)/80.) {
	    $p->{channel} = $1;
	    if ($p->{channel} >= 149) {
		$p->{_channel} = ($p->{channel}-149)/4 + 32;
	    } elsif ($p->{channel} >= 100) {
		$p->{_channel} = ($p->{channel}-100)/4 + 20;
	    } elsif ($p->{channel} >= 36) {
		$p->{_channel} = ($p->{channel}-36)/4 + 12;
	    }
	    $p->{bw} = 80;
	    delete $p->{chanspec};
	} elsif ($p->{chanspec} =~ /(\d+)[ab]?(?:\/(\d+))?([ul]?)/) {
	    $p->{channel} = $1;
	    if ($p->{channel} >= 149) {
		$p->{_channel} = ($p->{channel}-149)/4 + 32;
	    } elsif ($p->{channel} >= 100) {
		$p->{_channel} = ($p->{channel}-100)/4 + 20;
	    } elsif ($p->{channel} >= 36) {
		$p->{_channel} = ($p->{channel}-36)/4 + 12;
	    }
	    if (defined $2 && $2 ne "") {
		$p->{bw} = $2;
	    } elsif ($3 eq "") {
		$p->{bw} = 20;
	    } elsif ($p->{channel} >= 36 || !defined($2)) {
		$p->{bw} = 40;
	    } else {
		$p->{bw} = "40$2";
	    }
	    delete $p->{chanspec};
	}
    }

    if(exists $p->{wsec}) {
	my @sec;
	@sec = ("WEP") if $p->{wsec} & 1;
	@sec = ("TKIP") if $p->{wsec} & 2;
	@sec = ("AES") if $p->{wsec} & 4;
	push(@sec, "Software") if $p->{wsec} & 8;
	push(@sec, 'FIPS') if $p->{wsec} & 0x80;
	push(@sec, '') unless @sec;
	$p->{wsec} = join('|', @sec);
    }

    # wlver contains a list of <time>:<version> pairs.  Use <time> for
    # the shadow var so we can plot the timeline.  Multiple versions
    # will be turned into multiple wlver<n> variables.
    # This still needs work for AP versus STA versions.
    if (exists $p->{wlver}) {
	my $i=1;
	foreach (split(' ',$p->{wlver})) {
	    ($p->{"_wlver$i"},$p->{"wlver$i"}) = split(':', $_, 2);
	    $i++;
	}
	delete $p->{wlver};
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

    foreach my $k (qw(window maxsocram)) {
	if(exists $p->{$k}) {
	    if($p->{$k} =~ /(\d+)k/i) {
		$p->{"_$k"} = $1 * 1024;
	    } elsif($p->{$k} =~ /(\d+)m/i) {
		$p->{"_$k"} = $1 * 1024 * 1024;
	    } else {
		$p->{"_$k"} = $p->{$k};
	    }
	}
    }

    if(exists $p->{date}) {
	my ($y, $m, $d, $h);
	if (($y, $m, $d, $h) = 
	    $p->{date} =~ /(\d{4})\.(\d+)\.(\d+)(?:\.(\d+))/) {
	    if (!defined $h) {
		$h = 0;
	    }
	    $m--;
	    $p->{_date} = timelocal(0, 0, $h, $d, $m, $y);
	}
    }
    if (exists $p->{bpdiv}) {
	$p->{_bpdiv} = int(0.5 + 880 / $p->{bpdiv});
	$p->{bpdiv} = "$p->{bpdiv}/$p->{_bpdiv}"
    }
    if (exists $p->{attn} && param('Range')) {
	$p->{_attn} = exp(((32.6 + $p->{attn})/20 - log(2.4)/log(10))*log(10));
    }

    if (exists $p->{l}) {
	my ($l);
	if (($l) = $p->{l} =~ /(\d+)k/) {
	    $p->{_l} = $l * 1024;
	}
    }

    if (exists $p->{ack_ratio} && param('X-Axis') eq 'ack_ratio' &&
	param('log x') && $p->{ack_ratio} == 0) {
	$p->{_ack_ratio} = 0.5;
    }

    if (exists $p->{b}) {
	if ($p->{b} =~ /(\d+)G/i) {
	    $p->{_b} = $1 * 1000000000;
	} elsif ($p->{b} =~ /(\d+)M/i) {
	    $p->{_b} = $1 * 1000000;
	} elsif ($p->{b} =~ /(\d+)K/i) {
	    $p->{_b} = $1 * 1000;
	}
    }

    if (exists $p->{tlob}) {
	if ($p->{tlob} =~ /(\d+)K/i) {
	    $p->{_tlob} = $1 * 1000;
	} else {
	    $p->{_tlob} = $p->{tlob}
	}
    }
}

if(param('image') || param('pngfile')) {

    $|=1; # Make sure header gets out first
    
    # Gnuplot version should have been set up by the caller
    if(!defined(param('GnuplotVersion'))) {
	warn "GnuplotVersion unset.  Querying directly.";
	    if (`$gnuplot --version` =~ /gnuplot (\d+.\d+)/) {
		param('GnuplotVersion'=>$1);
	}
    }
    if(param('Gnuplot Source')) {
	print header(-type=>'text/plain',
		     -content_disposition=>'filename="GnuplotSrc.log"');
	print "# Gnuplot Source\n";
    } elsif(param('Gnuplot Output')) {
	print header(-type=>'text/plain',
		     -name=>'GnuplotOut.log');
	print "# Gnuplot Output\n";
	open(STDOUT, "|$gnuplot >/dev/null")
	    or die "Can't run $gnuplot: $!\n";
	print qq(show version\n);
    } else {
	if(param('Use SVG')) {
	    print header(-type=>'image/svg+xml',
			 -Refresh=>'10');
	    } elsif(param('pngfile') eq "") {
		# No header, dump to file
		print header(-type=>'image/png',
			     -Refresh=>'10');
	    }
	open(STDOUT, "|$gnuplot 2>/dev/null")
	    or die "Can't run $gnuplot: $!\n";
    }
    if(param('Use SVG')) {
	if (param('Size') && param('Size') =~ /^(\d+)x(\d+)$/) {
	    print qq(set terminal svg size $1,$2 fsize 9 enhanced\n);
	} else {
	    print qq(set terminal svg dynamic fsize 9 enhanced\n);
	}
    } elsif(param('GnuplotVersion') > 4.0 &&
	    param('Size') && param('Size') =~ /^(\d+)x(\d+)$/) {
	# The best way to set png plot size on Gnuplot 4.2 and
	# above is in the terminal declaration, but this doesn't
	# work on 4.0.
	print "set terminal png size $1,$2 medium\n";
    } else {
	print qq(set terminal png\n);
    }
    if(param('pngfile') ne "") {
	print qq(set output ").(param('pngfile')).qq("\n);
    }

    my %hide;
    foreach my $name (param()) {
	if ($name =~ /^Hide_(.*)/) {
	    map($hide{$1}{$_} = 1, param($name));
	}
    }
    my $ymax = param('Y-Max') || "";
    my $ymin = param('Y-Min') || "";
    my $sort;
    if (!defined(param('X-Axis'))) {
	param('X-Axis'=>'time');
    }
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

    
    if (!defined(param('Hist')) || param('Hist') !~ /^\d*/) {
	param('Hist'=>0);
    }

    my @entries;
    foreach my $file (@files) {
	if ($file =~ /.gz$/) {
	    $file = "zcat $file|"
	}
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
		   m!(\d\d:\d\d:\d\d)(?:\.\d+)?\s+XPASS (?:UTF|::)?(\S+) (.*):\s+([-\d.]+)(?: Mbit/sec)?$!) {
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
	next if $ymax ne "" && $tput > $ymax;
	next if $ymin ne "" && $tput < $ymin;
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
    my $autobold = 0;
    foreach my $k (sort keys %values) {
	next if grep($_ eq $k, (param('Ignore')));

	my $n = keys %{$values{$k}};
	my $v = join('',keys %{$values{$k}});
	if ($n == 1 && $v ne '') {
	    push(@titlekeys, "$k=$v");
	} elsif ($n > 1) {
	    push(@keys, $k);
	    if ($k eq "rate") {
		$autobold = 1;
	    }
	}
    }
    
    my $e = 0.0005; # errorbars
    my $maxhist = 0;
    my $maxhistat = 0;
    my $needy1=0;
    my $ylabel;
    my $y2label;
    my $ls;
    if(param('GnuplotVersion') > 4.0) {
	$ls = "ls";
    } else {
	$ls = "lt";
    }
    if(defined(param('Title')) && param('Title') eq "Memory") {
	$y2label = 'Memory (bytes)';
    } else {
	$y2label = 'Throughput (Mbit/sec)';
    }

    foreach my $s (sort keys %stream) {
	print "# $s\n";
	my %p = split($;, $s);
	my $average = "";
	my $lw = "";
	my $title = join('', map("$_=".(exists $p{$_}?$p{$_}:'').' ', @keys));
#	$title =~ s/(.{80}).*/$1/;
	my $a = 'x1y2';
	if ((exists $p{wlrxdrops} && $p{wlrxdrops})) {
	    $a = 'x1y1';
	    $needy1=1;
	    $ylabel = "wlrxdrops";
	}
	if (exists $p{data} && $p{data} ne "") {
	    $a = 'x1y1';
	    $needy1=1;
	    $ylabel = $p{data};
	}
	if ($autobold && $title=~/rate=\(auto\)/) {
	    $lw = " lw 3"
	}
	push(@plots, qq('-' using 1:2 axes $a title "$title" with lp $ls $i$lw));
	my $needdummy = 1;
	my %hist;
	my $histstart = "";
	my $histend = "";
	foreach my $c (sort $sort keys %{$stream{$s}}) {
	    foreach my $p (@{$stream{$s}->{$c}}) {
		if ($needdummy) {
		    $data .= "$c $p\ne\n"; # Dummy for title
		    $needdummy = 0;
		}
		$data .= "$c $p\n";
		if (param('X-Axis') eq 'time') {
		    if (param('Hist')) {
			my $bucket = param('Hist')*int(0.5+$p/(param('Hist')));
			my $mh = $hist{$bucket} += 1;
			if ($histstart eq "" || $histstart > $bucket) {
			    $histstart = $bucket;
			}
			if ($histend eq "" || $histend < $bucket) {
			    $histend = $bucket;
			}
			if ($mh > $maxhist) {
			    $maxhist = $mh;
			    $maxhistat = $bucket;
			}
		    }
		} else {
		    $xtics{$c} = $c unless exists $xtics{$c};
		}
	    }
	    $average.= "$c ".(($sum{$s}->{$c})/($num{$s}->{$c}))."\n";
	}
	push(@plots, qq('-' using 1:2 axes $a notitle with p $ls $i));
	push(@plots, qq('-' using 1:2 axes $a notitle with l $ls $i$lw));
	
	$data .= "e\n${average}e\n";

	if ($histstart ne $histend) {
	    my $hist;
	    my $x0 = 0;
	    my $x1 = 0;
	    my $y0 = 0;
	    my $y1 = 0;
	    my $h;
	    for($h=$histstart; $h < $histend + param('Hist'); $h+=param('Hist')) {
		$x0 = $h - param('Hist')/2;
		$x1 = $h + param('Hist')/2;
		$y1 = (exists $hist{$h})?-$hist{$h}:0;
		$hist .= "$y0 $x0\n";
		$hist .= "$y1 $x0\n";
		$hist .= "$y1 $x1\n";
		$y0 = $y1;
	    }
	    $x0 = $h - param('Hist')/2;
	    $hist .= "0 $x0\n";
	    if ($hist) {
		push(@plots, qq('-' using 1:2 axes x2y1 notitle with l lt $i));
		$data .= "${hist}e\n";
	    }
	}
	
	$i++;
    }
    if ($maxhist) {
	push(@plots, qq('-' using 1:2 axes x2y1 notitle with p lt 0));
	$data .= (-$maxhist*3)." $maxhistat\ne\n";
    }

    print "set title \"".join("\\n", @titlekeys)."\"\n";

    print qq(set y2label ").$y2label.qq("\n);
    print qq(set y2tics mirror\n);
    if ($needy1) {
	print qq(set ylabel ").$ylabel.qq("\n);
	print qq(set ytics\n);
    }
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
    if (param('log x')) {
	print qq(set logscale x\n);
    }
    if (param('log y')) {
	print qq(set logscale y\n);
    }
    if (param('nokey')) {
	print qq(set nokey\n);
    } else {
	print qq(set key below\n);
    }
    # Gnuplot 4.0 can't set png plot size in the terminal declaration,
    # so set it here.  Use the definition of NaN as a proxy for the
    # version check.
    if(param('GnuplotVersion') <= 4.0 &&
       param('Size') && param('Size') =~ /^(\d+)x(\d+)$/) {
	print "set size ".($1/640).",".(($2+2*(@plots+@titlekeys))/480)."\n";
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
    close(STDOUT);
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
	if ($file =~ /.gz$/) {
	    $file = "zcat $file|"
	}
	my $ap;
	my $parms;
	if(open(FILE, $file)) {
	    $files++;
	    while(<FILE>) {
		if(($ap, $parms) = 
		   m!XPASS (?:UTF|::)?(\S+) (.*):\s+[-\d.]+(?: Mbit/sec)?$!) {
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
		 td({-colspan=>2},
		    textarea(-name=>'FileList',
			     -default=>$DefaultFileList,
			     -rows=>10,
			     -columns=>80,
			     ))).
	      Tr(th('X-Axis').
		 td(radio_group
		    (-name=>'X-Axis',
		     -values=>[sort keys %Xvars],
		     )).
		 td(radio_group
		    (-name=>'Title',
		     -values=>[qw(Throughput Memory)]).
		    checkbox(-name=>'WrapMCS'))).
	      Tr(th('Y Limits').
		 td({-colspan=>2},
		    ' Min: '.
		    textfield(-name=>'Y-Min', -size=>5).
		    ' Max: '.
		    textfield(-name=>'Y-Max', -size=>5).
		    ' Start Hour: '.
		    popup_menu(-name=>'Start Hour', -values=>[0..23]).
		    ' Auto rate: '.
		    textfield(-name=>'AutoRate', -size=>2).
		    checkbox(-name=>'log x').
		    checkbox(-name=>'log y').
		    ' Histogram Size: '.
		    textfield(-name=>'Hist', -size=>5).
		    checkbox(-name=>'Range')))
	      );

    my $paramhidetable = "";
    if (keys %parmlist) {
	$paramhidetable = 
	    table({border=>1, summary=>'Display Panel'},
		  Tr([td([Checkbox_group
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
					  '800x600',
					  '1024x480',
					  '1024x768',
					  '1280x800',
					  '1280x1024',
					  '1400x1200',
					  'dynamic',
					  ])),
		  td(submit('Redraw')),
		  td(checkbox(-name=>'Use SVG')),
		 ]));
    my $numortext = sub {
	if($a =~ /^\d+$/ && $b =~ /^\d+$/) {
	    $a <=> $b;
	} else {
	    $a cmp $b;
	}
    };

    my $recordhidetable = "";
    if (keys %parmlist) {
	$recordhidetable =
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
    }

    my $othertable = 
	table({border=>0, summary=>'Other Options'},
	      Tr([td([a({-href=>$o->self_url,
			 -target=>'Output'}, 'Gnuplot Output'),
		      a({-href=>self_url}, 'Bookmark'),
		      defaults,
		      a({-href=>$g->self_url,
			 -target=>'Source'}, 'Gnuplot Source'),
		      ]),
		  ]));
    
    print start_form;

    if(!defined(param('GnuplotVersion'))) {
	if (`$gnuplot --version` =~ /gnuplot (\d+.\d+)/) {
	    param('GnuplotVersion'=>$1);
	}
    }
    print hidden(-name=>'GnuplotVersion', -default=>param('GnuplotVersion'));
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
    return a({-href=>$n->self_url},img({src=>$m->self_url,
					alt=>"@_"}));
}
