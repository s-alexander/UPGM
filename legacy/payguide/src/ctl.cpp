#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <mysql/mysql.h>
#include <mysql/mysql_com.h>
#include <mysql/errmsg.h>
#include <mysql/mysql_version.h>
#include "ctl.h"
#include "worker.h"
#include "db.h"
#include "log.h"
#include "paycheck.h"
#include "namespace.h"


static int ctl_work=1;
static string socket_name;
static int db_locked=0;

int ExecCmd(int cmd, const char *arg, unsigned int arg_size, int *result_size, CSocketRW *sock)
{

	char unauth[2];
	unauth[0]=2;
	unauth[1]=0;
	
	char answok[2];
	answok[0]=3;
	answok[1]=0;

	char answnok[2];
	answnok[0]=4;
	answnok[1]=0;
	
	if (arg_size<0)
		return -1;
		
	if (cmd==P_CMD_STOP)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{
			if (0==db_locked)sem_wait(&payguide::db_lock_read);
			payguide::db_accept_new_pays=0;
			if (0==db_locked)sem_post(&payguide::db_lock_read);
			sock->Send(answok, 2);
		}
		else
			sock->Send(unauth, 2);
//		GiveAnswer("Stopped");
	}
	else if (cmd==P_CMD_RUN)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{

			if (0==db_locked)sem_wait(&payguide::db_lock_read);
			payguide::db_accept_new_pays=1;
			if (0==db_locked)sem_post(&payguide::db_lock_read);
			sock->Send(answok, 2);
		}
		else
			sock->Send(unauth, 2);
	}
	else if (cmd==P_CMD_CANCEL)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{

			long long pay_id=0;
			memcpy(&pay_id, arg, sizeof(pay_id));
			CancelWorker(pay_id);
			sock->Send(answok, 2);
		}
		else
			sock->Send(unauth, 2);
	}
	else if (cmd==P_CMD_KILL)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{

			long long pay_id=0;
			if (arg_size>=sizeof(pay_id))
			{
				memcpy(&pay_id, arg, sizeof(pay_id));
				KillWorker(pay_id);
				sock->Send(answok, 2);
			}
			else
			{
				char tmp[256];
				snprintf(tmp, 256, "paycheck: wrong message format in call P_CMD_KILL. Packet rejected.");
				LogWrite(LOGMSG_WARNING, tmp);
				sock->Send(answnok, 2);
			}
		}
		else
			sock->Send(unauth, 2);
	
	}
	else if (cmd==P_CMD_RELOAD_OP)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{

			if (ReloadOperators()==0)
			{
			//	GiveAnswer("Operators reloaded.");
				SendLogMessages(0, "Operators reloaded");
				sock->Send(answok, 2);
			}
			else
			{
			//	GiveAnswer("Error occured. Can't create new operators list.");
				SendLogMessages(0, "Error occured. Can't create new operators list.");
				sock->Send(answnok, 2);
			}
		}
		else
			sock->Send(unauth, 2);

