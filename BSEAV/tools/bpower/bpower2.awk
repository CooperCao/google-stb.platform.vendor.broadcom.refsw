BEGIN {
    total=0;
    printf "CLK_TREE_REG_INFO registerInfo[] = { /* name, field, polarity, regOffset, regMask */\n";
}
{
   if (total > 0)
   {
       printf ",";
   }
    split($1,values,":");
    printf "{\"" values[1] "\",\"" values[2] "\"," values[3] ",BCHP_" values[1] ",BCHP_" values[1] "_" values[2] "_MASK,BCHP_" values[1] "_" values[2] "_SHIFT}\n";
    total++;
}
END {
    printf "};\n";
}
