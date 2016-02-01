/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.  
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

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
   string   result = str;

   for (auto &ch : result)
   {
      if (delims.find(ch) != string::npos)
         ch = ' ';
   }

   return result;
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
