#perl

sub search_glossary
{
local($language, $text) = @_;
local($glossary);
$glossary = "/h/ogre/projects/i18n/102104/$language/glossary.idx";
$ret = "";
if (open (GLOSSARY, $glossary))
	{
	seek GLOSSARY,0,2;
	$min = 0;
	$max = tell(GLOSSARY);

	while ($max != $min)
		{
		seek GLOSSARY,(int($max + $min)/2),0;
		$pos = tell (GLOSSARY);
		$_ = <GLOSSARY>;
		$_ = <GLOSSARY>;
		/^([^\|]*)\|/;
		#print "$min, $max, $1\n";
		if ($1 gt $text)
			{
			last if ($max == $pos);
			$max = $pos;
			}
		elsif ($1 lt $text)
			{
			last if ($min == $pos);
			$min = $pos;
			}
		elsif ($1 eq $text)
			{
			s/^(.*)\s*$/$1/;
			/^[^\|]*\|([^|]*).*$/;
	#		print STDERR "$1 => \n";
	#		print "$_\n";
			$ret = $1;
			last;
			}
		}
	close GLOSSARY;
	}
return $ret;
}

{

$untranslatable_id{"STR_PRODUCT_NAME"} = 1;
$untranslatable_id{"STR_CPL_SHORTCUT_ITEM_NAME"} = 1;
$untranslatable_id{"IDS_1117"} = 1;
$untranslatable_id{"IDS_1118"} = 1;
$untranslatable_id{"IDS_1120"} = 1;
$untranslatable_id{"IDS_1121"} = 1;
$untranslatable_id{"IDS_WEP"} = 1;
$untranslatable_id{"STR_TLS"} = 1;
$untranslatable_id{"STR_MD5"} = 1;
$untranslatable_id{"STR_LEAP"} = 1;
$untranslatable_id{"STR_TTLS_PEAP"} = 1;
$untranslatable_id{"STR_EAP_FAST"} = 1;
$untranslatable_id{"STR_TTLS"} = 1;
$untranslatable_id{"STR_PEAP"} = 1;
$untranslatable_id{"STR_GTC"} = 1;
$untranslatable_id{"STR_PAP"} = 1;
$untranslatable_id{"STR_CHAP"} = 1;
$untranslatable_id{"STR_MSCHAP"} = 1;
$untranslatable_id{"STR_MSCHAPV2"} = 1;
$untranslatable_id{"STR_WME"} = 1;
$untranslatable_id{"STR_Afterburner"} = 1;
$untranslatable_id{"STR_V_BCM"} = 1;
$untranslatable_id{"STR_54G_LRS"} = 1;
$untranslatable_id{"STR_54gLRS"} = 1;
$untranslatable_id{"IDS_1115"} = 1;
$untranslatable_id{"IDS_1116"} = 1;
$untranslatable_id{"STR_CPL_SHORTCUT_FOLDER_NAME"} = 1;
$untranslatable_id{"STR_CPL_SHORTCUT_FOLDER_NAME"} = 1;
$untranslatable_id{"STR_SSID"} = 1;
$untranslatable_id{"STR_GC79_DeviceDesc"} = 1;
$untranslatable_id{"STR_GC89_DeviceDesc"} = 1;
$untranslatable_id{"STR_Region1"} = 1;
$untranslatable_id{"STR_Region2"} = 1;
$untranslatable_id{"STR_Region3"} = 1;
$untranslatable_id{"STR_Region4"} = 1;
$untranslatable_id{"STR_USA"} = 1;
$untranslatable_id{"STR_Japan"} = 1;
$untranslatable_id{"STR_Thailand"} = 1;
$untranslatable_id{"STR_USA_TAIWAN"} = 1;

if (1)
{
$untranslatable_id{"STR_SNR_DB"} = 1;
$untranslatable_id{"STR_SNR"} = 1;
$untranslatable_id{"STR_802_11_A"} = 1;
$untranslatable_id{"STR_802_11_B"} = 1;
$untranslatable_id{"STR_802_11_G"} = 1;
$untranslatable_id{"STR_Aux"} = 1;
$untranslatable_id{"IDS_ADHOC"} = 1;
$untranslatable_id{"STR_AP_BAND"} = 1;
$untranslatable_id{"STR_AP_CH"} = 1;
$untranslatable_id{"STR_AP_MAC"} = 1;
$untranslatable_id{"STR_DELL_TRUE_MOBILE_1180_ADAPTER"} = 1;
$untranslatable_id{"STR_54G_AUTO"} = 1;
$untranslatable_id{"STR_AdHoc"} = 1;
$untranslatable_id{"STR_54gAuto"} = 1;
$untranslatable_id{"STR_SIGNAL_DBM"} = 1;
$untranslatable_id{"IDS_uCodeMem"} = 1;
$untranslatable_id{"STR_WZC_Managed_Ethernet"} = 1;
$untranslatable_id{"STR_Msft"} = 1;
$untranslatable_id{"STR_Israel"} = 1;
$untranslatable_id{"STR_Israel"} = 1;
}


#print search_glossary("french", "Use smart card");
opendir (ENGLISH, "english") || die "can't open english directory: $!";
@files = readdir(ENGLISH);
foreach $file (@files)
	{
	$_ = $file;
	if (/bcm.txt$/ || /hp.txt$/ || /dell.txt$/ || /se.txt$/ || /option.txt$/)
		{
		$oem_file=1;
		}
	else
		{
		$oem_file=0;
		}
	if (/\.txt$/)
		{
		$ifile = "english/$file";
#		print "$ifile\n";
		open (FILE, $ifile) || die "can't open $ifile: $!";
		while (<FILE>)
			{
			$foo = $_;
			$foo =~ s/\/\/.*//;
			$foo =~ s/\"\"/double-double-quote/g;
			if ($foo =~ /^(\S+)\s+L\"([^"]*)\"/) #"
				{
				$id = $1;
				$foo = $2;
				$foo =~ s/double-double-quote/\"\"/g;
				if ($oem_file)
					{
					$id = $file._.$id;
					}
				$english_str{$id} = $foo;
				}
			}
		close FILE;
		}
	}
