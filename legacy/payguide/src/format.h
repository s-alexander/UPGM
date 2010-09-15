#ifndef __Format__
#define __Format__
#include <pcre.h>
int ValidData(pcre *format, const char *data);
/*
 1 - data valid
 2 - bad format
-1 - data invalid
*/
#endif
