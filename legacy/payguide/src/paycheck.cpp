#include <sys/types.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

#include "paycheck.h"
#include "ctl.h"
#include "core.h"
#include "db.h"
#include "uremote_socket.h"
#include "ulist.h"
#include "sem.h"
#include "sizes.h"
#include "namespace.h"
#include "nettypes.h"

#define CRS_MAX_CON 64

static void *GetData(void *param);
static void *ConnectionManager(void *param);

static int PCSetState(SPay *pay, SPayResult *payres, bool dontupdate);

static int GetAuthorization(const char *login, const char *password);
static int pc_thread_1=0;
static int pc_thread_2=0;

//static int CreateMD5Sign(const char *text, char *buff, int buff_size);

static int package_timeout;
static CConfig users;
static sem_t users_lock;



static pthread_t pc_thread;
static pthread_t pc_thread_rw;
static CRemoteSocket paycheck_socket;
static sem_t pc_lock;
static unsigned int max_queue=100;

class CCheckPay
{
	public:
		CCheckPay(){buff=NULL;len=0; sock=NULL; pay=NULL;inwork=0;}
		CCheckPay(char *b, int l, CSocketRW *s, SPay *p){buff=b; len=l; sock=s; pay=p;inwork=0;}
		~CCheckPay(){if (buff!=NULL){delete [] buff;} if (pay!=NULL){delete pay;}};
		char *buff;
		CSocketRW *sock;	
		SPay *pay;
		int len;
		int inwork;
};


struct thread_param
{
	CRemoteSocket *socket;
	CUlist<CCheckPay> *in_list;
	CUlist<CCheckPay> *in_cash_list;
	CUlist<CCheckPay> *out_list;
	sem_t *pc_lock;
	CUlist<CPayguideClient> *sock_list;
	int pc_exit;
	CSemaphore socklist_delete;
};

static thread_param prm;

static SPay *CompileCheckPay(char *data, int size, CSocketRW *sk, CPayguideClient *pgc, thread_param *prm);

int PCInit(int port, int m_queue, const char *interface, const char *users_filename, int packagetimeout)
{
	package_timeout=packagetimeout;

	int err=-1;
	max_queue=abs(m_queue);
	sem_init(&pc_lock, 0,1);

	prm.pc_exit=0;
	prm.socket=&paycheck_socket;
	prm.pc_lock=&pc_lock;
	
	users.ReadFile(users_filename);
	sem_init(&users_lock, 0,1);
	
	prm.in_list=new CUlist<CCheckPay>;
	prm.in_cash_list=new CUlist<CCheckPay>;
	prm.out_list=new CUlist<CCheckPay>;
	prm.sock_list=new CUlist<CPayguideClient>;

	if (sizeof(short int)<SIZE_HEAD_LEN || sizeof(long long)<SIZE_BODY_ID || sizeof(long long)<SIZE_BODY_TERMINAL || sizeof(int) < SIZE_BODY_CURRENCY || sizeof(int)<SIZE_BODY_OPERATOR)
	{
		LogWrite(LOGMSG_CRITICAL, "Sorry, only gcc with x86_32 supported; x86_64 may works, but it's untested");
    		return -1;
	}

	while (err<0)
	{
		err=paycheck_socket.Bind(port, interface);
		if (err<0)
		{
			//Bind failed
			char tmp[512];
			snprintf(tmp, 512, "Bind in PCInit call FAILED (%s)", strerror(errno));
			LogWrite(LOGMSG_WARNING, tmp);
			return -1;
		}
		else
		{
			err=-1;
			while (err<0)
			{

				err=paycheck_socket.Listen();
				if (err<0)
				{
					//Listen failed;
					char tmp[512];
					snprintf(tmp, 512, "Listen in PCInit call FAILED (%s)", strerror(errno));
					LogWrite(LOGMSG_WARNING, tmp);
					return -1;
				}
			}
		}
	}
	
	if (pthread_create(&pc_thread, NULL, &ConnectionManager, &prm)==0)
		pc_thread_1=1;
		
	if (pthread_create(&pc_thread_rw, NULL, &GetData, &prm)==0)
		pc_thread_2=1;
	
	return 0;

}