//		if (0==db_locked)sem_wait(&payguide::db_lock_read);
//		payguide::db_accept_new_pays=old_v;
//		if (0==db_locked)sem_post(&payguide::db_lock_read);
	
	}
	else if (cmd==P_CMD_RELOAD_SO)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{

			ReConfPaysys();
			SendLogMessages(0, "Modules configs reloaded");
			sock->Send(answok, 2);
		}
		else
			sock->Send(unauth, 2);
	
	}
	else if (cmd==P_CMD_RELOAD_CONF)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{

			sem_wait(&payguide::reload_config_lock);
			payguide::reload_config=1;
			sem_post(&payguide::reload_config_lock);
			SendLogMessages(0, "Main config reloaded");
			sock->Send(answok, 2);
		}
		else
			sock->Send(unauth, 2);
	}
	else if (cmd==P_CMD_SHUTDOWN)
	{
		if (sock->Authorized()>=AUTH_ADMIN)
		{

			ctl_work=0;
//			raise(SIGINT);
			sem_wait(&payguide::shutdown_lock);
			payguide::working=false;
			payguide::quit=true;
			sem_post(&payguide::shutdown_lock);

			sock->Send(answok, 2);
		}
		else
			sock->Send(unauth, 2);

	}
	else if (cmd==P_CMD_REBOOT)
	{
		if (sock->Authorized()>=AUTH_ADMIN)
		{
			sock->Send(answok, 2);
			sem_wait(&payguide::shutdown_lock);
			payguide::working=false;
			payguide::quit=false;
			sem_post(&payguide::shutdown_lock);
			
		}
		else
			sock->Send(unauth, 2);
	}
	

	else if (cmd==P_CMD_LOAD)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{

			char *id=new char [arg_size+1];
			if (id!=NULL)
			{
				strncpy(id, arg, arg_size);
				id[arg_size]=0;
				sem_wait(&payguide::free_workers_lock);
				int lps_res=LoadPaySys(id);
				sem_post(&payguide::free_workers_lock);
				if (lps_res==0)
				{
					char t[1024];
					snprintf(t, 1023, "%s loaded.", id);
				//	GiveAnswer(t);
					sock->Send(answok, 2);
				}
				else if (lps_res==1024)
				{
					char t[1024];
					snprintf(t, 1023, "%s already has been loaded.", id);
				//	GiveAnswer(t);
					sock->Send(answok, 2);
				}
				else
				{
				//	GiveAnswer("Bad *.so file.");
					sock->Send(answnok, 2);
				}
				delete [] id;
			}
			else
				return -1;
		}
	}
	else if (cmd==P_CMD_UNLOAD)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{
			char *id=new char [arg_size+1];
			if (id!=NULL)
			{
				strncpy(id, arg, arg_size);
				id[arg_size]=0;
				sem_wait(&payguide::free_workers_lock);
				int ulps_res=UnLoadPaySys(id);
				sem_post(&payguide::free_workers_lock);
				if (ulps_res==-1)
				{
				//	GiveAnswer("No such *.so file");
					sock->Send(answnok, 2);
				}
				else
				{
				//	GiveAnswer("Module marked as \"unloading\" and will be unloaded as soon as possible");
					sock->Send(answok, 2);
				}
				delete [] id;
			}
			else
				return -1;
		}
	
	}
	else if (cmd==P_CMD_MONITOR)
	{
		if (sock->Authorized()>=AUTH_STAT)
		{
			char buff[10024];
			char *ptr=buff;
			ptr[0]=13; ptr++;
			ptr[0]=0; ptr++;
			int l=GetThreadsBin(ptr, 10020);
			if (l!=0)
				sock->Send(buff, l+2);
		}
		else
			sock->Send(unauth, 2);

	}
	else if (cmd==P_CMD_MODULES_LIST)
	{
		if (sock->Authorized()>=AUTH_STAT)
		{
			char buff[5024];
			char *ptr=buff;
			ptr[0]=14; ptr++;
			ptr[0]=0; ptr++;
			int l=GetModulesBin(ptr, sizeof(buff)-1);
			if (l!=0)
			{
				sock->Send(buff, l+2);
			}
		}
		else
			sock->Send(unauth, 2);

	}
	

	else if (cmd==P_CMD_CONSOLE && sock->Authorized()>=AUTH_CONTROL)
	{
		sock->Send(unauth, 2);
	}
	else if (cmd==P_CMD_ADD_IGNORE_OP && sock->Authorized()>=AUTH_CONTROL)
	{
		int id;
		if (arg_size>=sizeof(id))
		{
			memcpy(&id, arg, sizeof(id));
			AddOperatorToIgnoreList(id);
			sock->Send(answok, 2);
		}
		else
		{
			char tmp[256];
			snprintf(tmp, 256, "paycheck: wrong message format in call P_CMD_ADD_IGNORE_OP. Packet rejected.");
			LogWrite(LOGMSG_WARNING, tmp);
			sock->Send(answnok, 2);
		}
		//printf("Accepting operator %i now\n", id);
	}
	else if (cmd==P_CMD_ADD_IGNORE_EG && sock->Authorized()>=AUTH_CONTROL)
	{
		int id;
		if (arg_size>=sizeof(id))
		{
			memcpy(&id, arg, sizeof(id));
			AddEngineToIgnoreList(id);
			sock->Send(answok, 2);
		}
		else
		{
			char tmp[256];
			snprintf(tmp, 256, "paycheck: wrong message format in call P_CMD_ADD_IGNORE_OP. Packet rejected.");
			LogWrite(LOGMSG_WARNING, tmp);
			sock->Send(answnok, 2);
		}
		//printf("I*gnoring operator %i now\n", id);
	}
	else if (cmd==P_CMD_REMOVE_IGNORE_OP && sock->Authorized()>=AUTH_CONTROL)
	{
		int id;
		if (arg_size>=sizeof(id))
		{

			memcpy(&id, arg, sizeof(id));
			RemoveOperatorFromIgnoreList(id);
			sock->Send(answok, 2);
		}
		else
		{
			char tmp[256];
			snprintf(tmp, 256, "paycheck: wrong message format in call P_CMD_REMOVE_IGNORE_OP. Packet rejected.");
			LogWrite(LOGMSG_WARNING, tmp);
			sock->Send(answnok, 2);
		}
		//printf("Accepting operator %i now\n", id);
	}
	else if (cmd==P_CMD_REMOVE_IGNORE_EG && sock->Authorized()>=AUTH_CONTROL)
	{
		int id;
		if (arg_size>=sizeof(id))
		{

			memcpy(&id, arg, sizeof(id));
			RemoveEngineFromIgnoreList(id);
			sock->Send(answok, 2);
		}
		else
		{
			char tmp[256];
			snprintf(tmp, 256, "paycheck: wrong message format in call P_CMD_REMOVE_IGNORE_OP. Packet rejected.");
			LogWrite(LOGMSG_WARNING, tmp);
			sock->Send(answnok, 2);
		}
		//printf("Accepting engine %i now\n", id);
	}
	else if (cmd==P_CMD_CLEAR_IGNORE_OP && sock->Authorized()>=AUTH_CONTROL)
	{
		ClearOperatorIgnoreList();
		//printf("Accepting all operators\n");
		sock->Send(answok, 2);
	}
	else if (cmd==P_CMD_CLEAR_IGNORE_EG && sock->Authorized()>=AUTH_CONTROL)
	{
		ClearEngineIgnoreList();
		//printf("Accepting all engines\n");
		sock->Send(answok, 2);
	}
	else if (cmd==P_CMD_CLEAR_IGNORE_ALL && sock->Authorized()>=AUTH_CONTROL)
	{
		ClearEngineIgnoreList();
		ClearOperatorIgnoreList();
		//printf("Accepting EVERYTHING!\n");
		sock->Send(answok, 2);
	}
	else if (cmd==P_CMD_VIEW_IGNORE_OP && sock->Authorized()>=AUTH_CONTROL)
	{
		char tmp[1024];
		int l=0;
		tmp[0]=15;
		tmp[1]=0;
		char *buff=tmp+2;
		
		if (GetOperatorIgnoreList(buff, 1022, &l)==0)
			sock->Send(tmp,l+2);
		else
			sock->Send(answnok, 2);
	}
	else if (cmd==P_CMD_VIEW_IGNORE_EG && sock->Authorized()>=AUTH_CONTROL)
	{
		char tmp[1024];
		tmp[0]=15;
		tmp[1]=0;
		char *buff=tmp+2;
		
		int l=0;
		if (GetEngineIgnoreList(buff, 1022, &l)==0)
			sock->Send(tmp,l+2);
		else
			sock->Send(answnok, 2);
	
	}
	else
		sock->Send(answnok, 2);

	return 0;
}

