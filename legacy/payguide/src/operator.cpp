#include "operator.h"
#include "parser.h"
#include "log.h"
#include "core.h"
#include "namespace.h"

#include <iostream>
#include <errno.h>
#include <string>

#include <mysql/mysql.h>
#include <mysql/mysql_com.h>
#include <mysql/errmsg.h>
#include <mysql/mysql_version.h>
#include <mysql/mysqld_error.h>

static long GetRest(const char *rest_id);

static CConfig *rules=NULL;
static CConfig *engines=NULL;

static CConfig *limits=NULL;
static int limits_loaded=0;
static MYSQL op_mysql;
static int op_mysql_need_free=0;
static sem_t op_mysql_lock;
static int bad_connection=0;
/*string user;
string pass;
string host;
string db_name;*/


int OperatorsInit()
{
	sem_init(&op_mysql_lock,0,1);
	return 0;
}

int OperatorsShutdown()
{
	mysql_close(&op_mysql);
	return 0;
}



COperator::COperator()
{
    //valid_pay_sys=new CUlist<CPaySys>;
    wre=NULL;
    bre=NULL;
    operator_id=-1;
    sem_init(&sem, 0,1);
    op_config=NULL;
    filename="";
    use_count=0;
    white_data_format=NULL;
    black_data_format=NULL;
}

COperator::~COperator()
{
//	if (valid_pay_sys!=NULL)
//	{
//		valid_pay_sys->ClearToNull();
//		delete valid_pay_sys;valid_pay_sys=NULL;
//	}
	if (op_config!=NULL)
	{
		delete op_config;
		op_config=NULL;
	}
		
	if (wre!=NULL)
	{
		pcre_free(wre);
		wre=NULL;
	}

	if (bre!=NULL)
	{
		pcre_free(bre);
		bre=NULL;
	}
		
//	if (rules!=NULL)
//	{
//		delete rules;
//		rules=NULL;
//	}
}

pcre *COperator::GetWhiteFormat()
{
	return wre;
}

pcre *COperator::GetBlackFormat()
{
	return bre;
}

	//pcre *re = pcre_compile(format, 0, &error,&erroffset,NULL);
	
	//if (re==NULL)
	//{
	//	printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
	//	return 1;
	//}


CConfig *COperator::GetConfig()
{
	return op_config;
}

int COperator::GetOperatorId()
{
    return operator_id;
}

int COperator::ReloadConfig()
{
	return Load(filename.c_str());
	
}

int COperator::Load(const char *file_name)
{
filename=file_name;
	
//	char buff[100];
//	const char *value;
//	int done=1;
	int num=0;
//	int err;
//	std::cout << "Loading " << file_name << "..." << std::endl;
	CConfig *config=new CConfig();
	if (config==NULL) return 2;
	
	if (op_config!=NULL)
		delete op_config;
	op_config=config;
	
	if (wre!=NULL)
	{
		pcre_free(wre);
		wre=NULL;
	}

	if (bre!=NULL)
	{
		pcre_free(bre);
		bre=NULL;
	}

	if (config->ReadFile(file_name)==0)
	{
	
		
		operator_id=atoi(config->GetValue("operator_id"));
		white_data_format=config->GetValue("white_list");
		black_data_format=config->GetValue("black_list");
		
		const char *error;
		int erroffset;

		
		if (white_data_format!=NULL)
		{
			wre = pcre_compile(white_data_format, 0, &error,&erroffset,NULL);
			if (wre==NULL)
			{
				char tmp[1024];
				snprintf(tmp, 1024,"PCRE compilation failed at offset %d: %s in %s (white list)\n", erroffset, error, file_name);
				LogWrite(LOGMSG_WARNING, tmp);
			}
		}
		
		if (black_data_format!=NULL)
		{
			bre = pcre_compile(black_data_format, 0, &error,&erroffset,NULL);
			if (bre==NULL)
			{
				char tmp[1024];
				snprintf(tmp, 1024,"PCRE compilation failed at offset %d: %s in %s (black list)\n", erroffset, error, file_name);
				LogWrite(LOGMSG_WARNING, tmp);
			}
		}
		
/*		while (done)
		{
			
			char t[100];
			if (num==0)sprintf(t, "%s","paysys"); else sprintf(t, "%s%i","paysys_",num);
			value=config->GetValue(t);

			num++;
			if (value==NULL) done=0;
			else
			{
				if (rules!=NULL)
				{
					if (rules->GetValue(value)!=NULL)
						value=rules->GetValue(value);
						
				}
				else
				{
					LogWrite(LOGMSG_WARNING, "No 000_rules.cfg specified.");
				}
				strncpy(buff,value,99);
				CPaySys *tmp = GetPaySysBySoName(value);
				if (tmp!=NULL && tmp!=default_paysys)
				{
					std::string s1= "Pay system "; s1+=value; s1+=" is aviable for "; s1+=file_name;
					LogWrite(LOGMSG_SYSTEM,&s1);
					
//					valid_pay_sys->AddItem(tmp);
				}
				else
				{
//					std::cout << "Can't load " << value << ", using " << default_paysys->GetShortName() << std::endl;
					std::string s1= "Can't load "; s1+=value; s1+=" using libnull.so ";
					LogWrite(LOGMSG_WARNING,&s1);

//					valid_pay_sys->AddItem(default_paysys);
				}
				
			}
		}*/
		
	}
	else
	{
//		std::cout << "Can't read " << file_name << std::endl;
		std::string s1= "Can't read "; s1+=file_name;
		LogWrite(LOGMSG_SYSTEM,&s1);
	}
	//delete config; config=NULL;
	if (num==1) return 1;
	return 0;
}

