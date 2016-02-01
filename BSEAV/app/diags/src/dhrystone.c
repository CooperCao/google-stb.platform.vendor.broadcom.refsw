/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/*
 *
 *	"DHRYSTONE" Benchmark Program
 *
 *  Version:    C, Version 2.1
 *
 *  File:       dhry_1.c (part 2 of 3)
 *
 *  Date:       May 25, 1988
 *
 *	Author:		Reinhold P. Weicker,  CACM Vol 27, No 10, 10/84 pg. 1013
 *			Translated from ADA by Rick Richardson
 *			Every method to preserve ADA-likeness has been used,
 *			at the expense of C-ness.
 *
 *	Compile:	cc -O dry.c -o drynr			: No registers
 *			cc -O -DREG=register dry.c -o dryr	: Registers
 *
 *	Defines:	Defines are provided for old C compiler's
 *			which don't have enums, and can't assign structures.
 *			The time(2) function is library dependant; Most
 *			return the time in seconds, but beware of some, like
 *			Aztec C, which return other units.
 *			The LOOPS define is initially set for 50000 loops.
 *			If you have a machine with large integers and is
 *			very fast, please change this number to 500000 to
 *			get better accuracy.  Please select the way to
 *			measure the execution time using the TIME define.
 *			For single user machines, time(2) is adequate. For
 *			multi-user machines where you cannot get single-user
 *			access, use the times(2) function.  Be careful to
 *			adjust the HZ parameter below for the units which
 *			are returned by your times(2) function.  You can
 *			sometimes find this in <sys/param.h>.  If you have
 *			neither time(2) nor times(2), use a stopwatch in
 *			the dead of the night.
 *			Use a "printf" at the point marked "start timer"
 *			to begin your timings. DO NOT use the UNIX "time(1)"
 *			command, as this will measure the total time to
 *			run this program, which will (erroneously) include
 *			the time to malloc(3) storage and to compute the
 *			time it takes to do nothing.
 *
 *	Run:		drynr; dryr
 *
 *	Results:	If you get any new machine/OS results, please send to:
 *
 *				seismo!uunet!pcrat!rick	(Mail)
 *				seismo!uunet!pcrat!dry	(Submission Forms)
 *
 *			and thanks to all that do.
 *
 *	Note:		I order the list in increasing performance of the
 *			"with registers" benchmark.  If the compiler doesn't
 *			provide register variables, then the benchmark
 *			is the same for both REG and NOREG.
 *
 *	PLEASE:		Send complete information about the machine type,
 *			clock speed, OS and C manufacturer/version.  If
 *			the machine is modified, tell me what was done.
 *			On UNIX, execute uname -a and cc -V to get this info.
 *
 *	80x8x NOTE:	80x8x benchers: please try to do all memory models
 *			for a particular compiler.
 *
 *
 *	The following program contains statements of a high-level programming
 *	language (C) in a distribution considered representative:
 *
 *	assignments			53%
 *	control statements		32%
 *	procedure, function calls	15%
 *
 *	100 statements are dynamically executed.  The program is balanced with
 *	respect to the three aspects:
 *		- statement type
 *		- operand type (for simple data types)
 *		- operand access
 *			operand global, local, parameter, or constant.
 *
 *	The combination of these three aspects is balanced only approximately.
 *
 *	The program does not compute anything meaningfull, but it is
 *	syntactically and semantically correct.
 *
 */
#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bchp_timer.h"
#include "dhrystone.h"
#include "sys_handles.h"

#define NUMBER_OF_RUNS	3000000		/* number of times to run */

/*#define STRCMP_MTI*/
#ifdef __cplusplus
extern "C" {
#endif
int strcmp_mti( char *, char *);
#ifdef __cplusplus
}
#endif

#if defined(METROWERKS)
/* Compile this file under MetroWerks 4.1 to get the best results */
#pragma optimization_level 4
#pragma defer_codegen on
#pragma auto_inline on
#endif

/* Global Variables: */

static Rec_Pointer     Ptr_Glob,
                Next_Ptr_Glob;
static int             Int_Glob;
static Boolean         Bool_Glob;
static char            Ch_1_Glob,
                Ch_2_Glob;
static int             Arr_1_Glob [50];
static int             Arr_2_Glob [50] [50];

static char Reg_Define[] = "Register option selected.";