int ReloadPaySys()
{
	sem_wait(&payguide::free_workers_lock);
	if (payguide::modules_list==NULL)
	{
		sem_post(&payguide::free_workers_lock);
		return -13;
	}
	
	CUlist<CPaySys> *new_modules_list=LoadModules(payguide::modules_path.c_str());
	if (new_modules_list==NULL)
	{
		sem_post(&payguide::free_workers_lock);
		return -13;
	}
	
	payguide::modules_list->ResetCursor();
	for (unsigned int i=0; i<payguide::modules_list->GetLen(); i++)
	{
		CPaySys *tmp=payguide::modules_list->GetNext();
		if (tmp!=NULL)
			tmp->UnLoad();
	}
	
	new_modules_list->ResetCursor();
	for (unsigned int i=0; i<new_modules_list->GetLen(); i++)
	{
		CPaySys *tmp=new_modules_list->GetNext();
		if (tmp!=NULL)
			payguide::modules_list->AddItem(tmp);
	}
	new_modules_list->ClearToNull();
	payguide::null_pay_sys=GetPaySysBySoName("libnull.so");
	delete new_modules_list;
	sem_post(&payguide::free_workers_lock);
	return 0;
}

int ReConfOperators()
{
	int result=0;
	sem_wait(&payguide::free_workers_lock);
	if (payguide::operators_list!=NULL)
	{
		payguide::operators_list->ResetCursor();
		for (unsigned int i=0; i<payguide::operators_list->GetLen(); i++)
		{
			COperator *tmp=payguide::operators_list->GetNext();
			if (tmp!=NULL)
			{
				tmp->lock.LockWrite();
				result+=tmp->ReloadConfig();
				tmp->lock.UnLockWrite();
			}
		}
	}
	sem_post(&payguide::free_workers_lock);		
	return result;
}

