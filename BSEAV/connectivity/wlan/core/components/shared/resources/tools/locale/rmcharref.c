/*
 * Copyright 2001 Unicode, Inc.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "getopt.h"

	
typedef unsigned long	UTF32;	/* at least 32 bits */
typedef unsigned short	UTF16;	/* at least 16 bits */
typedef unsigned char	UTF8;	/* typically 8 bits */
typedef unsigned char	Boolean; /* 0 or 1 */

static const int halfShift	= 10; /* used for shifting by 10 bits */

static const UTF32 halfBase	= 0x0010000UL;
static const UTF32 halfMask	= 0x3FFUL;

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_SUR_HIGH_START	(UTF32)0xD800
#define UNI_SUR_HIGH_END	(UTF32)0xDBFF
#define UNI_SUR_LOW_START	(UTF32)0xDC00
#define UNI_SUR_LOW_END		(UTF32)0xDFFF
#define false			0
#define true			1

typedef enum {
	conversionOK, 		/* conversion successful */
	sourceExhausted,	/* partial character in source, but hit end */
	targetExhausted,	/* insuff. room in target for conversion */
	sourceIllegal,		/* source sequence is illegal/malformed */
	noMemory,
} ConversionResult;

typedef enum {
	strictConversion = 0,
	lenientConversion
} ConversionFlags;

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 */
static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

static const	char *charlist1[96] =
	{
	"nbsp",
	"iexcl",
	"cent",
	"pound",
	"curren",
	"yen",
	"brvbar",
	"sect",
	"uml",
	"copy",
	"ordf",
	"laquo",
	"not",
	"shy",
	"reg",
	"macr",
	"deg",
	"plusmn",
	"sup2",
	"sup3",
	"acute",
	"micro",
	"para",
	"middot",
	"cedil",
	"sup1",
	"ordm",
	"raquo",
	"frac14",
	"frac12",
	"frac34",
	"iquest",
	"Agrave",
	"Aacute",
	"Acirc",
	"Atilde",
	"Auml",
	"Aring",
	"AElig",
	"Ccedil",
	"Egrave",
	"Eacute",
	"Ecirc",
	"Euml",
	"Igrave",
	"Iacute",
	"Icirc",
	"Iuml",
	"ETH",
	"Ntilde",
	"Ograve",
	"Oacute",
	"Ocirc",
	"Otilde",
	"Ouml",
	"times",
	"Oslash",
	"Ugrave",
	"Uacute",
	"Ucirc",
	"Uuml",
	"Yacute",
	"THORN",
	"szlig",
	"agrave",
	"aacute",
	"acirc",
	"atilde",
	"auml",
	"aring",
	"aelig",
	"ccedil",
	"egrave",
	"eacute",
	"ecirc",
	"euml",
	"igrave",
	"iacute",
	"icirc",
	"iuml",
	"eth",
	"ntilde",
	"ograve",
	"oacute",
	"ocirc",
	"otilde",
	"ouml",
	"divide",
	"oslash",
	"ugrave",
	"uacute",
	"ucirc",
	"uuml",
	"yacute",
	"thorn",
	"yuml"
	};