SPay *PCGetNextPay()
{
	sem_wait(&payguide::db_lock_read);
	if (payguide::db_accept_new_pays==0)
	{
		sem_post(&payguide::db_lock_read);
		return NULL;
	}
	sem_post(&payguide::db_lock_read);

	sem_wait(prm.pc_lock);
	prm.in_cash_list->ResetCursor();
	for (unsigned int i=0; i<prm.in_cash_list->GetLen(); i++)
	{
		CCheckPay *ccpay=prm.in_cash_list->GetNext();
		
		if (ccpay!=NULL && ccpay->pay!=NULL)
		{
			SPay *t=ccpay->pay;
			t->test=FORMAT_CHECKSYS_TEST;
			bool already_checked=false;
			
			int error_code=-1;
//			char error_msg[SIZE_REPONSE_MSG+1];
//			memset(error_msg, 0, SIZE_REPONSE_MSG+1);
			SPayResult pres;
			
			
			SPay *result=new SPay();
			(*result)=(*t);
			//memcpy(result, t, sizeof(SPay));
				
			prm.in_cash_list->RemoveThis();
			i--;
			sem_post(prm.pc_lock);

			if (result->id!=0)
			{
				char pay_query[512];
				snprintf(pay_query,512,"select error, result from check_reqs where bill_num='%lli' and tid=%lli", result->id, result->terminal_id);
				sem_wait(&payguide::db_lock_checkreqs);
				my_db_query(&payguide::db_checkreqs_connection, pay_query, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str());
				MYSQL_RES *quest_res=NULL;
				quest_res=mysql_store_result(&payguide::db_checkreqs_connection);
				if (quest_res!=NULL)
				{
					MYSQL_ROW row;
					row = mysql_fetch_row(quest_res);
					unsigned long *lengths=mysql_fetch_lengths(quest_res);
					if (row!=NULL)
					{
						already_checked=true;
						if (row[0]!=NULL && row[1]!=NULL)
						{
							pres.code=atoi(row[0]);
							if (lengths[1]<SIZE_REPONSE_MSG)
							{
								memcpy(pres.msg, row[1], lengths[1]);
								if (pres.code==RESULT_SUCESS_BIN)
								{
									pres.bdata_size=lengths[1];
								}
							}
							result->exists_int_checkreqs=true;
						}
			
					}
					mysql_free_result(quest_res);
				}
				sem_post(&payguide::db_lock_checkreqs);
			}


			if (already_checked)
			{
			//	sem_post(prm.pc_lock);
				if (error_code!=-1)
					PCSetState(result, &pres, true);
				delete result;
				return NULL;
			}
			else
			{
				if (result->id!=0)
				{
					char pay_query[512];
					snprintf(pay_query,512,"insert into check_reqs(bill_num, tid, stamp, error) values('%lli', %lli, now(), %i)", result->id, result->terminal_id, RESULT_UNKNOWN);
					sem_wait(&payguide::db_lock_checkreqs);
					my_db_query(&payguide::db_checkreqs_connection, pay_query, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str());
					sem_post(&payguide::db_lock_checkreqs);
				}
				return result;
			}
			
		}
		
	}
	sem_post(prm.pc_lock);
	return NULL;
}

int PCSetState(SPay *pay, SPayResult *payres)
{
	return PCSetState(pay, payres, false);
}