int ReConfPaysys()
{
	int result=0;
	sem_wait(&payguide::free_workers_lock);
	if (payguide::modules_list!=NULL)
	{
		result=ReloadPaySysConfigs();
	}
	sem_post(&payguide::free_workers_lock);		
	return result;
}


int ReloadOperators()
{
	sem_wait(&payguide::free_workers_lock);
	
	CUlist<COperator> *new_operators_list=NULL;
	new_operators_list = LoadOperators(payguide::operators_path.c_str());
	if (new_operators_list!=NULL)
	{
		
		int old_operators_used=0;
		if (payguide::old_operators_list!=NULL)
		{
			payguide::old_operators_list->ResetCursor();
			for (unsigned int i=0; i<payguide::old_operators_list->GetLen(); i++)
			{
				COperator *tmp=payguide::old_operators_list->GetNext();
				if (tmp!=NULL)
				{
					if (tmp->Use()>1)
						old_operators_used++;
					tmp->UnUse(NULL);
				}
			}
		}
		if (old_operators_used>0)
		{
			LogWrite(LOGMSG_NORMAL, "Can't reload operators list - some OLD operators from last reload are still used.");
			delete new_operators_list;
			new_operators_list=NULL;
			sem_post(&payguide::free_workers_lock);
			return old_operators_used;
		}
		else
		{
			if (payguide::old_operators_list!=NULL)
			{
				delete payguide::old_operators_list;
				payguide::old_operators_list=NULL;
			}
			LogWrite(LOGMSG_SYSTEM, "Reloading operators...");
			payguide::old_operators_list=payguide::operators_list;
			payguide::operators_list=new_operators_list;
			LogWrite(LOGMSG_SYSTEM, "operators list sucessfuly reloaded.");
		}
		sem_post(&payguide::free_workers_lock);	
		return 0;
	}
	return -1;

}

void ReloadConfigIfImportant()
{
	sem_wait(&payguide::reload_config_lock);
	if (1==payguide::reload_config)
	{
		ReloadConfigValues("/etc/payguide.cfg");
		payguide::reload_config=0;
	}
	sem_post(&payguide::reload_config_lock);
}