static const	struct
	{ UTF32 code; char *symbol; } charlist2[] =
	{
	{402,	"fnof"},
	{913,	"Alpha"},
	{914,	"Beta"},
	{915,	"Gamma"},
	{916,	"Delta"},
	{917,	"Epsilon"},
	{918,	"Zeta"},
	{919,	"Eta"},
	{920,	"Theta"},
	{921,	"Iota"},
	{922,	"Kappa"},
	{923,	"Lambda"},
	{924,	"Mu"},
	{925,	"Nu"},
	{926,	"Xi"},
	{927,	"Omicron"},
	{928,	"Pi"},
	{929,	"Rho"},
	{931,	"Sigma"},
	{932,	"Tau"},
	{933,	"Upsilon"},
	{934,	"Phi"},
	{935,	"Chi"},
	{936,	"Psi"},
	{937,	"Omega"},
	{945,	"alpha"},
	{946,	"beta"},
	{947,	"gamma"},
	{948,	"delta"},
	{949,	"epsilon"},
	{950,	"zeta"},
	{951,	"eta"},
	{952,	"theta"},
	{953,	"iota"},
	{954,	"kappa"},
	{955,	"lambda"},
	{956,	"mu"},
	{957,	"nu"},
	{958,	"xi"},
	{959,	"omicron"},
	{960,	"pi"},
	{961,	"rho"},
	{962,	"sigmaf"},
	{963,	"sigma"},
	{964,	"tau"},
	{965,	"upsilon"},
	{966,	"phi"},
	{967,	"chi"},
	{968,	"psi"},
	{969,	"omega"},
	{977,	"thetasym"},
	{978,	"upsih"},
	{982,	"piv"},
	{8226,	"bull"},
	{8230,	"hellip"},
	{8242,	"prime"},
	{8243,	"Prime"},
	{8254,	"oline"},
	{8260,	"frasl"},
	{8472,	"weierp"},
	{8465,	"image"},
	{8476,	"real"},
	{8482,	"trade"},
	{8501,	"alefsym"},
	{8592,	"larr"},
	{8593,	"uarr"},
	{8594,	"rarr"},
	{8595,	"darr"},
	{8596,	"harr"},
	{8629,	"crarr"},
	{8656,	"lArr"},
	{8657,	"uArr"},
	{8658,	"rArr"},
	{8659,	"dArr"},
	{8660,	"hArr"},
	{8704,	"forall"},
	{8706,	"part"},
	{8707,	"exist"},
	{8709,	"empty"},
	{8711,	"nabla"},
	{8712,	"isin"},
	{8713,	"notin"},
	{8715,	"ni"},
	{8719,	"prod"},
	{8721,	"sum"},
	{8722,	"minus"},
	{8727,	"lowast"},
	{8730,	"radic"},
	{8733,	"prop"},
	{8734,	"infin"},
	{8736,	"ang"},
	{8743,	"and"},
	{8744,	"or"},
	{8745,	"cap"},
	{8746,	"cup"},
	{8747,	"int"},
	{8756,	"there4"},
	{8764,	"sim"},
	{8773,	"cong"},
	{8776,	"asymp"},
	{8800,	"ne"},
	{8801,	"equiv"},
	{8804,	"le"},
	{8805,	"ge"},
	{8834,	"sub"},
	{8835,	"sup"},
	{8836,	"nsub"},
	{8838,	"sube"},
	{8839,	"supe"},
	{8853,	"oplus"},
	{8855,	"otimes"},
	{8869,	"perp"},
	{8901,	"sdot"},
	{8968,	"lceil"},
	{8969,	"rceil"},
	{8970,	"lfloor"},
	{8971,	"rfloor"},
	{9001,	"lang"},
	{9002,	"rang"},
	{9674,	"loz"},
	{9824,	"spades"},
	{9827,	"clubs"},
	{9829,	"hearts"},
	{9830,	"diams"},
	{338,	"OElig"},
	{339,	"oelig"},
	{352,	"Scaron"},
	{353,	"scaron"},
	{376,	"Yuml"},
	{710,	"circ"},
	{732,	"tilde"},
	{8194,	"ensp"},
	{8195,	"emsp"},
	{8201,	"thinsp"},
	{8204,	"zwnj"},
	{8205,	"zwj"},
	{8206,	"lrm"},
	{8207,	"rlm"},
	{8211,	"ndash"},
	{8212,	"mdash"},
	{8216,	"lsquo"},
	{8217,	"rsquo"},
	{8218,	"sbquo"},
	{8220,	"ldquo"},
	{8221,	"rdquo"},
	{8222,	"bdquo"},
	{8224,	"dagger"},
	{8225,	"Dagger"},
	{8240,	"permil"},
	{8249,	"lsaquo"},
	{8250,	"rsaquo"},
	{8364,	"euro"}
	};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static const UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
					 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... six byte sequence.)
 */
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */

/* --------------------------------------------------------------------- */


static void
Usage(char *name)
	{
	fprintf(stderr, "Usage: %s [-i <input file>] [-o <output file>]\n", name);
	exit(1);
	}

static void 
DisplayConversionStatus(ConversionResult result)
	{
	switch (result)
		{
		default:
		case conversionOK:
		case sourceExhausted:
		case targetExhausted:
			break;
		case sourceIllegal:
			fprintf(stderr, "Illegal source code\n");
			break;
		}
	}

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *	length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

