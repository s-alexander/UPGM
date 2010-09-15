#include "paysys.h"
#include "log.h"
#include "namespace.h"
#include <iostream>
#include <dlfcn.h>
#define MAX_FILE_NAME_LEN 100

CPaySys::CPaySys()
{
		broken=0;
		strncpy(name,"-",49);
		strncpy(short_name,"-",49);
		use_count=0;
		sem_init(&sem, 0,1);
		paysys_config=new CConfig();
		paysys_first_init_param=NULL;
		so_descriptor=NULL;
		unloaded=0;
		
		SendData=NULL;
		Clean=NULL;
		Init=NULL;
		InitExt=NULL;
		LoadSo=NULL;
		UnLoadSo=NULL;
		PThreadStart=NULL;
		PThreadEnd=NULL;
		SoCall=NULL;
		check_support=0;
		
}
CPaySys::~CPaySys()
{
	if (broken==0)
		UnLoadSo(this, paysys_first_init_param);
	if (so_descriptor!=NULL)
		dlclose(so_descriptor);
	so_descriptor=NULL;
	SendData=NULL;
	if (paysys_config!=NULL)
		delete paysys_config;
}

CConfig *CPaySys::GetConfig()
{
	return paysys_config;
}

int CPaySys::CheckSupport()
{
	return check_support;
}

int CPaySys::LoadConfig()
{
	if (GetPaySysDefault()==this) return 0;
	string cfg_name=name;
	cfg_name+=".cfg";
	
	string log_msg="Loading paysys config from "; log_msg+=cfg_name;
	LogWrite(LOGMSG_SYSTEM, &log_msg);
	
	return paysys_config->ReadFile(cfg_name.c_str());

}


int CPaySys::Load(const char *file_name, const char *short_file_name)
{
	if (file_name==NULL || short_file_name==NULL) return -1;   
	broken=0;
	char *error = NULL;
	std::string s1= "Loading "; s1+=file_name; LogWrite(LOGMSG_SYSTEM,&s1);

	strncpy(name, file_name,99);
	strncpy(short_name, short_file_name,49);
	so_descriptor = dlopen(file_name, RTLD_NOW);

	if(so_descriptor==NULL && ((error = dlerror()) != NULL)) 
	{

		std::string s1= "Error: "; s1+=file_name; s1+=" is not a valid *.so file (";s1+=error;s1+=")" ;  LogWrite(LOGMSG_WARNING,&s1);
		error = dlerror();
		broken=1;	//Mark as broken
	}
	

	if (broken==0)
		broken=LoadFunc();
		
	if (broken==0)
	{
		LoadConfig();
		paysys_first_init_param=LoadSo(this);
		
	}
	return broken;
}

int CPaySys::UnLoad()
{
	payguide::need_cleanup_modules=1;
	unloaded=1;
	return 0;
}