/* extern void     *malloc (); 
Enumeration     Func_1 ();	*/
  /* forward declaration necessary since Enumeration may not simply be int */


#ifndef ROPT
#define REG
        /* REG becomes defined as empty */
        /* i.e. no register variables   */
#else
#define REG register
#endif

static void
Proc_1 (REG Rec_Pointer Ptr_Val_Par);
static void
Proc_2 (One_Fifty *Int_Par_Ref);
static void
Proc_3 (Rec_Pointer *Ptr_Ref_Par);
static void
Proc_4 (void);
static void
Proc_5 (void);
static void Proc_6 (Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par);
static void Proc_7 (One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty *Int_Par_Ref);
static void Proc_8 (Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref, int Int_1_Par_Val, int Int_2_Par_Val);
static Enumeration Func_1 (Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val);
static Boolean Func_2 (Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref);
static Boolean Func_3 (Enumeration Enum_Par_Val);

#ifdef  NOSTRUCTASSIGN
 void memcpy (register char *, register char *, register int);
#endif

/*
extern Boolean Func_2 (Str_30, Str_30);
extern void Proc_6 (Enumeration, Enumeration *);
extern void Proc_7 (One_Fifty, One_Fifty, One_Fifty *);
extern void Proc_8 (Arr_1_Dim, Arr_2_Dim, int, int);
*/

/* variables for time measurement: */

#define Too_Small_Time 2
                /* Measurements should last at least 2 seconds */

double          Begin_Time,
                End_Time,
                User_Time;

double          Microseconds,
                Dhrystones_Per_Second,
                Vax_Mips;

/* end of variables for time measurement */

/*
void
main (void)
*/
/*****/

  /* main program, corresponds to procedures        */
  /* Main and Proc_0 in the Ada version             */
