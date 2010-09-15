#include "db.h"
#include "worker.h"
#include "log.h"
#include "core.h"
#include "namespace.h"

#include <iostream>
#include <mysql/mysql.h>
#include <mysql/mysql_com.h>
#include <mysql/errmsg.h>
//#include <mysql/mysqld_error.h>
#include <mysql/mysql_version.h>
#include "paycheck.h"
#include "statistic.h"

#include "xmlpaycheck/cpaycheckcore.h"

#define COM_LEN		1024

#define RESULT_ERROR_UNKNOWN 1000
#define DB_CONNECTION_FIXED -666

//int ACCEPT_NEW_PAYS=1;
static char db_select_additional_conditions[512];
//SPay *CompileNewPay(long long pay_id, int engine, long provider,const char *stamp, const char *data, const char *bill_num, long terminal_id, int currency, float lo_perc, float op_perc, float amount);
static int DBFixConnection(int sql_query_result);

static int db_query(MYSQL *connection, char *query);

struct SPayCash
{
long long id;
long account_id;
long provider_id;
int state;	/*
		0 - in progress
		1 - done
		*/
};

//int loads[3]={1, 3, 5};
//int load=0;
//static int db_reconnect_time=5;
//static int db_reconnect_attempts=0;

static int made_reconnects=0;

static CUlist<SPayCash> *active_pays_list=NULL;	/* List of pays already in progress. */
static CUlist<SPay> *pays_cash=NULL;	/* List of pays already in progress. */

static CUlist<int> *operators_ignore_list=NULL;
static CUlist<int> *engines_ignore_list=NULL;
static sem_t db_ignore_list;

static MYSQL db_read_connection;
static MYSQL db_write_connection;
//MYSQL db_checkreqs_connection;

//string host_saved, db_name_saved, user_saved, passwor_saved;

static MYSQL_RES *quest_res=NULL;
static int any_results=0;
static int db_down=1;

//sem_t db_lock_read;
//sem_t db_lock_write;
//sem_t db_lock_checkreqs;

int DBVeryFirstInit()
{
	mysql_init(&db_read_connection);
	mysql_init(&db_write_connection);
	mysql_init(&payguide::db_checkreqs_connection);
	strncpy(db_select_additional_conditions, "", 511);
	sem_init(&db_ignore_list, 0,1);
	return 0;
}

int DBInit(const string *host, const string *db_name, const string *user, const string *password)
{
	int result=0;
	sem_wait(&payguide::db_lock_read);
	sem_wait(&payguide::db_lock_write);
	payguide::host_saved=*host;
	payguide::db_name_saved=*db_name;
	payguide::user_saved=*user;
	payguide::passwor_saved=*password;
	db_down=1;
	
	if (active_pays_list==NULL)
	{
		active_pays_list=new CUlist<SPayCash>();
		pays_cash=new CUlist<SPay>();
		if (active_pays_list==NULL || pays_cash==NULL) return 13;
	}
	
	if (operators_ignore_list==NULL)
	{
		operators_ignore_list=new CUlist<int>();
	}
	if (engines_ignore_list==NULL)
	{
		engines_ignore_list=new CUlist<int>();
	}

	sem_post(&payguide::db_lock_write);
	sem_post(&payguide::db_lock_read);
	result=DBConnectionUp();
	return result;

}

int DBConnectionUp()
{
	mysql_init(&db_read_connection);
	mysql_init(&db_write_connection);
	mysql_init(&payguide::db_checkreqs_connection);
	MYSQL *tmp1=mysql_real_connect(&db_read_connection, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str(), 0, NULL, 0);
	MYSQL *tmp2=mysql_real_connect(&db_write_connection, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str(), 0, NULL, 0);
	MYSQL *tmp3=mysql_real_connect(&payguide::db_checkreqs_connection, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str(), 0, NULL, 0);
	
		if (tmp1==NULL || tmp2==NULL || tmp3==NULL)
		{
			std::cout << "MySQL connection failed. Please, check out login, password, host and database name options." << std::endl;
			return 2;
		}
	
	int r=db_query(&db_write_connection, "set names koi8r");
	r=r+db_query(&db_read_connection, "set names koi8r");
	r=r+db_query(&payguide::db_checkreqs_connection, "set names koi8r");
	if (r!=0) 
		std::cout << "Failed to execute SQL comand: \"set names koi8r\"" << std::endl;

	db_down=0;
	return 0;
}

