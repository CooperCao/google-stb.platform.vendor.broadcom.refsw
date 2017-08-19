while (<>)
	{
	s/\&quot;/"/g;
	s/(.)(\.define)/$1\n$2/g;
	s/(.)(\.block)/$1\n$2/g;
	s/(.)(\.endblock)/$1\n$2/g;
	s/(.)(\.build)/$1\n$2/g;
	s/(.)(\.if)/$1\n$2/g;
	s/(.)(\.else)/$1\n$2/g;
	s/(.)(\.endif)/$1\n$2/g;
	s/(.)(\.image)/$1\n$2/g;
	s/(.)(\.-)/$1\n$2/g;
	s/(\.else)(\s*\S)/$1\n$2/g;
	s/(\.endblock)(\s*\S)/$1\n$2/g;
	s/(\.endif)/$1\n/g;
	s/(\.block\s+[0-9A-Za-z_]*)([^0-9A-Za-z_])/$1\n/g;
	s/(\.build\s+[0-9A-Za-z_]*)([^0-9A-Za-z_])/$1\n/g;
	print $_;
	}
