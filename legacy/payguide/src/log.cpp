#include "log.h"
#include <syslog.h>
#include <semaphore.h>

static sem_t log_lock;
static int log_init=0;

int LogInit()
{
	openlog("payguide", LOG_PID,LOG_USER);
	{
		sem_init(&log_lock, 0, 1);
		log_init=1;
		return 0;
	}
	return 1;
}

int LogWrite(int priority, const std::string *msg)
{
	return LogWrite(priority,msg->c_str());
}

int LogWrite(int priority, const char *msg)
{
	if (msg==NULL) return 1;
	if (log_init==0) return 1;
	
	#ifndef ENABLE_DEBUG
	if (priority==LOGMSG_NORMAL) return 2;
	#endif
	
	int pr=LOG_INFO;
	
	if (priority==LOGMSG_SYSTEM) pr=LOG_INFO;
	else if (priority==LOGMSG_NORMAL) pr=LOG_INFO;
	else if (priority==LOGMSG_WARNING) pr=LOG_WARNING;
	else if (priority==LOGMSG_ERROR) pr=LOG_ERR;
	else if (priority==LOGMSG_CRITICAL) pr=LOG_EMERG;
	
	printf("%s\n",msg);
	sem_wait(&log_lock);
	syslog(priority, msg);
	sem_post(&log_lock);
	return 0;
}


int LogClose()
{
	if (log_init==0) return 1;
	closelog();
	log_init=0;
	return 0;
}

