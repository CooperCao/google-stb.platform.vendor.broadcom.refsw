$lineno=1;
while (<>)
	{
	if (!(/^\./))
		{
		s/^[^<>]*//;
		s/[^<>]*$//;
		s/>[^<>]*</></g;
		s/></>\n</g;
#		s/>/>\n\[$lineno\]/g;
		}
	if (!/^\s*$/)
		{
		print "$_\n";
		}
	$lineno++;
	}
