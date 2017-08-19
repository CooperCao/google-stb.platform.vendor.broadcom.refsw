BEGIN {
    total=0;
    found_beginning_of_names=0;
    name_count=0;
    memc_count=1;
    line = 0;
}
{
    line++;
    if (line == 1)
    {
        num=split(FILENAME,filename_paths,"/");
        ### printf ("/* num filename_paths is (" num ") */\n");
        num=split(filename_paths[num],filename_parts,"_");
        ### printf ("/* num filename_parts is (" num ") */\n");
        boxmode = substr(filename_parts[3],4);
        printf ("/* Processing: (" FILENAME "); boxmode (" boxmode ") */\n");
    }

    uint_tag=$1
    idx=$3;
    client_name=$4;
    ddr=$5;
    total++;
    ### printf "/* arg5 is " $5 " and arg6 is " $6 " */\n";
    if ( ($1 == "static" && $2 == "const" && $3 == "uint32_t") || ($1 == "const" && $2 == "uint32_t") || ($1 == "uint32_t") )
    {
        ### printf "/* found uint32_t */\n";
        found_beginning_of_names = 1;
    }
    else if ( substr($5,1,7) == "DDRDDR3" )
    {
        ### printf "/* found DDR " $5 " " $6" */\n";
        ddr = substr($5,9,99);
        split(ddr,a1,"M");
        split($6,a2,"/");
        split(a2[2],a3,"M");
        ### printf "/* DDR is " a1[1]"; SCB is "a3[1]" */\n";
        memc_count++;
    }
    ### some of the newer boxmode files have LPDDR4@1067MHz
    else if ( substr($5,1,6) == "LPDDR4" )
    {
        ### printf "/* found DDR " $5 " " $6" */\n";
        ddr = substr($5,8,99);
        split(ddr,a1,"M");
        split($6,a2,"/");
        split(a2[2],a3,"M");
        ### printf "/* DDR is " a1[1]"; SCB is "a3[1]" */\n";
        memc_count++;
    }
    ### some of the newer boxmode files do not have DDRDDR3@1067 ... they just have DDR3@1067 or DDR4@1200
    else if ( substr($5,1,4) == "DDR3" || substr($5,1,4) == "DDR4" )
    {
        ### printf "/* found DDR " $5 " " $6" */\n";
        ddr = substr($5,6,99);
        split(ddr,a1,"M");
        split($6,a2,"/");
        split(a2[2],a3,"M");
        ### printf "/* DDR is " a1[1]"; SCB is "a3[1]" */\n";
        memc_count++;
    }
    else if ($2 == "/*")
    {
        ### printf "/* idx " idx "; name " client_name "; Total = "total" */\n";
        if (found_beginning_of_names == 1) {
           if (name_count==0) {

              if (a1[1] == "0" || a1[1] == "") { printf("#warning DDR frequency ... " a1[1] " ... is invalid in file "FILENAME"\n"); a1[1]=800; }
              if (a3[1] == "0" || a3[1] == "") { printf("#warning SCB frequency ... " a3[1] " ... is invalid in file "FILENAME"\n"); a3[1]=432; }

              printf("bmemperf_info l_bmemperf_info"boxmode"={"memc_count","a1[1]","a3[1]"};\n\n");
              printf("char *l_bmemperf_clients"boxmode"[BMEMPERF_MAX_NUM_CLIENT] = {");
           }
           print "\""tolower(client_name)"\",";
           name_count++;
        }
    }
    else if ($1 == "};")
    {
           found_beginning_of_names = 0;
           ### print "/* found ending backet */\n"
           printf $1"\n"
      found_beginning_of_names = 0;
      exit 1;
    }
    else
    {
        ### printf "#if 0\n/* unknown line " $1 " " $3 " " $4 " " $5 " " $6" */\n#endif\n";
    }
}
END {
    if (memc_count == 0 ) printf("#error memc_count ... " memc_count " ... is invalid in file"FILENAME"\n");
    if (a1[1] == "0" || a1[1] == "") printf("#error DDR frequency ... " a1[1] " ... is invalid in file "FILENAME"\n");
    if (a3[1] == "0" || a3[1] == "") printf("#error SCB frequency ... " a3[1] " ... is invalid in file "FILENAME"\n");
}