int DBConnectionDown()
{
	mysql_close(&db_write_connection);
	mysql_close(&db_read_connection);
	mysql_close(&payguide::db_checkreqs_connection);
	return 0;
}

SPay *DBGetNextPay()
{
SPay *next_pay=NULL;
if (&db_read_connection==NULL) return NULL;

char pay_query[COM_LEN+1];
sem_wait(&db_ignore_list);

//snprintf(pay_query,COM_LEN,"select p.id, p.pay_engine, p.stamp, p.data, p.provider_id, p.bill_num, p.terminal_id, p.account_id, p.currency, 0, 0, p.real_sum, p.back_sum, rand() as rr from pays_big as p left join pays_engines as e on p.pay_engine=e.id where p.fatal=0 and p.gstatus=3 and p.sleep=0 and e.engine_num=10 %sorder by rr limit 150", db_select_additional_conditions);
//snprintf(pay_query,COM_LEN,"select p.id, p.pay_engine, p.stamp, p.data, p.provider_id, p.bill_num, p.terminal_id, p.account_id, p.currency, 0, 0, p.real_sum, p.back_sum, rand() as rr from pays_big as p left join pays_engines as e on p.pay_engine=e.id where p.fatal=0 and p.gstatus=3 and p.sleep=0 %sorder by rr limit 150", db_select_additional_conditions);
snprintf(pay_query,COM_LEN,"%s %sorder by rr limit 150", payguide::db_main_select, db_select_additional_conditions);
//printf("%s\n",pay_query);
sem_post(&db_ignore_list);

int err=0;
if (0==any_results && db_down==0)
{
	sem_wait(&payguide::db_lock_read);
	if (payguide::db_accept_new_pays==0)
	{
		sem_post(&payguide::db_lock_read);
		return NULL;
	}
	
	/* Clearing pays cash list */
	int l=active_pays_list->GetLen();
	active_pays_list->ResetCursor();
	for (int i=0; i<l; i++)
	{
		SPayCash *tmp=active_pays_list->GetNext();
		pays_cash->GetNext();
		if (1==tmp->state) 
		{
			active_pays_list->RemoveThis();
			i--;
			l--;
		}
	}

	
	
	err=db_query(&db_read_connection, pay_query);
	if (0!=err)
	{
		std::cout << "Error while executing SQL request:" << std::endl;
		std::cout << pay_query << std::endl;
		std::cout << "Check your connection to MySQL server and your databases content." << std::endl;
		any_results=0;
		err=mysql_errno(&db_read_connection);
		if (err!=0)
		{
			sem_wait(&payguide::db_lock_write);
			DBFixConnection(err);
			sem_post(&payguide::db_lock_write);
		}
		sem_post(&payguide::db_lock_read);
		return NULL;
	}
	quest_res=mysql_store_result(&db_read_connection);
	if (quest_res!=NULL)
	{
		MYSQL_ROW row;
		unsigned int num_fields=0;
		
		row = mysql_fetch_row(quest_res);
		num_fields = mysql_num_fields(quest_res);
		int pays_num=0;
		while (row!=NULL)
		{
			
			long long id=atoll(row[0]);
			int already_in_progress=0;
			

			unsigned int l=active_pays_list->GetLen();
		
			active_pays_list->ResetCursor();
			for (unsigned int i=0; i<l; i++)
			{
				SPayCash *tmp=active_pays_list->GetNext();
				if (tmp->id==id) {already_in_progress=1; i=l;}
			}
		
			if (already_in_progress==0)
			{
				pays_num++;
				int provider=0;
				long long terminal_id=0;
				long account_id=0;
				int lo_perc=0, op_perc=0;
				float real_summ=0;
				int currency=0;
				int pay_engine=0;
				float back_summ=0;
				float amount=0;
				if (row[1]!=NULL) pay_engine=atoi(row[1]);
				if (row[4]!=NULL) provider=atoi(row[4]);
				if (row[6]!=NULL) terminal_id=atoll(row[6]);
				if (row[7]!=NULL) account_id=atol(row[7]);
				if (row[8]!=NULL) currency=atoi(row[8]);
				if (row[9]!=NULL) lo_perc=atoi(row[9]);
				if (row[10]!=NULL) op_perc=atoi(row[10]);
				if (row[11]!=NULL) real_summ=atof(row[11]);
				if (row[12]!=NULL) back_summ=atof(row[12]);
				if (row[13]!=NULL) amount=atof(row[13]);
				
				SPayCash *tmp_id=new SPayCash();
				if (tmp_id==NULL) return NULL;
				
				SPay *tmp_pay=NULL;
			
				tmp_id->id=id;tmp_id->state=0;tmp_id->account_id=account_id; tmp_id->provider_id=provider;
				
				active_pays_list->AddItem(tmp_id);

				/* Compile a new job and put it on cash */
				tmp_pay=CompileNewPay(id, pay_engine, provider, row[2], row[3], row[5], terminal_id, currency, lo_perc, op_perc, real_summ, back_summ,amount);
				pays_cash->AddItem(tmp_pay);

			
			}
			else
			{
				next_pay=NULL;
//				std::cout << "Pay " << id << " already in progress..." << std::endl;
			}
			row = mysql_fetch_row(quest_res);
		}
		mysql_free_result(quest_res);
		if (pays_num!=0)
		{
			
			any_results=1;
		}
	}
	sem_post(&payguide::db_lock_read);	
}

if (any_results==1)
{
	pays_cash->ResetCursor();
	SPay *tmp=pays_cash->GetNext();
	if (tmp!=NULL)
	{
		next_pay=new SPay (*(tmp));
		if (next_pay==NULL) return NULL;
		
		/* UPDATE CHECK REQS */
		sem_wait(&payguide::db_lock_checkreqs);

		snprintf(pay_query, COM_LEN, "select pay_id from check_reqs where tid=%lli and bill_num='%s' limit 1",next_pay->terminal_id, next_pay->bill_num);
		err=db_query(&payguide::db_checkreqs_connection, pay_query);

		MYSQL_RES *check_res=mysql_store_result(&payguide::db_checkreqs_connection);
		if (check_res!=NULL)
		{
			bool check_reqs_need_update=false;
			MYSQL_ROW row;
			unsigned int num_fields=0;
			row = mysql_fetch_row(check_res);
			if (row!=NULL)
			{
				next_pay->exists_int_checkreqs=1;
				num_fields = mysql_num_fields(check_res);
//				int pays_num=0;
				if (row[0]==NULL)
				{
					check_reqs_need_update=true;
				}
					
			}
			mysql_free_result(check_res);

			if (check_reqs_need_update)
			{
//				printf("UPDATE\n");
				snprintf(pay_query, COM_LEN, "update check_reqs set pay_id=%lli where tid=%lli and bill_num='%s' and pay_id is NULL limit 1",next_pay->id, next_pay->terminal_id, next_pay->bill_num);
				err=db_query(&payguide::db_checkreqs_connection, pay_query);
			}
		}

		sem_post(&payguide::db_lock_checkreqs);
//		printf("check_reqs update done\n");
		/* END OF UPDATE */
		
		pays_cash->RemoveThis();
	}
	else
	{
		any_results=0;
		return NULL;
	}
	
	/* if cash is empty - request pays from db */
	if (pays_cash->GetLen()==0) any_results=0;
}

//std::cout << "-------------------------" << std::endl;
return next_pay;

}

