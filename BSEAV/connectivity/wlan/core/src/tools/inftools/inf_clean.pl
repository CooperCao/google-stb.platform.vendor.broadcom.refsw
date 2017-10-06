#perl
# This script read through std-in and filters out lines with unresolved %var%
# Then it resolves %var% from the same std-in to generate var=<some-var-value>
# output to stdout. Stdin is typically comprised of multiple <oem>.txt files
# and .ini or .inf templates. Values are in <oem>.txt and variable references
# are in .ini or .inf files
{
	while (<>)
	{
		push @lines, $_;
		s/;.*$//;
		while (/%([^%]*)%/)
		{
			# %var% found, store it
			$strings{$1} = 1;
			$_ = $';
		}
	}
	for $line (@lines)
	{
		$_ = $line;
		# Look for start of inf or ini [section] block
		if (/^\[([^\]]*)\]/)
		{
			# If [strings] section is found set checked=1
			if (/strings/)
			{
				$check = 1;
			}
			else
			{
				$check = 0;
			}
		}
		# Within [strings] body search for var=<value> entries
		# which are previously stored in while loop and skip others
		if ($check && /^(\S*)\s*=/)
		{
			if (!$strings{$1})
			{
				 next;
			}
		}
		print $_;
	}
	
}
