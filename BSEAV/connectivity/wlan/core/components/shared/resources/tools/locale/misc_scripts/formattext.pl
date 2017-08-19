
while (<>)
	{
	s/\&quot;/"/g;
	s/\&nbsp;/ /g;
	s/^\xEF\xBB\xBF//;
	s/^\s*//g;
	for (split(/\s+/))
		{
		print "$_ ";
		}
	}
