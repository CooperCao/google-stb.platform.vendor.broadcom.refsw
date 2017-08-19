while (<>)
	{
	if (!/^\s*$/)
		{
		if (/^\./)
			{
			print $_;
			}
		else
			{
			print "    $_";
			}
		}
	}
