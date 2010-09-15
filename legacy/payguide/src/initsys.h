#ifndef __InitSys__
#define __InitSys__
#include <dirent.h>
#include <dlfcn.h>

int LoadAllInitSO(const char *dir);
int UnLoadAllInitSO();

#endif