int ReloadConfigValues(char *config_name)
{
	sem_wait(&payguide::free_workers_lock);		
	CConfig *config=NULL;
	config=new CConfig();
	int result=0;
	int db_reconnect_time=-1;
	int db_reconnect_attempts=-1;
	int db_test=0;
	std::string db_select;
	if (config!=NULL)
	{
		if (0==config->ReadFile (config_name))
		{
			if (config->GetValue("thread_min")!=NULL)
				payguide::thread_min=atoi(config->GetValue("thread_min"));
			
			if (config->GetValue("thread_max")!=NULL)
				payguide::thread_max=atoi(config->GetValue("thread_max"));

			if (config->GetValue("thread_inactivity_time")!=NULL)
				payguide::thread_inactivity_time=atoi(config->GetValue("thread_inactivity_time"));
			
			if (config->GetValue("thread_autodelete")!=NULL)
				payguide::thread_autodelete=(int)(strcmp(config->GetValue("thread_autodelete"), "y")==0);
			
			if (config->GetValue("modules_path")!=NULL)
				payguide::modules_path=config->GetValue("modules_path");

			if (config->GetValue("modules_init_path")!=NULL)
				payguide::modules_init_path=config->GetValue("modules_init_path");

			if (config->GetValue("operators_path")!=NULL)
				payguide::operators_path=config->GetValue("operators_path");
			
			if (config->GetValue("db_host")!=NULL)
				payguide::db_host=config->GetValue("db_host");
			
			if (config->GetValue("db_name")!=NULL)
				payguide::db_name=config->GetValue("db_name");
			
			if (config->GetValue("db_user")!=NULL)
				payguide::db_user=config->GetValue("db_user");
			
			if (config->GetValue("db_password")!=NULL)
				payguide::db_password=config->GetValue("db_password");

			if (config->GetValue("perl_module")!=NULL)
				payguide::perl_module=config->GetValue("perl_module");

			if (config->GetValue("net_interface")!=NULL)
				payguide::bind_interface=config->GetValue("net_interface");

			if (config->GetValue("port")!=NULL)
				payguide::pc_port=atoi(config->GetValue("port"));
			
			if (config->GetValue("daemonize")!=NULL)
				payguide::daemonize=(int)(strcmp(config->GetValue("daemonize"), "y")==0);
			
			if (config->GetValue("backtrace_file")!=NULL)
				payguide::backtrace_file=config->GetValue("backtrace_file");
			
			if (config->GetValue("db_reconnect_time")!=NULL)
				payguide::db_reconnect_time=atoi(config->GetValue("db_reconnect_time"));
				
			if (config->GetValue("db_reconnect_attempts")!=NULL)
				payguide::db_reconnect_attempts=atoi(config->GetValue("db_reconnect_attempts"));

			if (config->GetValue("package_timeout")!=NULL)
				payguide::package_timeout=atoi(config->GetValue("package_timeout"));
			
			if (config->GetValue("mysql_select")!=NULL && strcmp(config->GetValue("mysql_select"),"TEST")==0)
				db_test=1;
			else
				db_test=0;
				
			if (config->GetValue("users")!=NULL)
				payguide::users_filename=config->GetValue("users");
				
//			ReloadPaySysConfigs();
		}
		else
		{
			result=1;
		}
		delete config;
		config=NULL;
	}
	else
	{
	    result=2;
	}
	sem_post(&payguide::free_workers_lock);
	
	sem_wait(&payguide::db_lock_read);
	sem_wait(&payguide::db_lock_write);
	string s_db_host=payguide::db_host;
	string s_db_name=payguide::db_name;
	string s_db_user=payguide::db_user;
	
	string s_db_password=payguide::db_password;

	if (db_test==1)
		payguide::db_main_select=payguide::test_db_select;
	else
		payguide::db_main_select=payguide::default_db_select;
	
	//printf("db_main_select=%s\n",db_main_select);
	SetDBConnectionParam(&s_db_host, &s_db_name, &s_db_user, &s_db_password);
	DBSetReconnectTime(db_reconnect_time);
	DBSetReconnectAttempts(db_reconnect_attempts);
	sem_post(&payguide::db_lock_write);
	sem_post(&payguide::db_lock_read);
	return result;
}