closedir ENGLISH;

opendir (LOCALE, ".") || die "can't open . directory: $!";
@dirs = readdir(LOCALE);
#push @dirs,"russian";
foreach $dir (@dirs)
	{
	next if (! -d $dir);
#	next if (! -f "$dir/bcm.txt");
	next if ($dir =~ /english/);
#	print "$dir\n";
	undef %brcm_glossary;
	foreach $file (@files)
		{
		if (/bcm.txt$/ || /hp.txt$/ || /dell.txt$/ || /se.txt$/ || /option.txt$/)
			{
			$oem_file=1;
			}
		else
			{
			$oem_file=0;
			}
		if (/\.txt$/ && !/LocationStrs.txt/)
			{
			$ifile = "$dir/$file";
			if (open(FILE, $ifile))
				{
				print "$ifile\n";
				while (<FILE>)
					{
					$foo = $_;
					$foo =~ s/\/\/.*//;
					$foo =~ s/\"\"/double-double-quote/g;
					if ($foo =~ /^(\S+)\s+L\"([^"]*)\"/)	#"
						{
						$id = $1;
						$foo = $2;
						$foo =~ s/double-double-quote/\"\"/g;
						if ($oem_file)
							{
							$id = $file._.$id;
							}
						if ($foo ne $english_str{$id})
							{
							$brcm_glossary{$id} = $foo;
							$brcm_glossary{$english_str{$id}} = $foo;
							}
						}
					}
				close FILE;
				}
			}
		}
	open(OFILE1, ">tmp/$dir.translated-strings.txt");
	open(OFILE2, ">tmp/$dir.untranslated-strings.txt");
	open(OFILE3, ">tmp/$dir.borrowed-strings.txt");
	undef %processed_strings;
	foreach $file (@files)
		{
		$_ = $file;
		if (/bcm.txt$/ || /hp.txt$/ || /dell.txt$/ || /se.txt$/ || /option.txt$/)
			{
			$oem_file=1;
			}
		else
			{
			$oem_file=0;
			}
		if (/\.txt$/ && !/LocationStrs.txt/)
			{
			$ifile = "$dir/$file";
			if (open(FILE, $ifile))
				{
				print "$ifile\n";
				while (<FILE>)
					{
					$foo = $_;
					$foo =~ s/\/\/.*//;
					$foo =~ s/\"\"/double-double-quote/g;
					if ($foo =~ /^(\S+)\s+L\"([^"]*)\"/)	#"
						{
						$id = $1;
						$foo = $2;
						$foo =~ s/double-double-quote/\"\"/g;
						$translated_str = $foo;
						if ($oem_file)
							{
							$id = $file._.$id;
							}
						next if ($processed_strings{$id} eq $translated_str);
#print "$id => $english_str{$id} => $translated_str\n";
						$processed_strings{$id} = $translated_str;
						if (1)
							{
							if ($id =~ /_MBPS/)
								{
								$untranslatable_id{$id} = 1;
								}
							if ($id =~ /_SHORTHAND/)
								{
								$untranslatable_id{$id} = 1;
								}
							if ($id =~ /_SHORTHAND/)
								{
								$untranslatable_id{$id} = 1;
								}
							if ($id =~ /STR_V_BCM/)
								{
								$untranslatable_id{$id} = 1;
								}
							}
						
						if ($untranslatable_id{$id} || ($translated_str ne $english_str{$id}))
							{
							print OFILE1 "$id\tL\"$translated_str\"\t// $english_str{$id}\n";
							}
						else
							{
							if ($brcm_glossary{$id} ne "")
								{
								print OFILE3 "$id\tL\"$brcm_glossary{$id}\"\t// $english_str{$id}\n";
								}
							elsif ($brcm_glossary{$translated_str} ne "")
								{
								print OFILE3 "$id\tL\"$brcm_glossary{$translated_str}\"\t// $translated_str\n";
								}
							else
								{
								if (1)
									{
									print OFILE2 "$id\tL\"$translated_str\"\t// $english_str{$id}\n";
									}
								else
									{
									$a = search_glossary($dir, $translated_str);
									if ($a eq $translated_str)
										{
										print OFILE1 "$id\tL\"$translated_str\"\t// $english_str{$id}\n";
										}
									elsif ($a ne "")
										{
										print OFILE3 "$id\tL\"$a\"\t// $english_str{$id}\n";
										}
									else
										{
										if (1 && $translated_str =~ /^([^\/\\]+)([\/\\]+)([^\/\\]+)$/)
											{
											if ((($a = search_glossary($dir, $1)) ne "") && (($b = search_glossary($dir, $3)) ne ""))
												{
												$c = $a.$2.$b;
												print OFILE2 "$id\tL\"$c\"\t// $english_str{$id}\n";
												}
											else
												{
												print OFILE2 "$id\tL\"$translated_str\"\t// $english_str{$id}\n";
												}
											}
										else
											{
											print OFILE2 "$id\tL\"$translated_str\"\t// $english_str{$id}\n";
											}
										}
									}
								}
							}
						}
					}
				close FILE;
				}
			}
		}
	close OFILE1;
	close OFILE2;
	close OFILE3;
	}
closedir LOCALE;
}