CPaySys *COperator::GetValidPaySys(SPay *pay)
{
	CPaySys *t=NULL;
	
	if (pay==NULL)
		return GetPaySysDefault();
		
	if (op_config==NULL)
		return GetPaySysDefault();
	
	/* GET CHECKSYS */
	if (pay->test!=NO_TEST)
	{
		const char *testtype=NULL;
		testtype=op_config->GetValue("checktype");
		if (testtype!=NULL)
		{
			if (strcmp(testtype,"full")==0)
				pay->test=FULL_TEST;
			else if (strcmp(testtype,"format")==0)
			{
				pay->test=FORMAT_TEST;
				return GetPaySysDefault();
			}
			else if (strcmp(testtype,"so_only")==0)
			{
				pay->test=CHECKSYS_TEST;
			}
			else if (strcmp(testtype,"ok")==0)
			{
				pay->test=OK_TEST;
				return GetPaySysDefault();
			}
			else if (strcmp(testtype,"nok")==0)
			{
				pay->test=NOK_TEST;
				return GetPaySysDefault();
			}
		}
	}
	
	if (pay->test!=NO_TEST)
	{
		const char *nm=op_config->GetValue("checksys");
		if (nm!=NULL)
		{
		

			if (rules!=NULL)
			{
				if (rules->GetValue(nm)!=NULL)
				{
					t=GetPaySysBySoName(rules->GetValue(nm));
					pay->paysys_codename=nm;
				}
			}
			else
			{
				t=GetPaySysBySoName(nm);
				pay->paysys_codename=nm;
			}

			if (t==NULL)
				return GetPaySysDefault();

			else if (t->IsBroken() || t->IsUnloaded())
				return GetPaySysDefault();
			
			nm=op_config->GetValue("checksum");
			if (nm!=NULL && pay->summ==-1)
				pay->summ=atof(nm);

			return t;
		}
	}
	
	/* FOR OLD CONFIGS (WITH paysys param) */
	const char *nm=op_config->GetValue("paysys");
	
	if (nm!=NULL)
	{
		

		if (rules!=NULL)
		{
			if (rules->GetValue(nm)!=NULL)
				t=GetPaySysBySoName(rules->GetValue(nm));
		}
		else
			t=GetPaySysBySoName(nm);

		if (t==NULL)
			return GetPaySysDefault();

		else if (t->IsBroken() || t->IsUnloaded())
			return GetPaySysDefault();

		return t;
	}
	
	
	
	else
	{

		/* FOR NEW CONFIGS (WITH paysys_XXX params) */
		int e_num=-1;
		int i=1;
		char tmp_name_buff[200];
		const char *nm="libnull.so";
		while (e_num!=pay->pay_engine && e_num<32000)
		{
			snprintf(tmp_name_buff, 99, "paysys_%i", i); 
			i++;
		
			nm=op_config->GetValue(tmp_name_buff);

			if (nm!=NULL)
			{
				
				const char *e_num_value=engines->GetValue(nm);
				if (e_num_value!=NULL)
				{
					e_num=atoi(e_num_value);
					
				}
				else
				{
					e_num=-1;
				}
				pay->paysys_codename=nm;
			}
			else 
				return GetPaySysDefault();

	
		}

		/* Limits */
		const char *limit=NULL;
		const char *rests_id=NULL;
		const char *rests_min=NULL;
		
		if (limits!=NULL)
		{
			limit=limits->GetValue("value",nm);
			rests_id=limits->GetValue("rest_id",nm);
			rests_min=limits->GetValue("rest_min",nm);
		}
		
		long rst_min=0;
		if (rests_min!=NULL)
			rst_min=atol(rests_min);
			
		if (limit!=NULL && limits!=NULL)
		{
			if (strcmp(limit, "n")!=0)
			{
				int lim=atoi(limit);
				lim--;
				char limit_str[16];
				snprintf(limit_str, 15, "%i", lim);
				limits->SetValue("value",nm,limit_str);

				if (lim<=-1 || GetRest(rests_id)-rst_min<=0)
				{
					return GetPaySysEngineLimit();
				}
			}
		}
		/* Loading "true" *.so file from engines */
		if (rules!=NULL)
		{
			const char *true_name=rules->GetValue(nm);
			if (true_name!=NULL)
			{
				pay->conv_sub=op_config->GetValue("conv_sub",true_name);

				t=GetPaySysBySoName(true_name);
			}
		}
		else
			t=GetPaySysBySoName(op_config->GetValue(tmp_name_buff));

		if (t==NULL)
			return GetPaySysDefault();

		else if (t->IsBroken() || t->IsUnloaded())
			return GetPaySysDefault();

	}
	return t;
}