int GetThreadsBin(char *buff, int buff_size)
{
	int acc_new_pays=0;
	sem_wait(&payguide::db_lock_read);
	if (payguide::db_accept_new_pays==1) acc_new_pays=1;
	sem_post(&payguide::db_lock_read);


	sem_wait(&payguide::free_workers_lock);

	if (payguide::workers_list==NULL) /* Rebooting */
	{
		sem_post(&payguide::free_workers_lock);
		return 0;
	}

	char *ptr=buff;
	int slp=payguide::workers_list->GetLen()-payguide::working_workers;
	
	memcpy(ptr, &slp, sizeof(slp)); ptr+=sizeof(slp);
	memcpy(ptr, &payguide::working_workers, sizeof(payguide::working_workers)); ptr+=sizeof(payguide::working_workers);
	memcpy(ptr, &payguide::pays_total, sizeof(payguide::pays_total)); ptr+=sizeof(payguide::pays_total);
	
	time_t time_now=time(NULL);
	time_now=time_now-payguide::time_start;
	long long t_n=0;
	memcpy(&t_n, &time_now, sizeof(time_now));
	memcpy(ptr, &t_n, sizeof(t_n)); ptr+=sizeof(t_n);

	ptr[0]=acc_new_pays;

	ptr++;

	payguide::workers_list->ResetCursor();
	for (unsigned int i=0; i<payguide::workers_list->GetLen(); i++)
	{
		CWorker *w=payguide::workers_list->GetNext();
		if (w!=NULL)
		{
			if (w->Busy()==1)
			{
				SPay *pay=w->GetPay();
				pay->lock.LockRead();
				if (pay->test!=NO_TEST)
					ptr[0]=0x81;
				else
					ptr[0]=0x01;
				ptr++;
				long long pid=pay->id;
				pay->lock.UnLockRead();
				int id=w->GetId();
				int lt=w->GetLifeTime();
				char *ps_name=w->GetPaySys()->GetShortName();
				int len=strlen(ps_name);
				memcpy(ptr, &id, sizeof(id)); ptr+=sizeof(id);
				memcpy(ptr, &lt, sizeof(lt)); ptr+=sizeof(lt);
				memcpy(ptr, &pid, sizeof(pid)); ptr+=sizeof(pid);
				memcpy(ptr, &len, sizeof(len)); ptr+=sizeof(len);
				memcpy(ptr, ps_name, len); ptr+=len;
			
			}
			else
			{
				ptr[0]=0;ptr++;
				int id=w->GetId();
				int lt=w->GetLifeTime();
				memcpy(ptr, &id, sizeof(id)); ptr+=sizeof(id);
				memcpy(ptr, &lt, sizeof(lt)); ptr+=sizeof(lt);
			}
		
		}
	}
	sem_post(&payguide::free_workers_lock);
	return ptr-buff;
}

int GetModulesBin(char *buff, int buff_size)
{
	sem_wait(&payguide::free_workers_lock);
	char *ptr=buff;
	payguide::modules_list->ResetCursor();
	for (unsigned int i=0; i<payguide::modules_list->GetLen(); i++)
	{
		CPaySys *w=payguide::modules_list->GetNext();
		if (w!=NULL)
		{
			if (w->IsUnloaded()==0 && w->IsBroken()==0 && w->GetShortName()!=NULL)
			{
				unsigned short len=(unsigned short)strlen(w->GetShortName());
				
				memcpy(ptr, &len, sizeof(unsigned short));
				ptr+=sizeof(unsigned short);
				memcpy(ptr, w->GetShortName(), (int)len);
				ptr+=(int)len;
				
			}
		}
	}
	sem_post(&payguide::free_workers_lock);
	return ptr-buff;
}
