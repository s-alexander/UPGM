#ifndef __PayguideNameSpace__
#define __PayguideNameSpace__

/*
 * Namespace "payguide" contains some variables, that are used in a large amount of *.cpp files:
 * settings, locks, connection to MySQL etc.
 */

#include <semaphore.h>
#include <mysql/mysql.h>
#include <mysql/mysql_com.h>
#include <mysql/errmsg.h>
#include "ulist.h"
#include "paysys.h"
#include "operator.h"
#include "worker.h"
#include "settings.h"
#include "perlm.h"

namespace paycheck
{
	class CPaycheckCore;
}

namespace payguide
{
	/* GLOBAL */
	
	/* Global payguide lock */
	extern sem_t free_workers_lock;
	
	/* Lock for working and quit var */
	extern sem_t shutdown_lock;
	
	/* True if we shoud work */
	extern bool working;
	
	/* True if we shoud termianate */
	extern bool quit;
	
	extern int pc_port;
	
	extern int daemonize;

	/* DB */
	
	/* Mysql select operator string */
	extern const char *default_db_select;
	extern const char *test_db_select;
	extern const char *db_main_select;

	/* Locks for operation with DB */
	extern sem_t db_lock_read;
	extern sem_t db_lock_write;
	
	/* Shoud we accept new pays from electropay_db1.pays_big? */
	extern int db_accept_new_pays;
	
	/* Lock and mysql connection for operation with checkreqs db */
	extern sem_t db_lock_checkreqs;
	extern MYSQL db_checkreqs_connection;

	/* Shoud we reload main payguide config? And lock for this. */
	extern sem_t reload_config_lock;
	extern int reload_config;
	

	extern CUlist<CWorker> *workers_list;
	extern CWorker *free_worker;
	extern unsigned int working_workers;
	extern unsigned int thread_max;
	extern unsigned int thread_min;
	extern int thread_inactivity_time;
	extern int thread_autodelete;

	extern CUlist<COperator> *operators_list;
	extern CUlist<COperator> *old_operators_list;

	extern CUlist<CPaySys> *modules_list;
	extern CPaySys *null_pay_sys;

	extern CPerlModule data_conventor;

	extern std::string modules_path;
	/* If set to 1, there are some old modules loaded modules. We need to check if they are not in use and unload them */

	extern int need_cleanup_modules;
	
	
	extern std::string host_saved, db_name_saved, user_saved, passwor_saved;
	extern int package_timeout;

	extern long long pays_total;
	
	/* Some settings */
	extern std::string modules_path;
	extern std::string modules_init_path;
	extern std::string operators_path;
	extern std::string perl_module;
	extern std::string bind_interface;
	
	extern int db_reconnect_time;
	extern int db_reconnect_attempts;

	/* default backtrace file */
	extern std::string backtrace_file;

	/* defaul db host */
	extern std::string db_host;

	/* default db name */
	extern std::string db_name;

	/* default db user name */
	extern std::string db_user;

	/* default db password */
	extern std::string db_password;

	/* full filename of users & passwords */
	extern std::string users_filename;
	extern time_t time_start;
	extern paycheck::CPaycheckCore *xml_paycheck_core;
}
#endif

#ifdef PAYGUIDE_MAIN_CPP
namespace payguide
{
	time_t time_start;
	int db_accept_new_pays=1;
	sem_t free_workers_lock;
	sem_t db_lock_read;
	sem_t db_lock_write;
	sem_t db_lock_checkreqs;
	sem_t shutdown_lock;
	CPerlModule data_conventor;
	int daemonize=0;
	int package_timeout=30;

	long long pays_total=0;

	MYSQL db_checkreqs_connection;
	bool working=true;
	bool quit=false;
	int reload_config=1;
	int pc_port=2500;
	sem_t reload_config_lock;
	CUlist<CWorker> *workers_list=NULL;
	CWorker *free_worker=NULL;
	unsigned int working_workers=0;
	unsigned int thread_max=100;
	unsigned int thread_min=10;
	int thread_inactivity_time=10;
	int thread_autodelete=1;

	int db_reconnect_time=5;
	int db_reconnect_attempts=0;

	int need_cleanup_modules=0;
	
	std::string modules_path="./modules/paysys/";
	std::string modules_init_path="./modules/init/";
	std::string operators_path="./operators/";
	std::string perl_module="./perl.pl";
	std::string bind_interface="lo";

	/* default backtrace file */
	std::string backtrace_file="/tmp/payguide.backtrace";

	/* defaul db host */
	std::string db_host="localhost";					

	/* default db name */
	std::string db_name="electropay_db1";				

	/* default db user name */
	std::string db_user="pays_daemon";

	/* default db password */
	std::string db_password="masteranubis";					

	/* full filename of users & passwords */
	std::string users_filename="/etc/payguide.users";


	std::string host_saved, db_name_saved, user_saved, passwor_saved;

	const char *default_db_select="select p.id, p.pay_engine, p.stamp, p.data, p.provider_id, p.bill_num, p.terminal_id, p.account_id, p.currency, 0, 0, p.real_sum, p.back_sum, p.amount, rand() as rr from pays_big as p left join pays_engines as e on p.pay_engine=e.id where p.fatal=0 and p.gstatus=3 and p.sleep=0 and e.engine_num=10";
	const char *test_db_select   ="select p.id, p.pay_engine, p.stamp, p.data, p.provider_id, p.bill_num, p.terminal_id, p.account_id, p.currency, 0, 0, p.real_sum, p.back_sum, p.amount, rand() as rr from pays_big as p left join pays_engines as e on p.pay_engine=e.id where p.fatal=0 and p.gstatus=3 and p.sleep=0";
	const char *db_main_select=default_db_select;

	CUlist<COperator> *operators_list=NULL;
	CUlist<COperator> *old_operators_list=NULL;

	CUlist<CPaySys> *modules_list=NULL;
	CPaySys *null_pay_sys=NULL;
	paycheck::CPaycheckCore *xml_paycheck_core=NULL;

	int namespace_init()
	{
		sem_init(&free_workers_lock, 0,1);
		sem_init(&db_lock_read, 0,1);
		sem_init(&db_lock_write, 0,1);
		sem_init(&db_lock_checkreqs, 0,1);
		sem_init(&reload_config_lock, 0,1);
		sem_init(&shutdown_lock, 0,1);

		return 0;
	}
}
#endif
