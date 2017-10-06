#include <dsp_lib/dsp_functions.h>
#include <dsp_lib/dsp_lib.h>
#include <dsp_lib/signed_magnitude.h>
#include <dsp_lib/signed_mantissa.h>
#include <dsp_lib/complex_float_type.h>
using namespace std;

#define DUMP(x)
//#define DUMP(x) std::cout << x

template<typename MANTISSA_TYPE,int MBITS,int EBITS> 
	complex_ce_float<MANTISSA_TYPE,MBITS,EBITS> 
	rtl_pu(
		   int start, int clr, int cld,int cycle,
		   int sgn, int sfl, 
		   complex_ce_float<MANTISSA_TYPE,MBITS,EBITS> AIn,
		   complex_ce_float<MANTISSA_TYPE,MBITS,EBITS> BIn,
		   complex_ce_float<MANTISSA_TYPE,MBITS,EBITS> CIn
		   )
{
    const int PU_EXAMPLE_NUM_CYCLES=4;
	complex_ce_float<MANTISSA_TYPE,MBITS,EBITS> SLoc;
	complex_ce_float<MANTISSA_TYPE,MBITS,EBITS> Out;
	
	complex_ce_float<MANTISSA_TYPE,2*MBITS-1,EBITS+1> TLoc;
	static complex_ce_float<MANTISSA_TYPE,2*MBITS-1,EBITS+1> XSLoc;
	static complex_ce_float<MANTISSA_TYPE,MBITS,EBITS> OReg[PU_EXAMPLE_NUM_CYCLES];

	DUMP( "AIn = " << AIn << " ");
	DUMP( "BIn = " << BIn << " ");
	DUMP( "CIn = " << CIn << "\n");
	
	if(start) {
		if(cld==1) {
			// Getting a value from C
			SLoc.setVal(0);
			SLoc += CIn;
			// since SLoc is originally 0
			SLoc.set_signbits(CIn.get_signbits());
		}
		else if(cld==3) {
			SLoc.setVal(0);
			SLoc += OReg[cycle];
			// since SLoc is originally 0
			SLoc.set_signbits(OReg[cycle].get_signbits());
		}
		else {
			SLoc=0;
		}
	}

	DUMP( "First SLoc = " << SLoc << "\n");

	// multiplication
	TLoc= AIn*BIn;
	DUMP( "TLoc before align = " << TLoc << "\n");

	TLoc.align();

	DUMP( "TLoc = " << TLoc << "\n");

	// Accumulation.
	SLoc += TLoc;

	DUMP( "Accumed SLoc = " << SLoc << "\n");

	SLoc.align();

	DUMP( "Aligned SLoc = " << SLoc << "\n");

	if(clr) {
		OReg[cycle] = 0;
	} else {
		OReg[cycle]=alignednorm(SLoc);
		Out = OReg[cycle];
	}

	DUMP( "Out = " << Out << "\n");

	return(Out);
}
