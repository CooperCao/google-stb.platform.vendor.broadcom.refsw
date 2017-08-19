DIRS:
-----
release:
    This directory contains built objects to be released to external world.
    Read release/README.txt for additional details on release files.

NOTE: 'release' folder should address all your needs and other directories
that are shown below are created and used by build process and you can 
ignore them.

build:
    This directory, if it exists,  contains filtered sources copied from
    'src' directory. This directory is used to run the actual build 
    (compilation). If this dir does not exist then, build is done 
    inside 'src' itself.

src:
    This directory contains original checked out sources from CVS. The 
    contents here are mogrified with DEFS and UNDEFS release.mk flags.
    Do not release to external customers anything from this dir directly

msftsrc:
    This directory, if it exists, is our wl driver sources extracted
    in the format needed by microsoft build/ddk/wdk tools.

misc:
    This directory contains misceleneous information about the build itself.
    Information like detailed build annotation (what build job ran where etc.,)
    is all preserved here.


FILES:
------
 ,succeeded    : If exists, it indicates build/compilation was completely successful
 release.mk    : The top level makefile that was used to produce this build
 ,release.log  : Complete build log
 ,swrlstemp.mk : Secondary makefile used during mogrification
 profile.log   : Sequence of build steps and timestamps
 misc/,env.log : Environment log on the system where build was done
 misc/,move.log: Log from deletion of any previous build iterations
 misc/,*.xml   : If exists, then it is detailed built annotation log

 misc/build_status.txt : Build status of individual pc-oem brand packaging. Use this to if ,succeeded does not exist to find out if a particular pc-oem xp or vista brand id built or not.
