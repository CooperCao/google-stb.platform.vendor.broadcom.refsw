/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_parse_utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>

using namespace std;

namespace bsg
{

string ParseUtils::StripWhite(const string &str)
{
   int lasc = str.find_first_not_of(" \t\n\r");
   int rasc = str.find_last_not_of(" \t\n\r");

   if (lasc != -1 && rasc != -1)
      return str.substr(lasc, rasc - lasc + 1);

   return "";
}

string ParseUtils::RemoveDelims(const string &str, const string &delims)
{
   string   ret = str;

   for (string::iterator sPtr = ret.begin(); sPtr != ret.end(); ++sPtr)
   {
      for (string::const_iterator dPtr = delims.begin(); dPtr != delims.end(); ++dPtr)
      {
         if (*sPtr == *dPtr)
            *sPtr = ' ';
      }
   }

   return ret;
}

string ParseUtils::ToLower(const string &str)
{
   string   res = str;

   transform(res.begin(), res.end(), res.begin(), ::tolower);

   return res;
}

string ParseUtils::ToUpper(const string &str)
{
   string   res = str;

   transform(res.begin(), res.end(), res.begin(), ::toupper);

   return res;
}

bool ParseUtils::StartsWith(const string &str, const string &startsWith)
{
   return str.substr(0, startsWith.length()) == startsWith;
}

bool ParseUtils::IsDirectory(const std::string &str)
{
   struct stat buffer;
   int         status;

   status = stat(str.c_str(), &buffer);
   if (status == 0)
      return (buffer.st_mode & S_IFDIR) != 0;

   return false;
}

bool ParseUtils::IsFile(const std::string &str)
{
   struct stat buffer;
   int         status;

   status = stat(str.c_str(), &buffer);
   if (status == 0)
      return (buffer.st_mode & S_IFREG) != 0;

   return false;
}

std::string ParseUtils::GetBasename(const std::string &str)
{
   std::string res(str);

#ifdef WIN32
   size_t   pos = res.find_last_of('\\');
#else
   size_t   pos = res.find_last_of('/');
#endif

   std::string filename;
   if (pos == std::string::npos)
      filename = str;
   else
      filename = str.substr(pos + 1);

   // Strip extension
   pos = filename.find_last_of('.');
   if (pos == std::string::npos)
      return filename;
   else
      return filename.substr(0, pos);
}

std::string ParseUtils::GetPathname(const std::string &str)
{
   std::string res(str);

#ifdef WIN32
   size_t   pos = res.find_last_of('\\');
#else
   size_t   pos = res.find_last_of('/');
#endif

   std::string pathname;
   if (pos == std::string::npos)
      pathname = ".";
   else
      pathname = str.substr(0, pos);

   return pathname;
}

}
