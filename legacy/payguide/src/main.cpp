/*
 *          ____                   ____                    __            
 *         /\  _`\                /\  _`\           __    /\ \           
 *         \ \ \L\ \ __     __  __\ \ \L\_\  __  __/\_\   \_\ \     __   
 *          \ \ ,__/'__`\  /\ \/\ \\ \ \L_L /\ \/\ \/\ \  /'_` \  /'__`\ 
 *           \ \ \/\ \L\.\_\ \ \_\ \\ \ \/, \ \ \_\ \ \ \/\ \L\ \/\  __/ 
 *            \ \_\ \__/.\_\\/`____ \\ \____/\ \____/\ \_\ \___,_\ \____\
 *             \/_/\/__/\/_/ `/___/> \\/___/  \/___/  \/_/\/__,_ /\/____/
 *                              /\___/                                   
 *                              \/__/                                    
 * PayGuide server
 *
 * This program is written by Sergeev Alexander
 * My phone: 8 926 613 35 66
 * My e-mail: funny.is2k1(AT)gmail.com, a.sergeev(AT)electropay.ru
 *
 * Year 2007-2008
 *
 * You can find lastest source in CVS on baby.*****.com:/root in payguide
 * repository.
 * */

#define PAYGUIDE_MAIN_CPP

#include <iostream>
#include <fstream>
#include <signal.h>
#include <mysql/mysql.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <curl/curl.h>
#include <time.h>

#include "paysys.h"
#include "initsys.h"
#include "operator.h"
#include "parser.h"
#include "worker.h"
#include "ctl.h"
#include "pay.h"
#include "db.h"
#include "log.h"
#include "paycheck.h"
#include "perlm.h"
#include "namespace.h"
#include "thrdmngr.h"
#include "statistic.h"
#include "xmlpaycheck/paycheck.h"

#ifdef ENABLE_BACKTRACE
#include "backtrace.h"
#endif

#define EVER	;;

static int pc_init_result=-1;

/* Call backs for thread-safe OpenSSL work)) */
static void ossl_locking_function(int mode, int n, const char *file, int line);
static unsigned long ossl_id_function(void);

/* OpenSSL locks */
static sem_t *ossl_lock=NULL;

/* Number of OpenSSL locks */
static int ossl_lock_num;

static bool force_quit=0;

static void SigHandler(int signal);
static int Daemonize();
static void CleanUp();
static void LaunchNewPay(SPay *pay);
static void WorkInit();
static void PayguideInit();
static int CheckIfPayguideAlreadyUp();
static int MakeLockFile();
static int RemoveLockFile();