//int PCSetState(SPay *pay, int result,int sleep, const char *msg, const char *sender_name, bool dontupdate, SPayResult *payres)
int PCSetState(SPay *pay, SPayResult *payres, bool dontupdate)
{

	int l=0;
	if (pay==NULL || payres==NULL)
		return -1;
	
	int msglen=0;
	
	if (payres->code==RESULT_SUCESS_BIN)
	{
//		result=RESULT_SUCESS;
		msglen=payres->bdata_size;
//		msg=payres->bdata;
	}
	else
	{
		msglen=strlen(payres->msg);
	}
	
	if (!dontupdate && pay!=NULL && pay->id!=0)
	{
		char pay_query[4096];
		char req[1024];
		snprintf(req,1024,",error=%i where tid='%lli' and bill_num='%lli'",payres->code, pay->terminal_id, pay->id);
		sem_wait(&payguide::db_lock_checkreqs);
		char *end = pay_query;
		strncpy(pay_query,"update check_reqs set result=",4096);
		end+=strlen(pay_query);
		*end++='\'';
		end += mysql_real_escape_string(&payguide::db_checkreqs_connection, end, payres->msg, msglen);
		*end++='\'';
		strncat(end, req, 3000);
		end+=strlen(req);
		my_db_query(&payguide::db_checkreqs_connection, pay_query, end-pay_query, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str());
		sem_post(&payguide::db_lock_checkreqs);	
	}

	char buff[5024];
	buff[0]=(signed char)pay->nettype;
	buff[1]=(signed char)pay->netversion;
	char *ptr=buff+2;
	memcpy(ptr, &(pay->id), sizeof(pay->id));
	ptr+=sizeof(pay->id);
	
	memcpy(ptr, &(pay->magic), SIZE_BODY_MAGIC);
	ptr+=SIZE_BODY_MAGIC;

	
	memcpy(ptr, &payres->code, sizeof(payres->code));
	
	if (payres->msg!=NULL)
	{
		ptr+=sizeof(payres->code);
		memcpy(ptr, payres->msg, msglen);
		ptr+=msglen;
	}
	l=ptr-buff;
	//Ищем адресата
	    CCheckPay *t=NULL;
	CSocketRW *sRW=NULL;
	
	sem_wait(prm.pc_lock);
	unsigned int list_l=prm.in_list->GetLen();
	prm.in_list->ResetCursor();
//	printf("searching pay id %lli\n", pay->id);
	for (unsigned int i=0; i<list_l; i++)
	{
		t=prm.in_list->GetNext();
//		printf("step=%i\n",i);
//		printf("l=%i\n", list_l);
//		printf("t=[%p]\n",t);
		
		if (t!=NULL && t->pay!=NULL)
		{
//			printf("pay_id=%lli, term=%lli\n", t->pay->id, t->pay->terminal_id);
			if (t->pay->id==pay->id && t->pay->terminal_id==pay->terminal_id)
			{
				sRW=t->sock;
				//printf("using socket [%p], pay_id=%lli, terminal_id=%lli\n",sRW, pay->id ,pay->terminal_id);
//				printf("removing %p\n", t);
				prm.in_list->RemoveThis();
				i=prm.in_list->GetLen();
//				i--;
//				if (list_l>0)list_l--;
			}
		}
	}
	/* Добавляем в список на отправку */
	if (sRW!=NULL)
	{
		prm.socklist_delete.LockRead();
		sem_post(prm.pc_lock);
		sRW->Send(buff, l);
		prm.socklist_delete.UnLockRead();
	}
	else
	{
		sem_post(prm.pc_lock);
	}
	//printf("returning form CPSetState  [%p], pay_id=%lli, terminal_id=%lli\n",sRW, pay->id ,pay->terminal_id);
	
	return 0;
}


int PCShutdown()
{
	//printf("Shutdown in progress....\n");
	
	sem_wait(prm.pc_lock);
	prm.pc_exit=1;
	sem_post(prm.pc_lock);
	//printf("Flag set\n");
	
	paycheck_socket.ListenerDown();
	
	//printf("JOIN\n");
	if (pc_thread_1!=0)
		pthread_join(pc_thread,NULL);
	//printf("tread 1 joined\n");
	if (pc_thread_2!=0)
		pthread_join(pc_thread_rw,NULL);
	//printf("tread 2 joined\n");
	if (prm.in_list!=NULL)
		delete prm.in_list;
	
	if (prm.in_cash_list!=NULL)
		delete prm.in_cash_list;
	if (prm.out_list!=NULL)
		delete prm.out_list;
	if (prm.sock_list!=NULL)
		delete prm.sock_list;
	
	pc_thread_1=0;
	pc_thread_2=0;
	
	return 0;
}

