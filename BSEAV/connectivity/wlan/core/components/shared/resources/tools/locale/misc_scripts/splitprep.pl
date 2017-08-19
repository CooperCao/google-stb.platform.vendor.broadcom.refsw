while (<>)
	{
	s/(\.define)/\n$1 /g;
	s/(\.block)/\n$1 /g;
	s/(\.build)/\n$1 /g;
	s/(\.endblock)/\n$1 /g;
	s/(\.if)/\n$1 /g;
	s/(\.else)/\n$1 /g;
	s/(\.endif)/\n$1 /g;
	s/(\.image)/\n$1 /g;
	s/(\.\-+)/\n$1 /g;
	print $_;
	}
