while (<>)
	{
	if (/^\./)
		{
		if (/(\.define)/)
			{
			print "$_";
			}
		else
			{
			if (/(\.block)/)
				{
				$nWords = 2;
				}
			elsif (/(\.build)/)
				{
				$nWords = 2;
				}
			elsif (/(\.endblock)/)
				{
				$nWords = 1;
				}
			elsif (/(\.if)/)
				{
				$nWords = 4;
				}
			elsif (/(\.else)/)
				{
				$nWords = 1;
				}
			elsif (/(\.endif)/)
				{
				$nWords = 1;
				}
			elsif (/(\.image)/)
				{
				$nWords = 2;
				}
			elsif (/(\.\-+)/)
				{
				$nWords = 1;
				}
			$pre = "";
			while (/(\S+\s*)/ && ($nWords > 0))
				{
				$nWords--;
				$pre = $pre.$`.$1;
				$_ = $';
				if ($nWords == 0)
					{
					print "$pre\n";
					print "$'";
					}
				}
			}
		}
	else
		{
		print "$_";
		}
	}
if (0)
{
$strlen=0;
$directive_active = 0;

while (<>)
	{
	for (split(/\s+/))
		{
		if (/<[hH]/)
			{
			print "\n";
			$nWords = 10000;
			}
		if (/<\/[hH][1-6]>/)
			{
			$nWords = 1;
			}
		if (/(\.define)/)
			{
			print "\n";
			$nWords = 10000;
			$directive_active = 1;
			}
		elsif (/(\.block)/)
			{
			print "\n";
			$nWords = 2;
			$directive_active = 1;
			}
		elsif (/(\.build)/)
			{
			print "\n";
			$nWords = 2;
			$directive_active = 1;
			}
		elsif (/(\.endblock)/)
			{
			print "\n";
			$nWords = 1;
			$directive_active = 1;
			}
		elsif (/(\.if)/)
			{
			print "\n";
			$nWords = 4;
			$directive_active = 1;
			}
		elsif (/(\.else)/)
			{
			print "\n";
			$nWords = 1;
			$directive_active = 1;
			}
		elsif (/(\.endif)/)
			{
			print "\n";
			$nWords = 1;
			$directive_active = 1;
			}
		elsif (/(\.image)/)
			{
			print "\n";
			$nWords = 2;
			$directive_active = 1;
			}
		elsif (/(\.\-+)/)
			{
			print "\n";
			$nWords = 1;
			$directive_active = 1;
			}
		if (($nWords > 0) && $directive_active)

			{
			s/\n//g;
			s/<[^>]*>//g;
			}
		if ($nWords > 0)
			{
			print "$_ ";
			$nWords--;
			if ($nWords == 0)
				{
				$directive_active = 0;
				print "\n    ";
				$strlen=4;
				}
			}
		else
			{
			if (/<[pP]/ || 
					/<\/*[tT][aA][bB][lL][eE]/ || 
					/<[bB][rR]/ || 
					/<[tT][dD]/ || 
					/<[tT][rR]/ || 
					/<\/*[uUoO][lL]/ || 
					($strlen > 72))
				{
				print "\n    ";
				$strlen=4;
				}
			elsif (/<[lL][iI]/)
				{
				print "\n      ";
				$strlen=6
				}
			print "$_ ";
			$strlen+=length()+1;
			}
		}
	
	}
}
