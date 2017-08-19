while (<>)
	{
	while (/\s*(<[^>]*>)\s*([\.\,\:\;\"\?\!\)])+/)
		{
		s/\s*(<[^>]*>)\s*([\.\,\:\;\"\?\!\)]+)/$2$1/g;
		}
	while (/([\(\"]\s*)(<[^>]*>)/)
		{
		s/([\(\"]\s*)(<[^>]*>)/$2$1/g;
		}
	s/\s*(<[^\/hH][^>]*>)\s*/\n$1\n/g;	#opening tag, but not a header;
	s/\s*(<\/[^hH][^>]*>)/\n$1\n/g;	#closing tag, but not a header;
	
	s/\s*(<[hH][^>]*>)\s*/\n$1/g;		#opening header tag
	s/\s*(<\/[hH][^>]*>)\s*/$1\n/g;	#closing header tag
	print $_;
	}
