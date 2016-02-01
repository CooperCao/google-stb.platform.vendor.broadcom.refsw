if [ "$1" == "clean" ]; then
for Dir in $(find ./* -maxdepth 0 -type d );
do
    FolderName=$(basename $Dir);
    cd $FolderName
    echo ------------------------ Cleaning \"$FolderName\" ---------------------------
    make clean
    cd ../
done
exit 0
fi
for Dir in $(find ./* -maxdepth 0 -type d );
do
    FolderName=$(basename $Dir);
    cd $FolderName
    echo ++++++++++++++++++++++++ Making \"$FolderName\" +++++++++++++++++++++++++++
    make -j 8
    cd ../
done