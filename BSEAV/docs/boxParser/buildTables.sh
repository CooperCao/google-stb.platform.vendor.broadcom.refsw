#!/bin/bash

# Need this to make plat work inside a script
BRCM_PLAT_DOT_OVERRIDE=y

REFSW_ROOT=`readlink -f ../../../`
PARSER_ROOT=BSEAV/docs/boxParser

ARRAY=( "7250:B0"
        "7260:A0"
        "7268:B0"
        "7271:B0"
        "7278:A0"
        "7364:C0"
        "7366:C0"
        "74371:A0"
        "7439:B0"
        "7445:E0"
      )

for chip in "${ARRAY[@]}" ; do
    PLATFORM=${chip%%:*}
    REV=${chip#*:}
    source $REFSW_ROOT/BSEAV/tools/build/plat 9$PLATFORM $REV
    make
    $REFSW_ROOT/${B_REFSW_OBJ_DIR}/$PARSER_ROOT/boxParser > "${PLATFORM}${REV}_box_mode_comparison.html"
done


SUMMARY_HTML=box_mode_comparison.html

# Build summary page
echo "<!DOCTYPE html>" > $SUMMARY_HTML
echo "<html>" >> $SUMMARY_HTML
echo "<head>" >> $SUMMARY_HTML
echo "<style>"  >> $SUMMARY_HTML
echo "    table, th, td {border: 1px solid black; border-collapse: collapse; text-align: center;}" >> $SUMMARY_HTML
echo "    th, td {padding: 5px;}" >> $SUMMARY_HTML
echo "</style>" >> $SUMMARY_HTML
echo "</head>" >> $SUMMARY_HTML
echo "<body>" >> $SUMMARY_HTML
echo "<h1>Box mode comparison table</h1>" >> $SUMMARY_HTML
echo "<table><tr><th>Chip</th><th>Rev</th><th>Box mode data</th></tr>" >> $SUMMARY_HTML
for chip in "${ARRAY[@]}" ; do
    PLATFORM=${chip%%:*}
    REV=${chip#*:}
    echo "    <tr><td>${PLATFORM}</td><td>${REV}</td><td><a href=${PLATFORM}${REV}_box_mode_comparison.html>${PLATFORM}${REV} Box modes</a></td></tr>"  >> $SUMMARY_HTML
done
echo "</table></body></html>" >> $SUMMARY_HTML