int DBShutdown()
{

	sem_wait(&payguide::db_lock_read);
	sem_wait(&payguide::db_lock_write);
	DBConnectionDown();
	
	if (active_pays_list!=NULL) delete active_pays_list;
	active_pays_list=NULL;
	
	
	if (pays_cash!=NULL) delete pays_cash;
	pays_cash=NULL;

	if (operators_ignore_list!=NULL) delete operators_ignore_list;
	operators_ignore_list=NULL;
	
	if (engines_ignore_list!=NULL) delete engines_ignore_list;
	engines_ignore_list=NULL;

	sem_post(&payguide::db_lock_write);
	sem_post(&payguide::db_lock_read);
	return 0;
}




//int DBSetState(SPay *pay, int result, int sleep, const char *msg, const char *sender_name, SPayResult *payres)
int DBSetState(SPay *pay, int result,int sleep, const char *msg, const char *sender_name)
{
	SPayResult payres;
	payres.code=result;
	if (msg!=NULL)
		strncpy(payres.msg, msg, SIZE_REPONSE_MSG);
	if (sender_name!=NULL)
		strncpy(payres.sender_name, sender_name, SIZE_REPONSE_SENDER);
	payres.sleep=sleep;
	return DBSetState(pay, &payres);
}
int DBSetState(SPay *pay, SPayResult *payres)
{
	/*char logmsg[101];
	if (result==RESULT_SUCESS)
		snprintf(logmsg, 100,"Pay %lli finished with code %i (SUCESS)", pay->id, result);
	else if (result==RESULT_SAVE_STATE)
		snprintf(logmsg, 100,"Pay %lli finished with code %i (SLEEP)", pay->id, result);
	else if (result==RESULT_DO_NOTHING)
		snprintf(logmsg, 100,"Pay %lli finished with code %i (DO NOTHING)", pay->id, result);
	else if (result==RESULT_FAILED)
		snprintf(logmsg, 100,"Pay %lli finished with code %i (FATAL)", pay->id, result);
	else
		snprintf(logmsg, 100,"Pay %lli finished with code %i", pay->id, result);


	SendLogMessages(0, logmsg);
	*/
//	StatisticAddPay(pay, result, sleep, msg, sender_name);
	long long int id = pay->id;
	int write_log=1;
	if (strcmp(payres->msg,"")==0)
		write_log=0;
	
	SPayCash *tmp=NULL;
	
	if (pay->test!=NO_TEST)
	{
		if (pay->xml_output && payguide::xml_paycheck_core!=NULL)
		{
			printf("XML\n");
			return payguide::xml_paycheck_core->XMLSetState(pay, payres);
		}
		else
			return PCSetState(pay, payres);
	}
	
	sem_wait(&payguide::db_lock_read);
	if (db_down)
	{
		sem_post(&payguide::db_lock_read);
		return -10;
	}
	
	unsigned int l=active_pays_list->GetLen();
	active_pays_list->ResetCursor();
	for (unsigned int i=0; i<l; i++)
	{
		tmp=active_pays_list->GetNext();
		if (tmp->id==id) 
			i=l;
		else
			tmp=NULL;
	}
	

	if (tmp==NULL)
	{
		sem_post(&payguide::db_lock_read);
		return -1;
	}
	
	sem_post(&payguide::db_lock_read);
	
	int err=0;
	char pay_query[COM_LEN+1]="";
	char log_msg[COM_LEN+1]="";
	snprintf(pay_query, COM_LEN, "update pays_main set gstatus=%i where id=%lli",50,id);
	snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', 'Произошла неизвестная ошибка.')",id,payres->sender_name);
	
	sem_wait(&payguide::db_lock_write);
	
	/* Pay complete sucessful */
	if (payres->code==RESULT_SUCESS)					
	{
		//snprintf(log_msg, COM_LEN, "insert into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', 'Платеж успешно зачислен')",id,sender_name);
		snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', '%s')",id,payres->sender_name,payres->msg);
		snprintf(pay_query, COM_LEN,"update pays_main set gstatus=3000, fatal=0 where id=%lli", id);
	}
	
	/* Pay failed */
	else if (payres->code==RESULT_FAILED)
	{
		snprintf(pay_query, COM_LEN, "update pays_main set gstatus=105, fatal=1 where id=%lli",id);
		snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', '%s')",id,payres->sender_name,payres->msg);
	}
	
	/* Pay killed */
	else if (payres->code==RESULT_KILLED)				
	{
		snprintf(pay_query, COM_LEN, "update pays_main set gstatus=105, fatal=1 where id=%lli",id);
		snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', 'Платеж был убит')",id,payres->sender_name);
		write_log=1;
	}	
	else if (payres->code==RESULT_31)
	{
		snprintf(pay_query, COM_LEN, "update pays_main set gstatus=31, fatal=0, sleep=%i where id=%lli", payres->sleep ,id);
		snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', '%s')",id,payres->sender_name, payres->msg);
		write_log=1;
	}
	/* Pay canceled */
	else if (payres->code==RESULT_CANCELED)
	{
		snprintf(pay_query, COM_LEN, "update pays_main set gstatus=105, fatal=1 where id=%lli",id);
		snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', 'Платеж был отменен')",id,payres->sender_name);
		write_log=1;
	}
	/* Requested PaySys (Pay Engine, or *.so file) not found */
	else if (payres->code==RESULT_NO_PAY_SYS)					
	{
		snprintf(pay_query, COM_LEN, "update pays_main set gstatus=105, fatal=1 where id=%lli",id);	
		snprintf(log_msg, COM_LEN, "insert into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', 'Неправильный оператор/платежная система')",id,payres->sender_name);
		write_log=1;
	}
	/* Do nothing with curent state, normal usage - with sleep>0 (with sleep=0 at your own risc!) */
	else if (payres->code==RESULT_SAVE_STATE)
	{
/*		if (write_log)
			snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', '%s')",id,payres->sender_name, payres->msg);*/
			snprintf(pay_query, COM_LEN, "update pays_main set sleep=%i where id=%lli",payres->sleep,id);
			snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', '%s')",id,payres->sender_name, payres->msg);

	}
	/* Pay canceled by Electropay payment gate (gstatus=101) */
	else if (payres->code==RESULT_101)
	{
		snprintf(pay_query, COM_LEN, "update pays_main set gstatus=101, fatal=1 where id=%lli",id);
		snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', '%s')",id,payres->sender_name, payres->msg);
	}
	else if (payres->code==RESULT_106)
	{
		snprintf(pay_query, COM_LEN, "update pays_main set gstatus=106, fatal=1 where id=%lli",id);
		snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', '%s')",id,payres->sender_name, payres->msg);
	}
	else if (payres->code==RESULT_109)
	{
		snprintf(pay_query, COM_LEN, "update pays_main set gstatus=109, fatal=1 where id=%lli",id);
		snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', '%s')",id,payres->sender_name, payres->msg);
	}
	else if (payres->code==RESULT_DO_NOTHING)
	{
		write_log=0;
	}
	

	/* Hmmm, paysys returns wrong error code! */
	else
	{
		payres->code=RESULT_ERROR_UNKNOWN;
		snprintf(pay_query, COM_LEN, "update pays_main set gstatus=105, fatal=1 where id=%lli",id);
		snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', 'Модуль вернул неожиданый код ошибки.')",id,payres->sender_name);
		write_log=1;
		char tmp[200];snprintf(tmp, 199, "Модуль %s вернул неожиданный код ошибки. Срочно исправте модуль.')",payres->sender_name);
		LogWrite(LOGMSG_ERROR, tmp);
	}

/*	if (payres->sleep>0)
	{
		if (payres->code==RESULT_SAVE_STATE)
		{
			snprintf(pay_query, COM_LEN, "update pays_main set sleep=%i where id=%lli",payres->sleep,id);
			snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', '%s')",id,payres->sender_name, payres->msg);
		}
		else if (payres->code==RESULT_31)
		{
		}
		else
		{
			snprintf(pay_query, COM_LEN, "update pays_main set sleep=%i where id=%lli",payres->sleep,id);
			snprintf(log_msg, COM_LEN, "insert delayed into pays_logg(pay_id, stamp, user, note) values(%lli, now(), '%s', 'Получен код результата [%i] и sleep>0 (!!!). Сообщите разработчику модуля [%s]')",id,payres->sender_name, payres->code,payres->sender_name);
		}
	}*/

	if (err==0)
	{
		/* Don't update BD if payres->code=RESULT_SAVE_STATE and sleep=0 */
		if (!(RESULT_SAVE_STATE==payres->code && 0==payres->sleep) && payres->code!=RESULT_DO_NOTHING)
			err=db_query(&db_write_connection, pay_query);
	
		if (err==0 && write_log)
			err=db_query(&db_write_connection, log_msg);
		
		/* ...and if pay passed, update pays_ext finally */
		if (err==0 && payres->code==RESULT_SUCESS)
		{
				snprintf(pay_query, COM_LEN,"insert into pays_ext(pay_id, utime, gstatus) values(%lli, unix_timestamp(now()), 3000)", id);
				err=db_query(&db_write_connection, pay_query);
		}
	}
	if (err!=0)
	{
			sem_wait(&payguide::db_lock_read);
			err=mysql_errno(&db_write_connection);
			err=DBFixConnection(err);
			sem_post(&payguide::db_lock_read);
	}

	sem_post(&payguide::db_lock_write);
	if (err==DB_CONNECTION_FIXED) err=DBSetState(pay, payres);
	
	/* Mark pay in active pays list as complited. */
	sem_wait(&payguide::db_lock_read);
	if (tmp!=NULL)
		tmp->state=1;
	sem_post(&payguide::db_lock_read);
	
	return err;
}

