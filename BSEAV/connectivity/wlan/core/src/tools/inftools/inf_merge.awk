{ 
if (NR == 1)
	{
	devIDMatch = 0; 
	i = 0; 
	while (getline s[i] < inf  > 0) i++ ; 
	N = i; 
	}
	if ($1 ~ /\[(.*)\]/ ) 
		{ 
		printOriginalLine = 1; 
		# a section is detected in the main inf file
		devIDMatch = 0; 
		if ($1 ~ /_Modems\]/ ) 
			{
			# detected device specification
			devIDMatch = 1; 
			devIDSection = $1;
			}
		insert = 0; 
		for (i = 0; i < N; i++) 
			{ 
			#loop through the merged inf file
			if (s[i] ~ /\[.*\]/) 
				{ 
				# a section is detected in the merged inf file
				insert = 0; 
				if (s[i] == $1) 
					{ 
					print; 
					printOriginalLine = 0; 
					insert = 1; 
					} 
				} 
			else 
				{ 
				if (insert && (s[i] != "")) 
					{
					if (devIDMatch && ((x = match(s[i], "VEN_....&DEV....")) != 0))
						{
						if (substr(s[i], x+17, 1) !~ /&/)
							print s[i]; 
						}
					else
						print s[i]; 
					}
				} 
			} 
		if (printOriginalLine) print; 
		}
	else
		{
		print;
		if (devIDMatch && ((x = match($0, "VEN_....&DEV....")) != 0))
			{
			if (substr($0, x+17, 1) !~ /&/)
				{
				devID = substr($0, x, 17);
				for (i = 0; i < N; i++) 
					{ 
					#loop through the merged inf file
					if ((x = match(s[i], devID)) != 0)
						{
						if (substr(s[i], x+17, 1) ~ /&/)
							{
							print s[i]; 
							}
						}
					}
				}
			}
		}
} 