void *ConnectionManager(void *param)
{
	LogWrite(LOGMSG_WARNING, "Connection manager thread started up!\n");
	thread_param  *prm=(thread_param *)param;
	
	sem_wait(prm->pc_lock);
	int quit=prm->pc_exit;
	sem_post(prm->pc_lock);
	
	while (quit==0)
	{
		sem_wait(prm->pc_lock);
		int clients_num=prm->sock_list->GetLen();
		sem_post(prm->pc_lock);
		CSocketRW *sock=NULL;
		
//		printf("ACCEPTING...\n");
		if (clients_num<500)
			sock=prm->socket->Accept();
//		printf("Accept returns.\n");
		if (sock!=NULL)
		{
			sem_wait(prm->pc_lock);
			prm->sock_list->ResetCursor();
			CPayguideClient *payguide_client=new CPayguideClient();
			payguide_client->LinkSocket(sock);
			prm->sock_list->AddItem(payguide_client);
			sem_post(prm->pc_lock);
//			printf("ACCEPTED!\n");
		}
		else
		{
//			printf("NO1 TO ACCEPT...:(\n");
			sem_wait(prm->pc_lock);
			CSocketRW *sRW=NULL;
			prm->sock_list->ResetCursor();
			for (unsigned int i=0; i<prm->sock_list->GetLen(); i++)
			{
				CPayguideClient *payguide_client;
				payguide_client=prm->sock_list->GetNext();
				if (payguide_client!=NULL)
				{
					if ((sRW=payguide_client->GetSocket())!=NULL)
					{
						if (sRW->Reading() || sRW->Authorized()==0)
						{
							int timeout=sRW->TimeOut();
							if (timeout>=package_timeout*20)
							{
								sRW->Down();
							}
						}
					}
				}
			}
			sem_post(prm->pc_lock);
			
			timespec req;
			req.tv_sec=0;
			req.tv_nsec=50000000;
			nanosleep(&req, NULL);
		}
		
		sem_wait(prm->pc_lock);
		quit=prm->pc_exit;
		sem_post(prm->pc_lock);
		
	}
	return NULL;
}

void *GetData(void *param)
{
	LogWrite(LOGMSG_WARNING, "Get Data thread started up!\n");
	thread_param  *prm=(thread_param *)param;
	int err=0;
	int ex=0;
	char in_buffer[UREMOTE_SOCKET_PACK_SIZE];
	unsigned int sock_num=0;
	unsigned int sock_list_len=0;
	CSocketRW *sock=NULL;
	
	sem_wait(prm->pc_lock);
	int quit=prm->pc_exit;
	sem_post(prm->pc_lock);

	while (quit==0 && ex==0)
	{

		sem_wait(prm->pc_lock);
		int read_count=readmultiselect(prm->sock_list,0,0);
		
		if (read_count<=0)
		{
			sem_post(prm->pc_lock);
			
			timespec req;
			req.tv_sec=0;
			req.tv_nsec=50000000;
			nanosleep(&req, NULL);
		
		}
		else
		{
			//sem_post(prm->pc_lock);

			sock=NULL;	
			sock_list_len=prm->sock_list->GetLen();
			if (sock_num<sock_list_len && sock_num>0)
				sock_num++;
			else
				sock_num=1;
		
		
			CPayguideClient *payguide_client;
		
			prm->sock_list->ResetCursor();
			for (unsigned int i=0; i<sock_num; i++)
			{
				payguide_client=prm->sock_list->GetNext();
			}
				if (payguide_client!=NULL)
					sock=payguide_client->GetSocket();
		
			sem_post(prm->pc_lock);

			if (sock_list_len==0)
			{
				//printf("no client connected - sleeping\n");
//					sleep(1);
			}
			else if (sock!=NULL && sock->ReadIsPossible())
			{
			
//				int l=0;
//				printf("Reading\n");
				//printf("Receiving new data....\n");
				int l=sock->Receive(in_buffer, UREMOTE_SOCKET_PACK_SIZE, 0, 0);
			
			
				//char *t=NULL;
				err=sock->Err();
				if (err==0 && l>=0)
				{
				
					if (l>0)
					{
						//printf("No error occured, data received\n");
						SPay *s1=CompileCheckPay(in_buffer, l, sock, payguide_client, prm);

						if (s1!=NULL)
						{
							SPay *s2=new SPay();
							//memcpy(s2,s1, sizeof(SPay));
							(*s2)=(*s1);
							CCheckPay *ccpay1= new CCheckPay(NULL, 0, sock, s1);
					
							sem_wait(prm->pc_lock);
							if (prm->in_list->GetLen()<max_queue)
							{
								CCheckPay *ccpay2= new CCheckPay(NULL, 0, sock, s2);
								prm->in_list->AddItem(ccpay1);
								prm->in_cash_list->AddItem(ccpay2);
								sem_post(prm->pc_lock);
							}
							else
							{
//								prm->in_list->AddItem(ccpay1);
//								printf("QUEUE FULL\n");
								sem_post(prm->pc_lock);
							
//								PCSetState(s2, RESULT_QUEUE_FULL,0, "QUEUE FULL", "online_check", NULL);
								SPayResult pres;
								pres.code=RESULT_QUEUE_FULL;
								PCSetState(s2, &pres);
							}
						}
					}
					else
					{
						//Timeout
						//printf("No error occured, no new data\n");
	
					}
					//printf("point XXX\n");
				}
				else
				{
					//printf("ERROR occured\n");
					bool deleted=false;
					sem_wait(prm->pc_lock);
				
					prm->sock_list->ResetCursor();
					for (unsigned int i=0; i<prm->sock_list->GetLen(); i++)
					{
						CPayguideClient *payguide_client;
						payguide_client=prm->sock_list->GetNext();
					
						if (payguide_client!=NULL)
						{
							//printf("compare [%p] and [%p]\n",payguide_client->GetSocket(),sock);
							if (payguide_client->GetSocket()!=NULL && sock==payguide_client->GetSocket())
							{
								prm->socklist_delete.LockWrite();
								payguide_client->GetSocket()->Down();
								prm->socklist_delete.UnLockWrite();
							
								prm->in_list->ResetCursor();
								for (unsigned int j=0; j<prm->in_list->GetLen(); j++)
								{
									CCheckPay *cp=prm->in_list->GetNext();
									if (cp!=NULL)
									{
										if (cp->sock==sock)
										{
			
											//printf("link to [%p] changing to null in IN list...\n",cp->sock);
											prm->socklist_delete.LockWrite();
											cp->sock=NULL;
											prm->socklist_delete.UnLockWrite();
											//printf("link to [%p] changed to null in IN list!\n",cp->sock);
										}
									}
								}
							
								prm->in_cash_list->ResetCursor();
								for (unsigned int k=0; k<prm->in_cash_list->GetLen(); k++)
								{
									CCheckPay *cp=prm->in_cash_list->GetNext();
									if (cp!=NULL)
									{
										if (cp->sock==sock)
										{
											//printf("link to [%p] changing to null in CASH list...\n",cp->sock);
											prm->socklist_delete.LockWrite();
											cp->sock=NULL;
											prm->socklist_delete.UnLockWrite();
											//printf("link to [%p] changed to null in CASH list!\n",cp->sock);

									
										}
									}
								}
								deleted=true;
								i=prm->sock_list->GetLen();
							}
						}
					}
					prm->socklist_delete.LockWrite();
					if (deleted)
						prm->sock_list->RemoveThis();
					
					prm->socklist_delete.UnLockWrite();
					sem_post(prm->pc_lock);

				}
			}
			else
			{
			
				//sleep(1);
			}
		}
		sem_wait(prm->pc_lock);
		quit=prm->pc_exit;
//		printf("loop done!\n");
		sem_post(prm->pc_lock);

	}
	return NULL;
}


