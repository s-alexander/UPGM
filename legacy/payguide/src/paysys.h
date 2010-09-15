#ifndef __PaySys__
#define __PaySys__

#include <dirent.h>
#include <pthread.h>
#include <dlfcn.h>
#include "settings.h"
#include "parser.h"
#include "ulist.h"
#include "pay.h"
#include "operator.h"
#include "sem.h"

#define MAX_FILE_NAME_LEN 100
#define MSG_NEW_JOB 1
class SPayResult;
class SPay;
class COperator;

/*struct SVersion
{
	int major;
	int minor;
	int revision;
	int build;
};*/

class CPaySys
{
	public:
	    CPaySys();
	    ~CPaySys();
	    int Load(const char *file_name, const char *short_file_name);
	    int UnLoad();
	    char *GetName();
	    char *GetShortName();
	    int IsBroken();
	    
	    CConfig *GetConfig();
	    CSemaphore lock;
	    
	    int CheckSupport();
	    int LoadConfig();
	    
	    void *InitPay(CPaySys *paysys, COperator *operat, SPay *pay, int worker_id);
	    SPayResult SendPay(SPay *data, void *init_results, void *thread_init);
	    void CleanUp(void *init_values);
	    void *ThreadStart();
	    void ThreadEnd(void *thread_init);
	    void Call();
	    
	    int Use();
	    int UnUse();
	    int IsUnloaded();
	    
	private:
	    void *so_descriptor;
	    char name[100];
	    char short_name[50];
	    SPayResult (*SendData)(SPay* data, void *init_values, void *first_init_param, void *thread_init);
	    
	    void  (*Clean)(void *init_values, void *first_init_param);
	    void *(*Init)(CPaySys *paysys, COperator *operat, int worker_id, void *first_init_param);
	    void *(*InitExt)(CPaySys *paysys, COperator *operat, SPay *pay, int worker_id, void *first_init_param);
	    void *(*LoadSo)(CPaySys *paysys);
	    void  (*UnLoadSo)(CPaySys *paysys, void *first_init_param);
	    void *(*PThreadStart)();
	    void  (*PThreadEnd)(void *thread_init);
	    void  (*SoCall)(void *first_init_param);
	    int broken;
	    int LoadFunc();
	    int use_count;
	    sem_t sem;
	    CConfig *paysys_config;
	    void *paysys_first_init_param;
	    int unloaded;
	    void *thread_init;
	    int check_support;
};
int ReloadPaySysConfigs();
CUlist<CPaySys> *LoadModules(const char *dir);
CPaySys *GetPaySysBySoName(const char *so_name);
CPaySys *GetPaySysDefault();
CPaySys *GetPaySysEngineLimit();
int LoadPaySys(const char *short_so_name);
int UnLoadPaySys(const char *short_so_name);
int RemoveUnloadedModules();
#endif