void Dhrystone()  
{
/*  double   dtime();*/

        One_Fifty       Int_1_Loc;
  REG   One_Fifty       Int_2_Loc = 0;
        One_Fifty       Int_3_Loc;
  REG   char            Ch_Index;
        Enumeration     Enum_Loc;
        Str_30          Str_1_Loc;
        Str_30          Str_2_Loc;
  REG   int             Run_Index;
  REG   int             Number_Of_Runs;
	long			starttime, benchtime, nulltime;
	int i;
 #ifdef riscos
        FILE            *Ap;
 #endif

  BREG_Write32(bcmGetRegHandle(), BCHP_TIMER_TIMER2_CTRL, BCHP_TIMER_TIMER0_CTRL_ENA_MASK + BCHP_TIMER_TIMER0_CTRL_TIMEOUT_VAL_MASK);
	
  /* Initializations */
 #ifdef riscos
  if ((Ap = fopen("dhry/res","a+")) == NULL)
 #else
   /* BSYT if ((Ap = fopen("dhry.res","a+")) == NULL) */
   if (0)
 #endif
    {
       printf("Can not open dhry.res\n\n");
       return;
    }

  Next_Ptr_Glob = (Rec_Pointer) BKNI_Malloc (sizeof (Rec_Type));
  Ptr_Glob = (Rec_Pointer) BKNI_Malloc (sizeof (Rec_Type));

  Ptr_Glob->Ptr_Comp                    = Next_Ptr_Glob;
  Ptr_Glob->Discr                       = Ident_1;
  Ptr_Glob->variant.var_1.Enum_Comp     = Ident_3;
  Ptr_Glob->variant.var_1.Int_Comp      = 40;
  strcpy (Ptr_Glob->variant.var_1.Str_Comp,
          "DHRYSTONE PROGRAM, SOME STRING");
  strcpy (Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING");

  Arr_2_Glob [8][7] = 10;
        /* Was missing in published program. Without this statement,    */
        /* Arr_2_Glob [8][7] would have an undefined value.             */
        /* Warning: With 16-Bit processors and Number_Of_Runs > 32000,  */
        /* overflow may occur for this array element.                   */

  printf ("\n");
  printf ("Dhrystone Benchmark, Version 2.1 (Language: C)\n");
  printf ("\n");
/*
  if (Reg)
  {
    printf ("Program compiled with 'register' attribute\n");
    printf ("\n");
  }
  else
  {
    printf ("Program compiled without 'register' attribute\n");
    printf ("\n");
  }
*/
  Number_Of_Runs = NUMBER_OF_RUNS;
  printf ("Execution starts, %d runs through Dhrystone\n",Number_Of_Runs);

  starttime = BREG_Read32(bcmGetRegHandle(), BCHP_TIMER_TIMER2_CTRL);

  for (i = 0; i < Number_Of_Runs; ++i);
  nulltime = BREG_Read32(bcmGetRegHandle(), BCHP_TIMER_TIMER2_CTRL) - starttime; /* Computes o'head of loop */

  /***************/
  /* Start timer */
  /***************/

  /* Begin_Time = dtime(); */
  starttime = BREG_Read32(bcmGetRegHandle(), BCHP_TIMER_TIMER2_CTRL);
  
  for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index)
  {

    Proc_5();
    Proc_4();
      /* Ch_1_Glob == 'A', Ch_2_Glob == 'B', Bool_Glob == true */
    Int_1_Loc = 2;
    Int_2_Loc = 3;
    strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
    Enum_Loc = Ident_2;
    Bool_Glob = ! Func_2 (Str_1_Loc, Str_2_Loc);
      /* Bool_Glob == 1 */
    while (Int_1_Loc < Int_2_Loc)  /* loop body executed once */
    {
      Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
        /* Int_3_Loc == 7 */
      Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
        /* Int_3_Loc == 7 */
      Int_1_Loc += 1;
    } /* while */
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Proc_8 (Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
      /* Int_Glob == 5 */
    Proc_1 (Ptr_Glob);
    for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index)
                             /* loop body executed twice */
    {
      if (Enum_Loc == Func_1 (Ch_Index, 'C'))
          /* then, not executed */
        {
        Proc_6 (Ident_1, &Enum_Loc);
        strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
        Int_2_Loc = Run_Index;
        Int_Glob = Run_Index;
        }
    }
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Int_2_Loc = Int_2_Loc * Int_1_Loc;
    Int_1_Loc = Int_2_Loc / Int_3_Loc;
    Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
      /* Int_1_Loc == 1, Int_2_Loc == 13, Int_3_Loc == 7 */
    Proc_2 (&Int_1_Loc);
      /* Int_1_Loc == 5 */

  } /* loop "for Run_Index" */

  /**************/
  /* Stop timer */
  /**************/

  /* End_Time = dtime(); */
benchtime = BREG_Read32(bcmGetRegHandle(), BCHP_TIMER_TIMER2_CTRL);
BREG_Write32(bcmGetRegHandle(), BCHP_TIMER_TIMER2_CTRL, 0);

  printf ("Execution ends\n");
  printf ("\n");
  printf ("Final values of the variables used in the benchmark:\n");
  printf ("\n");
  printf ("Int_Glob:            %d\n", Int_Glob);
  printf ("        should be:   %d\n", 5);
  printf ("Bool_Glob:           %d\n", Bool_Glob);
  printf ("        should be:   %d\n", 1);
  printf ("Ch_1_Glob:           %c\n", Ch_1_Glob);
  printf ("        should be:   %c\n", 'A');
  printf ("Ch_2_Glob:           %c\n", Ch_2_Glob);
  printf ("        should be:   %c\n", 'B');
  printf ("Arr_1_Glob[8]:       %d\n", Arr_1_Glob[8]);
  printf ("        should be:   %d\n", 7);
  printf ("Arr_2_Glob[8][7]:    %d\n", Arr_2_Glob[8][7]);
  printf ("        should be:   Number_Of_Runs + 10\n");
  printf ("Ptr_Glob->\n");
  printf ("  Ptr_Comp:          %d\n", (int) Ptr_Glob->Ptr_Comp);
  printf ("        should be:   (implementation-dependent)\n");
  printf ("  Discr:             %d\n", Ptr_Glob->Discr);
  printf ("        should be:   %d\n", 0);
  printf ("  Enum_Comp:         %d\n", Ptr_Glob->variant.var_1.Enum_Comp);
  printf ("        should be:   %d\n", 2);
  printf ("  Int_Comp:          %d\n", Ptr_Glob->variant.var_1.Int_Comp);
  printf ("        should be:   %d\n", 17);
  printf ("  Str_Comp:          %s\n", Ptr_Glob->variant.var_1.Str_Comp);
  printf ("        should be:   DHRYSTONE PROGRAM, SOME STRING\n");
  printf ("Next_Ptr_Glob->\n");
  printf ("  Ptr_Comp:          %d\n", (int) Next_Ptr_Glob->Ptr_Comp);
  printf ("        should be:   (implementation-dependent), same as above\n");
  printf ("  Discr:             %d\n", Next_Ptr_Glob->Discr);
  printf ("        should be:   %d\n", 0);
  printf ("  Enum_Comp:         %d\n", Next_Ptr_Glob->variant.var_1.Enum_Comp);
  printf ("        should be:   %d\n", 1);
  printf ("  Int_Comp:          %d\n", Next_Ptr_Glob->variant.var_1.Int_Comp);
  printf ("        should be:   %d\n", 18);
  printf ("  Str_Comp:          %s\n", Next_Ptr_Glob->variant.var_1.Str_Comp);
  printf ("        should be:   DHRYSTONE PROGRAM, SOME STRING\n");
  printf ("Int_1_Loc:           %d\n", Int_1_Loc);
  printf ("        should be:   %d\n", 5);
  printf ("Int_2_Loc:           %d\n", Int_2_Loc);
  printf ("        should be:   %d\n", 13);
  printf ("Int_3_Loc:           %d\n", Int_3_Loc);
  printf ("        should be:   %d\n", 7);
  printf ("Enum_Loc:            %d\n", Enum_Loc);
  printf ("        should be:   %d\n", 1);
  printf ("Str_1_Loc:           %s\n", Str_1_Loc);
  printf ("        should be:   DHRYSTONE PROGRAM, 1'ST STRING\n");
  printf ("Str_2_Loc:           %s\n", Str_2_Loc);
  printf ("        should be:   DHRYSTONE PROGRAM, 2'ND STRING\n");
  printf ("\n");

  /* User_Time = End_Time - Begin_Time; */
  User_Time = ((double) ((double)(benchtime - starttime - nulltime)) / (double) 27000000.0);
  if (User_Time < Too_Small_Time)
  {
    printf ("Measured time too small to obtain meaningful results\n");
    printf ("Please increase number of runs\n");
    printf ("\n");
printf( "User_Time=%f, benchtime=%ld, starttime=%ld, nulltime=%ld\n", User_Time, benchtime, starttime, nulltime);
  }
  else
  {

    Microseconds = User_Time * Mic_secs_Per_Second
                        / (double) Number_Of_Runs;
    Dhrystones_Per_Second = (double) Number_Of_Runs / User_Time;
    Vax_Mips = Dhrystones_Per_Second / 1757.0;

#ifdef ROPT
    printf ("Register option selected?  YES\n");
#else
    printf ("Register option selected?  NO\n");
    strcpy(Reg_Define, "Register option not selected.");
#endif
    printf ("Microseconds for one run through Dhrystone: ");
    printf ("%7.1f \n", Microseconds);
    printf ("Dhrystones per Second:                      ");
    printf ("%10.1f \n", Dhrystones_Per_Second);
    printf ("VAX MIPS rating = %10.3f \n",Vax_Mips);
    printf ("\n");

/*
  fprintf(Ap,"\n");
  fprintf(Ap,"Dhrystone Benchmark, Version 2.1 (Language: C)\n");
  fprintf(Ap,"%s\n",Reg_Define);
  fprintf(Ap,"Microseconds for one loop: %7.1f\n",Microseconds);
  fprintf(Ap,"Dhrystones per second: %10.1f\n",Dhrystones_Per_Second);
  fprintf(Ap,"VAX MIPS rating: %10.3f\n",Vax_Mips);
  fclose(Ap);
*/  
  printf("\n");
  printf("Dhrystone Benchmark, Version 2.1 (Language: C)\n");
  printf("%s\n",Reg_Define);
  printf("Microseconds for one loop: %7.1f\n",Microseconds);
  printf("Dhrystones per second: %10.1f\n",Dhrystones_Per_Second);
  printf("VAX MIPS rating: %10.3f\n",Vax_Mips);

  }

}


