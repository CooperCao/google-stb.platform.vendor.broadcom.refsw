$split_first_word=0;
$process_define=0;
$process_header=0;
$partial_if=0;

while (<>)
	{
	s/^\xEF\xBB\xBF//;
	if ($split_first_word)
		{
		s/(^\s*\S*)/ $1\n/g;
		$split_first_word=0;
		}
	if ($process_header)
		{
		if (/<\/[hH][1-6][^>]*>/)
			{
			$process_header=0;
			}
		else
			{
			chop($_);
			}
		}
	if ($partial_if)
		{
		print " ";
		$partial_if=0;
		s/^([^<]*)</$1\n</g;
		}
	if ($process_define)
		{
		if (/^\./)
			{
			$process_define=0;
			print "\n";
			}
		}
	s/\.if\$/.if \$/g;
	s/(\.if\s+\S+\s+\S+\s+\S+)(.*)/$1\n$2/g;
	s/(\.block\s+[0-9A-Za-z_]+)([^0-9A-Za-z_])/$1\n$2/g;
	s/(\.image\s+[\"0-9A-Za-z_\.]+)([^0-9A-Za-z_\.\"])/$1\n$2/g;
	s/(\.build\s+[0-9A-Za-z_]+)([^0-9A-Za-z_])/$1\n$2/g;
	s/(src=\").*\/([^\/]*\")/\1\2/g;
	s/(href=\").*\/([^\/]*\")/\1\2/g;
	if ((/\.if/) && !(/\.if\s+\S+\s+\S+\s+\S+/))
		{
		$partial_if=1;
		chop($_);
		}
	if ((/\.image/) && !(/\.gif/))
		{
		chop($_);
		}
	if ((/\.define/) || (process_define))
		{
		s/<[^>]*>//g;
		s/^[^>]*>//g;
		s/<[^>]*$//g;
		s/\&quot;/"/g;
		$process_define=1;
		chop($_);
		}
	if (/^\./)
		{
		s/\&quot;/"/g;
		}

	if ((/^\./) || (/\n\./))
		{
		if ((/\.block\s*$/) || (/\.build\s*$/))
			{
			$split_first_word=1;
			chop($_);
			}
		}
	if ((/<[hH][1-6][^>]*>/) && !(/<\/[hH][1-6][^>]*>/))
		{
		$process_header=1;
		chop($_);
		}
	print "$_";
	}
