// Common/MyString.cpp

#include "StdAfx.h"

#include <ctype.h>

#include "MyString.h"



wchar_t MyCharUpper(wchar_t c)
{
  return toupper(c);
}

/*
int MyStringCollateNoCase(const wchar_t *s1, const wchar_t *s2)
{
  for (;;)
  {
    wchar_t c1 = *s1++;
    wchar_t c2 = *s2++;
    wchar_t u1 = MyCharUpper(c1);
    wchar_t u2 = MyCharUpper(c2);

    if (u1 < u2) return -1;
    if (u1 > u2) return 1;
    if (u1 == 0) return 0;
  }
}
*/

int MyStringCompare(const char *s1, const char *s2)
{
  for (;;)
  {
    unsigned char c1 = (unsigned char)*s1++;
    unsigned char c2 = (unsigned char)*s2++;
    if (c1 < c2) return -1;
    if (c1 > c2) return 1;
    if (c1 == 0) return 0;
  }
}

int MyStringCompare(const wchar_t *s1, const wchar_t *s2)
{
  for (;;)
  {
    wchar_t c1 = *s1++;
    wchar_t c2 = *s2++;
    if (c1 < c2) return -1;
    if (c1 > c2) return 1;
    if (c1 == 0) return 0;
  }
}

int MyStringCompareNoCase(const wchar_t *s1, const wchar_t *s2)
{
  for (;;)
  {
    wchar_t c1 = *s1++;
    wchar_t c2 = *s2++;
    if (c1 != c2)
    {
      wchar_t u1 = MyCharUpper(c1);
      wchar_t u2 = MyCharUpper(c2);
      if (u1 < u2) return -1;
      if (u1 > u2) return 1;
    }
    if (c1 == 0) return 0;
  }
}

