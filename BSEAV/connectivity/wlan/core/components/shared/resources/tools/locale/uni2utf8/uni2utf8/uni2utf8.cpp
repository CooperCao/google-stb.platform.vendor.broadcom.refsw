// uni2utf8.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "ConvertUTF.h"

HANDLE hInFile = INVALID_HANDLE_VALUE;
HANDLE hOutFile = INVALID_HANDLE_VALUE;
DWORD dwNumOfByteRead;
DWORD dwNumOfByteWrite;
unsigned int inBufSize, inBufPos;
unsigned int inReadSize, outWriteSize;
char* inBuf;
char* outBuf;
int outBufSize, requiredSize;
int uni2utf8;
unsigned short uc;
char ch;

int bigEndian = 0;

int convert(void)
{
	char* localOutBuf;
	char* localInBuf;
	ConversionResult conversionResult;
	int converting = 1;

	while (converting)
	{
		localOutBuf = outBuf;
		localInBuf = inBuf;
		if (uni2utf8)
		{
			conversionResult = ConvertUTF16toUTF8((const UTF16**)&localInBuf, (UTF16*)&inBuf[inBufPos],
				(UTF8**)&localOutBuf, (UTF8*)&outBuf[outBufSize],
				strictConversion);
		}
		else
		{
			conversionResult = ConvertUTF8toUTF16((const UTF8**)&localInBuf, (const UTF8*)&inBuf[inBufPos],
				(UTF16**)&localOutBuf, (UTF16*)&outBuf[outBufSize],
				strictConversion);
		}
		switch (conversionResult)
		{
		case conversionOK:
			converting = 0;
			break;
		case sourceExhausted:
			fprintf(stderr, "Source Exhausted.\n");
			return -1;
		case targetExhausted:
		{
			outBufSize *= 2;
			outBuf = (char*)realloc(outBuf, outBufSize*sizeof(char));
			if (NULL == outBuf)
			{
				fprintf(stderr, "Target Exhausted.\n");
				return -2;
			}
			break;
		}
		case sourceIllegal:
			fprintf(stderr, "Illegal Source.\n");
			return -3;
		}

	}

	if (!uni2utf8)
	{
		if (*(short*)outBuf == 0xFEFF ||
			*(short*)outBuf == 0xFFFE)
			outBuf += 2;
	}
	requiredSize = localOutBuf - outBuf;
	WriteFile(hOutFile, outBuf, requiredSize, &dwNumOfByteWrite, NULL);
	if (requiredSize!= dwNumOfByteWrite)
	{
		fprintf(stderr, "Write failed.\n");
		return -5;
	}

	if (uni2utf8)
	{
		WriteFile(hOutFile, "\r\n", 2, &dwNumOfByteWrite, NULL);
		if (2 != dwNumOfByteWrite)
		{
			fprintf(stderr, "Write failed.\n");
			return -5;
		}
	}
	else
	{
		WriteFile(hOutFile, L"\r\n", 4, &dwNumOfByteWrite, NULL);
		if (4 != dwNumOfByteWrite)
		{
			fprintf(stderr, "Write failed.\n");
			return -5;
		}
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (_tcsstr(argv[1], _T("-r")) != NULL)
		uni2utf8 = 0;
	else
		uni2utf8 = 1;

	if (2 < argc && (0 != _tcscmp(argv[2], _T("-"))))
	{
		hInFile = CreateFile(argv[2],
			GENERIC_READ | FILE_SHARE_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (INVALID_HANDLE_VALUE == hInFile)
		{
			fprintf(stderr, "Unable to open %s to convert it.\n", argv[2]);
			return -1;
		}
	}

	if (argc == 4 && (0 != _tcscmp(argv[3], _T("-"))))
	{
		hOutFile = CreateFile(argv[3],
			GENERIC_WRITE | FILE_SHARE_READ,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
			NULL);

		if (INVALID_HANDLE_VALUE == hOutFile)
		{
			fprintf(stderr, "Unable to open %s for writing.\n", argv[3]);
			return -1;
		}
	}

	inBufSize = 1024;
	inBuf = (char*)malloc(inBufSize*sizeof(char));
	if (NULL == inBuf)
	{
		fprintf(stderr, "Malloc failed.\n");
		return -10;
	}
	outBufSize = inBufSize;
	outBuf = (char*)malloc(outBufSize*sizeof(char));
	if (NULL == outBuf)
	{
		fprintf(stderr, "Malloc failed.\n");
		return -10;
	}
	inBufPos = 0;

	if (uni2utf8)
	{
		ReadFile(hInFile, &uc, sizeof(short), &dwNumOfByteRead, NULL);
		if (sizeof(short) != dwNumOfByteRead)
		{
			fprintf(stderr, "Read error with file.\n");
			return -1;
		}
		if (0xFEFF != uc && 0xFFFE != uc)
		{
			fprintf(stderr, "Warning: no Byte Order Mark (BOM) found. Assuming UTF 16 little endian.\n");
			*(short*)inBuf = uc;
			inBufPos += 2;
		}
		else
		{
			if (0xFFFE == uc)
				bigEndian = 1;
		}
		inReadSize = 2; outWriteSize = 1;
	}
	else
	{
		*(short*)outBuf = 0xFEFF;
		WriteFile(hOutFile, outBuf, 2, &dwNumOfByteWrite, NULL);
		if (2 != dwNumOfByteWrite)
		{
			return -5;
		}
		inReadSize = 1; outWriteSize = 2;
	}

	while (ReadFile(hInFile, &(inBuf[inBufPos]), inReadSize, &dwNumOfByteRead, NULL))
	{
		if (dwNumOfByteRead == 0)
			break;
		if (uni2utf8 && bigEndian)
		{
			ch = inBuf[inBufPos];
			inBuf[inBufPos] = inBuf[inBufPos + 1];
			inBuf[inBufPos + 1] = ch;
		}

		if (uni2utf8)
			uc = *((short*)&(inBuf[inBufPos]));
		else
			ch = inBuf[inBufPos];

		if ((uni2utf8 && 0x000D == uc) ||
			(!uni2utf8 && 0x0D == ch))
			continue;

		if ((uni2utf8 && 0x000A == uc) ||
			(!uni2utf8 && 0x0A == ch))
		{
			if (convert() < 0)
			{
				fprintf(stderr, "convert() failed.\n");
				return -11;
			}
			inBufPos = 0;
		}
		else
		{
			inBufPos += inReadSize;
			if (inBufSize <= inBufPos + inReadSize)
			{
				inBufSize *= 2;
				inBuf = (char*)realloc(inBuf, inBufSize*sizeof(char));
				if (NULL == inBuf)
				{
					fprintf(stderr, "Malloc failed.\n");
					return -10;
				}
			}
		}
	}

	if (convert() < 0)
	{
		fprintf(stderr, "convert() failed.\n");
		return -11;
	}

	CloseHandle(hInFile);
	CloseHandle(hOutFile);
	return 0;
}