int DBFixConnection(int sql_query_result)
{
	if (0==sql_query_result) return 0;
	else if (CR_SERVER_GONE_ERROR==sql_query_result || CR_SERVER_LOST==sql_query_result && 0==db_down)
	{
		made_reconnects=1;
		DBConnectionDown();
		LogWrite(LOGMSG_SYSTEM, "Restoring MySQL connnection by DBFixConnection()...");
		int result=DBConnectionUp();
		while (result!=0 && (made_reconnects<payguide::db_reconnect_attempts || payguide::db_reconnect_attempts<0))
		{
			sleep(payguide::db_reconnect_time);
			DBConnectionDown();
			LogWrite(LOGMSG_SYSTEM, "Restoring MySQL connnection by DBFixConnection()...");
			result=DBConnectionUp();
			if (!(payguide::db_reconnect_attempts<0)) made_reconnects++;
		}
		/* Connection restored */
		if (result==0) 
		{
			LogWrite(LOGMSG_SYSTEM, "MySQL connection failed, but RESTORED automatical by DBFixConnection()");
			return DB_CONNECTION_FIXED;
		}
		
		/* Failed to restore */
		std::cout << "MySQL connection failed and NOT RESTORED by DBFixConnection()" << std::endl;
		LogWrite(LOGMSG_ERROR, "MySQL connection failed and NOT RESTORED by DBFixConnection()");
		DBConnectionDown();
		db_down=1;
		return 1;
	}
	
	return 1;
}

