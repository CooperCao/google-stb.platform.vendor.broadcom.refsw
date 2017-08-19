$rc = 0;
opendir (LOCALEDIR, $ARGV[0]) || die "can't open locale directory $ARGV[0]: $!";
@files = readdir(LOCALEDIR);
foreach $file (@files)
	{
	$_ = $file;
#	if (/\.txt$/ && !/bcm.txt$/ && !/hp.txt$/ && !/dell.txt$/ && !/se.txt$/ && !/option.txt$/)
	if (/\.txt$/)
		{
		$ifile = "$ARGV[0]/$file";
		open (FILE, $ifile) || die "can't open $ifile: $!";
		$lineno = 1;
		while (<FILE>)
			{
			$iline = $_;
			$foo = $_;
			$foo =~ s/\/\/.*//;
			$foo =~ s/\"\"/double-double-quote/g;
			if ($foo =~ /^(\S+)\s+L\"([^"]*)\"/) #"
				{
				$id = $1;
				$foo = $2;
				$foo =~ s/double-double-quote/\"\"/g;
				if ($str{$id} ne "" && $str{$id} ne $foo && ! ($id =~ /STR_OEM/) && ! ($id =~ /STR_BCM/) && ! ($id =~ /STR_V_BCM/))
					{
					$rc = -1;
					printf("ERROR: mismatched ID definitions\n");
					printf("$defile{$id}:$delineno{$id}:$deline{$id}");
					printf("$ifile:$lineno:$iline\n");
					}
				else
					{
					$str{$id} = $foo;
					$defile{$id} = $ifile;
					$deline{$id} = $iline;
					$delineno{$id} = $lineno;
					}
				}
			$lineno++;
			}
		close FILE;
		}
	}
closedir LOCALEDIR;
exit($rc);
