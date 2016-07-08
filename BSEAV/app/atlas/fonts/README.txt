Atlas uses bitmapped fonts generated from the original TrueType fonts.
To generate new bitmapped fonts, use the BSEAV/lib/bwin/utils/buildfont utility.

To build 'buildfont':
	1. cd BSEAV/lib/bwin/utils
	2. make clean
		this must be done so that bwin can be rebuilt with FREETYPE_SUPPORT=y
	3. make
		this will create the buildfont utility and copy it to the nexus bin directory

To run 'buildfont' (on set-top box):
	1. copy verdana.ttf and verdanai.ttf TrueType font files to your atlas bin/fonts directory
	2. copy BSEAV/app/atlas/fonts/mswebfonts/prerender script to your atlas bin/fonts directory
	3. on set-top box run the prerender script in the atlas bin/fonts directory.
		this will generate the necessary *.bwin_font bitmapped fonts.
	4. copy the *.bwin_font files back to the build server's BSEAV/app/atlas/fonts/mswebfonts/le
		directory to add to source control.
