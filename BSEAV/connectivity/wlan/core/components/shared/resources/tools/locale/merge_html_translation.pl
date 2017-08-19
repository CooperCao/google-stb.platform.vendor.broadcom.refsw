#perl
{
	if ($ARGV[0] =~ /exclude/)
		{
		$exclude = 1;
		}
	else
		{
		$exclude = 0;
		}
	open( SRCFILE, $ARGV[1]) || die "Unable to open $ARGV[0]\n";
	open( MRGFILE, $ARGV[2]) || die "Unable to open $ARGV[1]\n";
	while (<MRGFILE>)
		{
		if (/\.block\s*(\S*)/)
			{
			$block_name = $1; 
			if ($block{$block_name} ne "")
				{
				$block_name = "default";
				}
			}
		elsif (/\.endblock\s*(\S*)/)
			{
			$block_name = "default";
			}
		elsif (/\.define\s*(\S*)/)
			{
			$define{$1} = $_;
			}
		elsif ($block_name ne "default")
			{
			if (!/marked_block_/ && !/^\s*$/)
				{
				$block{$block_name} .= $_;
				}
			}
		}
	while (<SRCFILE>)
		{
		if (/\.block\s*(\S*)/)
			{

			if ($block{$1} ne "")
				{
				print $_;
				print $block{$1};
				$block_active = 1;
				}
			elsif ($exclude)
				{
				$block_active = 2;
				print STDERR "Block $1 untranslated\n";
				}
			else
				{
				print $_;
				}
			}
		elsif (/\.endblock\s*(\S*)/)
			{
			if ($block_active != 2)
				{
				print $_;
				}
			$block_active = 0;
			}
		elsif (/\.define\s*(\S*)/)
			{
			if ($define{$1} ne "")
				{
				print $define{$1};
				}
			elsif ($exclude)
				{
				print STDERR "Define $1 untranslated: $_";
				}
			else
				{
				print $_;
				}
			}
		elsif (!$block_active)
			{
			print $_;
			}
		}
}