int CPaySys::LoadFunc()
{
	char *error;
	dlerror(); 
	SendData = (SPayResult  (*)(SPay *, void *, void *, void *))dlsym(so_descriptor, "SendData");
	if ((error = dlerror()) != NULL)  
	{
		std::string s1= "Error: ["; s1+=error; s1+="], "; s1+=name; s1+=" doesn't contain \"int send_data(SPayData *)\" function.";  LogWrite(LOGMSG_WARNING,&s1);
		return 1;
	}
		
	Clean = (void (*)(void *, void *))dlsym(so_descriptor, "CleanUp");
	if ((error = dlerror()) != NULL)  
	{
		std::string s1= "Error: ["; s1+=error; s1+="], "; s1+=name; s1+=" doesn't contain \"void CleanUp(void *)\" function.";  LogWrite(LOGMSG_WARNING,&s1);
		return 1;
	}
		
	Init = (void *(*)(CPaySys *, COperator *, int , void *))dlsym(so_descriptor, "PayInit");
	if ((error = dlerror()) != NULL)  
	{
		Init=NULL;
//		std::string s1= "Error: ["; s1+=error; s1+="], "; s1+=name; s1+=" doesn't contain \"void *PayInit(CPaySys *paysys, COperator *operat, int worker_id)\" function. Checking for extended version";  LogWrite(LOGMSG_WARNING,&s1);
		InitExt = (void *(*)(CPaySys *, COperator *,SPay *, int , void *))dlsym(so_descriptor, "PayInitExt");
		if ((error = dlerror()) != NULL)  
		{
			std::string s1= "Error: ["; s1+=error; s1+="], "; s1+=name; s1+=" doesn't contain \"void *PayInitExt(CPaySys *paysys, COperator *operat,SPay* pay, int worker_id)\" function, too...";  LogWrite(LOGMSG_WARNING,&s1);
			return 1;
		}

	}

		
	LoadSo = (void *(*)(CPaySys *paysys))dlsym(so_descriptor, "Load");
	if ((error = dlerror()) != NULL)  
	{
		std::string s1= "Error: ["; s1+=error; s1+="], "; s1+=name; s1+=" doesn't contain \"void Load(CPaySys *paysys)\" function.";  LogWrite(LOGMSG_WARNING,&s1);
		return 1;
	}
		
	UnLoadSo = (void (*)(CPaySys *, void *))dlsym(so_descriptor, "UnLoad");
	if ((error = dlerror()) != NULL)  
	{
		std::string s1= "Error: ["; s1+=error; s1+="], "; s1+=name; s1+=" doesn't contain \"void UnLoad(CPaySys *paysys)\" function.";  LogWrite(LOGMSG_WARNING,&s1);
		return 1;
	}

	(int (*)())dlsym(so_descriptor, "PayCheckSupport");
	if ((error = dlerror()) != NULL)  
		check_support=0;
	else
	{
		check_support=1;
		std::string s1= "Paycheck for "; s1+=name; s1+=" ON";  LogWrite(LOGMSG_WARNING,&s1);

	}
	
	PThreadStart = (void *(*)())dlsym(so_descriptor, "ThreadStart");
	if ((error = dlerror()) != NULL)  
	{
//		std::string s1= "Error: ["; s1+=error; s1+="], "; s1+=name; s1+=" doesn't contain \"void *ThreadStart()\" function.";  LogWrite(LOGMSG_WARNING,&s1);
//		return 1;
	}

	PThreadEnd = (void (*)(void *))dlsym(so_descriptor, "ThreadEnd");
	if ((error = dlerror()) != NULL)  
	{
//		std::string s1= "Error: ["; s1+=error; s1+="], "; s1+=name; s1+=" doesn't contain \"void ThreadEnd(void *)\" function.";  LogWrite(LOGMSG_WARNING,&s1);
//		return 1;
	}

	SoCall = (void (*)(void *))dlsym(so_descriptor, "Call");
	if ((error = dlerror()) != NULL)  
	{
//		std::string s1= "Error: ["; s1+=error; s1+="], "; s1+=name; s1+=" doesn't contain \"void Call()\" function.";  LogWrite(LOGMSG_WARNING,&s1);
//		return 1;
	}

	return 0;
}

char *CPaySys::GetName()
{
	return name;
}


int CPaySys::UnUse()
{
	if (use_count>0) use_count--;
	payguide::need_cleanup_modules=1;
	return use_count;
}

int CPaySys::Use()
{
	use_count++;
	return use_count;
}

char *CPaySys::GetShortName()
{
	return short_name;
}

int CPaySys::IsBroken()
{
	return broken;
}

int CPaySys::IsUnloaded()
{
	return unloaded;
}

void * CPaySys::InitPay(CPaySys *paysys, COperator *operat,SPay *pay, int worker_id)
{
	if (Init!=NULL)
		return Init(paysys, operat, worker_id, paysys_first_init_param);
	else
		return InitExt(paysys, operat, pay,worker_id, paysys_first_init_param);
	
}

void *CPaySys::ThreadStart()
{
	if (PThreadStart==NULL)
	{
		return NULL;
	}
	return PThreadStart();
	

}

void  CPaySys::ThreadEnd(void *thread_init)
{
	if (PThreadEnd==NULL)
		return;
	PThreadEnd(thread_init);
	return;
}

void  CPaySys::Call()
{
	if (SoCall==NULL)
		return;
	SoCall(paysys_first_init_param);
	return;
}


SPayResult CPaySys::SendPay(SPay *pay, void *init_values, void *thread_init)
{
	if (broken)
	{
		SPayResult res;
		strncpy(res.msg,"ζακμ *.SO χ ξεχεςξον ζοςνατε. σοοβύιτε αδνιξιστςατοςυ.",SIZE_REPONSE_MSG);
		strncpy(res.sender_name,short_name,SIZE_REPONSE_SENDER);
		res.code=RESULT_FAILED;
		LogWrite(LOGMSG_ERROR, "YOU ARE USING *.SO FILE IN WRONG FORMAT");
		return res;
	}
	return SendData(pay, init_values, paysys_first_init_param, thread_init);
}

void CPaySys::CleanUp(void *init_values)
{
	if (broken)
	{
		LogWrite(LOGMSG_ERROR, "YOU ARE USING *.SO FILE IN WRONG FORMAT");
		return;
	}
	Clean(init_values, paysys_first_init_param);
}

