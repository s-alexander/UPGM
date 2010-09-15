#include "initsys.h"
#include "log.h"
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#define MAX_FILE_NAME_LEN 100

static char using_dir[1024]="";

static int ExecFunctionFromSOFiles(const char *dir, const char *func);

int LoadAllInitSO(const char *dir)
{
	if (dir==NULL) return -1;
	strncpy(using_dir,dir,1024);
	if (strlen(dir)>1023)
		using_dir[1023]='\0';

	return ExecFunctionFromSOFiles(dir, "Load");
}

int UnLoadAllInitSO()
{
	return ExecFunctionFromSOFiles(using_dir, "UnLoad");
}

int ExecFunctionFromSOFiles(const char *dir, const char *func)
{
	int result=0;
	char *error=NULL;
	if (dir==NULL || func==NULL) return -1;
	const char ext[]="so";
	DIR *dir_opened;
	dirent *cursor;
	bool add=0;
	dir_opened=opendir(dir);
	if (dir_opened!=NULL)
	{
		while ((cursor=readdir(dir_opened))!=NULL)
		{
			if (strcmp(cursor->d_name,".")!=0 && strcmp(cursor->d_name,"..")!=0)
			{
				if (add==0)
				{
					if (strstr(cursor->d_name,ext)!=NULL && strlen(strstr(cursor->d_name,ext))==strlen(ext))add=1;
				}
				if (add==1)
				{
					char buff[1024];
					strncpy(buff, dir,511);
					strncat(buff, cursor->d_name,511);

					void *so_descriptor = dlopen(buff, RTLD_NOW);

					if(so_descriptor==NULL)
					{
						std::string log_msg="Error occured while loading ";
						log_msg+=buff;log_msg+=". Wrong *.so file";
						LogWrite(LOGMSG_ERROR, &log_msg);
					}
					else
					{

						void (*FUNC)(void)=(void (*)(void))dlsym(so_descriptor, func);
						if ((error = dlerror()) != NULL)
						{
							std::string log_msg="Can't load function \"void ";
							log_msg+=func;log_msg+="(void)\" from ";log_msg+=buff;
							LogWrite(LOGMSG_ERROR, &log_msg);
						}
						else
						{
							FUNC();
						}

						dlclose(so_descriptor);
					}

				}
			}
			add=0;
		}
	        closedir(dir_opened);
	}
	else
	{
		std::string log_msg="Can't open dir ";
		log_msg+=dir;
		LogWrite(LOGMSG_ERROR, &log_msg);
	}
	return result;
}


