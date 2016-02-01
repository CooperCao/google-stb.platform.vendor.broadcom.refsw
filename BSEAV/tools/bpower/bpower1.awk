BEGIN {
    total=0;
}
{
    ### printf "got line 1(" $1 ") and (" $2 ")\n";
    clkgen1 = index($1,"______CLKGEN");
    clkgen2 = index($2,"______CLKGEN");
    ### printf "clkgen1 " clkgen1 "\n";
    if (clkgen1 > 0)
    {
        clkgen1 += 6;
        newstring = substr($1,clkgen1,199);
        printf newstring " " $2 "\n";
    }
    else if (clkgen2 > 0)
    {
        ### printf "got line 2(" $2 ") and (" $3 "); clkgen2 (" clkgen2 ")\n";
        clkgen2 += 6;
        newstring = substr($2,clkgen2,199);
        printf newstring " " $3 "\n";
    }
    else
    {
        ### printf "unknown line " $1 " " $3 " " $4 " " $5 " " $6"\n";
    }
    total++;
}
END {
}
