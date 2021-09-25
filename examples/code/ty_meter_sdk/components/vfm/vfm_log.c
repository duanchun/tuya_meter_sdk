
#include "vfm_log.h"
#include "stdio.h"
#include "stdarg.h"

int PrintLog(const char* pModuleName,
                        const char logLevel,
                        const char* pFile, const int line,
                        const char* pFunc,
                        const char* pFmt,...)
{
  const char* fmt = "VFM";
	va_list ap;
	   
	va_start(ap, pFmt);
	vprintf(pFmt, ap);
	printf("\r\n");
	return 0;
}