int main (int argc, char *argv[])
{
	payguide::time_start=time(NULL);
	int Ppid=CheckIfPayguideAlreadyUp();
	if (Ppid==0)
	{
		if (MakeLockFile()!=0)
		{
			std::cout << "Error while creating lock file. Can't write /tmp/payguide.lock" << std::endl;
			LogWrite(LOGMSG_CRITICAL, "Error while creating lock file. Can't write /tmp/payguide.lock");
		}
	}
	else
	{
		
		char msg[512];
		snprintf(msg, 511,"Error: payguide already running as process %i. Balling out.", Ppid);
		std::cout << msg << std::endl;
		LogWrite(LOGMSG_CRITICAL, msg);
		exit(0);
	}
	
	PayguideInit();
	WorkInit();
	bool work_init_already=true;
	
	bonbon::BonbonInit();
	
	
	PerlInit(argv[0], payguide::perl_module.c_str());
	payguide::data_conventor.Init();
	for (EVER)
	{
		std::cout << "Running xml paycheck..." << std::endl;
	        const char *main_config_file="/etc/payguide.cfg";
	        paycheck::CPaycheckCore paycheck_core;
	        paycheck_core.LoadConfig(main_config_file);
	        paycheck::CServer server_code;
	        bonbon::CJobManager in_manager;
	        std::vector <paycheck::CSocket *> connections_store;
	        paycheck::CPaycheckThreadParam params(&in_manager, &connections_store, &paycheck_core);
	        bonbon::CThread xml_server1(server_code, params);
	        payguide::xml_paycheck_core=&paycheck_core;

		payguide::quit=false;
		if (!work_init_already)
			WorkInit();
		int sleep_count=0;
		timespec req;
		req.tv_sec=0;
		req.tv_nsec=50000000;
		
		
		payguide::working=true;
		
		LogWrite(LOGMSG_SYSTEM, "Payguide server started sucessful and ready for a work");
		
		/* Ok, lets rock */
		
		SPay *new_pay=NULL;
		sem_wait(&payguide::shutdown_lock);
		bool working_state=payguide::working;
		sem_post(&payguide::shutdown_lock);
		
		printf("TESTING printf(): if you see that in log - world gone mad\n");
		std::cout << "TESTING std::cout: if you see that in log - world gone mad" << std::endl;
		
		while (working_state)
		{
			sem_wait(&payguide::shutdown_lock);
			working_state=payguide::working;
			sem_post(&payguide::shutdown_lock);

			/* Get next pay from DB */
			if (new_pay==NULL)
			{
				new_pay=paycheck_core.XMLGetNextPay();
				if (new_pay==NULL)
				{
					new_pay=PCGetNextPay();
				
					if (new_pay==NULL)
					{
						//printf("get new pay from DB\n");
						if (sleep_count==1)
							new_pay=DBGetNextPay();
					}
					else if (new_pay->test==NO_TEST)
					{
						LogWrite(LOGMSG_CRITICAL, "Payguide received from BIN PAYCHECK (payd) must be marked as testing. Shutdown.");
						char tmp[1024];
						snprintf(tmp, 1024, "Pay params: id/session=[%lli] terminal=[%lli] operator=%i", new_pay->id, new_pay->terminal_id, new_pay->provider_id); 
						LogWrite(LOGMSG_CRITICAL, tmp);
						exit(-1);
					}
				}
				else if (new_pay->test==NO_TEST)
				{
					LogWrite(LOGMSG_CRITICAL, "Payguide received from XML PAYCHECK must be marked as testing. Shutdown.");
					char tmp[1024];
					snprintf(tmp, 1024, "Pay params: id/session=[%lli] terminal=[%lli] operator=%i", new_pay->id, new_pay->terminal_id, new_pay->provider_id); 
					LogWrite(LOGMSG_CRITICAL, tmp);
					exit(-1);
				}
			}

			if (new_pay!=NULL)
			{
//				char logmsg[101]; snprintf(logmsg, 100,"Working on pay %lli", new_pay->id); SendLogMessages(0, logmsg);
				sem_wait(&payguide::free_workers_lock);
				//printf("lock catched\n");
				/* If were is no free worker - find one or create new. */
				if (payguide::free_worker==NULL)
					SetFreeWorker(NULL);
				
				/* If it's ok - init free worker with a new pay */
				if (payguide::free_worker!=NULL)
				{
					LaunchNewPay(new_pay);
					new_pay=NULL;
					sem_post(&payguide::free_workers_lock);	
				}
				
				/* We can't create a new worker - threads limit hit. */
				else
				{
					//LogWrite(LOGMSG_WARNING, "Threads limit hit. Payguide server is overloaded.");
					sem_post(&payguide::free_workers_lock);
					
					if (sleep_count==20)
					{
						ReloadConfigIfImportant();
						ManagerEx();
						sleep_count=0;
						
					}

					nanosleep(&req, NULL);sleep_count++;
					StatisticTick(0.2);

					
				}
			}
			
			/* No new pays, sleep (nojobs_sleep_time) sec*/
			else
			{
				
				/* Manage inactive workers */
				if (sleep_count==20)
				{
					ReloadConfigIfImportant();
					ManagerEx();
					sleep_count=0;
				}
//				sleep(SLEEP_TIME); sleep_count=20;
				nanosleep(&req, NULL);sleep_count++;
				StatisticTick(0.2);

			}
		}
		
		/* Waiting for all workers, or even cancel all pays if force_quit is 1 ... */
		LogWrite(LOGMSG_SYSTEM, "Waiting for all active pays to end - for safe exit - you had to specify right timeouts in your modules.");
		WaitForAllPaysToFinish(force_quit);
		
		/* If it's not reboot - exit */
		sem_wait(&payguide::shutdown_lock);
		bool quit_state=payguide::quit;
		sem_post(&payguide::shutdown_lock);
		if (quit_state)
		{
			LogWrite(LOGMSG_SYSTEM, "Payguide server shutdown normally.");
			LogWrite(LOGMSG_SYSTEM, "Bye-bye!");
			
			/* Stop connection with MySQL database */
			DBShutdown();
		
			/* Stop control server. */
			//CtlServerStop();
			
			mysql_library_end();
			
			if (ossl_lock!=NULL)
				delete [] ossl_lock;
			if (pc_init_result==0)
				PCShutdown();
			/* CleanUp will be automatically called by atexit() function */
			if (RemoveLockFile()!=0)
			{
				std::cout << "Can't remove /tmp/payguide.lock. File doesn't exist or maybe you should delete it manually."  << std::endl;
				LogWrite(LOGMSG_CRITICAL, "Can't remove /tmp/payguide.lock. File doesn't exist or maybe you should delete it manually.");
			}
			OperatorsShutdown();
			EVP_cleanup();
			payguide::data_conventor.Clean();
			PerlShutdown();
			printf("destroying queue\n");
			in_manager.Destroy();
			printf("done\n");
			exit (0);
		}
		
		/* Stop connection with MySQL database */
		DBShutdown();
		
		printf("destroying queue\n");
		in_manager.Destroy();
		printf("done\n");
	        server_code.Kill();
		LogWrite(LOGMSG_SYSTEM, "Rebooting...");
		work_init_already=false;
		CleanUp();
	}
	return 0;
}

