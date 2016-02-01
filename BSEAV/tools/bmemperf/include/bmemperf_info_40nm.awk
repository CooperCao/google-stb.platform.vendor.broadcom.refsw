BEGIN {
    total=0;
    found_beginning_of_names=0;
    name_count=0;
    memc_count=0;
    boxmode=0;
}
{
    uint_tag=$1
    idx=$3;
    client_name=$5;
    ddr=$5;
    spb=$6
    total++;
    ## printf("/*    arg2 "$2"; arg7 "$7"   */\n");
    ### sample line to parse:  #define   BCHP_MEMC_ARB_0_CLIENT_INFO_0_VAL  0x00805017  /* XPT_WR_RS 7420ns */
    ###                        $1        $2                                 $3          $4 $5        $6
    if ($1 == "#define" && substr($2,1,4) == "BCHP")
    {
        ### printf "idx " idx "; name " client_name "; Total = "total"\n";
        if (found_beginning_of_names == 1) {
           if (name_count==0) {
              printf("bmemperf_info l_bmemperf_info"boxmode"={"memc_count","a1[1]","a3[1]"};\n\n");
              printf("char *l_bmemperf_clients"boxmode"[BMEMPERF_MAX_NUM_CLIENT] = {");
           }
           print "\""tolower(client_name)"\",";
           name_count++;
        }
    }
    else if ($1 == "#define")
    {
        ### printf "/* found $define */\n";
        found_beginning_of_names = 1;
    }
    ###   #   for: Platform 7429/MemC 0 (32-bit DDR3@800MHz) w/277.714285714286MHz clock)
    ###   $1  $2   $3       $4        $5 $6     $7           $8                    $9
    else if ( substr($7,1,5) == "DDR3@")
    {
        printf "/* found DDR " $7 " " $8"*/\n";
        ddr = substr($7,6,99);
        split(ddr,a1,"M");
        split($8,a2,"/");
        split(a2[2],a3,".");
        printf "/* DDR is " a1[1]"; SCB is "a3[1]"*/\n";
        memc_count++;
    }
    else if ($1 == "};")
    {
           found_beginning_of_names = 0;
           ### print "found ending backet\n"
           printf $1"\n"
      found_beginning_of_names = 0;
      exit 1;
    }
    else
    {
        ##printf "unknown line " $1 " " $3 " " $4 " " $5 " " $6"\n";
    }
}
END {
   printf "};\n"
}