void DBSetReconnectAttempts(int new_value)
{
	payguide::db_reconnect_attempts=new_value;
}

void DBSetReconnectTime(int new_time)
{
	if (new_time<=0) new_time=1;
	if (new_time >300) new_time=300;
	payguide::db_reconnect_time=new_time;
}

void SetDBConnectionParam(const string *new_host, const string *new_db_name, const string *new_user, const string *new_password)
{
	payguide::host_saved=*new_host;
	payguide::db_name_saved=*new_db_name;
	payguide::user_saved=*new_user;
	payguide::passwor_saved=*new_password;
}

int db_query(MYSQL *connection, char *query)
{
	return my_simple_db_query(connection, query);
//	return my_db_query(connection, query, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str());
}

int CompileAdditionalSelectCase()
{
	
	strncpy(db_select_additional_conditions, "", 511);

	char tmp[16];
	if (operators_ignore_list!=NULL && operators_ignore_list->GetLen()!=0)
	{
		strncat(db_select_additional_conditions, "and p.provider_id not in(", 511);
		operators_ignore_list->ResetCursor();
		unsigned int l=operators_ignore_list->GetLen();
		for (unsigned int i=0; i<l; i++)
		{
			int *t=operators_ignore_list->GetNext();
			if (t!=NULL)
			{
				if (i+1<l)
					snprintf(tmp, 15, "%i,", *t);
				else
					snprintf(tmp, 15, "%i) ", *t);
				
				strncat(db_select_additional_conditions, tmp, 511);
			}
		}
	}
		
	if (engines_ignore_list!=NULL && engines_ignore_list->GetLen()!=0)
	{
		strncat(db_select_additional_conditions, "and p.pay_engine not in(", 511);
		engines_ignore_list->ResetCursor();
		unsigned int l=engines_ignore_list->GetLen();
		for (unsigned int i=0; i<l; i++)
		{
			int *t=engines_ignore_list->GetNext();
			if (t!=NULL)
			{
				if (i+1<l)
					snprintf(tmp, 15, "%i,", *t);
				else
					snprintf(tmp, 15, "%i) ", *t);
					
				strncat(db_select_additional_conditions, tmp, 511);
			}
		}
	}
	return 0;
}

