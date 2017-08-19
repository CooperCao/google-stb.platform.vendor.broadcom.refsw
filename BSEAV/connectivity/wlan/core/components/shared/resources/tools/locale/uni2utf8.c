#include <windows.h>
#include <stdio.h>

#include "ConvertUTF.c"

FILE* inFile;
FILE* outFile;
unsigned int inBufSize, inBufPos;
unsigned int inReadSize, outWriteSize;
char* inBuf;
char* outBuf;
int outBufSize, requiredSize;
int uni2utf8;
unsigned short uc;
char ch;

int bigEndian = 0;

int convert( void )
	{
	char* localOutBuf;
	char* localInBuf;
	ConversionResult conversionResult;
	int converting = 1;

	while( converting )
		{
		localOutBuf = outBuf;
		localInBuf = inBuf;
		if( uni2utf8 )
			conversionResult = ConvertUTF16toUTF8( (UTF16**)&localInBuf, (UTF16*)&inBuf[inBufPos],
												   &localOutBuf, &outBuf[outBufSize],
												   strictConversion );
		else
			conversionResult = ConvertUTF8toUTF16( &localInBuf, &inBuf[inBufPos],
												   (UTF16**)&localOutBuf, (UTF16*)&outBuf[outBufSize],
												   strictConversion );
		switch( conversionResult )
			{
			case conversionOK:
				converting = 0;
				break;
			case sourceExhausted:
				fprintf( stderr, "Source Exhausted.\n" );
				return -1;
			case targetExhausted:
			{
			outBufSize *= 2;
			outBuf = (char*)realloc( outBuf, outBufSize*sizeof(char) );
			if( NULL == outBuf )
				{
				fprintf( stderr, "Target Exhausted.\n" );
				return -2;
				}
			break;
			}
			case sourceIllegal:
				fprintf( stderr, "Illegal Source.\n" );
				return -3;
			}

		}

	if( !uni2utf8 )
		{
		if( *(short*)outBuf == 0xFEFF ||
			*(short*)outBuf == 0xFFFE )
			outBuf += 2;
		}
	requiredSize = localOutBuf-outBuf;
	if( requiredSize != fwrite( outBuf, sizeof(char), requiredSize, outFile ) )
		{
		fprintf( stderr, "Write failed.\n" );
		return -5;
		}

	if( uni2utf8 )
		{
		if( 2 != fwrite( "\r\n", 1, 2, outFile ) )
			{
			fprintf( stderr, "Write failed.\n" );
			return -5;
			}
		}
	else
		{
		if( 4 != fwrite( L"\r\n", 1, 4, outFile ) )
			{
			fprintf( stderr, "Write failed.\n" );
			return -5;
			}
		}

	return 0;
}

int main( int argc, char** argv )
	{

	if( strstr( argv[1], "-r" ) )
		uni2utf8 = 0;
	else
		uni2utf8 = 1;

	if( 2 < argc && strcmp( argv[2], "-" ) )
		{
		inFile = fopen( argv[2], "rb" );
		if( NULL == inFile )
			{
			fprintf( stderr, "Unable to open %s to convert it.\n", argv[2] );
			return -1;
			}
		}
	else
		inFile = stdin;

	if( argc == 4 && strcmp( argv[3], "-" ) )
		{
		outFile = fopen( argv[3], "wb" );
		if( NULL == outFile )
			{
			fprintf( stderr, "Unable to open %s for writing.\n", argv[3] );
			return -1;
			}
		}
	else
		outFile = stdout;

	inBufSize = 1024;
	inBuf = (char*)malloc( inBufSize*sizeof(char) );
	if( NULL == inBuf )
		{
		fprintf( stderr, "Malloc failed.\n" );
		return -10;
		}
	outBufSize = inBufSize;
	outBuf = (char*)malloc( outBufSize*sizeof(char) );
	if( NULL == outBuf )
		{
		fprintf( stderr, "Malloc failed.\n" );
		return -10;
		}
	inBufPos = 0;

	if( uni2utf8 )
		{
		if( 1 != fread( &uc, sizeof(short), 1, inFile ) )
			{
			fprintf( stderr, "Read error with file.\n" );
			return -1;
			}
		if( 0xFEFF != uc && 0xFFFE != uc )
			{
			fprintf( stderr, "Warning: no Byte Order Mark (BOM) found. Assuming UTF 16 little endian.\n" );
			*(short*)inBuf = uc;
			inBufPos+=2;
			}
		else
			{
			if( 0xFFFE == uc )
				bigEndian = 1;
			}
		inReadSize=2; outWriteSize=1;
		}
	else
		{
		*(short*)outBuf = 0xFEFF;
		if( 1 != fwrite( outBuf, 2, 1, outFile ) )
			{
			fprintf( stderr, "Write failed.\n" );
			return -5;
			}
		inReadSize=1; outWriteSize=2;
		}

	while( 1 == fread( &(inBuf[inBufPos]), inReadSize, 1, inFile ) )
		{
		if( uni2utf8 && bigEndian )
			{
			ch = inBuf[inBufPos];
			inBuf[inBufPos] = inBuf[inBufPos+1];
			inBuf[inBufPos+1] = ch;
			}

		if( uni2utf8 )
			uc = *((short*)&(inBuf[inBufPos]));
		else
			ch = inBuf[inBufPos];

		if( (uni2utf8 && 0x000D == uc ) ||
			(!uni2utf8 && 0x0D == ch ) )
			continue;

		if( (uni2utf8 && 0x000A == uc ) ||
			(!uni2utf8 && 0x0A == ch ) )
			{
			if( convert() < 0 )
				{
				fprintf( stderr, "convert() failed.\n" );
				return -11;
				}
			inBufPos = 0;
			}
		else
			{
			inBufPos += inReadSize;
			if( inBufSize <= inBufPos+inReadSize )
				{
				inBufSize *= 2;
				inBuf = (char*)realloc( inBuf, inBufSize*sizeof(char) );
				if( NULL == inBuf )
					{
					fprintf( stderr, "Malloc failed.\n" );
					return -10;
					}
				}
			}
		}
	if( !feof( inFile ) )
		{
		fprintf( stderr, "Read error.\n" );
		return -1;
		}
	if( convert() < 0 )
		{
		fprintf( stderr, "convert() failed.\n" );
		return -11;
		}

	fclose(inFile);
	fclose(outFile);

	return 0;
	}
