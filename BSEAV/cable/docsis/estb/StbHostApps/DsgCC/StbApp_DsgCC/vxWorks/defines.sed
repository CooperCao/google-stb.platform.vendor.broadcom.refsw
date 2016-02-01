# This sed script takes a list of "-D" compiler options and turns them into
# C-style "#define" statements.
s/$/ /
s/-D[^=-]* /&=1 /g
s/-D/\
#define /g
s/=/ /g