int AddToIgnoreList(int id, CUlist<int> *list)
{
	int *t=new int();
	*t=id;
	sem_wait(&db_ignore_list);
	if (list==NULL)
		list=new CUlist<int>();
	list->AddItem(t);
	CompileAdditionalSelectCase();
	sem_post(&db_ignore_list);
	return 0;

}


int RemoveFromIgnoreList(int id, CUlist<int> *list)
{
	sem_wait(&db_ignore_list);
	if (list==NULL)
	{
		sem_post(&db_ignore_list);
		return -1;
	}

	list->ResetCursor();
	for (unsigned int i=0; i<list->GetLen(); i++)
	{
		int *t=list->GetNext();
		if (t!=NULL)
		{
			if (*t==id)
				list->RemoveThis();
		}
	}
	CompileAdditionalSelectCase();
	sem_post(&db_ignore_list);
	return 0;

}


int ClearIgnoreList(CUlist<int> *list)
{
	if (list==NULL)
		return -1;

	sem_wait(&db_ignore_list);
//	delete *list;
//	*list=NULL;
	list->ResetCursor();
	for (unsigned int i=0; i<list->GetLen(); i++)
	{
		int *t=list->GetNext();
		if (t!=NULL)
		{
			list->RemoveThis();
		}
	}

	CompileAdditionalSelectCase();
	sem_post(&db_ignore_list);
	return 0;
	
}