static void
Proc_1 (REG Rec_Pointer Ptr_Val_Par)
    /* executed once */
{
  REG Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;
                                        /* == Ptr_Glob_Next */
  /* Local variable, initialized with Ptr_Val_Par->Ptr_Comp,    */
  /* corresponds to "rename" in Ada, "with" in Pascal           */

  structassign (*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
  Ptr_Val_Par->variant.var_1.Int_Comp = 5;
  Next_Record->variant.var_1.Int_Comp
        = Ptr_Val_Par->variant.var_1.Int_Comp;
  Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
  Proc_3 (&Next_Record->Ptr_Comp);
    /* Ptr_Val_Par->Ptr_Comp->Ptr_Comp
                        == Ptr_Glob->Ptr_Comp */
  if (Next_Record->Discr == Ident_1)
    /* then, executed */
  {
    Next_Record->variant.var_1.Int_Comp = 6;
    Proc_6 (Ptr_Val_Par->variant.var_1.Enum_Comp,
           &Next_Record->variant.var_1.Enum_Comp);
    Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
    Proc_7 (Next_Record->variant.var_1.Int_Comp, 10,
           &Next_Record->variant.var_1.Int_Comp);
  }
  else /* not executed */
    structassign (*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
} /* Proc_1 */


static void
Proc_2 (One_Fifty *Int_Par_Ref)
/******************/
    /* executed once */
    /* *Int_Par_Ref == 1, becomes 4 */
{
  One_Fifty  Int_Loc;
  Enumeration   Enum_Loc = Ident_1;

  Int_Loc = *Int_Par_Ref + 10;
  do /* executed once */
    if (Ch_1_Glob == 'A')
      /* then, executed */
    {
      Int_Loc -= 1;
      *Int_Par_Ref = Int_Loc - Int_Glob;
      Enum_Loc = Ident_1;
    } /* if */
  while (Enum_Loc != Ident_1); /* true */
} /* Proc_2 */


static void
Proc_3 (Rec_Pointer *Ptr_Ref_Par)
/******************/
    /* executed once */
    /* Ptr_Ref_Par becomes Ptr_Glob */
{
  if (Ptr_Glob != Null)
    /* then, executed */
    *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
  Proc_7 (10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
} /* Proc_3 */


static void
Proc_4 (void) /* without parameters */
/*******/
    /* executed once */
{
  Boolean Bool_Loc;

  Bool_Loc = Ch_1_Glob == 'A';
  Bool_Glob = Bool_Loc | Bool_Glob;
  Ch_2_Glob = 'B';
} /* Proc_4 */


static void
Proc_5 (void) 	/* without parameters */
		/* executed once */
{
  Ch_1_Glob = 'A';
  Bool_Glob = false;
} /* Proc_5 */


void Proc_6 (Enumeration, Enumeration *);
void Proc_7 (One_Fifty, One_Fifty, One_Fifty *);
void Proc_8 (Arr_1_Dim, Arr_2_Dim, int, int);
Enumeration Func_1 (Capital_Letter, Capital_Letter);
Boolean Func_2 (Str_30, Str_30);
Boolean Func_3 (Enumeration);


static void Proc_6 (Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par)
/*********************************/
    /* executed once */
    /* Enum_Val_Par == Ident_3, Enum_Ref_Par becomes Ident_2 */
{
  *Enum_Ref_Par = Enum_Val_Par;
  if (! Func_3 (Enum_Val_Par))
    /* then, not executed */
    *Enum_Ref_Par = Ident_4;
  switch (Enum_Val_Par)
  {
    case Ident_1:
      *Enum_Ref_Par = Ident_1;
      break;
    case Ident_2:
      if (Int_Glob > 100)
        /* then */
      *Enum_Ref_Par = Ident_1;
      else *Enum_Ref_Par = Ident_4;
      break;
    case Ident_3: /* executed */
      *Enum_Ref_Par = Ident_2;
      break;
    case Ident_4: break;
    case Ident_5:
      *Enum_Ref_Par = Ident_3;
      break;
  } /* switch */
} /* Proc_6 */


static void Proc_7 (One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty *Int_Par_Ref)
/**********************************************/
    /* executed three times                                      */
    /* first call:      Int_1_Par_Val == 2, Int_2_Par_Val == 3,  */
    /*                  Int_Par_Ref becomes 7                    */
    /* second call:     Int_1_Par_Val == 10, Int_2_Par_Val == 5, */
    /*                  Int_Par_Ref becomes 17                   */
    /* third call:      Int_1_Par_Val == 6, Int_2_Par_Val == 10, */
    /*                  Int_Par_Ref becomes 18                   */
{
  One_Fifty Int_Loc;

  Int_Loc = Int_1_Par_Val + 2;
  *Int_Par_Ref = Int_2_Par_Val + Int_Loc;
} /* Proc_7 */


static void Proc_8 (Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref, int Int_1_Par_Val, int Int_2_Par_Val)
/*********************************************************************/
    /* executed once      */
    /* Int_Par_Val_1 == 3 */
    /* Int_Par_Val_2 == 7 */
{
  REG One_Fifty Int_Index;
  REG One_Fifty Int_Loc;

  Int_Loc = Int_1_Par_Val + 5;
  Arr_1_Par_Ref [Int_Loc] = Int_2_Par_Val;
  Arr_1_Par_Ref [Int_Loc+1] = Arr_1_Par_Ref [Int_Loc];
  Arr_1_Par_Ref [Int_Loc+30] = Int_Loc;
  for (Int_Index = Int_Loc; Int_Index <= Int_Loc+1; ++Int_Index)
    Arr_2_Par_Ref [Int_Loc] [Int_Index] = Int_Loc;
  Arr_2_Par_Ref [Int_Loc] [Int_Loc-1] += 1;
  Arr_2_Par_Ref [Int_Loc+20] [Int_Loc] = Arr_1_Par_Ref [Int_Loc];
  Int_Glob = 5;
} /* Proc_8 */


static Enumeration Func_1 (Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val)
/*************************************************/
    /* executed three times                                         */
    /* first call:      Ch_1_Par_Val == 'H', Ch_2_Par_Val == 'R'    */
    /* second call:     Ch_1_Par_Val == 'A', Ch_2_Par_Val == 'C'    */
    /* third call:      Ch_1_Par_Val == 'B', Ch_2_Par_Val == 'C'    */
{
  Capital_Letter        Ch_1_Loc;
  Capital_Letter        Ch_2_Loc;

  Ch_1_Loc = Ch_1_Par_Val;
  Ch_2_Loc = Ch_1_Loc;
  if (Ch_2_Loc != Ch_2_Par_Val)
    /* then, executed */
    return (Ident_1);
  else  /* not executed */
  {
    Ch_1_Glob = Ch_1_Loc;
    return (Ident_2);
   }
} /* Func_1 */


static Boolean Func_2 (Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref)
/*************************************************/
    /* executed once */
    /* Str_1_Par_Ref == "DHRYSTONE PROGRAM, 1'ST STRING" */
    /* Str_2_Par_Ref == "DHRYSTONE PROGRAM, 2'ND STRING" */
{
  REG One_Thirty        Int_Loc;
      Capital_Letter    Ch_Loc = 0;

  Int_Loc = 2;
  while (Int_Loc <= 2) /* loop body executed once */
    if (Func_1 (Str_1_Par_Ref[Int_Loc],
                Str_2_Par_Ref[Int_Loc+1]) == Ident_1)
      /* then, executed */
    {
      Ch_Loc = 'A';
      Int_Loc += 1;
    } /* if, while */
  if (Ch_Loc >= 'W' && Ch_Loc < 'Z')
    /* then, not executed */
    Int_Loc = 7;
  if (Ch_Loc == 'R')
    /* then, not executed */
    return (true);
  else /* executed */
  {
#ifdef	STRCMP_MTI
    if (strcmp_mti (Str_1_Par_Ref, Str_2_Par_Ref) > 0)
#else
    if (strcmp (Str_1_Par_Ref, Str_2_Par_Ref) > 0)
#endif
      /* then, not executed */
    {
      Int_Loc += 7;
      Int_Glob = Int_Loc;
      return (true);
    }
    else /* executed */
      return (false);
  } /* if Ch_Loc */
} /* Func_2 */


static Boolean Func_3 (Enumeration Enum_Par_Val)
/***************************/
    /* executed once        */
    /* Enum_Par_Val == Ident_3 */
{
  Enumeration Enum_Loc;

  Enum_Loc = Enum_Par_Val;
  if (Enum_Loc == Ident_3)
    /* then, executed */
    return (true);
  else /* not executed */
    return (false);
} /* Func_3 */


        /* Procedure for the assignment of structures,          */
        /* if the C compiler doesn't support this feature       */
#ifdef  NOSTRUCTASSIGN
void
memcpy (register char *d, register char *s, register int l)
/* register char   *d; register char   *s; register int    l; */
{
        while (l--) *d++ = *s++;
}
#endif

