#perl
{
	open( SRCFILE, $ARGV[0]) || die "Unable to open $ARGV[0]\n";
	open( MRGFILE, $ARGV[1]) || die "Unable to open $ARGV[1]\n";
	while (<MRGFILE>)
		{
		s/\xef\xbb\xbf//g;
		# ensure that quotes are properly escaped
		if (/^([^\"]*L\")(.*)(\"[^\"]*\/\/.*$)/)
			{
			$a = $1;
			$b = $2;
			$c = $3;

			$b =~ s/""/quote-quote/g;
			$b =~ s/"/""/g;
			$b =~ s/quote-quote/""/g;

			$_= $a.$b.$c."\n";
			}

		$line = $_;
		s/\/\/.*$//;
		s/"[^"]*$//;

		if (/^(\S*)\s*L\"(.*)$/)
			{
#print STDERR  "Read: $1\n";

			$ss{$1} = $line;
			$tt{$1} = $2;
			$nn{$1} = 0;
			}
		}
	while (<SRCFILE>)
		{
		$line = $_;
		s/\xef\xbb\xbf//g;
		s/\/\/.*$//;
		s/"[^"]*$//;
		if (/^(\S*)\s*L\"(.*)$/)
			{
			if (($ss{$1} ne "") && ($tt{$1} ne $2))
				{
				print $ss{$1};
				$nn{$1} = 1;
				}
			else
				{
				print $line;
				}
			}
		else
			{
			print $line;
			}
		}
	foreach $key (sort keys(%nn))
		{
		if ($nn{$key} != 0)
			{
			print STDERR  "        Merged: $key\n";
			}
		}
	
}
