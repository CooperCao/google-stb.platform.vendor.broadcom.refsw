BEGIN {
    total=0;
    found_beginning_of_names=0;
    name_count=0;
    memc_count=0;
    boxmode=0;
    ddr_string="";
    scb_string="";
}
{
    uint_tag=$1
    idx=$3;
    ddr=$5;
    spb=$6
    total++;
    ## printf("/*    arg2 "$2"; arg7 "$7"   */\n");
    ### sample line to parse:  #define   BCHP_MEMC_ARB_0_CLIENT_INFO_0_VAL  0x00805017  /* XPT_WR_RS 7420ns */
    ###                        $1        $2                                 $3          $4 $5        $6
    ### ./bmemperf/7231/b0/memc_0_default_config.h:*   for: Platform 7230_randys/MemC        0 (32-bit DDR3@866MHz) w/277.714285714286MHz clock)
    ### ./bmemperf/7429/b0/memc_0_default_config.h:*   for: Platform 7429/MemC 0 (32-bit DDR3@833MHz) w/277.714285714286MHz clock)
    ###
    if ($2 == "for:" && $3 == "Platform" )
    {
        num=split($4,parts,"_");
        ### printf "/* platform_string is " $4 " num is " num " */\n";
        if ( num == 1 )
        {
            num=split($4,parts,"/");
            ### printf "/* platform_string is " $4 " num is " num " */\n";
            if ( num == 1 )
            {
                platform = "unknown";
            }
            else
            {
                platform = parts[1];
            }
        }
        else
        {
            platform = parts[1];
            ### printf "/* else platform is " platform " */\n";
        }
    }
    if ($1 == "#define" && substr($2,1,4) == "BCHP")
    {
       if (found_beginning_of_names == 1) {
          if (name_count==0 && boxmode>0) { # skip boxmode 0 because it is pre-initialized in bmemconfig_box_info_pre.awk
             printf(",{"boxmode","memc_count"}\n\n");
          }
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
        ddr_string = $7;
        if ( substr($8,0,2) == "w/" ) {
            scb_string = substr($8,3,99);
        } else {
            scb_string = $8;
        }
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
        exit 1;
    }
    else
    {
        ##printf "unknown line " $1 " " $3 " " $4 " " $5 " " $6"\n";
    }
}
END {
    split(ddr_string,ddr_string2,")"); # separate out the parenthesis at the end of the string
    printf(",\""ddr_string2[1]" SCB@"scb_string"\",\""platform"\","a1[1]","a3[1]"}\n\n");
}