CUlist<COperator> *LoadOperators(const char *dir)
{
	
	sem_wait(&op_mysql_lock);
	if (op_mysql_need_free)
		mysql_close(&op_mysql);
	op_mysql_need_free=0;
	if (NULL==mysql_real_connect(&op_mysql, "localhost", "pays_daemon", "masteranubis", "electropay_db1", 0, NULL, 0))
		bad_connection=1;	
	else
		op_mysql_need_free=1;
	sem_post(&op_mysql_lock);

	if (rules!=NULL)
	{
		delete rules;
		rules=NULL;
	}
	
	if (engines!=NULL)
	{
		delete engines;
		engines=NULL;
	}

	if (limits!=NULL && dir==NULL)
	{
		delete limits;
		limits=NULL;
		limits_loaded=0;
	}
	
	/* If NULL passed as argument - delete rules, engines and exit */
	if (dir==NULL)
	{
		return NULL;
	}
		
	rules=new CConfig();
	if (rules==NULL) return NULL;
	std::string RULES_file=dir; RULES_file+="000_rules.cfg";
	if (rules->ReadFile (RULES_file.c_str())!=0)
	{
		delete rules;
		rules=NULL;
		LogWrite(LOGMSG_WARNING, "Can't load 000_rules.cfg");
	}

	engines=new CConfig();
	if (engines==NULL) return NULL;
	std::string ENGINES_file=dir; ENGINES_file+="000_engines.cfg";
	if (engines->ReadFile (ENGINES_file.c_str())!=0)
	{
		delete engines;
		engines=NULL;
		LogWrite(LOGMSG_WARNING, "Can't load 000_engines.cfg");
	}
	
	if (limits_loaded==0)
	{
		limits=new CConfig();
		if (limits==NULL) return NULL;
		std::string LIMITS_file=dir; LIMITS_file+="000_limits.cfg";
		if (limits->ReadFile (LIMITS_file.c_str())!=0)
		{
			delete limits;
			limits=NULL;
			LogWrite(LOGMSG_WARNING, "Can't load 000_limits.cfg");
		}
		else
			limits_loaded=1;
	}

	
	const char ext[]="op";
	CUlist<COperator> *result = new CUlist<COperator>;
	if (result==NULL) return NULL;
	
	DIR *dir_opened=NULL;
	dirent *cursor=NULL;
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
			if (add)
			{
				COperator *tmp=new COperator();
				if (tmp==NULL) return NULL;
				
				char buff[100];
				strncpy(buff, dir,49);
				strncat(buff, cursor->d_name,49);
				if (tmp->Load(buff)==0) result->AddItem(tmp);
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
//		std::cout << "Can't open dir " << dir << std::endl;
		char tmp [200];
		snprintf(tmp, 200, "Can't open dir %s, error %i", dir, errno);
		LogWrite(LOGMSG_WARNING, tmp);
	}
	return result;
}


int COperator::UnUse(const char *engine_name)
{
	/* Increase limit value */
	if (engine_name!=NULL && limits!=NULL)
	{
		const char *limit=limits->GetValue("value",engine_name);
		if (limit!=NULL)
		{
			if (strcmp(limit,"n")!=0)
			{
				int lim=atoi(limit);
				lim++;
				char limit_str[16];
				snprintf(limit_str, 15, "%i", lim);
				limits->SetValue("value",engine_name,limit_str);
			}
		}
	}
	if (use_count>0) use_count--;
	return use_count;
}

int COperator::Use()
{
	use_count++;
	return use_count;
}


long GetRest(const char *rest_id)
{
	if (rest_id==NULL)
		return 1;
	MYSQL_RES *quest_res=NULL;
	long result=1;
	char query[512];
	snprintf(query, 511, "select value from rests where id=%s", rest_id);

	sem_wait(&op_mysql_lock);
	
	if (bad_connection)
	{
		sem_post(&op_mysql_lock);
		return 1;
	}
	
	int err=my_db_query(&op_mysql, query,"localhost", "pays_daemon", "masteranubis", "electropay_db1");
	if (0!=err)
	{
		sem_post(&op_mysql_lock);
		return 1;
	}
	
	quest_res=mysql_store_result(&op_mysql);
	if (quest_res!=NULL)
	{
		MYSQL_ROW row;
		row = mysql_fetch_row(quest_res);
		//int pays_num=0;
		if (row!=NULL)
		{	
			result=atol(row[0]);
			
		}
		mysql_free_result(quest_res);
	}
	
	sem_post(&op_mysql_lock);
	return result;
}

