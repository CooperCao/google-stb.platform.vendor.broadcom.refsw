BEGIN {
    B_ANDROID_TEMP = ENVIRON["B_ANDROID_TEMP"];
    templen = length(B_ANDROID_TEMP);
}
{
    if ( ($1 == "ErrorLog") || ($1 == "AccessLog") )
    {
        ### printf "### found arg1 "$1"; arg2 "$2"; templen "templen";\n";
        if ( templen ) {
            printf $1" "B_ANDROID_TEMP""substr($2,6,99)"\n";
        } else {
            print ### use the line as-is
        }
    }
    else
    {
        print ### use the line as-is
    }
}
END {
}
