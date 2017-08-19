while(<>)
	{
	$len = length;
	if (/^\./ || /^\s*</)
		{
		print $_;
		}
	else
		{
		$pre = "    ";
		s/^\s*//g;
		while (/(\s+)/)
			{
			$len = length $pre.$`;
			if ($len > 80)
				{
				print "$pre\n";
				$pre = "    ";
				}
			$pre = $pre.$`.$1;
			$_ = $';
			}
		print $pre.$_;
		}

	}