int GetIgnoreList(CUlist<int> *list, char *buff, int buff_size, int *l)
{
	if (list==NULL)
		return 0;
	char *ptr=buff;
	sem_wait(&db_ignore_list);
	if ((unsigned int)buff_size<=sizeof(list->GetLen()))
	{
		sem_post(&db_ignore_list);
		return -1;
	}

	unsigned int len=list->GetLen();
	list->ResetCursor();
	memcpy(ptr, &len, sizeof(list->GetLen()));
	ptr+=sizeof(list->GetLen());
	for (unsigned int i=0; i<list->GetLen(); i++)
	{
		int *t=list->GetNext();
		if (t!=NULL)
		{
			memcpy(ptr, t, sizeof(int));
			ptr+=sizeof(int);
			if ((unsigned int)(ptr-buff)>(buff_size-sizeof(int)))
			{
				sem_post(&db_ignore_list);
				return -1;
			}
		}
	}
	*l=ptr-buff;
	sem_post(&db_ignore_list);
	return 0;

}

int AddOperatorToIgnoreList(int operator_id)
{
	return AddToIgnoreList(operator_id, operators_ignore_list);
}

int AddEngineToIgnoreList(int engine_id)
{
	return AddToIgnoreList(engine_id, engines_ignore_list);
}
int RemoveOperatorFromIgnoreList(int operator_id)
{
	return RemoveFromIgnoreList(operator_id, operators_ignore_list);
}

int RemoveEngineFromIgnoreList(int engine_id)
{
	return RemoveFromIgnoreList(engine_id, engines_ignore_list);
}

int ClearOperatorIgnoreList()
{
	return ClearIgnoreList(operators_ignore_list);
}

int ClearEngineIgnoreList()
{
	return ClearIgnoreList(engines_ignore_list);
}

int GetOperatorIgnoreList(char *buff, int buff_size, int *l)
{
	return GetIgnoreList(operators_ignore_list, buff, buff_size, l);
}

int GetEngineIgnoreList(char *buff, int buff_size, int *l)
{
	return GetIgnoreList(engines_ignore_list, buff, buff_size, l);
}

