         ********************************************************
         * How to use splash tool on broadcom's reference board *
         ********************************************************
---------------------------------
What is the splash tool?
---------------------------------

The splashtool is used to generate and capture the requisite register writes
and RUL to display a graphics image on a given display during CFE boot time.
The tool is comprised of 2 sub-tools: splashgen and splashrun. Splashgen
generates and captures the register writes and RUL into .h files. Splashrun
verifies if the captured rgister writes and RUL works independently of the CPU.
These .h files are then integrated into the CFE.

---------------------------------
Features supported :
---------------------------------
	1. RGB565 and ARGB8888 surfaces.
	2. HD Output support
	3. 480p + NTSC support
	4. 576p + PAL support

---------------------------------
What does the tool contain ?
---------------------------------
	1. Two linux usermode applications
		a) splashgen - generates the register and rul dumps
		b) splashrun - Simple utility to download and run the VDC
		   scripts. This loader splash_script_load.c has also been
		   ported to the CFE
	   Both have portable libary components with simple apps wrapping
	   them for the linux usermode environment.
	2. Magnum basemodules/commonutils/portinginterace used by the
	   splashgen and splashrun. Some of the files may be different from
	   the regular refsw files.
	3. The nexus build framework and user mode board support package
	   for the 97xxx board.

---------------------------------
Build and Execute Instructions :
---------------------------------

The below assumes that the required build variables are set.

1. Splashgen make Instructions

    - > cd BSEAV\app\splash\splashgen
    - > Edit BSEAV\app\splash\splashgen\platform_name\bsplash_board.h to your
	    required confiuration.
    - > make
    - With above instructions, the binary will be generated in
	  obj.platform_name\nexus\bin directory.

    You can optionally provide a suffix that will be used files written by
	splashgen. In this case, you would invoke make as, for example:
	- > make suffix=_composite_only
	More on the suffix later.

	- For chips that support box modes, default RTS settings, ie., box mode 0
	RTS will be used unless BBOX_USE_BOX_MODE_0_RTS is set to n.

2. Running Splashgen

    - Splashgen has available options. To see these, do splashgen -h.
    - Splashgen without any options will default to display formats specified
      in bsplash_board.h.
    - Splashgen also has the option (-f) to scale the bmp to fit the destination
      display.
    - When splashgen is ran, the bmp file will display the bmp after the .h
	  files are created.
    - Make sure HDMI is connected.
    - > nexus splashgen
    - Check and make sure the logo is displayed on all outputs of interest.
    - The 2 files below will be copied to obj.platform_name\nexus\bin directory.
        - splash_vdc_rul.h
        - splash_vdc_rul.c

	    If you have provided a suffix on the command line to make, the above
		two files will involve your suffix of choice. For example, if you
		invoked make as
			make suffix=_composite_only
		then the two generated files will be named
        - splash_vdc_rul_composite_only.h
        - splash_vdc_rul_composite_only.c

        But these files are not final. New file names will be created when
		splashrun is executed. More on this later.

3. Splashrun make Instructions

    - The splashrun utlity is a simple loader function that loads the C
	  register dumps and the RUL dumps and triggers the BVN.
    - > cd BSEAV\app\splash\splashrun
    - > plat 97405 B0
    - > make
    - This above instructions will generate the splashrun binary in
	  obj.platform_name\nexus\bin directory.

	If you built splashgen with a suffix, then you should build splashgen in
	exactly the same way. Continuing previous examples,
		make suffix=_composite_only

	- For chips that support box modes, default RTS settings, ie., box mode 0
	RTS will be used unless BBOX_USE_BOX_MODE_0_RTS is set to n.

4. Running Splashrun

    - cd obj.platform_name\nexus\bin
    - > splashrun
    - Check and make sure the logo is displayed on all outputs of interest.

         ****************************************************
         * How to customize splash tool on customer's board *
         ****************************************************
Two file that need to be changed
	BSEAV\app\splash\splashgen\93549\bsplash_board.h,
	BSEAV\app\splash\splashgen\93549\platformconfig.c,
in this file, customer can add or remove registers that need to be excluded
from the dump in the splash_vdc_reg.h based on their own board.
Generally, PWM, PIN MUX, GPIO, LVDS, DVPO will be involved.

---------------------------------
  File description
---------------------------------
       splash_vdc_rul.h - Declares a function that fetches prebuilt RULs and
	                      other info for getting the splash screen output.
						  The function name is, by default, GetSplashData().
       splash_vdc_rul.c - Implementation behind the above file. You have to
	                      compile it. You don't have to modify it.

	   These files will have different names if you provide a suffix on the
	   command line to make. Continuing the previous examples, if you invoked
	   make as
			make suffix=_composite_only
	   you will obtain two files
	   splash_vdc_rul_composite_only.h
	   splash_vdc_rul_composite_only.c

	   The function name will be GetSplashData_composite_only()

		Finally, when you invoke make to build splashrun, it will copy these
		files from the obj.platform_name\nexus\bin directory back to the
		platform_name_platform_revision subdirectory of
		BSEAV\app\splash\splashrun. The file names (but not content) will be
		modified. By default, the file names are

        60Hz format:
        - splash_vdc_rul_def.c
        - splash_vdc_rul_def.h

        50Hz format:
        - splash_vdc_rul_pal.c
        - splash_vdc_rul_pal.h

		If you specified a suffix when you invoked make, these file names will
		be different. Continuing the above examples, if you invoked make as
			make suffix=_composite_only
		then the file names will be as follows:

        60Hz format:
        - splash_vdc_rul_composite_only_def.c
        - splash_vdc_rul_composite_only_def.h

        50Hz format:
        - splash_vdc_rul_composite_only_pal.c
        - splash_vdc_rul_composite_only_pal.h


       Important for BOLT integration:
            If the above will be integrated to BOLT, the suffix
       needs to be removed for both the .h and .c files and the GetSplashData
       function as BOLT expects these to be without the suffix.

       splash_vdc_setup.c - Key file that does all the loading of the REG and
                            RUL scripts.

           This file is has portable functions for loading the register script
           and the RULs into appropriate memory location and triggering the
	   Video backend
           It uses the magnum functions
	BREG_Write32
	BMMA_Alloc
	BKNI_Memcpy
	BMMA_Lock/BMMA_Unlock
           Some of these magnum functions can have a cheaper alternate
	   definition for the environment in question. This is implemented
	   in splash_magnum.h in the BSEAV\app\splash\os\cfe
	BRDC_OP_IMM_TO_REG - magnum/commonutils/rdc/7038/brdc.h
	BRDC_REGISTER - magnum/commonutils/rdc/7038/brdc.h
	BCHP_FIELD_DATA - magnum/basemodules/chp/bchp.h
       splash_vdc_run.c - Top level file demonstrates calls to the
           splash_vdc_setup.c
       bsplash_board.h - Files implements
           1. marcos for controlling the output configration
           2. Exclude marco for excluding registers from the dump
	   3. DACs on the board used for various outputs
	   4. Surface type
	   etc.
       platformconfig.c - This configures the top level pin muxes to get the
	       desired output on on the custom board.
       splash_bmp.c - Implements graphics operations like rendering BMP files
	       with RGB565/ARGB8888.
