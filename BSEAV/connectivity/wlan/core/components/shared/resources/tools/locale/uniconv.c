/*
 * Copyright 2001 Unicode, Inc.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "getopt.h"

#include <fcntl.h>
#include <io.h>

#include <windows.h>

typedef enum
	{
	unknown,
	utf8,
	utf16,
	mb
	} encoding;
	
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

static struct
	{
	encoding	code;
	char		*name;
	} encodingNames[] = 
	{
		{ unknown, "unknown" },
		{ utf8, "utf8" },
		{ utf16, "utf16" },
		{ mb, "mb" }
	} ;

unsigned int inBufSize, outBufSize;
char *inBuf, *outBuf;
static int line;
static int html;
static int err_count;

static void
Usage(char *name)
	{
	fprintf(stderr, "Usage: %s -s <utf8 | utf16 | mb> -d <utf8 | utf16 | mb> [-p <codepage>] [-h] [-i <input file>] [-o <output file>]\n", name);
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

static encoding
GetEncodingArg(char *arg)
	{
	encoding x, rc = unknown;

	for (x = utf8; x <= mb; x++)
		{
		if (strcmp(arg, encodingNames[x].name) == 0)
			{
			rc = encodingNames[x].code;
			break;
			}
		}
	return rc;
	}

static int
ValidateCodePage(int codepage)
	{
	switch( codepage )
		{
		/* MultiByte Character Sets */
		case 932: /* Japanese Shift-JIS */
		case 936: /* Chinese Simplified GBK */
		case 949: /* Korean */
		case 950: /* Chinese Traditional Big5 */

			/* Single Byte Character Sets */
		case 874: /* Thai */
		case 1250: /* Central Europe */
		case 1251: /* Cyrillic */
		case 1252: /* Latin I */
		case 1253: /* Greek */
		case 1254: /* Turkish */
		case 1255: /* Hebrew */
		case 1256: /* Arabic */
		case 1257: /* Baltic */
			break;
		default:
			return 0;
		}
	return 1;
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

ConversionResult ConvertUTF16toUTF8 (FILE *ifile, FILE *ofile, ConversionFlags flags)
	{
	ConversionResult result = conversionOK;
	UTF16	srcBuffer[2];
	UTF8	dstBuffer[16];
	UTF8* 	target;

	while (1) 
		{
		UTF32 ch;
		unsigned short bytesToWrite = 0;
		const UTF32 byteMask = 0xBF;
		const UTF32 byteMark = 0x80; 

		if (fread( srcBuffer, 2, 1, ifile) != 1)
			{
			result = conversionOK; 
			break;
			}
		ch = srcBuffer[0];
		/* If we have a surrogate pair, convert to UTF32 first. */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END)
			{
			UTF32 ch2;
			if (fread( srcBuffer+1, 2, 1, ifile) != 1)
				{
				result = sourceExhausted; 
				break;
				}
			ch2 = srcBuffer[1];
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
			}
		else if ((flags == strictConversion) && (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END))
			{
			result = sourceIllegal;
			break;
			}
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
	return result;
}

int ReadLine( FILE *ifile, encoding srcEncoding, long codePage, int bigEndian, ConversionFlags flags )
	{
	char ch;
	int requiredSize;

	int outBufPos = 0;
	int inBufPos = 0;
	int result = -1;

	if( srcEncoding == utf16 )
		{
		while( 1 == fread( &(inBuf[inBufPos]), 2, 1, ifile ) )
			{
			if( bigEndian )
				{
				ch = inBuf[inBufPos];
				inBuf[inBufPos] = inBuf[inBufPos+1];
				inBuf[inBufPos+1] = ch;
				}
			if( 0x000A == (short)inBuf[inBufPos] )
				{
				inBufPos += 2;
				break;
				}
			inBufPos += 2;
			if( inBufSize <= inBufPos+2 )
				{
				inBufSize *= 2;
				inBuf = (char*)realloc( inBuf, inBufSize*sizeof(char) );
				if( NULL == inBuf )
					return -1;
				}
			}
		result = inBufPos;
		}
	else
		{
		codePage = srcEncoding == utf8 ? CP_UTF8 : codePage;

		while( 1 == fread( &(outBuf[outBufPos]), 1, 1, ifile ) )
			{
			if( 0x0A == outBuf[outBufPos] )
				{
				outBufPos++;
				break;
				}
			outBufPos++;
			if( outBufSize <= outBufPos+1 )
				{
				outBufSize *= 2;
				outBuf = (char*)realloc( outBuf, outBufSize*sizeof(char) );
				if( NULL == outBuf )
					return -1;
				}
			}
		if( 0 == outBufPos )
			return 0;
		requiredSize = MultiByteToWideChar( codePage, 0, outBuf, outBufPos, (wchar_t*)inBuf, 0 )*sizeof(short);
		if( 0 == requiredSize )
			return -1;
		else if( inBufSize < requiredSize )
			{
			inBufSize = requiredSize < 2*inBufSize ? 2*inBufSize : requiredSize;
			inBuf = (char*)realloc( inBuf, inBufSize*sizeof(char) );
			if( NULL == inBuf )
				return -1;
			}
		if( 0 == MultiByteToWideChar( codePage, 0, outBuf, outBufPos, (wchar_t*)inBuf, inBufSize/sizeof(short) ) )
			return -1;
		result = requiredSize;
		}

	return result;
	}