void OpenSSLInit()
{
	LogWrite(LOGMSG_SYSTEM, "Registring OpenSSL multithread lock functions...");
	ossl_lock_num=CRYPTO_num_locks();
	ossl_lock=new sem_t[ossl_lock_num+1];

	if (ossl_lock==NULL)
	{
		LogWrite(LOGMSG_CRITICAL, "Registring OpenSSL failed, work is NOT SAFE now");
		return;
	}
		
	for (int i=0; i<ossl_lock_num; i++)
		sem_init(&(ossl_lock[i]), 0,1);
	
	CRYPTO_set_locking_callback(ossl_locking_function);
	CRYPTO_set_id_callback(ossl_id_function);
	
	// получаем доступ к хэш-функции MD5
	OpenSSL_add_all_digests();

	LogWrite(LOGMSG_SYSTEM, "OpenSSL lock function registred.");
}

void ossl_locking_function(int mode, int n, const char *file, int line)
{
	if (n<ossl_lock_num+1)
	{
		if (mode & CRYPTO_LOCK)
			sem_wait(&(ossl_lock[n]));
		else
			sem_post(&(ossl_lock[n]));
	}
	return;
}

unsigned long ossl_id_function(void)
{
	return (unsigned long)pthread_self();
}



void PayguideInit()
{
	/* Init variables from payguide namespace */
	payguide::namespace_init();

	ReloadConfigIfImportant();
	
	if (curl_global_init(CURL_GLOBAL_ALL)!=0)
	{
		printf("CURL init failed! Payguide shutdown.\n");
		exit(1);
	}
	
	/* Deatch from console */
	if (payguide::daemonize==1)
	{
		if (Daemonize()!=0) 
		{
			LogWrite(LOGMSG_ERROR, "Daemonize failed!");
		}
	}


	
	my_init();
	DBVeryFirstInit();
	
	OperatorsInit();
	OpenSSLInit();

	StatisticInit();
	
	/* Catching TERM signal - shut down server */
	struct sigaction sterm;
	memset (&sterm, 0, sizeof(sterm));
	sterm.sa_handler=&SigHandler;
	sigaction(SIGTERM, &sterm, NULL);
	
	/* Catching INT signal - shut down server */
	struct sigaction sint;
	memset (&sint, 0, sizeof(sint));
	sint.sa_handler=&SigHandler;
	sigaction(SIGINT, &sint, NULL);
	
	atexit (CleanUp);

	/* Init backtrace */
	#ifdef ENABLE_BACKTRACE
		open_backtrace_fd(payguide::backtrace_file.c_str());
		install_handlers() ;
	#endif
	
}

