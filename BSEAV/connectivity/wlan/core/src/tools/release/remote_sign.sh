#!/bin/bash
#
# Runs signtool on remote computer. Usage:
# remote_sign.sh signtool_path signtool_args files
# signtool_path - directory where signtool is located on remote computer. Path shall be in Windows format, with forward or
#                 backward slashes as directory separators. z:\projects\hnd\tools\win changed to c:\tools as neither Z: nor
#				  UNC is available to remote computer
# signtool_args - command arguments without files to sign.z:\projects\hnd\tools\win changed to c:\tools as neither Z: nor
#                 UNC is available to remote computer
# files         - space separated list to files to be passed to sign command. Wildcards not allowed. Basenames of files must
#                 be different (e.g foo/bar baz/bar is not allowed)

# Signing 'server'
SSH_TARGET=wc-sjca-0604
# Save command line arguments (e.g. to avoid damaging with 'set -x')
local_files=${@:3}
signtool_path=$1
signtool_args=$2
# Drive Z: is inaccessible on far end. UNC is not quite accessible. Mapping most likely not performed. So - tweaking command
# to 'all from c:'. Many tweaks may need to be nedcessary
signtool_path=`echo "$signtool_path" | sed 's;z:[/\\\\]projects[/\\\\]hnd[/\\\\]tools[/\\\\]win[/\\\\];c:\\\\tools\\\\;i'`
signtool_args=`echo "$signtool_args" | sed 's;z:[/\\\\]projects[/\\\\]hnd[/\\\\]tools[/\\\\]win[/\\\\];c:\\\\tools\\\\;i'`
signtool_path=`echo "$signtool_path" | sed 's;8161wdk;9200wdksdk;i'`
signtool_path=`echo "$signtool_path" | sed 's;\([^\\\\/]\)$;\1/;'`
# All this ..._u and ..._w here and there are because cygpath is unavailable to this script
signtool_path_u=`echo "$signtool_path" | sed 's;\\\\;/;g'`
signtool_path_w=`echo "$signtool_path" | sed 's;/;\\\\;g'`
echo "Running signtool on $SSH_TARGET"
echo "Signtool executable: ${signtool_path_u}signtool.exe"
echo "Signtool parameters: $signtool_args"
echo "Files passed to signtool: ${local_files[@]}"
# Command will be passed to remote computer in form of batch file. Start preparing it in 'local_command_file'
local_command_file=`mktemp --suffix _remote_sign`
echo -n "@c:\\tools\\build\\bcmretrycmd ${signtool_path_w}signtool $signtool_args " >> $local_command_file
# Creating remote temporary directory for files being signed and for command file
remote_temp=`ssh $SSH_TARGET mktemp -d --suffix _remote_sign`
# Loop over files to be signed
for full_filename in $local_files; do
    full_filename_u=`echo "$full_filename" | sed 's;\\\\;/;g'`
    base_filename=$(basename $full_filename_u)
    # Appending to command file
    echo -n "$base_filename " >> $local_command_file
    # Copying to remote directory
    scp -B $full_filename_u $SSH_TARGET:$remote_temp/$base_filename || echo "ERROR: Failed to copy $full_filename to $SSH_TARGET"
done
# Copying command file to remote directory
scp -B $local_command_file $SSH_TARGET:$remote_temp/signing_command.bat || echo "ERROR: Failed to copy signing command file to $SSH_TARGET"
# Perform signing remotely
ssh  $SSH_TARGET "cd $remote_temp ; cmd /c signing_command.bat"
# ... and saving its status
exit_status=$?
# Copying remotely signed files back
for full_filename in $local_files; do
    full_filename_u=`echo "$full_filename" | sed 's;\\\\;/;g'`
    base_filename=$(basename $full_filename_u)
    scp -B $SSH_TARGET:$remote_temp/$base_filename $full_filename_u
    copy_status=$?
    if [ $copy_status -ne 0 ]; then
        $exit_status=$copy_status
        echo "ERROR: Failed to copy $full_filename from $SSH_TARGET"
    fi
done
# Cleaning up
ssh  $SSH_TARGET 'rm -rf $remote_temp'
rm -f $local_command_file
# Returning success status
exit $exit_status
