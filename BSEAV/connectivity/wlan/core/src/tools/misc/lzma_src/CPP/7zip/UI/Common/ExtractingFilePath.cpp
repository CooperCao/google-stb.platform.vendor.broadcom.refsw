// ExtractingFilePath.cpp

#include "StdAfx.h"
#include "ExtractingFilePath.h"

static UString ReplaceIncorrectChars(const UString &s)
{
  #ifdef _WIN32
  UString res;
  for (int i = 0; i < s.Length(); i++)
  {
    wchar_t c = s[i];
    if (c < 0x20 || c == '*' || c == '?' || c == '<' || c == '>'  || c == '|' || c == ':' || c == '"')
      c = '_';
    res += c;
  }
  res.TrimRight();
  return res;
  #else
  return s;
  #endif
}


static UString GetCorrectFileName(const UString &path)
{
  if (path == L".." || path == L".")
    return UString();
  return ReplaceIncorrectChars(path);
}

void MakeCorrectPath(UStringVector &pathParts)
{
  for (int i = 0; i < pathParts.Size();)
  {
    UString &s = pathParts[i];
    s = GetCorrectFileName(s);
    if (s.IsEmpty())
      pathParts.Delete(i);
    else
    {
      #ifdef _WIN32
      if (!IsSupportedName(s))
        s = (UString)L"_" + s;
      #endif
      i++;
    }
  }
}