void WorkInit()
{
	/* Init logs */
	LogInit();
	
	if (payguide::working) 
		LogWrite(LOGMSG_WARNING, "Initializing payuguide server... ");
	else
		LogWrite(LOGMSG_WARNING, "Restarting payuguide server... ");
		
	/* Load config file */
	LogWrite(LOGMSG_SYSTEM, "Loading configuration file from /etc/payguide.cfg");
	if (ReloadConfigValues("/etc/payguide.cfg")!=0)
		LogWrite(LOGMSG_WARNING, "Can't load  /etc/payguide.cfg - using default values instead.");
	else
		LogWrite(LOGMSG_SYSTEM, "Configuration file /etc/payguide.cfg loaded.");
		
	
	if (pc_init_result==-1)
	{
		if (payguide::bind_interface!="all" && payguide::bind_interface!="ALL")
			pc_init_result=PCInit(payguide::pc_port, 100, payguide::bind_interface.c_str(), payguide::users_filename.c_str(), payguide::package_timeout);
		else pc_init_result=PCInit(payguide::pc_port, 100, NULL, payguide::users_filename.c_str(), payguide::package_timeout);
	}
	
	
	/* Loading pay modules from *.so  files */
	LogWrite(LOGMSG_SYSTEM, "Loading pay modules (*.so files)");
	payguide::modules_list = LoadModules(payguide::modules_path.c_str());
	LogWrite(LOGMSG_SYSTEM, "Pay modules loaded.");
	
	/* Loading default module  */
	payguide::null_pay_sys=GetPaySysBySoName("libnull.so");
	
	if (payguide::null_pay_sys==NULL)
	{
		LogWrite(LOGMSG_CRITICAL, "Critical error - can't load NULL pay system (libnull.so)");
//		std::cout << "Critical error - can't load NULL pay system (libnull.so)" << std::endl;
		payguide::quit=true;
		payguide::working=false;
		RemoveLockFile();
		exit(1);
	}
	
	
	/* Loading operators */
	LogWrite(LOGMSG_SYSTEM, "Loading operators...");
	payguide::operators_list = LoadOperators(payguide::operators_path.c_str());
	LogWrite(LOGMSG_SYSTEM, "Operators loaded.");
	
	
	/* Creating thread_min workers */
	{
		char logmsg[256]; snprintf(logmsg, 255, "Creating %i threads...",payguide::thread_min);LogWrite(LOGMSG_SYSTEM,logmsg);
	}
	sem_wait(&payguide::free_workers_lock);
	payguide::workers_list = CreateWorkers(payguide::thread_min);
	payguide::working_workers=0;
	
	LogWrite(LOGMSG_SYSTEM, "Threads created.");
	
	/* Free worker is a first worker in list */
	payguide::workers_list->ResetCursor();
	payguide::free_worker=payguide::workers_list->GetNext();
	sem_post(&payguide::free_workers_lock);	
	
	/* Init MySQL database */
	if (0==DBInit(&payguide::db_host, &payguide::db_name, &payguide::db_user, &payguide::db_password))
		LogWrite(LOGMSG_SYSTEM, "Connection to MySQL database established.");
	else
		LogWrite(LOGMSG_ERROR, "Connection to MySQL database failed.");
		
		
	LoadAllInitSO(payguide::modules_init_path.c_str());
	return;
}