int WriteLine( FILE *ofile, encoding dstEncoding, long codePage, ConversionFlags flags, int nBytes )
	{
	int requiredSize;
	DWORD usedDefaultChar;

	int result = -1;

	codePage = dstEncoding == utf8 ? CP_UTF8 : codePage;

	if( dstEncoding == utf16 )
		{	
		if( nBytes != fwrite( inBuf, sizeof(char), nBytes, ofile ) )
			return -1;
		result = 0;
		}
	else
		{
		requiredSize = WideCharToMultiByte( codePage, 0, (wchar_t*)inBuf, nBytes/sizeof(SHORT),
											outBuf, 0, NULL, NULL );
		if( 0 == requiredSize )
			return -1;
		else if( outBufSize < requiredSize )
			{
			outBufSize = requiredSize < 2*outBufSize ? 2*outBufSize : requiredSize;
			outBuf = (char*)realloc( outBuf, outBufSize*sizeof(char) );
			if( NULL == outBuf )
				return -1;
			}
		usedDefaultChar = 0;
		if( 0 == WideCharToMultiByte( codePage, 0, (wchar_t*)inBuf, nBytes/sizeof(SHORT),
									  outBuf, outBufSize, NULL,
									  dstEncoding == utf8 ? NULL : &usedDefaultChar ) )
			return -1;
		if( usedDefaultChar)
			{
			int k = 0;
			usedDefaultChar = 0;
			for (k = 0; k < nBytes; k+=sizeof(SHORT))
				{
				char 	tmpBuf[32];
				requiredSize = WideCharToMultiByte( codePage, 0, (wchar_t*)(inBuf + k), 1,
									  tmpBuf, 32, NULL,
									  &usedDefaultChar );
				if( usedDefaultChar)
					{
					if ((k==0) && (line == 1) && ((int)(*((wchar_t*)(inBuf + k))) == 0xfeff))
						{
						}
					else
						{
						if (html)
							fprintf( stderr, "Warning: ");
						else
							{
							err_count++;
							if (err_count > 100)
								{
								return -1;
								}
							fprintf( stderr, "Error: ");
							}

						fprintf( stderr, "Couldn't perform conversion. Line %4d, Unicode = %x\n",line, (int)(*((wchar_t*)(inBuf + k))));
						sprintf(tmpBuf, "&#%d;", (int)(*((wchar_t*)(inBuf + k))));
						if( strlen(tmpBuf) != fwrite( tmpBuf, sizeof(char), strlen(tmpBuf), ofile ) )
							return -1;
						}
					}
				else
					{
					if( requiredSize != fwrite( tmpBuf, sizeof(char), requiredSize, ofile ) )
						return -1;
					}
				}
			}
		else
			{
			if( requiredSize != fwrite( outBuf, sizeof(char), requiredSize, ofile ) )
				return -1;
			}
		result = 0;
		}

	return result;
	}

ConversionResult Convert(FILE *ifile, FILE *ofile, encoding srcEncoding, encoding dstEncoding,
						 long codePage, ConversionFlags flags )
	{
	int bigEndian = 0;
	unsigned short uc;
	int nBytes;
	int	rc;

	inBufSize = 1024;
	inBuf = (char*)malloc( inBufSize*sizeof(char) );
	if( NULL == inBuf )
		return noMemory;
	outBufSize = inBufSize;
	outBuf = (char*)malloc( outBufSize*sizeof(char) );
	if( NULL == outBuf )
		return noMemory;

	/* If utf-16 handle Byte Order Marks and endianess. */
	if( srcEncoding == utf16 )
		{
		if( 1 != fread( &uc, sizeof(unsigned short), 1, ifile ) )
			return sourceExhausted;
		if( 0xFEFF == uc )
			bigEndian = 0;
		else if( 0xFFFE == uc )
			bigEndian = 1;
		else
			{
			fprintf( stderr, "Error: Illegal source code. Line %4d, Unicode = %x\n",line, uc);
			return sourceIllegal;
			}
		}

	/* Write out a little endian BOM for utf-16 */
	if( dstEncoding == utf16 )
		{
		uc = 0xFEFF;
		if( 1 != fwrite( &uc, sizeof(unsigned short), 1, ofile ) )
			return targetExhausted;
		}
	
	line = 1;
	rc = conversionOK;
	while( !feof( ifile ) )
		{
		nBytes = ReadLine( ifile, srcEncoding, codePage, flags, bigEndian );
		if( nBytes < 0 )
			return sourceIllegal;
		else if( nBytes == 0 )
			continue;
		if( WriteLine( ofile, dstEncoding, codePage, flags, nBytes ) < 0 )
			rc = sourceIllegal;
		line ++;
		}

	return rc;
	}

int main( int argc, char** argv )
	{
	FILE		*ifile, *ofile;
	encoding	srcEncoding = unknown, dstEncoding = unknown;
	long		codepage = 1252;
	int			opt;
	ConversionResult rc;

	ifile = stdin;
	ofile = stdout;
	html = 0;
	while ((opt = getopt(argc, argv, "s:d:p:i:o:h")) != EOF)
		{
		switch (opt)
			{
			case 's':
				srcEncoding = GetEncodingArg(optarg);
				break;
			case 'd':
				dstEncoding = GetEncodingArg(optarg);
				break;
			case 'p':
				codepage = atoi(optarg);
				break;
			case 'h':
				html = 1;
				break;
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

	if ((srcEncoding == unknown) || (dstEncoding == unknown) || !ValidateCodePage(codepage))
		Usage(argv[0]);

	/* Turn off CRLF to LF conversion */
	_setmode( _fileno( ifile ), _O_BINARY );
	_setmode( _fileno( ofile ), _O_BINARY );
	err_count = 0;
	rc = Convert(ifile, ofile, srcEncoding, dstEncoding, codepage, strictConversion);
	DisplayConversionStatus(rc);
	if ((rc == conversionOK) && (err_count != 0))
		rc = sourceIllegal;
	return rc;
	}
	
