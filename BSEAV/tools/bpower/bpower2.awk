BEGIN {
    total=0;
    HighIs=0;
    printf "CLK_TREE_REG_INFO registerInfo[] = { /* name, field, polarity, regOffset, regMask */\n";
}
{
   if (total > 0)
   {
       printf ",";
   }
    num=split($1,values,":");
    ### some of the lines only have 2 entries on it ... missing the HighIsOn or HighIsOff value
    if (num<3)
    {
        HighIs = "HighIsOn";
    }
    else
    {
        HighIs = values[3];
    }
    printf "{\"" values[1] "\",\"" values[2] "\"," HighIs ",BCHP_" values[1] ",BCHP_" values[1] "_" values[2] "_MASK,BCHP_" values[1] "_" values[2] "_SHIFT}\n";
    total++;
}
END {
    printf "};\n";
}