void LaunchNewPay(SPay *pay)
{
	//printf("Launching new pay...\n");
	CWorker *worker=InitFreeWorker(pay);
	//printf("1\n");
	if (worker!=NULL)
	{
		//printf("2\n");
		worker->AddPay(pay);
		payguide::free_worker=NULL;
		worker->Run();
		payguide::working_workers++;
		if (payguide::pays_total<100000000) payguide::pays_total++;
		char logmsg[101]; snprintf(logmsg, 100,"New pay with id %lli, %lli pays total", pay->id, payguide::pays_total);
		LogWrite(LOGMSG_NORMAL, logmsg);
		//printf("3\n");
	}
	return;
}

void CleanUp()
{
	#ifdef ENABLE_BACKTRACE
		onexit();
	#endif
		sem_wait(&payguide::free_workers_lock);
		/* Cleaning up and exit */
		
		/* Clean operators rules */
		LoadModules(NULL);
		LoadOperators(NULL);
		
		if (payguide::modules_list!=NULL)
		{
			delete payguide::modules_list;
			payguide::modules_list=NULL;
		}
		if (payguide::operators_list!=NULL)
		{
			
			delete payguide::operators_list;
			payguide::operators_list=NULL;
		}
		
		if (payguide::workers_list!=NULL)
		{
			delete payguide::workers_list;
			payguide::workers_list=NULL;
		}
		UnLoadAllInitSO();
		
		
		LogWrite(LOGMSG_SYSTEM, "(Clean up complete sucessful.)");
		LogClose();
		sem_post(&payguide::free_workers_lock);
		
}

void SigHandler(int signal)
{
	if (signal==SIGTERM)
	{
		payguide::working=false;
		payguide::quit=true;
		force_quit=1;
		return;
	}
	
	else if (signal==SIGINT)
	{
		payguide::working=false;
		payguide::quit=true;
		return;
	}
}

int Daemonize()
{
	pid_t pid, sid;
	int result=1;
	
	pid = fork();
	if (pid < 0)  return 1;
	if (pid > 0)
	{
		/*We are in parent thread - payguide daemon started ok already - we can exit now */
		LogWrite(LOGMSG_SYSTEM, "Payguide daemon started sucessful.");
		exit(0);
	}
	
	/* We are daemon now!! */
	
	/* Update PID in lock file */
	MakeLockFile();

	sid = setsid();
	if (sid < 0) result=1;
	
	/* Change current dir to root */
	if ((chdir("/")) < 0) result=1;
	
	/* Close input/output streams */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	/* stdout, stdin and stderr go directly in /dev/null... */
	int i=open("/dev/null",O_RDWR);
	dup(i);
	dup(i);
	
	return result;
}


int CheckIfPayguideAlreadyUp()
{
	int n=0;
	
	ifstream lock_file;
	lock_file.open("/tmp/payguide.lock");
	
	if (lock_file.good()==0)
	{
		lock_file.close();
	}
	else
	{
		std::string cur_val;
		lock_file >> cur_val;
		n=atoi(cur_val.c_str());
		lock_file.close();
	}
	
	return n;

}

int MakeLockFile()
{
	ofstream new_lock_file;
	new_lock_file.open("/tmp/payguide.lock", ios_base::out);
	if (new_lock_file.good())
	{
		new_lock_file << (int)getpid();
		new_lock_file.close();
		return 0;

	}
	new_lock_file.close();
	return -1;
	
}

int RemoveLockFile()
{
	return unlink("/tmp/payguide.lock");
}
