#perl

if( !defined( $ARGV[0] ) )
{
    die( "Need locale directory.\n" );
}

if( !defined( $ARGV[1] ) )
{
    die( "Need file name to check.\n" );
}

$localeDir = $ARGV[0];
$fileName = $ARGV[1];
$printWarnings = 0;
if( defined( $ARGV[2] ) && $ARGV[2] =~ /-[wW]/ )
{
    $printWarnings = 1;
}

$finalResult = 0;

$headerMap{ LANG_ARABIC		}{ SUBLANG_NEUTRAL	}=1256;
$headerMap{ LANG_BULGARIAN	}{ SUBLANG_NEUTRAL	}=1251;
$headerMap{ LANG_CHINESE	}{ SUBLANG_CHINESE_SIMPLIFIED	}=936;
$headerMap{ LANG_CHINESE	}{ SUBLANG_CHINESE_TRADITIONAL	}=950;
$headerMap{ LANG_CROATIAN	}{ SUBLANG_NEUTRAL	}=1250;
$headerMap{ LANG_CZECH		}{ SUBLANG_NEUTRAL	}=1250;
$headerMap{ LANG_DANISH		}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_DUTCH		}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_ENGLISH	}{ SUBLANG_ENGLISH_US	}=1252;
$headerMap{ LANG_ESTONIAN	}{ SUBLANG_NEUTRAL	}=1257;
$headerMap{ LANG_FINNISH	}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_FRENCH		}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_GERMAN		}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_GREEK		}{ SUBLANG_NEUTRAL	}=1253;
$headerMap{ LANG_HEBREW		}{ SUBLANG_NEUTRAL	}=1255;
$headerMap{ LANG_HUNGARIAN	}{ SUBLANG_NEUTRAL	}=1250;
$headerMap{ LANG_ITALIAN	}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_JAPANESE	}{ SUBLANG_NEUTRAL	}=932;
$headerMap{ LANG_KOREAN		}{ SUBLANG_NEUTRAL	}=949;
$headerMap{ LANG_LATVIAN	}{ SUBLANG_NEUTRAL	}=1257;
$headerMap{ LANG_LITHUANIAN	}{ SUBLANG_NEUTRAL	}=1257;
$headerMap{ LANG_NORWEGIAN	}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_POLISH		}{ SUBLANG_NEUTRAL	}=1250;
$headerMap{ LANG_PORTUGUESE	}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_PORTUGUESE	}{ SUBLANG_PORTUGUESE_BRAZILIAN }=1252;
$headerMap{ LANG_ROMANIAN	}{ SUBLANG_NEUTRAL	}=1250;
$headerMap{ LANG_RUSSIAN	}{ SUBLANG_NEUTRAL	}=1251;
$headerMap{ LANG_SLOVAK		}{ SUBLANG_NEUTRAL	}=1250;
$headerMap{ LANG_SLOVENIAN	}{ SUBLANG_NEUTRAL	}=1250;
$headerMap{ LANG_SPANISH	}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_SWEDISH	}{ SUBLANG_NEUTRAL	}=1252;
$headerMap{ LANG_THAI		}{ SUBLANG_NEUTRAL	}=874;
$headerMap{ LANG_TURKISH	}{ SUBLANG_NEUTRAL	}=1254;

$ignoreMap{ IDS_ADHOC	}=0;
$ignoreMap{ STR_AP_CH	}=0;
$ignoreMap{ STR_AP_MAC	}=0;
$ignoreMap{ STR_SNR	}=0;
$ignoreMap{ STR_54G_LRS	}=0;
$ignoreMap{ STR_COPYRIGHT	}=0;
$ignoreMap{ STR_OK	}=0;
$ignoreMap{ IDS_OK	}=0;
$ignoreMap{ STR_MBPS	}=0;
$ignoreMap{ STR_SNR_DB	}=0;
$ignoreMap{ STR_54G_AUTO	}=0;
$ignoreMap{ STR_PRODUCT_NAME	}=0;
$ignoreMap{ STR_CPL_SHORTCUT_FOLDER_NAME	}=0;
$ignoreMap{ STR_CPL_SHORTCUT_FOLDER_NAME	}=0;

