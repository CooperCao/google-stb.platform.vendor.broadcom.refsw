BEGIN {
    total=0;
    found_beginning_of_names=0;
    name_count=0;
    printf( "static char * postProcessing[]={\n" );
}
{
###typedef enum NEXUS_AudioPostProcessing
###{
###    NEXUS_AudioPostProcessing_eSampleRateConverter,
###    NEXUS_AudioPostProcessing_eCustomVoice,
###    NEXUS_AudioPostProcessing_eAutoVolumeLevel,
###    NEXUS_AudioPostProcessing_eTrueSurround,
###    NEXUS_AudioPostProcessing_eTruVolume,
###    NEXUS_AudioPostProcessing_eDsola,
###    NEXUS_AudioPostProcessing_eBtsc,
###    NEXUS_AudioPostProcessing_eMax
###} NEXUS_AudioPostProcessing;
    uint_tag=$1
    idx=$3;
    ### look for this line ... typedef enum NEXUS_AudioPostProcessing
    if ( $1 == "typedef" && $2 == "enum" && $3 == "NEXUS_AudioPostProcessing" )
    {
        ### printf("found ...  "$0 "   \n");
        found_beginning_of_names = 1;
    }
    if ( found_beginning_of_names )
    {
        pos=index( $2, ";" );
        if ( pos > 0 ) {
            found_beginning_of_names = 0;
            quit
        }

        pos=index( $1, "NEXUS_AudioPostProcessing_e" );
        if ( pos > 0 ) {
            end_of_string = substr($1,28,99);
            num=split(end_of_string,parts,",");
            if ( num == 1 ) {
                printf("};\n"); ## we found the end of the enum
            } else {
                ##printf("found ...  eos(" end_of_string ");   parts1(" parts[1] ")   num (" num ") \n");
                if ( total > 0 ) printf(",");
                printf("\"" parts[1] "\"\n");
                total++;
            }
        }
    }
}
END {
}
