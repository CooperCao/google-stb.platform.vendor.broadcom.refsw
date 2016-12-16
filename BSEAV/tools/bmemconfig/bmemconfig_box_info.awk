### There are three ways to determine which variant of the chip we are dealing with:
###  1) in file bbox_memc_box?_config.c, there is a line similar to this ---> for: Box 7252_4Kstb
###     (this is the preferred method, but does not always work ... fails for the 7366, 7364.)
###  2) in file bbox_memc_box?_config.c, there is a line similar to this ---> static const uint32_t aulMemc0_20150528194048_4563_2t[] = {
###     (this doesn't always work because the naming conventions are inconsistant and do not always have the variant in it.)
###  3) failing both of the above, the fail safe is to extract the platform from the filename --->
###     magnum/commonutils/box/src/7366/c0/bbox_memc_box1_config.c
BEGIN {
    total=0;
    found_beginning_of_names=0;
    name_count=0;
    memc_count=0;
    ddr_string="";
    scb_string="";
    line = 0;
    platform_from_filename="";
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

        ### split this /local/public/users/detrick/refsw/gitrepo038/nexus/../magnum/commonutils/box/src/7366/c0/bbox_memc_box1_config.c
        num=split(FILENAME,PLAT_ELEMENTS,"magnum/commonutils/box/src/");
        num=split(PLAT_ELEMENTS[2],PLAT_ELEMENTS2,"/");
        platform_from_filename = PLAT_ELEMENTS2[1];
        ### printf ("/* FILE: (" PLAT_ELEMENTS[2] "); platform_from_filename is (" platform_from_filename ") */\n");
    }

    ### printf ("line is: " $0 "\n");
    ### look for line that starts with: static const uint32_t aulMemc0_20141112013607_1stb1
    if ( $1 == "static" && $2 == "const" && $3 == "uint32_t" && substr($4,1,8) == "aulMemc0" )
    {
        ### sometimes we can determine the variant from the aulMemc0 structure name
        ### printf ("/* found line: " $0 " */\n");
        num=split($4,variants,"_");
        ### if ( num >=3 ) printf ("/* variants3 (" variants[3] ") length (" length(variants[3]) ") substr (" substr(variants[3],1,4) ") */\n");
        if ( (variants[3] ~ /^[0-9]+$/) && (length(variants[3]) >= 4) ) {
            ### printf ("/* variant is NUMERIC (" variants[3] ") ... platform is (" platform ")  platform_from_filename (" platform_from_filename ") */\n" );
            if ( length(platform) < 4 ) {
                printf ("/* overriding bad platform (" platform ") ... with one from filename (" platform_from_filename ") */\n" );
                platform = platform_from_filename;
            }
        } else if ( (length(variants[3]) == 5) && (substr(variants[3],1,4) ~ /^[0-9]+$/) ) {
            ### printf ("/* variant is numeric with one char at end " variants[3] ") ... platform is (" platform ") platform_from_filename (" platform_from_filename ") */\n" );
        } else {
            ### we could not determine the variant from the struct so use the platform name from the filename
            ### printf ("/* variant is NOT numeric and at least 4 chars (" variants[3] ") ... platform is (" platform ") */\n" );
            printf ("/* overriding bad platform (" platform ") ... with one from filename (" platform_from_filename ") */\n" );
            platform = platform_from_filename;
        }
    }
    uint_tag=$1
    idx=$3;
    ddr=$5;
    total++;
    ### printf "/* line is " $1 $2 $3 " */\n";
    if ($2 == "for:" && $3 == "Box" )
    {
        ### $4 = "7445D0";
        num=split($4,parts,"_");
        ### printf "/* platform_string is " $4 " num is " num " */\n";
        if ( num == 1 )
        {
            platform = substr(parts[1],1,4);
            ### printf "/* then platform is " platform " */\n";
        }
        else
        {
            platform = parts[1];
            ### printf "/* else platform is " platform " */\n";
        }
    }
    if ( ($1 == "static" && $2 == "const" && $3 == "uint32_t") || ($1 == "const" && $2 == "uint32_t") || ($1 == "uint32_t") )
    {
        ### printf "/* found uint32_t */\n";
        found_beginning_of_names = 1;
    }
    else if ( substr($5,1,7) == "DDRDDR3" )
    {
        ### printf "/* found DDR " $5 " " $6" */\n";
        ddr_string = $5;
        if ( substr($6,0,2) == "w/" ) {
            scb_string = substr($6,3,99);
        } else {
            scb_string = $6;
        }
        ddr = substr($5,9,99);
        split(ddr,a1,"M");
        split($6,a2,"/");
        split(a2[2],a3,"M");
        ### printf "/* DDR is " a1[1]"; SCB is "a3[1]" */\n";
        memc_count++;
    }
    ### some of the newer boxmode files have LPDDR4@1067MHz
    ### $1        $2   $3$4      $5              $6       $7
    ### *         MemC 0 (32-bit LPDDR4@1067MHz) w/432MHz clock

    else if ( substr($5,1,6) == "LPDDR4" )
    {
        ### printf "/* found DDR " $5 " " $6" */\n";
        ddr_string = $5;
        if ( substr($6,0,2) == "w/" ) {
            scb_string = substr($6,3,99);
        } else {
            scb_string = $6;
        }
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
        ddr_string = $5;
        if ( substr($6,0,2) == "w/" ) {
            scb_string = substr($6,3,99);
        } else {
            scb_string = $6;
        }
        ddr = substr($5,6,99);
        split(ddr,a1,"M");
        split($6,a2,"/");
        split(a2[2],a3,"M");
        ### printf "/* DDR is " a1[1]"; SCB is "a3[1]" */\n";
        memc_count++;
    }
    else if ($2 == "/*")
    {
        if (found_beginning_of_names == 1) {
           if (name_count==0 && boxmode>0) { # skip boxmode 0 because it is pre-initialized in bmemconfig_box_info_pre.awk
              split(ddr_string,ddr_string2,")"); # separate out the parenthesis at the end of the string
              printf(",{"boxmode","memc_count",\""ddr_string2[1]" SCB@"scb_string"\",\"" toupper(platform) "\","a1[1]","a3[1]"}\n\n");
           }
           name_count++;
        }
    }
    else if ($1 == "};")
    {
        found_beginning_of_names = 0;
        ### print "/* found ending backet */\n"
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