SPay *CompileCheckPay(char *data, int size, CSocketRW *sock, CPayguideClient *pgc, thread_param *prm)
{
	char unauth[2];
	unauth[0]=2;
	unauth[1]=0;
	unsigned char nettype=0x01;
	unsigned char netversion=0x00;
	SPay *result=NULL;
	if (size<1)
		return NULL;
	const char *pointer=data;
	memcpy(&nettype,data,1);
	if (size>1)
		memcpy(&netversion,(data+1),1);
	
	if (nettype==NETTYPE_GETUSERS)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{
			char buff[1024];
			buff[0]=22;
			buff[1]=0;
			char *ptr=buff+2;
			sem_wait(prm->pc_lock);
			prm->sock_list->ResetCursor();
			for (unsigned int i=0; i<prm->sock_list->GetLen(); i++)
			{
				CPayguideClient *pc=prm->sock_list->GetNext();
				if (pc!=NULL)
				{
					unsigned short l=(unsigned short)strlen(pc->GetName());
				
					memcpy(ptr, &l, sizeof(unsigned short));
					ptr+=sizeof(unsigned short);
				
					memcpy(ptr, pc->GetName(), (int)l);
					ptr+=(int)l;
				}
			}
			sem_post(prm->pc_lock);
			sock->Send(buff, ptr-buff);
		}
		else
			sock->Send(unauth, 2);
	}
	
	if (nettype==NETTYPE_PAYCHECK1 || nettype==NETTYPE_PAYCHECK2  || nettype==NETTYPE_PAYCHECK3)
	{
		if (sock->Authorized()>=AUTH_PAYCHECK)
		{
			/* Regular pay */
			long long pay_id=0;
			int provider=0;
			long long terminal_id=0;
			int currency=0;
			char msg[SIZE_DATA+1];
			char stamp[20];
			
			char magic;
			timeval time_now;
			tm *ptm;
			gettimeofday(&time_now, NULL);
			ptm = localtime (&time_now.tv_sec);
			strftime(stamp, 20, "%Y-%m-%d %H:%M:%S", ptm);
	
			pointer+=(SIZE_BODY_TYPE+SIZE_BODY_VERSION);
			
			memcpy(&pay_id, pointer, SIZE_BODY_ID);
			pointer+=SIZE_BODY_ID;
			
			memcpy(&magic, pointer, SIZE_BODY_MAGIC);
			pointer+=SIZE_BODY_MAGIC;
			
			memcpy(&provider, pointer, SIZE_BODY_OPERATOR);
			pointer+=SIZE_BODY_OPERATOR;
			
			memcpy(&terminal_id, pointer, SIZE_BODY_TERMINAL);
			pointer+=SIZE_BODY_TERMINAL;
	
			memcpy(&currency, pointer, SIZE_BODY_CURRENCY);
			pointer+=SIZE_BODY_CURRENCY;
			
			int l=size-SIZE_BODY_CURRENCY-SIZE_BODY_TERMINAL-SIZE_BODY_OPERATOR-SIZE_BODY_ID-SIZE_BODY_TYPE-SIZE_BODY_VERSION-SIZE_BODY_MAGIC;
			if (netversion>=0x01) l-=(SIZE_BODY_SUMMINT+SIZE_BODY_SUMMFLOAT)*2;
			if (netversion>=0x02) l-=(SIZE_BODY_SUMMINT+SIZE_BODY_SUMMFLOAT);

			if (l<0)
				l=0;
			else if (l>SIZE_DATA)
				l=SIZE_DATA;
			if (l>=0)
				memcpy(&msg, pointer, l);
			msg[l]=0;
			
			pointer+=l;
			float real_summ=0; float amount=0; float back_summ=0;
			unsigned int i_summ_buff=0;
			unsigned char f_summ_buff=0;
			if (netversion>=0x01)
			{
				memcpy(&i_summ_buff, pointer, SIZE_BODY_SUMMINT);
				pointer+=SIZE_BODY_SUMMINT;
				
				memcpy(&f_summ_buff, pointer, SIZE_BODY_SUMMFLOAT);
				pointer+=SIZE_BODY_SUMMFLOAT;
				real_summ=(float)i_summ_buff+(unsigned int)f_summ_buff/100.0f;
			}
			if (netversion>=0x02)
			{
				memcpy(&i_summ_buff, pointer, SIZE_BODY_SUMMINT);
				pointer+=SIZE_BODY_SUMMINT;
				
				memcpy(&f_summ_buff, pointer, SIZE_BODY_SUMMFLOAT);
				pointer+=SIZE_BODY_SUMMFLOAT;
				amount=(float)i_summ_buff+(unsigned int)f_summ_buff/100.0f;
			}

			if (netversion>=0x03)
			{
				memcpy(&i_summ_buff, pointer, SIZE_BODY_SUMMINT);
				pointer+=SIZE_BODY_SUMMINT;
				
				memcpy(&f_summ_buff, pointer, SIZE_BODY_SUMMFLOAT);
				pointer+=SIZE_BODY_SUMMFLOAT;
				back_summ=(float)i_summ_buff+(unsigned int)f_summ_buff/100.0f;
			}
			
			if (size!=pointer-data)
			{
				char tmp[256];
				snprintf(tmp, 256, "paycheck: wrong message lenght. l=%i in packet head but %i in real. Packet rejected.", size, (pointer-data));
				LogWrite(LOGMSG_WARNING, tmp);
			}
			else
			{
				result=CompileNewPay(pay_id, 10, provider, stamp, msg, 0, terminal_id, currency, 0, 0, -1,-1,-1);

				if (netversion>=0x01)
					result->summ=real_summ;

				if (netversion>=0x02)
					result->amount=amount;

				if (netversion>=0x03)
					result->back_summ=back_summ;
				
				result->magic=magic;
//				result->nettype=nettype;
				result->netversion=netversion;
			}
		}
		else
			sock->Send(unauth, 2);
	}

	if (nettype==NETTYPE_PING)
	{
		char ping[2];
		ping[0]=5;
		ping[1]=0;
		sock->Send(ping, 2);
	}
		
	if (nettype==NETTYPE_COMMAND)
	{
		//printf("case 4\n");
		/* Command */
		char cmd=0;
		int command=0;
		pointer+=(SIZE_BODY_TYPE+SIZE_BODY_VERSION);
		memcpy(&cmd, pointer, sizeof(cmd));
		command=(int)cmd;
		pointer+=sizeof(cmd);
		
		int l=size-sizeof(cmd);
		if (l>=0)
		{
			int result_size=0;
			ExecCmd(command, pointer, l, &result_size, sock);
		}
		else
		{
//			char tmp[256];
//			snprintf(tmp, 256, "paycheck: wrong message lenght. l=%i in packet head but %i in real. Packet rejected.");
//			LogWrite(LOGMSG_WARNING, tmp);
		}
		
	//	msg[l]=0;
		result=NULL;
	}
	
	if (nettype==NETTYPE_SIGN)
	{
		if (sock->Authorized()>=AUTH_CONTROL)
		{
			pgc->AddToDelivery();
			result=NULL;
		}
		else
			sock->Send(unauth, 2);
	}
	
	if (nettype==NETTYPE_UNSIGN)
	{
		if (sock->Authorized()>=AUTH_STAT)
		{
			pgc->RemoveFromDelivery();
			result=NULL;
		}
		else
			sock->Send(unauth, 2);
	}
	
	if (nettype==NETTYPE_AUTHORIZATION)
	{
		char login[1024];
		char password[1024];
		char *ptr=data+2;
		unsigned short login_size;
		unsigned short password_size;
		char unauth[2];
		unauth[0]=2;
		unauth[1]=0;

		if (size>4)
		{
			memcpy(&login_size,ptr, sizeof(login_size));
			ptr+=sizeof(login_size);

			memcpy(&password_size,ptr, sizeof(password_size));
			ptr+=sizeof(password_size);
			
			if (login_size<1024 && password_size<1024)
			{
				memcpy(login, ptr, login_size);
				login[login_size]=0;
				ptr+=login_size;
				memcpy(password, ptr, password_size);
				password[password_size]=0;
				ptr+=password_size;
				if (ptr-data==size)
				{
					int auth_level=GetAuthorization(login, password);
					if (auth_level==0)
					{
//						printf("Authorization failed\n");
						sock->Send(unauth, 2);
						sock->Down();
					}
					else
					{
						char auth[2];
						auth[0]=3;
						auth[1]=0;
						sock->Authorize(auth_level);
						pgc->Authorize(auth_level, login);
						sock->Send(auth, 2);
//						printf("User %s connected.\n", login);
					}
				}
				else
				{
					sock->Send(unauth, 2);
				}
			}
			else
				sock->Send(unauth, 2);
		}
		else
			sock->Send(unauth, 2);
	
	}
	//printf("returning\n");
	
	return result;
}

