#s,\\,/,g
# Sometimes gpp puts the target and source fine on separate lines.  Join them.
/: \\$/N
# Remove the backslash and newline in the middle of the joined line.
s/ \\\n//
# Change the colon-space so it won't be changed later.
s/: /:=/
# Get rid of leading spaces.
s/^ //g
# Get rid of delete-able words.  They are marked before we get here.
s/ DELETE//g
# Put "./" in front of MergedNonVolDefaults.h.
s/MergedNonVolDefaults\.h/.\/MergedNonVolDefaults.h/g
# Get rid of defines.h
s/defines\.h//g
# The previous command might leave a stray blank, so delete it.
s/  / /g
# Split lines with multiple dependencies.
s/ \./ \\\
./g
# Now delete all remaining delete-able lines.
/DELETE/D
#s, /,,
# Turn the equal sign on the first line back into a space.
s/=/ /
#s/p$/p \\/
# If there are any lines ending in "h", add a continuation character.
s/h$/h \\/
s/h\n/h \\\
/g
s/H$/H \\/
s/H\n/H \\\
/g
# Add a continuation character to the last line.
#$s/([^\\])$/\1 \\/