sub ValidateHeader
{
    local *APPFILE = shift( @_ );
    my $lineNumber = shift( @_ );
    my $fullPath = shift( @_ );
    my $line;

    $$lineNumber++;
    $line = <APPFILE>;   
    if( $line !~ /^(\xEF\xBB\xBF)*LANGUAGE\s+(LANG_\w+),\s*(SUBLANG_\w+)\s*$/ )
    {
	print( STDERR "$fullPath($$lineNumber) : error : Invalid header.\n" );
	return -1;
    }
    $lang = $2; $sublang = $3;

    $$lineNumber++;
    $line = <APPFILE>;   
    if( $line !~ /^\#pragma\s+code_page\(\s*(\d+)\s*\)\s*$/ )
    {
	print( STDERR "$fullPath($$lineNumber) : error : Invalid header.\n" );
	return -1;
    }
    #print( "$lang $sublang $1 $headerMap{LANG_ENGLISH}{ SUBLANG_ENGLISH_US }\n" );
    if( $headerMap{ $lang }{ $sublang } != $1 )
    {
	print( STDERR "$fullPath($$lineNumber) : error : Invalid header.\n" );
	return -1;
    }

    $$lineNumber++;
    $line = <APPFILE>;   
    if( $line !~ /^STRINGTABLE\s+DISCARDABLE\s*$/ )
    {
	print( STDERR "$fullPath($$lineNumber) : error : Invalid header.\n" );
	return -1;
    }

    while( $line = <APPFILE> )
    {
  	$$lineNumber++;
  	if( $line =~ /^\s*$/ )
  	{
  	    next;
  	}
  	elsif( $line =~ /^BEGIN\s*$/ )
  	{
  	    last;
  	}
  	else
  	{
  	    print( STDERR "$fullPath($$lineNumber) : error : Invalid header.\n" );
	    return -1;
  	}
    }
    return 0;
}

sub ParseLine
{
    my $line = shift( @_ );
    my $tag = shift( @_ );
    my $value = shift( @_ );

    my $quoteSuppressedLine = $line;

    $quoteSuppressedLine =~ s/\"\"//g;
    if( $line =~ /^\s*$/ )
    {
	return 0;
    }
    elsif( $line =~ /^END\s*$/ )
    {
	return 0;
    }
    elsif( $line =~ /^\w+\s+L\"\"[^\"]*$/ )
    {
	$line =~ /^(\w+)\s+L\"\"[^\"]*$/;
	$$tag = $1; $$value = "";
    }
    elsif( $quoteSuppressedLine =~ /^\w+\s+L\"[^\"]*\"[^\"]*$/ )
    {
	$line =~ /^(\w+)\s+L\"(.*)\"[^\"]*$/;
	$$tag = $1; $$value = $2;
    }
    elsif( $quoteSuppressedLine =~ /^\w+\s+L\"[^\"]*\"\s*\/\*.*\*\/[^\"]*$/ )
    {
	$line =~ /^(\w+)\s+L\"(.*)\"\s*\/\*.*\*\/[^\"]*$/;
	$$tag = $1; $$value = $2;
    }
    else
    {
	return -1;
    }

    return 1;
}

    $fullEnglishPath = $localeDir."/english/".$fileName;
    if( !open( APPFILE, $fullEnglishPath ) )
    {
	die( "failed to open file $fullEnglishPath $!\n" );
    }

    $lineNumber = 0;
    if( ValidateHeader( *APPFILE, \$lineNumber, $fullEnglishPath ) < 0 )
    {
	$finalResult = -1;
    }
    my %stringMap;
    while( $line = <APPFILE> )
    {
	$lineNumber++;

	$result = ParseLine( $line, \$tag, \$value );
	if( $result < 0 )
	{
	    print( STDERR "$fullEnglishPath($lineNumber) : error : Invalid string table entry.\n" );
	    $finalResult = -1;
	}
	elsif( 0 < $result )
	{
	    $stringMap{$tag} = $value;
	}
    }
    close( APPFILE );

    if( !opendir( DIR, $localeDir ) )
    {
	die( "failed to open directory $localeDir $!\n" );
    }
    while( $langDir = readdir( DIR ) )
    {
	if( $langDir =~ /^CVS$/	||
	    $langDir =~ /^\.$/	|| 
	    $langDir =~ /^\.\.$/	||
	    $langDir =~ /^english$/	||
	    $langDir =~ /^docs$/	||
	    $langDir =~ /^Locale.mk$/	|| 
		! -d $langDir )
	{
	    next;
	}


	$fullPath = $localeDir."/".$langDir."/".$fileName;
	#print( "Examining $fullPath\n" );
	if( !open( APPFILE, $fullPath ) )
	{
	    die( "failed to open file $fullPath $!\n" );
	}
	$lineNumber = 0;
	if( ValidateHeader( *APPFILE, \$lineNumber, $fullPath ) < 0 )
	{
	    $finalResult = -1;
	}
	local %langMap;
	while( $line = <APPFILE> )
	{
	    $lineNumber++;

	    $result = ParseLine( $line, \$tag, \$value );
	    if( $result < 0 )
	    {
		print( STDERR "$fullPath($lineNumber) : error : Invalid string table entry.\n" );
		$finalResult = -1;
	    }
	    elsif( 0 < $result )
	    {
		$langMap{$tag} = $value;
		if( defined( $stringMap{$tag} ) && $value eq $stringMap{$tag} && 
		    $printWarnings==1 &&
		    !defined( $ignoreMap{$tag} ) )
		{
		    print( STDERR "$fullPath($lineNumber) : warning : Identical string \"$tag\" \"$value\".\n" );
		}
	    }
	}

	foreach $tag ( keys( %stringMap ) )
	{
	    if( !defined( $langMap{$tag} ) )
	    {
		print( STDERR "$fullPath($lineNumber) : error : Missing string table entry \"$tag\".\n" );
		$finalResult = -1;
	    }
	}
	close( APPFILE );
	#print( "$fullPath\n" );
	#`free/uni2utf8.exe -r $fullPath tempA`;
	#`free/uni2mb.exe tempA tempB`;
    }
    closedir( DIR );

$_ = $finalResult;
exit $finalResult;