int ReloadPaySysConfigs()
{
	int result=0;
	if (payguide::modules_list!=NULL)
	{
		payguide::modules_list->ResetCursor();
		for (unsigned int i=0; i<payguide::modules_list->GetLen(); i++)
		{
			CPaySys *tmp=payguide::modules_list->GetNext();
			if (tmp!=NULL)
			{
				tmp->lock.LockRead();
				tmp->GetConfig()->Clear();
				result+=tmp->LoadConfig();
				tmp->lock.UnLockRead();
			}
	
		}
	}
	return result;
}

CUlist<CPaySys> *LoadModules(const char *dir)
{
	if (dir==NULL) return NULL;
	const char *ext=".so";
	CUlist<CPaySys> *result = new CUlist<CPaySys>;
	if (result==NULL) return NULL;
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
					
					CPaySys *tmp=new CPaySys();
					if (tmp==NULL) return NULL;
					
					char buff[1024];
					strncpy(buff, dir,511);
					

					strncat(buff, cursor->d_name,511);
					if (tmp->Load(buff, cursor->d_name)==0)
						result->AddItem(tmp);
					else
					{
						delete tmp;
						tmp=NULL;
					}
				}
			}
			add=0;
		}
	        closedir(dir_opened);
	}
	else
	{
		string log_msg="Can't open dir ";
		log_msg+=dir;
		LogWrite(LOGMSG_ERROR, &log_msg);
//		std::cout << "Can't open dir " << dir << std::endl;

	}
	return result;
}

CPaySys *GetPaySysBySoName(const char *so_name)
{
	payguide::modules_list->ResetCursor();
	for (unsigned int i=0; i<payguide::modules_list->GetLen(); i++)
	{
		CPaySys *tmp=payguide::modules_list->GetNext();
		if (tmp!=NULL)
		{
//			std::cout << "compare [" <<tmp->GetShortName() << "] and [" << so_name <<"]\n";
			if (strcmp(tmp->GetShortName(), so_name)==0 && tmp->IsUnloaded()==0)
			{
				payguide::modules_list->ResetCursor();
				return tmp;
			}
		}
	}
	
	return payguide::null_pay_sys;
	
}

CPaySys *GetPaySysDefault()
{
	return payguide::null_pay_sys;
}

CPaySys *GetPaySysEngineLimit()
{
	return GetPaySysBySoName("_limit.so");
}


int LoadPaySys(const char *short_so_name)
{
	int already_loaded=0;
	int result=0;
	payguide::modules_list->ResetCursor();
	for (unsigned int i=0; i<payguide::modules_list->GetLen(); i++)
	{
		CPaySys *tmp=payguide::modules_list->GetNext();
		if (tmp!=NULL)
		{
			if (strcmp(tmp->GetShortName(), short_so_name)==0 && tmp->IsUnloaded()==0)
				already_loaded=1;
		}
	}
	if (already_loaded==0)
	{
		CPaySys *tmp=new CPaySys();
		if (tmp==NULL) return 13;
					
		char buff[1024];
		strncpy(buff, payguide::modules_path.c_str(),511);
//		std::cout << "loading " << short_so_name << std::endl;
		strncat(buff, short_so_name,511);
		result=tmp->Load(buff, short_so_name);
		if (result==0)
		{
			payguide::modules_list->AddItem(tmp);
		}
		else
		{
			delete tmp;
		}
	
	}
	else
		result=1024;

	return result;
}

int UnLoadPaySys(const char *short_so_name)
{
	payguide::modules_list->ResetCursor();
	for (unsigned int i=0; i<payguide::modules_list->GetLen(); i++)
	{
		CPaySys *tmp=payguide::modules_list->GetNext();
		if (tmp!=NULL)
		{
			if (strcmp(tmp->GetShortName(), short_so_name)==0)
			{
				tmp->UnLoad();
				return 0;
			}
		}
	}

	return -1;
}

int RemoveUnloadedModules()
{
	int c=0;
	int rest=0;
	if (payguide::modules_list!=NULL)
	{
		
		payguide::modules_list->ResetCursor();
		for (unsigned int i=0; i<payguide::modules_list->GetLen(); i++)
		{
			CPaySys *tmp=payguide::modules_list->GetNext();
			if (tmp!=NULL)
			{
				if (tmp->IsUnloaded()==1)
				{
					if (tmp->Use()==1)
					{
						payguide::modules_list->RemoveThis();
						c++;
					}
					else
					{
						tmp->UnUse();
						rest++;
					}
					
				}
				
			}
		}

	}
	if (rest==0) payguide::need_cleanup_modules=0;
	return c;
}
