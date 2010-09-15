#ifndef __Log__
#define __Log__
#include "settings.h"
#include <string>

#define LOGMSG_NORMAL	0
#define LOGMSG_SYSTEM	1
#define LOGMSG_WARNING 2
#define LOGMSG_ERROR 3
#define LOGMSG_CRITICAL 4

/* Init log writing */
int LogInit();

/* Write a message to log (std::string form) */
int LogWrite(int priority, const std::string *msg);

/* Write a message to log (C char* style) */
int LogWrite(int priority, const char *msg);

/* Finish log writing */
int LogClose();

#endif