static Boolean isLegalUTF8(const UTF8 *source, int length) {
	UTF8 a;
	const UTF8 *srcptr = source+length;
	switch (length) {
	default: return false;
		/* Everything else falls through when "true"... */
	case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 2: if ((a = (*--srcptr)) > 0xBF) return false;
		switch (*source) {
		    /* no fall-through in this inner switch */
		    case 0xE0: if (a < 0xA0) return false; break;
		    case 0xF0: if (a < 0x90) return false; break;
		    case 0xF4: if (a > 0x8F) return false; break;
		    default:  if (a < 0x80) return false;
		}
    	case 1: if (*source >= 0x80 && *source < 0xC2) return false;
		if (*source > 0xF4) return false;
	}
	return true;
}

ConversionResult ConvertUTF8toUTF16 (FILE *ifile, FILE *ofile, ConversionFlags flags)
	{
	ConversionResult result = conversionOK;
	UTF8	srcBuffer[16];
	UTF8* source;
	unsigned short extraBytesToRead;
	
	while (1)
		{
		UTF32 ch, ch1;
		
		ch = 0;
		source = srcBuffer;
		if (fread( source, 1, 1, ifile) != 1)
			{
			result = conversionOK; 
			break;
			}
		if ((extraBytesToRead = trailingBytesForUTF8[*source]))
			if (fread( source+1, 1, extraBytesToRead, ifile) != extraBytesToRead)
				{
				result = sourceExhausted; 
				break;
				}
		
		/* Do this check whether lenient or strict */
		if (! isLegalUTF8(source, extraBytesToRead+1))
			{
			result = sourceIllegal;
			break;
			}
		/*
		 * The cases all fall through. See "Note A" below.
		 */
		switch (extraBytesToRead) {
			case 3:	ch += *source++; ch <<= 6;
			case 2:	ch += *source++; ch <<= 6;
			case 1:	ch += *source++; ch <<= 6;
			case 0:	ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (ch <= UNI_MAX_BMP)
			{ /* Target is a character <= 0xFFFF */
			if ((flags == strictConversion) && (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END))
				{
				result = sourceIllegal;
				break;
				} 
			else
				{
				if(fwrite( &ch, 2, 1, ofile) != 1)
					{
					result = targetExhausted;
					break;
					}
				}
			} 
		else if (ch > UNI_MAX_UTF16)
			{
			if (flags == strictConversion)
				{
				result = sourceIllegal;
				break; /* Bail out; shouldn't continue */
				} 
			else
				{
				ch = UNI_REPLACEMENT_CHAR;
				if(fwrite( &ch, 2, 1, ofile) != 1)
					{
					result = targetExhausted;
					break;
					}
				}
			} 
		else
			{
			/* target is a character in range 0xFFFF - 0x10FFFF. */
			ch -= halfBase;
			ch1 = (ch >> halfShift) + UNI_SUR_HIGH_START;
			if(fwrite( &ch1, 2, 1, ofile) != 1)
				{
				result = targetExhausted;
				break;
				}
			ch1 = (ch & halfMask) + UNI_SUR_LOW_START;
			if(fwrite( &ch1, 2, 1, ofile) != 1)
				{
				result = targetExhausted;
				break;
				}
			}
	}
	return result;
	}

ConversionResult Convert (FILE *ifile, FILE *ofile, ConversionFlags flags)
	{
	ConversionResult result = conversionOK;
	UTF16	srcBuffer[2];
	UTF8	dstBuffer[16];
	UTF8* 	target;
	char	*symbPtr, symb, symbBuffer[32];
	Boolean	passthrough = true;
	Boolean	secondUnichar = false;

	while (1) 
		{
		UTF32 ch, ch2;
		unsigned short bytesToWrite = 0;
		const UTF32 byteMask = 0xBF;
		const UTF32 byteMark = 0x80; 

		if (fread(&symb, 1, 1, ifile) != 1)
			{
			result = conversionOK; 
			break;
			}
		if (passthrough)
			{
			if (symb != '&')
				{
				if (secondUnichar)
					{
					result = sourceIllegal;
					break;
					}
				if(fwrite( &symb, 1, 1, ofile) != 1)
					{
					result = targetExhausted;
					break;
					}
				}
			else
				{
				passthrough=false;
				symbPtr = symbBuffer;
				*symbPtr++ = symb;
				}			
			}
		else
			{
			int	nBytes, i;
			Boolean	dumpBuffer = false;
			
			*symbPtr++ = symb;
			nBytes = symbPtr - symbBuffer;
			if (symb == ';')
				{
				passthrough = true;
				if (symbBuffer[1] == '#')
					{
					for (i = 2; i < nBytes - 1; i++)
						{
						if ((symbBuffer[i] < '0') || (symbBuffer[i] > '9'))
							{
							dumpBuffer = true;
							break;
							}
						}
					if (!dumpBuffer)
						{
						if (!secondUnichar)
							{
							ch = atoi(&symbBuffer[2]);
							if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END)
								secondUnichar = true;
							else if ((flags == strictConversion) && (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END))
								{
								result = sourceIllegal;
								break;
								}
							}
						else
							{
							ch2 = atoi(&symbBuffer[2]);
							if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END)
								{
								ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
									+ (ch2 - UNI_SUR_LOW_START) + halfBase;
								}
							else if (flags == strictConversion)
								{ /* it's an unpaired high surrogate */
								result = sourceIllegal;
								break;
								}
							secondUnichar = false;
							}
						}
					}
				else
					{
					int i;
					Boolean found;
					found = false;
					for (i = 0; i < sizeof(charlist1)/sizeof(charlist1[0]); i++)
						{
						if (strncmp(symbBuffer+1, charlist1[i], nBytes-2) == 0)
							{
							ch = 160 + i;
							found = true;
							break;
							}
						}
					if (!found)
						{
						for (i = 0; i < sizeof(charlist2)/sizeof(charlist2[0]); i++)
							{
							if (strncmp(symbBuffer+1, charlist2[i].symbol, nBytes-2) == 0)
								{
								ch = charlist2[i].code;
								found = true;
								break;
								}
							}
						}
					if (!found)
						{
						dumpBuffer = true;
						}
					}

				if (!dumpBuffer)
					{
					if (!secondUnichar)
						{
						/* Figure out how many bytes the result will require */
						if (ch < (UTF32)0x80)
							{			
							bytesToWrite = 1;
							}
						else if (ch < (UTF32)0x800)
							{
							bytesToWrite = 2;
							}
						else if (ch < (UTF32)0x10000)
							{
							bytesToWrite = 3;
							}
						else if (ch < (UTF32)0x200000)
							{
							bytesToWrite = 4;
							}
						else
							{				
							bytesToWrite = 2;
							ch = UNI_REPLACEMENT_CHAR;
							}

						target = dstBuffer + bytesToWrite;
						switch (bytesToWrite)
							{	/* note: everything falls through. */
							case 4:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
							case 3:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
							case 2:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
							case 1:	*--target =  ch | firstByteMark[bytesToWrite];
							}
						if(fwrite( dstBuffer, 1, bytesToWrite, ofile) != bytesToWrite)
							{
							result = targetExhausted;
							break;
							}
						}
					}
				}
			if (dumpBuffer || (nBytes >= 32))
				{
				passthrough = true;
				if(fwrite( symbBuffer, 1, nBytes, ofile) != nBytes)
					{
					result = targetExhausted;
					break;
					}
				}
			
			}
		
		
		ch = srcBuffer[0];
		}
	return result;
	}

int main( int argc, char** argv )
	{
	FILE		*ifile, *ofile;
	int			opt;

	ifile = stdin;
	ofile = stdout;

	while ((opt = getopt(argc, argv, "?i:o:")) != EOF)
		{
		switch (opt)
			{
			case 'i':
				ifile = fopen(optarg, "r");
				if (!ifile)
					{
					fprintf(stderr, "%s: %s %s\n", argv[0], optarg, strerror(errno));
					exit(1);
					}
				break;
			case 'o':
				ofile = fopen(optarg, "w");
				if (!ofile)
					{
					fprintf(stderr, "%s: %s %s\n", argv[0], optarg, strerror(errno));
					exit(1);
					}
				break;
			default:
				Usage(argv[0]);
			}
		}
	DisplayConversionStatus(Convert (ifile, ofile, strictConversion));

	return 0;
	}
	
