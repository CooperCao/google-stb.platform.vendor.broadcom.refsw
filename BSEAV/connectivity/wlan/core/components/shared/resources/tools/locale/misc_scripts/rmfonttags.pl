while (<>)
	{
	s/<\/*FONT[^>]*>//g;
	s/<\/*font[^>]*>//g;
	s/<\/*SPAN[^>]*>//g;
	s/<\/*span[^>]*>//g;
	s/<\/*DIV[^>]*>//g;
	s/<\/*div[^>]*>//g;
	print $_;
	}
