/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <iostream>

#ifdef WIN32
#include <windows.h>

void *GetFunctionAddress(const char *module, const char *funcName)
{
   HMODULE h = GetModuleHandle(module);
   void *res = (void*)GetProcAddress(h, funcName);

   if (res == NULL)
   {
      DWORD err = GetLastError();
      LPTSTR errorText = NULL;

      FormatMessage(
         FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL,
         err,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (LPTSTR)&errorText,
         0,
         NULL);

      if (errorText)
      {
         std::cout << "GetFunctionAddress generated error : " << errorText << '\n';
         LocalFree(errorText);
         errorText = NULL;
      }
      else
         std::cout << "GetFunctionAddress generated error : " << err << '\n';
   }
   return res;
}
#else
#include <dlfcn.h>

void *GetFunctionAddress(const char *module, const char *funcName)
{
   return (void*)dlsym(RTLD_DEFAULT, funcName);
}
#endif
