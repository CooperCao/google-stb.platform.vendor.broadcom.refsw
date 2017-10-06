#perl
{
$table_idx = 0;
while (<>)
	{
	if (/STRINGTABLE\s*DISCARDABLE/)
		{
		$table_idx++;
		}
	if (/^STR_\S*/ || /^IDS_\S*/ || /^IDR_\S*/)
		{
		if (!$table_type{$&})
			{
			$table_type{$&} = $table_idx;
			push @table, $&;
			}
		}
	}
$idx = 4000;
for ($i = 1; $i <= $table_idx; $i++)
	{
	foreach $id (@table)
		{
		if ($table_type{$id} == $i)
			{
			print "#define $id\t$idx\n";
			$idx++;
			}
		}
	$next_idx = $idx + 99;
	$next_idx -= $next_idx % 100;
	if (($next_idx - $idx) >= 32)
		{
		$idx = $next_idx;
		}
	else
		{
		$idx = $next_idx + 100;
		}
	}
}
