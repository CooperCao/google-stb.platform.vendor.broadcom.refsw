#!/bin/sh
COMMENT=$1
FILENAME=$2
#Convert to lower case
LOWERFILENAME=`echo $FILENAME | tr '[A-Z]' '[a-z]'`
mv $FILENAME $LOWERFILENAME
echo "Finished conversion to lower case..."
#remove newline and eof characters
tr -d '\015\032' < $LOWERFILENAME > temp.csv
echo "Removed newline and eof characters..."
mv temp.csv $LOWERFILENAME
#svn add and checkin
echo "svn add "$LOWERFILENAME
echo "svn ci -m \""$COMMENT"\"" $LOWERFILENAME
svn add $LOWERFILENAME
sleep 5
svn ci -m \""$COMMENT\"" $LOWERFILENAME