void SendLogMessages(int priority, const char *message)
{
		sem_wait(prm.pc_lock);
		prm.sock_list->ResetCursor();
		CPayguideClient *payguide_client;
		int l=strlen(message)+sizeof(priority)+2;
		char *msg=new char[l];
		char *ptr=msg;
		
		ptr[0]=20; ptr[1]=0;ptr+=2;
		memcpy(ptr, &priority, sizeof(priority));
		ptr+=sizeof(priority);
		memcpy(ptr, message, strlen(message));
		for (unsigned int i=0; i<prm.sock_list->GetLen(); i++)
		{
			payguide_client=prm.sock_list->GetNext();
			if (payguide_client!=NULL)
			{
				if (payguide_client->InDelivery())
				{
					CSocketRW *sock=payguide_client->GetSocket();
					if (sock!=NULL)
					{
						sock->Send(msg, l);
					}
				}
			}
		}
		delete [] msg;
		sem_post(prm.pc_lock);

}

int GetAuthorization(const char *login, const char *password)
{
//	printf("incoming login [%s], password [%s]\n", login, password);
	const char *auth_names[4];
	int adm_level=0;
	auth_names[0]="view";
	auth_names[1]="paycheck";
	auth_names[2]="control";
	auth_names[3]="admin";
	sem_wait(&users_lock);
	for (int i=0; i<4; i++)
	{
		const char *tmp=NULL;
		if ((tmp=users.GetValue(login,auth_names[i]))!=NULL)
		{
			char md5_inc[1024];
			if (CreateMD5HexSign(password, md5_inc, 1023)==0)
			{
				if (strcmp(tmp, md5_inc)==0)
					adm_level=i+1;
			}
		}
	}
	sem_post(&users_lock);
//	printf("returning %i\n", adm_level);
	return adm_level;
}
